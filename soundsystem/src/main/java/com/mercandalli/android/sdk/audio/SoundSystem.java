package com.mercandalli.android.sdk.audio;

import android.content.res.AssetManager;
import android.os.Looper;
import android.support.annotation.MainThread;

/**
 * Class used to communicate with the sound system.
 */
public interface SoundSystem {

    class Instance {

        private static SoundSystem sInstance;

        /**
         * Used to get an instance of this class. Will instantiate this class the first time this method
         * is called.
         *
         * @return Get the instance of this class.
         */
        public static SoundSystem getInstance() {
            if (sInstance == null) {
                SoundSystemEntryPoint soundSystemEntryPoint = new SoundSystemEntryPoint(Looper.getMainLooper());
                sInstance = new SoundSystemImpl(soundSystemEntryPoint);
            }
            return sInstance;
        }
    }

    /**
     * Initialize the sound system.
     *
     * @param nativeFrameRate    Native value tf the device frame rate
     * @param nativeFramesPerBuf Native value of the number of frames per buffer.
     */
    void initSoundSystem(final int nativeFrameRate, final int nativeFramesPerBuf);

    /**
     * Release native objects and this object.
     */
    void release();

    boolean isSoundSystemInit();

    /**
     * Load track file into the RAM.
     *
     * @param filePath Path of the file on the hard disk.
     */
    void loadFile(final String filePath);

    /**
     * Load track file into the RAM.
     *
     * @param filePath Path of the file on the hard disk.
     */
    void loadFileWithFfmpeg(final String filePath);

    /**
     * Play music if params is true, otherwise, pause the music.
     *
     * @param isPlaying True if music is playing.
     */
    void playMusic(final boolean isPlaying);

    /**
     * Stop music.
     */
    void stopMusic();

    /**
     * Get if a track is currently playing or not.
     *
     * @return True if a track is played.
     */
    boolean isPlaying();

    /**
     * Get if a track has been loaded .
     *
     * @return True if a track is loaded.
     */
    boolean isLoaded();

    /**
     * Provide extracted data from audio file. Data[2n] represent one channel and Data[2n+1]
     * represente the other channel.
     *
     * @return A short array containing all extracted data from audio file.
     */
    short[] getExtractedData();

    /**
     * Provide mono data generate from extracted data which where in stereo mode.
     *
     * @return A shorrt array containing mono data of extracted audio file.
     */
    short[] getExtractedDataMono();

    /**
     * Extract and directly play the audio file without step of extract the whole file into RAM.
     *
     * @param assetManager An {@link AssetManager}.
     * @param fileName     Name of the file to play which is inside Asset folder.
     */
    void playSong(final AssetManager assetManager, final String fileName);

    boolean addPlayingStatusListener(final PlayingStatusListener observer);

    boolean removePlayingStatusListener(final PlayingStatusListener observer);

    boolean addExtractionListener(final ExtractionListener observer);

    boolean removeExtractionListener(final ExtractionListener observer);

    /**
     * Listener for the status of the player
     */
    interface PlayingStatusListener {

        /**
         * Callback to notify when player state change beetwenn "play" and "pause".
         * @param isPlaying True if we want to play the extracted music.
         */
        @MainThread
        void onPlayingStatusDidChange(boolean isPlaying);

        /**
         * Callback to notify when music end.
         */
        @MainThread
        void onEndOfMusic();

        /**
         * Callback to notify when music is stopped.
         */
        @MainThread
        void onStopTrack();
    }

    /**
     * Listener for the extraction state.
     */
    interface ExtractionListener {

        /**
         * Callback for the moment where the extraction start.
         */
        @MainThread
        void onExtractionStarted();

        /**
         * Callback for the moment where the extraction end.
         */
        @MainThread
        void onExtractionCompleted();

    }
}
