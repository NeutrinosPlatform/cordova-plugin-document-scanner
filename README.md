
# cordova-plugin-document-scanner

>  Please raise a pull request if there are fixes or enhancements that you want to add!

>  **DOCUMENTATION** - This doc explains use of ver 3.x.x and ver 4.x.x. For 2.x.x and below please read the documentation within each of the npm releases.

This plugin defines a global `scan` object, which provides an API for scan the document from taking pictures and choosing image from the system's library.

Although the object is attached to the global scoped `window`, it is not available until after the `deviceready` event.

```
document.addEventListener("deviceready", onDeviceReady, false);
function onDeviceReady() {
console.log(scan);
}
```
  
  
## Installation

`cordova plugin add cordova-plugin-document-scanner@4.x.x`

> Plugin versions **2.x.x and below are now deprecated**. Please use 4.x.x for a stable releases and 3.x.x if you want to be able to use the WeScan Swift library
This requires cordova 7.1.0+ and cordova android 6.4.0+ <br/>

npm link :- https://www.npmjs.com/package/cordova-plugin-document-scanner

> 4.x.x uses IRLDocumentScanner ios scan Library (Objective C) - More stable because cordova works seamlessly with objective C

> 3.x.x uses WeScan ios scan Library (Swift)

*Please read issues and fixes section of readme for Ionic & PhoneGap installation*

### scan.scanDoc(successCallback, errorCallback, options)

Takes a photo using the scan plugin, or retrieve a photo from the device's

image gallery. The image is passed to the document scanner and the scanned image passed to success callback as the URI for the image file.

The `scan.scanDoc` function opens the device's camera that allows users to snap pictures by default. Once the user snaps the photo, the scan application closes and the application is restored.

#### options
 - **sourceType** [Default value is 1]  :- When `sourceType` in options object is `1`(default) device camera opens so user can click a pic. When `0` gallery opens up so user can select an image.
	- **Platform Support** : Android only
	- **Version Support** : 3.x.x & 4.x.x 
	 
 - **fileName** [Default value is "image"] :- User can specify the name of the file with the file extension. File extension is always .png for now
	- **Platform Support** : iOS only
	- **Version Support** : 4.x.x only
	- **Important Notes** : Please cleanup the files if not using default value.

The return value is sent to the [`scanSuccess`](#module_scan.onSuccess) callback function, in fileUri format. You can do whatever you want with the URI, for example, render the image in an `<img>` tag.

> *Plugin adds file:// in front of the imageuri returned for both android and ios [iOS example imageURI returned :- file:///var/mobile/Containers/Data/Application/8376778A-983B-4FBA-B21C-A4CFDD047AAA/Documents/image.png]*

## Supported Platforms

- Android

- iOS


## Example

```js

scan.scanDoc(scanSuccess, scanError, options);

```

**options example**
```js
{
	sourceType : 1,
	fileName : "myfile"
}
```

**working example**


Take a photo and retrieve the image's file location:
```
    scan.scanDoc(onSuccess, onFail, {sourceType:1, fileName:"myfilename"}); 
    // sourceType will by default take value 1 if no value is set | 0 for gallery | 1 for camera. 
    // fileName will take default value "image" if no value set. Supported only on 4.x.x plugin version

    function onSuccess(imageURI) {
        alert(imageURI);
        console.log(imageURI);
        //var image = document.getElementById('myImage');
        //image.src = imageURI; // For iOS, use image.src = imageURI + '?' + Date.now(); to solve issue 10 if unique fileName is not set.

    }

    function onFail(message) {
        alert('Failed because: ' + message);
    }
```
> Options need not be passed in, if the default values are being used.

## iOS Quirks

- iOS has only document scan via camera for now (Any argument passed will start the camera scan). Document Scan from gallery will be available in future version. Also scanned images aren't saved to the gallery in iOS. 

- Please don't forget to delete the files if you use the `fileName` option to create your own filenames.

 - An example file URI obtained from success call back of scanDoc function looks like this file:///var/mobile/Containers/Data/Application/8376778A-983B-4FBA-B21C-A4CFDD047AAA/Documents/image.png


## Android Quirks

NOTE :- Android allows delete of files from the file manager. In iOS this is not possible hence the use of `fileName`. Android won't accept the file name parameter in options.


## Issues and Fixes

- Error:Execution failed for task ':app:transformNativeLibsWithStripDebugSymbolForDebug' <br/>

Delete local ndk-bundle folder. Example location :- C:\Users\Administrator\AppData\Local\Android\sdk\ndk-bundle

- CropViewController fails in Xcode due to Incompatible Swift Versions <br/>

Refer issue [13](https://github.com/NeutrinosPlatform/cordova-plugin-document-scanner/issues/13)

- Couldn't find "libopencv_java3.so" [Problem mainly with 64 bit build devices]<br/>

Refer issue [8](https://github.com/NeutrinosPlatform/cordova-plugin-document-scanner/issues/8)

- iOS scan UI buttons documentation <br/>

Refer issue [15](https://github.com/NeutrinosPlatform/cordova-plugin-document-scanner/issues/15)

- Adding plugin in Ionic <br/>

Refer 6th response in issue [17](https://github.com/NeutrinosPlatform/cordova-plugin-document-scanner/issues/17)

- Adding plugin in PhoneGap <br/>

Refer entire issue [22](https://github.com/NeutrinosPlatform/cordova-plugin-document-scanner/issues/22)

- iOS: multiple scan does not override the first image <br/>

Refer entire issue [10](https://github.com/NeutrinosPlatform/cordova-plugin-document-scanner/issues/10)

- Multiple scans don't override the first image | Browser caching issue <br/>

Refer issue [10](https://github.com/NeutrinosPlatform/cordova-plugin-document-scanner/issues/10) <br/>

## Credits / Native library links

Android :- [AndroidScanner](https://github.com/jhansireddy/AndroidScannerDemo) <br/>

iOS [4.x.x] :- [IRLDocumentScanner](https://github.com/charlymr/IRLDocumentScanner) <br/>

iOS [3.x.x] :- [WeScan](https://github.com/WeTransfer/WeScan)
  
Huge thanks to these authors for making their document scanning native libraries public.


## More about us!

Find out more or contact us directly here :- http://www.neutrinos.co/

Facebook :- https://www.facebook.com/Neutrinos.co/ <br/>

LinkedIn :- https://www.linkedin.com/company/25057297/ <br/>

Twitter :- https://twitter.com/Neutrinosco <br/>

Instagram :- https://www.instagram.com/neutrinos.co/