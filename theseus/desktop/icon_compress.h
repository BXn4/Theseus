// Shrinks the cached art in Configs/icons. Scraped box art lands at print
// resolution (1480x1080 typical) and gets drawn on a small orb.

#pragma once

#include <stddef.h>

struct IconCompressResult {
    int    scanned;
    int    converted;
    size_t bytesBefore;
    size_t bytesAfter;
};

// Re-encode Configs/icons to fit maxDim on the long side. Already-small files
// are skipped, so a second run costs one decode per new file.
IconCompressResult Icons_Compress(int maxDim, int quality);
