package org.virgil.notification;

import java.util.UUID;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

import me.pushy.sdk.Pushy;

import com.virgilsecurity.android.virgil.R;

public class PushyMessageReceiver extends BroadcastReceiver {

    private static final String TAG = "PushyMessageReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d(TAG, "Try to decrypt Notification.");

        //
        //  Expect a data notification with a structure:
        //      {
        //          "recipient" : "Recipient XMPP JID",
        //          "sender" : "Sender XMPP JID",
        //          "ciphertext" : "Base64 encoded cipher text",
        //      }
        //
        String senderId = intent.getStringExtra("sender");
        String recipientId = intent.getStringExtra("recipient");
        String ciphertext = intent.getStringExtra("ciphertext");

        Bundle data = new Bundle();
        data.putString(Constants.PUSH.MESSAGE_ID, UUID.randomUUID().toString());
        data.putString(Constants.PUSH.SENDER_JID, senderId != null ? senderId : "");
        data.putString(Constants.PUSH.RECIPIENT_JID, recipientId != null ? recipientId : "");
        data.putString(Constants.PUSH.CIPHERTEXT, ciphertext != null ? ciphertext : "");

        final PendingResult pendingResult = goAsync();
        PushMessageHandler asyncTask = new PushMessageHandler(pendingResult, context, intent, data);
        asyncTask.execute();
   }
}
