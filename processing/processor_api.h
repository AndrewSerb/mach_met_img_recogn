#ifndef PROCESSOR_API
#define PROCESSOR_API
#include "image.h"

// TODO: could implement "command" pattern instead, to discontinue a mirroring
// "ProcCtx" hierarchy for history saving. but i'm not gonna bother

class ImageProcessor
{
public:
    ImageProcessor() = default;
    virtual ~ImageProcessor() = default;

    virtual bool process(ImageData& image)
    {
        return false;
    };
};

#endif
