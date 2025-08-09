// WebSocket setup
const ws = new WebSocket(`ws://www.wulabs.com:81`);
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
const scale = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"]
const octaves = ["Low", "Mid", "High"];

// Create Whacker Quantity parameter list
const col0 = document.getElementById('col0'),
      col1 = document.getElementById('col1'),
      col2 = document.getElementById('col2');

// Label each note in quantity table
for (let i = 0; i < 32; i++) {
  // Set displayed whacker name in table
  const label = document.createElement('label');
  let octave = octaves[Math.floor(i / 12)];
  let note = scale[i % 12];
  label.textContent = `${octave} ${note}`;

  // Create input
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

// Update player display
function renderPlayers(n) {
  playersBody.innerHTML = '';
  for (let i = 0; i < n; i++) {
    const tr = document.createElement('tr');
    tr.innerHTML = `
      <td>Player ${i}</td>
      <td><input type="number" step="1" name="holdLimit${i}" min="0" max="99"></td>
      <td><input type="number" name="switchTime${i}" min="0" max="99"></td>
      <td><input type="checkbox" name="oneHandedRolls${i}"></td>
      <td><button class="details-btn" aria-expanded="false" aria-controls="exclude-${i}" data-player-index="${i}">▾</button></td>
    `;
    playersBody.appendChild(tr);

    const details = document.createElement('tr');
    details.id = `exclude-${i}`;
    details.className = 'details-row';
    details.setAttribute('hidden', '');
    details.innerHTML = `
      <td colspan="5">
        <div class="range-inputs" role="group" aria-label="Exclude ranges for Player ${i}"></div>
        <div class="hint">Enter time ranges like <code>3.5-7.2</code> (seconds). Must be <code>start &gt; 0</code> and <code>end &gt; start</code>. A new field appears when the last one is valid. Esc or click to close.</div>
      </td>
    `;
    playersBody.appendChild(details);
  }
}

// Re-render player table on initial load and input
numPlayersInput.addEventListener('input', () => {
  renderPlayers(+numPlayersInput.value || 0);

// ====== Exclude Range Menu Helpers ======
function normalizeDash(s) { return String(s).replace(/[–—−]/g, '-'); }

function cleanInputString(s) {
  s = normalizeDash(s);
  s = s.replace(/[^0-9.\- ]+/g, ''); // keep digits, dot, hyphen, space
  s = s.replace(/\s+/g, ' ').trim();
  s = s.replace(/-{2,}/g, '-');
  return s;
}

// Parse "a-b" floats; both > 0 and end > start
function parseFloatRange(s) {
  s = normalizeDash(String(s)).trim();
  const m = s.match(/^(\d*\.?\d+)\s*-\s*(\d*\.?\d+)$/);
  if (!m) return { ok:false };
  const start = parseFloat(m[1]);
  const end   = parseFloat(m[2]);
  if (!(start > 0) || !(end > 0) || !(end > start)) return { ok:false };
  return { ok:true, start, end };
}

function setValidity(el, ok) { el.setAttribute('aria-invalid', ok ? 'false' : 'true'); }

function addEmptyRange(detailsRow) {
  const wrap = detailsRow.querySelector('.range-inputs');
  const idx  = wrap.querySelectorAll('.range-input').length + 1;

  const input = document.createElement('input');
  input.type = 'text';
  input.className = 'range-input';
  input.placeholder = `Range ${idx} (e.g., 3.5-7.2)`;
  input.inputMode = 'decimal';
  input.autocomplete = 'off';
  input.setAttribute('aria-label', `Range ${idx} start-end in seconds`);
  input.setAttribute('aria-invalid', 'true');

  input.addEventListener('input', (e) => {
    const cleaned = cleanInputString(e.target.value);
    if (e.target.value !== cleaned) e.target.value = cleaned;
    const { ok } = parseFloatRange(e.target.value);
    setValidity(e.target, ok);
    ensureTrailingEmpty(detailsRow);
  });

  input.addEventListener('keydown', (e) => {
    const allowed = ['Backspace','Delete','ArrowLeft','ArrowRight','Tab','Home','End','Enter'];
    if (allowed.includes(e.key)) return;
    if (/^[0-9.\- ]$/.test(e.key)) return;
    e.preventDefault();
  });

  input.addEventListener('blur', (e) => {
    const v = e.target.value.trim();
    const { ok, start, end } = parseFloatRange(v);
    if (ok) {
      e.target.value = `${start}-${end}`;
      setValidity(e.target, true);
    }
  });

  wrap.appendChild(input);
  return input;
}

function ensureTrailingEmpty(detailsRow) {
  const fields = Array.from(detailsRow.querySelectorAll('.range-input'));
  if (fields.length === 0) { addEmptyRange(detailsRow); return; }
  const last = fields[fields.length - 1];
  const { ok } = parseFloatRange(last.value);
  if (ok) addEmptyRange(detailsRow);
}

function toggleDetails(btn) {
  const id = btn.getAttribute('aria-controls');
  const detailsRow = document.getElementById(id);
  const isHidden = detailsRow.hasAttribute('hidden');

  // Close others
  document.querySelectorAll('.details-row').forEach(row => {
    row.setAttribute('hidden','');
    const ctrlBtn = document.querySelector(`.details-btn[aria-controls="${row.id}"]`);
    if (ctrlBtn) ctrlBtn.setAttribute('aria-expanded','false');
  });

  if (isHidden) {
    detailsRow.removeAttribute('hidden');
    btn.setAttribute('aria-expanded','true');
    ensureTrailingEmpty(detailsRow);
    const first = detailsRow.querySelector('.range-input');
    if (first) first.focus();
  } else {
    detailsRow.setAttribute('hidden','');
    btn.setAttribute('aria-expanded','false');
  }
}

// Delegated events for players table
playersBody.addEventListener('click', (e) => {
  const btn = e.target.closest('.details-btn');
  if (!btn) return;
  toggleDetails(btn);
});

document.addEventListener('click', (e) => {
  const inDetails = e.target.closest('.details-row');
  const btn = e.target.closest('.details-btn');
  if (!inDetails && !btn) {
    document.querySelectorAll('.details-row').forEach(row => row.setAttribute('hidden',''));
    document.querySelectorAll('.details-btn').forEach(b => b.setAttribute('aria-expanded','false'));
  }
});

document.addEventListener('keydown', (e) => {
  if (e.key === 'Escape') {
    document.querySelectorAll('.details-row').forEach(row => row.setAttribute('hidden',''));
    document.querySelectorAll('.details-btn').forEach(b => b.setAttribute('aria-expanded','false'));
  }
});
// ====== End Exclude Range Menu Helpers ======

});
renderPlayers(+numPlayersInput.value || 0);

// ====== Exclude Range Menu Helpers ======
function normalizeDash(s) { return String(s).replace(/[–—−]/g, '-'); }

function cleanInputString(s) {
  s = normalizeDash(s);
  s = s.replace(/[^0-9.\- ]+/g, ''); // keep digits, dot, hyphen, space
  s = s.replace(/\s+/g, ' ').trim();
  s = s.replace(/-{2,}/g, '-');
  return s;
}

// Parse "a-b" floats; both > 0 and end > start
function parseFloatRange(s) {
  s = normalizeDash(String(s)).trim();
  const m = s.match(/^(\d*\.?\d+)\s*-\s*(\d*\.?\d+)$/);
  if (!m) return { ok:false };
  const start = parseFloat(m[1]);
  const end   = parseFloat(m[2]);
  if (!(start > 0) || !(end > 0) || !(end > start)) return { ok:false };
  return { ok:true, start, end };
}

function setValidity(el, ok) { el.setAttribute('aria-invalid', ok ? 'false' : 'true'); }

function addEmptyRange(detailsRow) {
  const wrap = detailsRow.querySelector('.range-inputs');
  const idx  = wrap.querySelectorAll('.range-input').length + 1;

  const input = document.createElement('input');
  input.type = 'text';
  input.className = 'range-input';
  input.placeholder = `Range ${idx} (e.g., 3.5-7.2)`;
  input.inputMode = 'decimal';
  input.autocomplete = 'off';
  input.setAttribute('aria-label', `Range ${idx} start-end in seconds`);
  input.setAttribute('aria-invalid', 'true');

  input.addEventListener('input', (e) => {
    const cleaned = cleanInputString(e.target.value);
    if (e.target.value !== cleaned) e.target.value = cleaned;
    const { ok } = parseFloatRange(e.target.value);
    setValidity(e.target, ok);
    ensureTrailingEmpty(detailsRow);
  });

  input.addEventListener('keydown', (e) => {
    const allowed = ['Backspace','Delete','ArrowLeft','ArrowRight','Tab','Home','End','Enter'];
    if (allowed.includes(e.key)) return;
    if (/^[0-9.\- ]$/.test(e.key)) return;
    e.preventDefault();
  });

  input.addEventListener('blur', (e) => {
    const v = e.target.value.trim();
    const { ok, start, end } = parseFloatRange(v);
    if (ok) {
      e.target.value = `${start}-${end}`;
      setValidity(e.target, true);
    }
  });

  wrap.appendChild(input);
  return input;
}

function ensureTrailingEmpty(detailsRow) {
  const fields = Array.from(detailsRow.querySelectorAll('.range-input'));
  if (fields.length === 0) { addEmptyRange(detailsRow); return; }
  const last = fields[fields.length - 1];
  const { ok } = parseFloatRange(last.value);
  if (ok) addEmptyRange(detailsRow);
}

function toggleDetails(btn) {
  const id = btn.getAttribute('aria-controls');
  const detailsRow = document.getElementById(id);
  const isHidden = detailsRow.hasAttribute('hidden');

  // Close others
  document.querySelectorAll('.details-row').forEach(row => {
    row.setAttribute('hidden','');
    const ctrlBtn = document.querySelector(`.details-btn[aria-controls="${row.id}"]`);
    if (ctrlBtn) ctrlBtn.setAttribute('aria-expanded','false');
  });

  if (isHidden) {
    detailsRow.removeAttribute('hidden');
    btn.setAttribute('aria-expanded','true');
    ensureTrailingEmpty(detailsRow);
    const first = detailsRow.querySelector('.range-input');
    if (first) first.focus();
  } else {
    detailsRow.setAttribute('hidden','');
    btn.setAttribute('aria-expanded','false');
  }
}

// Delegated events for players table
playersBody.addEventListener('click', (e) => {
  const btn = e.target.closest('.details-btn');
  if (!btn) return;
  toggleDetails(btn);
});

document.addEventListener('click', (e) => {
  const inDetails = e.target.closest('.details-row');
  const btn = e.target.closest('.details-btn');
  if (!inDetails && !btn) {
    document.querySelectorAll('.details-row').forEach(row => row.setAttribute('hidden',''));
    document.querySelectorAll('.details-btn').forEach(b => b.setAttribute('aria-expanded','false'));
  }
});

document.addEventListener('keydown', (e) => {
  if (e.key === 'Escape') {
    document.querySelectorAll('.details-row').forEach(row => row.setAttribute('hidden',''));
    document.querySelectorAll('.details-btn').forEach(b => b.setAttribute('aria-expanded','false'));
  }
});
// ====== End Exclude Range Menu Helpers ======


// Define configuration presets
const presets = {
  // Basic preset
  basic: {
    whackers: Array.from({length: 32}, () => 2),
    players: Array.from({length: 9}, () => ({holdLimit: 2, switchTime: 2.0, oneHandedRoll: false})),
  },
  // Texas SOUnD preset
  sound: {
    whackers: [
      2, 3, 5, 3, 5, 5, 3, 5, 3, 5, 3, 5,
      7, 4, 4, 4, 4, 4, 4, 5, 4, 4, 4, 4,
      2, 2, 2, 2, 2, 2, 2, 2
    ],
    players: Array.from({length: 9}, () => ({holdLimit: 2, switchTime: 2.0, oneHandedRoll: false})),
  },
  // Harvard THUD preset
  harvard: {
    whackers: Array.from({length:32}, () => 4),
    players: Array.from({length: 9}, () => ({holdLimit: 2, switchTime: 2.0, oneHandedRoll: false})),
  },
  // A&M percussion studio preset
  studio: {
    whackers: Array.from({length:32}, ()=>1),
    players: Array.from({length: 6}, () => ({holdLimit: 1, switchTime: 4.0, oneHandedRoll: false})),
  },
  // Clear all values
  clear: {
    whackers: Array.from({length:32},()=>''), 
    players: []                    
  }
};

// Apply selected preset
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
      document.querySelector(`input[name=oneHandedRolls${i}]`).checked = pl.oneHandedRoll;
    });
  });
});

