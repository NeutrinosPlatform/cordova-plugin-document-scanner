import UIKit

@objc public class ScanPlugin: UIViewController {
    var dcsView:DcsView!

    override public func viewDidLoad() { 
        super.viewDidLoad()
        dcsView = DcsView.self.init(frame:CGRect.init(x: 0, y: 0, width: self.view.frame.size.width, height: self.view.frame.size.height))
        view.addSubview(dcsView)
    }
}