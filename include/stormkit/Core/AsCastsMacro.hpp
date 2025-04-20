// Copryright (C) 2022 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#ifndef STORMKIT_ASCASTS_MACRO_HPP
#define STORMKIT_ASCASTS_MACRO_HPP

#include <stormkit/Core/PlatformMacro.hpp>

#define ASCASTER_STRICT_DECLARE(_To, _From)                                                    \
    template<stormkit::meta::IsStrict<_To> To, stormkit::meta::IsStrict<_From> From>           \
    struct stormkit::casts::AsCaster<To, From> {                                               \
        static constexpr auto operator()(const From&                 from,                     \
                                         const std::source_location& location) noexcept -> To; \
    };

#define ASCASTER_STRICT_DEFINE(_To, _From)                                                \
    template<stormkit::meta::IsStrict<_To> To, stormkit::meta::IsStrict<_From> From>      \
    STORMKIT_FORCE_INLINE constexpr auto stormkit::casts::AsCaster<To, From>::operator()( \
        const From&                                  from,                                \
        [[maybe_unused]] const std::source_location& location) noexcept -> To

#define ASCASTER_STRICT(_To, _From)     \
    ASCASTER_STRICT_DECLARE(_To, _From) \
    ASCASTER_STRICT_DEFINE(_To, _From)

#define ASCASTER_DECLARE(_To, _From)                                                           \
    template<_To To, _From From>                                                               \
    struct stormkit::casts::AsCaster<To, From> {                                               \
        static constexpr auto operator()(const From&                 from,                     \
                                         const std::source_location& location) noexcept -> To; \
    };

#define ASCASTER_DEFINE(_To, _From)                                                       \
    template<_To To, _From From>                                                          \
    STORMKIT_FORCE_INLINE constexpr auto stormkit::casts::AsCaster<To, From>::operator()( \
        const From&                                  from,                                \
        [[maybe_unused]] const std::source_location& location) noexcept -> To

#define ASCASTER(_To, _From)     \
    ASCASTER_DECLARE(_To, _From) \
    ASCASTER_DEFINE(_To, _From)

#define NARROWCASTER_STRICT_DECLARE(_To, _From)                                                \
    template<stormkit::meta::IsStrict<_To> To, stormkit::meta::IsStrict<_From> From>           \
    struct stormkit::casts::NarrowCaster<To, From> {                                           \
        static constexpr auto operator()(const From&                 from,                     \
                                         const std::source_location& location) noexcept -> To; \
    };

#define NARROWCASTER_STRICT_DEFINE(_To, _From)                                                \
    template<stormkit::meta::IsStrict<_To> To, stormkit::meta::IsStrict<_From> From>          \
    STORMKIT_FORCE_INLINE constexpr auto stormkit::casts::NarrowCaster<To, From>::operator()( \
        const From&                                  from,                                    \
        [[maybe_unused]] const std::source_location& location) noexcept -> To

#define NARROWCASTER_STRICT(_To, _From)     \
    NARROWCASTER_STRICT_DECLARE(_To, _From) \
    NARROWCASTER_STRICT_DEFINE(_To, _From)

#define NARROWCASTER_DECLARE(_To, _From)                                                       \
    template<_To To, _From From>                                                               \
    struct stormkit::casts::NarrowCaster<To, From> {                                           \
        static constexpr auto operator()(const From&                 from,                     \
                                         const std::source_location& location) noexcept -> To; \
    };

#define NARROWCASTER_DEFINE(_To, _From)                                                       \
    template<_To To, _From From>                                                              \
    STORMKIT_FORCE_INLINE constexpr auto stormkit::casts::NarrowCaster<To, From>::operator()( \
        const From&                                  from,                                    \
        [[maybe_unused]] const std::source_location& location) noexcept -> To

#define NARROWCASTER(_To, _From)     \
    NARROWCASTER_DECLARE(_To, _From) \
    NARROWCASTER_DEFINE(_To, _From)

#endif
