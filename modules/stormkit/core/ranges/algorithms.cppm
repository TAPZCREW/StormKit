// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

export module stormkit.core.ranges.algorithms;

import std;

import stormkit.core.meta.concepts;
import stormkit.core.meta.type_query;
import stormkit.core.meta.type_manipulation;
import stormkit.core.types;

namespace stdr = std::ranges;
namespace stdv = std::views;

export namespace stormkit { inline namespace core {
    namespace meta {
        template<typename T, typename U>
        concept range_predicate = meta::unary_predicate<T, meta::range_value_type<U>>;

        namespace plain {
            template<typename T, typename U>
            concept range_predicate = meta::plain::apply_to<T, range_predicate, U>;
        }

        template<typename T, typename U>
        concept range_unary_op = std::regular_invocable<T, meta::range_value_type_ref<U>>;

        namespace plain {
            template<typename T, typename U>
            concept range_unary_op = meta::plain::apply_to<T, range_unary_op, U>;
        }
    } // namespace meta

    template<stdr::input_range Range, meta::range_predicate<Range> Predicate>
    [[nodiscard]]
    constexpr auto copy_if(const Range& input, const Predicate& predicate) noexcept -> decltype(auto);

    template<stdr::input_range Range, meta::range_unary_op<Range> Operation>
    [[nodiscard]]
    constexpr auto transform(const Range& input, const Operation& op) noexcept -> decltype(auto);

    template<stdr::input_range Range, meta::range_predicate<Range> Predicate, meta::range_unary_op<Range> Operation>
    [[nodiscard]]
    constexpr auto transform_if(const Range& input, const Predicate& predicate, const Operation& op) noexcept -> decltype(auto);

    template<stdr::input_range                                                                        Range,
             meta::range_predicate<Range>                                                             Predicate,
             meta::range_unary_op<Range>                                                              Operation,
             std::output_iterator<std::invoke_result_t<Operation, meta::range_value_type_ref<Range>>> Iterator>
    constexpr auto transform_if(const Range& input, const Iterator& it, const Predicate& predicate, const Operation& op) noexcept
      -> void;
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    /////////////////////////////////////
    /////////////////////////////////////
    template<stdr::input_range Range, meta::range_predicate<Range> Predicate>
    constexpr auto copy_if(const Range& input, Predicate&& predicate) noexcept -> decltype(auto) {
        return input | stdv::filter(predicate) | stdr::to<dynarray<meta::range_value_type<Range>>>();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<stdr::input_range Range, meta::range_unary_op<Range> Operation>
    constexpr auto transform(const Range& input, const Operation& op) noexcept -> decltype(auto) {
        return input | stdv::transform(op) | stdr::to<dynarray<std::invoke_result_t<Operation, meta::range_value_type<Range>>>>();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<stdr::input_range Range, meta::range_predicate<Range> Predicate, meta::range_unary_op<Range> Operation>
    constexpr auto transform_if(const Range& input, const Predicate& predicate, const Operation& op) noexcept -> decltype(auto) {
        return input
               | stdv::filter(predicate)
               | stdv::transform(op)
               | stdr::to<dynarray<std::invoke_result_t<Operation, meta::range_value_type<Range>>>>();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<stdr::input_range                                                                        Range,
             meta::range_predicate<Range>                                                             Predicate,
             meta::range_unary_op<Range>                                                              Operation,
             std::output_iterator<std::invoke_result_t<Operation, meta::range_value_type_ref<Range>>> Iterator>
    constexpr auto transform_if(const Range& input, const Iterator& it, const Predicate& predicate, const Operation& op) noexcept
      -> void {
        stdr::for_each(std::forward<Range>(input), [&it, &predicate, &op](auto&& elem) {
            if (predicate(elem)) *it++ = op(elem);
        });
    }
}} // namespace stormkit::core
