#include "pixel_view.h"

PixelView_3x3::PixelView_3x3(const ImageData& image)
    : image(image)
{}

bool PixelView_3x3::check_pixel_color(Point coord, unsigned color, bool out_of_bounds_value) const
{
    if (coord.x < 0 || coord.x > image.width ||
        coord.y < 0 || coord.y > image.height)
        return out_of_bounds_value;

    return image.channels_data[0][coord.to_linear(image.width)] == color;
}

unsigned PixelView_3x3::count_adjacent(unsigned idx, unsigned color) const
{
    unsigned count = 0;

    Point coord = Point::from_linear(idx, image.width);
    bool oob_value = true;

    count +=
        (unsigned)check_pixel_color(coord.offset(-1, -1), color, oob_value) +
        (unsigned)check_pixel_color(coord.offset( 0, -1), color, oob_value) +
        (unsigned)check_pixel_color(coord.offset( 1, -1), color, oob_value) +
        (unsigned)check_pixel_color(coord.offset(-1,  0), color, oob_value) +
        (unsigned)check_pixel_color(coord.offset( 1,  0), color, oob_value) +
        (unsigned)check_pixel_color(coord.offset(-1,  1), color, oob_value) +
        (unsigned)check_pixel_color(coord.offset( 0,  1), color, oob_value) +
        (unsigned)check_pixel_color(coord.offset( 1,  1), color, oob_value);

    return count;
}

bool PixelView_3x3::is_letter_border(unsigned idx, BorderSide side) const
{
    unsigned color = 0;
    bool oob_value = true;

    Point coord = Point::from_linear(idx, image.width);
    bool tl = check_pixel_color(coord.offset(-1, -1), color, oob_value);//not used in right, bottom
    bool t  = check_pixel_color(coord.offset( 0, -1), color, oob_value);
    bool tr = check_pixel_color(coord.offset( 1, -1), color, oob_value);//not used in bottom, left
    bool l  = check_pixel_color(coord.offset(-1,  0), color, oob_value);
    bool r  = check_pixel_color(coord.offset( 1,  0), color, oob_value);
    bool bl = check_pixel_color(coord.offset(-1,  1), color, oob_value);//not used in top, right
    bool b  = check_pixel_color(coord.offset( 0,  1), color, oob_value);
    bool br = check_pixel_color(coord.offset( 1,  1), color, oob_value);//not used in top, left

    // the pixel being checked here is assumed to have a value of "color"
    switch (side)
    {
    case TOP:
    {
        bool is_top = !t && b;
        return is_top || is_top && (!tr && l || !tl && r && l && r);
    }
        break;
    case RIGHT:
    {
        bool is_right = !r && l;
        return is_right || is_right && (!br && t || !tr && b || t && b);
    }
        break;
    case BOTTOM:
    {
        bool is_bottom = !b && t;
        return is_bottom || is_bottom && (!bl && r || !br && l || r && l);
    }
        break;
    case LEFT:
    {
        bool is_left = !l && r;
        return is_left || is_left && (!tl && b || ~bl && t || t && b);
    }
        break;
    default:
        return false;
    }
}

bool PixelView_3x3::is_irregularity(unsigned idx, BorderSide side) const
{
    unsigned color = 0;
    bool oob_value = true;

    Point coord = Point::from_linear(idx, image.width);
    bool tl = check_pixel_color(coord.offset(-1, -1), color, oob_value);
    bool t  = check_pixel_color(coord.offset( 0, -1), color, oob_value);
    bool tr = check_pixel_color(coord.offset( 1, -1), color, oob_value);
    bool l  = check_pixel_color(coord.offset(-1,  0), color, oob_value);
    bool r  = check_pixel_color(coord.offset( 1,  0), color, oob_value);
    bool bl = check_pixel_color(coord.offset(-1,  1), color, oob_value);
    bool b  = check_pixel_color(coord.offset( 0,  1), color, oob_value);
    bool br = check_pixel_color(coord.offset( 1,  1), color, oob_value);

    // the pixel being checked here is assumed to have a value of "color"
    switch (side)
    { // TODO: do these actually account for stray pixels?
    case TOP:
        return !tr && !t && !tl && !l && !r && (!bl || b); // TODO: should this also have br somewhere?
    case RIGHT:
        return !b && !br && !r && !tr && !t && (!tl || l);
    case BOTTOM:
        return !l && !bl && !b && !br && !r && (!tr || t);
    case LEFT:
        return !t && !tl && !l && !bl && !b && (!br || r);
    default:
        return false;
    }
}
