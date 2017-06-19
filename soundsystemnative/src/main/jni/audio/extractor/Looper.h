
#ifdef MEDIACODEC_EXTRACTOR

#ifndef SOUND_SYSTEM_AUDIO_EXTRACTOR_LOOPER_H
#define SOUND_SYSTEM_AUDIO_EXTRACTOR_LOOPER_H

#include <pthread.h>
#include <semaphore.h>

#include <assert.h>
#include <jni.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <semaphore.h>
#include <utils/android_debug.h>

typedef struct LooperMessage {
    int what;
    void *obj;
    LooperMessage *next;
    bool quit;
} LooperMessage;

class Looper {
public:
    Looper();

    Looper &operator=(const Looper &) = delete;

    Looper(Looper &) = delete;

    virtual ~Looper();

    void post(int what, void *data, bool flush = false);

    void quit();

    virtual void handle(int what, void *data);

private:
    void addmsg(LooperMessage *msg, bool flush);

    static void *trampoline(void *p);

    void loop();

    LooperMessage *head;
    pthread_t worker;
    sem_t headwriteprotect;
    sem_t headdataavailable;
    bool running;
};

#endif // SOUND_SYSTEM_AUDIO_EXTRACTOR_LOOPER_H

#endif // MEDIACODEC_EXTRACTOR
