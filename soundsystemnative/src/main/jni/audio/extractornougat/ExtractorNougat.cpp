/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* This is a JNI example where we use native methods to play video
 * using the native AMedia* APIs.
 * See the corresponding Java source file located at:
 *
 *   src/com/example/nativecodec/NativeMedia.java
 *
 * In this example we use assert() for "impossible" error conditions,
 * and explicit handling and recovery for more likely error conditions.
 */

#ifdef MEDIACODEC_EXTRACTOR

#include "ExtractorNougat.h"

//FILE* file;

static double now_ms(void) {
    struct timespec res;
    clock_gettime(CLOCK_REALTIME, &res);
    return 1000.0 * res.tv_sec + (double) res.tv_nsec / 1e6;
}

void doCodecWork(workerdata *d) {
    ssize_t bufidx;
    if (!d->sawInputEOS) {
        bufidx = AMediaCodec_dequeueInputBuffer(d->codec, 1000);
        if (bufidx >= 0) {
            size_t bufsize;
            auto buf = AMediaCodec_getInputBuffer(d->codec, bufidx, &bufsize);
            auto sampleSize = AMediaExtractor_readSampleData(d->ex, buf, bufsize);
            if (sampleSize < 0) {
                sampleSize = 0;
                d->sawInputEOS = true;
            }

            AMediaCodec_queueInputBuffer(d->codec, bufidx, 0, sampleSize, 0,
                                         d->sawInputEOS ? AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM
                                                        : 0);
            AMediaExtractor_advance(d->ex);

            //fclose(file);
        }
    }

    if (!d->sawOutputEOS) {
        AMediaCodecBufferInfo info;
        auto status = AMediaCodec_dequeueOutputBuffer(d->codec, &info, 1000);
        if (status >= 0) {
            if (info.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM) {
                LOGI("Extraction nougat duration : %f", now_ms() - d->extractionTimeStart);
                d->sawOutputEOS = true;
                d->soundSystem->notifyExtractionEnded();
            }

            // get extracted data from output buffer
            if (!d->renderonce) {
                int nbChannel = 2;
                size_t bufsize;
                auto *buf = AMediaCodec_getOutputBuffer(d->codec, status, &bufsize);
                memcpy(d->extractedData + d->extractionPosition,
                       reinterpret_cast<AUDIO_HARDWARE_SAMPLE_TYPE *>(buf),
                       bufsize / nbChannel);
                d->extractionPosition += bufsize / nbChannel / sizeof(AUDIO_HARDWARE_SAMPLE_TYPE);

                /*fwrite(reinterpret_cast<AUDIO_HARDWARE_SAMPLE_TYPE *>(buf),
                            sizeof(AUDIO_HARDWARE_SAMPLE_TYPE),
                            bufsize / nbChannel / sizeof(AUDIO_HARDWARE_SAMPLE_TYPE),
                            file);*/
            }

            AMediaCodec_releaseOutputBuffer(d->codec, status, false);
            if (d->renderonce) {
                d->renderonce = false;
                return;
            }

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

    if (!d->sawInputEOS || !d->sawOutputEOS) {
        mlooper->post(kMsgCodecBuffer, d);
    }
}

void MyLooper::handle(int what, void *obj) {
    switch (what) {
        case kMsgCodecBuffer:
            doCodecWork((workerdata *) obj);
            break;
        case kMsgDecodeDone: {
            workerdata *d = (workerdata *) obj;
            AMediaCodec_stop(d->codec);
            AMediaCodec_delete(d->codec);
            AMediaExtractor_delete(d->ex);
            d->sawInputEOS = true;
            d->sawOutputEOS = true;
        }
            break;
        case kMsgPause: {
            workerdata *d = (workerdata *) obj;
            if (d->isPlaying) {
                // flush all outstanding codecbuffer messages with a no-op message
                d->isPlaying = false;
                post(kMsgPauseAck, NULL, true);
            }
        }
            break;
        case kMsgResume: {
            workerdata *d = (workerdata *) obj;
            if (!d->isPlaying) {
                d->isPlaying = true;
                post(kMsgCodecBuffer, d);
            }
        }
            break;
    }
}

ExtractorNougat::ExtractorNougat(SoundSystem *soundSystem, const unsigned short frameRate) :
        _frameRate(frameRate) {
    data.soundSystem = soundSystem;
    //file = fopen("/sdcard/Music/sample", "w+");
}

// shut down the native media system
ExtractorNougat::~ExtractorNougat() {
    if (mlooper) {
        mlooper->post(kMsgDecodeDone, &data, true /* flush */);
        mlooper->quit();
        delete mlooper;
        mlooper = NULL;
    }
}

bool ExtractorNougat::extract(const char *filename) {
    AMediaExtractor *ex = AMediaExtractor_new();

    media_status_t err = AMediaExtractor_setDataSource(ex, filename);
    workerdata *d = &data;

    if (err != AMEDIA_OK) {
        LOGV("setDataSource error: %d", err);
        return false;
    }

    int numtracks = AMediaExtractor_getTrackCount(ex);

    AMediaCodec *codec = NULL;

    LOGV("input has %d tracks", numtracks);
    for (int i = 0; i < numtracks; i++) {
        AMediaFormat *format = AMediaExtractor_getTrackFormat(ex, i);
        const char *s = AMediaFormat_toString(format);
        LOGV("track %d format: %s", i, s);
        const char *mime;
        if (!AMediaFormat_getString(format, AMEDIAFORMAT_KEY_MIME, &mime)) {
            LOGV("no mime type");
            return false;
        } else if (strncmp(mime, "audio/", 6) == 0) {
            extractMetadata(format);
            // Omitting most error handling for clarity.
            // Production code should check for errors.
            AMediaExtractor_selectTrack(ex, i);
            codec = AMediaCodec_createDecoderByType(mime);
            AMediaCodec_configure(codec, format, NULL, NULL, 0);
            d->ex = ex;
            d->codec = codec;
            d->sawInputEOS = false;
            d->sawOutputEOS = false;
            d->isPlaying = false;
            d->renderonce = true;
            AMediaCodec_start(codec);

            d->soundSystem->notifyExtractionStarted();
            d->isBufferInitialized = false;
            d->extractionPosition = 0;
        }
        AMediaFormat_delete(format);
    }

    mlooper = new MyLooper();
    mlooper->post(kMsgCodecBuffer, d);

    setPlayingStreamingMediaPlayer(true);

    d->extractionTimeStart = now_ms();

    return true;
}

void ExtractorNougat::extractMetadata(AMediaFormat *format) {
    // extract track information
    AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_CHANNEL_COUNT, &_number_channels);
    AMediaFormat_getInt64(format, AMEDIAFORMAT_KEY_DURATION, &_duration);
    AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_SAMPLE_RATE, &_file_sample_rate);

    // duration is in micro seconds
    _totalFrames = (unsigned int) (((double) _duration * (double) _frameRate / 1000000.0));

    data.extractedData = (AUDIO_HARDWARE_SAMPLE_TYPE *) calloc(_totalFrames * _number_channels,
                                                               sizeof(AUDIO_HARDWARE_SAMPLE_TYPE));
    data.soundSystem->setExtractedData(data.extractedData);
    data.soundSystem->setTotalNumberFrames(_totalFrames);
}

// set the playing state for the streaming media player
void ExtractorNougat::setPlayingStreamingMediaPlayer(const bool isPlaying) {
    LOGV("@@@ playpause: %d", isPlaying);
    if (mlooper) {
        if (isPlaying) {
            mlooper->post(kMsgResume, &data);
        } else {
            mlooper->post(kMsgPause, &data);
        }
    }
}

#endif