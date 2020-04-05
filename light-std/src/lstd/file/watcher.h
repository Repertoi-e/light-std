#pragma once

#include "handle.h"

LSTD_BEGIN_NAMESPACE

namespace file {

// Watches for file changes in a given directory
struct watcher {
    const path Path;
    delegate<void()> Callback;

    
};

}  // namespace file

LSTD_END_NAMESPACE
