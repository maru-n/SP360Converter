'use strict';

const electron = require('electron');
const app = electron.app;
const BrowserWindow = electron.BrowserWindow;

var mainWindow = null;

app.on('window-all-closed', function() {
    if (process.platform != 'darwin') {
        app.quit();
    }
});

app.on('activate', function() {
    if (mainWindow == null) {
        openWindow();
    }
});

app.on('ready', function() {
    openWindow();
});

function openWindow() {
    mainWindow = new BrowserWindow({
        'width': 800,
        'height': 650,
        'max-width': 800,
        'max-height': 650,
        'min-width': 800,
        'min-height': 650
    });
    mainWindow.loadURL('file://' + __dirname + '/index.html');
    mainWindow.on('closed', function() {
        mainWindow = null;
    });
};

app.commandLine.appendSwitch("--enable-experimental-web-platform-features");
