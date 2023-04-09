#ifndef PSD_MANAGER_H
#define PSD_MANAGER_H

#include <array>
#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <vector>

#include "../processing/image.h"

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
    ImageData image;
    uint16_t& n_channels;       // [1, 24]
    uint32_t& height;           // [1, 30000], also called "rows"
    uint32_t& width;            // [1, 30000], also called "collumns"
    uint16_t depth;             // bits per channel
    uint16_t color_mode;        // [0, 9], only 3 (RGB) is supported
    std::vector<std::vector<uint8_t>>& channels_data;

    PsdData();
    inline ImageData& get_raw()
    {
        return image;
    }
};

class PsdManager
{
public:
    PsdManager() = default;
    virtual ~PsdManager() = default;

    bool open(const char* filepath);
    bool save() const;

    inline PsdData& get_image()
    {
        return image;
    }

    inline void set_save_path(const char* path)
    {
        this->path = path;
    }

private:
    PsdData image;
    std::string path;

    static bool read_file_header(FILE*, PsdData&);
    static bool read_color_mode_data(FILE*, PsdData&);
    static bool read_image_resources(FILE*, PsdData&);
    static bool read_layer_and_mask_info(FILE*, PsdData&);
    static bool read_image_data(FILE*, PsdData&);

    static bool write_file_header(FILE*, const PsdData&);
    static bool write_color_mode_data(FILE*, const PsdData&);
    static bool write_image_resources(FILE*, const PsdData&);
    static bool write_layer_and_mask_info(FILE*, const PsdData&);
    static bool write_image_data(FILE*, const PsdData&);

    typedef bool (*SectionReader)(FILE*, PsdData&);
    typedef bool (*SectionWriter)(FILE*, const PsdData&);

    static constexpr SectionReader section_readers[] =
    {
        read_file_header,
        read_color_mode_data,
        read_image_resources,
        read_layer_and_mask_info,
        read_image_data,
        nullptr
    };

    static constexpr SectionWriter section_writers[] =
    {
        write_file_header,
        write_color_mode_data,
        write_image_resources,
        write_layer_and_mask_info,
        write_image_data,
        nullptr
    };
};

#endif // PSD_MANAGER_H
