#!/bin/bash

export OMP_PROC_BIND=spread
COMPILER=clang++
BUILD_DIR=build/clang
cmake -S . -B $BUILD_DIR \
      -DCMAKE_CXX_COMPILER=$COMPILER \
      -DKokkos_ENABLE_SERIAL=ON \
      -DKokkos_ENABLE_OPENMP=ON \
      -DKokkos_ENABLE_TESTS=OFF \
      -DKokkos_ENABLE_DEBUG=ON \
      -DKokkos_ENABLE_DEBUG_BOUNDS_CHECK=ON \
      -DKokkos_ARCH_NATIVE=ON \
&& cmake --build $BUILD_DIR -j \
&& ctest --test-dir $BUILD_DIR
