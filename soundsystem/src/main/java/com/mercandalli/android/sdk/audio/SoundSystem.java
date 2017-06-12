package com.mercandalli.android.sdk.audio;

@SuppressWarnings("WeakerAccess")
public interface SoundSystem {

    boolean create();

    void destroy();

    boolean start();

    void stop();
}
