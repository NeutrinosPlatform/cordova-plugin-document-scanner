
module.exports = {
    scanDoc: function (successCallback, errorCallback, options) {
        options = options || {};
		options.sourceType = (options.sourceType == undefined) ? 1 : options.sourceType;
		options.fileName = (options.fileName == undefined) ? "image" : options.fileName;
    	if((options.sourceType === 1 || options.sourceType === 0) && typeof options.fileName === "string")
    	{
			var sourceType = options.sourceType;	// 0 Gallery, 1 Camera
			var fileName = options.fileName;	// "image" if not specified
			var args = [sourceType, fileName];
        	cordova.exec(successCallback, errorCallback, "Scan", "scanDoc", args);
    	}
    	else
    	{
    		alert("Incorrect options/argument values specified by the plugin user!");
    	}
    }
};