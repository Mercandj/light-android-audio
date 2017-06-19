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
#include "SynchronousFfmpegExtractor.h"

SynchronousFfmpegExtractor::SynchronousFfmpegExtractor(
        SoundSystem *soundSystem,
        const unsigned short frameRate) :
        _frameRate(frameRate),
        _soundSystem(soundSystem) {
}

// shut down the native media system
SynchronousFfmpegExtractor::~SynchronousFfmpegExtractor() {

}

bool SynchronousFfmpegExtractor::extract(const char *filename) {

    AMediaExtractor *ex = AMediaExtractor_new();
    media_status_t err = AMediaExtractor_setDataSource(ex, filename);

    if (err != AMEDIA_OK) {
        LOGV("setDataSource error: %d", err);
        return false;
    }

    int numtracks = AMediaExtractor_getTrackCount(ex);

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
            _soundSystem->notifyExtractionStarted();
            decode_audio_file(filename, _extractedData, &_size);
        }
        AMediaFormat_delete(format);
    }
    return true;
}

void SynchronousFfmpegExtractor::extractMetadata(AMediaFormat *format) {
    // extract track information
    AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_CHANNEL_COUNT, &_file_number_channels);
    AMediaFormat_getInt64(format, AMEDIAFORMAT_KEY_DURATION, &_file_duration);
    AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_SAMPLE_RATE, &_file_sample_rate);

    // duration is in micro seconds
    _file_total_frames = (unsigned int) (((double) _file_duration * (double) _device_frame_rate /
                                          1000000.0));

    _extractedData = (uint8_t *) calloc(_file_total_frames * _file_number_channels * 2,
                                        sizeof(uint8_t));
    _soundSystem->setExtractedData(reinterpret_cast<short *>(_extractedData));
    _soundSystem->setTotalNumberFrames(_file_total_frames);
}

void printAudioFrameInfo(const AVCodecContext *codecContext, const AVFrame *frame) {
    /*
     This url: http://ffmpeg.org/doxygen/trunk/samplefmt_8h.html#af9a51ca15301871723577c730b5865c5
     contains information on the type you will need to utilise to access the audio data.
    */
    // format the tabs etc. in this string to suit your font, they line up for mine but may not for yours:)
    LOGD("jm/debug Audio frame info:\n\tSample count: %d\n\tChannel count: %d\n\tFormat: %s\n\tBytes per sample: %d\n\tPlanar storage format: %d\n",
         frame->nb_samples,
         codecContext->channels,
         av_get_sample_fmt_name(codecContext->sample_fmt),
         av_get_bytes_per_sample(codecContext->sample_fmt),
         av_sample_fmt_is_planar(codecContext->sample_fmt));
}

