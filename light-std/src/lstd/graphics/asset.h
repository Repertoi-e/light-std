#pragma once

#include "../file/path.h"
#include "../memory/string.h"

LSTD_BEGIN_NAMESPACE

// We call an asset something with a name and a file path (file path is optional).
// Things like shader and texture inherit this.
struct asset {
    string Name;
    file::path FilePath;
};

LSTD_END_NAMESPACE
