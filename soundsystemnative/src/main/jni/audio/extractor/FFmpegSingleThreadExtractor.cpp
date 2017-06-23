
#include "FFmpegSingleThreadExtractor.h"

FFmpegSingleThreadExtractor::FFmpegSingleThreadExtractor(
        SoundSystem *soundSystem,
        const unsigned short frameRate) :
        _frameRate(frameRate),
        _soundSystem(soundSystem) {
}

// shut down the native media system
FFmpegSingleThreadExtractor::~FFmpegSingleThreadExtractor() {
    // TODO
}

bool FFmpegSingleThreadExtractor::extract(const char *path) {
    // TODO
    return false;
}