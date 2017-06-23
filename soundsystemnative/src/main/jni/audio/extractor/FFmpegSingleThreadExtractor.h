
#ifndef SOUND_SYSTEM_AUDIO_EXTRACTOR_FFMPEG_SINGLE_THREAD_EXTRACTOR_H
#define SOUND_SYSTEM_AUDIO_EXTRACTOR_FFMPEG_SINGLE_THREAD_EXTRACTOR_H

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

class FFmpegSingleThreadExtractor {
public:

    FFmpegSingleThreadExtractor(SoundSystem *soundSystem, const unsigned short frameRate);

    ~FFmpegSingleThreadExtractor();

    bool extract(const char *path);

private:

    const unsigned short _frameRate;
    SoundSystem *_soundSystem;

};

#endif // SOUND_SYSTEM_AUDIO_EXTRACTOR_FFMPEG_SINGLE_THREAD_EXTRACTOR_H
