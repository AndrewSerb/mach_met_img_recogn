#ifndef IMAGE
#define IMAGE

#include <cstdint>
#include <vector>

struct ImageData
{
    uint16_t n_channels;        // [1, 24]
    uint32_t height;            // [1, 30000], also called "rows"
    uint32_t width;             // [1, 30000], also called "collumns"
    std::vector<std::vector<uint8_t>> channels_data;

    void clear()
    {
        n_channels = height = width = 0;
        channels_data.clear();
    }
    // Could implement iterators over rows/collumns, for convenience
};

#endif
