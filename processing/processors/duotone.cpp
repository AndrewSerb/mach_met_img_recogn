#include "../common_processors.h"

Duotone::Duotone() : split_value(127)
{}

void Duotone::set_split_value(unsigned value)
{
    split_value = value;
}

unsigned Duotone::get_split_value() const
{
    return split_value;
}

const ImageData& Duotone::get_preview() const
{
    return preview;
}

void Duotone::clear_preview()
{
    preview.clear();
}

// expects the image to already be grayscaled
bool Duotone::process(ImageData& image)
{
    if (image.n_channels != 1)
        return false;

    preview = image;

    std::vector<uint8_t>& pix = preview.channels_data[0];

    for (uint8_t& pixel : pix)
    {
        if (pixel >= split_value)
            pixel = WHITE;
        else
            pixel = BLACK;
    }

    return true;
}
