'use strict';

var remote = require('remote');
//var Converter = remote.require('./cpp/build/Release/converter');
var Converter = require('./cpp/build/Release/converter');
var Package = require('./package.json');

var Menu = remote.require('menu');
var template = [
    {
        label: 'SP360Converter',
        submenu: [{
            label: 'About SP360Converter',
            selector: 'orderFrontStandardAboutPanel:'
        },{
            type: 'separator'
        },{
            label: 'Quit',
            accelerator: 'Command+Q',
            selector: 'terminate:'
        }]
    }
];

var menu = Menu.buildFromTemplate(template);
Menu.setApplicationMenu(menu);

var SP360ConverterApp = angular.module('SP360Converter', ['ngElectron']);

SP360ConverterApp.directive('numberInput', function() {
    return {
        require: 'ngModel',
        link: function(scope, element, attrs, ngModel) {
            ngModel.$parsers.push(function(value) {
                return parseFloat(value);
            });
        }
    };
});

SP360ConverterApp.controller('MainController', ['$scope', '$q', '$timeout', 'electron',
function($scope, $q, $timeout, electron) {
    var converter = new Converter();
    var BrowserWindow = electron.browserWindow;
    var Dialog = electron.dialog;

    $scope.src_file     = "";
    $scope.dst_file     = "";
    $scope.start_time   = 0;
    $scope.end_time     = 1000;
    $scope.preview_time = 0;
    $scope.dst_width    = 1280;
    $scope.dst_height   = 720;
    $scope.angle_start  = 0;
    $scope.angle        = 360;
    $scope.radius_in    = 0.0;
    $scope.radius_out   = 1.0
    $scope.n_split_choices = [1, 2];
    $scope.n_split     = $scope.n_split_choices[0];
    $scope.resolutions = [
        {name:'VGA          640x480 ', width:640, height:480, aspect:"4:3" },
        {name:'SVGA         800x600 ', width:800, height:600, aspect:"4:3" },
        {name:'HD+         1600x900 ', width:1600, height:900, aspect:"16:9" },
        {name:'HD (720p)   1280x720 ', width:1280, height:720, aspect:"16:9" },
        {name:'FHD (1080p) 1920x1080', width:1920, height:1080, aspect:"16:9" },
        {name:'パノラマ     1200x400 ', width:1200, height:400, aspect:"3:1" },
    ];
    $scope.resolution = $scope.resolutions[0];

    $scope.convert_progress = 0.0;


    var originalPreviewWidth = 200;
    var originalPreviewWheight = 200;
    var originalPreviewCanvas = document.getElementById('original-preview-canvas');
    originalPreviewCanvas.width = originalPreviewWidth;
    originalPreviewCanvas.height = originalPreviewWheight;
    var originalPreviewContext = originalPreviewCanvas.getContext('2d');
    var originalPreviewData = originalPreviewContext.createImageData(originalPreviewWidth, originalPreviewWheight);

    var convertedPreviewWidth = 400;
    var convertedPreviewHeight = 225;
    var convertedPreviewCanvas = document.getElementById('converted-preview-canvas');
    convertedPreviewCanvas.width = convertedPreviewWidth;
    convertedPreviewCanvas.height = convertedPreviewHeight;
    var convertedPreviewContext = convertedPreviewCanvas.getContext('2d');
    var convertedPreviewData = convertedPreviewContext.createImageData(convertedPreviewWidth, convertedPreviewHeight);

    var updateConverter = function() {
        converter.setup({
            start_time:  $scope.start_time,
            end_time:    $scope.end_time,
            preview_time:$scope.preview_time,
            dst_width:   $scope.resolution.width,
            dst_height:  $scope.resolution.height,
            radius_in:   $scope.radius_in,
            radius_out:  $scope.radius_out,
            angle_start: $scope.angle_start * 2.0 * Math.PI / 360.0,
            angle_end:   ($scope.angle_start + $scope.angle) * 2.0 * Math.PI / 360.0,
            n_split:     $scope.n_split,
        });
    }

    $scope.updatePreview = function() {
        updateConverter();
        $timeout(function(){
            converter.makeOriginalPreviewImage(originalPreviewData.data, originalPreviewWidth, originalPreviewWheight, true);
            originalPreviewContext.putImageData(originalPreviewData, 0, 0);
        },0);
        $timeout(function(){
            converter.makeConvertedPreviewImage(convertedPreviewData.data, convertedPreviewWidth, convertedPreviewHeight);
            convertedPreviewContext.putImageData(convertedPreviewData, 0, 0);
        },0);
    }

    $scope.isConverting = function() {
        return (0.0 < $scope.convert_progress) && ($scope.convert_progress<1.0);
    }

    $scope.openFile = function() {
        var deferred = $q.defer()
        var options = {
            filters: [
                { name: 'Movies', extensions: ['mp4'] },
            ]};
        Dialog.showOpenDialog(BrowserWindow.getFocusedWindow(), options, function(filenames){
            if (filenames) {
                deferred.resolve(filenames);
            }
        });
        deferred.promise.then(function(filenames){
            $scope.src_file = filenames[0];
            converter.open(filenames[0]);
            updateConverter();
            $scope.updatePreview();
        });
    };

    $scope.convert = function() {
        if (!$scope.src_file) {
            return
        }
        var deferred = $q.defer()
        var options = {
            filters: [
                { name: 'Movies', extensions: ['mp4'] },
            ]};
        Dialog.showSaveDialog(BrowserWindow.getFocusedWindow(), options, function(filename){
            $scope.dst_file = filename;
            if (!$scope.src_file || !$scope.dst_file) {
                return;
            }
            updateConverter();
            converter.convert(filename,
                function(err, status, progress){
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
