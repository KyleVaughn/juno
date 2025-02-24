cmake_minimum_required(VERSION 3.31 FATAL_ERROR)

project(
  juno
  VERSION 0.1.0
  HOMEPAGE_URL "https://github.com/KyleVaughn/juno"
  LANGUAGES CXX)

#===============================================================================
# Contents
#===============================================================================
# 1. Options
# 2. Basic CMake configuration
# 3. Find dependencies
# 4. Flags
# 5. Create libjuno target
# 6. Link/set properties for dependencies
# 7. Build optional subdirectories
# 8. Install

#===============================================================================
# Options
#===============================================================================

# Build
#-------------------------------------------------------------------------------

# Build juno as a shared library (ON) or static library (OFF).
option(JUNO_BUILD_SHARED_LIB "Build juno as a shared library" OFF)

# Build tests. These are unit tests that are used to verify correctness.
option(JUNO_BUILD_TESTS "Build tests" ON)

# Features
#------------------------------------------------------------------------------

# Enable assertions. This option enables the JUNO_ASSERT*(condition) macros, 
# which are evaluated regardless of the build type, unlike the standard assert 
# macro which is only evaluated if NDEBUG is not defined.
option(JUNO_ENABLE_ASSERTS "Enable assertions" OFF)

# Enable fast math optimizations.
option(JUNO_ENABLE_FASTMATH "Enable fast math optimizations" ON)

# Set the Float type to 64-bit (double) instead of 32-bit (float). This option
# determines the precision of the floating point numbers used in juno.    
option(JUNO_ENABLE_FLOAT64 "Enable 64-bit Float" OFF)

# External tools/dependencies
#-----------------------------------------------------------------------------

# Use clang-format for code formatting. This option enables the format-check
# and format-fix targets, which check and fix the formatting of the code.
option(JUNO_USE_CLANG_FORMAT "Use clang-format" ON)

# Use clang-tidy for static analysis. Enable clang-tidy on all targets.
option(JUNO_USE_CLANG_TIDY "Use clang-tidy" ON)

# Use CUDA for GPU acceleration. (NOT CURRENTLY SUPPORTED)
option(JUNO_USE_CUDA "Use CUDA" OFF)

# Use HIP for GPU acceleration.
option(JUNO_USE_HIP "Use HIP" OFF)

#===============================================================================
# Basic CMake configuration
#===============================================================================

# Set JUNO_USE_GPU if using HIP or CUDA
if (JUNO_USE_HIP OR JUNO_USE_CUDA)
  set(JUNO_USE_GPU ON)
else()
  set(JUNO_USE_GPU OFF)
endif()

# Disable in-source builds
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR
    "In-source builds are not allowed."
    " Create a separate directory for build files and delete CMakeCache.txt.")
endif()

# If no build type is specified, default to Release
if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, defaulting to Release")
  set(CMAKE_BUILD_TYPE "Release")
endif()

# Set the C++ standard
set(JUNO_CXX_STANDARD 23)
if (PROJECT_IS_TOP_LEVEL)
  set(CMAKE_CXX_STANDARD ${JUNO_CXX_STANDARD} CACHE STRING "Default C++ standard")
  set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "Require C++ standard")
  set(CMAKE_CXX_EXTENSIONS ON CACHE BOOL "Allow C++ extensions")
endif()

# Max log level for compile-time filtering of log messages
# Off = 0,        // no messages
# Error = 1,      // only errors
# Warn = 2,       // errors and warnings
# Info = 3,       // errors, warnings and info
# Debug = 4,      // errors, warnings, info and debug
if (CMAKE_BUILD_TYPE STREQUAL "Release")
  set(JUNO_MAX_LOG_LEVEL 3)
else()
  set(JUNO_MAX_LOG_LEVEL 4)
endif()
message(STATUS "JUNO_MAX_LOG_LEVEL: ${JUNO_MAX_LOG_LEVEL}")

# RPATH handling
# https://gitlab.kitware.com/cmake/community/wikis/doc/cmake/RPATH-handling
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_FULL_LIBDIR}" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}")
endif()

include(GNUInstallDirs)

#===============================================================================
# Find dependencies
#===============================================================================

# Kokkos
# Options for Kokkos are defined using -DKokkos_ENABLE_XYZ=ON/OFF
add_subdirectory(tpl/kokkos)
set(Kokkos_INCLUDE_DIRS
   ${Kokkos_SOURCE_DIR}/core/src
   ${Kokkos_SOURCE_DIR}/containers/src
   ${Kokkos_SOURCE_DIR}/algorithms/src
   ${Kokkos_BINARY_DIR} # to find KokkosCore_config.h
)

# OpenMP
find_package(OpenMP REQUIRED)

# clang-format
if (JUNO_USE_CLANG_FORMAT)
  find_program(CLANG_FORMAT clang-format REQUIRED)
endif()

