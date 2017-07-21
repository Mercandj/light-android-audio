#ifndef SOUND_EXTRACTOR_LISTENER_H_
#define SOUND_EXTRACTOR_LISTENER_H_

#include <stdint.h>
#include "extractor/Extractor.h"

class SoundExtractorListener : public ExtractorListener {
public:
    virtual void onSoundExtractorListenerStarted(
            const char **filePaths) {}

    virtual void onSoundExtractorListenerSucceeded(
            const char **filePaths,
            double duration) {}

    //region ExtractorListener
    virtual void onExtractorListenerStarted(
            const char *filePath) {}

    virtual void onExtractorListenerSucceeded(
            const char *filePath,
            short *writePos,
            size_t written) {}

    virtual void onExtractorListenerFailed(
            const char *filePath,
            int errorCore) {}
    //endregion ExtractorListener
};

#endif // SOUND_EXTRACTOR_LISTENER_H_
