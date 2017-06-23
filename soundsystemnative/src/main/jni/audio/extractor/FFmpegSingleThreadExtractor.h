
#ifndef SOUND_SYSTEM_AUDIO_EXTRACTOR_FFMPEG_SINGLE_THREAD_EXTRACTOR_H
#define SOUND_SYSTEM_AUDIO_EXTRACTOR_FFMPEG_SINGLE_THREAD_EXTRACTOR_H

#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <media/NdkMediaCodec.h>
#include <media/NdkMediaExtractor.h>

#include <audio/SoundSystem.h>

#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>

typedef struct {
    AVFormatContext *format;
    AVStream *stream;
    AVCodecContext *codec;
    unsigned short _device_frame_rate;

    bool sawInputEOS;
    bool sawOutputEOS;

    SoundSystem *soundSystem;
    short *extractedData;

    double extractionTimeStart;
    unsigned int extractionPosition;
    unsigned short numberChannel;

} WorkerData;

static WorkerData workerData = {NULL, NULL, NULL, 0, false, false, NULL, NULL, 0, 0, 0};

class FFmpegSingleThreadExtractor {
public:

    FFmpegSingleThreadExtractor(SoundSystem *soundSystem, const unsigned short frameRate);

    ~FFmpegSingleThreadExtractor();

    bool extract(const char *path);

private:

    unsigned int _file_total_frames;
    int32_t _file_sample_rate;
    int32_t _file_number_channels;
    int64_t _file_duration;

    void extractMetadata(
            const AVFormatContext *format,
            const AVCodecContext *codec);

    void startExtractorThread();

    static void *doExtraction(void *);
};

#endif // SOUND_SYSTEM_AUDIO_EXTRACTOR_FFMPEG_SINGLE_THREAD_EXTRACTOR_H
