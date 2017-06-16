#ifndef TEST_SOUNDSYSTEM_SOUNDSYSTEM_ENTRYPOINT_H
#define TEST_SOUNDSYSTEM_SOUNDSYSTEM_ENTRYPOINT_H

#include <jni.h>
#include <utils/android_debug.h>
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <assert.h>

#include "audio/extractornougat/ExtractorNougat.h"
#include "aaudio/AAudioManager.h"
#include "audio/SoundSystem.h"
#include "listener/SoundSystemCallback.h"

static SoundSystem *_soundSystem;

static SoundSystemCallback *_soundSystemCallback;

#ifdef MEDIACODEC_EXTRACTOR
static ExtractorNougat *_extractorNougat;
#endif

#ifdef AAUDIO_API
static AAudioManager *_aaudio_manager;
#endif

extern "C" {

void Java_com_mercandalli_android_sdk_audio_SoundSystem_native_1init_1soundsystem(
        JNIEnv *env,
        jclass jclass1,
        jint sample_rate,
        jint frames_per_buf);

void Java_com_mercandalli_android_sdk_audio_SoundSystem_native_1load_1file(
        JNIEnv *env,
        jclass jclass1,
        jstring filePath);

void Java_com_mercandalli_android_sdk_audio_SoundSystem_native_1play(
        JNIEnv *env,
        jclass jclass1,
        jboolean play);

jboolean Java_com_mercandalli_android_sdk_audio_SoundSystem_native_1is_1playing(
        JNIEnv *env,
        jclass jclass1);

jboolean Java_com_mercandalli_android_sdk_audio_SoundSystem_native_1is_1loaded(
        JNIEnv *env,
        jclass jclass1);

jboolean Java_com_mercandalli_android_sdk_audio_SoundSystem_native_1is_1soundsystem_1init(
        JNIEnv *env,
        jclass jclass1);

void Java_com_mercandalli_android_sdk_audio_SoundSystem_native_1stop(
        JNIEnv *env,
        jclass jclass1);

void Java_com_mercandalli_android_sdk_audio_SoundSystem_native_1extract_1and_1play(
        JNIEnv *env,
        jobject obj,
        jstring filePath);

void Java_com_mercandalli_android_sdk_audio_SoundSystem_native_1release_1soundsystem(
        JNIEnv *env,
        jclass jclass1);

void Java_com_mercandalli_android_sdk_audio_SoundSystem_native_1extract_1from_1assets_1and_1play(
        JNIEnv *env,
        jobject obj,
        jobject assetManager,
        jstring filename);

jshortArray Java_com_mercandalli_android_sdk_audio_SoundSystem_native_1get_1extracted_1data(
        JNIEnv *env,
        jclass jclass1);

jshortArray Java_com_mercandalli_android_sdk_audio_SoundSystem_native_1get_1extracted_1data_1mono(
        JNIEnv *env,
        jclass jclass1);
}

bool isSoundSystemInit();

SLDataLocator_URI *dataLocatorFromURLString(JNIEnv *env, jstring fileURLString);

void convertFloatDataToShort(float *data, unsigned int length, short *dst);

SLDataLocator_AndroidFD getTrackFromAsset(JNIEnv *env, jobject assetManager, jstring filename);

#endif //TEST_SOUNDSYSTEM_SOUNDSYSTEM_ENTRYPOINT_H
