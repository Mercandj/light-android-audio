package com.mercandalli.android.sdk.audio;

import android.content.res.AssetManager;
import android.os.Process;

/**
 * Class used to communicate with the sound system.
 */
/* package */
class SoundSystemImpl implements SoundSystem {

    @SuppressWarnings("unused")
    private static final String TAG = "SoundSystem";

    /**
     * Load native library
     */
    static {
        System.loadLibrary("soundsystem");
        /*
        try {
            final String filename = Environment.getExternalStorageDirectory().getAbsolutePath() + "/printa/lib/arm64-v8a/libmylibrary.so";
            File file = new File(filename);
            File dst = new File("/data/data/com.mercandalli.android.apps.audio/libmylibrary.so");
            copyFile(file, dst);
            SecurityManager security = System.getSecurityManager();
            if (security != null) {
                security.checkLink(filename);
            }
            System.load(dst.getAbsolutePath());
        } catch (UnsatisfiedLinkError e) {
            Log.e("jm/debug", "SoundSystem UnsatisfiedLinkError", e);
            System.loadLibrary("mylibrary");
        } catch (IOException e) {
            Log.e("jm/debug", "SoundSystem IOException", e);
            System.loadLibrary("mylibrary");
        }
        System.loadLibrary("mylibrary");
        */
    }

    /*
    private static boolean copyFile(File src, File dst) throws IOException {
        if (src.getAbsolutePath().toString().equals(dst.getAbsolutePath().toString())) {

            return true;

        } else {
            InputStream is = new FileInputStream(src);
            OutputStream os = new FileOutputStream(dst);
            byte[] buff = new byte[1024];
            int len;
            while ((len = is.read(buff)) > 0) {
                os.write(buff, 0, len);
            }
            is.close();
            os.close();
        }
        return true;
    }
    */

    private final SoundSystemEntryPoint soundSystemEntryPoint;

    /**
     * Private constructor.
     */
    /* package */ SoundSystemImpl(SoundSystemEntryPoint soundSystemEntryPoint) {
        this.soundSystemEntryPoint = soundSystemEntryPoint;
    }

    /**
     * Initialize the sound system.
     *
     * @param nativeFrameRate    Native value tf the device frame rate
     * @param nativeFramesPerBuf Native value of the number of frames per buffer.
     */
    public void initSoundSystem(final int nativeFrameRate, final int nativeFramesPerBuf) {
        soundSystemEntryPoint.native_init_soundsystem(nativeFrameRate, nativeFramesPerBuf);
    }

    /**
     * Release native objects and this object.
     */
    public void release() {
        soundSystemEntryPoint.native_release_soundsystem();
    }

    public boolean isSoundSystemInit() {
        return soundSystemEntryPoint.native_is_soundsystem_init();
    }

    /**
     * Load track file into the RAM.
     *
     * @param filePath Path of the file on the hard disk.
     */
    public void loadFileOpenSl(final String filePath) {
        Preconditions.checkNotNull(filePath);
        soundSystemEntryPoint.native_load_file_open_sl(filePath);
    }

    /**
     * Load track file into the RAM.
     *
     * @param filePath Path of the file on the hard disk.
     */
    public void loadFileMediaCodec(final String filePath) {
        Preconditions.checkNotNull(filePath);
        soundSystemEntryPoint.native_load_file_media_codec(filePath);
    }

    /**
     * Load track file into the RAM.
     *
     * @param filePath Path of the file on the hard disk.
     */
    public void loadFileFFMPEGJavaThread(final String filePath) {
        Preconditions.checkNotNull(filePath);
        final Thread thread = new Thread("extractor-thread") {
            @Override
            public void run() {
                super.run();
                Process.setThreadPriority(Process.THREAD_PRIORITY_URGENT_AUDIO);
                soundSystemEntryPoint.native_load_file_synchronous_ffmpeg(filePath);
            }
        };
        thread.setPriority(Thread.MAX_PRIORITY);
        thread.start();
    }

    /**
     * Play music if params is true, otherwise, pause the music.
     *
     * @param isPlaying True if music is playing.
     */
    public void playMusic(final boolean isPlaying) {
        soundSystemEntryPoint.native_play(isPlaying);
    }

    /**
     * Stop music.
     */
    public void stopMusic() {
        soundSystemEntryPoint.native_stop();
    }

    /**
     * Get if a track is currently playing or not.
     *
     * @return True if a track is played.
     */
    public boolean isPlaying() {
        return soundSystemEntryPoint.native_is_playing();
    }

    /**
     * Get if a track has been loaded .
     *
     * @return True if a track is loaded.
     */
    public boolean isLoaded() {
        return soundSystemEntryPoint.native_is_loaded();
    }

    /**
     * Provide extracted data from audio file. Data[2n] represent one channel and Data[2n+1]
     * represente the other channel.
     *
     * @return A short array containing all extracted data from audio file.
     */
    public short[] getExtractedData() {
        return soundSystemEntryPoint.native_get_extracted_data();
    }

    /**
     * Provide mono data generate from extracted data which where in stereo mode.
     *
     * @return A shorrt array containing mono data of extracted audio file.
     */
    public short[] getExtractedDataMono() {
        return soundSystemEntryPoint.native_get_extracted_data_mono();
    }

    /**
     * Extract and directly play the audio file without step of extract the whole file into RAM.
     *
     * @param assetManager An {@link AssetManager}.
     * @param fileName     Name of the file to play which is inside Asset folder.
     */
    public void playSong(final AssetManager assetManager, final String fileName) {
        soundSystemEntryPoint.native_extract_from_assets_and_play(assetManager, fileName);
    }

    public boolean addPlayingStatusListener(PlayingStatusListener listener) {
        return soundSystemEntryPoint.addPlayingStatusListener(listener);
    }

    public boolean removePlayingStatusListener(PlayingStatusListener listener) {
        return soundSystemEntryPoint.removePlayingStatusListener(listener);
    }

    public boolean addExtractionListener(ExtractionListener listener) {
        return soundSystemEntryPoint.addExtractionListener(listener);
    }

    public boolean removeExtractionListener(ExtractionListener observer) {
        return soundSystemEntryPoint.removeExtractionListener(observer);
    }
}