int SynchronousFfmpegExtractor::decode_audio_file(const char *path, uint8_t *data, int *size) {
    LOGD("jm/debug decode_audio_file %s", path);

    // initialize all muxers, demuxers and protocols for libavformat
    // (does nothing if called twice during the course of one program execution)
    av_register_all();

    // get format from audio file
    AVFormatContext *format = avformat_alloc_context();
    if (avformat_open_input(&format, path, NULL, NULL) != 0) {
        LOGD("Could not open file '%s'\n", path);
        return -1;
    }
    if (avformat_find_stream_info(format, NULL) < 0) {
        LOGD("Could not retrieve stream info from file '%s'\n", path);
        return -1;
    }

    // Find the index of the first audio stream
    // Find the audio stream
    AVCodec *cdc = nullptr;
    int stream_index = av_find_best_stream(format, AVMEDIA_TYPE_AUDIO, -1, -1, &cdc, 0);
    if (stream_index < 0) {
        avformat_close_input(&format);
        LOGD("Could not find any audio stream in the file.  Come on! I need data!\n");
        return 165;
    }

    if (stream_index == -1) {
        LOGD("Could not retrieve audio stream from file '%s'\n", path);
        return -1;
    }
    AVStream *stream = format->streams[stream_index];

    // find & open codec
    AVCodecContext *codec = stream->codec;
    if (avcodec_open2(codec, cdc/*avcodec_find_decoder(codec->codec_id)*/, NULL) < 0) {
        LOGD("Failed to open decoder for stream #%u in file '%s'\n", stream_index, path);
        return -1;
    }
    av_opt_set_int(codec, "refcounted_frames", 1, 0);

    // prepare resampler
    struct SwrContext *swr = swr_alloc();
    av_opt_set_int(swr, "in_channel_count", codec->channels, 0);
    av_opt_set_int(swr, "out_channel_count", 2, 0);
    av_opt_set_int(swr, "in_channel_layout", codec->channel_layout, 0);
    av_opt_set_int(swr, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
    av_opt_set_int(swr, "in_sample_rate", codec->sample_rate, 0);
    av_opt_set_int(swr, "out_sample_rate", _frameRate, 0);
    av_opt_set_sample_fmt(swr, "in_sample_fmt", codec->sample_fmt, 0);
    av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_U8, 0);
    swr_init(swr);
    if (!swr_is_initialized(swr)) {
        LOGD("Resampler has not been properly initialized\n");
        return -1;
    }

    // prepare to read data
    AVPacket packet;
    av_init_packet(&packet);
    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        LOGD("Error allocating the frame\n");
        return -1;
    }

    // iterate through frames
    *size = 0;

    // https://stackoverflow.com/questions/20545767/decode-audio-from-memory-c
    // Read the packets in a loop
    bool test = false;

    while (av_read_frame(format, &packet) == 0) {

        if (packet.stream_index == stream->index) {
            AVPacket decodingPacket = packet;

            // Audio packets can have multiple audio frames in a single packet
            while (decodingPacket.size > 0) {

                // Try to decode the packet into a frame(s)
                // Some frames rely on multiple packets, so we have to make sure the frame is finished
                // before utilising it
                int gotFrame = 0;
                int result = avcodec_decode_audio4(codec, frame, &gotFrame, &decodingPacket);

                LOGD("jm/debug result = %d", result);

                if (result >= 0 && gotFrame && decodingPacket.size > 0) {
                    decodingPacket.size -= result;
                    decodingPacket.data += result;

                    /*LOGD("jm/debug RESAMPLE FRAMES decodingPacket.size== %d decodingPacket.data==%"PRIu8"",
                         decodingPacket.size,
                         decodingPacket.data);*/

                    LOGD("jm/debug coucou -1");
                    memcpy(data + *size, frame->data, result * sizeof(uint8_t));
                    LOGD("jm/debug coucou 0");
                    *size += result * sizeof(uint8_t);

                    /*
                    FILE *file = fopen("/sdcard/Music/hello.txt", "w+");
                    //fwrite(data, *size, *size * sizeof(uint8_t), file);
                    char str[] = "This is tutorialspoint.com";
                    LOGI("str %d", sizeof(str));
                    //fwrite(&str , sizeof(char), sizeof(str) , file );
                    fputs("HELLO WORLD!\n", file);
                    //fflush(file);
                    fclose(file);
                     */

                    /*
                    LOGD("jm/debug coucou 1");
                    if (*size > 40000 && !test) {
                        LOGD("jm/debug coucou 2");
                        for (int i = 0; i < *size; i++) {
                            LOGI("data %d", data[i]);
                        }
                        test = true;
                    }*/

                    // ***************************************************** //
                    // ****************** RESAMPLE FRAMES ****************** //
                    /*
                    uint8_t *buffer;
                    av_samples_alloc(&buffer, NULL, 1, frame->nb_samples, AV_SAMPLE_FMT_DBL, 0);
                    int frame_count = swr_convert(
                            swr,
                            &buffer,
                            frame->nb_samples,
                            (const uint8_t **) frame->data,
                            frame->nb_samples);
                    // append resampled frames to data
                    *data = (uint8_t *) realloc(
                            *data,
                            (*size + frame->nb_samples) * sizeof(uint8_t));
                    memcpy(*data + *size, buffer, frame_count * sizeof(double));
                    *size += frame_count;
                    */
                    // ****************** RESAMPLE FRAMES ****************** //
                    // ***************************************************** //

                    // et voila! a decoded audio frame!
                    printAudioFrameInfo(codec, frame);
                } else {
                    decodingPacket.size = 0;
                    decodingPacket.data = nullptr;
                }
            }
        }

        // You MUST call av_free_packet() after each call to av_read_frame()
        // or you will leak so much memory on a large file you will need a memory-plumber!
        //av_packet_unref(&packet);
    }
    /*
    // Some codecs will cause frames to be buffered in the decoding process.
    // If the CODEC_CAP_DELAY flag is set, there can be buffered frames that need to be flushed
    // therefore flush them now....
    if (codec->codec->capabilities & CODEC_CAP_DELAY) {
        av_init_packet(&packet);
        // Decode all the remaining frames in the buffer
        int gotFrame = 0;
        while (avcodec_decode_audio4(codec, frame, &gotFrame, &packet) >= 0 && gotFrame) {
            // Again: a fully decoded audio frame!
            printAudioFrameInfo(codec, frame);
        }
    }
    */

    // clean up
    av_frame_free(&frame);
    swr_free(&swr);
    avcodec_close(codec);
    avformat_free_context(format);

    // success
    return 0;
}