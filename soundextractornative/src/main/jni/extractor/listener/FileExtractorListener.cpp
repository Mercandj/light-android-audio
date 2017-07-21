#include "FileExtractorListener.h"

FileExtractorListener::FileExtractorListener() {
    _extractorListener = nullptr;
}

FileExtractorListener::FileExtractorListener(ExtractorListener *extractorListener) :
        _extractorListener(extractorListener) {
}

FileExtractorListener::~FileExtractorListener() {
    _extractorListener = nullptr;
}

void FileExtractorListener::onExtractorListenerStarted(
        Extractor *extractor,
        const char *filePath) {
    LogExtractorListener::onExtractorListenerStarted(extractor, filePath);
    if (_extractorListener != nullptr) {
        _extractorListener->onExtractorListenerStarted(extractor, filePath);
    }
}

void FileExtractorListener::onExtractorListenerSucceeded(
        Extractor *extractor,
        const char *filePath,
        short *writePos,
        size_t written) {
    LogExtractorListener::onExtractorListenerSucceeded(
            extractor,
            filePath,
            writePos,
            written);
    char *pathOut = pathToDat(filePath);
    LOGD("jm/debug [FileExtractorListener] onExtractorListenerSucceeded pathOut=%s", pathOut);
    writeDat(pathOut, writePos, written);

    if (_extractorListener != nullptr) {
        _extractorListener->onExtractorListenerSucceeded(
                extractor,
                filePath,
                writePos,
                written);
    }
}

void FileExtractorListener::onExtractorListenerFailed(
        Extractor *extractor,
        const char *filePath,
        int errorCore) {
    LogExtractorListener::onExtractorListenerFailed(extractor, filePath, errorCore);

    if (_extractorListener != nullptr) {
        _extractorListener->onExtractorListenerFailed(extractor, filePath, errorCore);
    }
}