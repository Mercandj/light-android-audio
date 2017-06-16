/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MINI_SOUND_SYSTEM_LOOPER_H
#define MINI_SOUND_SYSTEM_LOOPER_H


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


#endif //MINI_SOUND_SYSTEM_LOOPER_H


