#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

CMAKE_BIN="${CMAKE_BIN:-cmake}"
CTEST_BIN="${CTEST_BIN:-ctest}"
BUILD_DIR="${BUILD_DIR:-${REPO_ROOT}/build/ca-cellular-automata-tests}"
GENERATOR="${GENERATOR:-Ninja}"
TARGET="genesys_test_cellular_automata"

log() {
    printf '[ca-unit-tests] %s\n' "$*"
}

if ! command -v "${CMAKE_BIN}" >/dev/null 2>&1; then
    printf '[ca-unit-tests] error: cmake not found. Set CMAKE_BIN or install CMake.\n' >&2
    exit 1
fi

if ! command -v "${CTEST_BIN}" >/dev/null 2>&1; then
    printf '[ca-unit-tests] error: ctest not found. Set CTEST_BIN or install CTest.\n' >&2
    exit 1
fi

log "Configuring in ${BUILD_DIR}"
"${CMAKE_BIN}" -S "${REPO_ROOT}" -B "${BUILD_DIR}" -G "${GENERATOR}" \
    -DGENESYS_BUILD_TESTS=ON \
    -DGENESYS_BUILD_TERMINAL_APPLICATION=OFF \
    -DGENESYS_BUILD_WEB_APPLICATION=OFF

log "Building ${TARGET}"
"${CMAKE_BIN}" --build "${BUILD_DIR}" --target "${TARGET}"

log "Running CellularAutomata unit tests"
"${CTEST_BIN}" --test-dir "${BUILD_DIR}" -R CellularAutomata --output-on-failure
