#ifndef EXTRACTOR_FILE_UTILS_H
#define EXTRACTOR_FILE_UTILS_H

#include <stdio.h>

static char *pathToDat(const char *inFilePath) {
    char pathOut[500];
    sscanf(inFilePath, "%[^.]", pathOut);  // foo.bar => foo
    sprintf(pathOut, "%s.dat", pathOut);
    return pathOut;
}

static void writeDat(
        const char *pathOut,
        short *writePos,
        size_t written) {
    // Delete out file if exist.
    remove(pathOut);

    FILE *file;
    file = fopen(pathOut, "ab+");
    fwrite(writePos, written, sizeof(uint8_t), file);
}

#endif // EXTRACTOR_FILE_UTILS_H
