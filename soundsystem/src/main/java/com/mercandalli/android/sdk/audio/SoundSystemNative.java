package com.mercandalli.android.sdk.audio;

public class SoundSystemNative implements SoundSystem {

    public SoundSystemNative() {

    }

    @Override
    public boolean create() {
        return nativeCreate();
    }

    @Override
    public void destroy() {
        nativeDestroy();
    }

    @Override
    public boolean start() {
        return nativeStart();
    }

    @Override
    public void stop() {
        nativeStop();
    }

    /*
     * Loading Native lib(s)
     */
    static {
        System.loadLibrary("soundsystem");
    }

    public static native boolean nativeCreate();

    public static native void nativeDestroy();

    public static native boolean nativeStart();

    public static native void nativeStop();
}
