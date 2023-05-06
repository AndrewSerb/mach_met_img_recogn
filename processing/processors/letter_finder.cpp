#include "../common_processors.h"

LetterFinder::LetterFinder() : color(BLACK), idx(0)
{}

const std::vector<LetterData>& LetterFinder::get_letters() const
{
    return letters;
}
const LetterData& LetterFinder::last_letter() const
{
    return letters.back();
}

void LetterFinder::clear()
{
    letters.clear();
    idx = 0;
}

static bool letters_contain(const std::vector<LetterData>& letters, unsigned idx)
{
    for (auto& letter : letters)
        if (letter.pixels_idxs.contains(idx))
            return true;
    return false;
}

static void check_pixel(LetterData& letter, unsigned idx,
    const ImageData& image, unsigned letter_color)
{
    if (image.channels_data[0][idx] != letter_color || letter.pixels_idxs.contains(idx))
        return;

    letter.pixels_idxs.insert(idx);

    Point p = Point::from_linear(idx, image.width);

    if (p.x < letter.top_left.x)
        letter.top_left.x = p.x;
    if (p.x > letter.bottom_right.x)
        letter.bottom_right.x = p.x;

    if (p.y < letter.top_left.y)
        letter.top_left.y = p.y;
    if (p.y > letter.bottom_right.y)
        letter.bottom_right.y = p.y;

    if (p.y < image.height - 1)
        check_pixel(letter, p.offset( 0, 1).to_linear(image.width), image, letter_color);
    if (p.y > 0)
        check_pixel(letter, p.offset( 0,-1).to_linear(image.width), image, letter_color);
    if (p.x < image.width - 1)
        check_pixel(letter, p.offset( 1, 0).to_linear(image.width), image, letter_color);
    if (p.x > 0)
        check_pixel(letter, p.offset(-1, 0).to_linear(image.width), image, letter_color);

    if (p.y < image.height - 1 && p.x < image.width - 1)
        check_pixel(letter, p.offset( 1, 1).to_linear(image.width), image, letter_color);
    if (p.y > 0 && p.x < image.width - 1)
        check_pixel(letter, p.offset( 1,-1).to_linear(image.width), image, letter_color);
    if (p.x > 0 && p.y < image.height - 1)
        check_pixel(letter, p.offset(-1, 1).to_linear(image.width), image, letter_color);
    if (p.x > 0 && p.y > 0)
        check_pixel(letter, p.offset(-1,-1).to_linear(image.width), image, letter_color);
}

// TODO: rewrite without recursion || fortify against stack overflow
static void trace_letter(std::vector<LetterData>& letters,
    const ImageData& image, unsigned idx, unsigned letter_color)
{
    LetterData& letter = letters.emplace_back();
    letter.top_left.x = letter.top_left.y = INT32_MAX;


    check_pixel(letter, idx, image, letter_color);
}

bool LetterFinder::process(ImageData& image)
{
    if (image.n_channels != 1)
        return false;

    auto& pixels = image.channels_data[0];

    if (idx >= pixels.size())
        return false;

    if (pixels[idx] == color && !letters_contain(letters, idx))
        trace_letter(letters, image, idx, color);

    ++idx;

    return true;
}
