package com.mercandalli.android.apps.audio.file;

import android.content.res.AssetManager;

import java.io.File;

public interface FileManager {

    void initialize(String fileDir);

    boolean isInitialized();

    File getFile();

    void registerInitializeListener(InitializeListener listener);

    void unregisterInitializeListener(InitializeListener listener);

    interface InitializeListener {

        void onFileManagerInitialized();
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
