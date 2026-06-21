#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

CMAKE_BIN="${CMAKE_BIN:-cmake}"
BUILD_DIR="${BUILD_DIR:-${REPO_ROOT}/build/ca-comp-rule90}"
GENERATOR="${GENERATOR:-Ninja}"
TARGET="genesys_terminal_application"
EXAMPLE="smarts/Smart_CellularAutomataCompRule90.cpp"
EXECUTABLE="${BUILD_DIR}/source/applications/terminal/genesys_terminal_application"

log() {
    printf '[ca-comp-rule90] %s\n' "$*"
}

if ! command -v "${CMAKE_BIN}" >/dev/null 2>&1; then
    printf '[ca-comp-rule90] error: cmake not found. Set CMAKE_BIN or install CMake.\n' >&2
    exit 1
fi

log "Configuring in ${BUILD_DIR}"
"${CMAKE_BIN}" -S "${REPO_ROOT}" -B "${BUILD_DIR}" -G "${GENERATOR}" \
    -DGENESYS_BUILD_TERMINAL_APPLICATION=ON \
    -DGENESYS_BUILD_TESTS=OFF \
    -DGENESYS_TERMINAL_EXAMPLE="${EXAMPLE}"

log "Building ${TARGET}"
"${CMAKE_BIN}" --build "${BUILD_DIR}" --target "${TARGET}"

log "Running Rule 90 CellularAutomataComp example"
"${EXECUTABLE}"
