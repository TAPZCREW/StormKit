// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/lua/lua.hpp>

module stormkit.lua;

import std;

import stormkit.core;

namespace stormkit::lua::core {
    namespace {
        ////////////////////////////////////////
        ////////////////////////////////////////
        template<auto... Rects>
        auto _bind_rect(auto& parent) {
            (
              [&] noexcept {
                  using type = typename decltype(Rects)::type;
                  using Rect = math::rect<type>;

                  parent.template new_usertype<Rect>(
                    string { Rects.name },
                    sol::constructors<Rect(), Rect(Type, Type, Type, Type)> {},
                    "position",
                    &Rect::position,
                    "extent",
                    &Rect::extent,
                    "to_bounding_rect",
                    +[](const Rect* rect) noexcept -> math::bounding_rect<type> { return to_bounding_rect<type>(*rect); });
              }(),
              ...);
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<auto... Rects>
        auto _bind_bounding_rect(auto& parent) {
            (
              [&] noexcept {
                  using type = typename decltype(Rects)::type;
                  using Rect = math::bounding_rect<type>;

                  parent.template new_usertype<Rect>(
                    string { Rects.name },
                    sol::constructors<Rect(), Rect(Type, Type, Type, Type)> {},
                    "topleft",
                    &Rect::topleft,
                    "bottomright",
                    &Rect::bottomright,
                    "to_rect",
                    +[](const Rect* rect) noexcept -> math::rect<type> { return to_rect<type>(*rect); });
              }(),
              ...);
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<template<class> typename T, typename U>
        auto _bind_angle(string_view name, auto& parent) {
            auto metatable     = parent.template new_usertype<T<U>>(name, sol::constructors<T<U>(U)> {});
            metatable["value"] = sol::property(
              +[](T<U>* angle) static noexcept { return angle->get(); },
              +[](T<U>* angle, U val) static noexcept { return angle->get() = val; });
            metatable[sol::meta_function::addition]    = +[](T<U> first, T<U> second) static noexcept { return first + second; };
            metatable[sol::meta_function::subtraction] = +[](T<U> first, T<U> second) static noexcept { return first - second; };
            metatable[sol::meta_function::multiplication] = +[](T<U> first, T<U> second) static noexcept {
                return first * second;
            };
            metatable[sol::meta_function::division] = +[](T<U> first, T<U> second) static noexcept { return first / second; };
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T, typename... Constructors>
        auto _bind_extent(string_view name, auto& parent, auto... values) {
            auto metatable = parent.template new_usertype<T>(name, sol::constructors<T(), Constructors...> {});

            metatable[sol::meta_function::as<string>] = &math::as<string><T>;
            metatable[sol::meta_function::is]  = +[](const T& first, const T& second) static noexcept {
                return first == second;
            };

            metatable["rank"] = sol::var(std::cref(T::RANK));

            (
              [&metatable, &values] mutable {
                  auto&& [k, v] = std::forward<decltype(values)>(values);
                  metatable[k]  = v;
              }(),
              ...);
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T, typename... Constructors>
        auto _bind_vector(string_view name, auto& parent, auto... values) {
            auto metatable = parent.template new_usertype<T>(name, sol::constructors<T(), Constructors...> {});

            metatable[sol::meta_function::as<string>] = +[](const T& value) static noexcept { return as<string>(value); };
            // metatable[sol::meta_function::is]  = +[](const T& first, const T& second) static noexcept {
            //     return first == second;
            // };

            (
              [&metatable, &values] mutable {
                  auto&& [k, v] = std::forward<decltype(values)>(values);
                  metatable[k]  = v;
              }(),
              ...);
        }

        ////////////////////////////////////////
        ////////////////////////////////////////
        template<typename T, typename... Constructors>
        auto _bind_matrix(string_view name, auto& parent) {
            auto metatable = parent.template new_usertype<T>(name, sol::constructors<T(), Constructors...> {});

            metatable[sol::meta_function::index] = +[](T* value, usize index) static noexcept {
                return array_view<typename T::value_type> { &((*value)[index, 0u]), T::EXTENTS[1] };
            };
            metatable[sol::meta_function::as<string>] = +[](const T& value) static noexcept { return math::as<string>(value); };

            if constexpr (T::EXTENTS[0] == T::EXTENTS[1]) {
                auto identity  = parent["identity"].template get_or_create<sol::table>();
                identity[name] = T::identity();
            }
            // metatable[sol::meta_function::is]  = +[](const T& first, const T& second) static noexcept {
            //     return first == second;
            // };
        }
    } // namespace

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto bind_extent(sol::table& metatable) noexcept -> void {
        _bind_extent<math::uextent2, math::uextent2(u32, u32)>("uextent2",
                                                               metatable,
                                                               std::pair { "width", &math::uextent2::width },
                                                               std::pair { "height", &math::uextent2::height });
        _bind_extent<math::uextent3, math::uextent3(u32, u32, u32)>("math::uextent3",
                                                                    metatable,
                                                                    std::pair { "width", &math::uextent3::width },
                                                                    std::pair { "height", &math::uextent3::height },
                                                                    std::pair { "depth", &math::uextent3::depth });
        _bind_extent<math::fextent2, math::fextent2(f32, f32)>("fextent2",
                                                               metatable,
                                                               std::pair { "width", &math::fextent2::width },
                                                               std::pair { "height", &math::fextent2::height });
        _bind_extent<math::fextent3, math::fextent3(f32, f32, f32)>("fextent3",
                                                                    metatable,
                                                                    std::pair { "width", &math::fextent3::width },
                                                                    std::pair { "height", &math::fextent3::height },
                                                                    std::pair { "depth", &math::fextent3::depth });
        _bind_extent<math::iextent2, math::iextent2(i32, i32)>("iextent2",
                                                               metatable,
                                                               std::pair { "width", &math::iextent2::width },
                                                               std::pair { "height", &math::iextent2::height });
        _bind_extent<math::iextent3, math::iextent3(i32, i32, i32)>("iextent3",
                                                                    metatable,
                                                                    std::pair { "width", &math::iextent3::width },
                                                                    std::pair { "height", &math::iextent3::height },
                                                                    std::pair { "depth", &math::iextent3::depth });
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto bind_linear_vector(sol::table& metatable) noexcept -> void {
        _bind_vector<math::fvec2, math::fvec2(f32, f32)>("fvec2",
                                                         metatable,
                                                         std::pair { "x", &math::fvec2::x },
                                                         std::pair { "y", &math::fvec2::y });
        _bind_vector<math::fvec3, math::fvec3(f32, f32, f32)>("fvec3",
                                                              metatable,
                                                              std::pair { "x", &math::fvec3::x },
                                                              std::pair { "y", &math::fvec3::y },
                                                              std::pair { "z", &math::fvec3::z });
        _bind_vector<math::fvec4, math::fvec4(f32, f32)>("fvec4",
                                                         metatable,
                                                         std::pair { "x", &math::fvec4::x },
                                                         std::pair { "y", &math::fvec4::y },
                                                         std::pair { "z", &math::fvec4::z },
                                                         std::pair { "w", &math::fvec4::w });
        _bind_vector<math::uvec2, math::uvec2(u32, u32)>("uvec2",
                                                         metatable,
                                                         std::pair { "x", &math::uvec2::x },
                                                         std::pair { "y", &math::uvec2::y });
        _bind_vector<math::uvec3, math::uvec3(u32, u32, u32)>("uvec3",
                                                              metatable,
                                                              std::pair { "x", &math::uvec3::x },
                                                              std::pair { "y", &math::uvec3::y },
                                                              std::pair { "z", &math::uvec3::z });
        _bind_vector<math::uvec4, math::uvec4(u32, u32)>("uvec4",
                                                         metatable,
                                                         std::pair { "x", &math::uvec4::x },
                                                         std::pair { "y", &math::uvec4::y },
                                                         std::pair { "z", &math::uvec4::z },
                                                         std::pair { "w", &math::uvec4::w });
        _bind_vector<math::ivec2, math::ivec2(i32, i32)>("ivec2",
                                                         metatable,
                                                         std::pair { "x", &math::ivec2::x },
                                                         std::pair { "y", &math::ivec2::y });
        _bind_vector<math::ivec3, math::ivec3(i32, i32, i32)>("ivec3",
                                                              metatable,
                                                              std::pair { "x", &math::ivec3::x },
                                                              std::pair { "y", &math::ivec3::y },
                                                              std::pair { "z", &math::ivec3::z });
        _bind_vector<math::ivec4, math::ivec4(i32, i32)>("ivec4",
                                                         metatable,
                                                         std::pair { "x", &math::ivec4::x },
                                                         std::pair { "y", &math::ivec4::y },
                                                         std::pair { "z", &math::ivec4::z },
                                                         std::pair { "w", &math::ivec4::w });

        // [](const auto&... args) static noexcept { return math::add(args...); });

        metatable["add"] = sol::overload(&math::add<math::fvec2>,
                                         &math::add<math::fvec3>,
                                         &math::add<math::fvec4>,
                                         &math::add<math::uvec2>,
                                         &math::add<math::uvec3>,
                                         &math::add<math::uvec4>,
                                         &math::add<math::ivec2>,
                                         &math::add<math::ivec3>,
                                         &math::add<math::ivec4>);
        metatable["sub"] = sol::overload(&math::sub<math::fvec2>,
                                         &math::sub<math::fvec3>,
                                         &math::sub<math::fvec4>,
                                         &math::sub<math::uvec2>,
                                         &math::sub<math::uvec3>,
                                         &math::sub<math::uvec4>,
                                         &math::sub<math::ivec2>,
                                         &math::sub<math::ivec3>,
                                         &math::sub<math::ivec4>);

        metatable["mul"] = sol::overload(&math::mul<math::fvec2>,
                                         &math::mul<math::fvec3>,
                                         &math::mul<math::fvec4>,
                                         &math::mul<math::uvec2>,
                                         &math::mul<math::uvec3>,
                                         &math::mul<math::uvec4>,
                                         &math::mul<math::ivec2>,
                                         &math::mul<math::ivec3>,
                                         &math::mul<math::ivec4>);
        metatable["div"] = sol::overload(&math::div<math::fvec2>,
                                         &math::div<math::fvec3>,
                                         &math::div<math::fvec4>,
                                         &math::div<math::uvec2>,
                                         &math::div<math::uvec3>,
                                         &math::div<math::uvec4>,
                                         &math::div<math::ivec2>,
                                         &math::div<math::ivec3>,
                                         &math::div<math::ivec4>);

        metatable["dot"] = sol::overload(&math::dot<math::fvec2>,
                                         &math::dot<math::fvec3>,
                                         &math::dot<math::fvec4>,
                                         &math::dot<math::uvec2>,
                                         &math::dot<math::uvec3>,
                                         &math::dot<math::uvec4>,
                                         &math::dot<math::ivec2>,
                                         &math::dot<math::ivec3>,
                                         &math::dot<math::ivec4>);

        metatable["cross"] = sol::overload(
          +[](const math::fvec3& first, const math::fvec3& second) static noexcept { return math::cross(first, second); },
          +[](const math::uvec3& first, const math::uvec3& second) static noexcept { return math::cross(first, second); },
          +[](const math::ivec3& first, const math::ivec3& second) static noexcept { return math::cross(first, second); });

        metatable["normalize"] = sol::overload(&math::normalize<math::fvec2>,
                                               &math::normalize<math::fvec3>,
                                               &math::normalize<math::fvec4>,
                                               &math::normalize<math::uvec2>,
                                               &math::normalize<math::uvec3>,
                                               &math::normalize<math::uvec4>,
                                               &math::normalize<math::ivec2>,
                                               &math::normalize<math::ivec3>,
                                               &math::normalize<math::ivec4>);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    auto bind_linear_matrix(sol::table& metatable) noexcept -> void {
        _bind_matrix<math::fmat2>("fmat2", metatable);
        _bind_matrix<math::fmat2x3>("fmat2x3", metatable);
        _bind_matrix<math::fmat2x4>("fmat2x4", metatable);
        _bind_matrix<math::fmat3>("fmat3", metatable);
        _bind_matrix<math::fmat3x2>("fmat3x2", metatable);
        _bind_matrix<math::fmat3x4>("fmat3x4", metatable);
        _bind_matrix<math::fmat4>("fmat4", metatable);
        _bind_matrix<math::fmat4x2>("fmat4x2", metatable);
        _bind_matrix<math::fmat4x3>("fmat4x3", metatable);
        _bind_matrix<math::umat2>("umat2", metatable);
        _bind_matrix<math::umat2x3>("umat2x3", metatable);
        _bind_matrix<math::umat2x4>("umat2x4", metatable);
        _bind_matrix<math::umat3>("umat3", metatable);
        _bind_matrix<math::umat3x2>("umat3x2", metatable);
        _bind_matrix<math::umat3x4>("umat3x4", metatable);
        _bind_matrix<math::umat4>("umat4", metatable);
        _bind_matrix<math::umat4x2>("umat4x2", metatable);
        _bind_matrix<math::umat4x3>("umat4x3", metatable);
        _bind_matrix<math::imat2>("imat2", metatable);
        _bind_matrix<math::imat2x3>("imat2x3", metatable);
        _bind_matrix<math::imat2x4>("imat2x4", metatable);
        _bind_matrix<math::imat3>("imat3", metatable);
        _bind_matrix<math::imat3x2>("imat3x2", metatable);
        _bind_matrix<math::imat3x4>("imat3x4", metatable);
        _bind_matrix<math::imat4>("imat4", metatable);
        _bind_matrix<math::imat4x2>("imat4x2", metatable);
        _bind_matrix<math::imat4x3>("imat4x3", metatable);
        metatable["determinant"] = sol::overload(&math::determinant<math::fmat2>,
                                                 &math::determinant<math::fmat3>,
                                                 &math::determinant<math::fmat4>,
                                                 &math::determinant<math::umat2>,
                                                 &math::determinant<math::umat3>,
                                                 &math::determinant<math::umat4>,
                                                 &math::determinant<math::imat2>,
                                                 &math::determinant<math::imat3>,
                                                 &math::determinant<math::imat4>);
        metatable["transpose"]   = sol::overload(&math::transpose<math::fmat2>,
                                                 &math::transpose<math::fmat3>,
                                                 &math::transpose<math::fmat4>,
                                                 &math::transpose<math::umat2>,
                                                 &math::transpose<math::umat3>,
                                                 &math::transpose<math::umat4>,
                                                 &math::transpose<math::imat2>,
                                                 &math::transpose<math::imat3>,
                                                 &math::transpose<math::imat4>);
        metatable["inverse"]     = sol::overload(&math::inverse<math::fmat2>,
                                                 &math::inverse<math::fmat3>,
                                                 &math::inverse<math::fmat4>,
                                                 &math::inverse<math::umat2>,
                                                 &math::inverse<math::umat3>,
                                                 &math::inverse<math::umat4>,
                                                 &math::inverse<math::imat2>,
                                                 &math::inverse<math::imat3>,
                                                 &math::inverse<math::imat4>);

        metatable["is_inversible"] = sol::
          overload(&math::is_inversible<math::fmat2>,
                   &math::is_inversible<math::fmat2x3>,
                   &math::is_inversible<math::fmat2x4>,
                   &math::is_inversible<math::fmat3>,
                   &math::is_inversible<math::fmat3x2>,
                   &math::is_inversible<math::fmat3x4>,
                   &math::is_inversible<math::fmat4>,
                   &math::is_inversible<math::fmat4x2>,
                   &math::is_inversible<math::fmat4x3>,
                   &math::is_inversible<math::umat2>,
                   &math::is_inversible<math::umat2x3>,
                   &math::is_inversible<math::umat2x4>,
                   &math::is_inversible<math::umat3>,
                   &math::is_inversible<math::umat3x2>,
                   &math::is_inversible<math::umat3x4>,
                   &math::is_inversible<math::umat4>,
                   &math::is_inversible<math::umat4x2>,
                   &math::is_inversible<math::umat4x3>,
                   &math::is_inversible<math::imat2>,
                   &math::is_inversible<math::imat2x3>,
                   &math::is_inversible<math::imat2x4>,
                   &math::is_inversible<math::imat3>,
                   &math::is_inversible<math::imat3x2>,
                   &math::is_inversible<math::imat3x4>,
                   &math::is_inversible<math::imat4>,
                   &math::is_inversible<math::imat4x2>,
                   &math::is_inversible<math::imat4x3>);
        metatable["is_orthogonal"] = sol::
          overload(&math::is_orthogonal<math::fmat2>,
                   &math::is_orthogonal<math::fmat2x3>,
                   &math::is_orthogonal<math::fmat2x4>,
                   &math::is_orthogonal<math::fmat3>,
                   &math::is_orthogonal<math::fmat3x2>,
                   &math::is_orthogonal<math::fmat3x4>,
                   &math::is_orthogonal<math::fmat4>,
                   &math::is_orthogonal<math::fmat4x2>,
                   &math::is_orthogonal<math::fmat4x3>,
                   &math::is_orthogonal<math::umat2>,
                   &math::is_orthogonal<math::umat2x3>,
                   &math::is_orthogonal<math::umat2x4>,
                   &math::is_orthogonal<math::umat3>,
                   &math::is_orthogonal<math::umat3x2>,
                   &math::is_orthogonal<math::umat3x4>,
                   &math::is_orthogonal<math::umat4>,
                   &math::is_orthogonal<math::umat4x2>,
                   &math::is_orthogonal<math::umat4x3>,
                   &math::is_orthogonal<math::imat2>,
                   &math::is_orthogonal<math::imat2x3>,
                   &math::is_orthogonal<math::imat2x4>,
                   &math::is_orthogonal<math::imat3>,
                   &math::is_orthogonal<math::imat3x2>,
                   &math::is_orthogonal<math::imat3x4>,
                   &math::is_orthogonal<math::imat4>,
                   &math::is_orthogonal<math::imat4x2>,
                   &math::is_orthogonal<math::imat4x3>);
        metatable["mul"] = sol::overload(
          +[](const math::fmat2& a, typename math::fmat2::value_type b) static noexcept { return mul(a, b); },
          +[](const math::fmat2x3& a, typename math::fmat2x3::value_type b) static noexcept { return mul(a, b); },
          +[](const math::fmat2x4& a, typename math::fmat2x4::value_type b) static noexcept { return mul(a, b); },
          +[](const math::fmat3& a, typename math::fmat3::value_type b) static noexcept { return mul(a, b); },
          +[](const math::fmat3x2& a, typename math::fmat3x2::value_type b) static noexcept { return mul(a, b); },
          +[](const math::fmat3x4& a, typename math::fmat3x4::value_type b) static noexcept { return mul(a, b); },
          +[](const math::fmat4& a, typename math::fmat4::value_type b) static noexcept { return mul(a, b); },
          +[](const math::fmat4x2& a, typename math::fmat4x2::value_type b) static noexcept { return mul(a, b); },
          +[](const math::fmat4x3& a, typename math::fmat4x3::value_type b) static noexcept { return mul(a, b); },
          +[](const math::umat2& a, typename math::umat2::value_type b) static noexcept { return mul(a, b); },
          +[](const math::umat2x3& a, typename math::umat2x3::value_type b) static noexcept { return mul(a, b); },
          +[](const math::umat2x4& a, typename math::umat2x4::value_type b) static noexcept { return mul(a, b); },
          +[](const math::umat3& a, typename math::umat3::value_type b) static noexcept { return mul(a, b); },
          +[](const math::umat3x2& a, typename math::umat3x2::value_type b) static noexcept { return mul(a, b); },
          +[](const math::umat3x4& a, typename math::umat3x4::value_type b) static noexcept { return mul(a, b); },
          +[](const math::umat4& a, typename math::umat4::value_type b) static noexcept { return mul(a, b); },
          +[](const math::umat4x2& a, typename math::umat4x2::value_type b) static noexcept { return mul(a, b); },
          +[](const math::umat4x3& a, typename math::umat4x3::value_type b) static noexcept { return mul(a, b); },
          +[](const math::imat2& a, typename math::imat2::value_type b) static noexcept { return mul(a, b); },
          +[](const math::imat2x3& a, typename math::imat2x3::value_type b) static noexcept { return mul(a, b); },
          +[](const math::imat2x4& a, typename math::imat2x4::value_type b) static noexcept { return mul(a, b); },
          +[](const math::imat3& a, typename math::imat3::value_type b) static noexcept { return mul(a, b); },
          +[](const math::imat3x2& a, typename math::imat3x2::value_type b) static noexcept { return mul(a, b); },
          +[](const math::imat3x4& a, typename math::imat3x4::value_type b) static noexcept { return mul(a, b); },
          +[](const math::imat4& a, typename math::imat4::value_type b) static noexcept { return mul(a, b); },
          +[](const math::imat4x2& a, typename math::imat4x2::value_type b) static noexcept { return mul(a, b); },
          +[](const math::imat4x3& a, typename math::imat4x3::value_type b) static noexcept { return mul(a, b); });
        metatable["div"] = sol::overload(
          +[](const math::fmat2& a, typename math::fmat2::value_type b) static noexcept { return div(a, b); },
          +[](const math::fmat2x3& a, typename math::fmat2x3::value_type b) static noexcept { return div(a, b); },
          +[](const math::fmat2x4& a, typename math::fmat2x4::value_type b) static noexcept { return div(a, b); },
          +[](const math::fmat3& a, typename math::fmat3::value_type b) static noexcept { return div(a, b); },
          +[](const math::fmat3x2& a, typename math::fmat3x2::value_type b) static noexcept { return div(a, b); },
          +[](const math::fmat3x4& a, typename math::fmat3x4::value_type b) static noexcept { return div(a, b); },
          +[](const math::fmat4& a, typename math::fmat4::value_type b) static noexcept { return div(a, b); },
          +[](const math::fmat4x2& a, typename math::fmat4x2::value_type b) static noexcept { return div(a, b); },
          +[](const math::fmat4x3& a, typename math::fmat4x3::value_type b) static noexcept { return div(a, b); },
          +[](const math::umat2& a, typename math::umat2::value_type b) static noexcept { return div(a, b); },
          +[](const math::umat2x3& a, typename math::umat2x3::value_type b) static noexcept { return div(a, b); },
          +[](const math::umat2x4& a, typename math::umat2x4::value_type b) static noexcept { return div(a, b); },
          +[](const math::umat3& a, typename math::umat3::value_type b) static noexcept { return div(a, b); },
          +[](const math::umat3x2& a, typename math::umat3x2::value_type b) static noexcept { return div(a, b); },
          +[](const math::umat3x4& a, typename math::umat3x4::value_type b) static noexcept { return div(a, b); },
          +[](const math::umat4& a, typename math::umat4::value_type b) static noexcept { return div(a, b); },
          +[](const math::umat4x2& a, typename math::umat4x2::value_type b) static noexcept { return div(a, b); },
          +[](const math::umat4x3& a, typename math::umat4x3::value_type b) static noexcept { return div(a, b); },
          +[](const math::imat2& a, typename math::imat2::value_type b) static noexcept { return div(a, b); },
          +[](const math::imat2x3& a, typename math::imat2x3::value_type b) static noexcept { return div(a, b); },
          +[](const math::imat2x4& a, typename math::imat2x4::value_type b) static noexcept { return div(a, b); },
          +[](const math::imat3& a, typename math::imat3::value_type b) static noexcept { return div(a, b); },
          +[](const math::imat3x2& a, typename math::imat3x2::value_type b) static noexcept { return div(a, b); },
          +[](const math::imat3x4& a, typename math::imat3x4::value_type b) static noexcept { return div(a, b); },
          +[](const math::imat4& a, typename math::imat4::value_type b) static noexcept { return div(a, b); },
          +[](const math::imat4x2& a, typename math::imat4x2::value_type b) static noexcept { return div(a, b); },
          +[](const math::imat4x3& a, typename math::imat4x3::value_type b) static noexcept { return div(a, b); });

        metatable["translate"] = sol::overload(&math::matrix::translate<f32>, &math::matrix::translate<i32>);
        metatable["scale"]     = sol::overload(&math::matrix::scale<f32>, &math::matrix::scale<u32>, &math::matrix::scale<i32>);
        metatable["rotate"]    = &math::matrix::rotate<f32>;
        metatable["orthographique"] = sol::overload(
          +[](f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far) static noexcept {
              return math::orthographique(left, right, bottom, top, near, far);
          },
          +[](f32 left, f32 right, f32 bottom, f32 top) static noexcept {
              return math::orthographique(left, right, bottom, top);
          });
        metatable["perspective"] = &math::matrix::perspective<f32>;
        metatable["look_at"]     = &math::matrix::look_at<f32>;
    }

    auto bind_linear(sol::table& parent) noexcept -> void {
        auto angle = parent["angle"].get_or_create<sol::table>();

        _bind_angle<math::angle::euler, f32>("euler", angle);
        _bind_angle<math::angle::radian, f32>("radian", angle);

        angle["radians"] = sol::overload(&math::angle::radians<f32>);

        bind_linear_vector(parent);
        bind_linear_matrix(parent);
    }

    template<typename T, static_string Name>
    struct _Rect {
        using type                 = T;
        static constexpr auto name = Name;
    };

    auto bind_geometry(sol::table& parent) noexcept -> void {
        _bind_rect<_Rect<i32, "irect"> {}, _Rect<f32, "frect"> {}, _Rect<u32, "urect"> {}>(parent);
        _bind_bounding_rect<_Rect<i32, "ibounding_rect"> {},
                            _Rect<f32, "fbounding_rect"> {},
                            _Rect<u32, "ubounding_rect"> {}>(parent);
        parent["AABB"] = sol::overload(
          +[](const math::irect& rect1, const math::irect& rect2) static noexcept { return AABB(rect1, rect2); },
          +[](const math::ivec2& vec, const math::irect& rect) static noexcept { return AABB(vec, rect); },
          +[](const math::ivec2& vec, const math::ibounding_rect& rect) static noexcept { return AABB(vec, rect); },
          +[](const math::frect& rect1, const math::frect& rect2) static noexcept { return AABB(rect1, rect2); },
          +[](const math::fvec2& vec, const math::frect& rect) static noexcept { return AABB(vec, rect); },
          +[](const math::fvec2& vec, const math::fbounding_rect& rect) static noexcept { return AABB(vec, rect); },
          +[](const math::urect& rect1, const math::urect& rect2) static noexcept { return AABB(rect1, rect2); },
          +[](const math::uvec2& vec, const math::urect& rect) static noexcept { return AABB(vec, rect); },
          +[](const math::uvec2& vec, const math::ubounding_rect& rect) static noexcept { return AABB(vec, rect); });
    }

    auto bind_math(sol::state& global_state) noexcept -> void {
        auto math_table = global_state["math"].get_or_create<sol::table>();
        bind_extent(math_table);
        bind_linear(math_table);
        bind_geometry(math_table);
    }
} // namespace stormkit::lua::core
