package com.mercandalli.android.apps.audio.file;

import android.content.res.AssetManager;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;

/* package */ class FileManagerImpl implements FileManager {

    private static final String ASSET_FILE_NAME = "over_the_horizon.mp3";
    private static final String FILE_NAME = "over_the_horizon.mp3";

    private String fileDir;
    private final List<InitializeListener> initializeListeners;
    private final AssetManager assetManager;
    private boolean initialized;
    private File file;

    /* package */ FileManagerImpl(AssetManager assetManager) {
        this.initializeListeners = new ArrayList<>();
        this.assetManager = assetManager;
    }

    @Override
    public void initialize(String fileDir) {
        this.fileDir = fileDir;
        // Should be async...
        initialized = extractAsset(fileDir);
        for (InitializeListener listener : initializeListeners) {
            listener.onFileManagerInitialized();
        }
    }

    @Override
    @SuppressWarnings("SimplifiableIfStatement")
    public boolean isInitialized() {
        if (fileDir == null || file ==null) {
            return false;
        }
        return initialized;
    }

    @Override
    public File getFile() {
        return file;
    }

    @Override
    public void registerInitializeListener(InitializeListener listener) {
        if (listener == null || initializeListeners.contains(listener)) {
            return;
        }
        initializeListeners.add(listener);
    }

    @Override
    public void unregisterInitializeListener(InitializeListener listener) {
        initializeListeners.remove(listener);
    }

    private boolean extractAsset(String outDir) {
        InputStream in = null;
        OutputStream out = null;
        try {
            in = assetManager.open(ASSET_FILE_NAME);
            file = new File(outDir, FILE_NAME);
            if (file.exists()) {
                return true;
            }
            out = new FileOutputStream(file);
            copyFile(in, out);
            in.close();
            in = null;
            out.flush();
            out.close();
            out = null;
            return true;
        } catch (IOException e) {
            Log.e("tag", "Failed to copy asset file: " + ASSET_FILE_NAME, e);
        }
        return false;
    }

    private void copyFile(InputStream in, OutputStream out) throws IOException {
        byte[] buffer = new byte[1024];
        int read;
        while ((read = in.read(buffer)) != -1) {
            out.write(buffer, 0, read);
        }
    }
}
