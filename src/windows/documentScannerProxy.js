/*
 * Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
 */

var urlutil = require('cordova/urlutil');

var CAMERA_STREAM_STATE_CHECK_RETRY_TIMEOUT = 200; // milliseconds
var OPERATION_IS_IN_PROGRESS = -2147024567;
var REGDB_E_CLASSNOTREG = -2147221164;
var INITIAL_FOCUS_DELAY = 200; // milliseconds
var CLICK_FOCUS_DELAY = 50; // milliseconds
var CHECK_PLAYING_TIMEOUT = 100; // milliseconds

var CHECK_FRAME_ARRIVED_TIMEOUT = 2000; // milliseconds

// Default aspect ratio 1.78 (16:9 hd video standard)
var DEFAULT_ASPECT_RATIO = '1.8';

// Highest possible z-index supported across browsers. Anything used above is converted to this value.
var HIGHEST_POSSIBLE_Z_INDEX = 2147483647;

var PROPERTY_QUALITY  = "{698649BE-8EAE-4551-A4CB-2FA79A4E1E70}";
var PROPERTY_GRAYSCALE= "{698649BE-8EAE-4551-A4CB-2FA79A4E1E71}";
var PROPERTY_ROTATION = "{698649BE-8EAE-4551-A4CB-2FA79A4E1E72}";
var PROPERTY_CAPTURE  = "{698649BE-8EAE-4551-A4CB-2FA79A4E1E79}";
var PROPERTY_RESULT   = "{698649BE-8EAE-4551-A4CB-2FA79A4E1E80}";

var analyticsVersionInfo = Windows.System.Profile.AnalyticsInfo && Windows.System.Profile.AnalyticsInfo.versionInfo;
var getVersion = function() {
    return analyticsVersionInfo && analyticsVersionInfo.deviceFamilyVersion;
}
var getMajorVersion = function() {
	var majorVersion = 0;
	if (analyticsVersionInfo && analyticsVersionInfo.deviceFamilyVersion) {
		majorVersion = Math.floor(parseInt(analyticsVersionInfo.deviceFamilyVersion) / 0x1000000000000);
	}
	return majorVersion;
}
var getAppData = function () {
    return Windows.Storage.ApplicationData.current;
};
var encodeToBase64String = function (buffer) {
    return Windows.Security.Cryptography.CryptographicBuffer.encodeToBase64String(buffer);
};
var OptUnique = Windows.Storage.CreationCollisionOption.generateUniqueName;
var fileIO = Windows.Storage.FileIO;

/**
 * Detects the first appropriate camera located at the back panel of device. If
 *   there is no back cameras, returns the first available.
 *
 * @returns {Promise<String>} Camera id
 */
function findCamera() {
    var Devices = Windows.Devices.Enumeration;

    // Enumerate cameras and add them to the list
    return Devices.DeviceInformation.findAllAsync(Devices.DeviceClass.videoCapture)
    .then(function (cameras) {

        if (!cameras || cameras.length === 0) {
            throw new Error("No cameras found");
        }

        var backCameras = cameras.filter(function (camera) {
            return camera.enclosureLocation && camera.enclosureLocation.panel === Devices.Panel.back;
        });

        // If there is back cameras, return the id of the first,
        // otherwise take the first available device's id
        return (backCameras[0] || cameras[0]).id;
    });
}

/**
 * @param {Windows.Graphics.Display.DisplayOrientations} displayOrientation
 * @return {Number}
 */
function videoPreviewRotationLookup(displayOrientation, isMirrored) {
    var degreesToRotate;

    switch (displayOrientation) {
        case Windows.Graphics.Display.DisplayOrientations.landscape:
            degreesToRotate = 0;
            break;
        case Windows.Graphics.Display.DisplayOrientations.portrait:
            if (isMirrored) {
                degreesToRotate = 270;
            } else {
                degreesToRotate = 90;
            }
            break;
        case Windows.Graphics.Display.DisplayOrientations.landscapeFlipped:
            degreesToRotate = 180;
            break;
        case Windows.Graphics.Display.DisplayOrientations.portraitFlipped:
            if (isMirrored) {
                degreesToRotate = 90;
            } else {
                degreesToRotate = 270;
            }
            break;
        default:
            degreesToRotate = 0;
            break;
    }

    return degreesToRotate;
}

    /**
     * Converts SimpleOrientation to a VideoRotation to remove difference between camera sensor orientation
     * and video orientation
     * @param  {number} orientation - Windows.Devices.Sensors.SimpleOrientation
     * @return {number} - Windows.Media.Capture.VideoRotation
     */
    function orientationToRotation(orientation) {
        // VideoRotation enumerable and BitmapRotation enumerable have the same values
        // https://msdn.microsoft.com/en-us/library/windows/apps/windows.media.capture.videorotation.aspx
        // https://msdn.microsoft.com/en-us/library/windows/apps/windows.graphics.imaging.bitmaprotation.aspx

        switch (orientation) {
            // portrait
        case Windows.Devices.Sensors.SimpleOrientation.notRotated:
            return Windows.Media.Capture.VideoRotation.clockwise90Degrees;
        // landscape
        case Windows.Devices.Sensors.SimpleOrientation.rotated90DegreesCounterclockwise:
            return Windows.Media.Capture.VideoRotation.none;
        // portrait-flipped (not supported by WinPhone Apps)
        case Windows.Devices.Sensors.SimpleOrientation.rotated180DegreesCounterclockwise:
            // Falling back to portrait default
            return Windows.Media.Capture.VideoRotation.clockwise90Degrees;
        // landscape-flipped
        case Windows.Devices.Sensors.SimpleOrientation.rotated270DegreesCounterclockwise:
            return Windows.Media.Capture.VideoRotation.clockwise180Degrees;
        // faceup & facedown
        default:
            // Falling back to portrait default
            return Windows.Media.Capture.VideoRotation.clockwise90Degrees;
        }
    }


/**
 * The pure JS implementation of MediaFrameReader
 *   For backward compatibility.
 *
 * @class {MediaFrameReader}
 */
function MediaFrameReader (newCapture, newWidth, newHeight) {
    this.isPureJS = true;
    var capture = newCapture;
    var width = newWidth;
    var height = newHeight;
    var frame = null;
    var running = false;

    var element = document.createElement("div");
    var addEventListener = function(type, listener, options) {
        return element.addEventListener(type, listener, options);
    };
    this.addEventListener = addEventListener;

    var removeEventListener = function(type, listener, options) {
        return element.removeEventListener(type, listener, options);
    };
    this.removeEventListener = removeEventListener;

/**
 * 	Disposes of the object and associated resources.
 *
 */
    var close = function () {
        running = false;
        frame = null;
        capture = null;
    };
    this.close = close;

    /**
     * Grabs a frame from preview stream uning Win10-only API
     */
    var getPreviewFrameAsync = function () {
        if (!running || !capture) {
            return WinJS.Promise.as();
        }
        // Shortcuts for namespaces
        var Imaging = Windows.Graphics.Imaging;

        var newFrame = new Windows.Media.VideoFrame(Imaging.BitmapPixelFormat.bgra8, width, height);
        return capture.getPreviewFrameAsync(newFrame).then(function(capturedFrame) {
            frame = {
                videoMediaFrame: capturedFrame
            };
            element.dispatchEvent(new Event('framearrived'));
            // give some delay to enable UI interaction
            return WinJS.Promise.timeout(50).then(function() {
                getPreviewFrameAsync();
            });
        });
    }

/**
 * Asynchronously starts the reading of frames from a MediaFrameSource.
 *
 */
    var startAsync = function () {
        running = true;
        getPreviewFrameAsync();
    };
    this.startAsync = startAsync;

/**
 * Asynchronously stops the reading of frames from a MediaFrameSource.
 *
 */
    var stopAsync = function () {
        running = false;
    };
    this.stopAsync = stopAsync;
/**
 * Attempts to obtain a MediaFrameReference object representing the latest frame from the MediaFrameSource.
 *
 */
    var tryAcquireLatestFrame = function () {
        return frame;
    };
    this.tryAcquireLatestFrame = tryAcquireLatestFrame;

}





