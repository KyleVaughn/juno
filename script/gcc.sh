#!/bin/bash

export OMP_PROC_BIND=spread
COMPILER=g++
BUILD_DIR=build/gcc
cmake -S . -B $BUILD_DIR \
      -DCMAKE_CXX_COMPILER=$COMPILER \
      -DKokkos_ENABLE_SERIAL=ON \
      -DKokkos_ENABLE_OPENMP=ON \
      -DKokkos_ENABLE_TESTS=OFF \
      -DKokkos_ARCH_ZEN4=ON \
&& cmake --build $BUILD_DIR -j \
&& ctest --test-dir $BUILD_DIR
