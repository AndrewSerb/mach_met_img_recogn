#ifndef PIXEL_VIEW_H
#define PIXEL_VIEW_H

#include "image.h"

struct Point
{
    int x = 0;
    int y = 0;

    Point() : x(0), y(0)
    {}
    Point(int x, int y) : x(x), y(y)
    {}

    inline Point offset(int x, int y)
    {
        return Point(this->x + x, this->y + y);
    }

    inline int to_linear(unsigned width)
    {
        return point_to_linear(*this, width);
    }

    inline static Point from_linear(int coord, unsigned width)
    {
        return Point(coord % width, coord / width);
    }

    static int point_to_linear(const Point& coord, unsigned width)
    {
        return coord.y * width + coord.x;
    }
};

class PixelView_3x3
{
public:
    enum BorderSide
    {
        TOP,
        RIGHT,
        BOTTOM,
        LEFT
    };

    PixelView_3x3(const ImageData&);

    unsigned count_adjacent(unsigned, unsigned) const;
    bool is_letter_border(unsigned, BorderSide) const;
    bool is_irregularity(unsigned, BorderSide) const;

private:
    const ImageData& image;

    bool check_pixel_color(Point idx, unsigned color,
        bool out_of_bounds_value) const;
};

#endif // PIXELVIEW_H
