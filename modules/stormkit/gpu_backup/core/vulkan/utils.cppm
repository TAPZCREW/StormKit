// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

#include <stormkit/gpu/vulkan.hpp>

export module stormkit.gpu.core:vulkan.utils;

import std;

import stormkit.core;
import stormkit.log;

import :vulkan.enums;
import :vulkan.structs;

namespace stdr = std::ranges;

namespace cmeta    = stormkit::core::meta;
namespace cmonadic = stormkit::core::monadic;

export namespace stormkit::gpu::vk {
    namespace meta {
        template<typename Func, typename... Ts>
        concept vulkan_fn = std::invocable<Func, Ts...>;

        template<typename Func, typename Out, typename... Ts>
        concept has_out_argument = vulkan_fn<Func, Ts..., Out*>;

        template<typename Func, typename... Ts>
        concept return_void = vulkan_fn<Func, Ts...> and cmeta::is<std::invoke_result_t<Func, Ts...>, void>;

        template<typename Func, typename... Ts>
        concept return_result = vulkan_fn<Func, Ts...> and cmeta::is<std::invoke_result_t<Func, Ts...>, VkResult>;
    } // namespace meta

    template<cmeta::integral T>
    [[nodiscard]]
    constexpr auto make_version(T major, T minor, T patch) noexcept -> u32;
    [[nodiscard]]
    constexpr auto version_major(cmeta::integral auto version) noexcept -> u32;
    [[nodiscard]]
    constexpr auto version_minor(cmeta::integral auto version) noexcept -> u32;
    [[nodiscard]]
    constexpr auto version_patch(cmeta::integral auto version) noexcept -> u32;

    template<typename... Ts, meta::vulkan_fn<Ts...> Func>
        requires meta::return_void<Func, Ts...>
    auto call(const Func& func, Ts&&... args) noexcept -> void;

    template<typename Out, typename... Ts, meta::vulkan_fn<Ts...> Func>
        requires(not meta::return_void<Func, Ts...>)
    [[nodiscard]]
    auto call(const Func& func, Ts&&... args) noexcept -> Out;

    template<typename Out, typename... Ts, meta::has_out_argument<Out, Ts...> Func>
        requires meta::return_void<Func, Ts..., Out*>
    [[nodiscard]]
    auto call(const Func& func, Ts&&... args) noexcept -> Out;

    template<VkResult... SUCCESS_RESULTS, typename... Ts, meta::return_result<Ts...> Func>
    auto call_checked(const Func& func, Ts&&... args) noexcept -> expected<void>;

    template<cmeta::is<VkResult> Out, VkResult... SUCCESS_RESULTS, typename... Ts, meta::return_result<Ts...> Func>
    auto call_checked(const Func& func, Ts&&... args) noexcept -> expected<Out>;

    template<typename Out, typename... Ts, VkResult... SUCCESS_RESULTS, meta::has_out_argument<Out, Ts...> Func>
        requires meta::return_result<Func, Ts..., Out*>
    auto call_checked(const Func& func, Ts&&... args) noexcept -> expected<Out>;

    template<typename... Ts, meta::return_result<Ts...> Func>
    auto call_unchecked(const Func& func, Ts&&... args) noexcept -> void;

    template<cmeta::is<VkResult> Out, typename... Ts, meta::return_result<Ts...> Func>
    [[nodiscard]]
    auto call_unchecked(const Func& func, Ts&&... args) noexcept -> VkResult;

    template<typename Out, typename... Ts, meta::has_out_argument<Out, Ts...> Func>
        requires meta::return_result<Func, Ts..., Out*>
    [[nodiscard]]
    auto call_unchecked(const Func& func, Ts&&... args) noexcept -> Out;

    template<typename Out, typename... Ts, meta::has_out_argument<Out, Ts...> Func>
        requires(meta::return_void<Func, Ts..., Out*> and not cmeta::same_as<Out, void>)
    [[nodiscard]]
    auto allocate(usize count, const Func& func, Ts&&... args) noexcept -> dynarray<Out>;

