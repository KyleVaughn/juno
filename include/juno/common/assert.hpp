#pragma once

#include <juno/config.hpp>

#include <cstdio>  // printf
#include <cstdlib> // abort

//========================================================================================
// Assertions
//========================================================================================
// This file implements features for design by contract (DBC)
//  - We want to check that preconditions and postconditions to functions are enforced.
//    - This allows for easier debugging.
//    - For example, if making our own sqrt(x), we likely want to ASSERT(x >= 0).
//  - We want the checking of these conditions to be opt-in, and when not in use should
//    not impact performance.
//  - We want the ASSERT to be usable in a relase build, which means that we have
//    to make our own, since <cassert> is disabled when NDEBUG is defined.
//  - If we can assume a condition to be true, then we want the compiler to be able
//    to do the same.
//    - Use C++23 [[assume(expr)]] to provide optimization hints to the compiler.
//    - Use ASSERT_ASSUME(expr) to assert that the expression is true when assertions
//      are enabled, and to assume that the expression is true when assertions are
//      disabled.
//
// Usage:
//  ASSERT(expr) - Asserts that expr is true.
//    If JUNO_ENABLE_ASSERTS is 0, ASSERT does nothing and expr is not evaluated.
//
//  ASSERT_NEAR(a, b, eps) - Asserts that a and b are within eps of each other.
//    If JUNO_ENABLE_ASSERTS is 0, ASSERT_NEAR does nothing and a, b, and eps are
//    not evaluated.
//
//  ASSERT_ASSUME(expr) - Asserts that expr is true, with varying effects.
//    If JUNO_ENABLE_ASSERTS is 1, this is equivalent to ASSERT(expr).
//    If JUNO_ENABLE_ASSERTS is 0, this is equivalent to ASSUME(expr).
//
//  NOTE:
//    1. ASSUME(expr) must be able to be evaluated at compile time. If the
//        assumption does not hold, the program will be ill-formed.
//    2. printf is used so that ASSERT and ASSERT_NEAR can be used in device code.

#if !JUNO_ENABLE_ASSERTS
#  define ASSERT(expr)
#  define ASSERT_NEAR(a, b, eps)
#  define ASSERT_ASSUME(expr) ASSUME(expr)
#else
#  define ASSERT(cond)                                                                   \
    if (!(cond)) {                                                                       \
      juno::failedAssert(__FILE__, __LINE__, #cond);                                     \
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

//========================================================================================
// Implementation
//========================================================================================
namespace juno
{

//------------------------------------------------------------------------------
[[noreturn]] HOSTDEV inline void
failedAssert(char const * const file, int const line, char const * const msg) noexcept
{
  printf("Assertion failed: %s:%d: %s\n", file, line, msg);
  abort();
}

//------------------------------------------------------------------------------
[[noreturn]] HOSTDEV inline void
failedAssertNear(char const * const file, int const line, char const * const a,
                 char const * const b, char const * const eps) noexcept
{
  printf("Assertion failed: %s:%d: Expected %s == %s +/- %s\n", file, line, a, b, eps);
  abort();
}

} // namespace juno

#endif // JUNO_ENABLE_ASSERTS
