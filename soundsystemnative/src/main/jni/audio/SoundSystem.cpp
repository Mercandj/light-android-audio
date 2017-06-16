#include "SoundSystem.h"

static double now_ms(void) {
    struct timespec res;
    clock_gettime(CLOCK_REALTIME, &res);
    return 1000.0 * res.tv_sec + (double) res.tv_nsec / 1e6;
}

static void extractionEndCallback(SLPlayItf caller, void *pContext, SLuint32 event) {
    if (event & SL_PLAYEVENT_HEADATEND) {
        SoundSystem *self = static_cast<SoundSystem *>(pContext);

        const double extractionEndTime = now_ms();
        LOGI("Extraction opensl duration %f", extractionEndTime - self->getExtractionStartTime());

        self->setIsLoaded(true);
        self->notifyExtractionEnded();
    }
}

static void extractionAndPlayEndCallback(SLPlayItf caller, void *pContext, SLuint32 event) {
    if (event & SL_PLAYEVENT_HEADATEND) {
        SoundSystem *self = static_cast<SoundSystem *>(pContext);
        self->releasePlayer();
    }
}

static void queueExtractorCallback(SLAndroidSimpleBufferQueueItf aSoundQueue, void *aContext) {
    SoundSystem *self = static_cast<SoundSystem *>(aContext);
    self->fillDataBuffer();

    // send new buffer in the queue
    self->sendSoundBufferExtract();
}

static void queuePlayerCallback(SLAndroidSimpleBufferQueueItf aSoundQueue, void *aContext) {
    SoundSystem *self = static_cast<SoundSystem *>(aContext);
    self->getData();

    // send filled buffer in the queue
    self->sendSoundBufferPlay();
}

void SoundSystem::fillDataBuffer() {
    if (_needExtractInitialisation) {
        notifyExtractionStarted();

        extractMetaData();

        _needExtractInitialisation = false;
        _extractedData = (AUDIO_HARDWARE_SAMPLE_TYPE *) calloc(_totalFrames * 2,
                                                               sizeof(AUDIO_HARDWARE_SAMPLE_TYPE));

        _extractionStartTime = now_ms();
    }

    int sizeBuffer = _bufferSize * sizeof(short);
    memmove(_extractedData + _positionExtract, _soundBuffer, sizeBuffer);
    _positionExtract += _bufferSize;
}

void SoundSystem::getData() {
    if (_positionPlay > _totalFrames * 2 * sizeof(AUDIO_HARDWARE_SAMPLE_TYPE)) {
        endTrack();
        return;
    }

    int sizeBuffer = _bufferSize * sizeof(AUDIO_HARDWARE_SAMPLE_TYPE);
    memmove(_playerBuffer, _extractedData + _positionPlay, sizeBuffer);
    _positionPlay += _bufferSize;
}

SoundSystem::SoundSystem(SoundSystemCallback *callback,
                         int sampleRate,
                         int bufSize) :
        _soundSystemCallback(callback),
        _needExtractInitialisation(true),
        _isLoaded(false),
        _positionExtract(0),
        _positionPlay(0),
        _totalFrames(0),
        _soundBuffer(nullptr),
        _playerBuffer(nullptr) {
    this->_sampleRate = sampleRate;
    this->_bufferSize = bufSize;

    /*
     * A type for standard OpenSL ES errors that all functions defined in the API return.
     * Can have some of these values :
     * SL_RESULT_SUCCESS
     * SL_RESULT_PARAMETER_INVALID
     * SL_RESULT_MEMORY_FAILURE
     * SL_RESULT_FEATURE_UNSUPPORTED
     * SL_RESULT_RESOURCE_ERROR
     */
    SLresult result;

    // engine
    const SLuint32 engineMixIIDCount = 1;
    const SLInterfaceID engineMixIIDs[] = {SL_IID_ENGINE};
    const SLboolean engineMixReqs[] = {SL_BOOLEAN_TRUE};

    /*object : an abstraction of a set of resources, assigned for a well-defined set of tasks,
     and the state of these resources. To allocate the resources the object must be Realized.*/

    // create engine
    result = slCreateEngine(&_engineObj, 0, nullptr, engineMixIIDCount, engineMixIIDs,
                            engineMixReqs);
    SLASSERT(result);

    // 2nd parameter : True if it's asynchronous and false to be synchronous
    result = (*_engineObj)->Realize(_engineObj, SL_BOOLEAN_FALSE);
    SLASSERT(result);

    // interface : an abstraction of a set of related features that a certain object provides.

    // get interfaces
    result = (*_engineObj)->GetInterface(_engineObj, SL_IID_ENGINE, &_engine);
    SLASSERT(result);

    // creation of objects used to send data to HW device

    // mixed output
    const SLuint32 outputMixIIDCount = 0;
    const SLInterfaceID outputMixIIDs[] = {};
    const SLboolean outputMixReqs[] = {};

    result = (*_engine)->CreateOutputMix(_engine, &_outPutMixObj, outputMixIIDCount, outputMixIIDs,
                                         outputMixReqs);
    SLASSERT(result);

    result = (*_outPutMixObj)->Realize(_outPutMixObj, SL_BOOLEAN_FALSE);
    SLASSERT(result);
}

