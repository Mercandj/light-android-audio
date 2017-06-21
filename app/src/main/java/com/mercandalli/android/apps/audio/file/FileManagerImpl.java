package com.mercandalli.android.apps.audio.file;

import android.content.res.AssetManager;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;

/* package */ class FileManagerImpl implements FileManager {

    private static final String ASSET_FILE_NAME_AAC = "over_the_horizon.aac";
    private static final String ASSET_FILE_NAME_MP3 = "over_the_horizon.mp3";
    private static final String ASSET_FILE_NAME_WAV = "over_the_horizon.wav";

    private static final String FILE_NAME_AAC = "over_the_horizon.aac";
    private static final String FILE_NAME_MP3 = "over_the_horizon.mp3";
    private static final String FILE_NAME_WAV = "over_the_horizon.wav";

    private String fileDir;
    private final List<OnLoadListener> initializeListeners;
    private final AssetManager assetManager;
    private boolean initialized;
    private File file;

    /* package */ FileManagerImpl(AssetManager assetManager) {
        this.initializeListeners = new ArrayList<>();
        this.assetManager = assetManager;
    }

    @Override
    public void load(String fileDir, @Format String format) {
        this.fileDir = fileDir;
        // Should be async...
        initialized = extractAsset(fileDir, format);
        for (OnLoadListener listener : initializeListeners) {
            listener.onLoadEnded(format);
        }
    }

    @Override
    @SuppressWarnings("SimplifiableIfStatement")
    public boolean isInitialized() {
        if (fileDir == null || file == null) {
            return false;
        }
        return initialized;
    }

    @Override
    public File getFile() {
        return file;
    }

    @Override
    public void registerOnLoadListener(OnLoadListener listener) {
        if (listener == null || initializeListeners.contains(listener)) {
            return;
        }
        initializeListeners.add(listener);
    }

    @Override
    public void unregisterOnLoadListener(OnLoadListener listener) {
        initializeListeners.remove(listener);
    }

    private boolean extractAsset(String outDir, @Format String format) {
        InputStream in;
        OutputStream out;
        try {
            switch (format) {
                case FORMAT_AAC:
                    in = assetManager.open(ASSET_FILE_NAME_AAC);
                    file = new File(outDir, FILE_NAME_AAC);
                    break;
                case FORMAT_MP3:
                    in = assetManager.open(ASSET_FILE_NAME_MP3);
                    file = new File(outDir, FILE_NAME_MP3);
                    break;
                case FORMAT_WAV:
                    in = assetManager.open(ASSET_FILE_NAME_WAV);
                    file = new File(outDir, FILE_NAME_WAV);
                    break;
                default:
                    in = assetManager.open(ASSET_FILE_NAME_MP3);
                    file = new File(outDir, FILE_NAME_MP3);
                    break;
            }

            if (file.exists()) {
                return true;
            }
            out = new FileOutputStream(file);
            copyFile(in, out);
            in.close();
            out.flush();
            out.close();
            return true;
        } catch (IOException ignored) {
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
