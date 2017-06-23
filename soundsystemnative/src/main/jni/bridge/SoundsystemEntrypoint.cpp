#include "SoundsystemEntrypoint.h"

void Java_com_mercandalli_android_sdk_audio_SoundSystemEntryPoint_native_1init_1soundsystem(
        JNIEnv *env,
        jclass jclass1,
        jint sample_rate,
        jint frames_per_buf) {
    _soundSystemCallback = new SoundSystemCallback(env, jclass1);
    _soundSystem = new SoundSystem(_soundSystemCallback, sample_rate, frames_per_buf);
#ifdef MEDIACODEC_EXTRACTOR
    _mediaCodecSingleThreadExtractor = new MediaCodecSingleThreadExtractor(
            _soundSystem,
            sample_rate);
#endif
    _ffmpegSynchronousExtractor = new FFmpegSynchronousExtractor(
            _soundSystem,
            sample_rate);
    _ffmpegSingleThreadExtractor = new FFmpegSingleThreadExtractor(
            _soundSystem,
            sample_rate);

#ifdef AAUDIO
    _aaudio_manager = new AAudioManager();
    _aaudio_manager->createEngine(_soundSystem);
#endif
}

jboolean Java_com_mercandalli_android_sdk_audio_SoundSystemEntryPoint_native_1is_1soundsystem_1init(
        JNIEnv *env,
        jclass jclass1) {
    return (jboolean) isSoundSystemInit();
}

void Java_com_mercandalli_android_sdk_audio_SoundSystemEntryPoint_native_1load_1file_1open_1sl(
        JNIEnv *env,
        jclass jclass1,
        jstring filePath) {
    if (!isSoundSystemInit()) {
        return;
    }
    _soundSystem->extractMusic(dataLocatorFromURLString(env, filePath));
    _soundSystem->initAudioPlayer();
}

void Java_com_mercandalli_android_sdk_audio_SoundSystemEntryPoint_native_1load_1file_1media_1codec(
        JNIEnv *env,
        jclass jclass1,
        jstring filePath) {
    if (!isSoundSystemInit()) {
        return;
    }
#ifdef MEDIACODEC_EXTRACTOR
    const char *urf8FileURLString = env->GetStringUTFChars(filePath, NULL);
    _mediaCodecSingleThreadExtractor->extract(urf8FileURLString);
#else
    assert(false);
#endif
    _soundSystem->initAudioPlayer();
}

void Java_com_mercandalli_android_sdk_audio_SoundSystemEntryPoint_native_1load_1file_1ffmpeg(
        JNIEnv *env,
        jclass jclass1,
        jstring filePath) {
    if (!isSoundSystemInit()) {
        return;
    }
    const char *urf8FileURLString = env->GetStringUTFChars(filePath, NULL);
    _soundSystem->initAudioPlayer();
    _ffmpegSingleThreadExtractor->extract(urf8FileURLString);
}

void
Java_com_mercandalli_android_sdk_audio_SoundSystemEntryPoint_native_1load_1file_1ffmpeg_1synchronous(
        JNIEnv *env,
        jclass jclass1,
        jstring filePath) {
    if (!isSoundSystemInit()) {
        return;
    }
    const char *urf8FileURLString = env->GetStringUTFChars(filePath, NULL);
    _soundSystem->initAudioPlayer();
    _ffmpegSynchronousExtractor->extract(urf8FileURLString);
    _soundSystem->setIsLoaded(true);
    _soundSystem->notifyExtractionEnded();
}

void Java_com_mercandalli_android_sdk_audio_SoundSystemEntryPoint_native_1play(
        JNIEnv *env,
        jclass jclass1,
        jboolean play) {
    if (!isSoundSystemInit()) {
        return;
    }

#ifdef AAUDIO
    if (play) {
        _aaudio_manager->start();
    } else {
        _aaudio_manager->stop();
    }
#else
    _soundSystem->play(play);
#endif
}

