var remote = require('remote');
var converter = remote.require('./build/Release/converter');
var app = remote.require('app');
var BrowserWindow = remote.require('browser-window');
var dialog = remote.require('dialog');

var src_file = "";
var dst_file = "";

function openFile() {
    var options = {};
    dialog.showOpenDialog(BrowserWindow.getFocusedWindow(), options, function(filenames){
        src_file = filenames[0];
    });
}

function convert() {
    if (!src_file) {
        return
    }
    var options = {};
    dialog.showSaveDialog(BrowserWindow.getFocusedWindow(), options, function(filename){
        dst_file = filename;
        converter.convert({
            src_file: src_file,
            dst_file: dst_file,
            start_time:  0,
            end_time:    1000,
            dst_width:   1280,
            dst_height:  720,
            n_split:     2,
            start_theta: 0.0 * Math.PI,
            end_theta:   2.0 * Math.PI
        }, function(err, status, progress){
            console.log(err);
            console.log(status);
            console.log(progress);
        });
    });
}
