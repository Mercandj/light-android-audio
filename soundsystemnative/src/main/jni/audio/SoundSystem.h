#ifndef TEST_SOUNDSYSTEM_SOUNDSYSTEM_H
#define TEST_SOUNDSYSTEM_SOUNDSYSTEM_H

#define SLASSERT(x) assert(x == SL_RESULT_SUCCESS)

// Include OpenSLES
#include <SLES/OpenSLES.h>

// Include OpenSL ES android extensions
#include <SLES/OpenSLES_Android.h>

// C++ header
#include <assert.h>

// Provide memmove
#include <cstring>

// Androiddebug
#include <utils/android_debug.h>

#include <stdio.h>
#include <climits>

// Access to malloc, free etc...
#include <malloc.h>

// Use to compute extraction duration
#include <time.h>

#include "listener/SoundSystemCallback.h"

#define AUDIO_HARDWARE_SAMPLE_TYPE short

static void extractionEndCallback(SLPlayItf caller, void *pContext, SLuint32 event);

static void queueExtractorCallback(SLAndroidSimpleBufferQueueItf aSoundQueue, void *aContext);

static void queuePlayerCallback(SLAndroidSimpleBufferQueueItf aSoundQueue, void *aContext);

class SoundSystem {

public:
    SoundSystem(
            SoundSystemCallback *callback,
            int sampleRate,
            int bufSize);

    ~SoundSystem();

    void extractMusic(SLDataLocator_URI *fileLoc);

    void extractAndPlayDirectly(void *sourceFile);

    void initAudioPlayer();

    void sendSoundBufferExtract();

    void sendSoundBufferPlay();

    void stopSoundPlayer();

    void play(bool play);

    bool isPlaying();

    void stop();

    int getPlayerState();

    void fillDataBuffer();

    void getData();

    void endTrack();

    void release();

    void releasePlayer();

    AUDIO_HARDWARE_SAMPLE_TYPE *getExtractedDataMono();

    inline AUDIO_HARDWARE_SAMPLE_TYPE *getExtractedData() {
        return _extractedData;
    }

    inline void setExtractedData(AUDIO_HARDWARE_SAMPLE_TYPE *extractedData) {
        _extractedData = extractedData;
    }

    inline void setTotalNumberFrames(unsigned int totalFrames) {
        _totalFrames = totalFrames;
    }

    inline unsigned int getTotalNumberFrames() {
        return _totalFrames;
    }

    inline bool isLoaded() {
        return _isLoaded;
    }

    inline void setIsLoaded(bool isLoaded) {
        _isLoaded = isLoaded;
    }

    inline double getExtractionStartTime() {
        return _extractionStartTime;
    }

    //------------------------
    // - Extraction methods -
    //------------------------
    void notifyExtractionEnded();

    void notifyExtractionStarted();

    void notifyPlayPause(bool play);

    void notifyStopTrack();

    void notifyEndOfTrack();

private :

    void extractMetaData();

    // device features
    int _sampleRate;
    int _bufferSize;

    // positions used during extraction and play of the track
    int _positionExtract;
    int _positionPlay;

    bool _isLoaded;

    bool _needExtractInitialisation;

    double _extractionStartTime;

    // extracted track info
    SLmillisecond _musicDuration = 0;
    unsigned int _totalFrames;

    // object used to notify of some events
    SoundSystemCallback *_soundSystemCallback = nullptr;

    // engine
    SLObjectItf _engineObj = nullptr;
    SLEngineItf _engine = nullptr;

    // output
    SLObjectItf _outPutMixObj = nullptr;

    //extract
    SLObjectItf _extractPlayerObject = nullptr;
    SLPlayItf _extractPlayerPlay = nullptr;
    SLAndroidSimpleBufferQueueItf _extractPlayerBufferQueue = nullptr;
    SLMetadataExtractionItf _extractPlayerMetadata = nullptr;

    //play
    SLObjectItf _playerObject = nullptr;
    SLPlayItf _playerPlay = nullptr;
    SLAndroidSimpleBufferQueueItf _playerQueue = nullptr;

    //buffer
    short *_soundBuffer = nullptr;
    AUDIO_HARDWARE_SAMPLE_TYPE *_playerBuffer = nullptr;

    //extracted music
    AUDIO_HARDWARE_SAMPLE_TYPE *_extractedData = nullptr;

};

#endif //TEST_SOUNDSYSTEM_SOUNDSYSTEM_H
