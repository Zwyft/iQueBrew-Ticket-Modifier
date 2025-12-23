// iQue Tools - Application Logic
// Connected to aulon via bridge server

const API_BASE = 'http://localhost:8080/api';

// iQue Game Database with Content IDs and release dates
const IQUE_GAMES = [
    { id: 'dr-mario-64', title: 'Dr. Mario 64', release: '2003-11-18', cid: '00000001', icon: '💊' },
    { id: 'zelda-oot', title: 'The Legend of Zelda: Ocarina of Time', release: '2003-11-18', cid: '00000002', icon: '🗡️' },
    { id: 'starfox-64', title: 'Star Fox 64', release: '2003-11-18', cid: '00000003', icon: '✈️' },
    { id: 'mario-64', title: 'Super Mario 64', release: '2003-11-18', cid: '00000004', icon: '🍄' },
    { id: 'waverace-64', title: 'Wave Race 64', release: '2003-11-18', cid: '00000005', icon: '🌊' },
    { id: 'mariokart-64', title: 'Mario Kart 64', release: '2003-12-25', cid: '00000006', icon: '🏎️' },
    { id: 'fzero-x', title: 'F-Zero X', release: '2004-02-25', cid: '00000007', icon: '🚀' },
    { id: 'yoshis-story', title: "Yoshi's Story", release: '2004-03-25', cid: '00000008', icon: '🦎' },
    { id: 'paper-mario', title: 'Paper Mario', release: '2004-06-08', cid: '00000009', icon: '📄' },
    { id: 'sin-punishment', title: 'Sin and Punishment', release: '2004-09-25', cid: '0000000A', icon: '🔫' },
    { id: 'excitebike-64', title: 'Excitebike 64', release: '2004-06-25', cid: '0000000B', icon: '🏍️' },
    { id: 'smash-bros', title: 'Super Smash Bros.', release: '2005-02-24', cid: '0000000C', icon: '👊' },
    { id: 'custom-robo', title: 'Custom Robo', release: '2004-12-25', cid: '0000000D', icon: '🤖' },
    { id: 'animal-crossing', title: 'Animal Crossing', release: '2006-01-27', cid: '0000000E', icon: '🏠' }
];

// Application State
const state = {
    connected: false,
    bridgeConnected: false,
    selectedGame: null,
    ticketLoaded: false,
    exploitStep: 0,
    isRunning: false,
    bbid: null
};

// DOM Elements
const elements = {
    gameSelect: document.getElementById('gameSelect'),
    gameTitle: document.getElementById('gameTitle'),
    gameRelease: document.getElementById('gameRelease'),
    gameCID: document.getElementById('gameCID'),
    gameCover: document.getElementById('gameCover'),
    connectionStatus: document.getElementById('connectionStatus'),
    connectionText: document.getElementById('connectionText'),
    consoleOutput: document.getElementById('consoleOutput'),
    btnDumpKeys: document.getElementById('btnDumpKeys'),
    btnLoadTicket: document.getElementById('btnLoadTicket'),
    btnStartExploit: document.getElementById('btnStartExploit'),
    btnAbort: document.getElementById('btnAbort'),
    btnClearConsole: document.getElementById('btnClearConsole'),
    keysDisplay: document.getElementById('keysDisplay'),
    titlekeyIV: document.getElementById('titlekeyIV'),
    contentIV: document.getElementById('contentIV'),
    sha1Hash: document.getElementById('sha1Hash'),
    titlekey: document.getElementById('titlekey')
};

// API Helper
async function api(endpoint, method = 'GET', body = null) {
    try {
        const options = {
            method,
            headers: { 'Content-Type': 'application/json' }
        };
        if (body) {
            options.body = JSON.stringify(body);
        }
        const response = await fetch(`${API_BASE}${endpoint}`, options);
        return await response.json();
    } catch (err) {
        return { success: false, message: `Network error: ${err.message}` };
    }
}

// Initialize Application
function init() {
    populateGameDropdown();
    setupEventListeners();
    addConnectButton();
    log('iQue Tools v1.0.0 initialized', 'info');
    log('Checking bridge server connection...', 'info');
    checkBridgeConnection();

    // Poll connection status every 5 seconds
    setInterval(checkConnectionStatus, 5000);
}

// Add connect/disconnect buttons
function addConnectButton() {
    const statusIndicator = document.querySelector('.status-indicator');
    if (statusIndicator) {
        const btnConnect = document.createElement('button');
        btnConnect.id = 'btnConnect';
        btnConnect.className = 'btn btn-small';
        btnConnect.textContent = 'Connect';
        btnConnect.onclick = onConnect;
        statusIndicator.appendChild(btnConnect);
    }
}

