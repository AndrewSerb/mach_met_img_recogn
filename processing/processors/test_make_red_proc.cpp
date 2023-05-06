#include "test_make_red_proc.h"

bool MakeRed80Percent::process(ImageData& img)
{
    if (img.n_channels < 1)
        return false;

    std::vector<uint8_t>& red = img.channels_data[0];
    std::fill(red.begin(), red.end(), 205);

    return true;
}
