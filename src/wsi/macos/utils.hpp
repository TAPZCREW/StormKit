// Copyright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#ifndef UTILS_HPP
#define UTILS_HPP

namespace stormkit::wsi::macos {
    auto mouse_button(int button) noexcept -> int;
    auto localized_key(char code) noexcept -> int;
    auto non_localized_key(unsigned short code) noexcept -> int;
    auto usage_to_virtual_code(int usage) noexcept -> int;
} // namespace stormkit::wsi::macos

#endif
