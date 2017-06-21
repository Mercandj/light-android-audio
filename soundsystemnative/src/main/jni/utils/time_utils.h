//
// Created by Vincent on 15/06/2017.
//

#ifndef MEDIAEXTRACTOR_SAMPLE_TIME_UTILS_H
#define MEDIAEXTRACTOR_SAMPLE_TIME_UTILS_H

#include <linux/time.h>
#include <time.h>

static double now_ms(void) {
    struct timespec res;
    clock_gettime(CLOCK_REALTIME, &res);
    return 1000.0 * res.tv_sec + (double) res.tv_nsec / 1e6;
}

#endif //MEDIAEXTRACTOR_SAMPLE_TIME_UTILS_H
