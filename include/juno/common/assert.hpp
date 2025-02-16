#pragma once

#include <juno/config.hpp>

#include <cstdio>  // printf
#include <cstdlib> // exit

//==============================================================================
// Assertions
//==============================================================================
//
// ASSERT(expr) - Asserts that expr is true.
//  If JUNO_ENABLE_ASSERTS is 0, ASSERT does nothing and expr is not evaluated.
//
// ASSERT_NEAR(a, b, eps) - Asserts that a and b are within eps of each other.
//  If JUNO_ENABLE_ASSERTS is 0, ASSERT_NEAR does nothing and a, b, and eps are not
//  evaluated.
//
// ASSERT_ASSUME(expr) - Asserts that expr is true, with varying effects.
//  If JUNO_ENABLE_ASSERTS is 1, this is equivalent to ASSERT(expr).
//  If JUNO_ENABLE_ASSERTS is 0, this is equivalent to ASSUME(expr).
//
// NOTE:
//  1. ASSUME(expr) must be able to be evaluated at compile time. If the
//      assumption does not hold, the program will be ill-formed.
//
//  2. printf is used so that ASSERT and ASSERT_NEAR can be used in device code.

#if !JUNO_ENABLE_ASSERTS
#  define ASSERT(expr)
#  define ASSERT_NEAR(a, b, eps)
#  define ASSERT_ASSUME(expr) ASSUME(expr)
#else

namespace juno
{

[[noreturn]] inline void
failedAssert(char const * const file, int const line, char const * const msg) noexcept
{
  printf("Assertion failed: %s:%d: %s\n", file, line, msg);
  exit(1);
}

[[noreturn]] inline void
failedAssertNear(char const * const file, int const line, char const * const a,
                 char const * const b, char const * const eps) noexcept
{
  printf("Assertion failed: %s:%d: Expected %s == %s +/- %s\n", file, line, a, b, eps);
  exit(1);
}

} // namespace juno

#  define ASSERT(cond)                                                                   \
    if (!(cond)) {                                                                       \
      juno::failedAssert(__FILE__, __LINE__, #cond);                                      \
    }

#  define ASSERT_NEAR(a, b, eps)                                                         \
    {                                                                                    \
      auto const a_eval = (a);                                                           \
      auto const b_eval = (b);                                                           \
      auto const diff_eval = a_eval < b_eval ? b_eval - a_eval : a_eval - b_eval;        \
      if (diff_eval > (eps)) {                                                           \
        juno::failedAssertNear(__FILE__, __LINE__, #a, #b, #eps);                        \
      }                                                                                  \
    }

#  define ASSERT_ASSUME(expr) ASSERT(expr)

#endif // JUNO_ENABLE_ASSERTS
