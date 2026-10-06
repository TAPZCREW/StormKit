// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#define SUBSTITUTION 0

export module stormkit.core.typesafe.flags;

import std;

import stormkit.core.types;
import stormkit.core.meta.concepts;
import stormkit.core.meta.type_query;
import stormkit.core.string.static_string;

namespace stdr = std::ranges;

export {
    namespace stormkit { inline namespace core {
        namespace meta {
            template<enumeration T>
            inline constexpr auto FLAG_TRAIT = false;

            template<typename T>
            concept is_flag = FLAG_TRAIT<T>;
        } // namespace meta

        template<meta::is_flag T>
        [[nodiscard]]
        constexpr auto has_flag_bit(T value, T flag) noexcept -> bool;

        template<meta::enumeration T, usize N, T DEFAULT_VALUE, usize BUF_LEN = 50>
        consteval auto generate_substitution_strings_for(string_view                                    prefix,
                                                         array_view<const std::pair<T, string_view>, N> mapping,
                                                         char separator = '|') noexcept -> decltype(auto);

        template<meta::enumeration T, usize N, usize BUF_LEN = 50>
        consteval auto generate_substitution_strings_for(string_view                                    prefix,
                                                         array_view<const std::pair<T, string_view>, N> mapping,
                                                         char separator = '|') noexcept -> decltype(auto);
    }} // namespace stormkit::core

    template<stormkit::meta::is_flag T>
    [[nodiscard]]
    constexpr auto operator|(T lhs, T rhs) noexcept -> T;

    template<stormkit::meta::is_flag T>
    [[nodiscard]]
    constexpr auto operator&(T lhs, T rhs) noexcept -> T;

    template<stormkit::meta::is_flag T>
    [[nodiscard]]
    constexpr auto operator^(T lhs, T rhs) noexcept -> T;

    template<stormkit::meta::is_flag T>
    [[nodiscard]]
    constexpr auto operator~(T lhs) noexcept -> T;

    template<stormkit::meta::is_flag T>
    constexpr auto operator|=(T& lhs, T rhs) noexcept -> T&;

    template<stormkit::meta::is_flag T>
    constexpr auto operator&=(T& lhs, T rhs) noexcept -> T&;

    template<stormkit::meta::is_flag T>
    constexpr auto operator^=(T& lhs, T rhs) noexcept -> T&;
}

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    namespace details {
        template<meta::arithmetic T>
        STORMKIT_FORCE_INLINE STORMKIT_CONST
        constexpr auto factoriel(T n) noexcept -> T {
            constexpr auto FTABLE = array<u64, 14> {
                1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880, 3628800, 39916800, 479001600, 6227020800
            };

            if constexpr (meta::integral<T>) {
                const auto i = [](auto n) static noexcept {
                    if constexpr (meta::signed_type<T>) return static_cast<usize>(std::abs(n));
                    else
                        return static_cast<usize>(n);
                }(n);

                if constexpr (sizeof(T) <= 4) {
                    expects(FTABLE[i], "factoriel<i8> with n >= 5 result in integer overflow");
                    return static_cast<T>(FTABLE[i]);
                } else {
                    if (i < stdr::size(FTABLE)) return static_cast<T>(FTABLE[static_cast<usize>(n)]);
                }
            }

            auto res = T { 1 };
            for (auto i = T { 2 }; i <= n; ++i) res *= i;
            return res;
        }
    } // namespace details

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::is_flag T>
    STORMKIT_FORCE_INLINE STORMKIT_CONST
    constexpr auto has_flag_bit(T value, T flag) noexcept -> bool {
        return (value & flag) == flag;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::enumeration T, usize N, T DEFAULT_VALUE, usize BUF_LEN>
    consteval auto generate_substitution_strings_for(string_view                                    prefix,
                                                     array_view<const std::pair<T, string_view>, N> mapping,
                                                     char separator) noexcept -> decltype(auto) {
        constexpr auto OUT_SIZE = [] {
            auto res = 0uz;

            constexpr auto n_fact = details::factoriel(N);

            if constexpr (static_cast<int>(DEFAULT_VALUE) == 0)
                for (auto R = 0uz; R < (N - 1); ++R) res += n_fact / (details::factoriel(N - R) * details::factoriel(R));
            else
                for (auto R = 0uz; R < N; ++R) res += n_fact / (details::factoriel(N - R) * details::factoriel(R));

            return res + 1;
        }();

        auto out   = array<std::pair<T, static_string<BUF_LEN>>, OUT_SIZE> {};
        auto queue = dynarray<std::tuple<T, string, bool>> {};
        for (const auto& [k, v] : mapping) queue.emplace_back(k, string { v }, true);

        auto i = 0uz;
        while (not stdr::empty(queue)) {
            const auto [key, string_, single_value] = queue.back();
            if (not stdr::any_of(out, [&key](auto& pair) noexcept { return pair.first == key; })) {
                auto& [k, v] = out[i];
                k            = key;

                auto out_string = string { prefix };
                if (single_value) out_string += string_;
                else {
                    out_string += "(";
                    out_string += string_;
                    out_string += ")";
                }

                stdr::copy(out_string, stdr::begin(v));
                // v.update_size();

                i += 1;
            }
            if (key != DEFAULT_VALUE) {
                for (const auto& [k, v] : mapping) {
                    const auto has_key = stdr::any_of(queue, ([_k = k | key](auto&& tuple) noexcept {
                                                          const auto& [_key, _, _] = tuple;
                                                          return _key == _k;
                                                      }));

                    if (not has_key) {
                        auto str = string_;
                        if (k != DEFAULT_VALUE) {
                            str += " ";
                            str += separator;
                            str += " ";
                            str += v;
                        }
                        queue.emplace(stdr::begin(queue), key | k, str, false);
                    }
                }
            }
            queue.pop_back();
        }
        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<meta::enumeration T, usize N, usize BUF_LEN>
    consteval auto generate_substitution_strings_for(string_view                                    prefix,
                                                     array_view<const std::pair<T, string_view>, N> mapping,
                                                     char separator) noexcept -> decltype(auto) {
        constexpr auto OUT_SIZE = [] {
            auto res = 0uz;

            constexpr auto n_fact = details::factoriel(N);
            for (auto R = 0uz; R < N; ++R) res += n_fact / (details::factoriel(N - R) * details::factoriel(R));

            return res;
        }();

        auto out   = array<std::pair<T, static_string<BUF_LEN>>, OUT_SIZE> {};
        auto queue = dynarray<std::tuple<T, string, bool>> {};
        for (const auto& [k, v] : mapping) queue.emplace_back(k, string { v }, true);

        auto i = 0uz;
        while (not stdr::empty(queue)) {
            const auto [key, string_, single_value] = queue.back();
            if (not stdr::any_of(out, [&key](auto& pair) noexcept { return pair.first == key; })) {
                auto& [k, v] = out[i];
                k            = key;

                auto out_string = string { prefix };
                if (single_value) out_string += string_;
                else {
                    out_string += "(";
                    out_string += string_;
                    out_string += ")";
                }

                stdr::copy(out_string, stdr::begin(v));
                // v.update_size();

                i += 1;
            }
            for (const auto& [k, v] : mapping) {
                const auto has_key = stdr::any_of(queue, [_k = k | key](auto tuple) noexcept {
                    const auto& [_key, _, _] = tuple;
                    return _key == _k;
                });

                if (not has_key) queue.emplace(stdr::begin(queue), key | k, string_ + " " + separator + " " + v, false);
            }
            queue.pop_back();
        }
        return out;
    }
}} // namespace stormkit::core

