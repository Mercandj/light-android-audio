#ifndef TEST_SOUNDSYSTEM_SOUNDSYSTEM_ENTRYPOINT_H
#define TEST_SOUNDSYSTEM_SOUNDSYSTEM_ENTRYPOINT_H

#include <jni.h>
#include <utils/android_debug.h>
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <assert.h>
#include <audio/extractor/FFmpegSynchronousExtractor.h>
#include <audio/extractor/FFmpegSingleThreadExtractor.h>

#include "audio/extractor/MediaCodecSingleThreadExtractor.h"
#include "SoundExtractorManager.h"

#include "audio/SoundSystem.h"
#include "listener/SoundSystemCallback.h"

#ifdef AAUDIO
#include "aaudio/AAudioManager.h"
#endif

static SoundSystem *_soundSystem;
static SoundExtractorManager *_soundExtractorManager;

static SoundSystemCallback *_soundSystemCallback;

#ifdef MEDIACODEC_EXTRACTOR
static MediaCodecSingleThreadExtractor *_mediaCodecSingleThreadExtractor;
#endif

static FFmpegSynchronousExtractor *_ffmpegSynchronousExtractor;
static FFmpegSingleThreadExtractor *_ffmpegSingleThreadExtractor;

#ifdef AAUDIO
static AAudioManager *_aaudio_manager;
#endif

extern "C" {

void Java_com_mercandalli_android_sdk_audio_SoundSystemEntryPoint_native_1init_1soundsystem(
        JNIEnv *env,
        jclass jclass1,
        jint sample_rate,
        jint frames_per_buf);

void Java_com_mercandalli_android_sdk_audio_SoundSystemEntryPoint_native_1extraction_1wrapper(
        JNIEnv *env,
        jclass jclass1,
        jobjectArray filePaths);

void Java_com_mercandalli_android_sdk_audio_SoundSystemEntryPoint_native_1load_1file_1open_1sl(
        JNIEnv *env,
        jclass jclass1,
        jstring filePath);

void Java_com_mercandalli_android_sdk_audio_SoundSystemEntryPoint_native_1load_1file_1media_1codec(
        JNIEnv *env,
        jclass jclass1,
        jstring filePath);

void Java_com_mercandalli_android_sdk_audio_SoundSystemEntryPoint_native_1load_1file_1ffmpeg(
        JNIEnv *env,
        jclass jclass1,
        jstring filePath);

void
Java_com_mercandalli_android_sdk_audio_SoundSystemEntryPoint_native_1load_1file_1ffmpeg_1synchronous(
        JNIEnv *env,
        jclass jclass1,
        jstring filePath);

void Java_com_mercandalli_android_sdk_audio_SoundSystemEntryPoint_native_1play(
        JNIEnv *env,
        jclass jclass1,
        jboolean play);

jboolean Java_com_mercandalli_android_sdk_audio_SoundSystemEntryPoint_native_1is_1playing(
        JNIEnv *env,
        jclass jclass1);

jboolean Java_com_mercandalli_android_sdk_audio_SoundSystemEntryPoint_native_1is_1loaded(
        JNIEnv *env,
        jclass jclass1);

jboolean Java_com_mercandalli_android_sdk_audio_SoundSystemEntryPoint_native_1is_1soundsystem_1init(
        JNIEnv *env,
        jclass jclass1);

void Java_com_mercandalli_android_sdk_audio_SoundSystemEntryPoint_native_1stop(
        JNIEnv *env,
        jclass jclass1);

void Java_com_mercandalli_android_sdk_audio_SoundSystemEntryPoint_native_1release_1soundsystem(
        JNIEnv *env,
        jclass jclass1);

jshortArray
Java_com_mercandalli_android_sdk_audio_SoundSystemEntryPoint_native_1get_1extracted_1data(
        JNIEnv *env,
        jclass jclass1);

jshortArray
Java_com_mercandalli_android_sdk_audio_SoundSystemEntryPoint_native_1get_1extracted_1data_1mono(
        JNIEnv *env,
        jclass jclass1);
}

bool isSoundSystemInit();

SLDataLocator_URI *dataLocatorFromURLString(JNIEnv *env, jstring fileURLString);

#endif //TEST_SOUNDSYSTEM_SOUNDSYSTEM_ENTRYPOINT_H