# clang-tidy
if (JUNO_USE_CLANG_TIDY)
  find_program(CLANG_TIDY clang-tidy REQUIRED)
endif()

# HIP
if (JUNO_USE_HIP)
  include(CheckLanguage)
  check_language(HIP)
  enable_language(HIP)
endif()

#===============================================================================
# Flags
#===============================================================================

# If the compiler supports the flag, use it
include(CheckCXXCompilerFlag)
macro(juno_add_cxx_compiler_flag FLAG)
  string(REGEX REPLACE "-" "" SFLAG ${FLAG})
  check_cxx_compiler_flag(${FLAG} COMPILER_SUPPORT_${SFLAG})
  if(COMPILER_SUPPORT_${SFLAG})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG}")
  endif()
endmacro()

# Set general warning flags
juno_add_cxx_compiler_flag("-Wall")
juno_add_cxx_compiler_flag("-Wextra")
juno_add_cxx_compiler_flag("-Wcast-align")
juno_add_cxx_compiler_flag("-Wcast-qual")
juno_add_cxx_compiler_flag("-Wconversion")
juno_add_cxx_compiler_flag("-Wdouble-promotion")
juno_add_cxx_compiler_flag("-Wfloat-equal")
juno_add_cxx_compiler_flag("-Wimplicit-fallthrough")
juno_add_cxx_compiler_flag("-Wmissing-noreturn")
juno_add_cxx_compiler_flag("-Wnon-virtual-dtor")
juno_add_cxx_compiler_flag("-Wshadow")
juno_add_cxx_compiler_flag("-Wunused")
juno_add_cxx_compiler_flag("-Wvla")
juno_add_cxx_compiler_flag("-Wzero-as-null-pointer-constant")
juno_add_cxx_compiler_flag("-Werror")
juno_add_cxx_compiler_flag("-Wold-style-cast")
juno_add_cxx_compiler_flag("-pedantic")

# Set potentially compiler-specific warning flags
juno_add_cxx_compiler_flag("-Wsuggest-attribute=noreturn")
juno_add_cxx_compiler_flag("-Wunreachable-code")
juno_add_cxx_compiler_flag("-Wconditional-uninitialized")
juno_add_cxx_compiler_flag("-Wheader-hygiene")
juno_add_cxx_compiler_flag("-Wmissing-noreturn")
juno_add_cxx_compiler_flag("-Woverriding-method-mismatch")
juno_add_cxx_compiler_flag("-Wrange-loop-analysis")
juno_add_cxx_compiler_flag("-Wreserved-identifier")
juno_add_cxx_compiler_flag("-Wshadow-all")
juno_add_cxx_compiler_flag("-Wshift-sign-overflow")
juno_add_cxx_compiler_flag("-Wtautological-compare")
juno_add_cxx_compiler_flag("-Wunaligned-access")
juno_add_cxx_compiler_flag("-Wunneeded-member-function")
juno_add_cxx_compiler_flag("-Wundef")
juno_add_cxx_compiler_flag("-Wvector-conversion")

# Math optimizations
if (JUNO_ENABLE_FASTMATH)
  # In addition to the flags below, fast math enables:
  # -fassociative-math: (x + y) - x  == y, etc.
  # -freciprocal-math: (x/y) == x * (1/y)
  # -funsafe-math-optimizations: sqrt(x)*sqrt(y) = sqrt(x*y), etc
  juno_add_cxx_compiler_flag("-Ofast")
else()
  juno_add_cxx_compiler_flag("-O3")
  juno_add_cxx_compiler_flag("-ffinite-math-only")
  juno_add_cxx_compiler_flag("-fno-signed-zeros")
  juno_add_cxx_compiler_flag("-fno-trapping-math")
  juno_add_cxx_compiler_flag("-fno-math-errno")
  juno_add_cxx_compiler_flag("-fno-rounding-math")
  juno_add_cxx_compiler_flag("-fpp-contract=fast")
  juno_add_cxx_compiler_flag("-fexcess-precision=fast")
endif()

# Native architecture/Link-time optimization
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -flto")

# Sanitizers (debug only)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -fsanitize=memory,address,undefined -fno-omit-frame-pointer")

# Enable OpenMP
# Add flags here, so they are transferred to GPU language 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")

# Set compiler-specific flags
if (JUNO_USE_HIP)
  set(CMAKE_HIP_FLAGS "${CMAKE_CXX_FLAGS}")
  set(CMAKE_HIP_STANDARD ${JUNO_CXX_STANDARD})
  set(CMAKE_HIP_STANDARD_REQUIRED ON)
elseif (JUNO_USE_CUDA)
  # NOT CURRENTLY SUPPORTED
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # Would love to use Clang's C++ stdlib, but Kokkos does not play well
  ## Set the default C++ standard library to libc++
  ## set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  # Kokkos does not specify the rpath for libomp, so we need to do it manually, or
  # risk a user setting LD_LIBRARY_PATH.
  get_filename_component(LIBOMP_PATH ${OpenMP_omp_LIBRARY} DIRECTORY)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,${LIBOMP_PATH}")
