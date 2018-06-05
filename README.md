# cordova-plugin-document-scanner

This plugin defines a global `scan` object, which provides an API for scan the document from taking pictures and choosing image from the system's library. 

Although the object is attached to the global scoped `window`, it is not available until after the `deviceready` event.

    document.addEventListener("deviceready", onDeviceReady, false);
    function onDeviceReady() {
        console.log(scan);
    }

## Installation


This requires cordova 7.1.0+
npm link :- https://www.npmjs.com/package/cordova-plugin-document-scanner

    cordova plugin add cordova-plugin-document-scanner
    

### scan.scanDoc(sourceType, successCallback, errorCallback)
Takes a photo using the scan, or retrieves a photo from the device's
image gallery.  The image is passed to the document scanner and the scanned image passed to success callback as the URI for the image file.

The `scan.scanDoc` function opens the device's default scan
application that allows users to snap pictures by default - this behavior occurs,
when `sourceType` equals `1`.
Once the user snaps the photo, the scan application closes and the application is restored.

If `sourceType` is `0`, then a dialog displays
that allows users to select an existing image.

The return value is sent to the [`scanSuccess`](#module_scan.onSuccess) callback function, in
the fileUri formats.

You can do whatever you want with the URI, for
example:

- Render the image in an `<img>` tag.

__Supported Platforms__

- Android
- iOS

**Example**  
```js
scan.scanDoc(sourceType, scanSuccess, scanError);
```

## `scan.scanDoc`

Take a photo and retrieve the image's file location:

    scan.scanDoc(0, onSuccess, onFail);

    function onSuccess(imageURI) {
        var image = document.getElementById('myImage');
        image.src = imageURI;
    }

    function onFail(message) {
        alert('Failed because: ' + message);
    }

## iOS Quirks

NOTE :- iOS has only document scan via camera for now (Any argument passed will start the camera scan). Document Scan from gallery will be available in future version.

An example file URI obtained from success call back of scanDoc function looks like this  file:///var/mobile/Containers/Data/Application/8376778A-983B-4FBA-B21C-A4CFDD047AAA/Documents/image.png

## Issues and Fixes

- Error:Execution failed for task ':app:transformNativeLibsWithStripDebugSymbolForDebug' <br/>
    Delete local ndk-bundle folder. Example location :- C:\Users\Administrator\AppData\Local\Android\sdk\ndk-bundle
    
- CropViewController fails in Xcode due to Incompatible Swift Versions <br/>
    Refer issue [13](https://github.com/NeutrinosPlatform/cordova-plugin-document-scanner/issues/13)
    
- Couldn't find "libopencv_java3.so" <br/>
    Refer issue [8](https://github.com/NeutrinosPlatform/cordova-plugin-document-scanner/issues/8)
    
- iOS scan UI buttons documentation <br/>
    Refer issue [15](https://github.com/NeutrinosPlatform/cordova-plugin-document-scanner/issues/15) <br/>
## Credits / Native library links

Android :- https://github.com/jhansireddy/AndroidScannerDemo <br/>
iOS :- https://github.com/charlymr/IRLDocumentScanner

Huge thanks to these authors for making their document scanning native libraries public.

## More about us!

Find out more or contact us directly here :- http://www.neutrinos.co/

Facebook :- https://www.facebook.com/Neutrinos.co/ <br/>
LinkedIn :- https://www.linkedin.com/company/25057297/ <br/>
Twitter :- https://twitter.com/Neutrinosco <br/>
Instagram :- https://www.instagram.com/neutrinos.co/



