#!/usr/bin/env bash
# Deploys Qt into the app bundle, signs it ad hoc, checks SDK-independent startup,
# and writes GoPieMenu-<version>.dmg plus SHA256SUMS.macos.txt to Output/.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build}"
CONFIG="${CONFIG:-Release}"
OUTPUT_DIR="${OUTPUT_DIR:-$ROOT/Output}"
QT_ROOT="${QT_ROOT_DIR:-}"
VERSION="$(sed -n 's/^project(GoPieMenu VERSION \([0-9][0-9.]*\).*/\1/p' "$ROOT/CMakeLists.txt")"
if [[ -z "$VERSION" ]]; then
    echo "Could not read the application version from CMakeLists.txt." >&2
    exit 1
fi

APP="$BUILD_DIR/GoPieMenu.app"
[[ -d "$APP" ]] || APP="$BUILD_DIR/$CONFIG/GoPieMenu.app"
[[ -d "$APP" ]] || { echo "GoPieMenu.app not found; run scripts/build-macos.sh first." >&2; exit 1; }

if [[ -n "$QT_ROOT" ]]; then
    export PATH="$QT_ROOT/bin:$PATH"
fi

macdeployqt "$APP" -qmldir="$ROOT/src/ui/qml" -always-overwrite

# Ad-hoc signing keeps the bundle launchable on the build machine and on downloaded
# copies after the user lets Gatekeeper run it. Notarization needs a Developer ID.
xattr -cr "$APP"
codesign --force --deep --sign - "$APP"
codesign --verify --deep --strict "$APP"

if [[ "${SKIP_SMOKE_TEST:-0}" != "1" ]]; then
    SMOKE_LOG="$BUILD_DIR/macos-smoke.log"
    # Deployed bundles must start without the Qt SDK on PATH.
    if ! QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software \
        "$APP/Contents/MacOS/GoPieMenu" --smoke-test >"$SMOKE_LOG" 2>&1; then
        cat "$SMOKE_LOG" >&2
        echo "SDK-independent startup smoke test failed." >&2
        exit 1
    fi
    echo "SDK-independent startup smoke test passed."
fi

mkdir -p "$OUTPUT_DIR"
STAGE="$BUILD_DIR/dmg"
rm -rf "$STAGE"
mkdir -p "$STAGE"
cp -R "$APP" "$STAGE/"
ln -s /Applications "$STAGE/Applications"
DMG="$OUTPUT_DIR/GoPieMenu-$VERSION.dmg"
rm -f "$DMG"
hdiutil create -volname "GoPieMenu $VERSION" -srcfolder "$STAGE" -ov -format UDZO "$DMG"
(
    cd "$OUTPUT_DIR"
    shasum -a 256 "GoPieMenu-$VERSION.dmg" >SHA256SUMS.macos.txt
)
echo "Packaged GoPieMenu $VERSION as $DMG"
