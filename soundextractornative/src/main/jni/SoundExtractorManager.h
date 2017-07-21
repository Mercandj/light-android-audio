#ifndef SOUND_EXTRACTOR_MANAGER_H
#define SOUND_EXTRACTOR_MANAGER_H

#include <pthread.h>
#include <assert.h>
#include <sys/types.h>
#include <cstdlib>
#include <cmath>
#include <utils/android_debug.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "SoundExtractorListener.h"
#include "utils/android_utils.h"
#include "utils/time_utils.h"
#include "extractor/Extractor.h"
#include "extractor/listener/LogExtractorListener.h"
#include "extractor/listener/FileExtractorListener.h"
#include "extractor/mediacodec/MediaCodecExtractor.h"
#include "extractor/opensl/OpenSLExtractor.h"

#define EXTRACTION_METHOD_AUTO 0
#define EXTRACTION_METHOD_OPEN_SL 1
#define EXTRACTION_METHOD_MEDIA_CODEC 2
#define EXTRACTION_METHOD_FFMPEG 3

#define NB_THREADS_OPEN_SL_MAX 1

class ExtractorListener;

class SoundExtractorManager : public ExtractorListener {

public:
    /* public */ SoundExtractorManager(
            const unsigned short deviceFrameRate,
            int openSLBufSize);

    /* public */ ~SoundExtractorManager();

    /**
     *
     * @param method One of this int
     *                  EXTRACTION_METHOD_AUTO 0
     *                  EXTRACTION_METHOD_OPEN_SL 1
     *                  EXTRACTION_METHOD_MEDIA_CODEC 2
     *                  EXTRACTION_METHOD_FFMPEG 3
     * @param filePaths Files paths
     * @param nbFiles Number of files to extract
     */
    /* public */ void extract(
            int extractionMethod,
            const char **filePaths,
            int nbFiles);

    /**
     *
     * @param soundExtractorListener
     */
    /* public */ void setSoundExtractorListener(
            SoundExtractorListener *soundExtractorListener) {
        _soundExtractorListener = soundExtractorListener;
    }

    /**
     * Do not use outside this lib.
     */
    /* package */ virtual void onExtractorListenerStarted(
            Extractor* extractor,
            const char *filePath) override;

    /**
     * Do not use outside this lib.
     */
    /* package */ virtual void onExtractorListenerSucceeded(
            Extractor* extractor,
            const char *filePath,
            short *writePos,
            size_t written) override;

    /**
     * Do not use outside this lib.
     */
    /* package */ virtual void onExtractorListenerFailed(
            Extractor* extractor,
            const char *filePath,
            int errorCore) override;

private:
    const unsigned short _deviceFrameRate = 0;
    int _openSLBufSize = 0;
    int _androidVersion = 1;
    int _extractorType = EXTRACTION_METHOD_AUTO;
    SoundExtractorListener *_soundExtractorListener = nullptr;
    Extractor **_extractors = 0;
    const char **_filePaths = nullptr;
    int _nbFilesLeft = 0;
    int _nbExtractionStarted = 0;
    int _nbFiles = 0;
    double _timeStart = 0;

    /* private */ void release();

    /* private */ int chooseExtractor(
            int extractionMethod,
            int androidVersion);

    /* private */ Extractor *createExtractor(
            int extractor);

    /* private */ void startExtraction(int nbExtraction);
};

#endif // SOUND_EXTRACTOR_MANAGER_H
