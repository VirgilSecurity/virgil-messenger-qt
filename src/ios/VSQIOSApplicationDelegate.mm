//  Copyright (C) 2015-2020 Virgil Security, Inc.
//
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//      (1) Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//      (2) Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in
//      the documentation and/or other materials provided with the
//      distribution.
//
//      (3) Neither the name of the copyright holder nor the names of its
//      contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
//  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
//  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
//  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//
//  Lead Maintainer: Virgil Security Inc. <support@virgilsecurity.com>

#import "VirgilMessenger-Swift.h"

#import <UIKit/UIKit.h>

#include <QtGui>
#include <QtGui/qpa/qplatformnativeinterface.h>

#include "ios/VSQIOSApplicationDelegate.h"


static System * sis = NULL;

//add a category to QIOSApplicationDelegate
@interface QIOSApplicationDelegate : UIResponder <UIApplicationDelegate>
@end

@implementation QIOSApplicationDelegate (NotificationDelegate)

- (BOOL)application:(UIApplication *)application
    didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    
    // This may be necessary to receive notifications
    [application registerUserNotificationSettings:[UIUserNotificationSettings settingsForTypes:(UIUserNotificationTypeSound | UIUserNotificationTypeAlert | UIUserNotificationTypeBadge) categories:nil]];

    [application registerForRemoteNotifications];
    NSLog(@"registered for remote notifications");
    return YES;
}

- (void)application:(UIApplication *)application didRegisterForRemoteNotificationsWithDeviceToken:(NSData *)deviceToken {
    NSLog(@"Did Register for Remote Notifications with Device Token (%@)", deviceToken);
//    NotificationHandler::GetInstance()->RegisterToken(deviceToken.bytes, deviceToken.length);
}

- (void)application:(UIApplication *)application didFailToRegisterForRemoteNotificationsWithError:(NSError *)error {
    NSLog(@"Did Fail to Register for Remote Notifications");
    NSLog(@"%@, %@", error, error.localizedDescription);

}

@end

/******************************************************************************/
QString
iosShowImagePicker() {
    if (!sis) {
        sis = [[System alloc] init];
    }
    
    QWindow* m = NULL;
    QWindow* m_last;
    foreach(QWindow *w, QGuiApplication::topLevelWindows()) {
        m_last = w;
        if (w->inherits("QMainWindow")) {
            m = w; break;
        }
    }
    
    if (!m) {
        m = m_last;
    }

    UIView *view = static_cast<UIView *>(QGuiApplication::platformNativeInterface()->nativeResourceForWindow("uiview", m));
    
    [sis openImagePickerWithQtView:view];

    return "";
}

/******************************************************************************/
