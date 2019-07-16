import Foundation
import WeScan

var com = CDVInvokedUrlCommand()
var uri = "";

@objc(Scan) class Scan : CDVPlugin, ImageScannerControllerDelegate {
    
    
    @objc(scanDoc:)
    func scanDoc(_ command: CDVInvokedUrlCommand){
        com = command

        let scannerViewController = ImageScannerController()
        scannerViewController.imageScannerDelegate = self
        
        // Get the top most view and start the camera
        if var topController = UIApplication.shared.keyWindow?.rootViewController {
            while let presentedViewController = topController.presentedViewController {
                topController = presentedViewController
            }
            topController.present(scannerViewController, animated: true)
        }
    }
    
    
    func imageScannerController(_ scanner: ImageScannerController, didFailWithError error: Error) {
        print(error)
        let pluginResult = CDVPluginResult(status: CDVCommandStatus_ERROR, messageAs: (error as? String));
        commandDelegate.send(pluginResult, callbackId:com.callbackId);
        scanner.dismiss(animated: true)
    }
    
    func imageScannerController(_ scanner: ImageScannerController, didFinishScanningWithResults results: ImageScannerResults) {
        if(saveImage(image: results.doesUserPreferEnhancedImage ? (results.enhancedImage ?? results.scannedImage) : results.scannedImage)) {
            let pluginResult = CDVPluginResult(status: CDVCommandStatus_OK, messageAs: uri);
            commandDelegate.send(pluginResult, callbackId:com.callbackId);
        }
        else {
            let pluginResult = CDVPluginResult(status: CDVCommandStatus_ERROR, messageAs: "File creation failed!");
            commandDelegate.send(pluginResult, callbackId:com.callbackId);
        }
        scanner.dismiss(animated: true)
    }
    
    func saveImage(image: UIImage) -> Bool {
        guard let data = image.jpegData(compressionQuality:1.0) ?? image.pngData() else {
            return false
        }
        guard let directory = try? FileManager.default.url(for: .documentDirectory, in: .userDomainMask, appropriateFor: nil, create: false) as NSURL else {
            return false
        }
        do {
            try data.write(to: directory.appendingPathComponent("scanimg.png")!)
            uri = directory.appendingPathComponent("scanimg.png")?.absoluteString ?? ""
            if (uri == "") {
                return false
            }
            return true
        } catch {
            print(error.localizedDescription)
            return false
        }
    }
    
    func imageScannerControllerDidCancel(_ scanner: ImageScannerController) {
        let pluginResult = CDVPluginResult(status: CDVCommandStatus_ERROR, messageAs: "Cancelled");
        commandDelegate.send(pluginResult, callbackId:com.callbackId);
        scanner.dismiss(animated: true)
    }
    
    override func pluginInitialize() {
        super.pluginInitialize()
    }
}
