
module.exports = {
    scanDoc: function (successCallback, errorCallback, options) {
        options = options || {};
		options.sourceType = (options.sourceType == undefined) ? 1 : options.sourceType;
		options.fileName = (options.fileName == undefined) ? "image" : options.fileName;
		options.quality = (!isNaN(options.quality) && options.quality > 0) ? options.quality : 1;
    	if((options.sourceType === 1 || options.sourceType === 0) && typeof options.fileName === "string")
    	{
			var sourceType = options.sourceType;	// 0 Gallery, 1 Camera
			var fileName = options.fileName;	// "image" if not specified
			var quality = options.quality;	// Quality defaults to 1 (highest). If value > 1, a smaller image is returned to save memory. https://developer.android.com/reference/android/graphics/BitmapFactory.Options.html#inSampleSize
			var args = [sourceType, fileName, quality];
        	cordova.exec(successCallback, errorCallback, "Scan", "scanDoc", args);
    	}
    	else
    	{
    		alert("Incorrect options/argument values specified by the plugin user!");
    	}
    }
};