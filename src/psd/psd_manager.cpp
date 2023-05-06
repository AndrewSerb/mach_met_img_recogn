#include "psd_manager.h"

PsdData::PsdData() : image(), n_channels(image.n_channels),
    width(image.width), height(image.height),
    channels_data(image.channels_data)
{}

bool PsdManager::open(const char* filepath)
{
    FILE* file = fopen(filepath, "rb");
    if (!file)
        return false;

    path = filepath;

    int section_idx = 0;
    SectionReader reader = section_readers[section_idx];
    while (reader)
    {
        if (!reader(file, image))
            break;

        ++section_idx;
        reader = section_readers[section_idx];
    }

    fclose(file);
    if (reader) // error in one of the handlers
        return false;
    return true;
}

bool PsdManager::save() const
{
    // only write header and image data
    FILE* file = fopen(path.c_str(), "wb");
    if (!file)
        return false;

    int section_idx = 0;
    SectionWriter writer = section_writers[section_idx];
    while (writer)
    {
        if (!writer(file, image))
            break;

        ++section_idx;
        writer = section_writers[section_idx];
    }

    fclose(file);
    if (writer) // error in one of the handlers
        return false;

    return true;
}
