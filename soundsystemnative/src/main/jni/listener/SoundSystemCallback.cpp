#include "SoundSystemCallback.h"

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved) {
    _JVM = jvm;
    return JNI_VERSION_1_6;
}

SoundSystemCallback::SoundSystemCallback(JNIEnv *env, jclass jclass1) {
    _soundSystemInstance = (jclass) env->NewGlobalRef(jclass1);
    jclass test = env->GetObjectClass(jclass1);

    _playPauseMethodId = getMethodId(env, test, "notifyPlayingStatusObserversPlayPause", "(Z)V");
    _endTrackMethodId = getMethodId(env, test, "notifyPlayingStatusObserversEndTrack", "()V");
    _extractionCompleteMethodId = getMethodId(env, test, "notifyExtractionCompleted", "()V");
    _extractionStartedMethodId = getMethodId(env, test, "notifyExtractionStarted", "()V");
    _stopTrackMethodId = getMethodId(env, test, "notifyStopTrack", "()V");
}

jmethodID SoundSystemCallback::getMethodId(
        JNIEnv *env,
        jclass jclass1,
        char *methodName,
        char *sign) {
    jmethodID methodId = env->GetMethodID(jclass1, methodName, sign);
    if (methodId == 0) {
        LOGE("Method %s doesn't exist", methodName);
    }
    return methodId;
}

SoundSystemCallback::~SoundSystemCallback() {
    jint detachedStatus;
    JNIEnv *env = getEventCallbackEnvironnement(_JVM, &detachedStatus);
    env->DeleteGlobalRef(_soundSystemInstance);
    _soundSystemInstance = nullptr;
    _endTrackMethodId = nullptr;
}

void SoundSystemCallback::notifyPlayPause(bool play) {
    jint detachedStatus;
    JNIEnv *env = getEventCallbackEnvironnement(_JVM, &detachedStatus);

    jvalue value;
    value.z = (jboolean) play;
    env->CallVoidMethodA(_soundSystemInstance, _playPauseMethodId, &value);

    if (detachedStatus == JNI_EDETACHED) {
        _JVM->DetachCurrentThread();
    }
}

void SoundSystemCallback::notifyEndOfTrack() {
    jint detachedStatus;
    JNIEnv *env = getEventCallbackEnvironnement(_JVM, &detachedStatus);

    env->CallVoidMethod(_soundSystemInstance, _endTrackMethodId);

    if (detachedStatus == JNI_EDETACHED) {
        _JVM->DetachCurrentThread();
    }
}

void SoundSystemCallback::notifyExtractionCompleted() {
    jint detachedStatus;
    JNIEnv *env = getEventCallbackEnvironnement(_JVM, &detachedStatus);

    env->CallVoidMethod(_soundSystemInstance, _extractionCompleteMethodId);

    if (detachedStatus == JNI_EDETACHED) {
        _JVM->DetachCurrentThread();
    }
}

void SoundSystemCallback::notifyExtractionStarted() {
    jint detachedStatus;
    JNIEnv *env = getEventCallbackEnvironnement(_JVM, &detachedStatus);

    env->CallVoidMethod(_soundSystemInstance, _extractionStartedMethodId);

    if (detachedStatus == JNI_EDETACHED) {
        _JVM->DetachCurrentThread();
    }
}

void SoundSystemCallback::notifyStopTrack() {
    jint detachedStatus;
    JNIEnv *env = getEventCallbackEnvironnement(_JVM, &detachedStatus);

    env->CallVoidMethod(_soundSystemInstance, _stopTrackMethodId);

    if (detachedStatus == JNI_EDETACHED) {
        _JVM->DetachCurrentThread();
    }
}

JNIEnv *SoundSystemCallback::getEventCallbackEnvironnement(JavaVM *JVM, jint *detachedStatus) {
    JNIEnv *env;
    jint status = JVM->GetEnv((void **) &env, JNI_VERSION_1_6);
    *detachedStatus = status;
    if (status == JNI_EDETACHED) {
        status = JVM->AttachCurrentThread(&env, nullptr);
        if (status < 0) {
            return nullptr;
        }
    }
    return env;
}
