// Copryright (C) 2022 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#ifndef STORMKIT_PIMPLIMPL_MACRO_HPP
#define STORMKIT_PIMPLIMPL_MACRO_HPP

#define PIMPL_IMPLEMENTATION                                                   \
    namespace stormkit::core {                                                 \
        template<class T, bool Defer>                                          \
        Pimpl<T, Defer>::Pimpl() noexcept(not Defer) {                         \
            if constexpr (Defer) init();                                       \
        }                                                                      \
        template<class T, bool Defer>                                          \
        Pimpl<T, Defer>::~Pimpl() = default;                                   \
        template<class T, bool Defer>                                          \
        template<meta::IsNot<T> First, class... Args>                                \
        Pimpl<T, Defer>::Pimpl(First&& first, Args&&... args) {                \
            init(std::forward<First>(first), std::forward<Args>(args)...);     \
        }                                                                      \
        template<class T, bool Defer>                                          \
        Pimpl<T, Defer>::Pimpl(Pimpl&&) noexcept = default;                    \
        template<class T, bool Defer>                                          \
        auto Pimpl<T, Defer>::operator=(Pimpl&&) noexcept -> Pimpl& = default; \
        template<class T, bool Defer>                                          \
        template<class... Args>                                                \
        auto Pimpl<T, Defer>::init(Args&&... args) -> void {                   \
            m_data = std::make_unique<T>(std::forward<Args>(args)...);         \
        }                                                                      \
        template<class T, bool Defer>                                          \
        auto Pimpl<T, Defer>::operator->() noexcept -> T* {                    \
            return &get();                                                     \
        }                                                                      \
        template<class T, bool Defer>                                          \
        auto Pimpl<T, Defer>::operator->() const noexcept -> const T* {        \
            return &get();                                                     \
        }                                                                      \
        template<class T, bool Defer>                                          \
        auto Pimpl<T, Defer>::operator*() noexcept -> T& {                     \
            return get();                                                      \
        }                                                                      \
        template<class T, bool Defer>                                          \
        auto Pimpl<T, Defer>::operator*() const noexcept -> const T& {         \
            return get();                                                      \
        }                                                                      \
        template<class T, bool Defer>                                          \
        auto Pimpl<T, Defer>::get() noexcept -> T& {                           \
            expects(operator bool());                                          \
            return *m_data;                                                    \
        }                                                                      \
        template<class T, bool Defer>                                          \
        auto Pimpl<T, Defer>::get() const noexcept -> const T& {               \
            expects(operator bool());                                          \
            return *m_data;                                                    \
        }                                                                      \
        template<class T, bool Defer>                                          \
        Pimpl<T, Defer>::operator bool() const noexcept {                      \
            return m_data != nullptr;                                          \
        }                                                                      \
    } // namespace stormkit::core

#endif