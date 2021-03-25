package org.virgil.utils;

import org.qtproject.qt5.android.QtNative;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;
import androidx.core.content.FileProvider;
import java.io.File;

public class DocumentInteraction
{
    private static final String TAG = "DocumentInteraction";

    public static void viewFile(Context context, String filePath) {
        File file = new File(filePath);
        try {
            Uri uri = FileProvider.getUriForFile(context, "com.virgilsecurity.android.virgil.provider", file);
            Intent viewIntent = new Intent(Intent.ACTION_VIEW, uri);
            QtNative.activity().startActivity(viewIntent);
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "The selected file can't be opened: " + file.toString());
            Log.e(TAG, "Error: " + e.toString());
        }
    }
}
