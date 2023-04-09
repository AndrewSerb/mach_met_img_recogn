#ifndef TEST_MAKE_RED_PROC
#define TEST_MAKE_RED_PROC
#include "processor_api.h"

class MakeRed80Percent : public ImageProcessor
{
public:
    ~MakeRed80Percent() override = default;

    bool process(ImageData&) override;
};

#endif