// Send command request to server
document.getElementById('sendBtn').onclick = async () => {
  // Collect file from document
  const fileInput = document.getElementById('scoreFile');
  if (!fileInput.files.length) {
    return alert('Please select a MuseScore file');
  }
  const file = fileInput.files[0];
  const fileName = file.name;
  const fileBuf = await file.arrayBuffer();
  
  // Collect player data
  const numPlayers = +numPlayersInput.value || 0;
  if (numPlayers == 0) {
    alert('Specify a quantity of players.');
    return;
  }
  const playerSwitchTimes = [];
  const playerHoldLimits = [];
  const playerOneHandedRolls = [];
  const playerExcludeRanges = [];
  for (let i = 0; i < numPlayers; i++) {
    playerSwitchTimes.push(Number(document.querySelector(`input[name=switchTime${i}]`).value) || 0);
    playerHoldLimits.push(Number(document.querySelector(`input[name=holdLimit${i}]`).value) || 0);
    playerOneHandedRolls.push(document.querySelector(`input[name=oneHandedRolls${i}]`).checked);

    // Collect player exclude ranges
    const detailsRow = document.getElementById(`exclude-${i}`);
    const ranges = Array.from(detailsRow ? detailsRow.querySelectorAll('.range-input') : [])
      .map(inp => parseFloatRange(inp.value))
      .filter(r => r.ok)
      .map(r => [r.start, r.end]);
    playerExcludeRanges.push(ranges);
  }
  
  // Collect whacker quantities
  const whackerQuantities = [];
  for (let i = 0; i < 32; i++) {
    whackerQuantities.push(Number(document.querySelector(`input[name=Param${i}]`).value) || 0);
  }

  const gamble = document.getElementById("seed").checked;

  // Serialize JSON object with parameters
  const cmd = {command: 'COLOR_NEW_MUSESCORE', params: {numPlayers, playerHoldLimits, playerSwitchTimes, playerOneHandedRolls, playerExcludeRanges, whackerQuantities, gamble}, fileName};
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

// Initialize default receive status
let incomingName = 'default.zip';
let ok = true;

// Receive server message
ws.onmessage = msg => {
  // Handle non-file messages
  if (typeof msg.data === 'string') {
    try {
      const pkt = JSON.parse(msg.data);
      switch (pkt.type) {
        case 'progress': 
          // Update progress status
          statusEl.textContent = pkt.message;
          break;
        case 'filename':
          // Set zip file name to download
          incomingName = pkt.message;
          break;
        case 'error': {
          // Display error
          let errMsg = 'Error: ' + pkt.message;
          console.error(errMsg);
          ok = window.confirm(errMsg + "\nDownload anyway?");
          break;
        }
        case 'shutdown': {
          // Server shutdown
          let errMsg = 'Error: ' + pkt.message;
          console.error(errMsg);
          alert(errMsg);
          break;
        }
        case 'debug':
          // Debug message
          console.log('Debug:', pkt.message);
          break;
        default:
          console.error('Unknown packet type:', pkt.type);
          console.error('Message contents:', pkt);
      }
    } catch {
      console.warn('Text message', msg.data);
    }
    return;
  }

  // Cancel download if user rejected
  if (!ok) {
    statusEl.textContent = '';
    ok = true;
    return;
  }

  // Download ZIP file from server
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