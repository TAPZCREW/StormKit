// Copyright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <sys/timerfd.h>
#include <unistd.h>

export module stormkit.wsi:linux.common.fd;

import stormkit.core;

export namespace stormkit::wsi::linux::common {
    using FD = stormkit::RAIICapsule<i32, monadic::noop(), close, struct FDTag, -1>;
}