/**
 * The implementation of camera user interface
 *
 * @class {CameraUI}
 */
function CameraUI () {
    this._promise = null;
    this._cancelled = false;
    this._captured = null;
    this._props = new Windows.Foundation.Collections.PropertySet();
    this._props.insert(PROPERTY_QUALITY, 50);
    this._props.insert(PROPERTY_GRAYSCALE, false);
    this._props.insert(PROPERTY_ROTATION, 0);
    this._props.insert(PROPERTY_CAPTURE, false);
    this._props.insert(PROPERTY_RESULT, "");

    this._reader = null;
    this._onFrameArrivedBound = null;
    this._helper = null;
    this._hasFrameArrived = false;
    this._captureLaterPromise = null;
}

CameraUI.prototype.hasFrameArrived = function() {
    return this._hasFrameArrived;
}

/**
 * Returns an instance of CameraUI
 *
 * @static
 * @constructs {CameraUI}
 *
 * @param   {MediaCapture}   mediaCaptureInstance  Instance of
 *   Windows.Media.Capture.MediaCapture class
 *
 * @return  {CameraUI}  CameraUI instance that could be used for
 *   capturing photos
 */
CameraUI.get = function () {
    return new CameraUI();
};

/**
 * Initializes instance of camera.
 *
 * @param   {MediaCapture}  capture  Instance of
 *   Windows.Media.Capture.MediaCapture class, used for acquiring images/ video stream
 *
 * @param   {Number}  width    Video/image frame width
 * @param   {Number}  height   Video/image frame height
 */
CameraUI.prototype.init = function (capture, videoWidth, videoHeight, preview, canvas, fail) {
    this._capture = capture;
    this._videoWidth = videoWidth;
    this._videoHeight = videoHeight;
    this._preview = preview;
    this._canvas = canvas;
    this._fail = fail;
    this._points = null;
    this._hasFrameArrived = false;
    this._captureLaterPromise = null;
    this._inCapturePhoto = false;
};

CameraUI.prototype.attach = function(reader) {
    this._reader = reader;
    if (reader) {
        this._helper = new OpenCVBridge.OpenCVHelper();
        reader.acquisitionMode = Windows.Media.Capture.Frames &&
            Windows.Media.Capture.Frames.MediaFrameReaderAcquisitionMode &&
            Windows.Media.Capture.Frames.MediaFrameReaderAcquisitionMode.realtime;
        this._onFrameArrivedBound = this.onFrameArrived.bind(this);
        reader.addEventListener("framearrived", this._onFrameArrivedBound);
        return reader.startAsync();
    } else {
        return WinJS.Promise.as();
    }
}


CameraUI.prototype.createReader = function(bAvoidNativeReader) {
    var capture = this._capture;
    if (capture) {
        var bitmapSize = {
            width: this._videoWidth,
            height: this._videoHeight
        };
        var frameSource = !bAvoidNativeReader && capture.frameSources &&
            capture.frameSources.first() &&
            capture.frameSources.first().current &&
            capture.frameSources.first().current.value;
        if (capture.createFrameReaderAsync && frameSource) {
            console.log('create MediaFrameReader object by createFrameReaderAsync()');
            return capture.createFrameReaderAsync(frameSource, Windows.Media.MediaProperties.MediaEncodingSubtypes.bgra8, bitmapSize);
        } else {
            console.log('create own MediaFrameReader object');
            return new WinJS.Promise.as().then(function() {
                return new MediaFrameReader(capture, bitmapSize.width, bitmapSize.height);
            });
        }
    } else {
        return WinJS.Promise.as();
    }
}

CameraUI.prototype.release = function(eventArgs) {
    if (this._reader) {
        if (this._onFrameArrivedBound) {
            this._reader.removeEventListener("framearrived", this._onFrameArrivedBound);
            this._onFrameArrivedBound = null;
        }
        var reader = this._reader;
        this._reader = null;
        this._helper = null;
        return reader.stopAsync();
    } else {
        return WinJS.Promise.as();
    }
};

CameraUI.prototype.onFrameArrived = function() {
    //console.log('onFrameArrived called...');
    this._hasFrameArrived = true;
    if (this._reader && this._helper && this._canvas && !this._inCapturePhoto) {
        var result = null;
        var reader = this._reader;
        if (reader.isPureJS) {
            var frame = this._reader.tryAcquireLatestFrame();
            if (frame) {
                var inputBitmap = frame.videoMediaFrame && frame.videoMediaFrame.softwareBitmap;
                if (inputBitmap) {
                    var originalBitmap = Windows.Graphics.Imaging.SoftwareBitmap.convert(inputBitmap,
                        Windows.Graphics.Imaging.BitmapPixelFormat.bgra8,
                        Windows.Graphics.Imaging.BitmapAlphaMode.premultiplied);
                    try {
                        result = this._helper.getPoints(originalBitmap);
                    } catch (e) {
                    }
                    //console.log('onFrameArrived: result=' + result);
                }
            }
        } else {
            try {
                result = this._helper.getPointsFromMediaFrameReader(reader);
            } catch (e) {
            }
        }
        // draws the result polygon over the video preview
        var c2 = this._canvas.getContext('2d');
        c2.clearRect(0, 0, this._canvas.width, this._canvas.height);
        if (result && result.length === 8 && this._videoWidth > 0 && this._videoHeight) {
            var i;
            var viewResult = [];
            switch (this._canvas.rotDegree) {
                case 90:
                for (i = 0; i < 4; i++) {
                    viewResult.push((1 - result[i * 2 + 1] / this._videoHeight) * this._canvas.width);
                    viewResult.push(result[i * 2]  / this._videoWidth * this._canvas.height);
                };
                break;
                case 180:
                for (i = 0; i < 4; i++) {
                    viewResult.push((1 - result[i * 2] / this._videoWidth) * this._canvas.width);
                    viewResult.push((1 - result[i * 2 + 1] / this._videoHeight) * this._canvas.height);
                };
                break;
                case 270:
                for (i = 0; i < 4; i++) {
                    viewResult.push(result[i * 2 + 1] / this._videoHeight * this._canvas.width);
                    viewResult.push((1 - result[i * 2]  / this._videoWidth ) * this._canvas.height);
                };
                break;
                default:
                for (i = 0; i < 4; i++) {
                    viewResult.push(result[i * 2] / this._videoWidth * this._canvas.width);
                    viewResult.push(result[i * 2 + 1] / this._videoHeight * this._canvas.height);
                };
            }
            c2.fillStyle = "rgba(255,0,0,0.25)";
            c2.strokeStyle = "#800";
            c2.beginPath();
            c2.moveTo(viewResult[0], viewResult[1]);
            c2.lineTo(viewResult[2], viewResult[3]);
            c2.lineTo(viewResult[4], viewResult[5]);
            c2.lineTo(viewResult[6], viewResult[7]);
            c2.closePath();
            c2.fill();
            c2.stroke();
        }
        this._points = result;
    }
};

