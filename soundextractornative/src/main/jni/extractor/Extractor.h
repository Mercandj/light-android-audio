#ifndef EXTRACTOR_H_
#define EXTRACTOR_H_

#include <stdint.h>
#include <utils/android_debug.h>

class Extractor;

class ExtractorListener {
public:
    virtual void onExtractorListenerStarted(
            Extractor *extractor,
            const char *filePath) {}

    virtual void onExtractorListenerSucceeded(
            Extractor *extractor,
            const char *filePath,
            short *writePos,
            size_t written) {}

    virtual void onExtractorListenerFailed(
            Extractor *extractor,
            const char *filePath,
            int errorCore) {}
};

class Extractor {
public:

    Extractor(
            const unsigned short deviceFrameRate) :
            _deviceFrameRate(deviceFrameRate) {
    }

    virtual ~Extractor() {
        setExtractorListener(nullptr);
    }

    virtual void extract() {};

    void setFilePath(
            const char *filePath) {
        _filePath = filePath;
    }

    void setExtractorListener(
            ExtractorListener *extractorListener) {
        _extractorListener = extractorListener;
    }

protected:
    const char *_filePath = nullptr;
    const unsigned short _deviceFrameRate;
    ExtractorListener *_extractorListener = nullptr;
};

#endif // EXTRACTOR_H_
