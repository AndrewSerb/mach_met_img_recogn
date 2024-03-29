#ifndef LETTER_READER_H
#define LETTER_READER_H

#include <unordered_map>

#include "../../common_processors.h"

namespace LetterReader
{
constexpr double LONG_PERCENTAGE = 0.8;

static const std::unordered_map<char, LetterData::LinesMetrics> etalons = {
    {'.', {{-1}, {-1}}},

    {'a', {{1, 2, 1, 2, -1, 2}, {1, 2, 3, 1, -1}}},
    {'b', {{1, 2, 1, 2, 1}, {1, -1, 2, 1}}},
    {'c', {{1, 2, 1, 2, 1}, {1, 2}}},
    {'d', {{1, 2, 1, 2, 1, 2}, {1, 2, 3, 1, -1}}},
    {'e', {{1, 2, -1, 1, 2, -1, 1}, {1, 3, 2}}},
    {'f', {{-1, 2, 1, 2, 1}, {-1, 2, -1, 3, 1}}},
    {'g', {{-1, 2, 1, -1, 2, 1}, {3, 5, 4, 3, 4, 3, 2}}},
    {'h', {{1, 2, 1, 2}, {2, -1, 2, 1, 2, -1}}},
    {'i', {{1, -1}, {2, -1}}},
    {'j', {{-1, 2, -1, 1, 2, 1}, {1, -1, 2, 1}}},
    {'k', {{1, 2, 1, 2, 1}, {2, 1, 2, 1, 2, 3, 2}}},
    {'l', {{1, -1}, {2, -1}}},
    {'m', {{3}, {2, 1, 2, -1, 2, 1, 2, -1}}},
    {'n', {{2}, {2, -1, 2, 1, 2, -1}}},
    {'o', {{1, 2, 1}, {1, 2, 1}}},
    {'p', {{2, 1, 2, 1, 2, 1}, {2, -1, 3, 2, 1}}},
    {'q', {{2, 1, -1}, {1, 2, 3, -1}}},
    {'r', {{2, 1}, {2, -1, 2, 1}}},
    {'s', {{1, -1, 2, 1, -1, 2}, {2, 3, 2}}},
    {'t', {{1, -1, 1, -1, 1}, {1, 2}}},
    {'u', {{2, 1, 2}, {1, -1, 1, 2, 1, -1}}},
    {'v', {{2, 1}, {1, 2, -1, 1, 2, 1}}},
    {'w', {{3, 4, 2}, {1, 2, -1, 1, -1, 1, 2, 1}}},
    {'x', {{2, 1, 2}, {2, 1, 2}}},
    {'y', {{2, 1, 2, 1}, {1, 2, 3, 1, 2, 1}}},
    {'z', {{-1, 1, -1}, {2}}},

    {'A', {{}, {}}},
    {'B', {{1, 2, 1, 2, 1}, {2, -1, 3, 3, 1}}},
    {'C', {{1, 2, 1, 2, 1}, {1, 2}}},
    {'D', {{1, 2, 1}, {2, -1, 2, 1}}},
    {'E', {{1, 2, 1, 2, 1, 2, 1, 2, 1, 2}, {2, -1, 3, 2, 1}}},
    {'F', {{1, 2, 1, 2, 1, 2, 1}, {2, -1, 3, 2, 1}}},
    {'G', {{2, 1, 2, 1, 2, 1}, {1, 2, 3, 2, 1}}},
    {'H', {{2, 1, 2}, {2, -1, 3, 1, 3, -1, 2}}},
    {'S', {{-1, 2, 1, -1, 2}, {2, 3, 2}}},
    {'T', {{1, -1, 3, 1}, {1, 2, -1, 2, 1}}},
};

void detect(LetterData& letter, const ImageData& image);
}

#endif // LETTER_READER_H
