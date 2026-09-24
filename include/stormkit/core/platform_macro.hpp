// Copryright (C) 2022 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#ifndef STORMKIT_PLATFORM_MACRO_HPP
#define STORMKIT_PLATFORM_MACRO_HPP

#include <stormkit/core/macro_utils.hpp>

#include <version>

#if defined(__cplusplus)
    #ifndef __clang__ // remove when clang support headerunits with clang-scan-deps
        #if not(_MSC_VER >= 1900 or __cplusplus >= 202002L)
            #error "Stormkit need a C++20 compiler"
        #endif
    #endif
#else
    #error "Stormkit need a c++ compiler"
#endif

#if defined(_MSC_VER) and not defined(__clang__)
    #pragma warning(disable: 4251)
    #define STORMKIT_COMPILER_MSSTL  "MSSTL"
    #define STORMKIT_COMPILER_CXXLIB STORMKIT_COMPILER_MSSTL
    #define STORMKIT_COMPILER_MSVC   "MSVC " STORMKIT_STRINGIRY(_MSC_VER)
    #define STORMKIT_COMPILER        STORMKIT_COMPILER_MSVC
    #define STORMKIT_EXPORT          __declspec(dllexport)
    #define STORMKIT_IMPORT          __declspec(dllimport)
    #define STORMKIT_RESTRICT        __restrict
    #define STORMKIT_PRIVATE
    #define STORMKIT_FORCE_INLINE_IMPL [[msvc::forceinline]]
    #define STORMKIT_INTRINSIC         [[msvc::intrinsic]]
    #define STORMKIT_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
    #define STORMKIT_PUSH_WARNINGS     _Pragma("warning(push)")
    //clang-format off
    #define STORMKIT_WARNING_IGNORE_MSVC(value) STORMKIT_PRAGMA_FROM_STRING(warning(disable : value))
    //clang-format on
    #define STORMKIT_POP_WARNINGS  _Pragma("warning(pop)")
    #define STORMKIT_ARRAY_IF_MSVC array
#elif defined(_MSC_VER) and defined(__clang__)
    #if defined(_LIBCPP_VERSION)
        #define STORMKIT_COMPILER_LIBCPP "libc++"
        #define STORMKIT_COMPILER_CXXLIB STORMKIT_COMPILER_LIBCPP
        #define STORMKIT_ARRAY_IF_MSVC
    #else
        #define STORMKIT_COMPILER_MSSTL  "MSSTL"
        #define STORMKIT_COMPILER_CXXLIB STORMKIT_COMPILER_MSSTL
        #define STORMKIT_ARRAY_IF_MSVC   array
    #endif
    #define STORMKIT_EXPORT                     __declspec(dllexport)
    #define STORMKIT_IMPORT                     __declspec(dllimport)
    #define STORMKIT_PRIVATE                    [[gnu::visibility("hidden")]]
    #define STORMKIT_RESTRICT                   __restrict
    //clang-format off
    #define STORMKIT_WARNING_IGNORE_MSVC(value) STORMKIT_PRAGMA_FROM_STRING(warning(disable : value))
    //clang-format on
    #define STORMKIT_FORCE_INLINE_IMPL          [[gnu::always_inline]]
    #define STORMKIT_INTRINSIC
    #define STORMKIT_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#elif defined(__MINGW32__)
    #if defined(_LIBCPP_VERSION)
        #define STORMKIT_COMPILER_LIBCPP "libc++"
        #define STORMKIT_COMPILER_CXXLIB STORMKIT_COMPILER_LIBCPP
    #else
        #define STORMKIT_COMPILER_LIBSTDCPP "libstdc++"
        #define STORMKIT_COMPILER_CXXLIB    STORMKIT_COMPILER_LIBSTDCPP
    #endif
    #define STORMKIT_EXPORT __declspec(dllexport)
    #define STORMKIT_IMPORT __declspec(dllimport)
    #define STORMKIT_PRIVATE
    #define STORMKIT_RESTRICT          __restrict
    #define STORMKIT_FORCE_INLINE_IMPL [[gnu::always_inline]]
    #define STORMKIT_INTRINSIC
    #define STORMKIT_NO_UNIQUE_ADDRESS [[no_unique_address]]
    #define STORMKIT_ARRAY_IF_MSVC
#else
    #if defined(_LIBCPP_VERSION)
        #define STORMKIT_COMPILER_LIBCPP "libc++"
        #define STORMKIT_COMPILER_CXXLIB STORMKIT_COMPILER_LIBCPP
    #else
        #define STORMKIT_COMPILER_LIBSTDCPP "libstdc++"
        #define STORMKIT_COMPILER_CXXLIB    STORMKIT_COMPILER_LIBSTDCPP
    #endif
    #define STORMKIT_IMPORT            [[gnu::visibility("default")]]
    #define STORMKIT_EXPORT            [[gnu::visibility("default")]]
    #define STORMKIT_PRIVATE           [[gnu::visibility("hidden")]]
    #define STORMKIT_RESTRICT          __restrict
    #define STORMKIT_FORCE_INLINE_IMPL [[gnu::always_inline]]
    #define STORMKIT_INTRINSIC
    #define STORMKIT_NO_UNIQUE_ADDRESS [[no_unique_address]]
    #define STORMKIT_ARRAY_IF_MSVC