/////////////////////////////////////
/////////////////////////////////////
template<stormkit::meta::is_flag T>
STORMKIT_FORCE_INLINE STORMKIT_CONST
constexpr auto operator|(T lhs, T rhs) noexcept -> T {
    using type = stormkit::meta::underlying_type<T>;
    return static_cast<T>(static_cast<type>(lhs) | static_cast<type>(rhs));
}

/////////////////////////////////////
/////////////////////////////////////
template<stormkit::meta::is_flag T>
STORMKIT_FORCE_INLINE STORMKIT_CONST
constexpr auto operator&(T lhs, T rhs) noexcept -> T {
    using type = stormkit::meta::underlying_type<T>;
    return static_cast<T>(static_cast<type>(lhs) & static_cast<type>(rhs));
}

/////////////////////////////////////
/////////////////////////////////////
template<stormkit::meta::is_flag T>
STORMKIT_FORCE_INLINE STORMKIT_CONST
constexpr auto operator^(T lhs, T rhs) noexcept -> T {
    using type = stormkit::meta::underlying_type<T>;
    return static_cast<T>(static_cast<type>(lhs) ^ static_cast<type>(rhs));
}

/////////////////////////////////////
/////////////////////////////////////
template<stormkit::meta::is_flag T>
STORMKIT_FORCE_INLINE STORMKIT_CONST
constexpr auto operator~(T lhs) noexcept -> T {
    using type = stormkit::meta::underlying_type<T>;
    return static_cast<T>(~static_cast<type>(lhs));
}

/////////////////////////////////////
/////////////////////////////////////
template<stormkit::meta::is_flag T>
STORMKIT_FORCE_INLINE
constexpr auto operator|=(T& lhs, T rhs) noexcept -> T& {
    lhs = lhs | rhs;
    return lhs;
}

/////////////////////////////////////
/////////////////////////////////////
template<stormkit::meta::is_flag T>
STORMKIT_FORCE_INLINE
constexpr auto operator&=(T& lhs, T rhs) noexcept -> T& {
    lhs = lhs & rhs;
    return lhs;
}

/////////////////////////////////////
/////////////////////////////////////
template<stormkit::meta::is_flag T>
STORMKIT_FORCE_INLINE
constexpr auto operator^=(T& lhs, T rhs) noexcept -> T& {
    lhs = lhs ^ rhs;
    return lhs;
}