SoundSystem::~SoundSystem() {
    release();
}

void SoundSystem::extractMusic(SLDataLocator_URI *fileLoc) {
    SLresult result;

    SLDataFormat_MIME format_mime;
    format_mime.formatType = SL_DATAFORMAT_MIME;
    format_mime.mimeType = nullptr;
    format_mime.containerType = SL_CONTAINERTYPE_UNSPECIFIED;

    SLDataSource audioSrc;
    audioSrc.pLocator = fileLoc;
    audioSrc.pFormat = &format_mime;

    //Struct representing a data locator for a buffer queue
    //We say that the data will be in memory buffer and that we have two buffers
    SLDataLocator_AndroidSimpleBufferQueue dataLocatorInput;
    dataLocatorInput.locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
    dataLocatorInput.numBuffers = 2;

    // format of data
    SLDataFormat_PCM dataFormat;
    dataFormat.formatType = SL_DATAFORMAT_PCM;
    dataFormat.numChannels = 2; // Stereo sound.
    dataFormat.samplesPerSec = (SLuint32) _sampleRate * 1000;
    dataFormat.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
    dataFormat.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
    dataFormat.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    dataFormat.endianness = SL_BYTEORDER_LITTLEENDIAN;

    SLDataSink audioSnk = {&dataLocatorInput, &dataFormat};

    // SL_IID_ANDROIDSIMPLEBUFFERQUEUE will allow us to control the queue with buffers
    // (we have two of them) create sound player
    const SLuint32 soundPlayerIIDCount = 2;
    const SLInterfaceID soundPlayerIIDs[] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                             SL_IID_METADATAEXTRACTION};
    const SLboolean soundPlayerReqs[] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    result = (*_engine)->CreateAudioPlayer(_engine, &_extractPlayerObject, &audioSrc, &audioSnk,
                                           soundPlayerIIDCount, soundPlayerIIDs, soundPlayerReqs);
    SLASSERT(result);

    result = (*_extractPlayerObject)->Realize(_extractPlayerObject, SL_BOOLEAN_FALSE);
    SLASSERT(result);

    // get the buffer queue interface
    result = (*_extractPlayerObject)->GetInterface(_extractPlayerObject,
                                                   SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                                   &_extractPlayerBufferQueue);
    SLASSERT(result);

    // get the play interface
    result = (*_extractPlayerObject)->GetInterface(_extractPlayerObject, SL_IID_PLAY,
                                                   &_extractPlayerPlay);
    SLASSERT(result);

    // register callback on the player event
    result = (*_extractPlayerPlay)->RegisterCallback(_extractPlayerPlay, extractionEndCallback,
                                                     this);
    // enables/disables notification of playback events.
    result = (*_extractPlayerPlay)->SetCallbackEventsMask(_extractPlayerPlay,
                                                          SL_PLAYEVENT_HEADATEND);
    SLASSERT(result);

    result = (*_extractPlayerObject)->GetInterface(_extractPlayerObject, SL_IID_METADATAEXTRACTION,
                                                   &_extractPlayerMetadata);
    SLASSERT(result);

    // register callback for queue
    result = (*_extractPlayerBufferQueue)->RegisterCallback(_extractPlayerBufferQueue,
                                                            queueExtractorCallback, this);
    SLASSERT(result);

    // allocate space for the buffer
    _soundBuffer = (short *) calloc(_bufferSize, sizeof(short));

    // send two buffers
    sendSoundBufferExtract();
    sendSoundBufferExtract();

    // start the extraction
    result = (*_extractPlayerPlay)->SetPlayState(_extractPlayerPlay, SL_PLAYSTATE_PLAYING);
    SLASSERT(result);

    _isLoaded = false;
}

