#ifndef FILE_EXTRACTOR_LISTENER_H
#define FILE_EXTRACTOR_LISTENER_H

#include "extractor/Extractor.h"
#include "LogExtractorListener.h"
#include <utils/android_debug.h>
#include <utils/file_utils.h>
#include <stdio.h>

class FileExtractorListener : public LogExtractorListener {

public:
    FileExtractorListener();

    FileExtractorListener(ExtractorListener *extractorListener);

    ~FileExtractorListener();

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

private:
    ExtractorListener *_extractorListener;
};

#endif // FILE_EXTRACTOR_LISTENER_H
