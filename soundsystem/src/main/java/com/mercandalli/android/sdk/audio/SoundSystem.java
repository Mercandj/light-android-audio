package com.mercandalli.android.sdk.audio;

import android.content.res.AssetManager;
import android.os.Handler;
import android.os.Looper;
import android.os.Process;

import com.mercandalli.android.sdk.audio.listener.SSExtractionObserver;
import com.mercandalli.android.sdk.audio.listener.SSPlayingStatusObserver;

import java.util.ArrayList;
import java.util.List;


/**
 * Class used to communicate with the sound system.
 */
public class SoundSystem {

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

    /**
     * Private instance of this class.
     */
    private static SoundSystem sInstance;

    /**
     * Used to get an instance of this class. Will instantiate this class the first time this method
     * is called.
     *
     * @return Get the instance of this class.
     */
    public static SoundSystem getInstance() {
        if (sInstance == null) {
            sInstance = new SoundSystem(Looper.getMainLooper());
        }
        return sInstance;
    }

    /**
     * List of all playing status observer.
     */
    private final List<SSPlayingStatusObserver> mPlayingStatusObservers;

    /**
     * List of all observer listening for the extraction state.
     */
    private final List<SSExtractionObserver> mExtractionObservers;

    /**
     * Handler attach to the main thread.
     */
    private final Handler mMainHandler;

    /**
     * Private constructor.
     */
    private SoundSystem(final Looper mainThreadLooper) {
        mMainHandler = new Handler(mainThreadLooper);

        mPlayingStatusObservers = new ArrayList<>();
        mExtractionObservers = new ArrayList<>();
    }

    /**
     * Initialize the sound system.
     *
     * @param nativeFrameRate    Native value tf the device frame rate
     * @param nativeFramesPerBuf Native value of the number of frames per buffer.
     */
    public void initSoundSystem(final int nativeFrameRate, final int nativeFramesPerBuf) {
        native_init_soundsystem(nativeFrameRate, nativeFramesPerBuf);
    }

    /**
     * Release native objects and this object.
     */
    public void release() {
        native_release_soundsystem();
        sInstance = null;
    }

    public boolean isSoundSystemInit() {
        return false;//native_is_soundsystem_init();
    }

    /**
     * Load track file into the RAM.
     *
     * @param filePath Path of the file on the hard disk.
     */
    public void loadFile(final String filePath) {
        native_load_file(filePath);
    }

