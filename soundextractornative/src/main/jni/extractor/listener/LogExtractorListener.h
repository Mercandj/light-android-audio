#ifndef LOG_EXTRACTOR_LISTENER_H
#define LOG_EXTRACTOR_LISTENER_H

#include "extractor/Extractor.h"
#include <utils/android_debug.h>

class LogExtractorListener : public ExtractorListener {

public:
    virtual void onExtractorListenerStarted(
            Extractor *extractor,
            const char *filePath) override;

    virtual void onExtractorListenerSucceeded(
            Extractor *extractor,
            const char *filePath,
            short *writePos,
            size_t written) override;

    virtual void onExtractorListenerFailed(
            Extractor *extractor,
            const char *filePath,
            int errorCore) override;
};

#endif // LOG_EXTRACTOR_LISTENER_H
