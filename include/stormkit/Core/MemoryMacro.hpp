// Copryright (C) 2022 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#ifndef STORMKIT_MEMORY_MACRO_HPP
#define STORMKIT_MEMORY_MACRO_HPP

#if defined(_MSC_VER) && _MSC_VER >= 1910
    #define STORMKIT_EBCO __declspec(empty_bases)
#else
    #define STORMKIT_EBCO
#endif

#define SINGLE_ARG(...) __VA_ARGS__

#define ALLOCATE_HELPERS(T)                                     \
    template<class... Args>                                     \
    [[nodiscard]] static inline auto allocate(Args&&... args) { \
        return makeUnique<T>(std::forward<Args>(args)...)       \
            .transform_error(stormkit::core::monadic::assert()) \
            .value();                                           \
    }

/// \exclude
#define DECLARE_PTR_AND_REF_TEMPLATE_(x, y)                        \
    template<class y>                                              \
    using x##OwnedPtr = std::unique_ptr<x<y>>;                     \
    template<class y>                                              \
    using x##SharedPtr = std::shared_ptr<x<y>>;                    \
    template<class y>                                              \
    using x##std::shared_ptr<Const> = std::shared_ptr<const x<y>>; \
    template<class y>                                              \
    using x##WeakPtr = std::weak_ptr<x<y>>;                        \
    template<class y>                                              \
    using x##ConstWeakPtr = std::weak_ptr<const x<y>>;             \
    template<class y>                                              \
    using x##Ref = stormkit::NakedRef<x<y>>;                       \
    template<class y>                                              \
    using x##ConstRef = stormkit::NakedRef<const x<y>>;

/// \exclude
#define DECLARE_PTR_AND_REF_(x)                                 \
    using x##OwnedPtr               = std::unique_ptr<x>;       \
    using x##SharedPtr              = std::shared_ptr<x>;       \
    using x##std::shared_ptr<Const> = std::shared_ptr<const x>; \
    using x##WeakPtr                = std::weak_ptr<x>;         \
    using x##ConstWeakPtr           = std::weak_ptr<const x>;   \
    using x##Ref                    = stormkit::NakedRef<x>;    \
    using x##ConstRef               = stormkit::NakedRef<const x>;

/// \brief Declare pointer and reference_wrapper aliases to a type
/// \param type The type to declare aliases
#define DECLARE_PTR_AND_REF(type) DECLARE_PTR_AND_REF_(type)

/// \brief Declare pointer and reference_wrapper aliases to a type
/// \param type The type to declare aliases
#define DECLARE_PTR_AND_REF_TEMPLATE(type, tp) DECLARE_PTR_AND_REF_TEMPLATE_(type, tp)

/// \brief Add padding to a struct or a class
/// \param size The size of the padding
#define PADDING(size) stormkit::Byte private____padding[size];

/// \exclude
#define STORMKIT_UNUSED(x) (void)(x);

#define STORMKIT_RAII_CAPSULE_OPAQUE(name, x, _constructor, _deleter, release_value) \
    struct name##Scoped {                                                            \
        template<class... Args>                                                      \
        name##Scoped(Args&&... args) noexcept {                                      \
            m_handle = _constructor(std::forward<Args>(args)...);                    \
        }                                                                            \
        explicit name##Scoped(x v) noexcept {                                        \
            m_handle = v;                                                            \
        }                                                                            \
        ~name##Scoped() noexcept {                                                   \
            destroy();                                                               \
        }                                                                            \
        name##Scoped(name##Scoped& other)                    = delete;               \
        auto operator=(name##Scoped& other) -> name##Scoped& = delete;               \
        name##Scoped(name##Scoped&& other) noexcept {                                \
            m_handle = other.release();                                              \
        }                                                                            \
        auto operator=(name##Scoped&& other) noexcept -> name##Scoped& {             \
            m_handle = other.release();                                              \
            return *this;                                                            \
        }                                                                            \
        operator x() noexcept {                                                      \
            return m_handle;                                                         \
        }                                                                            \
        auto handle() noexcept -> x& {                                               \
            return m_handle;                                                         \
        }                                                                            \
        auto handle() const noexcept -> x {                                          \
            return m_handle;                                                         \
        }                                                                            \
        auto release() noexcept -> x {                                               \
            auto tmp = std::exchange(m_handle, release_value);                       \
            return tmp;                                                              \
        }                                                                            \
        auto reset(x handle = release_value) noexcept -> void {                      \
            destroy();                                                               \
            m_handle = handle;                                                       \
        }                                                                            \
                                                                                     \
      private:                                                                       \
        auto destroy() noexcept -> void {                                            \
            if (m_handle != release_value) { _deleter(release()); }                  \
        }                                                                            \
        x m_handle = release_value;                                                  \
    };

#define STORMKIT_RAII_CAPSULE(name, x, deleter)          \
    struct name##Deleter {                               \
        auto operator()(x* ptr) const noexcept -> void { \
            if (ptr) [[likely]]                          \
                deleter(ptr);                            \
        }                                                \
    };                                                   \
    using name##Scoped = std::unique_ptr<x, name##Deleter>;
#define STORMKIT_RAII_CAPSULE_PP(name, x, deleter)       \
    struct name##Deleter {                               \
        auto operator()(x* ptr) const noexcept -> void { \
            if (ptr) [[likely]]                          \
                deleter(&ptr);                           \
        }                                                \
    };                                                   \
    using name##Scoped = std::unique_ptr<x, name##Deleter>;

#define STORMKIT_FORWARD(x) static_cast<decltype(x) &&>(x);
#define STORMKIT_RETURNS(expr)                 \
    noexcept(noexcept(expr))->decltype(expr) { \
        return expr;                           \
    }
#define STORMKIT_OVERLOADS_OF(name) [&](auto&&... args) STORMKIT_RETURNS(name(FWD(args)...))

#endif
