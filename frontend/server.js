const express = require('express');
const http = require('http');
const { WebSocket, WebSocketServer } = require('ws');
const { spawn } = require('child_process');
const fs = require('fs');
const path = require('path');
const os = require('os');
const crypto = require('crypto');
const archiver = require('archiver');
const { clearInterval } = require('timers');

const app = express();
let publicPath = path.join(__dirname, 'public');
app.use(express.static(publicPath));
const server = http.createServer(app);

const wss = new WebSocketServer({ server });
let connections = [];

const progressPath = path.join(__dirname, 'progress.txt');
const progressMessages = fs.readFileSync(progressPath, 'utf-8').split(/\r?\n/);

// Gen script locations
const projectDir = path.dirname(__dirname);
const script = 'gen.py';
const scriptDir = 'src';
const scriptPath = path.join(projectDir, scriptDir, script);
const genExe = 'gen.exe';
const genDir = 'build';
const genPath = path.join(projectDir, genDir, genExe);

// Zip a directory
function zipDirectory(sourceDir, outPath) {
  return new Promise((resolve, reject) => {
    const output = fs.createWriteStream(outPath);
    const archive = archiver('zip', { zlib: { level: 9 } });

    output.on('close', () => resolve());
    archive.on('error', err => reject(err));

    archive.pipe(output);
    archive.glob('**/*', {
      cwd: sourceDir,
      ignore: [path.basename(outPath)]
    });
    archive.finalize();
  });
}

function sendProgress(ws, progressIndex) {
  if (ws.readyState === WebSocket.OPEN) {
    let msg = {type: 'progress', message: progressMessages[progressIndex]};
    ws.send(JSON.stringify(msg));
  }
}

function generateAssignment(ws, params, fileName, fileBuf) {
  const tmpDir = fs.mkdtempSync(path.join(os.tmpdir(), 'proc-'));
  
  // Isolate musescore name and extension
  const parsed = path.parse(fileName);
  const fileBase = parsed.name;
  const fileExt = parsed.ext;

  // Create temporary file locations for client input
  const scorePath = path.join(tmpDir, fileName);
  const paramsPath = path.join(tmpDir, 'params.json');

  // Write uploaded musescore and parameters
  fs.writeFileSync(scorePath, fileBuf);
  fs.writeFileSync(paramsPath, JSON.stringify(params, null, 2) + '\n');

  // Call python script
  const py = spawn('python', [scriptPath, tmpDir, scorePath, paramsPath, genPath]);

  // Send pseudo-progress commands
  sendProgress(ws, 0);
  const ticker = setInterval(() => sendProgress(ws, Math.floor(Math.random() * progressMessages.length)), 5000);

  // Collect stdout of python process
  let stdoutBuf = '';
  py.stdout.setEncoding('utf8');
  py.stdout.on('data', chunk => {
    stdoutBuf += chunk;
  });

  let stderrBuf = '';
  py.stderr.setEncoding('utf8');
  py.stderr.on('data', chunk => {
    stderrBuf += chunk;
  });

  py.on('exit', code => {
    clearInterval(ticker);
    console.log('Python:', stdoutBuf);
    console.log('Python debug:', stderrBuf);
    if (code !== 0) {
      let errorMsg = {type: 'error', message: 'Python script failed.'};
      let debugMsg = {type: 'debug', message: stdoutBuf};
      ws.send(JSON.stringify(errorMsg));
      ws.send(JSON.stringify(debugMsg));
    }

    // Format final zip file name
    const now = new Date();
    const mm = String(now.getMonth() + 1).padStart(2, '0');
    const dd   = String(now.getDate()).padStart(2, '0');
    const yy   = String(now.getFullYear()).slice(-2);
    const timestamp = `${mm}-${dd}-${yy}`;
    const curTime = now.getHours() * 3600 + now.getMinutes() * 60 + now.getSeconds();
    const seconds = String(curTime).padStart(5, '0');
    const zipName = `${fileBase}_${timestamp}_${seconds}.zip`;
    const zipPath = path.join(tmpDir, zipName);

    // Zip all files in temporary directory and send zip file over websocket
    zipDirectory(tmpDir, zipPath)
      .then(() => {
        const zipBuf = fs.readFileSync(zipPath);
        let msg = {type: 'filename', message: zipName};
        ws.send(JSON.stringify(msg));
        ws.send(zipBuf);
        fs.rmSync(tmpDir, {recursive: true, force: true});
      }).catch(err => {
        let msg = {type: 'error', message: `Failed to zip files: ${err.message}`};
        ws.send(JSON.stringify(msg));
        fs.rmSync(tmpDir, {recursive: true, force: true});
    });
  });
}

wss.on('connection', ws => {
  connections.push(ws);

  ws.on('message', data => {
    if (!Buffer.isBuffer(data)) {
      console.log('Unexpected text frame:', message);
      return;
    }

    // Read message
    let offset = 0;
    const jsonLen = data.readUInt32BE(0);
    offset += 4;
    const jsonBuf = data.subarray(offset, offset + jsonLen);
    offset += jsonLen;
    const fileBuf = data.subarray(offset);

    // Parse JSON object
    let message;
    try {
      message = JSON.parse(jsonBuf.toString('utf8'));
    } catch (err) {
      console.error('Invalid JSON header:', err);
      return;
    }

    console.log('Received message.');

    switch (message.command) {
      case 'COLOR_NEW_MUSESCORE': {
        generateAssignment(ws, message.params, message.fileName, fileBuf);
        break;
      }
      default: {
        console.error('Unknown command.');
        let msg = {type: 'error', message: 'Unknown command or malformed data.'};
        ws.send(JSON.stringify(msg));
      }
    }
  });

   ws.on('close', () => {
    // Remove from connections on close
    const i = connections.indexOf(ws);
    if (i !== -1) connections.splice(i, 1);
  });

  ws.on('error', () => {
    // Remove from connecitons on error
    const i = connections.indexOf(ws);
    if (i !== -1) connections.splice(i, 1);
  });

});

// Listen on webserver
const PORT = process.env.PORT || 8081;
server.listen(PORT, () => {
  console.log(`Listening on http://localhost:${PORT}`);
});

// 3) Helper to broadcast an error to *your* array
function broadcastError(text) {
  const msg = JSON.stringify({ type: 'error', message: text});
  for (const sock of connections) {
    if (sock.readyState === WebSocket.OPEN) {
      sock.send(msg);
    }
  }
}

// 4) On fatal server errors, notify everybody then exit
process.on('uncaughtException', err => {
  console.error('Fatal:', err);
  broadcastError('Server encountered a fatal error and will shut down.');
  setTimeout(() => process.exit(1), 500);
});

process.on('unhandledRejection', reason => {
  console.error('Unhandled Rejection:', reason);
  broadcastError('Server promise rejection—shutting down.');
  setTimeout(() => process.exit(1), 500);
});

// (Optional) graceful shutdown via signals
['SIGINT','SIGTERM'].forEach(sig => {
  process.on(sig, () => {
    console.log(`Received ${sig}, notifying clients…`);
    broadcastError('Server is shutting down.');
    setTimeout(() => process.exit(0), 500);
  });
});