void SoundSystem::initAudioPlayer() {
    SLresult result;

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq;
    loc_bufq.locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
    loc_bufq.numBuffers = 1;

    // format of data
    SLDataFormat_PCM dataFormat;
    dataFormat.formatType = SL_DATAFORMAT_PCM;
    dataFormat.numChannels = 2; // Stereo sound.
    dataFormat.samplesPerSec = (SLuint32) _sampleRate * 1000;
    dataFormat.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
    dataFormat.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
    dataFormat.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    dataFormat.endianness = SL_BYTEORDER_LITTLEENDIAN;

    SLDataSource audioSrc;
    audioSrc.pLocator = &loc_bufq;
    audioSrc.pFormat = &dataFormat;

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, _outPutMixObj};
    SLDataSink audioSnk = {&loc_outmix, nullptr};

    const SLInterfaceID ids[] = {SL_IID_VOLUME, SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    const SLboolean req[] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    const int numberInterface = sizeof(ids) / sizeof(ids[0]);

    result = (*_engine)->CreateAudioPlayer(_engine, &_playerObject, &audioSrc, &audioSnk,
                                           numberInterface, ids, req);
    // note that an invalid URI is not detected here, but during prepare/prefetch on Android,
    // or possibly during Realize on other platforms
    SLASSERT(result);

    // realize the player
    result = (*_playerObject)->Realize(_playerObject, SL_BOOLEAN_FALSE);

    // get the play interface
    result = (*_playerObject)->GetInterface(_playerObject, SL_IID_PLAY, &_playerPlay);
    SLASSERT(result);

    // get the buffer queue interface
    result = (*_playerObject)->GetInterface(_playerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                            &_playerQueue);
    SLASSERT(result);
    // register callback for queue
    result = (*_playerQueue)->RegisterCallback(_playerQueue, queuePlayerCallback, this);
    SLASSERT(result);

    _playerBuffer = (AUDIO_HARDWARE_SAMPLE_TYPE *) calloc(_bufferSize,
                                                          sizeof(AUDIO_HARDWARE_SAMPLE_TYPE));
}

bool SoundSystem::isPlaying() {
    return getPlayerState() == SL_PLAYSTATE_PLAYING;
}

void SoundSystem::extractAndPlayDirectly(void *sourceFile) {
    if (_playerPlay != nullptr && isPlaying()) {
        return;
    }

    SLresult result;

    SLDataFormat_MIME format_mime;
    format_mime.formatType = SL_DATAFORMAT_MIME;
    format_mime.mimeType = nullptr;
    format_mime.containerType = SL_CONTAINERTYPE_UNSPECIFIED;

    SLDataSource audioSrc;
    audioSrc.pLocator = sourceFile;
    audioSrc.pFormat = &format_mime;

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, _outPutMixObj};
    SLDataSink audioSnk = {&loc_outmix, nullptr};

    result = (*_engine)->CreateAudioPlayer(_engine, &_playerObject, &audioSrc, &audioSnk,
                                           0, nullptr, nullptr);
    SLASSERT(result);

    // realize the player
    result = (*_playerObject)->Realize(_playerObject, SL_BOOLEAN_FALSE);
    SLASSERT(result);

    // get the play interface
    result = (*_playerObject)->GetInterface(_playerObject, SL_IID_PLAY, &_playerPlay);
    SLASSERT(result);
    // register callback on the player event
    result = (*_playerPlay)->RegisterCallback(_playerPlay, extractionAndPlayEndCallback, this);
    // enables/disables notification of playback events.
    result = (*_playerPlay)->SetCallbackEventsMask(_playerPlay, SL_PLAYEVENT_HEADATEND);
    SLASSERT(result);

    result = (*_playerPlay)->SetPlayState(_playerPlay, SL_PLAYSTATE_PLAYING);
    SLASSERT(result);
}

void SoundSystem::extractMetaData() {
    (*_extractPlayerPlay)->GetDuration(_extractPlayerPlay, &_musicDuration);
    _totalFrames = (unsigned int) (((double) _musicDuration * (double) _sampleRate / 1000.0));
}

void SoundSystem::play(bool play) {
    SLresult result;
    if (nullptr != _playerPlay) {
        SLuint32 currentState;
        (*_playerPlay)->GetPlayState(_playerPlay, &currentState);
        if (play
            && (currentState == SL_PLAYSTATE_PAUSED || currentState == SL_PLAYSTATE_STOPPED)) {
            sendSoundBufferPlay();
            result = (*_playerPlay)->SetPlayState(_playerPlay, SL_PLAYSTATE_PLAYING);
            SLASSERT(result);

            notifyPlayPause(true);
        } else if (currentState == SL_PLAYSTATE_PLAYING) {
            result = (*_playerPlay)->SetPlayState(_playerPlay, SL_PLAYSTATE_PAUSED);
            SLASSERT(result);
            result = (*_playerQueue)->Clear(_playerQueue);
            SLASSERT(result);
            notifyPlayPause(false);
        }
    }
}

