#pragma once

#include <juno/config.hpp>

//==============================================================================
// SETTINGS
//==============================================================================
// A collection of settings that can be used to configure the behavior of the
// library.

// Suppress warnings for non-const global variables, since these are global
// settings that are intended to be modified by the user.
// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)

//==============================================================================
// LOG
//==============================================================================

namespace juno::settings::logger
{

namespace defaults
{
inline constexpr int32_t level = 3; // 3 == info
inline constexpr bool timestamped = true;
inline constexpr bool colorized = true;
inline constexpr bool exit_on_error = true;
} // namespace defaults

// Global settings
extern int32_t level;
extern bool timestamped;
extern bool colorized;
extern bool exit_on_error;

} // namespace juno::settings::logger

// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)
