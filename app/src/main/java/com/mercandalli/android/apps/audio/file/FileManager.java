package com.mercandalli.android.apps.audio.file;

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
    @interface Format {
    }

    String FORMAT_AAC = "FORMAT_AAC";
    String FORMAT_MP3 = "FORMAT_MP3";
    String FORMAT_WAV = "FORMAT_WAV";

    void load(String fileDir, @Format String format);

    boolean isInitialized();

    File getFile();

    void registerOnLoadListener(OnLoadListener listener);

    void unregisterOnLoadListener(OnLoadListener listener);

    interface OnLoadListener {

        void onLoadEnded(@Format String format);
    }

    class Instance {

        private static FileManager sInstance;

        /**
         * Used to get an instance of this class. Will instantiate this class the first time this method
         * is called.
         *
         * @return Get the instance of this class.
         */
        public static FileManager getInstance(AssetManager assetManager) {
            if (sInstance == null) {
                sInstance = new FileManagerImpl(assetManager);
            }
            return sInstance;
        }
    }

}