void SoundSystem::stop() {
    _positionPlay = 0;
    (*_playerPlay)->SetPlayState(_playerPlay, SL_PLAYSTATE_STOPPED);
    notifyStopTrack();
};

int SoundSystem::getPlayerState() {
    if (_playerPlay != nullptr) {
        SLuint32 currentState;
        (*_playerPlay)->GetPlayState(_playerPlay, &currentState);
        return currentState;
    }
    return -1;
}

void SoundSystem::sendSoundBufferExtract() {
    if (_extractPlayerBufferQueue != nullptr) {
        SLuint32 result = (*_extractPlayerBufferQueue)->Enqueue(_extractPlayerBufferQueue,
                                                                _soundBuffer,
                                                                sizeof(short) * _bufferSize);
        SLASSERT(result);
    }
}

void SoundSystem::sendSoundBufferPlay() {
    assert(_playerBuffer != nullptr);
    SLuint32 result = (*_playerQueue)->Enqueue(_playerQueue, _playerBuffer,
                                               sizeof(AUDIO_HARDWARE_SAMPLE_TYPE) * _bufferSize);
    SLASSERT(result);
}

void SoundSystem::notifyExtractionEnded() {
    _soundSystemCallback->notifyExtractionCompleted();
}

void SoundSystem::notifyStopTrack() {
    _soundSystemCallback->notifyStopTrack();
}

void SoundSystem::notifyEndOfTrack() {
    _soundSystemCallback->notifyEndOfTrack();
}

void SoundSystem::notifyExtractionStarted() {
    _soundSystemCallback->notifyExtractionStarted();
}

void SoundSystem::notifyPlayPause(bool play) {
    _soundSystemCallback->notifyPlayPause(play);
}

void SoundSystem::endTrack() {
    _positionPlay = 0;
    (*_playerPlay)->SetPlayState(_playerPlay, SL_PLAYSTATE_STOPPED);
    notifyEndOfTrack();
}

void SoundSystem::release() {
    // destroy sound player
    stopSoundPlayer();

    if (_outPutMixObj != nullptr) {
        (*_outPutMixObj)->Destroy(_outPutMixObj);
        _outPutMixObj = nullptr;
    }

    if (_engineObj != nullptr) {
        (*_engineObj)->Destroy(_engineObj);
        _engineObj = nullptr;
        _engine = nullptr;
    }
}

void SoundSystem::stopSoundPlayer() {
    if (_playerObject != nullptr) {
        SLuint32 soundPlayerState;
        (*_playerObject)->GetState(_playerObject, &soundPlayerState);
        if (soundPlayerState == SL_OBJECT_STATE_REALIZED) {
            if (_extractPlayerBufferQueue != nullptr) {
                (*_extractPlayerBufferQueue)->Clear(_extractPlayerBufferQueue);
                _extractPlayerBufferQueue = nullptr;
            }

            if (_playerQueue != nullptr) {
                (*_playerQueue)->Clear(_playerQueue);
                _playerQueue = nullptr;
            }

            if (_extractPlayerObject != nullptr) {
                (*_extractPlayerObject)->AbortAsyncOperation(_extractPlayerObject);
                (*_extractPlayerObject)->Destroy(_extractPlayerObject);
                _extractPlayerObject = nullptr;
                _extractPlayerPlay = nullptr;
            }

            releasePlayer();

            if (_soundBuffer != nullptr) {
                free(_soundBuffer);
                _soundBuffer = nullptr;
            }
        }
    }
}

void SoundSystem::releasePlayer() {
    if (_playerObject != nullptr) {
        (*_playerObject)->AbortAsyncOperation(_playerObject);
        (*_playerObject)->Destroy(_playerObject);
        _playerObject = nullptr;
        _playerPlay = nullptr;
    }
}

AUDIO_HARDWARE_SAMPLE_TYPE *SoundSystem::getExtractedDataMono() {
    unsigned int sizeDataMono = _totalFrames / 2;
    AUDIO_HARDWARE_SAMPLE_TYPE *dataMono = (AUDIO_HARDWARE_SAMPLE_TYPE *) calloc(
            sizeof(AUDIO_HARDWARE_SAMPLE_TYPE), sizeDataMono);

    for (int i = 0; i < sizeDataMono; i++) {
        dataMono[i] = (_extractedData[i * 2] + _extractedData[i * 2 + 1]) / 2;
    }
    return dataMono;
}




