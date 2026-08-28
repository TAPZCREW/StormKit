// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
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
        template<typename Func, typename... Args>
        concept IsVulkanFunc = std::invocable<Func, Args...>;

        template<typename Func, typename Out, typename... Args>
        concept HasOutValueAsArgument = IsVulkanFunc<Func, Args..., Out*>;

        template<typename Func, typename... Args>
        concept HasNoReturnValue = IsVulkanFunc<Func, Args...> and cmeta::Is<std::invoke_result_t<Func, Args...>, void>;

        template<typename Func, typename... Args>
        concept HasResultReturnValue = IsVulkanFunc<Func, Args...> and cmeta::Is<std::invoke_result_t<Func, Args...>, VkResult>;
    } // namespace meta

    template<std::integral T>
    [[nodiscard]]
    constexpr auto make_version(T major, T minor, T patch) noexcept -> u32;
    [[nodiscard]]
    constexpr auto version_major(std::integral auto version) noexcept -> u32;
    [[nodiscard]]
    constexpr auto version_minor(std::integral auto version) noexcept -> u32;
    [[nodiscard]]
    constexpr auto version_patch(std::integral auto version) noexcept -> u32;

    template<typename... Args, meta::IsVulkanFunc<Args...> Func>
        requires meta::HasNoReturnValue<Func, Args...>
    auto call(const Func& func, Args&&... args) noexcept -> void;

    template<typename Out, typename... Args, meta::IsVulkanFunc<Args...> Func>
        requires(not meta::HasNoReturnValue<Func, Args...>)
    [[nodiscard]]
    auto call(const Func& func, Args&&... args) noexcept -> Out;

    template<typename Out, typename... Args, meta::HasOutValueAsArgument<Out, Args...> Func>
        requires meta::HasNoReturnValue<Func, Args..., Out*>
    [[nodiscard]]
    auto call(const Func& func, Args&&... args) noexcept -> Out;

    template<VkResult... SUCCESS_RESULTS, typename... Args, meta::HasResultReturnValue<Args...> Func>
    auto call_checked(const Func& func, Args&&... args) noexcept -> Expected<void>;

    template<cmeta::Is<VkResult> Out, VkResult... SUCCESS_RESULTS, typename... Args, meta::HasResultReturnValue<Args...> Func>
    auto call_checked(const Func& func, Args&&... args) noexcept -> Expected<Out>;

    template<typename Out, typename... Args, VkResult... SUCCESS_RESULTS, meta::HasOutValueAsArgument<Out, Args...> Func>
        requires meta::HasResultReturnValue<Func, Args..., Out*>
    auto call_checked(const Func& func, Args&&... args) noexcept -> Expected<Out>;

    template<typename... Args, meta::HasResultReturnValue<Args...> Func>
    auto call_unchecked(const Func& func, Args&&... args) noexcept -> void;

    template<cmeta::Is<VkResult> Out, typename... Args, meta::HasResultReturnValue<Args...> Func>
    [[nodiscard]]
    auto call_unchecked(const Func& func, Args&&... args) noexcept -> VkResult;

    template<typename Out, typename... Args, meta::HasOutValueAsArgument<Out, Args...> Func>
        requires meta::HasResultReturnValue<Func, Args..., Out*>
    [[nodiscard]]
    auto call_unchecked(const Func& func, Args&&... args) noexcept -> Out;

    template<typename Out, typename... Args, meta::HasOutValueAsArgument<Out, Args...> Func>
        requires(meta::HasNoReturnValue<Func, Args..., Out*> and not cmeta::SameAs<Out, void>)
    [[nodiscard]]
    auto allocate(usize count, const Func& func, Args&&... args) noexcept -> dyn_array<Out>;

    template<typename Out, typename... Args, VkResult... SUCCESS_RESULTS, meta::HasOutValueAsArgument<Out, Args...> Func>
        requires(meta::HasResultReturnValue<Func, Args..., Out*> and not cmeta::SameAs<Out, void>)
    auto allocate_checked(usize count, const Func& func, Args&&... args) noexcept -> Expected<dyn_array<Out>>;

    template<typename Out, typename... Args, meta::HasOutValueAsArgument<Out, Args...> Func>
        requires(meta::HasResultReturnValue<Func, Args..., Out*> and not cmeta::SameAs<Out, void>)
    [[nodiscard]]
    auto allocate_unchecked(usize count, const Func& func, Args&&... args) noexcept -> dyn_array<Out>;

    template<typename Out, typename... Args, meta::HasOutValueAsArgument<Out, Args..., u32*> Func>
        requires(meta::HasNoReturnValue<Func, Args..., u32*, Out*> and not cmeta::SameAs<Out, void>)
    [[nodiscard]]
    auto enumerate(const Func& func, Args&&... args) noexcept -> dyn_array<Out>;

    template<typename Out, typename... Args, VkResult... SUCCESS_RESULTS, meta::HasOutValueAsArgument<Out, Args..., u32*> Func>
        requires(meta::HasResultReturnValue<Func, Args..., u32*, Out*> and not cmeta::SameAs<Out, void>)
    auto enumerate_checked(const Func& func, Args&&... args) noexcept -> Expected<dyn_array<Out>>;

    template<typename Out, typename... Args, meta::HasOutValueAsArgument<Out, Args..., u32*> Func>
        requires(meta::HasResultReturnValue<Func, Args..., u32*, Out*> and not cmeta::SameAs<Out, void>)
    [[nodiscard]]
    auto enumerate_unchecked(const Func& func, Args&&... args) noexcept -> dyn_array<Out>;

    template<typename T>
    class Owned {
      public:
        // TODO function
        using Deleter = std::function<void(T)>;

        Owned(Deleter deleter) noexcept;
        ~Owned() noexcept;

        Owned(const Owned&)                    = delete;
        auto operator=(const Owned&) -> Owned& = delete;

        Owned(Owned&& other) noexcept;
        auto operator=(Owned&& other) noexcept -> Owned&;

        auto operator=(T&& value) noexcept -> void;

        auto value() const noexcept -> T;

        operator T() const noexcept;

      private:
        T       m_value = VK_NULL_HANDLE;
        Deleter m_deleter;
    };

    template<typename T>
    class Observer {
      public:
        Observer(T value) noexcept;
        Observer(const Owned<T>& value) noexcept;

        ~Observer() noexcept;

        Observer(const Observer&) noexcept;
        auto operator=(const Observer&) noexcept -> Observer&;

        Observer(Observer&&) noexcept;
        auto operator=(Observer&&) noexcept -> Observer&;

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
    template<std::integral T>
    STORMKIT_FORCE_INLINE
    STORMKIT_CONST
    constexpr auto make_version(T major, T minor, T patch) noexcept -> u32 {
        return version_major(major) | version_minor(minor) | version_patch(patch);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto version_major(std::integral auto version) noexcept -> u32 {
        return as<u32>(version) >> 22u;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto version_minor(std::integral auto version) noexcept -> u32 {
        return ((as<u32>(version) >> 12u) & 0x3ffu);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    STORMKIT_FORCE_INLINE STORMKIT_CONST STORMKIT_INTRINSIC
    constexpr auto version_patch(std::integral auto version) noexcept -> u32 {
        return as<u32>(version) & 0xfffu;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename... Args, meta::IsVulkanFunc<Args...> Func>
        requires meta::HasNoReturnValue<Func, Args...> 
    STORMKIT_FORCE_INLINE
    inline auto call(const Func& func, Args&&... args) noexcept -> void {
        std::invoke(func, std::forward<Args>(args)...);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out, typename... Args, meta::IsVulkanFunc<Args...> Func>
        requires(not meta::HasNoReturnValue<Func, Args...>)
    STORMKIT_FORCE_INLINE
    inline auto call(const Func& func, Args&&... args) noexcept -> Out {
        return std::invoke(func, std::forward<Args>(args)...);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out, typename... Args, meta::HasOutValueAsArgument<Out, Args...> Func>
        requires meta::HasNoReturnValue<Func, Args..., Out*> 
    STORMKIT_FORCE_INLINE
    inline auto call(const Func& func, Args&&... args) noexcept -> Out {
        auto out = Out {};
        std::invoke(func, std::forward<Args>(args)..., &out);
        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<VkResult... _SUCCESS_RESULTS, typename... Args, meta::HasResultReturnValue<Args...> Func>
    inline auto call_checked(const Func& func, Args&&... args) noexcept -> Expected<void> {
        static constexpr auto SUCCESS_RESULTS = array { VK_SUCCESS, _SUCCESS_RESULTS... };

        using OutExpected = Expected<void>;
        auto out_expected = OutExpected { std::in_place };

        const auto result = std::invoke(func, std::forward<Args>(args)...);
        if (not stdr::any_of(SUCCESS_RESULTS, cmonadic::is_equal(result))) [[likely]]
            out_expected = std::unexpected { vk::from_vk<Result>(result) };

        return out_expected;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<cmeta::Is<VkResult> Out, VkResult... _SUCCESS_RESULTS, typename... Args, meta::HasResultReturnValue<Args...> Func>
    inline auto call_checked(const Func& func, Args&&... args) noexcept -> Expected<Out> {
        static constexpr auto SUCCESS_RESULTS = array { VK_SUCCESS, _SUCCESS_RESULTS... };

        using OutExpected = Expected<Out>;
        auto out_expected = OutExpected { std::in_place };

        const auto result = std::invoke(func, std::forward<Args>(args)...);
        if (not stdr::any_of(SUCCESS_RESULTS, cmonadic::is_equal(result))) [[likely]]
            out_expected = std::unexpected { vk::from_vk<Result>(result) };
        else
            out_expected = result;

        return out_expected;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out, typename... Args, VkResult... _SUCCESS_RESULTS, meta::HasOutValueAsArgument<Out, Args...> Func>
        requires meta::HasResultReturnValue<Func, Args..., Out*>
    inline auto call_checked(const Func& func, Args&&... args) noexcept -> Expected<Out> {
        static constexpr auto SUCCESS_RESULTS = array { VK_SUCCESS, _SUCCESS_RESULTS... };

        using OutExpected = Expected<Out>;
        auto out_expected = OutExpected { std::in_place };

        auto       out    = Out {};
        const auto result = std::invoke(func, std::forward<Args>(args)..., &out);
        if (not stdr::any_of(SUCCESS_RESULTS, cmonadic::is_equal(result))) [[likely]]
            out_expected = std::unexpected { vk::from_vk<Result>(result) };
        else
            out_expected = out;

        return out_expected;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename... Args, meta::HasResultReturnValue<Args...> Func>
    STORMKIT_FORCE_INLINE
    inline auto call_unchecked(const Func& func, Args&&... args) noexcept -> void {
        const auto _ = std::invoke(func, std::forward<Args>(args)...);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<cmeta::Is<VkResult> Out, typename... Args, meta::HasResultReturnValue<Args...> Func>
    STORMKIT_FORCE_INLINE
    inline auto call_unchecked(const Func& func, Args&&... args) noexcept -> Out {
        return std::invoke(func, std::forward<Args>(args)...);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out, typename... Args, meta::HasOutValueAsArgument<Out, Args...> Func>
        requires meta::HasResultReturnValue<Func, Args..., Out*>
    STORMKIT_FORCE_INLINE
    inline auto call_unchecked(const Func& func, Args&&... args) noexcept -> Out {
        auto       out = Out {};
        const auto _   = std::invoke(func, std::forward<Args>(args)..., &out);
        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out, typename... Args, meta::HasOutValueAsArgument<Out, Args...> Func>
        requires(meta::HasNoReturnValue<Func, Args..., Out*> and not cmeta::SameAs<Out, void>)
    inline auto allocate(usize count, const Func& func, Args&&... args) noexcept -> dyn_array<Out> {
        auto out = dyn_array<Out> {};
        out.resize(count, VK_NULL_HANDLE);
        std::invoke(func, std::forward<Args>(args)..., stdr::data(out));
        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out, typename... Args, VkResult... _SUCCESS_RESULTS, meta::HasOutValueAsArgument<Out, Args...> Func>
        requires(meta::HasResultReturnValue<Func, Args..., Out*> and not cmeta::SameAs<Out, void>)
    inline auto allocate_checked(usize count, const Func& func, Args&&... args) noexcept -> Expected<dyn_array<Out>> {
        static constexpr auto SUCCESS_RESULTS = array { VK_SUCCESS, _SUCCESS_RESULTS... };

        using OutExpected = Expected<dyn_array<Out>>;
        auto out_expected = OutExpected { std::in_place };

        auto& out = out_expected.value();
        out.resize(count, VK_NULL_HANDLE);
        const auto result = std::invoke(func, std::forward<Args>(args)..., stdr::data(out));
        if (not stdr::any_of(SUCCESS_RESULTS, cmonadic::is_equal(result))) [[likely]]
            out_expected = std::unexpected { vk::from_vk<Result>(result) };

        return out_expected;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out, typename... Args, meta::HasOutValueAsArgument<Out, Args...> Func>
        requires(meta::HasResultReturnValue<Func, Args..., Out*> and not cmeta::SameAs<Out, void>)
    inline auto allocate_unchecked(usize count, const Func& func, Args&&... args) noexcept -> dyn_array<Out> {
        auto out = dyn_array<Out> {};
        out.resize(count, VK_NULL_HANDLE);
        const auto _ = std::invoke(func, std::forward<Args>(args)..., stdr::data(out));
        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out, typename... Args, meta::HasOutValueAsArgument<Out, Args..., u32*> Func>
        requires(meta::HasNoReturnValue<Func, Args..., u32*, Out*> and not cmeta::SameAs<Out, void>)
    inline auto enumerate(const Func& func, Args&&... args) noexcept -> dyn_array<Out> {
        auto out  = dyn_array<Out> {};
        auto size = 0_u32;
        std::invoke(func, std::forward<Args>(args)..., &size, nullptr);
        out.resize(size);
        std::invoke(func, std::forward<Args>(args)..., &size, stdr::data(out));

        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out, typename... Args, VkResult... _SUCCESS_RESULTS, meta::HasOutValueAsArgument<Out, Args..., u32*> Func>
        requires(meta::HasResultReturnValue<Func, Args..., u32*, Out*> and not cmeta::SameAs<Out, void>)
    inline auto enumerate_checked(const Func& func, Args&&... args) noexcept -> Expected<dyn_array<Out>> {
        static constexpr auto SUCCESS_RESULTS = array { VK_SUCCESS, _SUCCESS_RESULTS... };

        using OutExpected = Expected<dyn_array<Out>>;
        auto out_expected = OutExpected { std::in_place };

        auto out  = dyn_array<Out> {};
        auto size = 0_u32;
        {
            const auto result = std::invoke(func, std::forward<Args>(args)..., &size, nullptr);
            if (not stdr::any_of(SUCCESS_RESULTS, cmonadic::is_equal(result))) [[likely]]
                out_expected = std::unexpected { vk::from_vk<Result>(result) };
        }
        out.resize(size);
        {
            const auto result = std::invoke(func, std::forward<Args>(args)..., &size, stdr::data(out));
            if (not stdr::any_of(SUCCESS_RESULTS, cmonadic::is_equal(result))) [[likely]]
                out_expected = std::unexpected { vk::from_vk<Result>(result) };
            else
                out_expected = std::move(out);
        }

        return out_expected;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out, typename... Args, meta::HasOutValueAsArgument<Out, Args..., u32*> Func>
        requires(meta::HasResultReturnValue<Func, Args..., u32*, Out*> and not cmeta::SameAs<Out, void>)
    inline auto enumerate_unchecked(const Func& func, Args&&... args) noexcept -> dyn_array<Out> {
        auto       out  = dyn_array<Out> {};
        auto       size = 0_u32;
        const auto _    = std::invoke(func, std::forward<Args>(args)..., &size, nullptr);
        out.resize(size);
        const auto _ = std::invoke(func, std::forward<Args>(args)..., &size, stdr::data(out));

        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline Owned<T>::Owned(Deleter deleter) noexcept
        : m_deleter { std::move(deleter) } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline Owned<T>::~Owned() noexcept {
        if (m_value != VK_NULL_HANDLE) {
            m_deleter(m_value);
            m_value = VK_NULL_HANDLE;
        }
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline Owned<T>::Owned(Owned&& other) noexcept
        : m_value { std::exchange(other.m_value, VK_NULL_HANDLE) }, m_deleter { std::move(other.m_deleter) } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline auto Owned<T>::operator=(Owned&& other) noexcept -> Owned& {
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
    inline auto Owned<T>::operator=(T&& value) noexcept -> void {
        if (m_value == value) return;
        if (m_value) m_deleter(m_value);
        m_value = std::move(value);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline auto Owned<T>::value() const noexcept -> T {
        return m_value;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline Owned<T>::operator T() const noexcept {
        return value();
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline Observer<T>::Observer(T value) noexcept
        : m_value { value } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline Observer<T>::Observer(const Owned<T>& value) noexcept
        : m_value { value.value() } {
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline Observer<T>::~Observer() noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline Observer<T>::Observer(const Observer&) noexcept = default;
    /////////////////////////////////////
    /////////////////////////////////////

    template<typename T>
    STORMKIT_FORCE_INLINE
    inline auto Observer<T>::operator=(const Observer&) noexcept -> Observer& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline Observer<T>::Observer(Observer&&) noexcept = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline auto Observer<T>::operator=(Observer&&) noexcept -> Observer& = default;

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline auto Observer<T>::value() const noexcept -> T {
        return m_value;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    inline Observer<T>::operator T() const noexcept {
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
