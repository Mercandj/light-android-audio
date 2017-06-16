package com.mercandalli.android.sdk.audio.listener;

import android.support.annotation.MainThread;

/**
 * Listener for the status of the player
 */
public interface SSPlayingStatusObserver {

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
