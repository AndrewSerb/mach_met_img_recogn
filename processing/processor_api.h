#ifndef PROCESSOR_API
#define PROCESSOR_API
#include "image.h"

class ImageProcessor
{
public:
    ImageProcessor() = default;
    virtual ~ImageProcessor() = default;

    virtual bool process(ImageData& image)
    {
        return true;
    };
};

#endif
