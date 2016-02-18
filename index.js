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
                start_time:  0,
                end_time:    1000,
                dst_width:   1280,
                dst_height:  720,
                n_split:     2,
                start_theta: 0.0 * Math.PI,
                end_theta:   2.0 * Math.PI
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
            $scope.converter_progress = 0.0;
        }, function(error){
            console.error(error);
        }, function(progress){
            $scope.convert_progress = progress;
        });
    }
}]);
