#include "../common_processors.h"
#include "../pixel_view.h"

Fill::Fill() : ImageProcessor(), color(BLACK)
{}

void Fill::set_color(unsigned color)
{
    this->color = color;
}

bool Fill::process(ImageData& image)
{
    if (image.n_channels != 1)
        return false;

    bool processed = false;
    PixelView_3x3 v(image);
    auto& pixels = image.channels_data[0];
    unsigned limit;
    unsigned w = image.width;
    unsigned h = image.height;
    unsigned x, y;

    for (unsigned i = 0; i < pixels.size(); ++i)
    {
        if (pixels[i] == color)
            continue;

        x = i % w;
        y = i / w;

        // if it's an edge - out of bound pixels are counted as matching,
        // so this is taken into account
        if (x == 0 || y == 0 || x == w - 1 || y == h - 1)
            limit = 8;
        else
            limit = 5;

        if (v.count_adjacent(i, color) >= limit)
        {
            pixels[i] = color;
            processed = true;
        }
    }

    return processed;
}
