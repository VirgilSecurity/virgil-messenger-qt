package org.virgil.utils;

import org.qtproject.qt5.android.QtNative;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;

public class DocumentInteraction
{
    private static final String TAG = "DocumentInteraction";

    public static void viewFile(Context context, String filePath) {
        Uri uri = Uri.parse(filePath);
        Intent viewIntent = new Intent(Intent.ACTION_VIEW, uri);
        QtNative.activity().startActivity(viewIntent);
    }
}
