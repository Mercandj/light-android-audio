
#ifndef SOUND_SYSTEM_AUDIO_EXTRACTOR_FFMPEG_SYNCHRONOUS_EXTRACTOR_H
#define SOUND_SYSTEM_AUDIO_EXTRACTOR_FFMPEG_SYNCHRONOUS_EXTRACTOR_H

#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <media/NdkMediaCodec.h>
#include <media/NdkMediaExtractor.h>

#include <audio/SoundSystem.h>

#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>

class FFmpegSynchronousExtractor {
public:

    FFmpegSynchronousExtractor(SoundSystem *soundSystem, const unsigned short frameRate);

    ~FFmpegSynchronousExtractor();

    bool extract(const char *path);

private:

    const unsigned short _frameRate;
    SoundSystem *_soundSystem;

    short *_extractedData;
    int32_t _file_sample_rate;
    int32_t _file_number_channels;
    int64_t _file_duration;
    unsigned int _file_total_frames;
    int _size;

    void extractMetadata(
            const AVFormatContext *format,
            const AVCodecContext *codec);

    int decodeAudioFile(
            AVFormatContext *format,
            AVStream *stream,
            AVCodecContext *codec,
            short **data,
            int *size);
};

#endif // SOUND_SYSTEM_AUDIO_EXTRACTOR_FFMPEG_SYNCHRONOUS_EXTRACTOR_H