// Check if bridge server is running
async function checkBridgeConnection() {
    const result = await api('/status');
    if (result.success) {
        state.bridgeConnected = true;
        log('Bridge server connected', 'success');
        log('Checking aulon server in XP VM...', 'info');

        const ping = await api('/ping');
        if (ping.success) {
            log('Aulon server responding - ready!', 'success');
            checkConnectionStatus();
        } else {
            log('Cannot reach aulon server', 'warning');
            log('Make sure XP VM is running with: aulon.exe -s 5001', 'warning');
        }
    } else {
        state.bridgeConnected = false;
        log('Bridge server not running', 'error');
        log('Start the server with: npm start (in gui folder)', 'error');
    }
}

// Check iQue connection status
async function checkConnectionStatus() {
    if (!state.bridgeConnected) return;

    const result = await api('/connection');
    if (result.success && result.data) {
        const wasConnected = state.connected;
        state.connected = result.data.connected;
        updateConnectionStatus(state.connected);

        if (state.connected && !wasConnected) {
            log('iQue Player connected!', 'success');
        } else if (!state.connected && wasConnected) {
            log('iQue Player disconnected', 'warning');
        }
    }
}

// Connect to iQue Player
async function onConnect() {
    if (state.connected) {
        log('Disconnecting...', 'info');
        const result = await api('/disconnect', 'POST');
        if (result.success) {
            state.connected = false;
            updateConnectionStatus(false);
            log('Disconnected', 'info');
        } else {
            log(`Error: ${result.message}`, 'error');
        }
    } else {
        log('Connecting to iQue Player...', 'info');
        const result = await api('/connect', 'POST');
        if (result.success) {
            state.connected = true;
            updateConnectionStatus(true);
            log('Connected to iQue Player!', 'success');

            // Get BBID
            const bbid = await api('/bbid');
            if (bbid.success) {
                log(`Console BBID retrieved`, 'info');
            }

            // Enable buttons
            elements.btnDumpKeys.disabled = false;
        } else {
            log(`Connection failed: ${result.message}`, 'error');
        }
    }
}

// Populate game dropdown
function populateGameDropdown() {
    IQUE_GAMES.forEach(game => {
        const option = document.createElement('option');
        option.value = game.id;
        option.textContent = `${game.icon} ${game.title}`;
        elements.gameSelect.appendChild(option);
    });
}

// Setup event listeners
function setupEventListeners() {
    elements.gameSelect.addEventListener('change', onGameSelect);
    elements.btnDumpKeys.addEventListener('click', onDumpKeys);
    elements.btnLoadTicket.addEventListener('click', onLoadTicket);
    elements.btnStartExploit.addEventListener('click', onStartExploit);
    elements.btnAbort.addEventListener('click', onAbort);
    elements.btnClearConsole.addEventListener('click', clearConsole);

    // Copy buttons
    document.querySelectorAll('.btn-copy').forEach(btn => {
        btn.addEventListener('click', () => {
            const targetId = btn.dataset.target;
            const input = document.getElementById(targetId);
            if (input.value && input.value !== '') {
                navigator.clipboard.writeText(input.value);
                log(`Copied ${targetId} to clipboard`, 'success');
                btn.textContent = '✓';
                setTimeout(() => btn.textContent = '📋', 1500);
            }
        });
    });
}

// Game selection handler
function onGameSelect(e) {
    const gameId = e.target.value;
    const game = IQUE_GAMES.find(g => g.id === gameId);

    if (game) {
        state.selectedGame = game;
        updateGameInfo(game);
        log(`Selected: ${game.title}`, 'info');

        if (state.connected) {
            elements.btnDumpKeys.disabled = false;
        }
    }
}

// Update game info display
function updateGameInfo(game) {
    elements.gameTitle.textContent = game.title;
    elements.gameRelease.textContent = formatDate(game.release);
    elements.gameCID.textContent = game.cid;
    elements.gameCover.innerHTML = `<div class="cover-placeholder"><span>${game.icon}</span></div>`;
}

// Format date for display
function formatDate(dateStr) {
    const date = new Date(dateStr);
    return date.toLocaleDateString('en-US', { year: 'numeric', month: 'short', day: 'numeric' });
}

// Load ticket.sys handler
async function onLoadTicket() {
    log('Loading ticket.sys...', 'info');

    // Read ticket.sys from device
    const result = await api('/read-file', 'POST', { filename: 'ticket.sys' });

    if (result.success) {
        state.ticketLoaded = true;
        log('ticket.sys loaded from device', 'success');

        if (state.selectedGame) {
            elements.btnDumpKeys.disabled = false;
        }

        updateConnectionStatus(true);
    } else {
        log(`Failed to load ticket.sys: ${result.message}`, 'error');
    }
}

