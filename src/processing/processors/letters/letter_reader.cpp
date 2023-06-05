#include "letter_reader.h"

namespace LetterReader
{

static size_t hamming_distance(const std::vector<int>& v1, const std::vector<int>& v2)
{
    size_t dist = 0;
    size_t proc_size = std::min(v1.size(), v2.size());

    for (size_t i = 0; i < proc_size; ++i)
        if (v1[i] == v2[i])
            ++dist;

    return dist;
}

static size_t (*metrics_compare)(const std::vector<int>&,
    const std::vector<int>&) = hamming_distance;

// returns similarity percentage, [0; 1]
static double compare_letters(const LetterData::LinesMetrics& l1,
    const LetterData::LinesMetrics& l2)
{
    size_t horizontal_dist = metrics_compare(l1.first, l2.first);
    size_t vertical_dist = metrics_compare(l1.second, l2.second);

    size_t sum = horizontal_dist + vertical_dist;
    auto uh = std::max(l1.first.size(), l2.first.size()) +
            std::max(l1.second.size(), l2.second.size());

    return (horizontal_dist + vertical_dist) /
        (double)(std::max(l1.first.size(), l2.first.size()) +
        std::max(l1.second.size(), l2.second.size()));
}

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

        add_group_count(letter.metrics.first, groups_num);
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

        add_group_count(letter.metrics.second, groups_num);
    }

    return;
}

static void determine_chars(LetterData& letter)
{
    // compare letter with every available etalon
    for (auto& etalon : etalons)
        letter.similarity_values.insert({
            compare_letters(letter.metrics, etalon.second), etalon.first});
}

void detect(LetterData& letter, const ImageData& image)
{
    proc_rows(letter, image);
    proc_cols(letter, image);
    determine_chars(letter);
}
}