    template<typename Out, typename... Ts, VkResult... SUCCESS_RESULTS, meta::has_out_argument<Out, Ts...> Func>
        requires(meta::return_result<Func, Ts..., Out*> and not cmeta::same_as<Out, void>)
    auto allocate_checked(usize count, const Func& func, Ts&&... args) noexcept -> expected<dynarray<Out>>;

    template<typename Out, typename... Ts, meta::has_out_argument<Out, Ts...> Func>
        requires(meta::return_result<Func, Ts..., Out*> and not cmeta::same_as<Out, void>)
    [[nodiscard]]
    auto allocate_unchecked(usize count, const Func& func, Ts&&... args) noexcept -> dynarray<Out>;

    template<typename Out, typename... Ts, meta::has_out_argument<Out, Ts..., u32*> Func>
        requires(meta::return_void<Func, Ts..., u32*, Out*> and not cmeta::same_as<Out, void>)
    [[nodiscard]]
    auto enumerate(const Func& func, Ts&&... args) noexcept -> dynarray<Out>;

    template<typename Out, typename... Ts, VkResult... SUCCESS_RESULTS, meta::has_out_argument<Out, Ts..., u32*> Func>
        requires(meta::return_result<Func, Ts..., u32*, Out*> and not cmeta::same_as<Out, void>)
    auto enumerate_checked(const Func& func, Ts&&... args) noexcept -> expected<dynarray<Out>>;

    template<typename Out, typename... Ts, meta::has_out_argument<Out, Ts..., u32*> Func>
        requires(meta::return_result<Func, Ts..., u32*, Out*> and not cmeta::same_as<Out, void>)
    [[nodiscard]]
    auto enumerate_unchecked(const Func& func, Ts&&... args) noexcept -> dynarray<Out>;

    template<typename T>
    class owned {
      public:
        using deleter_type = std::function<void(T)>;

        owned(deleter_type deleter) noexcept;
        ~owned() noexcept;

        owned(const owned&)                    = delete;
        auto operator=(const owned&) -> owned& = delete;

        owned(owned&& other) noexcept;
        auto operator=(owned&& other) noexcept -> owned&;

        auto operator=(T&& value) noexcept -> void;

        auto value() const noexcept -> T;

        operator T() const noexcept;

      private:
        T            m_value = VK_NULL_HANDLE;
        deleter_type m_deleter;
    };

    template<typename T>
    class view {
      public:
        view(T value) noexcept;
        view(const owned<T>& value) noexcept;

        ~view() noexcept;

        view(const view&) noexcept;
        auto operator=(const view&) noexcept -> view&;

        view(view&&) noexcept;
        auto operator=(view&&) noexcept -> view&;

        auto value() const noexcept -> T;

        operator T() const noexcept;

      private:
        T m_value;
    };

    namespace monadic {
        template<typename T>
        [[nodiscard]]
        constexpr auto to_vk() noexcept -> decltype(auto);

        [[nodiscard]]
        constexpr auto to_vk() noexcept -> decltype(auto);

        [[nodiscard]]
        constexpr auto from_vk() noexcept -> decltype(auto);

