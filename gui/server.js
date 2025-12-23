/**
 * iQue Tools Bridge Server
 * 
 * This Node.js server runs natively on Windows and:
 * 1. Serves the web GUI on http://localhost:8080
 * 2. Provides a REST API that proxies commands to aulon running in the XP VM
 * 
 * The aulon TCP server runs in the XP VM on port 5001 (forwarded via VirtualBox)
 */

const express = require('express');
const cors = require('cors');
const net = require('net');
const path = require('path');
const fs = require('fs');

const app = express();
const GUI_PORT = 8080;
const AULON_HOST = '127.0.0.1'; // Connects to Host Loopback -> NAT -> XP VMirtualBox port forwarding
const AULON_PORT = 5001;

// Middleware
app.use(cors());
app.use(express.json());
app.use(express.static(__dirname));

// Send command to aulon server and get response
function sendToAulon(command) {
    return new Promise((resolve, reject) => {
        const client = new net.Socket();
        let response = '';

        client.setTimeout(10000);

        client.connect(AULON_PORT, AULON_HOST, () => {
            console.log(`[Bridge] Sending to aulon: ${JSON.stringify(command)}`);
            client.write(JSON.stringify(command));
        });

        client.on('data', (data) => {
            response += data.toString();
            // Check if we have complete JSON
            try {
                const parsed = JSON.parse(response);
                client.destroy();
                resolve(parsed);
            } catch (e) {
                // Not complete yet, wait for more data
            }
        });

        client.on('close', () => {
            if (response) {
                try {
                    resolve(JSON.parse(response));
                } catch (e) {
                    reject(new Error('Invalid response from aulon'));
                }
            }
        });

        client.on('error', (err) => {
            console.error(`[Bridge] Connection error: ${err.message}`);
            reject(err);
        });

        client.on('timeout', () => {
            client.destroy();
            reject(new Error('Connection timeout'));
        });
    });
}

// API Routes

// Check bridge server status
app.get('/api/status', (req, res) => {
    res.json({
        success: true,
        message: 'Bridge server running',
        aulon_host: AULON_HOST,
        aulon_port: AULON_PORT
    });
});

// Ping aulon server
app.get('/api/ping', async (req, res) => {
    try {
        const result = await sendToAulon({ cmd: 'ping' });
        res.json(result);
    } catch (err) {
        res.json({
            success: false,
            message: `Cannot reach aulon server: ${err.message}. Is the XP VM running?`
        });
    }
});

// Get connection status
app.get('/api/connection', async (req, res) => {
    try {
        const result = await sendToAulon({ cmd: 'status' });
        res.json(result);
    } catch (err) {
        res.json({ success: false, message: err.message, data: { connected: false } });
    }
});

// Connect to iQue Player
app.post('/api/connect', async (req, res) => {
    try {
        const result = await sendToAulon({ cmd: 'init' });
        res.json(result);
    } catch (err) {
        res.json({ success: false, message: err.message });
    }
});

// Disconnect from iQue Player
app.post('/api/disconnect', async (req, res) => {
    try {
        const result = await sendToAulon({ cmd: 'close' });
        res.json(result);
    } catch (err) {
        res.json({ success: false, message: err.message });
    }
});

// Get BBID
app.get('/api/bbid', async (req, res) => {
    try {
        const result = await sendToAulon({ cmd: 'get_bbid' });
        res.json(result);
    } catch (err) {
        res.json({ success: false, message: err.message });
    }
});

// List files
app.get('/api/files', async (req, res) => {
    try {
        const result = await sendToAulon({ cmd: 'list_files' });
        res.json(result);
    } catch (err) {
        res.json({ success: false, message: err.message });
    }
});

// Dump NAND
app.post('/api/dump-nand', async (req, res) => {
    try {
        const result = await sendToAulon({ cmd: 'dump_nand' });
        res.json(result);
    } catch (err) {
        res.json({ success: false, message: err.message });
    }
});

// Dump filesystem
app.post('/api/dump-fs', async (req, res) => {
    try {
        const result = await sendToAulon({ cmd: 'dump_fs' });
        res.json(result);
    } catch (err) {
        res.json({ success: false, message: err.message });
    }
});

// Read file from iQue
app.post('/api/read-file', async (req, res) => {
    try {
        const { filename } = req.body;
        if (!filename) {
            return res.json({ success: false, message: 'Filename required' });
        }
        const result = await sendToAulon({ cmd: 'read_file', filename });
        res.json(result);
    } catch (err) {
        res.json({ success: false, message: err.message });
    }
});

// Set LED
app.post('/api/led', async (req, res) => {
    try {
        const { value } = req.body;
        const result = await sendToAulon({ cmd: 'set_led', value: value || '1' });
        res.json(result);
    } catch (err) {
        res.json({ success: false, message: err.message });
    }
});

// Serve index.html for root
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'index.html'));
});

// Start server
app.listen(GUI_PORT, () => {
    console.log('╔════════════════════════════════════════════════════════════╗');
    console.log('║             iQue Tools Bridge Server                       ║');
    console.log('╠════════════════════════════════════════════════════════════╣');
    console.log(`║  Web GUI:     http://localhost:${GUI_PORT}                        ║`);
    console.log(`║  Aulon:       ${AULON_HOST}:${AULON_PORT}                           ║`);
    console.log('╠════════════════════════════════════════════════════════════╣');
    console.log('║  Make sure the Windows XP VM is running with:              ║');
    console.log('║    aulon.exe -s 5001                                       ║');
    console.log('╚════════════════════════════════════════════════════════════╝');
    console.log('');
    console.log('Press Ctrl+C to stop the server.');
});
