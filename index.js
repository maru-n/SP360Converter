'use strict';

var remote = require('remote');
var converter = remote.require('./build/Release/converter');

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
    $scope.n_split     = 2;
    $scope.start_angle = 0;
    $scope.end_angle   = 360;


    $scope.convert_progress = 0.0;

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
            converter.convert({
                src_file:    $scope.src_file,
                dst_file:    $scope.dst_file,
                start_time:  $scope.start_time,
                end_time:    $scope.end_time,
                dst_width:   $scope.dst_width,
                dst_height:  $scope.dst_height,
                n_split:     $scope.n_split,
                start_angle: $scope.start_angle * 2.0 * Math.PI / 360.0,
                end_angle:   $scope.end_angle * 2.0 * Math.PI / 360.0,
            }, function(err, status, progress){
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
            $scope.convert_progress = 0.0;
        }, function(error){
            console.error(error);
        }, function(progress){
            $scope.convert_progress = progress;
        });
    }
}]);
