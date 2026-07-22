#!/usr/bin/env bash
#
# Build libkiwix, run its test suite, and (only if tests pass) install it
# into BUILD_native_dyn/INSTALL - the prefix kiwix-tools/kiwix-desktop are
# configured to consume via pkg-config.
#
# Usage: scripts/test_and_install.sh [build-dir]
#   build-dir defaults to BUILD_native_dyn/libkiwix.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${1:-${REPO_ROOT}/BUILD_native_dyn/libkiwix}"

if [ ! -d "${BUILD_DIR}" ]; then
  echo "error: build directory '${BUILD_DIR}' does not exist. Run 'meson setup' first." >&2
  exit 1
fi

echo "==> Compiling (${BUILD_DIR})"
meson compile -C "${BUILD_DIR}"

echo "==> Running tests (${BUILD_DIR})"
meson test -C "${BUILD_DIR}" --print-errorlogs

echo "==> Installing (${BUILD_DIR})"
meson install -C "${BUILD_DIR}"

echo "==> Done. Installed under $(meson introspect "${BUILD_DIR}" --buildoptions | python3 -c 'import json,sys; [print(o["value"]) for o in json.load(sys.stdin) if o["name"]=="prefix"]')"
