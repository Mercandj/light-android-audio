package com.mercandalli.android.apps.audio.file;

import android.content.Context;
import android.content.res.AssetManager;
import android.support.annotation.StringDef;

import java.io.File;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

public interface FileManager {

    @Retention(RetentionPolicy.SOURCE)
    @StringDef({
            FORMAT_AAC,
            FORMAT_MP3,
            FORMAT_WAV})
    @interface TrackFormat {
    }

    String FORMAT_AAC = "FORMAT_AAC";
    String FORMAT_MP3 = "FORMAT_MP3";
    String FORMAT_WAV = "FORMAT_WAV";

    void initialize();

    boolean isInitialized();

    @TrackFormat
    String getTrackFormat();

    void setTrackFormat(@TrackFormat String format);

    File getFile();

    void registerOnInitListener(OnInitListener listener);

    void unregisterOnInitListener(OnInitListener listener);

    interface OnInitListener {

        void onInitEnded();
    }

    class Instance {

        private static FileManager sInstance;

        /**
         * Used to get an instance of this class. Will instantiate this class the first time this method
         * is called.
         *
         * @return Get the instance of this class.
         */
        public static FileManager getInstance(Context context) {
            if (sInstance == null) {
                AssetManager assetManager = context.getAssets();
                String folderPath = context.getFilesDir().getAbsolutePath();
                sInstance = new FileManagerImpl(
                        folderPath,
                        assetManager);
            }
            return sInstance;
        }
    }
}
