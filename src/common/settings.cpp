#include <juno/common/settings.hpp>

#include <cstdint>

// Suppress warnings for non-const global variables, since these are global
// settings that are intended to be modified by the user.
// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)

//==============================================================================
// LOG
//==============================================================================

namespace juno::settings::logger
{
int32_t level = defaults::level;
bool timestamped = defaults::timestamped;
bool colorized = defaults::colorized;
bool exit_on_error = defaults::exit_on_error;
} // namespace juno::settings::logger

// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)