    /**
     * Load track file into the RAM.
     *
     * @param filePath Path of the file on the hard disk.
     */
    public void loadFileWithFfmpeg(final String filePath) {
        final Thread thread = new Thread("extractor-thread") {
            @Override
            public void run() {
                super.run();
                Process.setThreadPriority(Process.THREAD_PRIORITY_URGENT_AUDIO);
                native_load_file_with_synchronous_ffmpeg(filePath);
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
        native_play(isPlaying);
    }

    /**
     * Stop music.
     */
    public void stopMusic() {
        native_stop();
    }

    /**
     * Get if a track is currently playing or not.
     *
     * @return True if a track is played.
     */
    public boolean isPlaying() {
        return native_is_playing();
    }

    /**
     * Get if a track has been loaded .
     *
     * @return True if a track is loaded.
     */
    public boolean isLoaded() {
        return native_is_loaded();
    }

    /**
     * Provide extracted data from audio file. Data[2n] represent one channel and Data[2n+1]
     * represente the other channel.
     *
     * @return A short array containing all extracted data from audio file.
     */
    public short[] getExtractedData() {
        return native_get_extracted_data();
    }

    /**
     * Provide mono data generate from extracted data which where in stereo mode.
     *
     * @return A shorrt array containing mono data of extracted audio file.
     */
    public short[] getExtractedDataMono() {
        return native_get_extracted_data_mono();
    }

    /**
     * Extract and directly play the audio file without step of extract the whole file into RAM.
     *
     * @param assetManager An {@link AssetManager}.
     * @param fileName     Name of the file to play which is inside Asset folder.
     */
    public void playSong(final AssetManager assetManager, final String fileName) {
        native_extract_from_assets_and_play(assetManager, fileName);
    }

    //---------------
    // - Listeners -
    //---------------

    public boolean addPlayingStatusObserver(final SSPlayingStatusObserver observer) {
        synchronized (mPlayingStatusObservers) {
            //noinspection SimplifiableIfStatement
            if (observer == null || mPlayingStatusObservers.contains(observer)) {
                return false;
            }
            return mPlayingStatusObservers.add(observer);
        }
    }

    public boolean removePlayingStatusObserver(final SSPlayingStatusObserver observer) {
        synchronized (mPlayingStatusObservers) {
            return mPlayingStatusObservers.remove(observer);
        }
    }

    /**
     * Notify that the track is finished
     * Called from native code.
     */
    @SuppressWarnings("unused")
    public void notifyPlayingStatusObserversEndTrack() {
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                synchronized (mPlayingStatusObservers) {
                    for (final SSPlayingStatusObserver observer : mPlayingStatusObservers) {
                        observer.onEndOfMusic();
                    }
                }
            }
        });
    }

    /**
     * Notify of the new player state.
     * Called from native code.
     */
    @SuppressWarnings("unused")
    public void notifyPlayingStatusObserversPlayPause(final boolean isPlaying) {
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                synchronized (mPlayingStatusObservers) {
                    for (final SSPlayingStatusObserver observer : mPlayingStatusObservers) {
                        observer.onPlayingStatusDidChange(isPlaying);
                    }
                }
            }
        });
    }

    /**
     * Notify track has stopped.
     * Called from native code.
     */
    @SuppressWarnings("unused")
    public void notifyStopTrack() {
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                synchronized (mPlayingStatusObservers) {
                    for (final SSPlayingStatusObserver observer : mPlayingStatusObservers) {
                        observer.onStopTrack();
                    }
                }
            }
        });
    }

    public boolean addExtractionObserver(final SSExtractionObserver observer) {
        synchronized (mExtractionObservers) {
            //noinspection SimplifiableIfStatement
            if (observer == null || mExtractionObservers.contains(observer)) {
                return false;
            }
            return mExtractionObservers.add(observer);
        }
    }

    public boolean removeExtractionObserver(final SSExtractionObserver observer) {
        synchronized (mExtractionObservers) {
            return mExtractionObservers.remove(observer);
        }
    }

    /**
     * Notify that the extraction has started.
     * Called from native code.
     */
    @SuppressWarnings("unused")
    public void notifyExtractionStarted() {
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                synchronized (mExtractionObservers) {
                    for (final SSExtractionObserver observer : mExtractionObservers) {
                        observer.onExtractionStarted();
                    }
                }
            }
        });
    }

    /**
     * Notify that the extraction has finished.
     * Called from native code.
     */
    @SuppressWarnings("unused")
    public void notifyExtractionCompleted() {
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                synchronized (mExtractionObservers) {
                    for (final SSExtractionObserver observer : mExtractionObservers) {
                        observer.onExtractionCompleted();
                    }
                }
            }
        });
    }

    //--------------------
    // - Native methods -
    //--------------------
    private native void native_init_soundsystem(int nativeFrameRate, int nativeFramesPerBuf);

    private native boolean native_is_soundsystem_init();

    private native void native_load_file(String filePath);

    private native void native_load_file_with_synchronous_ffmpeg(String filePath);

    private native void native_release_soundsystem();

    private native void native_play(boolean play);

    private native boolean native_is_playing();

    private native boolean native_is_loaded();

    private native void native_stop();

    private native void native_extract_and_play(String filePath);

    private native void native_extract_from_assets_and_play(AssetManager assetManager, String filename);

    private native short[] native_get_extracted_data();

    private native short[] native_get_extracted_data_mono();
}
