//
// Created by Frederic on 26/03/2017.
//

#ifndef MINI_SOUND_SYSTEM_EXTRACTORNOUGAT_H
#define MINI_SOUND_SYSTEM_EXTRACTORNOUGAT_H

#include <jni.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <audio/SoundSystem.h>

#include "Looper.h"
#include "media/NdkMediaCodec.h"
#include "media/NdkMediaExtractor.h"

typedef struct {
    AMediaExtractor *ex;
    AMediaCodec *codec;

    bool sawInputEOS;
    bool sawOutputEOS;
    bool isPlaying;
    bool renderonce;

    SoundSystem *soundSystem;
    AUDIO_HARDWARE_SAMPLE_TYPE *extractedData;

    double extractionTimeStart;
    bool isBufferInitialized;
    unsigned int extractionPosition;

} WorkerData;

enum {
    kMsgCodecBuffer,
    kMsgPause,
    kMsgResume,
    kMsgPauseAck,
    kMsgDecodeDone,
};

class MyLooper : public Looper {
    virtual void handle(int what, void *obj);
};

static MyLooper *mlooper = NULL;
static WorkerData data = {NULL, NULL, false, false, false, false, NULL};

class ExtractorNougat {
public:

    ExtractorNougat(SoundSystem *soundSystem, const unsigned short frameRate);

    ~ExtractorNougat();

    void setPlayingStreamingMediaPlayer(const bool isPlaying);

    bool extract(const char *filename);

    void extractMetadata(AMediaFormat *format);

private:

    unsigned int _totalFrames;
    int32_t _file_sample_rate;
    int32_t _number_channels;
    int64_t _duration;
    const unsigned short _frameRate;

};

#endif //MINI_SOUND_SYSTEM_EXTRACTORNOUGAT_H
