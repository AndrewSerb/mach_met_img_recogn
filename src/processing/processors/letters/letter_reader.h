#ifndef LETTER_READER_H
#define LETTER_READER_H

#include <unordered_set>

#include "../../common_processors.h"

namespace LetterReader
{
    constexpr double LONG_PERCENTAGE = 0.8;

    void detect(LetterData& letter, const ImageData& image);
}

#endif // LETTER_READER_H