endif()

#===============================================================================
# Create libjuno target
#===============================================================================

# Sources
set(JUNO_SOURCES
    "src/common/settings.cpp"
    "src/common/logger.cpp"
#    "src/math/matrix.cpp"
#    "src/mesh/polytope_soup.cpp"
#    "src/mesh/face_vertex_mesh.cpp"
#    "src/physics/cross_section.cpp"
#    "src/physics/nuclide.cpp"
#    "src/physics/cross_section_library.cpp"
#    "src/physics/material.cpp"
#    "src/physics/cmfd.cpp"
#    "src/mpact/model.cpp"
#    "src/mpact/powers.cpp"
#    "src/mpact/source.cpp"
#    "src/gmsh/base_gmsh_api.cpp"
#    "src/gmsh/io.cpp"
#    "src/gmsh/model.cpp"
#    "src/gmsh/mesh.cpp"
#    "src/junoc.cpp"
)

if (JUNO_BUILD_SHARED_LIB)
  add_library(juno SHARED ${JUNO_SOURCES})
else()
  add_library(juno STATIC ${JUNO_SOURCES})
endif()

target_include_directories(juno
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

# config.hpp
configure_file(
  "${PROJECT_SOURCE_DIR}/cmake/config.hpp.in"
  "${PROJECT_SOURCE_DIR}/include/juno/config.hpp")

#===============================================================================
# Link/set properties for dependencies
#===============================================================================

# Kokkos
target_link_libraries(juno PUBLIC Kokkos::kokkos)
target_include_directories(juno SYSTEM PUBLIC ${Kokkos_INCLUDE_DIRS})

# OpenMP
target_link_libraries(juno PUBLIC OpenMP::OpenMP_CXX)

# clang-format
if (JUNO_USE_CLANG_FORMAT)
  set(FORMAT_PATTERNS
    src/*.cpp
    include/*.hpp
    cmake/*.hpp.in
    test/*.hpp
    test/*.cpp
    CACHE STRING
    "Patterns to format")

  set(FORMAT_COMMAND ${CLANG_FORMAT})

  # Check formatting, but do not fix it
  add_custom_target(
    format-check
    COMMAND "${CMAKE_COMMAND}"
    -D "FORMAT_COMMAND=${FORMAT_COMMAND}"
    -D "PATTERNS=${FORMAT_PATTERNS}"
    -P "${PROJECT_SOURCE_DIR}/cmake/clang_format.cmake"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    COMMENT "Checking formatting"
    VERBATIM)

  # Fix formatting
  add_custom_target(
    format-fix
    COMMAND "${CMAKE_COMMAND}"
    -D "FORMAT_COMMAND=${FORMAT_COMMAND}"
    -D "PATTERNS=${FORMAT_PATTERNS}"
    -D FIX=YES
    -P "${PROJECT_SOURCE_DIR}/cmake/clang_format.cmake"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    COMMENT "Fixing formatting"
    VERBATIM)
endif()

# clang-tidy
if (JUNO_USE_CLANG_TIDY)
  # Macro for running clang-tidy on the given target
  macro(set_clang_tidy_properties TIDY_TARGET)
    set_target_properties(
      ${TIDY_TARGET} PROPERTIES
      CXX_CLANG_TIDY
      "clang-tidy;--extra-arg=-Wno-unknown-warning-option;--extra-arg=-Wno-ignored-optimization-argument")
  endmacro()
  set_clang_tidy_properties(juno)
endif()

# HIP
if (JUNO_USE_HIP)
  # Set HIP standard to the same as the C++ standard
  set(JUNO_HIP_STANDARD ${CMAKE_CXX_STANDARD})

  # Macro for treating the given target as HIP code
  macro(set_hip_properties HIP_TARGET)
    set_target_properties(${HIP_TARGET} PROPERTIES HIP_STANDARD ${JUNO_HIP_STANDARD})
    set_target_properties(${HIP_TARGET} PROPERTIES HIP_STANDARD_REQUIRED ON)
    set_target_properties(${HIP_TARGET} PROPERTIES HIP_ARCHITECTURES native)
    set_source_files_properties(${ARGN} PROPERTIES LANGUAGE HIP)
  endmacro()

  set_hip_properties(juno ${JUNO_SOURCES})

  #  target_link_libraries(juno PUBLIC hip::device)
endif()

#===============================================================================
# Build optional subdirectories
#===============================================================================

if (JUNO_BUILD_TESTS)
  include(CTest)
  add_subdirectory(test)
endif()

#===============================================================================
# Install
#===============================================================================

install(TARGETS juno
        RUNTIME DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/bin
        LIBRARY DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/lib
        ARCHIVE DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/lib)
install(DIRECTORY include/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include)
