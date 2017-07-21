#include "OpenSLExtractor.h"

#define SLASSERT(x) assert(x == SL_RESULT_SUCCESS)

static int index = 0;

OpenSLExtractor::OpenSLExtractor(
        const unsigned short deviceFrameRate,
        int openSLBufSize) :
        Extractor(deviceFrameRate),
        _bufferSize(openSLBufSize),
        _needExtractInitialisation(true) {

    LOGD("jm/debug OpenSLExtractor index:%d", index);
    index++;

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
}

OpenSLExtractor::~OpenSLExtractor() {
    LOGD("jm/debug ~OpenSLExtractor");
    if (_extractPlayerBufferQueue != nullptr) {
        (*_extractPlayerBufferQueue)->Clear(_extractPlayerBufferQueue);
        _extractPlayerBufferQueue = nullptr;
    }
    if (_extractPlayerObject != nullptr) {
        LOGD("jm/debug ~OpenSLExtractor destroy player");
        // AbortAsyncOperation is called in Destroy.
        (*_extractPlayerObject)->Destroy(_extractPlayerObject);
        _extractPlayerObject = nullptr;
        _extractPlayerPlay = nullptr;
    }
    if (_soundBuffer != nullptr) {
        free(_soundBuffer);
        _soundBuffer = nullptr;
    }
    if (_engineObj != nullptr) {
        (*_engineObj)->Destroy(_engineObj);
        _engineObj = nullptr;
        _engine = nullptr;
    }
}

/* public */ void OpenSLExtractor::extract() {
    if (_filePath == nullptr) {
        LOGE("OpenSLExtractor::extract() Set a _filePath first.");
        throw;
    }
    _positionExtract = 0;
    SLDataLocator_URI *fileLoc = dataLocatorFromURLString(_filePath);
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
    dataFormat.samplesPerSec = (SLuint32) _deviceFrameRate * 1000;
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

    result = (*_engine)->CreateAudioPlayer(
            _engine,
            &_extractPlayerObject,
            &audioSrc,
            &audioSnk,
            soundPlayerIIDCount,
            soundPlayerIIDs,
            soundPlayerReqs);
    SLASSERT(result);

    result = (*_extractPlayerObject)->Realize(
            _extractPlayerObject,
            SL_BOOLEAN_FALSE);
    SLASSERT(result);

    // get the buffer queue interface
    result = (*_extractPlayerObject)->GetInterface(
            _extractPlayerObject,
            SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
            &_extractPlayerBufferQueue);
    SLASSERT(result);

    // get the play interface
    result = (*_extractPlayerObject)->GetInterface(
            _extractPlayerObject, SL_IID_PLAY,
            &_extractPlayerPlay);
    SLASSERT(result);

    // register callback on the player event
    result = (*_extractPlayerPlay)->RegisterCallback(
            _extractPlayerPlay,
            extractionEndCallback,
            this);
    // enables/disables notification of playback events.
    result = (*_extractPlayerPlay)->SetCallbackEventsMask(
            _extractPlayerPlay,
            SL_PLAYEVENT_HEADATEND);
    SLASSERT(result);

    result = (*_extractPlayerObject)->GetInterface(
            _extractPlayerObject,
            SL_IID_METADATAEXTRACTION,
            &_extractPlayerMetadata);
    SLASSERT(result);

    // register callback for queue
    result = (*_extractPlayerBufferQueue)->RegisterCallback(
            _extractPlayerBufferQueue,
            queueExtractorCallback,
            this);
    SLASSERT(result);

    // allocate space for the buffer
    _soundBuffer = (short *) calloc(_bufferSize, sizeof(short));

    // send two buffers
    sendSoundBufferExtract();
    sendSoundBufferExtract();

    // start the extraction
    result = (*_extractPlayerPlay)->SetPlayState(_extractPlayerPlay, SL_PLAYSTATE_PLAYING);
    SLASSERT(result);
}

/**
 * Convert char* to SLDataLocator_URI
 * @param env
 * @param fileURLString
 * @return
 */
/* private */ SLDataLocator_URI *OpenSLExtractor::dataLocatorFromURLString(
        const char *urf8FileURLString) {
    assert(NULL != urf8FileURLString);
    SLDataLocator_URI *uri = (SLDataLocator_URI *) malloc(sizeof(SLDataLocator_URI));
    uri->locatorType = SL_DATALOCATOR_URI;
    uri->URI = (SLchar *) urf8FileURLString;
    return uri;
}

/* private */ void OpenSLExtractor::sendSoundBufferExtract() {
    if (_extractPlayerBufferQueue != nullptr) {
        SLuint32 result = (*_extractPlayerBufferQueue)->Enqueue(
                _extractPlayerBufferQueue,
                _soundBuffer,
                sizeof(short) * _bufferSize);
        SLASSERT(result);
    }
}

/* private */ void OpenSLExtractor::fillDataBuffer() {
    if (_needExtractInitialisation) {
        (*_extractPlayerPlay)->GetDuration(_extractPlayerPlay, &_musicDuration);
        _totalFrames = (unsigned int) (((double) _musicDuration * (double) _deviceFrameRate /
                                        1000.0));

        _needExtractInitialisation = false;
        _extractedData = (short *) calloc(_totalFrames * 2, sizeof(short));

        if (_extractorListener != nullptr) {
            _extractorListener->onExtractorListenerStarted(this, _filePath);
        }
    }

    int sizeBuffer = _bufferSize * sizeof(short);
    memmove(_extractedData + _positionExtract, _soundBuffer, sizeBuffer);
    _positionExtract += _bufferSize;
}

/* private */ void OpenSLExtractor::queueExtractorCallback(
        SLAndroidSimpleBufferQueueItf aSoundQueue,
        void *aContext) {
    OpenSLExtractor *self = static_cast<OpenSLExtractor *>(aContext);
    self->fillDataBuffer();

    // send new buffer in the queue
    self->sendSoundBufferExtract();
}

/* private */ void OpenSLExtractor::extractionEndCallback(
        SLPlayItf caller,
        void *pContext,
        SLuint32 event) {
    OpenSLExtractor *self = static_cast<OpenSLExtractor *>(pContext);
    if (event & SL_PLAYEVENT_HEADATEND) {
        if (self->_extractorListener != nullptr) {
            self->_extractorListener->onExtractorListenerSucceeded(
                    self,
                    self->_filePath,
                    self->_extractedData,
                    self->_positionExtract);
        }
    } else if (self->_extractorListener != nullptr) {
        self->_extractorListener->onExtractorListenerFailed(
                self,
                self->_filePath,
                -10);
    }
}