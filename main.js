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
app.on('ready', function() {
    mainWindow = new BrowserWindow({
        'width': 750,
        'height': 600,
        'max-width': 750,
        'max-height': 600,
        'min-width': 750,
        'min-height': 600
    });
    mainWindow.loadURL('file://' + __dirname + '/index.html');
    mainWindow.on('closed', function() {
        mainWindow = null;
    });
});

app.commandLine.appendSwitch("--enable-experimental-web-platform-features");
