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
        template<typename Func, typename... Ts>
        concept IsVulkanFunc = std::invocable<Func, Ts...>;

        template<typename Func, typename Out, typename... Ts>
        concept HasOutValueAsArgument = IsVulkanFunc<Func, Ts..., Out*>;

        template<typename Func, typename... Ts>
        concept HasNoReturnValue = IsVulkanFunc<Func, Ts...> and cmeta::is<std::invoke_result_t<Func, Ts...>, void>;

        template<typename Func, typename... Ts>
        concept HasResultReturnValue = IsVulkanFunc<Func, Ts...> and cmeta::is<std::invoke_result_t<Func, Ts...>, VkResult>;
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

    template<typename... Ts, meta::IsVulkanFunc<Ts...> Func>
        requires meta::HasNoReturnValue<Func, Ts...>
    auto call(const Func& func, Ts&&... args) noexcept -> void;

    template<typename Out, typename... Ts, meta::IsVulkanFunc<Ts...> Func>
        requires(not meta::HasNoReturnValue<Func, Ts...>)
    [[nodiscard]]
    auto call(const Func& func, Ts&&... args) noexcept -> Out;

    template<typename Out, typename... Ts, meta::HasOutValueAsArgument<Out, Ts...> Func>
        requires meta::HasNoReturnValue<Func, Ts..., Out*>
    [[nodiscard]]
    auto call(const Func& func, Ts&&... args) noexcept -> Out;

    template<VkResult... SUCCESS_RESULTS, typename... Ts, meta::HasResultReturnValue<Ts...> Func>
    auto call_checked(const Func& func, Ts&&... args) noexcept -> expected<void>;

    template<cmeta::is<VkResult> Out, VkResult... SUCCESS_RESULTS, typename... Ts, meta::HasResultReturnValue<Ts...> Func>
    auto call_checked(const Func& func, Ts&&... args) noexcept -> expected<Out>;

    template<typename Out, typename... Ts, VkResult... SUCCESS_RESULTS, meta::HasOutValueAsArgument<Out, Ts...> Func>
        requires meta::HasResultReturnValue<Func, Ts..., Out*>
    auto call_checked(const Func& func, Ts&&... args) noexcept -> expected<Out>;

    template<typename... Ts, meta::HasResultReturnValue<Ts...> Func>
    auto call_unchecked(const Func& func, Ts&&... args) noexcept -> void;

    template<cmeta::is<VkResult> Out, typename... Ts, meta::HasResultReturnValue<Ts...> Func>
    [[nodiscard]]
    auto call_unchecked(const Func& func, Ts&&... args) noexcept -> VkResult;

    template<typename Out, typename... Ts, meta::HasOutValueAsArgument<Out, Ts...> Func>
        requires meta::HasResultReturnValue<Func, Ts..., Out*>
    [[nodiscard]]
    auto call_unchecked(const Func& func, Ts&&... args) noexcept -> Out;

    template<typename Out, typename... Ts, meta::HasOutValueAsArgument<Out, Ts...> Func>
        requires(meta::HasNoReturnValue<Func, Ts..., Out*> and not cmeta::same_as<Out, void>)
    [[nodiscard]]
    auto allocate(usize count, const Func& func, Ts&&... args) noexcept -> dynarray<Out>;

    template<typename Out, typename... Ts, VkResult... SUCCESS_RESULTS, meta::HasOutValueAsArgument<Out, Ts...> Func>
        requires(meta::HasResultReturnValue<Func, Ts..., Out*> and not cmeta::same_as<Out, void>)
    auto allocate_checked(usize count, const Func& func, Ts&&... args) noexcept -> expected<dynarray<Out>>;

    template<typename Out, typename... Ts, meta::HasOutValueAsArgument<Out, Ts...> Func>
        requires(meta::HasResultReturnValue<Func, Ts..., Out*> and not cmeta::same_as<Out, void>)
    [[nodiscard]]
    auto allocate_unchecked(usize count, const Func& func, Ts&&... args) noexcept -> dynarray<Out>;

    template<typename Out, typename... Ts, meta::HasOutValueAsArgument<Out, Ts..., u32*> Func>
        requires(meta::HasNoReturnValue<Func, Ts..., u32*, Out*> and not cmeta::same_as<Out, void>)
    [[nodiscard]]
    auto enumerate(const Func& func, Ts&&... args) noexcept -> dynarray<Out>;

    template<typename Out, typename... Ts, VkResult... SUCCESS_RESULTS, meta::HasOutValueAsArgument<Out, Ts..., u32*> Func>
        requires(meta::HasResultReturnValue<Func, Ts..., u32*, Out*> and not cmeta::same_as<Out, void>)
    auto enumerate_checked(const Func& func, Ts&&... args) noexcept -> expected<dynarray<Out>>;

    template<typename Out, typename... Ts, meta::HasOutValueAsArgument<Out, Ts..., u32*> Func>
        requires(meta::HasResultReturnValue<Func, Ts..., u32*, Out*> and not cmeta::same_as<Out, void>)
    [[nodiscard]]
    auto enumerate_unchecked(const Func& func, Ts&&... args) noexcept -> dynarray<Out>;

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
    template<typename... Ts, meta::IsVulkanFunc<Ts...> Func>
        requires meta::HasNoReturnValue<Func, Ts...> 
    STORMKIT_FORCE_INLINE
    inline auto call(const Func& func, Ts&&... args) noexcept -> void {
        std::invoke(func, std::forward<Ts>(args)...);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out, typename... Ts, meta::IsVulkanFunc<Ts...> Func>
        requires(not meta::HasNoReturnValue<Func, Ts...>)
    STORMKIT_FORCE_INLINE
    inline auto call(const Func& func, Ts&&... args) noexcept -> Out {
        return std::invoke(func, std::forward<Ts>(args)...);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out, typename... Ts, meta::HasOutValueAsArgument<Out, Ts...> Func>
        requires meta::HasNoReturnValue<Func, Ts..., Out*> 
    STORMKIT_FORCE_INLINE
    inline auto call(const Func& func, Ts&&... args) noexcept -> Out {
        auto out = Out {};
        std::invoke(func, std::forward<Ts>(args)..., &out);
        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<VkResult... _SUCCESS_RESULTS, typename... Ts, meta::HasResultReturnValue<Ts...> Func>
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
    template<cmeta::is<VkResult> Out, VkResult... _SUCCESS_RESULTS, typename... Ts, meta::HasResultReturnValue<Ts...> Func>
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
    template<typename Out, typename... Ts, VkResult... _SUCCESS_RESULTS, meta::HasOutValueAsArgument<Out, Ts...> Func>
        requires meta::HasResultReturnValue<Func, Ts..., Out*>
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
    template<typename... Ts, meta::HasResultReturnValue<Ts...> Func>
    STORMKIT_FORCE_INLINE
    inline auto call_unchecked(const Func& func, Ts&&... args) noexcept -> void {
        const auto _ = std::invoke(func, std::forward<Ts>(args)...);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<cmeta::is<VkResult> Out, typename... Ts, meta::HasResultReturnValue<Ts...> Func>
    STORMKIT_FORCE_INLINE
    inline auto call_unchecked(const Func& func, Ts&&... args) noexcept -> Out {
        return std::invoke(func, std::forward<Ts>(args)...);
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out, typename... Ts, meta::HasOutValueAsArgument<Out, Ts...> Func>
        requires meta::HasResultReturnValue<Func, Ts..., Out*>
    STORMKIT_FORCE_INLINE
    inline auto call_unchecked(const Func& func, Ts&&... args) noexcept -> Out {
        auto       out = Out {};
        const auto _   = std::invoke(func, std::forward<Ts>(args)..., &out);
        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out, typename... Ts, meta::HasOutValueAsArgument<Out, Ts...> Func>
        requires(meta::HasNoReturnValue<Func, Ts..., Out*> and not cmeta::same_as<Out, void>)
    inline auto allocate(usize count, const Func& func, Ts&&... args) noexcept -> dynarray<Out> {
        auto out = dynarray<Out> {};
        out.resize(count, VK_NULL_HANDLE);
        std::invoke(func, std::forward<Ts>(args)..., stdr::data(out));
        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out, typename... Ts, VkResult... _SUCCESS_RESULTS, meta::HasOutValueAsArgument<Out, Ts...> Func>
        requires(meta::HasResultReturnValue<Func, Ts..., Out*> and not cmeta::same_as<Out, void>)
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
    template<typename Out, typename... Ts, meta::HasOutValueAsArgument<Out, Ts...> Func>
        requires(meta::HasResultReturnValue<Func, Ts..., Out*> and not cmeta::same_as<Out, void>)
    inline auto allocate_unchecked(usize count, const Func& func, Ts&&... args) noexcept -> dynarray<Out> {
        auto out = dynarray<Out> {};
        out.resize(count, VK_NULL_HANDLE);
        const auto _ = std::invoke(func, std::forward<Ts>(args)..., stdr::data(out));
        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename Out, typename... Ts, meta::HasOutValueAsArgument<Out, Ts..., u32*> Func>
        requires(meta::HasNoReturnValue<Func, Ts..., u32*, Out*> and not cmeta::same_as<Out, void>)
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
    template<typename Out, typename... Ts, VkResult... _SUCCESS_RESULTS, meta::HasOutValueAsArgument<Out, Ts..., u32*> Func>
        requires(meta::HasResultReturnValue<Func, Ts..., u32*, Out*> and not cmeta::same_as<Out, void>)
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
    template<typename Out, typename... Ts, meta::HasOutValueAsArgument<Out, Ts..., u32*> Func>
        requires(meta::HasResultReturnValue<Func, Ts..., u32*, Out*> and not cmeta::same_as<Out, void>)
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
