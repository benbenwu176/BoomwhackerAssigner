// WebSocket setup
const ws = new WebSocket(`ws://${location.host}`);
ws.binaryType = 'arraybuffer';

// Load tooltips
document.querySelectorAll('.tooltip').forEach(tooltip => {
  const text     = tooltip.dataset.tooltip;
  const contentEl = tooltip.querySelector('.tooltip-text');
  if (contentEl) contentEl.textContent = text;
});

// Status line
const statusEl = document.getElementById('status');

// Whacker name mapping
const scale = ["C", "C#", "D ", "D#", "E ", "F ", "F#", "G ", "G#", "A ", "A#", "B "]
const octaves = ["Low", "Mid", "High"];

// Create Whacker Quantity parameter list
const col0 = document.getElementById('col0'),
      col1 = document.getElementById('col1'),
      col2 = document.getElementById('col2');

// Label each note in quantity table
for (let i = 0; i < 32; i++) {
  const label = document.createElement('label');
  label.textContent = `Param${i}:`;
  let octave = octaves[Math.floor(i / 12)];
  let note = scale[i % 12];
  label.textContent = `${octave} ${note}`;
  const input = document.createElement('input');
  input.type = 'number';
  input.name = `Param${i}`;
  input.min = 0;
  input.max = 99;
  label.appendChild(input);
  if (i < 12)      col0.appendChild(label);
  else if (i < 24) col1.appendChild(label);
  else             col2.appendChild(label);
}

// Create player table
const numPlayersInput = document.getElementById('numPlayers');
const playersBody = document.getElementById('players-body');
function renderPlayers(n) {
  playersBody.innerHTML = '';
  for (let i = 0; i < n; i++) {
    const tr = document.createElement('tr');
    tr.innerHTML = `
      <td>Player ${i}</td>
      <td><input type="number" name="holdLimit${i}" min="0" max="99"></td>
      <td><input type="number" name="switchTime${i}" min="0" max="99"></td>
    `;
    playersBody.appendChild(tr);
  }
}

// Re-render player table on initial load and input
numPlayersInput.addEventListener('input', () => {
  renderPlayers(+numPlayersInput.value || 0);
});
renderPlayers(+numPlayersInput.value || 0);

// Define configuration presets
const examplePlayer = ({holdLimit: 2, switchTime: 2.0});
const presets = {
  basic: {
    whackers: Array.from({length: 32}, () => 2),
    players: Array.from({length: 9}, () => ({holdLimit: 2, switchTime: 2.0})),
  },
  sound: {
    whackers: [
      2, 3, 5, 3, 5, 5, 3, 5, 3, 5, 3, 5,
      7, 4, 4, 4, 4, 4, 4, 5, 4, 4, 4, 4,
      2, 2, 2, 2, 2, 2, 2, 2
    ],
    players: Array.from({length: 9}, () => ({holdLimit: 2, switchTime: 2.0})),
  },
  harvard: {
    whackers: Array.from({length:32}, () => 99),
    players: Array.from({length: 8}, () => ({holdLimit: 2, switchTime: 1.5})),
  },
  studio: {
    whackers: Array.from({length:32}, ()=>1),
    players: Array.from({length: 6}, () => ({holdLimit: 1, switchTime: 4.0})),
  },
  clear: {
    whackers: Array.from({length:32},()=>''), 
    players: []                    
  }
};

// --- Apply a preset ---
document.querySelectorAll('.preset-btn').forEach(btn=>{
  btn.addEventListener('click', () => {
    const p = presets[btn.dataset.preset];

    // Set whacker quantities
    p.whackers.forEach((val,i) => {
      document.querySelector(`input[name=Param${i}]`).value = val;
    });

    // Set player values
    numPlayersInput.value = p.players.length;
    renderPlayers(p.players.length);
    p.players.forEach((pl,i) => {
      document.querySelector(`input[name=holdLimit${i}]`).value  = pl.holdLimit;
      document.querySelector(`input[name=switchTime${i}]`).value = pl.switchTime;
    });
  });
});

// --- Send logic ---
document.getElementById('sendBtn').onclick = async () => {
  // Collect file from document
  const fileInput = document.getElementById('scoreFile');
  if (!fileInput.files.length) {
    return alert('Please select a MuseScore file');
  }
  const file = fileInput.files[0];
  const fileName = file.name;
  const fileBuf = await file.arrayBuffer();
  
  // Collect players
  const numPlayers = +numPlayersInput.value || 0;
  if (numPlayers == 0) {
    alert('Invalid configuration.');
    return;
  }
  const playerSwitchTimes = [];
  const playerHoldLimits = [];
  for (let i = 0; i < numPlayers; i++) {
    playerSwitchTimes.push(Number(document.querySelector(`input[name=switchTime${i}]`).value) || 0);
    playerHoldLimits.push(Number(document.querySelector(`input[name=holdLimit${i}]`).value) || 0);
  }
  
  // Collect whacker quantities
  const whackerQuantities = [];
  for (let i = 0; i < 32; i++) {
    whackerQuantities.push(Number(document.querySelector(`input[name=Param${i}]`).value) || 0);
  }

  // Serialize JSON object with parameters
  const cmd = {command: 'COLOR_NEW_MUSESCORE', params: {numPlayers, playerHoldLimits, playerSwitchTimes, whackerQuantities}, fileName};
  const encoder = new TextEncoder();
  const jsonBuf = encoder.encode(JSON.stringify(cmd)); 

  // Build a buffer to send as a message over WebSocket
  const headerLength = 4;
  const totalLength = headerLength + jsonBuf.byteLength + fileBuf.byteLength;
  let message = new ArrayBuffer(totalLength);
  const view = new DataView(message);
  let offset = 0;

  // Write JSON length header as UInt32
  view.setUint32(offset, jsonBuf.byteLength, false);
  offset += headerLength;

  // Write JSON parameter object
  new Uint8Array(message, offset, jsonBuf.byteLength).set(jsonBuf);
  offset += jsonBuf.byteLength;

  // Write file
  new Uint8Array(message, offset, fileBuf.byteLength).set(new Uint8Array(fileBuf));

  // Send message and update status
  ws.send(message);
  statusEl.textContent = 'Sent, waiting for response…';
};

let incomingName = 'default.zip';

// --- Response handling ---
ws.onmessage = msg => {
  // Handle non-binary messages
  if (typeof msg.data === 'string') {
    try {
      const pkt = JSON.parse(msg.data);
      switch (pkt.type) {
        case 'progress': 
          statusEl.textContent = pkt.message;
          break;
        case 'filename':
          incomingName = pkt.message;
          break;
        case 'error':
          let errMsg = 'Error: ' + pkt.message;
          console.error(errMsg);
          alert(errMsg);
          break;
        case 'debug':
          console.log('Debug:', pkt.message);
        default:
          console.error('Unknown packet type:', pkt.type);
          console.error('Message contents:', pkt);
      }
    } catch {
      console.warn('Text message', msg.data);
    }
    return;
  }

  // Binary ZIP
  statusEl.textContent = 'Download ready';
  const blob = new Blob([msg.data], { type: 'application/zip' });
  const url  = URL.createObjectURL(blob);
  const a    = document.createElement('a');
  a.href     = url;
  a.download = incomingName;
  a.click();
  URL.revokeObjectURL(url);
  statusEl.textContent = '';
};

ws.onerror = (err) => {
  console.error(err);
  alert('Websocket Error');
}

ws.onclose = () => {
  console.log('WS closed');
}