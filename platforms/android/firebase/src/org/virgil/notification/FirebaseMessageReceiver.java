package org.virgil.notification;

import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.util.Log;

import com.google.firebase.messaging.FirebaseMessagingService;
import com.google.firebase.messaging.RemoteMessage;

import com.virgilsecurity.android.virgil.R;
import org.virgil.notification.PushService;

public class FirebaseMessageReceiver extends FirebaseMessagingService {

    private static final String TAG = "FirebaseMessageReceiver";

    @Override
    public void onCreate() {
        super.onCreate();
    }

    /**
     * There are two scenarios when onNewToken is called:
     * 1) When a new token is generated on initial app startup
     * 2) Whenever an existing token is changed
     * Under #2, there are three scenarios when the existing token is changed:
     * A) App is restored to a new device
     * B) User uninstalls/reinstalls the app
     * C) User clears app data
     */
    @Override
    public void onNewToken(String token) {
        Log.d(TAG, "Refreshed token: " + token);

        PushService.updatePushToken(token);
    }

    // Override onMessageReceived() method to extract the
    // title and
    // body from the message passed in FCM
    @Override
    public void
    onMessageReceived(RemoteMessage remoteMessage) {
        Log.d(TAG, "Try to decrypt Notification.");

        //
        //  Expect a data notification with a structure:
        //      {
        //          "recipient" : "Recipient XMPP JID",
        //          "sender" : "Sender XMPP JID",
        //          "ciphertext" : "Base64 encoded cipher text",
        //      }
        //
        if (remoteMessage.getData().isEmpty()) {
            Log.w(TAG, "Data is empty.");
            return;
        }

        String senderJid = remoteMessage.getData().getOrDefault("sender", "");
        String recipientJid = remoteMessage.getData().getOrDefault("recipient", "");
        String ciphertext = remoteMessage.getData().getOrDefault("ciphertext", "");

        //
        //  Pack push message and send it to the push handler service.
        //
        Intent intent = new Intent(this, PushHandlerService.class);
        intent.setAction(Constants.PUSH.HANDLE_PUSH_ACTION);
        intent.addFlags(Intent.FLAG_INCLUDE_STOPPED_PACKAGES);
        intent.putExtra(Constants.PUSH.MESSAGE_ID, remoteMessage.getMessageId());
        intent.putExtra(Constants.PUSH.SENDER_JID, senderJid);
        intent.putExtra(Constants.PUSH.RECIPIENT_JID, recipientJid);
        intent.putExtra(Constants.PUSH.CIPHERTEXT, ciphertext);

        //
        //  Note, the message priority has to be set to high to allow the app to launch the background service intent!
        //
        Log.d(TAG, "Starting PushHandlerService...");
        startService(intent);
    }
}