jboolean Java_com_mercandalli_android_sdk_audio_SoundSystemEntryPoint_native_1is_1playing(
        JNIEnv *env,
        jclass jclass1) {
    if (!isSoundSystemInit()) {
        return JNI_FALSE;
    }
    return (jboolean) _soundSystem->isPlaying();
}

jboolean Java_com_mercandalli_android_sdk_audio_SoundSystemEntryPoint_native_1is_1loaded(
        JNIEnv *env,
        jclass jclass1) {
    if (!isSoundSystemInit()) {
        return JNI_FALSE;
    }
    return (jboolean) _soundSystem->isLoaded();
}

void Java_com_mercandalli_android_sdk_audio_SoundSystemEntryPoint_native_1stop(
        JNIEnv *env,
        jclass jclass1) {
    if (!isSoundSystemInit()) {
        return;
    }
    _soundSystem->stop();
#ifdef AAUDIO
    _aaudio_manager->deleteEngine();
#endif
}

void Java_com_mercandalli_android_sdk_audio_SoundSystemEntryPoint_native_1release_1soundsystem(
        JNIEnv *env,
        jclass jclass1) {
    if (_soundSystem != nullptr) {
        delete _soundSystem;
        _soundSystem = nullptr;
    }

#ifdef MEDIACODEC_EXTRACTOR
    if (_mediaCodecSingleThreadExtractor != nullptr) {
        delete _mediaCodecSingleThreadExtractor;
        _mediaCodecSingleThreadExtractor = nullptr;
    }
#endif

#ifdef AAUDIO
    if (_aaudio_manager != nullptr) {
        delete _aaudio_manager;
        _aaudio_manager = nullptr;
    }
#endif
}

jshortArray
Java_com_mercandalli_android_sdk_audio_SoundSystemEntryPoint_native_1get_1extracted_1data(
        JNIEnv *env,
        jclass jclass1) {
    if (!isSoundSystemInit()) {
        return nullptr;
    }
    unsigned int length = _soundSystem->getTotalNumberFrames() / 40;
    AUDIO_HARDWARE_SAMPLE_TYPE *tmpExtractedData = _soundSystem->getExtractedData();

    short *extractedData = tmpExtractedData;
    jshortArray jExtractedData = env->NewShortArray(length);
    if (jExtractedData == nullptr) {
        return nullptr;
    }
    env->SetShortArrayRegion(jExtractedData, 0, length, extractedData);
    return jExtractedData;
}

jshortArray
Java_com_mercandalli_android_sdk_audio_SoundSystemEntryPoint_native_1get_1extracted_1data_1mono(
        JNIEnv *env,
        jclass jclass1) {
    if (!isSoundSystemInit()) {
        return nullptr;
    }

    AUDIO_HARDWARE_SAMPLE_TYPE *tmpExtractedData = _soundSystem->getExtractedDataMono();
    unsigned int length = _soundSystem->getTotalNumberFrames() / 2;

    short *extractedData = tmpExtractedData;
    jshortArray jExtractedData = env->NewShortArray(length);
    if (jExtractedData == nullptr) {
        return nullptr;
    }
    env->SetShortArrayRegion(jExtractedData, 0, length, extractedData);
    return jExtractedData;
}

bool isSoundSystemInit() {
    if (_soundSystem == NULL) {
        LOGE("_soundSystem is not initialize");
        return false;
    }
    return true;
}

// Convert Java string to UTF-8
SLDataLocator_URI *dataLocatorFromURLString(JNIEnv *env, jstring fileURLString) {
    const char *urf8FileURLString = env->GetStringUTFChars(fileURLString, NULL);
    assert(NULL != urf8FileURLString);
    SLDataLocator_URI *fileLoc = (SLDataLocator_URI *) malloc(sizeof(SLDataLocator_URI));
    fileLoc->locatorType = SL_DATALOCATOR_URI;
    fileLoc->URI = (SLchar *) urf8FileURLString;
    return fileLoc;
}