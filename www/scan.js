/*global cordova, module*/

module.exports = {
    scanDoc: function (sourceType, successCallback, errorCallback) {
        cordova.exec(successCallback, errorCallback, "Scan", "scanDoc", [sourceType]);
    }
};