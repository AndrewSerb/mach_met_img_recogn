#ifndef COMMON_PROCESSORS_H
#define COMMON_PROCESSORS_H

#include <unordered_set>

#include "processor_api.h"
#include "pixel_view.h"

class Grayscale : public ImageProcessor
{
public:
    ~Grayscale() override = default;

    bool process(ImageData&) override;
};

enum Colors
{
    BLACK = 0,
    WHITE = 255
};

class Duotone : public ImageProcessor
{
public:
    Duotone();
    ~Duotone() override = default;

    bool process(ImageData&) override;

    void set_split_value(unsigned);
    unsigned get_split_value() const;

    void init_preview(const ImageData&);
    const ImageData& get_preview() const;
    void clear_preview();

private:
    unsigned split_value; // [0; 255]
    ImageData preview;
};

class Fill : public ImageProcessor
{
public:
    Fill();
    ~Fill() override = default;

    bool process(ImageData&) override;
    void set_color(unsigned);

private:
    unsigned color;
};

class DirectionalPrcessor : public ImageProcessor
{
public:
    DirectionalPrcessor() = default;
    DirectionalPrcessor(PixelView_3x3::BorderSide side)
        : side(side)
    {}
    virtual ~DirectionalPrcessor() = default;

    inline void set_side(PixelView_3x3::BorderSide side)
    {
        this->side = side;
    }

protected:
    PixelView_3x3::BorderSide side;
};

class ThinLetters : public DirectionalPrcessor
{
public:
    ThinLetters();
    ~ThinLetters() override = default;

    bool process(ImageData&) override;

private:
    unsigned color; // "clear" color

    bool process_pixel(const PixelView_3x3&, unsigned, uint8_t&);
};

class IrregCleanup : public DirectionalPrcessor
{
public:
    IrregCleanup();
    ~IrregCleanup() override = default;

    bool process(ImageData&) override;

private:
    unsigned color; // "clear" color

    bool process_pixel(const PixelView_3x3&, unsigned, uint8_t&);
};

struct LetterData
{
    Point top_left;
    Point bottom_right;

    std::unordered_set<unsigned> pixels_idxs;

    LetterData() = default;

    size_t width() const
    {
        return bottom_right.x - top_left.x;
    }

    size_t height() const
    {
        return bottom_right.y - top_left.y;
    }
};

class LetterFinder : public ImageProcessor
{
public:
    LetterFinder();
    ~LetterFinder() override = default;

    bool process(ImageData&) override;

    const std::vector<LetterData>& get_letters() const;
    const LetterData& last_letter() const;
    void clear();

private:
    unsigned color; // letter color
    std::vector<LetterData> letters;
    // This processor processes a single letter at a time,
    // saving its state
    unsigned idx;
};

#endif // COMMON_PROCESSORS_H
