#ifndef OPEN_SL_EXTRACTOR_H
#define OPEN_SL_EXTRACTOR_H

#include <utils/android_debug.h>
#include <SoundExtractorManager.h>
#include <malloc.h>
#include <extractor/Extractor.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

class OpenSLExtractor : public Extractor {

public:
    OpenSLExtractor(
            const unsigned short deviceFrameRate,
            int openSLBufSize);

    ~OpenSLExtractor() override ;

    void extract() override;

private:
    int _bufferSize = 0;

    short *_soundBuffer = nullptr;

    SLObjectItf _engineObj = nullptr;
    SLEngineItf _engine = nullptr;

    SLObjectItf _extractPlayerObject = nullptr;
    SLPlayItf _extractPlayerPlay = nullptr;
    SLAndroidSimpleBufferQueueItf _extractPlayerBufferQueue = nullptr;
    SLMetadataExtractionItf _extractPlayerMetadata = nullptr;

    SLmillisecond _musicDuration = 0;
    unsigned int _totalFrames;
    bool _needExtractInitialisation = true;

    //extracted music
    short *_extractedData = nullptr;
    int _positionExtract = 0;

    SLDataLocator_URI *dataLocatorFromURLString(const char *urf8FileURLString);

    void sendSoundBufferExtract();

    void fillDataBuffer();

    static void queueExtractorCallback(SLAndroidSimpleBufferQueueItf aSoundQueue, void *aContext);

    static void extractionEndCallback(SLPlayItf caller, void *pContext, SLuint32 event);
};

#endif // OPEN_SL_EXTRACTOR_H