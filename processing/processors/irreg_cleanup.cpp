#include "../common_processors.h"

IrregCleanup::IrregCleanup() : DirectionalPrcessor(PixelView_3x3::TOP), color(WHITE)
{}

bool IrregCleanup::process_pixel(const PixelView_3x3& view, unsigned idx,
    uint8_t& pixel)
{
    if (pixel == color)
        return false;

    if (view.is_irregularity(idx, side))
    {
        pixel = color;
        return true;
    }

    return false;
}

bool IrregCleanup::process(ImageData& image)
{
    if (image.n_channels != 1)
        return false;

    bool processed = false;
    PixelView_3x3 v(image);
    auto& pixels = image.channels_data[0];

    switch (side)
    {
    case PixelView_3x3::TOP:
    {
        unsigned idx = 0;
        for (long r = image.height - 1; r >= 0 ; --r)
            for (unsigned c = 0; c < image.width; ++c)
            {
                idx = r * image.width + c;
                processed = process_pixel(v, idx, pixels[idx]) || processed;
            }
    }
        break;
    case PixelView_3x3::RIGHT:
        for (unsigned i = 0; i < pixels.size(); ++i)
        {
            processed = process_pixel(v, i, pixels[i]) || processed;
        }
        break;
    case PixelView_3x3::BOTTOM:
    {
        unsigned idx = 0;
        for (unsigned r = 0; r < image.height ; ++r)
            for (unsigned c = 0; c < image.width; ++c)
            {
                idx = r * image.width + c;
                processed = process_pixel(v, idx, pixels[idx]) || processed;
            }
    }
        break;
    case PixelView_3x3::LEFT:
    {
        unsigned idx = 0;
        for (unsigned r = 0; r < image.height ; ++r)
            for (long c = image.width - 1; c >= 0 ; --c)
            {
                idx = r * image.width + c;
                processed = process_pixel(v, idx, pixels[idx]) || processed;
            }
    }
        break;
    }

    return processed;
}
