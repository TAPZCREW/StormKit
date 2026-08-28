module;

#include <stormkit/core/platform_macro.hpp>

#include <status-code/error.hpp>
#include <status-code/status_code.hpp>

#include <format>

export module stormkit.core.status_code;

// clang-format off
export SYSTEM_ERROR2_NAMESPACE_BEGIN
  // clang-format on

  using SYSTEM_ERROR2_NAMESPACE::system_code;
using SYSTEM_ERROR2_NAMESPACE::posix_code;

#ifdef STORMKIT_OS_WINDOWS
using SYSTEM_ERROR2_NAMESPACE::nt_code;
using SYSTEM_ERROR2_NAMESPACE::win32_code;

// using nt_code = SYSTEM_ERROR2_NAMESPACE::status_code<SYSTEM_ERROR2_NAMESPACE::nt_code_domain>;
#endif

// clang-format off
SYSTEM_ERROR2_NAMESPACE_END
// clang-format on
