#include "SoundsystemEntrypoint.h"

static double now_ms(void) {
    struct timespec res;
    clock_gettime(CLOCK_REALTIME, &res);
    return 1000.0 * res.tv_sec + (double) res.tv_nsec / 1e6;
}

void Java_com_mercandalli_android_sdk_audio_SoundSystem_native_1init_1soundsystem(
        JNIEnv *env,
        jclass jclass1,
        jint sample_rate,
        jint frames_per_buf) {
    _soundSystemCallback = new SoundSystemCallback(env, jclass1);
    _soundSystem = new SoundSystem(_soundSystemCallback, sample_rate, frames_per_buf);
#ifdef MEDIACODEC_EXTRACTOR
    _singleThreadNdkExtractor = new SingleThreadNdkExtractor(_soundSystem, sample_rate);
#endif
    _synchronousFfmpegExtractor = new SynchronousFfmpegExtractor(_soundSystem, sample_rate);

#ifdef AAUDIO
    _aaudio_manager = new AAudioManager();
    _aaudio_manager->createEngine(_soundSystem);
#endif
}

jboolean Java_com_mercandalli_android_sdk_audio_SoundSystem_native_1is_1soundsystem_1init(
        JNIEnv *env,
        jclass jclass1) {
    return (jboolean) isSoundSystemInit();
}

void Java_com_mercandalli_android_sdk_audio_SoundSystem_native_1load_1file(
        JNIEnv *env,
        jclass jclass1,
        jstring filePath) {
    if (!isSoundSystemInit()) {
        return;
    }
#ifdef MEDIACODEC_EXTRACTOR
    const char *urf8FileURLString = env->GetStringUTFChars(filePath, NULL);
    _singleThreadNdkExtractor->extract(urf8FileURLString);
#else
    _soundSystem->extractMusic(dataLocatorFromURLString(env, filePath));
#endif
    _soundSystem->initAudioPlayer();

    _soundSystem->initAudioPlayer();
}

void
Java_com_mercandalli_android_sdk_audio_SoundSystem_native_1load_1file_1with_1synchronous_1ffmpeg(
        JNIEnv *env,
        jclass jclass1,
        jstring filePath) {
    LOGD("jm/debug A");
    if (!isSoundSystemInit()) {
        return;
    }
    LOGD("jm/debug B");
    const char *urf8FileURLString = env->GetStringUTFChars(filePath, NULL);
    LOGD("jm/debug C");
    _soundSystem->initAudioPlayer();
    LOGD("jm/debug D");
    _synchronousFfmpegExtractor->extract(urf8FileURLString);

    const double extractionEndTime = now_ms();
    LOGI("Extraction ffmpeg duration %f",
         extractionEndTime - _soundSystem->getExtractionStartTime());
    _soundSystem->setIsLoaded(true);
    _soundSystem->notifyExtractionEnded();
}

void Java_com_mercandalli_android_sdk_audio_SoundSystem_native_1play(
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

jboolean Java_com_mercandalli_android_sdk_audio_SoundSystem_native_1is_1playing(
        JNIEnv *env,
        jclass jclass1) {
    if (!isSoundSystemInit()) {
        return JNI_FALSE;
    }
    return (jboolean) _soundSystem->isPlaying();
}

jboolean Java_com_mercandalli_android_sdk_audio_SoundSystem_native_1is_1loaded(
        JNIEnv *env,
        jclass jclass1) {
    if (!isSoundSystemInit()) {
        return JNI_FALSE;
    }
    return (jboolean) _soundSystem->isLoaded();
}

void Java_com_mercandalli_android_sdk_audio_SoundSystem_native_1stop(JNIEnv *env, jclass jclass1) {
    if (!isSoundSystemInit()) {
        return;
    }
    _soundSystem->stop();
#ifdef AAUDIO
    _aaudio_manager->deleteEngine();
#endif
}

void Java_com_mercandalli_android_sdk_audio_SoundSystem_native_1extract_1and_1play(
        JNIEnv *env,
        jobject obj,
        jstring filePath) {
    if (!isSoundSystemInit()) {
        return;
    }
    _soundSystem->extractAndPlayDirectly(dataLocatorFromURLString(env, filePath));
}

void Java_com_mercandalli_android_sdk_audio_SoundSystem_native_1extract_1from_1assets_1and_1play(
        JNIEnv *env,
        jobject obj,
        jobject assetManager,
        jstring filename) {
    if (!isSoundSystemInit()) {
        return;
    }
    SLDataLocator_AndroidFD locator = getTrackFromAsset(env, assetManager, filename);
    _soundSystem->extractAndPlayDirectly(&locator);
}

void Java_com_mercandalli_android_sdk_audio_SoundSystem_native_1release_1soundsystem(
        JNIEnv *env, jclass jclass1) {
    if (_soundSystem != nullptr) {
        delete _soundSystem;
        _soundSystem = nullptr;
    }

#ifdef MEDIACODEC_EXTRACTOR
    if (_singleThreadNdkExtractor != nullptr) {
        delete _singleThreadNdkExtractor;
        _singleThreadNdkExtractor = nullptr;
    }
#endif

#ifdef AAUDIO
    if (_aaudio_manager != nullptr) {
        delete _aaudio_manager;
        _aaudio_manager = nullptr;
    }
#endif
}

jshortArray Java_com_mercandalli_android_sdk_audio_SoundSystem_native_1get_1extracted_1data(
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

jshortArray Java_com_mercandalli_android_sdk_audio_SoundSystem_native_1get_1extracted_1data_1mono(
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

SLDataLocator_AndroidFD getTrackFromAsset(JNIEnv *env, jobject assetManager, jstring filename) {
    // convert Java string to UTF-8
    const char *utf8 = env->GetStringUTFChars(filename, NULL);
    assert(NULL != utf8);

    // use asset manager to open asset by filename
    AAssetManager *mgr = AAssetManager_fromJava(env, assetManager);
    assert(NULL != mgr);
    AAsset *asset = AAssetManager_open(mgr, utf8, AASSET_MODE_UNKNOWN);

    // release the Java string and UTF-8
    env->ReleaseStringUTFChars(filename, utf8);

    // open asset as file descriptor
    off_t start, length;
    int fd = AAsset_openFileDescriptor(asset, &start, &length);
    assert(0 <= fd);
    AAsset_close(asset);

    // configure audio source
    SLDataLocator_AndroidFD loc_fd = {SL_DATALOCATOR_ANDROIDFD, fd, start, length};
    return loc_fd;
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

void convertFloatDataToShort(float *src, unsigned int length, short *dst) {
    for (int i = 0; i < length; i++) {
        float tmp = src[i];
        if (tmp > 1.f) {
            tmp = 1.f;
        } else if (tmp < -1.f) {
            tmp = -1.f;
        }
        dst[i] = tmp * SHRT_MAX;
    }
}
