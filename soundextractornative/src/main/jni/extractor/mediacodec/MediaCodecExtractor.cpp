#if __ANDROID_API__ >= 21

#include "MediaCodecExtractor.h"

MediaCodecExtractor::MediaCodecExtractor(
        const unsigned short deviceFrameRate) :
        Extractor(deviceFrameRate) {
    _mediaCodecExtractorData = new MediaCodecExtractorData();
    _mediaCodecExtractorData->mediaCodecSynchronousExtractor = this;
}

MediaCodecExtractor::~MediaCodecExtractor() {
    MediaCodecExtractorData *d = _mediaCodecExtractorData;
    AMediaCodec_stop(d->codec);
    AMediaCodec_delete(d->codec);
    AMediaExtractor_delete(d->ex);
    d->sawInputEOS = true;
    d->sawOutputEOS = true;
    delete _mediaCodecExtractorData;
}

void MediaCodecExtractor::extract() {
    if (_filePath == nullptr) {
        LOGE("OpenSLExtractor::extract() Set a _filePath first.");
        throw;
    }

    AMediaExtractor *ex = AMediaExtractor_new();
    media_status_t err = AMediaExtractor_setDataSource(ex, _filePath);

    if (err != AMEDIA_OK) {
        // setDataSource error
        _mediaCodecExtractorData->mediaCodecSynchronousExtractor->_extractorListener->onExtractorListenerFailed(
                this,
                _mediaCodecExtractorData->mediaCodecSynchronousExtractor->_filePath,
                -1);
        return;
    }

    size_t numtracks = AMediaExtractor_getTrackCount(ex);
    if (numtracks < 1) {
        _mediaCodecExtractorData->mediaCodecSynchronousExtractor->_extractorListener->onExtractorListenerFailed(
                this,
                _mediaCodecExtractorData->mediaCodecSynchronousExtractor->_filePath,
                -2);
        return;
    }

    AMediaFormat *format = AMediaExtractor_getTrackFormat(ex, 0);
    const char *mime;
    if (!AMediaFormat_getString(format, AMEDIAFORMAT_KEY_MIME, &mime)) {
        // no mime type
        _mediaCodecExtractorData->mediaCodecSynchronousExtractor->_extractorListener->onExtractorListenerFailed(
                this,
                _mediaCodecExtractorData->mediaCodecSynchronousExtractor->_filePath,
                -3);
        return;
    }
    if (strncmp(mime, "audio/", 6) != 0) {
        _mediaCodecExtractorData->mediaCodecSynchronousExtractor->_extractorListener->onExtractorListenerFailed(
                this,
                _mediaCodecExtractorData->mediaCodecSynchronousExtractor->_filePath,
                -4);
        return;
    }

    extractMetadata(format);
    AMediaExtractor_selectTrack(ex, 0);

    int audio_format = MP3;

    // Force codec usage for MP3 and RAW (Wav) audio tracks
    AMediaCodec *codec = NULL;
    if (strncmp(mime, "audio/mpeg", 10) == 0) {
        codec = AMediaCodec_createCodecByName("OMX.google.mp3.decoder");
    } else if (strncmp(mime, "audio/raw", 9) == 0) {
        audio_format = RAW;
        codec = AMediaCodec_createCodecByName("OMX.google.raw.decoder");
    } else if (strncmp(mime, "audio/mp4a-latm", 15) == 0) {
        audio_format = M4A;
        codec = AMediaCodec_createCodecByName("OMX.google.aac.decoder");
    } else {
        codec = AMediaCodec_createDecoderByType(mime);
    }

    AMediaCodec_configure(codec, format, NULL, NULL, 0);

    _mediaCodecExtractorData->ex = ex;
    _mediaCodecExtractorData->codec = codec;
    _mediaCodecExtractorData->sawInputEOS = false;
    _mediaCodecExtractorData->sawOutputEOS = false;
    _mediaCodecExtractorData->format = audio_format;

    AMediaCodec_start(codec);

    _mediaCodecExtractorData->mediaCodecSynchronousExtractor->_extractorListener->onExtractorListenerStarted(
            this,
            _mediaCodecExtractorData->mediaCodecSynchronousExtractor->_filePath);
    _mediaCodecExtractorData->extractionPosition = 0;
    _mediaCodecExtractorData->numberChannel = (unsigned short) _file_number_channels;

    AMediaFormat_delete(format);
    startExtractorThread();
}

void MediaCodecExtractor::extractMetadata(AMediaFormat *format) {
    MediaCodecExtractorData *data = _mediaCodecExtractorData;
    // extract track information
    AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_CHANNEL_COUNT, &_file_number_channels);
    AMediaFormat_getInt64(format, AMEDIAFORMAT_KEY_DURATION, &_file_duration);
    AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_SAMPLE_RATE, &_file_sample_rate);

    // duration is in micro seconds
    _file_total_frames = (unsigned int) (((double) _file_duration * (double) _deviceFrameRate /
                                          1000000.0));

    data->extractedData = (short *) calloc(_file_total_frames * _file_number_channels * 2,
                                           sizeof(short));
}

void MediaCodecExtractor::startExtractorThread() {
    pthread_t worker;
    pthread_create(&worker, NULL, doExtraction, _mediaCodecExtractorData);
    pthread_detach(worker);
}

void *MediaCodecExtractor::doExtraction(void *i) {
    MediaCodecExtractorData *d = (MediaCodecExtractorData *) i;

    short *writePos = d->extractedData;
    size_t written = 0;

    while (!d->sawInputEOS || !d->sawOutputEOS) {
        ssize_t bufidx;
        if (!d->sawInputEOS) {

            bufidx = AMediaCodec_dequeueInputBuffer(d->codec, 0);
            if (bufidx >= 0) {
                size_t bufsize;
                auto buf = AMediaCodec_getInputBuffer(d->codec, bufidx, &bufsize);
                auto sampleSize = AMediaExtractor_readSampleData(d->ex, buf, bufsize);
                if (sampleSize < 0) {
                    sampleSize = 0;
                    d->sawInputEOS = true;
                }
                AMediaCodec_queueInputBuffer(
                        d->codec,
                        bufidx,
                        0, /* offset */
                        sampleSize,
                        0, /* time */
                        d->sawInputEOS ? AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM : 0);
                AMediaExtractor_advance(d->ex);
            }
        }

        if (!d->sawOutputEOS) {
            AMediaCodecBufferInfo info;
            auto status = AMediaCodec_dequeueOutputBuffer(d->codec, &info, 0);
            if (status >= 0) {
                if (info.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM) {
                    d->sawOutputEOS = true;
                    d->mediaCodecSynchronousExtractor->_extractorListener->onExtractorListenerSucceeded(
                            d->mediaCodecSynchronousExtractor,
                            d->mediaCodecSynchronousExtractor->_filePath,
                            d->extractedData,
                            written);
                }

                uint8_t *buf = AMediaCodec_getOutputBuffer(d->codec, status, NULL /* out_size */);
                if (buf == nullptr) {
                    break;
                }
                size_t dataSize = info.size;
                writePos += dataSize;
                written += dataSize;
                memcpy(writePos, buf, dataSize);
                AMediaCodec_releaseOutputBuffer(d->codec, status, info.size != 0);
            } else if (status == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED) {
                LOGV("output buffers changed");
            } else if (status == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
                auto format = AMediaCodec_getOutputFormat(d->codec);
                LOGV("format changed to: %s", AMediaFormat_toString(format));
                AMediaFormat_delete(format);
            } else if (status == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
                LOGV("no output buffer right now");
            } else {
                LOGV("unexpected info code: %zd", status);
            }
        }
    }
}

#endif