package org.virgil.utils;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.Build;
import android.provider.ContactsContract;
import android.provider.MediaStore;
import android.provider.OpenableColumns;
import android.util.Log;

public class Utils
{
    public static String getDisplayName(Context context, String url)
    {
        String nativeFileName = null;

        Uri uri = Uri.parse(url);
        try {
            Cursor cursor = context.getContentResolver().query(uri, null, null, null, null);
            if (cursor != null && cursor.moveToFirst()) {
                nativeFileName = cursor.getString(cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME));
            }
        }
        catch (Exception e) {
            e.printStackTrace();
        }

        return (nativeFileName == null) ? "" : nativeFileName;
    }

    public static int getFileSize(Context context, String url)
    {
        int fileSize = 0;

        Uri uri = Uri.parse(url);
        try {
            Cursor cursor = context.getContentResolver().query(uri, null, null, null, null);
            if (cursor != null && cursor.moveToFirst()) {
                fileSize = cursor.getInt(cursor.getColumnIndex(OpenableColumns.SIZE));
            }
        }
        catch (Exception e) {
            e.printStackTrace();
        }

        return fileSize;
    }

    private static String getContactEmail(String contactId, ContentResolver cr)
    {
        String email = "";
        Cursor cursor = cr.query(
            ContactsContract.CommonDataKinds.Email.CONTENT_URI, null,
            ContactsContract.CommonDataKinds.Email.CONTACT_ID + " = ?", new String[]{ contactId }, null);
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
                ContactsContract.CommonDataKinds.Phone.CONTACT_ID + " = ?", new String[]{ contactId }, null);
            if (cp != null && cp.moveToFirst()) {
                phone = cp.getString(cp.getColumnIndex(ContactsContract.CommonDataKinds.Phone.NUMBER));
                cp.close();
            }
        }
        return phone;
    }

    private static String getContactPhotoUrl(Cursor cursor, ContentResolver cr)
    {
        String photo = cursor.getString(cursor.getColumnIndex(ContactsContract.CommonDataKinds.Phone.PHOTO_URI));
        if (photo != null) {
            // TODO(fpohtmeh): implement
            //Uri photoUri = Uri.parse(photo);
            //photo = getImageFilePath(photoUri, cr);
        }
        return photo;
    }

    private static String getImageFilePath(Uri uri, ContentResolver cr)
    {
        if (uri == null) {
            return null;
        }
        try (Cursor cursor = cr.query(uri, null, null, null, null)) {
            if (cursor != null && cursor.moveToFirst()) {
                return cursor.getString(cursor.getColumnIndex(MediaStore.Images.ImageColumns.DATA));
            }
        }
        return null;
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
                String id = cursor.getString(cursor.getColumnIndex(ContactsContract.Contacts._ID));
                String name = cursor.getString(cursor.getColumnIndex(DISPLAY_NAME));
                String email = getContactEmail(id, cr);
                if (email == null) {
                    email = "";
                }
                String phone = getContactPhone(id, cursor, cr);
                if (phone == null) {
                    phone = "";
                }
                String photo = getContactPhotoUrl(cursor, cr);
                if (photo == null) {
                    photo = "";
                }
                list +=
                    name + sep +
                    phone + sep +
                    email + sep +
                    photo + sep;
            } while (cursor.moveToNext());
            cursor.close();
        }

        return list;
    }
}
