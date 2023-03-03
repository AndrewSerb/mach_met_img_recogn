#include <array>
#include <algorithm>
#include <bit>
#include <cstring>

#include "psd_manager.h"

#define SIGN_8BPS (1397768760) // ASCII string "8BPS"

// making my own because i'm having problems with building the app with c++23 standard
template<class T>
static void byteswap(T& value)
{
    auto value_representation = std::bit_cast<std::array<std::byte, sizeof(T)>>(value);
    std::ranges::reverse(value_representation);
    value = std::bit_cast<T>(value_representation);
}

template<class T>
static void confirm_endianness(T& value)
{
    // all values in PSD files are big endian
    if (std::endian::native == std::endian::big)
        return; // system endianness is the same

    byteswap(value);
}

bool PsdManager::open(const char* filepath)
{
    FILE* file = fopen(filepath, "r");
    if (!file)
        return false;

    int section_idx = 0;
    SectionHandler handler = section_handlers[section_idx];
    while (handler)
    {
        if (!handler(file, image))
            break;

        ++section_idx;
        handler = section_handlers[section_idx];
    }

    fclose(file);
    if (handler) // error in one of the handlers
        return false;
    return true;
}

bool PsdManager::read_file_header(FILE* file, PsdData& image)
{   // read img size and skip the rest
    uint32_t signature = 0;
    int16_t version = 0;

    // 4 byte signature
    if (!fread(&signature, 4, 1, file))
        return false;
    if (signature != SIGN_8BPS)
        return false;
    // 2 byte version
    if (!fread(&version, 2, 1, file))
        return false;
    confirm_endianness(version);
    if (version != 1)
        return false;
    // 6 bytes reserved
    if (fseek(file, 6, SEEK_CUR))
        return false;
    // 2 byte number of chanels
    if (!fread(&image.n_channels, 2, 1, file))
        return false;
    confirm_endianness(image.n_channels);
    // 4 byte height
    if (!fread(&image.height, 4, 1, file))
        return false;
    confirm_endianness(image.height);
    // 4 byte width
    if (!fread(&image.width, 4, 1, file))
        return false;
    confirm_endianness(image.width);
    // 2 byte depth
    if (!fread(&image.depth, 2, 1, file))
        return false;
    confirm_endianness(image.depth);
    // the task only requires 32 bpp, which is 8 bits per channel
    if (image.depth != 8)
        return false;
    // 2 byte color mode, skip
    if (fseek(file, 2, SEEK_CUR))
        return false;

    return true;
}

bool PsdManager::read_color_mode_data(FILE* file, PsdData& image)
{   // skip
    // 4 bytes color data length
    uint32_t len = 0;
    if (!fread(&len, 4, 1, file))
        return false;
    confirm_endianness(len);
    // skip the rest
    if (fseek(file, len, SEEK_CUR))
        return false;

    return true;
}

bool PsdManager::read_image_resources(FILE* file, PsdData& image)
{   // skip
    // 4 bytes resource section length
    uint32_t len = 0;
    if (!fread(&len, 4, 1, file))
        return false;
    confirm_endianness(len);
    // skip the rest
    if (fseek(file, len, SEEK_CUR))
        return false;

    return true;
}

bool PsdManager::read_layer_and_mask_info(FILE* file, PsdData& image)
{   // skip
    // 4 bytes layer and mask info length
    uint32_t len = 0;
    if (!fread(&len, 4, 1, file))
        return false;
    confirm_endianness(len);
    // skip the rest
    if (fseek(file, len, SEEK_CUR))
        return false;

    return true;
}

bool PsdManager::read_image_data(FILE* file, PsdData& image)
{
    // 2 byte compression method
    if (!fread(&image.compression, 2, 1, file))
        return false;
    confirm_endianness(image.compression);

    // image data
    switch (image.compression)
    {
    // only RLE compression is supported
    // PSD uses PackBits implementation of RLE
    // http://fileformats.archiveteam.org/wiki/PackBits
    case PsdData::PSD_COMPR_RLE:
    {
        uint32_t width = image.width;
        uint32_t height = image.height;
        uint64_t num_pixels = width * height;
        uint16_t bytes_per_color = image.depth / 8;
        uint64_t bytes_per_channel = bytes_per_color * num_pixels;

        image.channels_data.assign(image.n_channels, {});

        uint64_t read_bytes = 0;
        int8_t byte_buf = 0;

        // PSD RLE implementation adds data counts per each row per each channel
        // this includes both RLE markers and the data itself
        // skip
        if (fseek(file, height * image.n_channels * 2, SEEK_CUR))
            return false;

        for (std::vector<uint8_t>& channel : image.channels_data)
        {
            channel.assign(bytes_per_channel, 0);
            auto p = channel.data();

            read_bytes = 0;

            while (read_bytes < num_pixels)
            {
                // not converting to big endian because it's only 1 byte long
                // could be a problem if >8 bits per color are used
                if (!fread(&byte_buf, 1, 1, file))
                    return false;

                short length = byte_buf;

                if (length == -128)
                    continue;
                if (length >= 0)
                {
                    ++length;

                    if (!fread(p, 1, length, file))
                        return false;

                    p += length;
                    read_bytes += length;
                }
                else
                {
                    length = 1 - length;

                    if (!fread(&byte_buf, 1, 1, file))
                        return false;
                    memset(p, byte_buf, length);
                    p += length;

                    read_bytes += length;
                }
            }
        }
        break;
    }
    default:
        return false;
    }

    return true;
}
