#ifndef TEST_SOUNDSYSTEM_SOUNDSYSTEMCALLBACK_H
#define TEST_SOUNDSYSTEM_SOUNDSYSTEMCALLBACK_H

#include <jni.h>

#include <utils/android_debug.h>

static JavaVM *_JVM = nullptr;

class SoundSystemCallback {
public:
    SoundSystemCallback(JNIEnv *env, jclass jclass1);

    ~SoundSystemCallback();

    void notifyExtractionCompleted();

    void notifyExtractionStarted();

    void notifyEndOfTrack();

    void notifyStopTrack();

    void notifyPlayPause(bool play);

    JNIEnv *getEventCallbackEnvironnement(JavaVM *JVM, jint *detachedStatus);

private:
    jmethodID getMethodId(JNIEnv *env, jclass jclass1, char *methodName, char *sign);

    jclass _soundSystemInstance;
    jmethodID _endTrackMethodId;
    jmethodID _playPauseMethodId;
    jmethodID _extractionCompleteMethodId;
    jmethodID _extractionStartedMethodId;
    jmethodID _stopTrackMethodId;
};


#endif //TEST_SOUNDSYSTEM_SOUNDSYSTEMCALLBACK_H
