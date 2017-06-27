
#include "FFmpegSynchronousExtractor.h"

FFmpegSynchronousExtractor::FFmpegSynchronousExtractor(
        SoundSystem *soundSystem,
        const unsigned short frameRate) :
        _frameRate(frameRate),
        _soundSystem(soundSystem) {
}

// shut down the native media system
FFmpegSynchronousExtractor::~FFmpegSynchronousExtractor() {

}

bool FFmpegSynchronousExtractor::extract(const char *path) {

    // ************************************************************************* //
    // ****************** PREPARE METADATA / AUDIO EXTRACTION ****************** //

    // initialize all muxers, demuxers and protocols for libavformat
    // (does nothing if called twice during the course of one program execution)
    av_register_all();

    // get format from audio file
    AVFormatContext *format = avformat_alloc_context();
    if (avformat_open_input(&format, path, NULL, NULL) != 0) {
        LOGD("Could not open file '%s'\n", path);
        return false;
    }
    if (avformat_find_stream_info(format, NULL) < 0) {
        LOGD("Could not retrieve stream info from file '%s'\n", path);
        return false;
    }

    // Find the index of the first audio stream
    // Find the audio stream
    AVCodec *cdc = nullptr;
    int stream_index = av_find_best_stream(format, AVMEDIA_TYPE_AUDIO, -1, -1, &cdc, 0);
    if (stream_index < 0) {
        avformat_close_input(&format);
        LOGD("Could not find any audio stream in the file.  Come on! I need data!\n");
        return false;
    }

    if (stream_index == -1) {
        LOGD("Could not retrieve audio stream from file '%s'\n", path);
        return false;
    }
    AVStream *stream = format->streams[stream_index];

    // find & open codec
    AVCodecContext *codec = stream->codec;
    if (avcodec_open2(codec, cdc/*avcodec_find_decoder(codec->codec_id)*/, NULL) < 0) {
        LOGD("Failed to open decoder for stream #%u in file '%s'\n", stream_index, path);
        return false;
    }
    av_opt_set_int(codec, "refcounted_frames", 1, 0);

    // ****************** PREPARE METADATA / AUDIO EXTRACTION ****************** //
    // ************************************************************************* //

    extractMetadata(format, codec);
    _soundSystem->notifyExtractionStarted();
    decodeAudioFile(format, stream, codec, &_extractedData, &_size);
    _soundSystem->setExtractedData(_extractedData);
    return true;
}

void FFmpegSynchronousExtractor::extractMetadata(
        const AVFormatContext *format,
        const AVCodecContext *codec) {
    // extract track information
    _file_duration = format->duration;
    _file_number_channels = codec->channels;
    _file_sample_rate = codec->sample_rate;

    // duration is in micro seconds
    _file_total_frames = (unsigned int) (((double) _file_duration * (double) _frameRate /
                                          1000000.0));

    _extractedData = (short *) calloc(_file_total_frames * _file_number_channels, sizeof(short));
    _soundSystem->setExtractedData(_extractedData);
    _soundSystem->setTotalNumberFrames(_file_total_frames);
}

int FFmpegSynchronousExtractor::decodeAudioFile(
        AVFormatContext *format,
        AVStream *stream,
        AVCodecContext *codec,
        short **data,
        int *size) {

    // prepare resampler
    struct SwrContext *swr = swr_alloc();
    av_opt_set_int(swr, "in_channel_count", codec->channels, 0);
    av_opt_set_int(swr, "out_channel_count", 2, 0);
    av_opt_set_int(swr, "in_channel_layout", codec->channel_layout, 0);
    av_opt_set_int(swr, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
    av_opt_set_int(swr, "in_sample_rate", codec->sample_rate, 0);
    av_opt_set_int(swr, "out_sample_rate", _frameRate, 0);
    av_opt_set_sample_fmt(swr, "in_sample_fmt", codec->sample_fmt, 0);
    av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
    av_opt_set_int(swr, "force_resampling", 1, 0);
    assert(swr_init(swr) == 0);
    if (!swr_is_initialized(swr)) {
        LOGD("Resampler has not been properly initialized\n");
        return -1;
    }

    // prepare to read data
    AVPacket packet;
    av_init_packet(&packet);
    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        LOGD("Error allocating the frame\n");
        return -1;
    }

    // iterate through frames
    *data = NULL;
    *size = 0;

    // https://stackoverflow.com/questions/20545767/decode-audio-from-memory-c
    // Read the packets in a loop
    while (av_read_frame(format, &packet) == 0) {

        if (packet.stream_index == stream->index) {
            AVPacket decodingPacket = packet;

            // Audio packets can have multiple audio frames in a single packet
            while (decodingPacket.size > 0) {

                // Try to decode the packet into a frame(s)
                // Some frames rely on multiple packets, so we have to make sure the frame is finished
                // before utilising it
                int gotFrame = 0;
                int result = avcodec_decode_audio4(codec, frame, &gotFrame, &decodingPacket);

                if (result >= 0 && gotFrame) {
                    decodingPacket.size -= result;

                    // ***************************************************** //
                    // ****************** RESAMPLE FRAMES ****************** //
                    short *buffer;
                    av_samples_alloc(
                            (uint8_t **) &buffer,
                            NULL,
                            2,
                            frame->nb_samples,
                            AV_SAMPLE_FMT_S16,
                            0);
                    int frame_count = swr_convert(
                            swr,
                            (uint8_t **) &buffer,
                            frame->nb_samples,
                            (const uint8_t **) frame->data,
                            frame->nb_samples);
                    // append resampled frames to data
                    *data = (short *) realloc(
                            *data,
                            (*size + frame->nb_samples) * sizeof(short) * 2);
                    memcpy(*data + *size,
                           buffer,
                           frame_count * sizeof(short) * 2);
                    *size += frame_count * 2;
                    // ****************** RESAMPLE FRAMES ****************** //
                    // ***************************************************** //
                } else {
                    decodingPacket.size = 0;
                    decodingPacket.data = nullptr;
                }
            }
        }

        // You MUST call av_free_packet() after each call to av_read_frame()
        // or you will leak so much memory on a large file you will need a memory-plumber!
        // av_packet_unref(&packet);
    }
    /*
    // Some codecs will cause frames to be buffered in the decoding process.
    // If the CODEC_CAP_DELAY flag is set, there can be buffered frames that need to be flushed
    // therefore flush them now....
    if (codec->codec->capabilities & CODEC_CAP_DELAY) {
        av_init_packet(&packet);
        // Decode all the remaining frames in the buffer
        int gotFrame = 0;
        while (avcodec_decode_audio4(codec, frame, &gotFrame, &packet) >= 0 && gotFrame) {
            // Again: a fully decoded audio frame!
        }
    }
    */

    // Clean up! (unless you have a quantum memory machine with infinite RAM....)
    av_frame_free(&frame);
    swr_free(&swr);
    avcodec_close(codec);
    avformat_free_context(format);

    // success
    return 0;
}