
var argscheck = require('cordova/argscheck'),
	exec = require("cordova/exec");

module.exports = {
    scanDoc: function (successCallback, errorCallback, options) {
		function toInt(value) {
			if (typeof value === "string") {
				return parseInt(value);
			}
			return value;
		}
		function toFloat(value) {
			if (typeof value === "string") {
				return parseFloat(value);
			}
			return value;
		}
		argscheck.checkArgs('fFO', 'scanDoc', arguments);
        options = options || {};
		options.sourceType = (options.sourceType !== 1) ? 0 : options.sourceType;
		options.fileName = (typeof options.fileName === "string") ? options.fileName : "image";
		options.quality = (!isNaN(options.quality) && options.quality >= 1 && options.quality <= 5) ? options.quality : 1;
		options.returnBase64 = (typeof options.returnBase64 === "boolean") ? options.returnBase64 : false;

    	if(options.sourceType === 1 || options.sourceType === 0)
    	{
			var sourceType = options.sourceType;	// 0 Gallery, 1 Camera
			var fileName = options.fileName;	// "image" if not specified
			var quality = options.quality;	// Quality defaults to 1 (highest). If value > 1, a smaller image is returned to save memory. https://developer.android.com/reference/android/graphics/BitmapFactory.Options.html#inSampleSize
			var returnBase64 = options.returnBase64;	// return base64 output if set to true. Defaults to false.

            var convertToGrayscale = !!options.convertToGrayscale; // returns grayscale image if true
            var dontClip = !!options.dontClip; // do not detect document edge if true
            var maxResolution = toInt(options.maxResolution); // maximum pixels size of image
            var autoShutter = toInt(options.autoShutter); // take photo automatically after timeout
            var rotationDegree = toInt(options.rotationDegree); // rotate video preview
            var brightnessValue = toInt(options.brightnessValue); // manual brightness
            var focusValue = toInt(options.focusValue); // manual focus
            var contrastValue = toFloat(options.contrastValue); // manual contrast

			var args = [sourceType, fileName, quality, returnBase64, convertToGrayscale, dontClip, maxResolution, autoShutter, rotationDegree, brightnessValue, focusValue, contrastValue];

        	exec(successCallback, errorCallback, "Scan", "scanDoc", args);
    	}
    	else
    	{
    		alert("Incorrect options/argument values specified by the plugin user!");
    	}
    }
};
