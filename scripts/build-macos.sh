#!/usr/bin/env bash
# Builds and tests GoPieMenu on macOS. CI exports QT_ROOT_DIR from install-qt-action.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build}"
CONFIG="${CONFIG:-Release}"
QT_ROOT="${QT_ROOT_DIR:-}"

CMAKE_PREFIX_ARGS=()
if [[ -n "$QT_ROOT" ]]; then
    export PATH="$QT_ROOT/bin:$PATH"
    CMAKE_PREFIX_ARGS+=("-DCMAKE_PREFIX_PATH=$QT_ROOT")
fi

ARCH_ARGS=()
if [[ -n "${GPM_OSX_ARCHITECTURES:-}" ]]; then
    ARCH_ARGS+=("-DCMAKE_OSX_ARCHITECTURES=$GPM_OSX_ARCHITECTURES")
fi

cmake -S "$ROOT" -B "$BUILD_DIR" \
    -DBUILD_TESTING=ON \
    -DCMAKE_BUILD_TYPE="$CONFIG" \
    ${CMAKE_PREFIX_ARGS[@]+"${CMAKE_PREFIX_ARGS[@]}"} \
    ${ARCH_ARGS[@]+"${ARCH_ARGS[@]}"}

cmake --build "$BUILD_DIR" --config "$CONFIG" --parallel "$(sysctl -n hw.ncpu)"

if [[ "${SKIP_TESTS:-0}" != "1" ]]; then
    # Core and update coverage must pass on the first attempt. The QML suite
    # drives synthetic gestures on an offscreen screen, so a single retry is
    # allowed; every other failure still fails the build.
    ctest --test-dir "$BUILD_DIR" -C "$CONFIG" --output-on-failure \
        --output-junit "$BUILD_DIR/test-results.xml" -E '^ui-'
    ctest --test-dir "$BUILD_DIR" -C "$CONFIG" --output-on-failure \
        --output-junit "$BUILD_DIR/test-results-ui.xml" -R '^ui-' --repeat until-pass:2
fi
