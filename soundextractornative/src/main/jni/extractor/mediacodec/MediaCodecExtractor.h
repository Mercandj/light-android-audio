#if __ANDROID_API__ >= 21

#ifndef MEDIA_CODEC_EXTRACTOR_H
#define MEDIA_CODEC_EXTRACTOR_H

#include <media/NdkMediaCodec.h>
#include <media/NdkMediaExtractor.h>
#include <utils/android_debug.h>
#include <cstring>
#include <SoundExtractorManager.h>
#include <malloc.h>
#include <extractor/Extractor.h>

#define MP3 0
#define RAW 1
#define M4A 2

class MediaCodecExtractor;

typedef struct {
    AMediaExtractor *ex;
    AMediaCodec *codec;

    bool sawInputEOS;
    bool sawOutputEOS;

    short *extractedData;

    unsigned int extractionPosition;
    unsigned short numberChannel;
    int format;
    MediaCodecExtractor *mediaCodecSynchronousExtractor;

} MediaCodecExtractorData;

class MediaCodecExtractor : public Extractor {

public:

    MediaCodecExtractor(
            const unsigned short deviceFrameRate);

    ~MediaCodecExtractor();

    void extract() override;

private:
    MediaCodecExtractorData *_mediaCodecExtractorData;

    unsigned int _file_total_frames;
    int32_t _file_sample_rate;
    int32_t _file_number_channels;
    int64_t _file_duration;

    void extractMetadata(AMediaFormat *format);

    void startExtractorThread();

    static void *doExtraction(void *);
};

#endif // MEDIA_CODEC_EXTRACTOR_H

#endif // __ANDROID_API__ >= 21