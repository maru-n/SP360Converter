'use strict';

var remote = require('remote');
//var Converter = remote.require('./module/build/Release/converter');
var Converter = require('./module/build/Release/converter');
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

SP360ConverterApp.directive('frame2sec', function() {
    return {
        require: 'ngModel',
        link: function(scope, element, attrs, ngModel) {
            ngModel.$parsers.push(function(value) {
                return Math.ceil(value * scope.fps);
            });
            ngModel.$formatters.push(function (value) {
               return value / scope.fps;
           });
        }
    };
});


SP360ConverterApp.controller('MainController', ['$scope', '$q', '$timeout', 'electron',
function($scope, $q, $timeout, electron) {
    var converter = new Converter();
    var BrowserWindow = electron.browserWindow;
    var Dialog = electron.dialog;

    $scope.fps = 30;

    $scope.src_file     = "";
    $scope.dst_file     = "";
    $scope.start_frame  = 0;
    $scope.end_frame    = 300;
    $scope.is_all_frame = false;
    $scope.dst_width    = 1280;
    $scope.dst_height   = 720;
    $scope.angle_start  = 0;
    $scope.angle        = 360;
    $scope.radius_start = 0.0;
    $scope.radius_end   = 1.0
    $scope.n_split_choices = [1, 2, 4];
    $scope.n_split     = $scope.n_split_choices[0];
    $scope.resolutions = [
        {name:'VGA          640x480 ', width:640, height:480, aspect:4/3 },
        {name:'SVGA         800x600 ', width:800, height:600, aspect:4/3 },
        {name:'HD+         1600x900 ', width:1600, height:900, aspect:16/9 },
        {name:'HD (720p)   1280x720 ', width:1280, height:720, aspect:16/9 },
        {name:'FHD (1080p) 1920x1080', width:1920, height:1080, aspect:16/9 },
        {name:'パノラマ     1440x428 ', width:1440, height:428, aspect:1440/428 }, //FOV of SP360 is 360x214 dgree.
    ];
    $scope.resolution = $scope.resolutions[0];

    $scope.convert_progress = 0.0;

    $scope.projection_types = [
        {name:"通常", value:"equirectangular"},
        {name:"歪み補正", value:"central"}
    ];
    $scope.projection_type = $scope.projection_types[0];

    $scope.fov = 80;
    $scope.center_angle = 0;
    $scope.center_radius = 0.6;


    var originalPreviewWidth = 225;
    var originalPreviewWheight = 225;
    var originalPreviewCanvas = document.getElementById('original-preview-canvas');
    originalPreviewCanvas.width = originalPreviewWidth;
    originalPreviewCanvas.height = originalPreviewWheight;
    var originalPreviewContext = originalPreviewCanvas.getContext('2d');

    var convertedPreviewWidth = 400;
    var convertedPreviewHeight = 225;
    var convertedPreviewAspect = convertedPreviewWidth/convertedPreviewHeight;
    var convertedPreviewCanvas = document.getElementById('converted-preview-canvas');
    convertedPreviewCanvas.width = convertedPreviewWidth;
    convertedPreviewCanvas.height = convertedPreviewHeight;
    var convertedPreviewContext = convertedPreviewCanvas.getContext('2d');

    var updateConverter = function() {
        var split_x = 1;
        var split_y = 1;
        if ($scope.n_split == 2) {
            split_y = 2;
        } else if ($scope.n_split == 4) {
            split_x = 2;
            split_y = 2;
        }
        converter.setup({
            projection_type: $scope.projection_type.value,
            start_frame:  $scope.start_frame,
            end_frame:    $scope.end_frame,
            dst_width:    $scope.resolution.width,
            dst_height:   $scope.resolution.height,
            radius_start: $scope.radius_start,
            radius_end:   $scope.radius_end,
            angle_start:  $scope.angle_start * 2.0 * Math.PI / 360.0,
            angle_end:    ($scope.angle_start + $scope.angle) * 2.0 * Math.PI / 360.0,
            split_x:      split_x,
            split_y:      split_y,
            center_angle: $scope.center_angle * 2.0 * Math.PI / 360,
            center_radius:$scope.center_radius,
            fov:          $scope.fov * Math.PI / 180,
            aspect:       $scope.resolution.aspect,
        });
    }

    $scope.changeAllTime = function() {
        if ($scope.is_all_frame && converter.isOpened()) {
            $scope.start_frame = 0;
            $scope.end_frame = converter.totalFrame();
        }
    }

    $scope.updatePreview = function() {
        if (!converter.isOpened()) { return }
        updateConverter();
        $timeout(function(){
            var originalPreviewData = originalPreviewContext.createImageData(originalPreviewWidth, originalPreviewWheight);
            converter.makeOriginalPreviewImage(originalPreviewData.data, originalPreviewWidth, originalPreviewWheight, true);
            originalPreviewContext.putImageData(originalPreviewData, 0, 0);
        },0);
        $timeout(function(){
            if ($scope.resolution.aspect < convertedPreviewAspect) {
                var h = convertedPreviewHeight;
                var w = h * $scope.resolution.aspect;
                var dx = (convertedPreviewWidth - w) / 2;
                var dy = 0;
            } else {
                var w = convertedPreviewWidth;
                var h = w / $scope.resolution.aspect;
                var dx = 0;
                var dy = (convertedPreviewHeight - h) / 2;
            }
            var convertedPreviewData = convertedPreviewContext.createImageData(w, h);
            converter.makeConvertedPreviewImage(convertedPreviewData.data, w, h);
            convertedPreviewContext.fillStyle = "rgb(0, 0, 0)";
            convertedPreviewContext.fillRect(0, 0, convertedPreviewWidth, convertedPreviewHeight);
            convertedPreviewContext.putImageData(convertedPreviewData, dx, dy);
        },0);
    }

    $scope.isConverting = function() {
        return (0.0 < $scope.convert_progress) && ($scope.convert_progress<1.0);
    }

    $scope.openFile = function() {
        var deferred = $q.defer()
        var options = {
            filters: [
                { name: 'Movies', extensions: ['mp4','mov'] },
            ]};
        Dialog.showOpenDialog(BrowserWindow.getFocusedWindow(), options, function(filenames){
            if (filenames) {
                deferred.resolve(filenames);
            }
        });
        deferred.promise.then(function(filenames){
            $scope.src_file = filenames[0];
            converter.open(filenames[0]);
            $scope.changeAllTime();
            $scope.updatePreview();
            $scope.fps = converter.fps();
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
