#!/bin/bash

# Fail on any error.
set -e

DEST="${KOKORO_ARTIFACTS_DIR}/dest"
OUT="${KOKORO_ARTIFACTS_DIR}/out"
CMAKE_SRC=${KOKORO_ARTIFACTS_DIR}/git/cmake_src

BASEDIR=$(dirname "$0")

python3 --version
python3 ${BASEDIR}/build.py "${CMAKE_SRC}" "${OUT}" "${DEST}" "${KOKORO_BUILD_ID}" \
  --cmake=${KOKORO_ARTIFACTS_DIR}/git/cmake/bin/cmake \
  --ninja=${KOKORO_ARTIFACTS_DIR}/git/ninja/ninja \
  --android-cmake=${KOKORO_ARTIFACTS_DIR}/git/android-cmake \
  --clang-repo=${KOKORO_ARTIFACTS_DIR}/git/clang
