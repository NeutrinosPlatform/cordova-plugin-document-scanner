
module.exports = {
    scanDoc: function (successCallback, errorCallback, options) {
        options = options || {};
		options.sourceType = (options.sourceType == undefined) ? 1 : options.sourceType;
    	if(options.sourceType === 1 || options.sourceType === 0)
    	{
			var sourceType = options.sourceType;	// 0 Gallery, 1 Camera
			var args = [sourceType];
        	cordova.exec(successCallback, errorCallback, "Scan", "scanDoc", args);
    	}
    	else
    	{
    		alert("sourceType value incorrect!");
    	}
    }
};