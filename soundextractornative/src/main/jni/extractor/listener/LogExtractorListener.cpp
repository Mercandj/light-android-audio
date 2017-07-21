#include "LogExtractorListener.h"

void LogExtractorListener::onExtractorListenerStarted(
        Extractor *extractor,
        const char *filePath) {
    LOGD("jm/debug [LogExtractorListener] onExtractorListenerStarted filePath=%s",
         filePath);
}

void LogExtractorListener::onExtractorListenerSucceeded(
        Extractor *extractor,
        const char *filePath,
        short *writePos,
        size_t written) {
    LOGD("jm/debug [LogExtractorListener] onExtractorListenerSucceeded filePath=%s writePos=%d written=%d",
         filePath,
         writePos,
         written);
}

void LogExtractorListener::onExtractorListenerFailed(
        Extractor *extractor,
        const char *filePath,
        int error_core) {
    LOGD("jm/debug [LogExtractorListener] onExtractorListenerFailed filePath=%s error_core=%d",
         filePath,
         error_core);
}