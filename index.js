'use strict';

var remote = require('remote');
//var converter = remote.require('./build/Release/converter');
var converter = require('./build/Release/converter');

var SP360Converter = angular.module('SP360Converter', ['ngElectron']);

SP360Converter.controller('MainController', ['$scope', '$q', 'electron',
function($scope, $q, electron) {
    var BrowserWindow = electron.browserWindow;
    var Dialog = electron.dialog;

    $scope.src_file = "";
    $scope.dst_file = "";
    $scope.start_time  = 0;
    $scope.end_time    = 1000;
    $scope.dst_width   = 1280;
    $scope.dst_height  = 720;
    $scope.angle_start = 0;
    $scope.angle_end   = 360;
    $scope.radius_in   = 0.0;
    $scope.radius_out  = 1.0
    $scope.n_split_choice = [1, 2, 3];
    $scope.n_split     = $scope.n_split_choice[0];
    $scope.resolutions = [
        {name:'VGA          640x480 ', width:640, height:480 },
        {name:'SVGA         800x600 ', width:800, height:600 },
        {name:'HD+         1600x900 ', width:1600, height:900 },
        {name:'HD (720p)   1280x720 ', width:1280, height:720 },
        {name:'FHD (1080p) 1920x1080', width:1920, height:1080 },
        {name:'パノラマ     1200x400 ', width:1200, height:400 },
    ];
    $scope.resolution = $scope.resolutions[0];

    $scope.convert_progress = 0.0;


    var previewImageCanvas = document.getElementById('original-preview-canvas');

    var previewImageWidth = 600;
    var previewImageHeight = 600;

    previewImageCanvas.width = previewImageWidth;
    previewImageCanvas.height = previewImageHeight;
    var previewImageContext = previewImageCanvas.getContext('2d');
    var previewImageData = previewImageContext.createImageData(previewImageWidth, previewImageHeight);

    var updateConverter = function() {
        converter.setup({
            src_file:    $scope.src_file,
            dst_file:    $scope.dst_file,
            start_time:  $scope.start_time,
            end_time:    $scope.end_time,
            dst_width:   $scope.resolution.width,
            dst_height:  $scope.resolution.height,
            radius_in:   $scope.radius_in,
            radius_out:  $scope.radius_out,
            angle_start: $scope.angle_start * 2.0 * Math.PI / 360.0,
            angle_end:   $scope.angle_end * 2.0 * Math.PI / 360.0,
            n_split:     $scope.n_split,
        });
    }

    $scope.updatePreview = function() {
        converter.makeImage(previewImageData.data, previewImageWidth, previewImageHeight);
        previewImageContext.putImageData(previewImageData, 0, 0);
    }

    $scope.isConverting = function() {
        return (0.0 < $scope.convert_progress) && ($scope.convert_progress<1.0);
    }

    $scope.openFile = function() {
        var deferred = $q.defer()
        var options = {};
        Dialog.showOpenDialog(BrowserWindow.getFocusedWindow(), options, function(filenames){
            if (filenames) {
                deferred.resolve(filenames);
            }
        });
        deferred.promise.then(function(filenames){
            $scope.src_file = filenames[0];
            updateConverter();
            $scope.updatePreview();
        });
    };

    $scope.convert = function() {
        if (!$scope.src_file) {
            return
        }
        var deferred = $q.defer()
        var options = {};
        Dialog.showSaveDialog(BrowserWindow.getFocusedWindow(), options, function(filename){
            $scope.dst_file = filename;
            if (!$scope.src_file || !$scope.dst_file) {
                return;
            }
            updateConverter();
            converter.convert(function(err, status, progress){
                if (err) {
                    deferred.reject(err);
                } else if (status == "progress") {
                    deferred.notify(progress);
                } else if (status == "successed") {
                    deferred.resolve();
                } else {
                    deferred.reject("invalid response.");
                }
            });
        });
        deferred.promise.then(function(){
            $scope.convert_progress = 1.0;
            $scope.dst_file = "";
        }, function(error){
            $scope.dst_file = "";
            console.error(error);
        }, function(progress){
            $scope.convert_progress = progress;
        });
    }
}]);
