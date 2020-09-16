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
    let qtController = UIViewController()

    @objc func printSome() {
        print("Print line System")
    }
    
    @objc func openImagePicker(qtView: UIView) {
        let imageInput = self.createPhotoInputItem(qtView: qtView);
        
        let qtController = qtView.window?.rootViewController;
        
        qtController?.present(imageInput.presentingController!, animated: true, completion: {
            var frameStart = CGRect();
            frameStart.size.width = qtView.frame.width
            frameStart.size.height = 0
            frameStart.origin.x = 0
            frameStart.origin.y = qtView.frame.height
            imageInput.inputView!.frame = frameStart;

            var frame = qtView.frame;
            let dy = frame.size.height / 3;
            frame.size.height -= dy;
            frame.origin.y = frame.origin.y + dy;

            UIView.animate(withDuration: 0.7,
                           delay: 0.0,
                           options: [.curveEaseInOut , .allowUserInteraction],
                           animations: {
                                imageInput.inputView!.frame = frame
            },
                           completion: { finished in
                            print("Bug moved left!")
            })
        })

        imageInput.photoInputHandler = { image in
            print(">>>>>")
        }
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
        
        func 
    }
    
    func createPhotoInputItem(qtView: UIView) -> UIPhotosChatInputItem {
//        let qtController = qtView.window?.rootViewController;
        
        var liveCamaraAppearence = LiveCameraCellAppearance.createDefaultAppearance()
        liveCamaraAppearence.backgroundColor = .appThemeForegroundColor
        let photosAppearence = PhotosInputViewAppearance(liveCameraCellAppearence: liveCamaraAppearence)
        let item = UIPhotosChatInputItem(presentingController: qtController,
                                         tabInputButtonAppearance: PhotosChatInputItem.createDefaultButtonAppearance(),
                                         inputViewAppearance: photosAppearence)
        qtController.view.addSubview(item.inputView!)
        
        return item
    }
}
