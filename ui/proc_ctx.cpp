#include "proc_ctx.h"

void ProcCtx::serialize(FILE* file) const
{
    fwrite(&type, sizeof(type), 1, file);
    fwrite(&count, sizeof(count), 1, file);
}

ProcCtx ProcCtx::deserialize(FILE* file)
{
    ProcCtx ctx;

    fread(&ctx.type, sizeof(ctx.type), 1, file);
    fread(&ctx.count, sizeof(ctx.count), 1, file);

    return ctx;
}

void DirectionalActionCtx::serialize(FILE* file) const
{
    ProcCtx::serialize(file);
    fwrite(&side, sizeof(side), 1, file);
}

bool DirectionalActionCtx::deserialize(FILE* file)
{
    if (!fread(&side, sizeof(side), 1, file))
        return false;
    return true;
}

void ThresholdActionCtx::serialize(FILE* file) const
{
    ProcCtx::serialize(file);
    fwrite(&threshold, sizeof(threshold), 1, file);
}

bool ThresholdActionCtx::deserialize(FILE* file)
{
    if (!fread(&threshold, sizeof(threshold), 1, file))
        return false;
    return true;
}