/**
 * Starts capture asyncronously.
 *
 * @return  {Promise<CaptureResult>}  document capture result or null if search
 *   cancelled.
 */
CameraUI.prototype.capturing = function (usePropertySet) {

    /**
     * Checks for result from media filter. If there is no result
     *   found, returns null.
     */
    function checkForResult(capture, propertySet, frameReader, captured, fail) {
        return WinJS.Promise.timeout(200).then(function () {
            return new WinJS.Promise(function (complete) {
				if (usePropertySet) {
                    var value = null;
                    if (propertySet && captured) {
                        try {
                            value = propertySet.lookup(PROPERTY_RESULT);
                            if (value && !value.length) {
                                value = null;
                            }
                        } catch (e) {
                            console.error('propertySet.lookup failed: ' + e);
                        }
                    }
                    complete(value);
                    return WinJS.Promise.as();
                } else if (captured && typeof captured === "string") {
                    complete(captured);
                } else {
                    complete(null);
                }
            });
        });
    }

    var that = this;
    return checkForResult(this._capture, this._props, this._reader, this._captured, this._fail).then(function (result) {
        if (that._cancelled)
            return null;

        return result || (that._promise = that.capturing(usePropertySet));
    });
};

CameraUI.prototype.capturePhoto = function (bDontClip, useEffectFilter, quality, fileName, returnBase64, convertToGrayscale) {
    if (this._captureLaterPromise) {
        this._captureLaterPromise.cancel();
    }
    if (useEffectFilter) {
		this._captured = true;
    } else if (this._capture && this._helper) {
        var that = this;
        if (!bDontClip && !(this._points && this._points.length === 8)) {
            this._captureLaterPromise = WinJS.Promise.timeout(CHECK_PLAYING_TIMEOUT).then(function() {
                return that.capturePhoto(bDontClip, useEffectFilter, quality, fileName, returnBase64, convertToGrayscale);
            });
            return this._captureLaterPromise;
        }
        this._inCapturePhoto = true;
        this._captureStarted = false;
        var width = this._videoWidth;
        var height = this._videoHeight;
        var points = this._points;
        var helper = this._helper;
        var capture = this._capture;
        var fail = this._fail;
        var preview = this._preview;
        if (preview) {
            var isPlaying = !preview.paused && !preview.ended && preview.readyState > 2;
            if (isPlaying) {
                preview.pause();
            }
        }
        this.release();
        var tempFolder = getAppData().temporaryFolder;
        var tempCapturedFile = null;
        return new WinJS.Promise(function (complete) {
            var decoder = null;
            var outputBitmap = null;
            var encodingProperties = Windows.Media.MediaProperties.ImageEncodingProperties.createJpeg();
            var photoStream = new Windows.Storage.Streams.InMemoryRandomAccessStream();
            var finalStream = new Windows.Storage.Streams.InMemoryRandomAccessStream();
            return capture.capturePhotoToStreamAsync(encodingProperties, photoStream).then(function () {
                return Windows.Graphics.Imaging.BitmapDecoder.createAsync(photoStream);
            }).then(function (dec) {
                decoder = dec;
                if (bDontClip) {
                    return WinJS.Promise.as();
                } else {
                    return dec.getSoftwareBitmapAsync();
                }
            }).then(function (originalBitmap) {
                finalStream.size = 0; // BitmapEncoder requires the output stream to be empty
                if (!bDontClip && originalBitmap) {
                    var xScale = originalBitmap.pixelWidth / width;
                    var yScale = originalBitmap.pixelHeight / height;
                    var p0 = {
                        x: Math.floor(points[0] * xScale + 0.5),
                        y: Math.floor(points[1] * yScale + 0.5)
                    };
                    var p1 = {
                        x: Math.floor(points[2] * xScale + 0.5),
                        y: Math.floor(points[3] * yScale + 0.5)
                    };
                    var p2 = {
                        x: Math.floor(points[4] * xScale + 0.5),
                        y: Math.floor(points[5] * yScale + 0.5)
                    };
                    var p3 = {
                        x: Math.floor(points[6] * xScale + 0.5),
                        y: Math.floor(points[7] * yScale + 0.5)
                    };
                    var len0 = Math.sqrt((p1.x - p0.x) * (p1.x - p0.x) + (p1.y - p0.y) * (p1.y - p0.y));
                    var len1 = Math.sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y));
                    var len2 = Math.sqrt((p3.x - p2.x) * (p3.x - p2.x) + (p3.y - p2.y) * (p3.y - p2.y));
                    var len3 = Math.sqrt((p0.x - p3.x) * (p0.x - p3.x) + (p0.y - p3.y) * (p0.y - p3.y));
                    var outputWidth = Math.round((len0 + len2) / 2);
                    var outputHeight = Math.round((len1 + len3) / 2);
                    outputBitmap = new Windows.Graphics.Imaging.SoftwareBitmap(
                        Windows.Graphics.Imaging.BitmapPixelFormat.bgra8,
                        outputWidth,
                        outputHeight,
                        Windows.Graphics.Imaging.BitmapAlphaMode.premultiplied);
                    var newPoints = [p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y];
                    try {
                        helper.clipToPoints(originalBitmap, outputBitmap, newPoints);
                        return Windows.Graphics.Imaging.BitmapEncoder.createAsync(Windows.Graphics.Imaging.BitmapEncoder.jpegEncoderId, finalStream);
                    } catch (e) {
                        // fallback: don't clip
                        outputBitmap = null;
                        return Windows.Graphics.Imaging.BitmapEncoder.createForTranscodingAsync(finalStream, decoder);
                    }
                } else {
                    return Windows.Graphics.Imaging.BitmapEncoder.createForTranscodingAsync(finalStream, decoder);
                }
            }).then(function (enc) {
                if (!bDontClip && outputBitmap) {
                    enc.setSoftwareBitmap(outputBitmap);
                }
                var displayInformation =
                    Windows.Graphics.Display.DisplayInformation.getForCurrentView();
                var currentOrientation = displayInformation.currentOrientation;

                // We need to rotate the photo wrt sensor orientation
                enc.bitmapTransform.rotation = orientationToRotation(currentOrientation);
                return enc.flushAsync();
            }).then(function () {
                return tempFolder.createFileAsync(fileName, OptUnique);
            }).then(function (file) {
                tempCapturedFile = file;
                return file.openAsync(Windows.Storage.FileAccessMode.readWrite);
            }).then(function (fileStream) {
                return Windows.Storage.Streams.RandomAccessStream.copyAndCloseAsync(finalStream,
                    fileStream);
            }).done(function () {
                photoStream.close();
                finalStream.close();
                complete(tempCapturedFile);
            },
            function () {
                photoStream.close();
                finalStream.close();
                throw new Error("An error has occured while capturing the photo.");
            });
        }).then(function (capturedFile) {
            return new WinJS.Promise(function (complete) {
                if (!capturedFile) {
                    complete(null);
                    return WinJS.Promise.as();
                } else if (returnBase64) {
                    return fileIO.readBufferAsync(capturedFile).done(function(buffer) {
                        var strBase64 = encodeToBase64String(buffer);
                        capturedFile.deleteAsync().done(function() {
                            complete(strBase64);
                        }, function(err) {
                            fail(err);
                        });
                    }, fail);
                } else {
                    complete(tempCapturedFile && tempCapturedFile.path);
                    return WinJS.Promise.as();
                }
            }).then(function (result) {
                that._captured = result;
            });
        });
    }
};


