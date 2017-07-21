#ifndef EXTRACTOR_TIME_UTILS_H
#define EXTRACTOR_TIME_UTILS_H

#include <linux/time.h>
#include <time.h>

static double currentTimeMillis(void) {
    struct timespec res;
    clock_gettime(CLOCK_REALTIME, &res);
    return 1000.0 * res.tv_sec + (double) res.tv_nsec / 1e6;
}

#endif // EXTRACTOR_TIME_UTILS_H
