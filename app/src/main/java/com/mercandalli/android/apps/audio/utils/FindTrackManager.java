package com.mercandalli.android.apps.audio.utils;

import android.content.Context;
import android.content.CursorLoader;
import android.database.Cursor;
import android.provider.MediaStore;
import android.support.annotation.NonNull;

/**
 * Use to get, one by one, information on available music files on the device.
 */
public class FindTrackManager {

    private static int mCount = 0;

    public static Track getTrackPath(@NonNull final Context context) {
        String[] projMusics = {
                MediaStore.Audio.Media._ID,
                MediaStore.Audio.Media.ARTIST,
                MediaStore.Audio.Media.DISPLAY_NAME,
                MediaStore.Audio.Media.TITLE,
                MediaStore.Audio.Media.DATA};

        String whereArgs[] = new String[2];
        whereArgs[0] = "0";
        whereArgs[1] = "10000";

        final CursorLoader cursorLoaderMusicExter = new CursorLoader(
                context,
                MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                projMusics,
                MediaStore.Audio.Media.IS_MUSIC + "!= ? AND " + MediaStore.Audio.Media.DURATION + " > ? ",
                whereArgs,
                MediaStore.Audio.Media.TITLE_KEY + " ASC");
        final Cursor musicCursorExtern = cursorLoaderMusicExter.loadInBackground();

        final int numberMusicFiles = musicCursorExtern.getCount();
        if (numberMusicFiles == 0) {
            return null;
        } else if (numberMusicFiles == mCount - 1) {
            mCount = -1;
        }

        mCount++;
        musicCursorExtern.move(mCount);

        return new Track(musicCursorExtern.getLong(0),
                musicCursorExtern.getString(1),
                musicCursorExtern.getString(2),
                musicCursorExtern.getString(3),
                musicCursorExtern.getString(4));
    }
}
