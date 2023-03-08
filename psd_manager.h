#ifndef PSD_MANAGER_H
#define PSD_MANAGER_H

#include <array>
#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <vector>

// Consulted PSD specification from adobe website:
// https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/#50577409_72092

struct PsdData
{
    enum Compression : uint16_t
    {
        PSD_COMPR_RAW = 0,
        PSD_COMPR_RLE,
        PSD_COMPR_ZIP_NO_PREDICT,
        PSD_COMPR_ZIP_PREDICT
    };

    Compression compression;
    uint16_t n_channels;        // [1, 24]
    uint32_t height;            // [1, 30000], also called "rows"
    uint32_t width;             // [1, 30000], also called "collumns"
    uint16_t depth;             // bits per channel
    std::vector<std::vector<uint8_t>> channels_data;
};

class PsdManager
{
public:
    PsdManager() = default;
    virtual ~PsdManager() = default;

    bool open(const char* filepath);

    inline PsdData& get_image()
    {
        return image;
    }

private:
    PsdData image;

    static bool read_file_header(FILE*, PsdData&);
    static bool read_color_mode_data(FILE*, PsdData&);
    static bool read_image_resources(FILE*, PsdData&);
    static bool read_layer_and_mask_info(FILE*, PsdData&);
    static bool read_image_data(FILE*, PsdData&);

    typedef bool (*SectionHandler)(FILE*, PsdData&);

    static constexpr SectionHandler section_handlers[] =
    {
        read_file_header,
        read_color_mode_data,
        read_image_resources,
        read_layer_and_mask_info,
        read_image_data,
        nullptr
    };
};

#endif // PSD_MANAGER_H