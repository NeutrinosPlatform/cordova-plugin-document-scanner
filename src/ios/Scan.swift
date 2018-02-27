import Foundation
import UIKit

@objc(Scan) class Scan: CDVPlugin {
    private var scan: ScanPlugin?

    override func pluginInitialize() { 
        super.pluginInitialize()
        self.scan = ScanPlugin()
    }

    func scanDoc(command: CDVInvokedUrlCommand) {
        let args = command.argument(at: 0)
        let pluginResult:CDVPluginResult;

        if(args != nil) {
            pluginResult = CDVPluginResult(status: CDVCommandStatus_OK)
        } else {
            pluginResult = CDVPluginResult(status: CDVCommandStatus_ERROR, messageAs:"Arg was null")
        }

        self.commandDelegate!.send(pluginResult, callbackId: command.callbackId)
    }


}