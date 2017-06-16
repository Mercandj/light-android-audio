package com.mercandalli.android.sdk.audio.listener;

import android.support.annotation.MainThread;

/**
 * Listener for the extraction state.
 */
public interface SSExtractionObserver {

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
