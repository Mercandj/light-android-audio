
#include <utils/time_utils.h>
#include "MediaCodecSingleThreadExtractor.h"

MediaCodecSingleThreadExtractor::MediaCodecSingleThreadExtractor(SoundSystem *soundSystem,
                                                                 const unsigned short frameRate) :
        _device_frame_rate(frameRate) {
    data.soundSystem = soundSystem;
}

// shut down the native media system
MediaCodecSingleThreadExtractor::~MediaCodecSingleThreadExtractor() {
    workerdata *d = &data;
    AMediaCodec_stop(d->codec);
    AMediaCodec_delete(d->codec);
    AMediaExtractor_delete(d->ex);
    d->sawInputEOS = true;
    d->sawOutputEOS = true;
}

bool MediaCodecSingleThreadExtractor::extract(const char *filename) {
    AMediaExtractor *ex = AMediaExtractor_new();
    media_status_t err = AMediaExtractor_setDataSource(ex, filename);

    workerdata *d = &data;

    if (err != AMEDIA_OK) {
        LOGV("setDataSource error: %d", err);
        return false;
    }

    size_t numtracks = AMediaExtractor_getTrackCount(ex);
    if (numtracks >= 1) {
        AMediaFormat *format = AMediaExtractor_getTrackFormat(ex, 0);
        const char *s = AMediaFormat_toString(format);
        LOGV("track format: %s", s);
        const char *mime;
        if (!AMediaFormat_getString(format, AMEDIAFORMAT_KEY_MIME, &mime)) {
            LOGV("no mime type");
            return false;
        } else if (strncmp(mime, "audio/", 6) == 0) {
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

            d->ex = ex;
            d->codec = codec;
            d->sawInputEOS = false;
            d->sawOutputEOS = false;
            d->format = audio_format;

            AMediaCodec_start(codec);

            d->soundSystem->notifyExtractionStarted();
            d->extractionPosition = 0;
            d->numberChannel = (unsigned short) _file_number_channels;
        }

        AMediaFormat_delete(format);
        d->extractionTimeStart = now_ms();
        startExtractorThread();

        return true;
    } else {
        return false;
    }
}

void MediaCodecSingleThreadExtractor::extractMetadata(AMediaFormat *format) {
    // extract track information
    AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_CHANNEL_COUNT, &_file_number_channels);
    AMediaFormat_getInt64(format, AMEDIAFORMAT_KEY_DURATION, &_file_duration);
    AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_SAMPLE_RATE, &_file_sample_rate);

    // duration is in micro seconds
    _file_total_frames = (unsigned int) (((double) _file_duration * (double) _device_frame_rate /
                                          1000000.0));

    data.extractedData = (uint8_t *) calloc(_file_total_frames * _file_number_channels * 2,
                                            sizeof(uint8_t));
    data.soundSystem->setExtractedData(reinterpret_cast<short *>(data.extractedData));
    data.soundSystem->setTotalNumberFrames(_file_total_frames);
}

void MediaCodecSingleThreadExtractor::startExtractorThread() {
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    pthread_t worker;
    pthread_create(&worker, &attr, doExtraction, this);
    pthread_detach(worker);
}

void *MediaCodecSingleThreadExtractor::doExtraction(void *) {
    workerdata *d = &data;

    while (!d->sawInputEOS || !d->sawOutputEOS) {
        ssize_t bufidx;
        if (!d->sawInputEOS) {
            bufidx = AMediaCodec_dequeueInputBuffer(d->codec, 5000);
            if (bufidx >= 0) {
                size_t bufsize;
                auto buf = AMediaCodec_getInputBuffer(d->codec, (size_t) bufidx, &bufsize);
                ssize_t sampleSize = AMediaExtractor_readSampleData(d->ex, buf, bufsize);
                if (sampleSize < 0) {
                    sampleSize = 0;
                    d->sawInputEOS = true;
                }
                AMediaCodec_queueInputBuffer(d->codec,
                                             (size_t) bufidx,
                                             0, /* offset */
                                             (size_t) sampleSize,
                                             0, /* time */
                                             d->sawInputEOS
                                             ? AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM
                                             : 0);
                AMediaExtractor_advance(d->ex);
            }
        }

        if (!d->sawOutputEOS) {
            AMediaCodecBufferInfo info;
            ssize_t status = AMediaCodec_dequeueOutputBuffer(d->codec, &info, 5000);
            if (status >= 0) {
                if (info.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM) {
                    LOGI("Extraction nougat duration : %f", now_ms() - d->extractionTimeStart);
                    d->sawOutputEOS = true;
                    d->soundSystem->notifyExtractionEnded();
                }

                // get extracted data from output buffer
                size_t bufsize;
                uint8_t *buf = AMediaCodec_getOutputBuffer(d->codec, (size_t) status, &bufsize);

                if (d->format == RAW) {
                    memcpy(d->extractedData + d->extractionPosition, buf, bufsize);
                    // Force step of RAW_BUFFER_SIZE, it's like setting buf size to RAW_BUFFER_SIZE
                    // We don't trust bufSize value because on some devices, there may be less
                    // data in the returned buffer than bufSize value.
                    d->extractionPosition += RAW_BUFFER_SIZE;
                } else if (d->format == M4A) {
                    memcpy(d->extractedData + d->extractionPosition, buf, bufsize);
                    d->extractionPosition += bufsize / d->numberChannel / 4;
                } else {
                    memcpy(d->extractedData + d->extractionPosition, buf, bufsize);
                    d->extractionPosition += bufsize / d->numberChannel;
                }

                AMediaCodec_releaseOutputBuffer(d->codec, (size_t) status, false);

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

    return NULL;
}

