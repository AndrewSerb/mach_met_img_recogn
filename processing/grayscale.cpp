#include "common_processors.h"

// color weights taken from https://en.wikipedia.org/wiki/Grayscale
// there will only be one grayscaled channel after the processing is done
bool Grayscale::process(ImageData& image)
{
    if (image.n_channels < 3)
        return false;

    const std::vector<uint8_t>& r = image.channels_data[0];
    const std::vector<uint8_t>& g = image.channels_data[1];
    const std::vector<uint8_t>& b = image.channels_data[2];

    std::vector<uint8_t>& gray = image.channels_data[0];

    for (unsigned i = 0; i < gray.size(); ++i)
    {
        gray[i] = 0.299 * r[i] + 0.587 * g[i] + 0.114 * b[i];
    }

    for (unsigned i = 1; i < image.n_channels; ++i)
    {
        image.channels_data.pop_back();
    }
    image.n_channels = 1;

    return true;
}
