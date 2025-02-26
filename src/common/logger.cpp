#include <juno/common/logger.hpp>

#include <juno/common/assert.hpp>
#include <juno/common/settings.hpp>
#include <juno/config.hpp>

#include <algorithm>  // std::copy
#include <cstdint>    // int32_t
#include <cstdio>     // snprintf
#include <memory>     // std::addressof
#include <string>
#include <string_view>

namespace juno::logger
{

//========================================================================================
// Initialize global variables
//========================================================================================

// Suppress warnings for non-const global variables, since this is a global
// logger 
// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)

int32_t & level = juno::settings::logger::level;
bool & timestamped = juno::settings::logger::timestamped;
bool & colorized = juno::settings::logger::colorized;

TimePoint start_time = Clock::now();
char buffer[buffer_size] = {0};
char const * const buffer_end = buffer + buffer_size;

// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

//========================================================================================
// Functions
//========================================================================================

void
reset() noexcept
{
  // Reset options to default
  level = juno::settings::logger::defaults::level;
  timestamped = juno::settings::logger::defaults::timestamped;
  colorized = juno::settings::logger::defaults::colorized;

  // Reset data
  start_time = Clock::now();
}

//========================================================================================
// toBuffer functions
//========================================================================================

// When we don't use assertions, suppress warnings for unused variables, since the unused
// variables for assertions
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
// NOLINTBEGIN(clang-analyzer-deadcode.DeadStores,clang-diagnostic-unused-variable)

namespace
{

//---------------------------------------------------------------------------------------
auto
appendStringViewToBuffer(char * buffer_pos, std::string_view const sv) noexcept -> char *
{
  auto * const new_pos = buffer_pos + sv.size();
  ASSERT(new_pos < buffer_end);
  std::copy(sv.begin(), sv.end(), buffer_pos);
  return new_pos;
}

} // namespace

//---------------------------------------------------------------------------------------
// string
template <>
auto
toBuffer(char * buffer_pos, char const * const & value) noexcept -> char *
{
  char const * p = value;
  std::string_view const sv(p);
  return appendStringViewToBuffer(buffer_pos, sv);
}

//---------------------------------------------------------------------------------------
// char
template <>
auto
toBuffer(char * buffer_pos, char const & value) noexcept -> char *
{
  *buffer_pos = value;
  ++buffer_pos;
  ASSERT(buffer_pos < buffer_end);
  return buffer_pos;
}

//---------------------------------------------------------------------------------------
// Use snprintf to convert the value to a string and store it in the buffer
template <>
auto
toBuffer(char * buffer_pos, int32_t const & value) noexcept -> char *
{
  int32_t const len = snprintf(nullptr, 0, "%d", value);
  int32_t const written =
      snprintf(buffer_pos, static_cast<uint64_t>(buffer_end - buffer_pos), "%d", value);
  ASSERT(len == written);
  ASSERT(buffer_pos + len < buffer_end);
  return buffer_pos + len;
}

//---------------------------------------------------------------------------------------
template <>
auto
toBuffer(char * buffer_pos, int8_t const & value) noexcept -> char *
{
  // NOLINTNEXTLINE(bugprone-signed-char-misuse,cert-str34-c)
  auto const val32 = static_cast<int32_t>(value);
  return toBuffer(buffer_pos, val32);
}

//---------------------------------------------------------------------------------------
template <>
auto
toBuffer(char * buffer_pos, uint32_t const & value) noexcept -> char *
{
  int32_t const len = snprintf(nullptr, 0, "%u", value);
  int32_t const written =
      snprintf(buffer_pos, static_cast<uint64_t>(buffer_end - buffer_pos), "%u", value);
  ASSERT(len == written);
  ASSERT(buffer_pos + len < buffer_end);
  return buffer_pos + len;
}

//---------------------------------------------------------------------------------------
template <>
auto
toBuffer(char * buffer_pos, int64_t const & value) noexcept -> char *
{
  int32_t const len = snprintf(nullptr, 0, "%ld", value);
  int32_t const written =
      snprintf(buffer_pos, static_cast<uint64_t>(buffer_end - buffer_pos), "%ld", value);
  ASSERT(len == written);
  ASSERT(buffer_pos + len < buffer_end);
  return buffer_pos + len;
}

//---------------------------------------------------------------------------------------
template <>
auto
toBuffer(char * buffer_pos, uint64_t const & value) noexcept -> char *
{
  int32_t const len = snprintf(nullptr, 0, "%lu", value);
  int32_t const written =
      snprintf(buffer_pos, static_cast<uint64_t>(buffer_end - buffer_pos), "%lu", value);
  ASSERT(len == written);
  ASSERT(buffer_pos + len < buffer_end);
  return buffer_pos + len;
}

//---------------------------------------------------------------------------------------
template <>
auto
toBuffer(char * buffer_pos, double const & value) noexcept -> char *
{
  int32_t const len = snprintf(nullptr, 0, "%f", value);
  int32_t const written =
      snprintf(buffer_pos, static_cast<uint64_t>(buffer_end - buffer_pos), "%f", value);
  ASSERT(len == written);
  ASSERT(buffer_pos + len < buffer_end);
  return buffer_pos + len;
}

//---------------------------------------------------------------------------------------
template <>
auto
toBuffer(char * buffer_pos, float const & value) noexcept -> char *
{
  // snprintf does not support float, so we cast to double
  auto const dvalue = static_cast<double>(value);
  return toBuffer(buffer_pos, dvalue);
}

//---------------------------------------------------------------------------------------
template <>
auto
toBuffer(char * buffer_pos, bool const & value) noexcept -> char *
{
  if (value) {
    buffer_pos[0] = 't';
    buffer_pos[1] = 'r';
    buffer_pos[2] = 'u';
    buffer_pos[3] = 'e';
    buffer_pos += 4;
  } else {
    buffer_pos[0] = 'f';
    buffer_pos[1] = 'a';
    buffer_pos[2] = 'l';
    buffer_pos[3] = 's';
    buffer_pos[4] = 'e';
    buffer_pos += 5;
  }
  ASSERT(buffer_pos < buffer_end);
  return buffer_pos;
}

//---------------------------------------------------------------------------------------
template <>
auto
toBuffer(char * buffer_pos, std::string_view const & value) noexcept -> char *
{
  return appendStringViewToBuffer(buffer_pos, value);
}

//---------------------------------------------------------------------------------------
template <>
auto
toBuffer(char * buffer_pos, std::string const & value) noexcept -> char *
{
  std::string_view const sv(value);
  return toBuffer(buffer_pos, sv);
}

// NOLINTEND(clang-analyzer-deadcode.DeadStores,clang-diagnostic-unused-variable)
#pragma GCC diagnostic pop

//=======================================================================================

//---------------------------------------------------------------------------------------
// Add the timestamp to the buffer if the log is timestamped
auto
addTimestamp(char * buffer_pos) noexcept -> char *
{
  if (timestamped) {
    Duration const elapsed_seconds = Clock::now() - start_time;
    Int const hours = static_cast<Int>(elapsed_seconds.count()) / 3600;
    Int const minutes = (static_cast<Int>(elapsed_seconds.count()) / 60) % 60;
    Int const seconds = static_cast<Int>(elapsed_seconds.count()) % 60;
    Int const milliseconds = static_cast<Int>(elapsed_seconds.count() * 1000) % 1000;
    buffer_pos[0] = '[';
    if (hours < 10) {
      buffer_pos[1] = '0';
      buffer_pos[2] = static_cast<char>(hours + '0');
    } else {
      buffer_pos[1] = static_cast<char>((hours / 10) + '0');
      buffer_pos[2] = static_cast<char>((hours % 10) + '0');
    }
    buffer_pos[3] = ':';
    if (minutes < 10) {
      buffer_pos[4] = '0';
      buffer_pos[5] = static_cast<char>(minutes + '0');
    } else {
      buffer_pos[4] = static_cast<char>((minutes / 10) + '0');
      buffer_pos[5] = static_cast<char>((minutes % 10) + '0');
    }
    buffer_pos[6] = ':';
    if (seconds < 10) {
      buffer_pos[7] = '0';
      buffer_pos[8] = static_cast<char>(seconds + '0');
    } else {
      buffer_pos[7] = static_cast<char>((seconds / 10) + '0');
      buffer_pos[8] = static_cast<char>((seconds % 10) + '0');
    }
    buffer_pos[9] = '.';
    if (milliseconds < 10) {
      buffer_pos[10] = '0';
      buffer_pos[11] = '0';
      buffer_pos[12] = static_cast<char>(milliseconds + '0');
    } else if (milliseconds < 100) {
      buffer_pos[10] = '0';
      buffer_pos[11] = static_cast<char>((milliseconds / 10) + '0');
      buffer_pos[12] = static_cast<char>((milliseconds % 10) + '0');
    } else {
      buffer_pos[10] = static_cast<char>((milliseconds / 100) + '0');
      buffer_pos[11] = static_cast<char>(((milliseconds % 100) / 10) + '0');
      buffer_pos[12] = static_cast<char>(((milliseconds % 100) % 10) + '0');
    }
    buffer_pos[13] = ']';
    buffer_pos[14] = ' ';
    buffer_pos += 15;
  } // timestamped
  return buffer_pos;
} // addTimestamp

//---------------------------------------------------------------------------------------
// Add the color to the buffer if the log is colorized
auto
addColor(int32_t const msg_level, char * buffer_pos) noexcept -> char *
{
  if (colorized) {
    buffer_pos[0] = '\033';
    buffer_pos[1] = '[';
    buffer_pos[2] = '1';
    buffer_pos[3] = ';';
    buffer_pos[4] = '3';
    buffer_pos[6] = 'm';
    switch (msg_level) {
    case levels::error: // RED
      // \033[1;31m
      buffer_pos[5] = '1';
      break;
    case levels::warn: // YELLOW
      // \033[1;33m
      buffer_pos[5] = '3';
      break;
    case levels::debug: // MAGENTA
      // \033[1;35m
      buffer_pos[5] = '5';
      break;
    default: // NO COLOR
// False positive, since we -7 then +7 in the next line, so we never
// actually access buffer_pos[< 0]
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
      buffer_pos -= 7;
#pragma GCC diagnostic pop
      break;
    }
    buffer_pos += 7;
  } // if (colorized)
  return buffer_pos;
} // addColor

//---------------------------------------------------------------------------------------
// Add the level to the buffer
auto
addLevel(int32_t const msg_level, char * buffer_pos) noexcept -> char *
{
  switch (msg_level) {
  case levels::error:
    buffer_pos[0] = 'E';
    buffer_pos[1] = 'R';
    buffer_pos[2] = 'R';
    buffer_pos[3] = 'O';
    buffer_pos[4] = 'R';
    buffer_pos += 5;
    break;
  case levels::warn:
    buffer_pos[0] = 'W';
    buffer_pos[1] = 'A';
    buffer_pos[2] = 'R';
    buffer_pos[3] = 'N';
    buffer_pos += 4;
    break;
  case levels::info:
    buffer_pos[0] = 'I';
    buffer_pos[1] = 'N';
    buffer_pos[2] = 'F';
    buffer_pos[3] = 'O';
    buffer_pos += 4;
    break;
  case levels::debug:
    buffer_pos[0] = 'D';
    buffer_pos[1] = 'E';
    buffer_pos[2] = 'B';
    buffer_pos[3] = 'U';
    buffer_pos[4] = 'G';
    buffer_pos += 5;
    break;
  default:
    break;
  }
  buffer_pos[0] = ' ';
  buffer_pos[1] = '-';
  buffer_pos[2] = ' ';
  return buffer_pos + 3;
} // addLevel

//---------------------------------------------------------------------------------------
// Set the preamble of the message
auto
setPreamble(int32_t const msg_level) noexcept -> char *
{
  char * buffer_pos = std::addressof(buffer[0]);
  buffer_pos = addColor(msg_level, buffer_pos);
  buffer_pos = addTimestamp(buffer_pos);
  buffer_pos = addLevel(msg_level, buffer_pos);
  return buffer_pos;
}

//---------------------------------------------------------------------------------------
// Set the postamble of the message
void
setPostamble(char * buffer_pos) noexcept
{
  // Reset color
  if (colorized) {
    buffer_pos[0] = '\033';
    buffer_pos[1] = '[';
    buffer_pos[2] = '0';
    buffer_pos[3] = 'm';
    buffer_pos += 4;
  }
  ASSERT(buffer_pos < buffer_end);

  // Ensure null-terminated string
  buffer_pos[0] = '\0';
}

} // namespace juno::logger
