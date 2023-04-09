#include "psd_manager.h"

#include <array>
#include <algorithm>
#include <bit>
#include <cstring>

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
    // 2 byte color mode,
    if (!fread(&image.color_mode, 2, 1, file))
        return false;
    confirm_endianness(image.color_mode);
    // only Grayscale and RGB are supported
    if (image.color_mode != 1 && image.color_mode != 3)
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

bool PsdManager::write_file_header(FILE* file, const PsdData& image)
{
    // signature
    fputs("8BPS", file);
    // version - 0 1, big endian
    uint8_t one_byte_buf = 0;
    uint16_t two_byte_buf = 0;
    uint32_t four_byte_buf = 0;
    fwrite(&one_byte_buf, 1, 1, file);
    one_byte_buf = 1;
    fwrite(&one_byte_buf, 1, 1, file);
    // 6 reserved
    one_byte_buf = 0;
    fwrite(&one_byte_buf, 1, 6, file);
    // 2 byte number of channels - big endian
    two_byte_buf = image.n_channels;
    confirm_endianness(two_byte_buf);
    fwrite(&two_byte_buf, 2, 1, file);
    // 4 byte height - big endian
    four_byte_buf = image.height;
    confirm_endianness(four_byte_buf);
    fwrite(&four_byte_buf, 4, 1, file);
    // 4 byte width - big endian
    four_byte_buf = image.width;
    confirm_endianness(four_byte_buf);
    fwrite(&four_byte_buf, 4, 1, file);
    // 2 byte depth - big endian
    two_byte_buf = image.depth;
    confirm_endianness(two_byte_buf);
    fwrite(&two_byte_buf, 2, 1, file);
    // 2 byte color mode - big endian
    two_byte_buf = image.color_mode;
    confirm_endianness(two_byte_buf);
    fwrite(&two_byte_buf, 2, 1, file);

    return true;
}

bool PsdManager::write_color_mode_data(FILE* file, const PsdData&)
{
    // write as empty, 4 byte length of 0
    uint32_t zero = 0;
    fwrite(&zero, 4, 1, file);

    return true;
}

bool PsdManager::write_image_resources(FILE* file, const PsdData&)
{
    // write as empty, 4 byte length of 0
    uint32_t zero = 0;
    fwrite(&zero, 4, 1, file);

    return true;
}

bool PsdManager::write_layer_and_mask_info(FILE* file, const PsdData&)
{
    // write as empty, 4 byte length of 0
    uint32_t zero = 0;
    fwrite(&zero, 4, 1, file);

    return true;
}

bool PsdManager::write_image_data(FILE* file, const PsdData& image)
{
    // 2 byte compression method
    // at this point only 1 value could be used here
    PsdData::Compression compr = PsdData::PSD_COMPR_RLE;
    confirm_endianness(compr);
    fwrite(&compr, 2, 1, file);

    // 2 byte data lengths per row per channel
    // since these need to be written before the data itself, the data
    // should be encoded before determining its length
    uint64_t rows_lengths_count = image.height * image.n_channels;
    uint16_t row_len = 0;
    // leave blank and move file pointer back after determining lengths
    fpos_t lengths_section;
    fgetpos(file, &lengths_section);

    fwrite(&row_len, 2, rows_lengths_count, file);

    // RLE encoded rows
    uint8_t run_length = 0;
    uint8_t color_value = 0;
    int8_t marker = 0;
    size_t idx = 0;

    for (auto& channel : image.channels_data)
    {
        idx = 0;

        for (int r = 0, c = 0; r < image.height; ++r)
        {
            row_len = 0;
            c = 0;

            while (c < image.width)
            {
                // count amount of the same consecutive values
                run_length = 1;
                color_value = channel[idx++];
                ++c;

                while (color_value == channel[idx] && c < image.width
                    && run_length <= 127)
                {
                    ++idx;
                    ++c;
                    ++run_length;
                }

                if (run_length > 1)
                {
                    // repeat `color_value` `run_length` times
                    marker = 1 - run_length;
                    fwrite(&marker, 1, 1, file);
                    fwrite(&color_value, 1, 1, file);
                    row_len += 2;
                }
                else
                {
                    size_t start_idx = idx - 1;
                    // count amount of consecutive unique values
                    while (color_value != channel[idx] && c < image.width
                        && run_length <= 128)
                    {
                        color_value = channel[idx++];
                        ++c;
                        ++run_length;
                    }
                    // write `run_length` unique values
                    marker = run_length - 1;
                    fwrite(&marker, 1, 1, file);
                    fwrite(&channel[start_idx], 1, run_length, file);
                    row_len += run_length + 1;
                }
            }

            fsetpos(file, &lengths_section);
            confirm_endianness(row_len);
            fwrite(&row_len, 2, 1, file);
            fgetpos(file, &lengths_section);
            fseek(file, 0, SEEK_END);
        }
    }

    return true;
}
