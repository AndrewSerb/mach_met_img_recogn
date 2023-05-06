#ifndef PROC_CTX_H
#define PROC_CTX_H

#include <cstdio>

#include "../processing/pixel_view.h"

using BorderSide = PixelView_3x3::BorderSide;

enum ProcessorType
{
    GRAYSCALE,
    DUOTONE,
    FILL,
    THIN,
    IRREG_CLEANUP,
    TRACE_LETTERS
};

// Used for FILL
struct ProcCtx
{
    size_t count;
    ProcessorType type;

    ProcCtx() = default;
    ProcCtx(ProcessorType type) : type(type), count(0)
    {}
    virtual ~ProcCtx() = default;

    // Inherited classes should call parent's serialize method
    virtual void serialize(FILE*) const;
    // Inherited classes shouldn't call parent's deserialize method,
    // because at the time of reding from file we aren't sure what
    // heir we want to deserialize, and it should be decided ad-hoc
    // Just put file pointer at where relevant info for the class starts,
    // most probably right after parents'
    static ProcCtx deserialize(FILE*);
};

// Used for THIN, IRREG_CLEANUP
struct DirectionalActionCtx : public ProcCtx
{
    BorderSide side;

    DirectionalActionCtx(ProcessorType type, BorderSide side)
        : ProcCtx(type), side(side)
    {}

    DirectionalActionCtx(ProcessorType type, FILE* file)
        : ProcCtx(type)
    {
        deserialize(file);
    }

    void serialize(FILE*) const override;
    bool deserialize(FILE*);
};

// used for DUOTONE
struct ThresholdActionCtx : public ProcCtx
{
    long threshold;

    ThresholdActionCtx(ProcessorType type, long threshold)
        : ProcCtx(type), threshold(threshold)
    {}

    ThresholdActionCtx(ProcessorType type, FILE* file)
        : ProcCtx(type)
    {
        deserialize(file);
    }

    void serialize(FILE*) const override;
    bool deserialize(FILE*);
};

#endif