/**
 * Stops camera capture
 */
CameraUI.prototype.stop = function () {
    this._cancelled = true;
    this.release();
};

function degreesToRotation(degrees) {
    switch (degrees) {
        // portrait
        case 90:
            return Windows.Media.Capture.VideoRotation.clockwise90Degrees;
        // landscape
        case 0:
            return Windows.Media.Capture.VideoRotation.none;
        // portrait-flipped
        case 270:
            return Windows.Media.Capture.VideoRotation.clockwise270Degrees;
        // landscape-flipped
        case 180:
            return Windows.Media.Capture.VideoRotation.clockwise180Degrees;
        default:
            // Falling back to portrait default
            return Windows.Media.Capture.VideoRotation.clockwise90Degrees;
    }
}

module.exports = {

    /**
     * Opens preview via device camera and retieves photo from it.
     * @param  {function} success Success callback
     * @param  {function} fail    Error callback
     * @param  {array} args       Arguments array
     * args will contain :
     *  ...  it is an array, so be careful
     * 0 quality:0..100,
     * 1 convertToGrayscale:true|false,
     * 2 dontClip:true|false
     */
    scanDoc: function (success, fail, args) {
		var version = getMajorVersion();

        var captureCanvas,
            capturePreview,
            capturePreviewFrame,
            navigationButtonsDiv,
            previewMirroring,
            debugText,
            closeButton,
            settingsButton,
            photoButton,
            photoButtonBkg,
            capture,
            camera,
            videoProps,
            cancelPromise,
            photoProperties,
            videoPreviewProperties;

        // Save call state for suspend/resume
        CameraUI.openCameraCallArgs = {
            success: success,
            fail: fail,
            args: args
        };

        function getSourceType() {
            return CameraUI.openCameraCallArgs.args && CameraUI.openCameraCallArgs.args[0];
		}
        function getFileName() {
            return (CameraUI.openCameraCallArgs.args && CameraUI.openCameraCallArgs.args[1] || "image") + ".jpg";
		}
        function getQuality() {
            return CameraUI.openCameraCallArgs.args && CameraUI.openCameraCallArgs.args[2] ?
            Math.floor((1.0 - (CameraUI.openCameraCallArgs.args[2] - 1.0) / 4.0) * 100.0)
            : 100;
        }
        function getReturnBase64() {
            return CameraUI.openCameraCallArgs.args && CameraUI.openCameraCallArgs.args[3];
        }
        function getConvertToGrayscale() {
            return CameraUI.openCameraCallArgs.args && CameraUI.openCameraCallArgs.args[4];
        }
        function getDontClip() {
            return CameraUI.openCameraCallArgs.args && CameraUI.openCameraCallArgs.args[5];
        }
        function getDesiredMaxResolution() {
            return CameraUI.openCameraCallArgs.args && CameraUI.openCameraCallArgs.args[6];
        }
        function getAutoShutter() {
            return CameraUI.openCameraCallArgs.args && CameraUI.openCameraCallArgs.args[7];
        }
        function getRotationDegree() {
            return CameraUI.openCameraCallArgs.args && CameraUI.openCameraCallArgs.args[8];
        }
        function getAppBarSize() {
            /*if (CameraUI.openCameraCallArgs.args && CameraUI.openCameraCallArgs.args[9]) {
                if (typeof CameraUI.openCameraCallArgs.args[9] === "string") {
                    return parseInt(CameraUI.openCameraCallArgs.args[9]);
                }
                return CameraUI.openCameraCallArgs.args[9];
            }*/
            return null;
        }
        function getAppBarText() {
            /*if (CameraUI.openCameraCallArgs.args && CameraUI.openCameraCallArgs.args[10]) {
                return CameraUI.openCameraCallArgs.args[10];
            }*/
            return null;
        }

        var getUseEffectFilter = function() {
			if (!getDontClip() && version !== 10) {
				return true;
		    } else {
				return false;
		    }
	    }

        function resizeLater() {
            WinJS.Promise.timeout(CLICK_FOCUS_DELAY).then(function () {
                resizePreview();
            });
            return WinJS.Promise.as();
        }
        function updatePreviewForRotation(evt) {
            if (!capture) {
                return resizeLater();
            }
            var displayInformation = (evt && evt.target) || Windows.Graphics.Display.DisplayInformation.getForCurrentView();
            var currentOrientation = displayInformation.currentOrientation;

            previewMirroring = capture.getPreviewMirroring();

            // Lookup up the rotation degrees.
            var rotDegree = videoPreviewRotationLookup(currentOrientation, previewMirroring);

            capture.setPreviewRotation(degreesToRotation(rotDegree));
            return resizeLater();
        }


        function clickPreview() {
            tryFocus();
        }
        function resizePreview() {
            var width = document.body.clientWidth;
            var height = document.body.clientHeight;
            var displayInformation = Windows.Graphics.Display.DisplayInformation.getForCurrentView();
            var orientation = displayInformation.currentOrientation;
            reposition(width, height, orientation);
            if (photoButton && photoButton.style) {
                photoButtonBkg.style.width = (photoButton.clientHeight * 2).toString() + "px";
                photoButtonBkg.style.height = (photoButton.clientHeight * 2).toString() + "px";
                photoButtonBkg.style.borderWidth = photoButton.clientHeight.toString() + "px";
                photoButtonBkg.style.borderRadius = photoButton.clientHeight.toString() + "px";
                if (width > 499) {
                    var bottom = (height - photoButton.clientHeight) / 2;
                    photoButton.style.bottom = bottom.toString() + "px";
                    photoButton.style.right = "";
                    if (photoButtonBkg && photoButtonBkg.style) {
                        photoButtonBkg.style.bottom = (bottom - photoButton.clientHeight / 2 - photoButton.clientHeight / 10).toString() + "px";
                        photoButtonBkg.style.right = "";
                    }
                } else {
                    var right = width / 2 - photoButton.clientWidth;
                    photoButton.style.bottom = (photoButton.clientHeight / 2 + photoButton.clientHeight / 10).toString() + "px";
                    photoButton.style.right = right.toString() + "px";
                    if (photoButtonBkg && photoButtonBkg.style) {
                        photoButtonBkg.style.bottom = "0";
                        photoButtonBkg.style.right = right.toString() + "px";
                    }
                }
            }
        }

        function continueVideoOnFocus() {
            // if preview is defined it would be stuck, play it
            if (capturePreview) {
                capturePreview.play();
            }
        }

        /**
         * Creates a preview frame and necessary objects
         */
        function createPreview() {

            // Create fullscreen preview
            var capturePreviewFrameStyle = document.createElement("link");
            capturePreviewFrameStyle.rel = "stylesheet";
            capturePreviewFrameStyle.type = "text/css";
            capturePreviewFrameStyle.href = urlutil.makeAbsolute("/www/css/plugin-document-scanner.css");

            document.head.appendChild(capturePreviewFrameStyle);

            capturePreviewFrame = document.createElement("div");
            capturePreviewFrame.className = "document-scanner-ui-wrap";

            capturePreview = document.createElement("video");
            capturePreview.className = "document-scanner-ui-preview";

            if (getDontClip()) {
                capturePreview.addEventListener("click", clickPreview, false);
            } else {
                captureCanvas = document.createElement("canvas");
                captureCanvas.className = "document-scanner-ui-canvas";
                captureCanvas.addEventListener("click", clickPreview, false);
            }

            navigationButtonsDiv = document.createElement("div");
            navigationButtonsDiv.className = "document-scanner-ui-app-bar";
            navigationButtonsDiv.onclick = function (e) {
                e.cancelBubble = true;
            };
            if (!WinJS.Utilities.isPhone) {
                closeButton = document.createElement("span");
                closeButton.className = "document-scanner-app-bar-action document-scanner-action-close";
                navigationButtonsDiv.appendChild(closeButton);
            }
            var appBarText = getAppBarText();
            if (appBarText) {
                var actionText = document.createElement("span");
                actionText.className = "document-scanner-app-bar-action document-scanner-action-text";
                actionText.innerHTML = appBarText;
                navigationButtonsDiv.appendChild(actionText);
            }
            photoButtonBkg = document.createElement("span");
            photoButtonBkg.className = "document-scanner-app-bar-action-bkg document-scanner-action-bkg";
            navigationButtonsDiv.appendChild(photoButtonBkg);

            photoButton = document.createElement("span");
            photoButton.className = "document-scanner-app-bar-action document-scanner-action-photo";
            photoButton.addEventListener("click", capturePhoto, false);
            photoButton.addEventListener("mouseover", photoActionHot, false);
            photoButton.addEventListener("mouseout", photoActionNormal, false);
            navigationButtonsDiv.appendChild(photoButton);

            //settingsButton = document.createElement("span");
            //settingsButton.className = "document-scanner-app-bar-action document-scanner-action-settings";
            //navigationButtonsDiv.appendChild(settingsButton);

            CameraUI.captureCancelled = false;
            if (closeButton) {
                closeButton.addEventListener("click", cancelPreview, false);
            }

            if (getAutoShutter()) {
                cancelPromise = WinJS.Promise.timeout(60000).then(cancelPreview);
            }

            capturePreviewFrame.appendChild(capturePreview);
            if (!getDontClip()) {
                capturePreviewFrame.appendChild(captureCanvas);
            }
            capturePreviewFrame.appendChild(navigationButtonsDiv);

            window.addEventListener("resize", resizePreview, false);
            window.addEventListener("focus", continueVideoOnFocus, false);

            document.addEventListener("backbutton", cancelPreview, false);
        }

        function addEffectToImageStream(bDoCapture) {
            if (camera && typeof capture.addEffectAsync === "function") {
                var props = camera._props;
                var rotation = getRotationDegree();
                if (rotation) {
                    rotation = 360 - (rotation % 360);
                } else {
                    rotation = 0;
                }
                props.insert(PROPERTY_QUALITY, getQuality());
                props.insert(PROPERTY_GRAYSCALE, getConvertToGrayscale());
                props.insert(PROPERTY_ROTATION, rotation);
                props.insert(PROPERTY_CAPTURE, bDoCapture);

                return capture.clearEffectsAsync(Windows.Media.Capture.MediaStreamType.videoPreview).then(function() {
                    if (!getDontClip()) {
                        return capture.addEffectAsync(Windows.Media.Capture.MediaStreamType.videoPreview,
                            'ClippingCamera.ImageClipping',
                            props);
                    } else {
                        return null;
                    }
                });
            } else {
                return WinJS.Promise.as();
            }
        };

        function reposition(widthFrame, heightFrame, currentOrientation) {
            if (widthFrame > 0 && heightFrame > 0 && capturePreview && capturePreview.style) {
                // Lookup up the rotation degrees.
                var rotDegree = videoPreviewRotationLookup(currentOrientation, previewMirroring);

                var videoWidth;
                var videoHeight;
                if (rotDegree === 90 || rotDegree === 270) {
                    videoWidth = videoProps.height;
                    videoHeight = videoProps.width;
                } else {
                    videoWidth = videoProps.width;
                    videoHeight = videoProps.height;
                }
                if (videoWidth > 0 && videoHeight > 0) {
                    var width, height;
                    if (widthFrame * videoHeight > videoWidth * heightFrame) {
                        height = heightFrame;
                        width = Math.floor(heightFrame * videoWidth / videoHeight);
                    } else {
                        width = widthFrame;
                        height = Math.floor(widthFrame * videoHeight / videoWidth);
                    }
                    if (width > 0 && height > 0) {
                        var left = Math.floor((widthFrame - width) / 2);
                        var top = Math.floor((heightFrame - height) / 2);
                        if (rotDegree === 90 || rotDegree === 270) {
                            capturePreview.width = Math.floor(heightFrame * videoProps.width / videoProps.height);
                            capturePreview.height = heightFrame;
                            var previewLeft = Math.floor((widthFrame - capturePreview.width) / 2);
                            var previewTop = Math.floor((heightFrame - capturePreview.height) / 2);
                            capturePreview.style.left = previewLeft.toString() + "px";
                            capturePreview.style.top = previewTop.toString() + "px";
                        } else {
                            capturePreview.style.left = left.toString() + "px";
                            capturePreview.style.top = top.toString() + "px";
                            capturePreview.width = width;
                            capturePreview.height = height;
                        }
                        //capturePreview.style.width = width.toString() + "px";
                        //capturePreview.style.height = height.toString() + "px";
                        //if (capturePreview.msSetVideoRectangle) {
                        //    capturePreview.msSetVideoRectangle(0, 0, width-1, height-1);
                        //}
                        var rotAdd = getRotationDegree();
                        if (rotAdd) {
                            rotAdd = rotAdd % 360;
                            capturePreview.style.transform = "rotate(" + rotAdd.toString() + "deg)";
                        }
                        //console.log("width=" + width + " height=" + height + " left=" + left + " top=" + top);

                        if (captureCanvas) {
                            captureCanvas.width = width;
                            captureCanvas.height = height;
                            captureCanvas.rotDegree = rotDegree;
                            if (captureCanvas.style) {
                                if (rotAdd) {
                                    rotAdd = rotAdd % 360;
                                    captureCanvas.style.transform = "rotate(" + rotAdd.toString() + "deg)";
                                }
                                captureCanvas.style.left = left.toString() + "px";
                                captureCanvas.style.top = top.toString() + "px";
                                captureCanvas.style.width = width.toString() + "px";
                                captureCanvas.style.height = height.toString() + "px";
                            }
                        }
                    }
                }
            }
            var appBarSize = getAppBarSize();
            if (appBarSize && appBarSize > 48) {
                if (navigationButtonsDiv && navigationButtonsDiv.style) {
                    navigationButtonsDiv.style.height = appBarSize.toString() + "px";
                }
                var appBarActionSize = appBarSize - 8;
                var appBarFontSize = appBarActionSize - 22;
                if (closeButton && closeButton.style) {
                    closeButton.style.width = appBarActionSize.toString() + "px";
                    closeButton.style.height = appBarActionSize.toString() + "px";
                    closeButton.style.fontSize = appBarFontSize.toString() + "px";
                }
                if (photoButton && photoButton.style) {
                    photoButton.style.width = appBarActionSize.toString() + "px";
                    photoButton.style.height = appBarActionSize.toString() + "px";
                    photoButton.style.fontSize = appBarFontSize.toString() + "px";
                }
                //if (settingsButton && settingsButton.style) {
                //    settingsButton.style.width = appBarSize.toString() + "px";
                //    settingsButton.style.height = appBarSize.toString() + "px";
                //    settingsButton.style.fontSize = appBarFontSize.toString() + "px";
                //}
            }
        }

        function tryFocus(controller) {

            var result = WinJS.Promise.wrap();

            if (!capturePreview || capturePreview.paused) {
                // If the preview is not yet playing, there is no sense in running focus
                return result;
            }

            if (!controller) {
                try {
                    controller = capture && capture.videoDeviceController;
                } catch (err) {
                    console.log('Failed to access focus control for current camera: ' + err);
                    return result;
                }
            }

            if (!controller.focusControl || !controller.focusControl.supported) {
                if (controller.focus && typeof controller.focus.trySetAuto === "function") {
                    var bAutoOff = controller.focus.trySetAuto(false);
                    //console.log('Try set focus bAutoOff=' + bAutoOff);
                    WinJS.Promise.timeout(INITIAL_FOCUS_DELAY).then(function() {
                        var bAutoOn = controller.focus.trySetAuto(true);
                        //console.log('Try set focus bAutoOn=' + bAutoOn);
                    });
                } else {
                    console.log('Focus control for current camera is not supported');
                }
                return result;
            }

            // Multiple calls to focusAsync leads to internal focusing hang on some Windows Phone 8.1 devices
            // Also need to wrap in try/catch to avoid crash on Surface 3 - looks like focusState property
            // somehow is not accessible there.
            try {
                if (controller.focusControl.focusState === Windows.Media.Devices.MediaCaptureFocusState.searching) {
                    return result;
                }
            } catch (e) {
                // Nothing to do - just continue w/ focusing
            }

            try {
                return controller.focusControl.focusAsync().then(function () {
                    return result;
                }, function (e) {
                    // This happens on mutliple taps
                    if (e.number !== OPERATION_IS_IN_PROGRESS) {
                        console.error('focusAsync failed: ' + e);
                        return WinJS.Promise.wrapError(e);
                    }
                    return result;
                });
            } catch (e) {
                // This happens on mutliple taps
                if (e.number !== OPERATION_IS_IN_PROGRESS) {
                    console.error('focusAsync failed: ' + e);
                    return WinJS.Promise.wrapError(e);
                }
                return result;
            }
        }

        function setupFocus(controller) {

            var focusControl = controller.focusControl;
            if (focusControl && focusControl.supported && typeof focusControl.configure === "function") {
                function supportsFocusMode(mode) {
                    return (focusControl.supportedFocusModes &&
                        typeof focusControl.supportedFocusModes.indexOf === "function" &&
                        focusControl.supportedFocusModes.indexOf(mode) >= 0) ? true : false;
                }
                var focusConfig = new Windows.Media.Devices.FocusSettings();

                focusConfig.autoFocusRange = Windows.Media.Devices.AutoFocusRange.macro;

                // Determine a focus position if the focus search fails:
                focusConfig.disableDriverFallback = false;

                if (supportsFocusMode(Windows.Media.Devices.FocusMode.continuous)) {
                    console.log("Device supports continuous focus mode");
                    focusConfig.mode = Windows.Media.Devices.FocusMode.continuous;
                } else if (supportsFocusMode(Windows.Media.Devices.FocusMode.auto)) {
                    console.log("Device doesn\'t support continuous focus mode, switching to autofocus mode");
                    focusConfig.mode = Windows.Media.Devices.FocusMode.auto;
                }

                focusControl.configure(focusConfig);
            //} else if (controller.focus && typeof controller.focus.trySetAuto === "function") {
            //    controller.focus.trySetAuto(true);
            }

            // Continuous focus should start only after preview has started. See 'Remarks' at
            // https://msdn.microsoft.com/en-us/library/windows/apps/windows.media.devices.focuscontrol.configure.aspx
            function waitForIsPlaying() {
                var isPlaying = !capturePreview.paused && !capturePreview.ended && capturePreview.readyState > 2;

                if (!isPlaying) {
                    return WinJS.Promise.timeout(CHECK_PLAYING_TIMEOUT)
                        .then(function () {
                            return waitForIsPlaying();
                        });
                } else if (focusControl && focusControl.supported) {
                    return WinJS.Promise.timeout(INITIAL_FOCUS_DELAY)
                        .then(function() {
                            return tryFocus();
                        });
                } else {
                    return WinJS.Promise.as();
                }
            }
            return waitForIsPlaying();
        }

        function disableZoomAndScroll() {
            document.body.classList.add('no-zoom');
            document.body.classList.add('no-scroll');
        }

        function enableZoomAndScroll() {
            document.body.classList.remove('no-zoom');
            document.body.classList.remove('no-scroll');
        }

        function getAspectRatios() {
            var photoAspectRatios = [];

            if (photoProperties) {
                photoAspectRatios = photoProperties.map(function (element) {
                    return (element.width / element.height).toFixed(1);
                }).filter(function (element, index, array) { return (index === array.indexOf(element)); });
            }

            var videoPreviewAspectRatios = videoPreviewProperties.map(function (element) {
                return (element.width / element.height).toFixed(1);
            }).filter(function (element, index, array) { return (index === array.indexOf(element)); });

            var allAspectRatios = [].concat(photoAspectRatios, videoPreviewAspectRatios);

            var aspectObj = allAspectRatios.reduce(function (map, item) {
                if (!map[item]) {
                    map[item] = 0;
                }
                map[item]++;
                return map;
            }, {});

            if (photoAspectRatios.length > 0) {
                return Object.keys(aspectObj).filter(function (k) {
                    return aspectObj[k] === 2;
                });
            } else {
                return Object.keys(aspectObj);
            }
        }

        /**
         * Stops preview and then call success callback with cancelled=true
         */
        function cancelPreview() {
            CameraUI.captureCancelled = true;
            camera && camera.stop();
        }

        function capturePhoto() {
            if (getUseEffectFilter()) {
			  addEffectToImageStream(true);
		    }
            camera && camera.capturePhoto(getDontClip(), getUseEffectFilter(), getQuality(), getFileName(), getReturnBase64(), getConvertToGrayscale());
        }

        function photoActionHot() {
            if (photoButtonBkg && photoButtonBkg.style) {
                photoButtonBkg.style.backgroundColor = "black";
            }
            if (photoButton && photoButton.style) {
                photoButton.style.color = "white";
            }
        }

        function photoActionNormal() {
            if (photoButtonBkg && photoButtonBkg.style) {
                photoButtonBkg.style.backgroundColor = "";
            }
            if (photoButton && photoButton.style) {
                photoButton.style.color = "";
            }
        }

        function checkCancelled() {
            if (CameraUI.captureCancelled || CameraUI.suspended) {
                throw new Error('Canceled');
            }
        }

        /**
         * Starts stream transmission to preview frame and then run barcode search
         */
        function startPreview() {
            var captureSettings = null;
            return findCamera()
            .then(function (id) {
                var captureInitSettings, videoDeviceController;
                try {
                    captureInitSettings = new Windows.Media.Capture.MediaCaptureInitializationSettings();
                } catch (e) {
                    if (e.number === REGDB_E_CLASSNOTREG) {
                        throw new Error('Ensure that you have Windows Media Player and Media Feature pack installed.');
                    }

                    throw e;
                }
                captureInitSettings.videoDeviceId = id;
                captureInitSettings.sharingMode = Windows.Media.Capture.MediaCaptureSharingMode &&
                    Windows.Media.Capture.MediaCaptureSharingMode.exclusiveControl;
                captureInitSettings.streamingCaptureMode = Windows.Media.Capture.StreamingCaptureMode.video;

                // use of SoftwareBitmap member in captured frames
                captureInitSettings.memoryPreference = Windows.Media.Capture.MediaCaptureMemoryPreference &&
                    Windows.Media.Capture.MediaCaptureMemoryPreference.cpu;

                // try first with photo capture source
                captureInitSettings.photoCaptureSource = Windows.Media.Capture.PhotoCaptureSource.photo;

                capture = new Windows.Media.Capture.MediaCapture();
                return new WinJS.Promise(function(initComplete, initError) {
                    return capture.initializeAsync(captureInitSettings).then(function() {
                        videoDeviceController = capture.videoDeviceController;
                        photoProperties = videoDeviceController.getAvailableMediaStreamProperties(Windows.Media.Capture.MediaStreamType.photo);
                        videoPreviewProperties = videoDeviceController.getAvailableMediaStreamProperties(Windows.Media.Capture.MediaStreamType.videoPreview);
                        initComplete(capture);
                    }, function(error) {
                        // try again with automatic capture source
                        captureInitSettings.photoCaptureSource = Windows.Media.Capture.PhotoCaptureSource.videoPreview;
                        return capture.initializeAsync(captureInitSettings).then(function() {
                            videoDeviceController = capture.videoDeviceController;
                            videoPreviewProperties = videoDeviceController.getAvailableMediaStreamProperties(Windows.Media.Capture.MediaStreamType.videoPreview);
                            initComplete(capture);
                        },
                        function(err) {
                            initError(err);
                        });
                    });
                });

            })
            .then(function () {
                // Get available aspect ratios
                var defaultAspectRatio = DEFAULT_ASPECT_RATIO;
                var aspectRatios = getAspectRatios();
                var sortedAspectRatios = aspectRatios.sort(function (aspectA, aspectB) {
                    // sort aspect by defaultAspectRatio
                    return (aspectA - defaultAspectRatio)*(aspectA - defaultAspectRatio) - (aspectB - defaultAspectRatio)*(aspectB - defaultAspectRatio);
                });
                var aspect = sortedAspectRatios[0];

                var maxResolution = getDesiredMaxResolution() || 5000000;

                // Max photo resolution with desired aspect ratio
                var videoDeviceController = capture.videoDeviceController;
                var photoResolution = {};

                var filteredResolutions;
                if (photoProperties) {
                    filteredResolutions = photoProperties.filter(function(elem) {
                        return ((elem.width / elem.height).toFixed(1) === aspect && elem.width * elem.height <= maxResolution);
                    });
                    if (filteredResolutions && filteredResolutions.length > 0) {
                        // Max photo resolution with desired aspect ratio and <= maxResolution
                        photoResolution = filteredResolutions.reduce(function(prop1, prop2) {
                            return (prop1.width * prop1.height) > (prop2.width * prop2.height) ? prop1 : prop2;
                        });
                    } else {
                        // Min photo resolution with desired aspect ratio
                        photoResolution = photoProperties.filter(function(elem) {
                            return ((elem.width / elem.height).toFixed(1) === aspect);
                        }).reduce(function(prop1, prop2) {
                            return (prop1.width * prop1.height) < (prop2.width * prop2.height) ? prop1 : prop2;
                        });
                    }
                    if (maxResolution > 2100000) {
                        maxResolution = 2100000;
                    }
                }

                var videoPreviewResolution;
                filteredResolutions = videoPreviewProperties.filter(function(elem) {
                        return ((elem.width / elem.height).toFixed(1) === aspect && elem.width * elem.height <= maxResolution);
                    });
                if (filteredResolutions && filteredResolutions.length > 0) {
                    // Max video preview resolution with desired aspect ratio and <= maxResolution
                    videoPreviewResolution = filteredResolutions.reduce(function (prop1, prop2) {
                        return (prop1.width * prop1.height) > (prop2.width * prop2.height) ? prop1 : prop2;
                    });
                } else {
                    // Min video preview resolution with desired aspect ratio
                    videoPreviewResolution = videoPreviewProperties.filter(function(elem) {
                        return ((elem.width / elem.height).toFixed(1) === aspect);
                    }).reduce(function (prop1, prop2) {
                        return (prop1.width * prop1.height) < (prop2.width * prop2.height) ? prop1 : prop2;
                    });
                }


                videoProps = videoPreviewResolution;
                captureSettings = {
                    capture: capture,
                    width: videoProps.width,
                    height: videoProps.height
                }

                if (photoResolution.length > 0) {
                    return videoDeviceController.setMediaStreamPropertiesAsync(Windows.Media.Capture.MediaStreamType.photo, photoResolution)
                        .then(function () {
                            return videoDeviceController.setMediaStreamPropertiesAsync(Windows.Media.Capture.MediaStreamType.videoPreview, videoPreviewResolution);
                        });
                } else {
                    return videoDeviceController.setMediaStreamPropertiesAsync(Windows.Media.Capture.MediaStreamType.videoPreview, videoPreviewResolution);
                }
            })
            .then(function () {
                capturePreview.msZoom = true;
                capturePreview.src = URL.createObjectURL(capture);
                capturePreview.play();

                // Insert preview frame and controls into page
                document.body.appendChild(capturePreviewFrame);

                resizePreview();
                disableZoomAndScroll();

                return setupFocus(capture.videoDeviceController)
                .then(function () {
                    Windows.Graphics.Display.DisplayInformation.getForCurrentView().addEventListener("orientationchanged", updatePreviewForRotation, false);
                    return updatePreviewForRotation();
                })
                .then(function () {
                    if (!Windows.Media.Devices.CameraStreamState) {
                        // CameraStreamState is available starting with Windows 10 so skip this check for 8.1
                        // https://msdn.microsoft.com/en-us/library/windows/apps/windows.media.devices.camerastreamstate
                        return WinJS.Promise.as();
                    }
                    function checkCameraStreamState() {
                        if (capture.cameraStreamState !== Windows.Media.Devices.CameraStreamState.streaming) {
                            // Using loop as MediaCapture.CameraStreamStateChanged does not fire with CameraStreamState.streaming state.
                            return WinJS.Promise.timeout(CAMERA_STREAM_STATE_CHECK_RETRY_TIMEOUT)
                            .then(function () {
                                return checkCameraStreamState();
                            });
                        }
                        return WinJS.Promise.as();
                    }
                    // Ensure CameraStreamState is Streaming
                    return checkCameraStreamState();
                })
                .then(function () {
                    return WinJS.Promise.timeout(CHECK_PLAYING_TIMEOUT).then(function () {
                        if (getAutoShutter()) {
                            WinJS.Promise.timeout(getAutoShutter()).then(function () {
                                if (photoButton && photoButton.style) {
                                    photoButton.style.display = "none";
                                }
                                if (photoButtonBkg && photoButtonBkg.style) {
                                    photoButtonBkg.style.display = "none";
                                }
                                capturePhoto();
                            });
                        }
                    });
                })
                .then(function () {
                    resizePreview();
                    return captureSettings;
                });
            });
        }

        /**
         * Removes preview frame and corresponding objects from window
         */
        function destroyPreview() {

            var promise;
			if (getUseEffectFilter() && capture) {
			    promise = capture.clearEffectsAsync(Windows.Media.Capture.MediaStreamType.videoPreview)
		    } else {
				promise = WinJS.Promise.as();
			}

            if (cancelPromise) {
                cancelPromise.cancel();
                cancelPromise = null;
            }

            return promise.then(function() {
                Windows.Graphics.Display.DisplayInformation.getForCurrentView().removeEventListener("orientationchanged", updatePreviewForRotation, false);
                document.removeEventListener("backbutton", cancelPreview);

                if (capturePreview) {
                    var isPlaying = !capturePreview.paused && !capturePreview.ended && capturePreview.readyState > 2;
                    if (isPlaying) {
                        capturePreview.pause();
                    }

                    // http://stackoverflow.com/a/28060352/4177762
                    capturePreview.src = "";
                    if (capturePreview.load) {
                        capturePreview.load();
                    }
                    window.removeEventListener("focus", continueVideoOnFocus);
                    window.removeEventListener("resize", resizePreview);
                    if (captureCanvas) {
                        captureCanvas.removeEventListener("click", clickPreview);
                    } else {
                        capturePreview.removeEventListener("click", clickPreview);
                    }
                }
                if (closeButton) {
                    closeButton.removeEventListener("click", cancelPreview);
                }
                if (photoButton) {
                    photoButton.removeEventListener("click", capturePhoto);
                    photoButton.removeEventListener("mouseover", photoActionHot);
                    photoButton.removeEventListener("mouseout", photoActionNormal);
                }

                if (capturePreviewFrame) {
                    try {
                        document.body.removeChild(capturePreviewFrame);
                    } catch (e) {
                        // Catching NotFoundError
                        console.error(e);
                    }
                }
                capturePreviewFrame = null;

                camera && camera.stop();
                camera = null;

                if (capture) {
                    try {
                        promise = capture.stopRecordAsync();
                    } catch (e) {
                        // Catching NotFoundError
                        console.error(e);
                    }
                }
                capture = null;

                enableZoomAndScroll();
		    });
        }

        // Timeout is needed so that the .done finalizer below can be attached to the promise.
        CameraUI.capturePromise = WinJS.Promise.timeout()
        .then(function() {
            createPreview();
            checkCancelled();
            return startPreview();
        })
        .then(function (captureSettings) {
            checkCancelled();
            camera = CameraUI.get();
            camera.init(captureSettings.capture, captureSettings.width, captureSettings.height, capturePreview, captureCanvas, fail);

            // Add a small timeout before capturing first frame otherwise
            return WinJS.Promise.timeout(200).then(function() {
                return captureSettings;
            });
        })
        .then(function () {
            checkCancelled();
            if (getUseEffectFilter()) {
                return addEffectToImageStream(false);
		    } else if (getDontClip()) {
                return WinJS.Promise.as();
            } else {
                return camera.createReader().then(function (frameReader) {
					checkCancelled();
					if (getDontClip()) {
						return WinJS.Promise.as();
					}
					return camera.attach(frameReader);
				});
            }
        })
        .then(function () {
			if (!getUseEffectFilter() && !getDontClip()) {
                WinJS.Promise.timeout(CHECK_FRAME_ARRIVED_TIMEOUT).then(function() {
                    checkCancelled();
                    if (!camera.hasFrameArrived()) {
                        camera.release().then(function() {
                            checkCancelled();
                            return camera.createReader(true);
                        }).then(function(frameReader) {
                            checkCancelled();
                            camera.attach(frameReader);
                        });
                    }
                });
            }
            checkCancelled();
            return camera.capturing(getUseEffectFilter());
        })
        .then(function (result) {
            // Suppress null result (cancel) on suspending
            if (CameraUI.suspended) {
                return;
            }
            checkCancelled();

            destroyPreview();
            success(result);
        });

        // Catching any errors here
        CameraUI.capturePromise.done(function () { }, function (error) {
            // Suppress null result (cancel) on suspending
            if (CameraUI.suspended) {
                return;
            }

            destroyPreview();
            fail(error);
        });

        CameraUI.videoPreviewIsVisible = function () {
            return capturePreviewFrame !== null;
        }

        CameraUI.destroyPreview = destroyPreview;
    }
};