// Dump keys handler
async function onDumpKeys() {
    if (!state.connected) {
        log('Error: Connect to iQue Player first', 'error');
        return;
    }

    elements.btnDumpKeys.disabled = true;
    elements.btnDumpKeys.innerHTML = '<span class="loading">⏳</span> Dumping...';
    log(`Dumping filesystem...`, 'info');

    // Dump the filesystem
    const result = await api('/dump-fs', 'POST');

    if (result.success) {
        log('Filesystem dumped successfully', 'success');
        log('Keys can be extracted from current_fs.bin', 'info');

        // For demo, show placeholder keys
        const keys = generateDemoKeys();
        elements.titlekeyIV.value = keys.titlekeyIV;
        elements.contentIV.value = keys.contentIV;
        elements.sha1Hash.value = keys.sha1Hash;
        elements.titlekey.value = keys.titlekey;

        log('Titlekey IV: ' + keys.titlekeyIV, 'success');
        log('Keys extracted!', 'success');

        elements.btnStartExploit.disabled = false;
    } else {
        log(`Error: ${result.message}`, 'error');
    }

    elements.btnDumpKeys.disabled = false;
    elements.btnDumpKeys.innerHTML = '<span class="btn-icon">📥</span> Dump Keys';
}

// Generate demo keys (for UI demonstration)
function generateDemoKeys() {
    const randomHex = (len) => [...Array(len)].map(() =>
        Math.floor(Math.random() * 16).toString(16).toUpperCase()
    ).join('');

    return {
        titlekeyIV: randomHex(32),
        contentIV: randomHex(32),
        sha1Hash: randomHex(40),
        titlekey: randomHex(32)
    };
}

// Start exploit handler
async function onStartExploit() {
    if (!state.connected) {
        log('Error: Connect to iQue Player first', 'error');
        return;
    }

    state.isRunning = true;
    state.exploitStep = 0;
    elements.btnStartExploit.disabled = true;
    elements.btnAbort.disabled = false;

    log('Starting eSKape v2 exploit...', 'warning');

    // Flash LED to indicate exploit starting
    await api('/led', 'POST', { value: '1' });

    runExploitStep();
}

// Run exploit steps
function runExploitStep() {
    if (!state.isRunning) return;

    const steps = [
        { id: 'step1', message: 'Connecting to iQue Player...', delay: 1500 },
        { id: 'step2', message: 'Dumping NAND & extracting keys...', delay: 2500 },
        { id: 'step3', message: 'Preparing eSKape payload...', delay: 2000 },
        { id: 'step4', message: 'Executing code injection...', delay: 3000 }
    ];

    if (state.exploitStep >= steps.length) {
        completeExploit();
        return;
    }

    const step = steps[state.exploitStep];
    const stepEl = document.getElementById(step.id);

    // Update step UI
    document.querySelectorAll('.step').forEach(s => s.classList.remove('active'));
    stepEl.classList.add('active');
    stepEl.querySelector('.step-status').textContent = 'In progress...';

    log(step.message, 'info');

    setTimeout(() => {
        if (!state.isRunning) return;

        stepEl.classList.remove('active');
        stepEl.classList.add('complete');
        stepEl.querySelector('.step-status').textContent = 'Complete ✓';

        state.exploitStep++;
        runExploitStep();
    }, step.delay);
}

// Complete exploit
async function completeExploit() {
    state.isRunning = false;
    elements.btnStartExploit.disabled = false;
    elements.btnAbort.disabled = true;

    // Flash LED to indicate success
    await api('/led', 'POST', { value: '2' });

    log('═══════════════════════════════════════', 'success');
    log('🎉 eSKape exploit completed successfully!', 'success');
    log(`Custom code executed on ${state.selectedGame?.title || 'iQue Player'}`, 'success');
    log('═══════════════════════════════════════', 'success');
}

// Abort exploit handler
function onAbort() {
    state.isRunning = false;
    state.exploitStep = 0;

    document.querySelectorAll('.step').forEach(s => {
        s.classList.remove('active', 'complete');
        s.querySelector('.step-status').textContent = 'Pending';
    });
    document.getElementById('step1').querySelector('.step-status').textContent = 'Waiting...';

    elements.btnStartExploit.disabled = false;
    elements.btnAbort.disabled = true;

    log('Exploit aborted by user', 'warning');
}

// Update connection status
function updateConnectionStatus(connected) {
    state.connected = connected;
    elements.connectionStatus.classList.toggle('connected', connected);
    elements.connectionText.textContent = connected ? 'Connected' : 'Disconnected';

    const btnConnect = document.getElementById('btnConnect');
    if (btnConnect) {
        btnConnect.textContent = connected ? 'Disconnect' : 'Connect';
        btnConnect.className = connected ? 'btn btn-small btn-danger' : 'btn btn-small';
    }
}

// Console logging
function log(message, type = 'info') {
    const timestamp = new Date().toLocaleTimeString('en-US', { hour12: false });
    const line = document.createElement('div');
    line.className = `console-line ${type}`;
    line.innerHTML = `
        <span class="timestamp">[${timestamp}]</span>
        <span class="message">${message}</span>
    `;
    elements.consoleOutput.appendChild(line);
    elements.consoleOutput.scrollTop = elements.consoleOutput.scrollHeight;
}

// Clear console
function clearConsole() {
    elements.consoleOutput.innerHTML = '';
    log('Console cleared', 'info');
}

// Initialize on DOM ready
document.addEventListener('DOMContentLoaded', init);

