#include "SoundExtractorManager.h"

/**
 * Entry point of this lib.
 * @param deviceFrameRate The device frame rate. Provided by java.
 */
/* public */ SoundExtractorManager::SoundExtractorManager(
        const unsigned short deviceFrameRate,
        int openSLBufSize) :
        _deviceFrameRate(deviceFrameRate),
        _openSLBufSize(openSLBufSize),
        _androidVersion(androidVersionSdk()) {
}

/* public */ SoundExtractorManager::~SoundExtractorManager() {
    release();
    _soundExtractorListener = nullptr;
}

/* public */ void SoundExtractorManager::extract(
        int extractionMethod,
        const char **filePaths,
        int nbFiles) {
    release();

    _nbExtractionStarted = 0;
    _filePaths = filePaths;
    _nbFilesLeft = nbFiles;
    _nbFiles = nbFiles;
    _timeStart = currentTimeMillis();
    _extractorType = chooseExtractor(extractionMethod, _androidVersion);

    if (_soundExtractorListener != nullptr) {
        _soundExtractorListener->onSoundExtractorListenerStarted(filePaths);
    }
    _extractors = (Extractor **) malloc(nbFiles * sizeof(Extractor **));
    for (int i = 0; i < nbFiles; i++) {
        _extractors[i] = nullptr;
    }

    int nbThreadsToStart = nbFiles;
    if (_extractorType == EXTRACTION_METHOD_OPEN_SL) {
        nbThreadsToStart = (int) fmax(1, fmin(NB_THREADS_OPEN_SL_MAX, nbFiles));
    }
    startExtraction(nbThreadsToStart);
}

/* package */ void SoundExtractorManager::onExtractorListenerStarted(
        Extractor *extractor,
        const char *filePath) {
    if (_soundExtractorListener != nullptr) {
        _soundExtractorListener->onExtractorListenerStarted(filePath);
    }
}

/* package */ void SoundExtractorManager::onExtractorListenerSucceeded(
        Extractor *extractor,
        const char *filePath,
        short *writePos,
        size_t written) {
    LOGD("jm/debug onExtractorListenerSucceeded");
    char *pathOut = pathToDat(filePath);
    writeDat(pathOut, writePos, written);

    // Start another extraction if needed.
    _nbFilesLeft--;
    if (_nbFilesLeft > 0 && _nbExtractionStarted < _nbFiles) {
        startExtraction(1);
    }

    if (_soundExtractorListener != nullptr) {
        _soundExtractorListener->onExtractorListenerSucceeded(
                filePath,
                writePos,
                written);
    }

    // If the last extraction ended.
    if (_nbFilesLeft <= 0) {
        double durationMs = currentTimeMillis() - _timeStart;
        LOGD("jm/debug [SoundExtractorManager] durationMs:%lf", durationMs);

        if (_soundExtractorListener != nullptr) {
            _soundExtractorListener->onSoundExtractorListenerSucceeded(
                    _filePaths,
                    durationMs);
        }
    }

    // Be careful, will destroy the thread, so code after this method will not work
    delete extractor;
}

/* package */ void SoundExtractorManager::onExtractorListenerFailed(
        Extractor *extractor,
        const char *filePath,
        int errorCore) {
    if (_soundExtractorListener != nullptr) {
        _soundExtractorListener->onExtractorListenerFailed(filePath, errorCore);
    }
}

/* private */ void SoundExtractorManager::release() {
    for (int i = 0; i < _nbFiles; i++) {
        if (_extractors[i] != nullptr) {
            _extractors[i]->setExtractorListener(nullptr);
            delete _extractors[i];
        }
    }
    _nbFiles = 0;
    _nbFilesLeft = 0;
    _nbExtractionStarted = 0;
}

/* private */ int SoundExtractorManager::chooseExtractor(
        int extractionMethod,
        int androidVersion) {

    switch (extractionMethod) {
        case EXTRACTION_METHOD_AUTO:
#if __ANDROID_API__ >= 21
            if (androidVersion >= 21) {
                return EXTRACTION_METHOD_MEDIA_CODEC;
            }
            return EXTRACTION_METHOD_OPEN_SL;
#else
            return EXTRACTION_METHOD_OPEN_SL;
#endif

        case EXTRACTION_METHOD_OPEN_SL:
            return EXTRACTION_METHOD_OPEN_SL;

        case EXTRACTION_METHOD_MEDIA_CODEC:
            return EXTRACTION_METHOD_MEDIA_CODEC;

        case EXTRACTION_METHOD_FFMPEG:
            return EXTRACTION_METHOD_FFMPEG;

        default:
            LOGE("Unknown extractionMethod: %d", extractionMethod);
            throw;
    }
}

/* private */ Extractor *SoundExtractorManager::createExtractor(
        int extractor) {
    switch (extractor) {
        case EXTRACTION_METHOD_OPEN_SL:
            return new OpenSLExtractor(_deviceFrameRate, _openSLBufSize);

        case EXTRACTION_METHOD_MEDIA_CODEC:
#if __ANDROID_API__ >= 21
            return new MediaCodecExtractor(_deviceFrameRate);
#else
        LOGE("Media codec not supported");
        throw;
#endif

        case EXTRACTION_METHOD_FFMPEG:
            LOGE("EXTRACTION_METHOD_FFMPEG not implemented");
            throw;

        default:
            LOGE("Unknown extractor: %d", extractor);
            throw;
    }
}

/* private */ void SoundExtractorManager::startExtraction(int nbExtraction) {
    LOGD("jm/debug SoundExtractorManager startExtraction(%d)", nbExtraction);
    int loopEnd = _nbExtractionStarted + nbExtraction;
    int loopStart = _nbExtractionStarted;
    _nbExtractionStarted += nbExtraction;
    for (int i = loopStart; i < loopEnd; i++) {
        _extractors[i] = createExtractor(_extractorType);
        _extractors[i]->setFilePath(_filePaths[i]);
        _extractors[i]->setExtractorListener(this);
        _extractors[i]->extract();
    }
}