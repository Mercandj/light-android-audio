#ifndef EXTRACTOR_ANDROID_UTILS_H
#define EXTRACTOR_ANDROID_UTILS_H

#include <sys/system_properties.h>

#define ANDROID_OS_BUILD_VERSION_SDK "ro.build.version.sdk"

static int androidVersionSdk(void) {
    char osVersion[PROP_VALUE_MAX + 1];
    __system_property_get(ANDROID_OS_BUILD_VERSION_SDK, osVersion);
    return atoi(osVersion);
}

#endif // EXTRACTOR_TIME_UTILS_H