var app = WinJS.Application;

function waitForCaptureEnd() {
    return CameraUI.capturePromise || WinJS.Promise.as();
}

function suspend(args) {
    CameraUI.suspended = true;
    if (args) {
        args.setPromise(CameraUI.destroyPreview()
        .then(waitForCaptureEnd, waitForCaptureEnd));
    } else {
        CameraUI.destroyPreview();
    }
}

function resume() {
    CameraUI.suspended = false;
    module.exports.openCamera(CameraUI.openCameraCallArgs.success, CameraUI.openCameraCallArgs.fail, CameraUI.openCameraCallArgs.args);
}

function onVisibilityChanged() {
    if (document.visibilityState === 'hidden'
        && CameraUI.videoPreviewIsVisible && CameraUI.videoPreviewIsVisible() && CameraUI.destroyPreview) {
        suspend();
    } else if (CameraUI.suspended) {
        resume();
    }
}

// Windows 8.1 projects
document.addEventListener('msvisibilitychange', onVisibilityChanged);
// Windows 10 projects
document.addEventListener('visibilitychange', onVisibilityChanged);

// About to be suspended
app.addEventListener('checkpoint', function (args) {
    if (CameraUI.videoPreviewIsVisible && CameraUI.videoPreviewIsVisible() && CameraUI.destroyPreview) {
        suspend(args);
    }
});

// Resuming from a user suspension
Windows.UI.WebUI.WebUIApplication.addEventListener("resuming", function () {
    if (CameraUI.suspended) {
        resume();
    }
}, false);

require("cordova/exec/proxy").add("Scan", module.exports);