#endif

#if __has_cpp_attribute(lifetimebound)
    #define STORMKIT_LIFETIMEBOUND [[lifetimebound]]
#elif __has_cpp_attribute(msvc::lifetimebound)
    #define STORMKIT_LIFETIMEBOUND [[msvc::lifetimebound]]
#elif __has_cpp_attribute(clang::lifetimebound)
    #define STORMKIT_LIFETIMEBOUND [[clang::lifetimebound]]
#else
    #define STORMKIT_LIFETIMEBOUND
#endif

#if __has_cpp_attribute(clang::guarded_by)
    #define STORMKIT_GUARDED_BY(x) [[clang::guarded_by(x)]]
#else
    #define STORMKIT_GUARDED_BY(_)
#endif

#if __has_cpp_attribute(gnu::pure)
    #define STORMKIT_PURE [[gnu::pure]]
#else
    #define STORMKIT_PURE
#endif

#if __has_cpp_attribute(gsl::Owner)
    #define STORMKIT_OWNER [[gsl::Owner]]
#else
    #define STORMKIT_OWNER [[gsl::Owner]]
#endif

#if __has_cpp_attribute(gsl::Pointer)
    #define STORMKIT_VIEW [[gsl::Pointer]]
#else
    #define STORMKIT_VIEW [[gsl::Pointer]]
#endif

#if __has_cpp_attribute(trivially_relocatable)
    #define STORMKIT_TRIVIALLY_RELOCATABLE [[trivially_relocatable]]
#else
    #define STORMKIT_TRIVIALLY_RELOCATABLE [[clang::trivial_abi]]
#endif

#if not defined(STORMKIT_COMPILER_MSVC)
    #if __has_cpp_attribute(gnu::const)
        #define STORMKIT_CONST [[gnu::const]]
    #endif
#endif

#if not defined(STORMKIT_CONST)
    #define STORMKIT_CONST
#endif

// #if defined(STORMKIT_BUILD_DEBUG)
#define STORMKIT_FORCE_INLINE
// #else
//     #define STORMKIT_FORCE_INLINE STORMKIT_FORCE_INLINE_IMPL
// #endif

#if defined(__MINGW32__)
    #define STORMKIT_COMPILER STORMKIT_COMPILER_MINGW
    #if defined(__clang__)
        #define STORMKIT_COMPILER_CLANG string { "MinGW Clang " } + __clang_version__
        #define STORMKIT_COMPILER       STORMKIT_COMPILER_CLANG
        #define STORMKIT_PUSH_WARNINGS  _Pragma("clang diagnostic push")
        #define STORMKIT_POP_WARNINGS   _Pragma("clang diagnostic pop")
    #elif defined(__GNUC__) or defined(__GNUG__)
        #define STORMKIT_COMPILER_GCC           \
            "MinGW GCC "                        \
              + std::as<string>(__GNUC__)       \
              + "."                             \
              + std::as<string>(__GNUC_MINOR__) \
              + "."                             \
              + "."                             \
              + std::as<string>(__GNUC_PATCHLEVEL__)
        #define STORMKIT_COMPILER_MINGW STORMKIT_COMPILER_GCC
        #define STORMKIT_PUSH_WARNINGS  _Pragma("GCC diagnostic push")
        #define STORMKIT_POP_WARNINGS   _Pragma("GCC diagnostic pop")
    #endif
    #define STORMKIT_COMPILER_MINGW STORMKIT_COMPILER
#elif defined(__clang__)
    #define STORMKIT_COMPILER_CLANG string { "Clang " } + __clang_version__
    #define STORMKIT_COMPILER       STORMKIT_COMPILER_CLANG
    #define STORMKIT_PUSH_WARNINGS  _Pragma("clang diagnostic push")
    #define STORMKIT_WARNING_IGNORE_GCC(_)
    #ifndef STORMKIT_WARNING_IGNORE_MSVC
        #define STORMKIT_WARNING_IGNORE_MSVC(_)
    #endif
    #define STORMKIT_WARNING_IGNORE_CLANG(warning) STORMKIT_PRAGMA_FROM_STRING(clang diagnostic ignore warning)
    #define STORMKIT_POP_WARNINGS                  _Pragma("clang diagnostic pop")
