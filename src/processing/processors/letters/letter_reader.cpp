#include "letter_reader.h"

namespace LetterReader
{
static void add_group_count(std::vector<int>& vec, int num)
{
    // only add unique new entries
    if (vec.size() && vec.back() == num)
        return;
    else
        vec.push_back(num);
}

// note: extracted purely for the sake of not duplicating this code
static void proc_pixel(unsigned idx, const LetterData& letter,
    const std::vector<uint8_t>& image,
    bool& seq_started, int& groups_num, int& group_start_idx)
{
    // only process pixels that are parts of the letter
    if (!letter.pixels_idxs.count(idx))
    {
        if (seq_started)
        {
            seq_started = false;
            groups_num += 1;
            group_start_idx = -1;
        }
        return;
    }

    if (image[idx] == BLACK)
    {
        if (!seq_started)
        {
            seq_started = true;
            group_start_idx = idx;
        }
    }
    else
    {
        if (seq_started)
        {
            seq_started = false;
            groups_num += 1;
            group_start_idx = -1;
        }
    }
}

static void proc_rows(LetterData& letter, const ImageData& image)
{
    for (unsigned r = letter.top_left.y; r <= letter.bottom_right.y; ++r)
    {
        int groups_num = 0;
        int group_start_idx = -1;
        unsigned idx = 0;
        bool seq_started = false;

        for (unsigned c = letter.top_left.x, idx = r * image.width + c;
            c <= letter.bottom_right.x; ++c, ++idx)
        {
            proc_pixel(idx, letter, image.channels_data[0], seq_started,
                groups_num, group_start_idx);
        }

        if (seq_started)
            groups_num += 1;

        if (groups_num == 1 && group_start_idx != -1 &&
            idx - group_start_idx >= letter.width() * LONG_PERCENTAGE)
        {
            groups_num = -1;
        }

        add_group_count(letter.horizontal_lines, groups_num);
    }

    return;
}

static void proc_cols(LetterData& letter, const ImageData& image)
{
    for (unsigned c = letter.top_left.x; c <= letter.bottom_right.x; ++c)
    {
        int groups_num = 0;
        int group_start_idx = -1;
        unsigned idx = 0;
        bool seq_started = false;

        for (unsigned r = letter.top_left.y, idx = r * image.width + c;
            r <= letter.bottom_right.y; ++r, idx += image.width)
        {
            proc_pixel(idx, letter, image.channels_data[0], seq_started,
                groups_num, group_start_idx);
        }

        if (seq_started)
            groups_num += 1;

        if (groups_num == 1 && group_start_idx != -1 &&
            idx / image.width - group_start_idx / image.width >=
            letter.height() * LONG_PERCENTAGE)
        {
            groups_num = -1;
        }

        add_group_count(letter.vertical_lines, groups_num);
    }

    return;
}

void detect(LetterData& letter, const ImageData& image)
{
    proc_rows(letter, image);
    proc_cols(letter, image);
}
}
