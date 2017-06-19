
#ifdef AAUDIO

#ifndef MINI_SOUND_SYSTEM_AAUDIOMANAGER_H
#define MINI_SOUND_SYSTEM_AAUDIOMANAGER_H

#include <thread>
#include <cassert>
#include <audio/SoundSystem.h>

#include "AAudioCommon.h"
#include "aaudio/headers/stream_builder.h"

/*
 * This Sample's Engine Structure
 */
struct AAudioEngine {
    uint32_t sampleRate_;
    uint16_t sampleChannels_;
    uint16_t bitsPerSample_;
    aaudio_format_t sampleFormat_;

    SoundSystem *soundSystem_;

    AAudioStream *playStream_;
    bool requestStop_;
    bool playAudio_;

    int playPosition_;

    int32_t underRunCount_;
    int32_t bufSizeInFrames_;
    int32_t framesPerBurst_;
    int32_t defaultBufSizeInFrames_;
};
static AAudioEngine engine;

class AAudioManager {
public:

    bool createEngine(SoundSystem *soundSystem);

    bool start();

    bool stop();

    void deleteEngine();
};


#endif //MINI_SOUND_SYSTEM_AAUDIOMANAGER_H

#endif