#elif defined(__GNUC__) or defined(__GNUG__)
    #define STORMKIT_COMPILER_GCC           \
        "GCC "                              \
          + std::as<string>(__GNUC__)       \
          + "."                             \
          + std::as<string>(__GNUC_MINOR__) \
          + "."                             \
          + "."                             \
          + std::as<string>(__GNUC_PATCHLEVEL__)
    #define STORMKIT_COMPILER      STORMKIT_COMPILER_GCC
    #define STORMKIT_PUSH_WARNINGS _Pragma("GCC diagnostic push")
    #define STORMKIT_WARNING_IGNORE_CLANG(_)
    #ifndef STORMKIT_WARNING_IGNORE_MSVC
        #define STORMKIT_WARNING_IGNORE_MSVC(_)
    #endif
    #define STORMKIT_WARNING_IGNORE_GCC(warning) STORMKIT_PRAGMA_FROM_STRING(GCC diagnostic ignored warning)
    #define STORMKIT_POP_WARNINGS                _Pragma("GCC diagnostic pop")
#endif

#define STORMKIT_WARNING_IGNORE_GCC_CLANG(warning) \
    STORMKIT_WARNING_IGNORE_GCC(warning)           \
    STORMKIT_WARNING_IGNORE_CLANG(warning)

#if defined(__SWITCH__)
    #define STORMKIT_OS_NX "Nintendo Switch"
    #define STORMKIT_BITS_64
    #define STORMKIT_OS STORMKIT_OS_NX
#elif defined(_WIN64)
    #define STORMKIT_OS_WIN64   "Windows 64 bits" A
    #define STORMKIT_OS_WINDOWS STORMKIT_OS_WIN64
    #define STORMKIT_BITS_64
    #define STORMKIT_OS STORMKIT_OS_WIN64
#elif defined(_WIN32)
    #define STORMKIT_OS_WIN32   "Windows 32 bits"
    #define STORMKIT_OS_WINDOWS STORMKIT_OS_WIN32
    #define STORMKIT_BITS_32
    #define STORMKIT_PRIVATE
    #define STORMKIT_OS STORMKIT_OS_WIN32
#elif defined(__ANDROID__)
    #define STORMKIT_OS_ANDROID "Android"
    #define STORMKIT_OS         STORMKIT_OS_ANDROID
#elif defined(__linux__)
    #if defined(__x86_64__)
        #define STORMKIT_OS_LINUX64 "Linux 64 bits"
        #define STORMKIT_OS_LINUX   STORMKIT_OS_LINUX64
        #define STORMKIT_BITS_64
    #else
        #define STORMKIT_OS_LINUX32 "Linux 32 bits"
        #define STORMKIT_OS_LINUX   STORMKIT_OS_LINUX32
        #define STORMKIT_BITS_32
    #endif

    #define STORMKIT_OS STORMKIT_OS_LINUX
#elif defined(__MACH__)
extern "C" {
    #include <TargetConditionals.h>
}
    #if TARGET_OS_IPHONE
        #define STORMKIT_OS_IOS   "IOS"
        #define STORMKIT_OS_APPLE STORMKIT_OS_IOS
    #elif TARGET_OS_SIMULATOR
        #define STORMKIT_OS_IOS_SIMULATOR "IOS Simulator"
        #define STORMKIT_OS_IOS           STORMKIT_OS_IOS_SIMULATOR
        #define STORMKIT_OS_APPLE         STORMKIT_OS_IOS
    #elif TARGET_OS_MAC
        #define STORMKIT_OS_MACOS "OSX 64 bits"
        #define STORMKIT_OS_APPLE STORMKIT_OS_MACOS
    #endif
    #define STORMKIT_OS STORMKIT_OS_APPLE
#else
    #error "Targeted platform not supported !"
#endif

#ifdef STORMKIT_BUILD
    #define STORMKIT_API STORMKIT_EXPORT
#else
    #define STORMKIT_API STORMKIT_IMPORT
#endif

#ifdef _POSIX_VERSION
    #define STORMKIT_POSIX
#endif

#ifdef __GLIBC__
    #define STORMKIT_GLIBC
#endif

#ifdef _LIBCPP_VERSION
    #define STORMKIT_LIBCXX "libc++"
inline constexpr auto STORMKIT_CXX_LIBRARY = STORMKIT_LIBCXX;
#elif defined(STORMKIT_COMPILER_MSVC)
    #define STORMKIT_MSSTL "libc++"
inline constexpr auto STORMKIT_CXX_LIBRARY = STORMKIT_MSSTL;
#else
    #define STORMKIT_LIBSTDCXX "libstdc++"
inline constexpr auto STORMKIT_CXX_LIBRARY = STORMKIT_LIBSTDCXX;
#endif

#ifdef NDEBUG
    #define STORMKIT_RELEASE_MODE
#else
    #define STORMKIT_DEBUG_MODE
#endif

#endif
