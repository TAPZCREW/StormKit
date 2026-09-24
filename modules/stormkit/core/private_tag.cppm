// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

export module stormkit.core.private_tag;

export namespace stormkit { inline namespace core {
    template<typename T>
    struct private_tag_base {
        struct tag {
          private:
            tag() = default;
            friend class private_tag_base<T>;
        };

        static constexpr auto PRIVATE = tag {};

        friend T;
    };

    template<typename T>
    using private_tag = private_tag_base<T>::tag;

    template<typename T>
    inline constexpr auto PRIVATE = private_tag_base<T>::PRIVATE;
}} // namespace stormkit::core
