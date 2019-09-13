
# cordova-plugin-document-scanner [![npm version](https://badge.fury.io/js/cordova-plugin-document-scanner.svg)](//npmjs.com/package/cordova-plugin-document-scanner)

>  **DOCUMENTATION** - This doc explains the use of plugin ver 4.x.x. For documentation of ver 3.x.x of the plugin, please see the branch [ver/3.x.x](https://github.com/NeutrinosPlatform/cordova-plugin-document-scanner/tree/ver/3.x.x) on github. For ver 2.x.x and below please read the documentation within each of the npm releases.

This plugin defines a global `scan` object, which provides an API to scan the document using camera (iOS and Android) or by choosing an image from the system's photo library (Android). The plugin does provide edge detection while scanning.

Although the object is attached to the global scoped `window`, it is not available until after the `deviceready` event.

```js
document.addEventListener("deviceready", onDeviceReady, false);

function onDeviceReady() {
    console.log(scan);
}
```

# Supported Platforms

- Android

- iOS
  
# Installation

```
cordova plugin add cordova-plugin-document-scanner@4.x.x
```

> Plugin versions **2.x.x and below are now deprecated**. Please use 4.x.x for a stable releases and 3.x.x if you want to be able to use the WeScan Swift library. This requires cordova 7.1.0+ and cordova android 6.4.0+ <br/>

npm link :- https://www.npmjs.com/package/cordova-plugin-document-scanner

> 4.x.x uses IRLDocumentScanner ios scan Library (Objective C) - More stable because cordova works seamlessly with objective C

> 3.x.x uses WeScan ios scan Library (Swift)

*Please read issues and fixes section of readme for Ionic & PhoneGap installation*

# Usage

```js
scan.scanDoc(successCallback, errorCallback, options)
```

Takes a photo using the scan plugin, or retrieves a photo from the device's image gallery. The image is passed to the document scanner and the scanned image is passed to success callback as the URI for the image file.

The `scan.scanDoc` function opens the device's camera that allows users to snap pictures by default. Once the user snaps the photo, the scan application closes and the application is restored.

## successCallback
 - The function called when an image has been scanned successfully. It returns image URL or image as base64 depending on the options passed in by the user. You can do whatever you want with the URI or the base64, for example, render the image in an `<img>` tag.

## errorCallback
 - The function called when an error occurs. It returns the error message as a string back to the plugin user.

## options
 - **sourceType** [Default value is 1]  :- When `sourceType` in options object is `1`(default) device camera opens so user can click a pic. When `0` gallery opens up so user can select an image.
	- **Platform Support** : Android only
	- **Version Support** : 3.x.x & 4.x.x 
	 
 - **fileName** [Default value is "image"] :- User can specify the name of the file without the file extension. File extension is always .jpg for now
	- **Platform Support** : iOS only
	- **Version Support** : 4.x.x only
	- **Important Notes** : Please cleanup the files if not using default value.

 - **quality** [Default value is 1]  :- `quality` in options object can take float values values from `1.0`(default - Highest quality) to `5.0`(Lowest Quality). Any value not equal to or not between these values will default to the highest quality of `1.0`.
	- **Platform Support** : Android and iOS
	- **Version Support** : >= 4.1.0 
    - **Important Notes** : Android subsamples to change quality while iOS does JPEG compression to change quality so there might be small changes in quality between devices. 

 - **returnBase64** [Default value is false]  :- `returnBase64` in options object can only take boolean values. If `true`, the plugin will return the scanned image as base64. If `false`, the plugin will return the image URL of the image. 
	- **Platform Support** : Android and iOS
	- **Version Support** : >= 4.2.0 
    - **Important Notes** : For Android even when base64 is returned the image will still get saved to memory. For iOS this isn't the case.

> Plugin adds file:// in front of the imageuri returned for both android and ios. <br/>
**iOS example imageURI returned** :- file:///var/mobile/Containers/Data/Application/8376778A-983B-4FBA-B21C-A4CFDD047AAA/Documents/image.jpg <br/>
**Android example imageURI returned** :- file:///storage/emulated/0/Pictures/1563790575755.jpg

# Example

```js
scan.scanDoc(successCallback, errorCallback, options);
```

**options example**

```js
{
    sourceType : 1,
    fileName : "myfile",
    quality : 2.5,
    returnBase64 : true
}
```

**working example**

Take a photo and retrieve the image's file location. Options need not be passed in, if the default values are being used.
```js
    scan.scanDoc(successCallback, errorCallback, {sourceType : 1, fileName : "myfilename", quality : 1.0, returnBase64 : false}); 
    // sourceType will by default take value 1 if no value is set | 0 for gallery | 1 for camera. 
    // fileName will take default value "image" if no value set. Supported only on 4.x.x plugin version
    // quality will take default value 1.0 (highest). Lowest value is 5.0. Any value in between will be accepted
    // returnBase64 will take default boolean value false, meaning image URL is returned. If true base64 is returned
    function successCallback(imageData) {
        alert(imageData);
        console.log(imageData);
        //var image = document.getElementById('myImage');
        //image.src = imageData; // Image URL rendering. 
	//image.src = imageData + '?' + Date.now(); // For iOS, use this to solve issue 10 if unique fileName is not set.
	//image.src = "data:image/jpeg;base64," + imageData; // Base64 rendering
    }

    function errorCallback(message) {
        alert('Failed because: ' + message);
    }
```

# iOS Quirks

- iOS has only document scan via camera for now (Any argument passed will start the camera scan). Document Scan from gallery will be available in future version. Also scanned images aren't saved to the gallery in iOS. 

- Please don't forget to delete the files if you use the `fileName` option to create your own filenames.

- An example file URI obtained from success call back of scanDoc function looks like this file:///var/mobile/Containers/Data/Application/8376778A-983B-4FBA-B21C-A4CFDD047AAA/Documents/image.jpg


# Android Quirks

 - Android allows delete of files from the file manager. In iOS this is not possible hence the use of `fileName`. Android will ignore the file name parameter in options.

 - Android example imageURI returned :- file:///storage/emulated/0/Pictures/1563790575755.jpg

# Issues and Fixes

<details>
<summary>Expand to view the most common issues and their workarounds.</summary>
<p>

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

- If you are using WKWebView for iOS please follow the workaround in the following issue. <br/>

    Refer issue [56](https://github.com/NeutrinosPlatform/cordova-plugin-document-scanner/issues/56) <br/>

</p>
</details>

# Credits / Native library links

Android :- [AndroidScanner](https://github.com/jhansireddy/AndroidScannerDemo) <br/>

iOS [4.x.x] :- [IRLDocumentScanner](https://github.com/charlymr/IRLDocumentScanner) <br/>

iOS [3.x.x] :- [WeScan](https://github.com/WeTransfer/WeScan)
  
Huge thanks to these authors for making their document scanning native libraries public.


# More about us!

Find out more or contact us directly here :- http://www.neutrinos.co/

Facebook :- https://www.facebook.com/Neutrinos.co/ <br/>

LinkedIn :- https://www.linkedin.com/company/25057297/ <br/>

Twitter :- https://twitter.com/Neutrinosco <br/>

Instagram :- https://www.instagram.com/neutrinos.co/
