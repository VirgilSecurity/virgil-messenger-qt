package org.virgil.utils;

import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Build;
import android.provider.ContactsContract;
import android.provider.MediaStore;
import android.provider.OpenableColumns;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URI;
import java.net.URL;
import java.util.Set;
import java.util.TreeSet;

public class ContactUtils
{
    private static final String TAG = "ContactUtils";
    private static final String THUMBNAILS_DIR = "thumbnails";

    public static Set<String> findContactsByEmail(String email, ContentResolver cr)
    {
        Set<String> contactIds = new TreeSet<>();
        Cursor cursor = cr.query(
                ContactsContract.CommonDataKinds.Email.CONTENT_URI, null,
                ContactsContract.CommonDataKinds.Email.ADDRESS + " = ?", new String[]{email}, null);
        if (cursor != null) {
            for (cursor.moveToFirst(); !cursor.isAfterLast(); cursor.moveToNext()) {
                contactIds.add(cursor.getString(cursor.getColumnIndex(ContactsContract.CommonDataKinds.Email.CONTACT_ID)));
            }
            cursor.close();
        }
        return contactIds;
    }

    private static String getContactEmail(String contactId, ContentResolver cr)
    {
        String email = "";
        Cursor cursor = cr.query(
                ContactsContract.CommonDataKinds.Email.CONTENT_URI, null,
                ContactsContract.CommonDataKinds.Email.CONTACT_ID + " = ?", new String[]{contactId}, null);
        if (cursor != null && cursor.moveToFirst()) {
            email = cursor.getString(cursor.getColumnIndex(ContactsContract.CommonDataKinds.Email.DATA));
            cursor.close();
        }
        return email;
    }

    private static String getContactPhone(String contactId, Cursor cursor, ContentResolver cr)
    {
        String phone = "";
        Integer hasPhone = cursor.getInt(cursor.getColumnIndex(ContactsContract.Contacts.HAS_PHONE_NUMBER));
        if (hasPhone > 0) {
            Cursor cp = cr.query(ContactsContract.CommonDataKinds.Phone.CONTENT_URI, null,
                    ContactsContract.CommonDataKinds.Phone.CONTACT_ID + " = ?", new String[]{contactId}, null);
            if (cp != null && cp.moveToFirst()) {
                phone = cp.getString(cp.getColumnIndex(ContactsContract.CommonDataKinds.Phone.NUMBER));
                cp.close();
            }
        }
        return phone;
    }

    public static String getContactPhotoUrl(long contactId, boolean isThumbnail, Context context)
    {
        ContentResolver cr = context.getContentResolver();
        Uri contactUri = ContentUris.withAppendedId(ContactsContract.Contacts.CONTENT_URI, contactId);

        File thumbnailsDir = prepareThumbnailsDir(context);
        File photoFile = new File(thumbnailsDir, String.valueOf(contactId));

        if (thumbnailsDir != null) {
            if (isThumbnail) {
                // Thumbnails stored in a database
                Uri photoUri = Uri.withAppendedPath(contactUri, ContactsContract.Contacts.Photo.CONTENT_DIRECTORY);
                Cursor photoCursor = cr.query(photoUri, new String[]{ContactsContract.Contacts.Photo.PHOTO}, null, null, null);
                if (photoCursor != null) {
                    try (OutputStream os = new FileOutputStream(photoFile)) {
                        if (photoCursor.moveToFirst()) {
                            byte[] data = photoCursor.getBlob(0);
                            os.write(data);
                        }
                    } catch (IOException e) {
                        Log.e(TAG, "Can't write thumbnail to file: " + photoFile.getAbsolutePath(), e);
                        photoFile.delete();
                    } finally {
                        photoCursor.close();
                    }
                }
            } else {
                // Large photos stored in files
                Uri displayPhotoUri = Uri.withAppendedPath(contactUri, ContactsContract.Contacts.Photo.DISPLAY_PHOTO);
                try {
                    AssetFileDescriptor fd = cr.openAssetFileDescriptor(displayPhotoUri, "r");
                    try (InputStream is = fd.createInputStream();
                         OutputStream os = new FileOutputStream(photoFile)) {
                        // Copy image to cache directory
                        final byte[] buffer = new byte[1024 * 4];
                        int n = 0;
                        while (-1 != (n = is.read(buffer))) {
                            os.write(buffer, 0, n);
                        }
                    }
                } catch (IOException e) {
                    Log.e(TAG, "Can't write photo to file: " + photoFile.getAbsolutePath(), e);
                    photoFile.delete();
                }
            }
        }

        if (photoFile.exists()) {
            File pngFile = convertToPng(photoFile);
            if (pngFile != null) {
                photoFile.delete();
                photoFile = pngFile;
            }
        }

        return photoFile.getPath();
    }

    private static String notNullStr(String input)
    {
        return input == null ? "" : input;
    }

    public static String getContacts(Context context)
    {
        String list = "";
        final String sep = "\n";

        final String DISPLAY_NAME = Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB ?
            ContactsContract.Contacts.DISPLAY_NAME_PRIMARY : ContactsContract.Contacts.DISPLAY_NAME;
        final String[] PROJECTION = {
            ContactsContract.Contacts._ID,
            DISPLAY_NAME,
            ContactsContract.Contacts.HAS_PHONE_NUMBER,
            ContactsContract.CommonDataKinds.Phone.PHOTO_URI
        };

        ContentResolver cr = context.getContentResolver();
        Cursor cursor = cr.query(ContactsContract.Contacts.CONTENT_URI, PROJECTION, null, null, null);
        if (cursor != null && cursor.moveToFirst()) {
            do {
                int idColumnIndex = cursor.getColumnIndex(ContactsContract.Contacts._ID);
                String id = cursor.getString(idColumnIndex);
                String name = cursor.getString(cursor.getColumnIndex(DISPLAY_NAME));
                String email = notNullStr(getContactEmail(id, cr));
                //Log.i(TAG, "email " + email);
                String phone = notNullStr(getContactPhone(id, cursor, cr));
                //Log.i(TAG, "phone " + phone);
                //String photo = notNullStr(getContactPhotoUrl(cursor.getLong(idColumnIndex), true, context));
                String photo = "";
                //Log.i(TAG, "photo " + photo);
                list += name + sep + phone + sep + email + sep + photo + sep;
            } while (cursor.moveToNext());
            cursor.close();
        }

        return list;
    }

    private static File prepareThumbnailsDir(Context context)
    {
        // Prepare directory for thumbnails
        File thumbDir = new File(context.getCacheDir(), THUMBNAILS_DIR);
        boolean dirIsReady;
        if (thumbDir.exists()) {
            dirIsReady = thumbDir.isDirectory();
        } else {
            dirIsReady = thumbDir.mkdirs();
        }
        return dirIsReady ? thumbDir : null;
    }

    private static File convertToPng(File photoFile) {
        if (photoFile == null || !photoFile.exists()) {
            Log.e(TAG, "Input file is empty");
            return null;
        }
        try {
            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inPreferredConfig = Bitmap.Config.ARGB_8888;
            Bitmap bitmap = BitmapFactory.decodeFile(photoFile.getPath(), options);

            File pngFile = new File(photoFile.getPath()+ ".png");
            OutputStream os = new FileOutputStream(pngFile.getPath());
            bitmap.compress(Bitmap.CompressFormat.PNG, 100, os);
            return pngFile;
        } catch (Exception e) {
            Log.e(TAG, "Can't convert file to PNG: " + photoFile.getPath(), e);
        }

        return null;
    }
}
