// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.hash;

import std;

import stormkit.core.types;
import stormkit.core.meta.concepts;
import stormkit.core.meta.type_query;
import stormkit.core.meta.tag_invoke;
import stormkit.core.hash.crc;
import stormkit.core.typesafe.safecasts;
import stormkit.core.typesafe.ref_ptr;

namespace stdr = std::ranges;

export namespace stormkit { inline namespace core {
    template<meta::hash Ret>
    struct hash_fn final {
      private:
        template<typename T>
        using invoke_result = meta::tag_invoke_result<hash_fn<Ret>, const T&>;

        template<typename T>
        static constexpr auto IS_tag_invokable = meta::tag_invocable<hash_fn<Ret>, const T&>;

      public:
        template<typename T>
        static constexpr auto operator()(const T& value) noexcept
            requires(not meta::has_unique_object_representations<T> and not IS_tag_invokable<T>)
        = delete ("No hasher defined for this type");

        template<typename T>
        [[nodiscard]]
        static constexpr auto operator()(const T& value) noexcept -> Ret
            requires(meta::has_unique_object_representations<T> and not IS_tag_invokable<T>);

        template<typename T>
        [[nodiscard]]
        static constexpr auto operator()(const T& value) noexcept -> invoke_result<T>
            requires(IS_tag_invokable<T>);
    };

    template<meta::hash Ret>
    inline constexpr auto hash_cpo = hash_fn<Ret> {};

    template<meta::hash Ret = hash64, typename... Ts>
    constexpr auto hash_of(const Ts&... values) noexcept -> Ret
        requires(sizeof...(Ts) >= 1);

    template<meta::hash Ret = hash64, typename... Ts>
    constexpr auto hash_combine(Ret hash, const Ts&... args) noexcept -> Ret
        requires(sizeof...(Ts) >= 1);

    namespace meta {
        template<typename T>
        concept has_hasher = requires(const T& value) {
            { hash_of<hash64>(value) } -> same_as<hash64>;
        };
    } // namespace meta

    template<typename Ret, typename U>
    constexpr auto tag_invoke(hash_fn<Ret>, ref_ptr<U> ptr) -> Ret;
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::hash Ret>
    template<typename T>
    STORMKIT_FORCE_INLINE
    constexpr auto hash_fn<Ret>::operator()(const T& value) noexcept -> Ret
        requires(meta::has_unique_object_representations<T> and not IS_tag_invokable<T>)
    {
        if constexpr (meta::is<Ret, hash32>) return hash::crc32(view_of(as_bytes, value));
        else
            return hash::crc64(view_of(as_bytes, value));
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::hash Ret>
    template<typename T>
    STORMKIT_FORCE_INLINE
    constexpr auto hash_fn<Ret>::operator()(const T& value) noexcept -> invoke_result<T>
        requires(IS_tag_invokable<T>)
    {
        return tag_invoke(hash_fn<Ret> {}, value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::hash Ret = hash64, typename... Ts>
    constexpr auto hash_of(const Ts&... values) noexcept -> Ret
        requires(sizeof...(Ts) >= 1)
    {
        auto out = Ret {};

        ((out = hash_combine(out, values)), ...);

        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::hash Ret, typename... Ts>
    constexpr auto hash_combine(Ret with_, const Ts&... values) noexcept -> Ret
        requires(sizeof...(Ts) >= 1)
    {
        using type = meta::underlying_type<Ret>;
        auto with  = as<type>(with_);

        auto combine = [&with]<typename T>(const T& value) mutable noexcept {
            if constexpr (stdr::input_range<T> and not meta::is_any_of<T, string, string_view>)
                for (const auto& elem : value)
                    with = with ^= as<type>(hash_cpo<Ret>(elem)) + 0x9e3779b9 + (with << 6) + (with >> 2);
            else
                with ^= as<type>(hash_cpo<Ret>(value)) + 0x9e3779b9 + (with << 6) + (with >> 2);
            return with;
        };

        ((with = combine(values)), ...);

        return as<Ret>(with);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Ret, typename U>
    STORMKIT_FORCE_INLINE
    constexpr auto tag_invoke(hash_fn<Ret>, ref_ptr<U> ptr) -> Ret {
        return hash_of(std::bit_cast<uptr>(ptr.get()));
    }
}} // namespace stormkit::core
