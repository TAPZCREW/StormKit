// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <sys/timerfd.h>
#include <unistd.h>

export module stormkit.wsi:linux.common.fd;

import stormkit.core;

export namespace stormkit::wsi::linux::common {
    using fd = stormkit::raii_capsule<i32, monadic::noop(), close, struct fd_tag, -1>;
}
