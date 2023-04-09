#ifndef COMMON_PROCESSORS_H
#define COMMON_PROCESSORS_H

#include "processor_api.h"

class Grayscale : public ImageProcessor
{
public:
    ~Grayscale() override = default;

    bool process(ImageData&) override;
};


#endif // COMMON_PROCESSORS_H
