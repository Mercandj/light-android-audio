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
    private final List<OnInitListener> initializeListeners;
    private final AssetManager assetManager;
    private boolean initialized;
    private File fileAAC;
    private File fileMP3;
    private File fileWAV;
    @TrackFormat
    private String trackFormat;

    /* package */ FileManagerImpl(
            String fileDir,
            AssetManager assetManager) {
        this.initializeListeners = new ArrayList<>();
        this.assetManager = assetManager;
        this.fileDir = fileDir;
        this.initialized = false;

        trackFormat = FORMAT_MP3;
    }

    @Override
    public void initialize() {
        // Should be async...
        initialized = extractAsset(fileDir, FORMAT_AAC);
        initialized &= extractAsset(fileDir, FORMAT_MP3);
        initialized &= extractAsset(fileDir, FORMAT_WAV);
        if (!initialized) {
            throw new IllegalStateException("Cannot copy files from assets.");
        }
        for (OnInitListener listener : initializeListeners) {
            listener.onInitEnded();
        }
    }

    @Override
    public boolean isInitialized() {
        return initialized;
    }

    @Override
    public String getTrackFormat() {
        return trackFormat;
    }

    @Override
    public void setTrackFormat(@TrackFormat String format) {
        this.trackFormat = format;
    }

    @Override
    public File getFile() {
        switch (trackFormat) {
            case FORMAT_AAC:
                return fileAAC;
            case FORMAT_WAV:
                return fileWAV;
            case FORMAT_MP3:
            default:
                return fileMP3;
        }
    }

    @Override
    public void registerOnInitListener(OnInitListener listener) {
        if (listener == null || initializeListeners.contains(listener)) {
            return;
        }
        initializeListeners.add(listener);
    }

    @Override
    public void unregisterOnInitListener(OnInitListener listener) {
        initializeListeners.remove(listener);
    }

    private boolean extractAsset(String outDir, @TrackFormat String format) {
        InputStream in;
        OutputStream out;
        File file;
        try {
            switch (format) {
                case FORMAT_AAC:
                    in = assetManager.open(ASSET_FILE_NAME_AAC);
                    file = fileAAC = new File(outDir, FILE_NAME_AAC);
                    break;
                case FORMAT_WAV:
                    in = assetManager.open(ASSET_FILE_NAME_WAV);
                    file = fileWAV = new File(outDir, FILE_NAME_WAV);
                    break;
                case FORMAT_MP3:
                default:
                    in = assetManager.open(ASSET_FILE_NAME_MP3);
                    file = fileMP3 = new File(outDir, FILE_NAME_MP3);
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