        template<typename T>
        [[nodiscard]]
        constexpr auto from_vk() noexcept -> decltype(auto);
    } // namespace monadic
} // namespace stormkit::gpu::vk

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit::gpu::vk {
    /////////////////////////////////////
    /////////////////////////////////////
    template<cmeta::integral T>
    STORMKIT_FORCE_INLINE
    STORMKIT_CONST
    constexpr auto make_version(T major, T minor, T patch) noexcept -> u32 {
        return version_major(major) | version_minor(minor) | version_patch(patch);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto version_major(cmeta::integral auto version) noexcept -> u32 {
        return as<u32>(version) >> 22u;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto version_minor(cmeta::integral auto version) noexcept -> u32 {
        return ((as<u32>(version) >> 12u) & 0x3ffu);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto version_patch(cmeta::integral auto version) noexcept -> u32 {
        return as<u32>(version) & 0xfffu;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename... Ts, meta::vulkan_fn<Ts...> Func>
        requires meta::return_void<Func, Ts...> 
    STORMKIT_FORCE_INLINE
    inline auto call(const Func& func, Ts&&... args) noexcept -> void {
        std::invoke(func, std::forward<Ts>(args)...);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out, typename... Ts, meta::vulkan_fn<Ts...> Func>
        requires(not meta::return_void<Func, Ts...>)
    STORMKIT_FORCE_INLINE
    inline auto call(const Func& func, Ts&&... args) noexcept -> Out {
        return std::invoke(func, std::forward<Ts>(args)...);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out, typename... Ts, meta::has_out_argument<Out, Ts...> Func>
        requires meta::return_void<Func, Ts..., Out*> 
    STORMKIT_FORCE_INLINE
    inline auto call(const Func& func, Ts&&... args) noexcept -> Out {
        auto out = Out {};
        std::invoke(func, std::forward<Ts>(args)..., &out);
        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<VkResult... _SUCCESS_RESULTS, typename... Ts, meta::return_result<Ts...> Func>
    inline auto call_checked(const Func& func, Ts&&... args) noexcept -> expected<void> {
        static constexpr auto SUCCESS_RESULTS = array { VK_SUCCESS, _SUCCESS_RESULTS... };

        using Outexpected = expected<void>;
        auto out_expected = Outexpected { std::in_place };

        const auto result = std::invoke(func, std::forward<Ts>(args)...);
        if (not stdr::any_of(SUCCESS_RESULTS, cmonadic::is(result))) [[likely]]
            out_expected = std::unexpected { vk::from_vk<Result>(result) };

        return out_expected;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<cmeta::is<VkResult> Out, VkResult... _SUCCESS_RESULTS, typename... Ts, meta::return_result<Ts...> Func>
    inline auto call_checked(const Func& func, Ts&&... args) noexcept -> expected<Out> {
        static constexpr auto SUCCESS_RESULTS = array { VK_SUCCESS, _SUCCESS_RESULTS... };

        using Outexpected = expected<Out>;
        auto out_expected = Outexpected { std::in_place };

        const auto result = std::invoke(func, std::forward<Ts>(args)...);
        if (not stdr::any_of(SUCCESS_RESULTS, cmonadic::is(result))) [[likely]]
            out_expected = std::unexpected { vk::from_vk<Result>(result) };
        else
            out_expected = result;

        return out_expected;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out, typename... Ts, VkResult... _SUCCESS_RESULTS, meta::has_out_argument<Out, Ts...> Func>
        requires meta::return_result<Func, Ts..., Out*>
    inline auto call_checked(const Func& func, Ts&&... args) noexcept -> expected<Out> {
        static constexpr auto SUCCESS_RESULTS = array { VK_SUCCESS, _SUCCESS_RESULTS... };

        using Outexpected = expected<Out>;
        auto out_expected = Outexpected { std::in_place };

        auto       out    = Out {};
        const auto result = std::invoke(func, std::forward<Ts>(args)..., &out);
        if (not stdr::any_of(SUCCESS_RESULTS, cmonadic::is(result))) [[likely]]
            out_expected = std::unexpected { vk::from_vk<Result>(result) };
        else
            out_expected = out;

        return out_expected;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename... Ts, meta::return_result<Ts...> Func>
    STORMKIT_FORCE_INLINE
    inline auto call_unchecked(const Func& func, Ts&&... args) noexcept -> void {
        const auto _ = std::invoke(func, std::forward<Ts>(args)...);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<cmeta::is<VkResult> Out, typename... Ts, meta::return_result<Ts...> Func>
    STORMKIT_FORCE_INLINE
    inline auto call_unchecked(const Func& func, Ts&&... args) noexcept -> Out {
        return std::invoke(func, std::forward<Ts>(args)...);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out, typename... Ts, meta::has_out_argument<Out, Ts...> Func>
        requires meta::return_result<Func, Ts..., Out*>
    STORMKIT_FORCE_INLINE
    inline auto call_unchecked(const Func& func, Ts&&... args) noexcept -> Out {
        auto       out = Out {};
        const auto _   = std::invoke(func, std::forward<Ts>(args)..., &out);
        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out, typename... Ts, meta::has_out_argument<Out, Ts...> Func>
        requires(meta::return_void<Func, Ts..., Out*> and not cmeta::same_as<Out, void>)
    inline auto allocate(usize count, const Func& func, Ts&&... args) noexcept -> dynarray<Out> {
        auto out = dynarray<Out> {};
        out.resize(count, VK_NULL_HANDLE);
        std::invoke(func, std::forward<Ts>(args)..., stdr::data(out));
        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out, typename... Ts, VkResult... _SUCCESS_RESULTS, meta::has_out_argument<Out, Ts...> Func>
        requires(meta::return_result<Func, Ts..., Out*> and not cmeta::same_as<Out, void>)
    inline auto allocate_checked(usize count, const Func& func, Ts&&... args) noexcept -> expected<dynarray<Out>> {
        static constexpr auto SUCCESS_RESULTS = array { VK_SUCCESS, _SUCCESS_RESULTS... };

        using Outexpected = expected<dynarray<Out>>;
        auto out_expected = Outexpected { std::in_place };

        auto& out = out_expected.value();
        out.resize(count, VK_NULL_HANDLE);
        const auto result = std::invoke(func, std::forward<Ts>(args)..., stdr::data(out));
        if (not stdr::any_of(SUCCESS_RESULTS, cmonadic::is(result))) [[likely]]
            out_expected = std::unexpected { vk::from_vk<Result>(result) };

        return out_expected;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out, typename... Ts, meta::has_out_argument<Out, Ts...> Func>
        requires(meta::return_result<Func, Ts..., Out*> and not cmeta::same_as<Out, void>)
    inline auto allocate_unchecked(usize count, const Func& func, Ts&&... args) noexcept -> dynarray<Out> {
        auto out = dynarray<Out> {};
        out.resize(count, VK_NULL_HANDLE);
        const auto _ = std::invoke(func, std::forward<Ts>(args)..., stdr::data(out));
        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out, typename... Ts, meta::has_out_argument<Out, Ts..., u32*> Func>
        requires(meta::return_void<Func, Ts..., u32*, Out*> and not cmeta::same_as<Out, void>)
    inline auto enumerate(const Func& func, Ts&&... args) noexcept -> dynarray<Out> {
        auto out  = dynarray<Out> {};
        auto size = 0_u32;
        std::invoke(func, std::forward<Ts>(args)..., &size, nullptr);
        out.resize(size);
        std::invoke(func, std::forward<Ts>(args)..., &size, stdr::data(out));

        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out, typename... Ts, VkResult... _SUCCESS_RESULTS, meta::has_out_argument<Out, Ts..., u32*> Func>
        requires(meta::return_result<Func, Ts..., u32*, Out*> and not cmeta::same_as<Out, void>)
    inline auto enumerate_checked(const Func& func, Ts&&... args) noexcept -> expected<dynarray<Out>> {
        static constexpr auto SUCCESS_RESULTS = array { VK_SUCCESS, _SUCCESS_RESULTS... };

        using Outexpected = expected<dynarray<Out>>;
        auto out_expected = Outexpected { std::in_place };

        auto out  = dynarray<Out> {};
        auto size = 0_u32;
        {
            const auto result = std::invoke(func, std::forward<Ts>(args)..., &size, nullptr);
            if (not stdr::any_of(SUCCESS_RESULTS, cmonadic::is(result))) [[likely]]
                out_expected = std::unexpected { vk::from_vk<Result>(result) };
        }
        out.resize(size);
        {
            const auto result = std::invoke(func, std::forward<Ts>(args)..., &size, stdr::data(out));
            if (not stdr::any_of(SUCCESS_RESULTS, cmonadic::is(result))) [[likely]]
                out_expected = std::unexpected { vk::from_vk<Result>(result) };
            else
                out_expected = std::move(out);
        }

        return out_expected;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out, typename... Ts, meta::has_out_argument<Out, Ts..., u32*> Func>
        requires(meta::return_result<Func, Ts..., u32*, Out*> and not cmeta::same_as<Out, void>)
    inline auto enumerate_unchecked(const Func& func, Ts&&... args) noexcept -> dynarray<Out> {
        auto       out  = dynarray<Out> {};
        auto       size = 0_u32;
        const auto _    = std::invoke(func, std::forward<Ts>(args)..., &size, nullptr);
        out.resize(size);
        const auto _ = std::invoke(func, std::forward<Ts>(args)..., &size, stdr::data(out));

        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline owned<T>::owned(deleter_type deleter) noexcept
        : m_deleter { std::move(deleter) } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline owned<T>::~owned() noexcept {
        if (m_value != VK_NULL_HANDLE) {
            m_deleter(m_value);
            m_value = VK_NULL_HANDLE;
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline owned<T>::owned(owned&& other) noexcept
        : m_value { std::exchange(other.m_value, VK_NULL_HANDLE) }, m_deleter { std::move(other.m_deleter) } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline auto owned<T>::operator=(owned&& other) noexcept -> owned& {
        if (this == &other) [[unlikely]]
            return *this;

        m_value   = std::exchange(other.m_value, VK_NULL_HANDLE);
        m_deleter = std::move(other.m_deleter);

        return *this;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline auto owned<T>::operator=(T&& value) noexcept -> void {
        if (m_value == value) return;
        if (m_value) m_deleter(m_value);
        m_value = std::move(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline auto owned<T>::value() const noexcept -> T {
        return m_value;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline owned<T>::operator T() const noexcept {
        return value();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline view<T>::view(T value) noexcept
        : m_value { value } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline view<T>::view(const owned<T>& value) noexcept
        : m_value { value.value() } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline view<T>::~view() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline view<T>::view(const view&) noexcept = default;
    /////////////////////////////////////
    /////////////////////////////////////

    template<typename T>
    STORMKIT_FORCE_INLINE
    inline auto view<T>::operator=(const view&) noexcept -> view& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline view<T>::view(view&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline auto view<T>::operator=(view&&) noexcept -> view& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline auto view<T>::value() const noexcept -> T {
        return m_value;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline view<T>::operator T() const noexcept {
        return value();
    }

    namespace monadic {
        /////////////////////////////////////
        /////////////////////////////////////
        template<typename T>
        STORMKIT_FORCE_INLINE STORMKIT_CONST
        constexpr auto to_vk() noexcept -> decltype(auto) {
            return []<typename U>(const U& value) static noexcept -> decltype(auto)
                       requires(requires { gpu::vk::to_vk<T>(std::declval<U>()); })
            { return gpu::vk::to_vk<T>(value); };
        }

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE STORMKIT_CONST
        constexpr auto to_vk() noexcept -> decltype(auto) {
            return []<typename T>(const T& value) static noexcept -> decltype(auto) { return gpu::vk::to_vk(value); };
        }

        /////////////////////////////////////
        /////////////////////////////////////
        STORMKIT_FORCE_INLINE STORMKIT_CONST
        constexpr auto from_vk() noexcept -> decltype(auto) {
            return [](auto val) static noexcept -> decltype(auto) { return gpu::vk::from_vk(val); };
        }

        /////////////////////////////////////
        /////////////////////////////////////
        template<typename T>
        STORMKIT_FORCE_INLINE
        STORMKIT_CONST
        constexpr auto from_vk() noexcept -> decltype(auto) {
            return []<typename U>(U val) static noexcept -> T
                       requires(requires { gpu::vk::from_vk<T>(std::declval<U>()); })
            { return gpu::vk::from_vk<T>(val); };
        }
    } // namespace monadic
} // namespace stormkit::gpu::vk
