#include "assert.hpp"

import stormkit.core;

void vma_assert(bool val, char* expr) {
    stormkit::assert(val, expr);
}
