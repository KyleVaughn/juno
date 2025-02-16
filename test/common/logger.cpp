#include <juno/common/logger.hpp>

#include <string>

#include "../test_macros.hpp"

TEST_CASE(loggerTest)
{
  juno::logger::reset();

  // Check defaults
  ASSERT(juno::logger::level == juno::logger::levels::info);
  ASSERT(juno::logger::timestamped);
  ASSERT(juno::logger::colorized);
  ASSERT(juno::logger::exit_on_error);

  // Test printing a string
  juno::logger::exit_on_error = false;
  juno::logger::level = juno::logger::levels::debug;
  juno::logger::debug("debug");
  juno::logger::info("info");
  juno::logger::warn("warn");
  juno::logger::error("error");

  // Test printing non-string types
  juno::logger::info(1111);
  juno::logger::info(1.0);
  juno::logger::info(true);
  juno::logger::info(false);

  // Test printing multiple arguments
  juno::logger::info("multiple", " arguments");
  juno::logger::info("multiple", " arguments");
  juno::logger::info("1 + 1 = ", 1 + 1);

  // Test std::string
  std::string const s = "std::string";
  juno::logger::info(s);
  juno::logger::info(s, " with multiple", " arguments");
}

TEST_SUITE(logger) { TEST(loggerTest); }

auto
main() -> int
{
  RUN_SUITE(logger);
  return 0;
}
