import Foundation
import ChattoAdditions

import UIKit

extension UIColor {
    static let appThemeBackgroundColor = UIColor.black
    static let appThemeForegroundColor = UIColor.black
}

class UIPhotosChatInputItem: PhotosChatInputItem {
    override var inputView: UIView? {
        if let subviews = super.inputView?.subviews {
            for subview in subviews {
                subview.backgroundColor = .appThemeBackgroundColor
            }
        }
        return super.inputView
    }
}

@objc class System : NSObject {
    
    var chatInputPresenter: BasicChatInputBarPresenter!

    @objc func printSome() {
        print("Print line System")
    }
    
    @objc func openImagePicker(qtView: UIView) {
        let imageInput = self.createPhotoInputItem(qtView: qtView);
        qtView.addSubview(imageInput.inputView!);
        var frame = qtView.frame;
        let dy = frame.size.height / 3;
        frame.size.height -= dy;
        frame.origin.y = frame.origin.y + dy;
        imageInput.inputView!.frame = frame;
    }
    
    class UIPhotosChatInputItem: PhotosChatInputItem {
        override var inputView: UIView? {
            if let subviews = super.inputView?.subviews {
                for subview in subviews {
                    subview.backgroundColor = .appThemeBackgroundColor
                }
            }
            return super.inputView
        }
    }
    
    func createPhotoInputItem(qtView: UIView) -> UIPhotosChatInputItem {
        let qtController = qtView.window?.rootViewController;
        print(qtController)
        print(qtView.frame.size.width)
        print(qtView.frame.size.height)
        
        var liveCamaraAppearence = LiveCameraCellAppearance.createDefaultAppearance()
        liveCamaraAppearence.backgroundColor = .appThemeForegroundColor
        let photosAppearence = PhotosInputViewAppearance(liveCameraCellAppearence: liveCamaraAppearence)
        let item = UIPhotosChatInputItem(presentingController: qtController,
                                         tabInputButtonAppearance: PhotosChatInputItem.createDefaultButtonAppearance(),
                                         inputViewAppearance: photosAppearence)

        item.photoInputHandler = { [weak self] image in
            print(image)
        }
        
        return item
    }
}
