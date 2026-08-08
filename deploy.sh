#!/usr/bin/env bash
#
# deploy.sh - build and package Email Clustering Analysis Tool as a
# distributable macOS .app + .dmg, using a PNG from Frontend/assets/ as the
# app icon.
#
# Bundles Qt frameworks by copying a known, hardcoded list of modules and
# fixing the main binary's references with install_name_tool - NOT by
# walking the dependency tree with `otool -L`. This is deliberate: otool
# itself is broken on some newer Xcode installs (it internally shells out to
# a now-removed `otool-classic` helper and fails outright), while
# install_name_tool has no such dependency. Qt's official installer already
# uses @rpath for inter-framework references, so fixing only the main
# binary's rpath is enough - dyld resolves each framework's own @rpath
# references using the whole process's inherited rpath stack, not just its
# own.
#
# Usage:
#   ./deploy.sh                        # uses Frontend/assets/icon.png
#   ./deploy.sh path/to/other_icon.png # use a different source image
#
set -euo pipefail

if [[ "$(uname)" != "Darwin" ]]; then
    echo "Error: deploy.sh uses macOS-only tools (sips, iconutil, install_name_tool, hdiutil)." >&2
    echo "It must be run on macOS." >&2
    exit 1
fi

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ASSETS_DIR="$PROJECT_ROOT/Frontend/assets"
BUILD_DIR="$PROJECT_ROOT/build"
INSTALL_DIR="$PROJECT_ROOT/Installation"
APP_NAME="EmailClusteringFrontend"
APP="$BUILD_DIR/Frontend/$APP_NAME.app"
BINARY="$APP/Contents/MacOS/$APP_NAME"
ICON_ICNS="$ASSETS_DIR/AppIcon.icns"

# Qt modules to bundle. Core/Gui/Widgets/Charts/Concurrent are what
# Frontend/CMakeLists.txt directly links; OpenGL/OpenGLWidgets/DBus are
# pulled in transitively by Qt itself (Widgets/Charts use OpenGL-accelerated
# rendering paths, and Qt commonly weak-links DBus for desktop integration)
# and must be bundled too even though nothing in our own CMakeLists.txt
# mentions them.
QT_MODULES=(QtCore QtGui QtWidgets QtCharts QtConcurrent QtOpenGL QtOpenGLWidgets QtDBus)

# --- Parse arguments -------------------------------------------------------
ICON_PNG="$ASSETS_DIR/icon.png"
for arg in "$@"; do
    ICON_PNG="$arg"
done

if [[ ! -f "$ICON_PNG" ]]; then
    echo "Error: icon PNG not found at: $ICON_PNG" >&2
    echo "" >&2
    echo "Usage: ./deploy.sh [path/to/icon.png]" >&2
    echo "By default, looks for: Frontend/assets/icon.png" >&2
    exit 1
fi

echo "==> Using icon source: $ICON_PNG"

# --- 1. Build a macOS .iconset from the PNG and convert it to .icns --------
echo "==> Generating AppIcon.icns..."

ICONSET_TMP="$(mktemp -d)"
ICONSET_DIR="$ICONSET_TMP/AppIcon.iconset"
mkdir -p "$ICONSET_DIR"

sips -z 16 16     "$ICON_PNG" --out "$ICONSET_DIR/icon_16x16.png"      >/dev/null
sips -z 32 32     "$ICON_PNG" --out "$ICONSET_DIR/icon_16x16@2x.png"   >/dev/null
sips -z 32 32     "$ICON_PNG" --out "$ICONSET_DIR/icon_32x32.png"      >/dev/null
sips -z 64 64     "$ICON_PNG" --out "$ICONSET_DIR/icon_32x32@2x.png"   >/dev/null
sips -z 128 128   "$ICON_PNG" --out "$ICONSET_DIR/icon_128x128.png"    >/dev/null
sips -z 256 256   "$ICON_PNG" --out "$ICONSET_DIR/icon_128x128@2x.png" >/dev/null
sips -z 256 256   "$ICON_PNG" --out "$ICONSET_DIR/icon_256x256.png"    >/dev/null
sips -z 512 512   "$ICON_PNG" --out "$ICONSET_DIR/icon_256x256@2x.png" >/dev/null
sips -z 512 512   "$ICON_PNG" --out "$ICONSET_DIR/icon_512x512.png"    >/dev/null
sips -z 1024 1024 "$ICON_PNG" --out "$ICONSET_DIR/icon_512x512@2x.png" >/dev/null

mkdir -p "$ASSETS_DIR"
iconutil -c icns "$ICONSET_DIR" -o "$ICON_ICNS"
rm -rf "$ICONSET_TMP"

echo "    Icon ready: $ICON_ICNS"

# --- 2. Locate Qt (before building, so we can pin CMake to it) -------------
# This must happen before configure: without an explicit CMAKE_PREFIX_PATH,
# find_package(Qt6 ...) can pick up an unrelated second Qt install (e.g. one
# pulled in transitively by a Homebrew package like OpenCV's GUI backend)
# instead of - or mixed with - this one, producing a binary linked against
# incompatible Qt versions that crashes on launch with a missing-symbol
# error.
echo "==> Locating Qt..."

QT_ROOT=""
if [[ -d "$HOME/Qt" ]]; then
    QT_ROOT="$(find "$HOME/Qt" -maxdepth 2 -type d -name macos 2>/dev/null | sort -V | tail -n 1 || true)"
fi

if [[ -z "$QT_ROOT" ]]; then
    echo "Error: could not find a Qt macOS install under ~/Qt/." >&2
    echo "Set QT_ROOT manually at the top of this script if yours lives elsewhere." >&2
    exit 1
fi

echo "    Qt root: $QT_ROOT"

# --- 3. Configure and build (Release, host architecture, pinned Qt) --------
echo "==> Configuring and building (Release)..."

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cmake -S "$PROJECT_ROOT" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$QT_ROOT"
cmake --build "$BUILD_DIR" --parallel "$(sysctl -n hw.logicalcpu)"

if [[ ! -d "$APP" ]]; then
    echo "Error: expected app bundle not found at: $APP" >&2
    exit 1
fi

echo "    Built: $APP"

# --- 4. Copy Qt frameworks and plugins into the bundle ----------------------
echo "==> Copying Qt frameworks..."

mkdir -p "$APP/Contents/Frameworks"
mkdir -p "$APP/Contents/PlugIns/platforms"
mkdir -p "$APP/Contents/PlugIns/styles"
mkdir -p "$APP/Contents/Resources"

for fw in "${QT_MODULES[@]}"; do
    if [[ -d "$QT_ROOT/lib/$fw.framework" ]]; then
        echo "    $fw"
        cp -R "$QT_ROOT/lib/$fw.framework" "$APP/Contents/Frameworks/"
    else
        echo "    Warning: $fw.framework not found under $QT_ROOT/lib, skipping" >&2
    fi
done

echo "==> Copying Qt plugins..."

# Cocoa platform plugin is required for the app to launch on macOS at all.
if [[ -f "$QT_ROOT/plugins/platforms/libqcocoa.dylib" ]]; then
    cp "$QT_ROOT/plugins/platforms/libqcocoa.dylib" "$APP/Contents/PlugIns/platforms/"
else
    echo "    Warning: libqcocoa.dylib not found - the app will not launch without it." >&2
fi

# Native macOS look-and-feel; not strictly required, but cheap to include.
if [[ -f "$QT_ROOT/plugins/styles/libqmacstyle.dylib" ]]; then
    cp "$QT_ROOT/plugins/styles/libqmacstyle.dylib" "$APP/Contents/PlugIns/styles/"
fi

# --- 5. qt.conf --------------------------------------------------------------
cat > "$APP/Contents/Resources/qt.conf" << 'QTCONF_EOF'
[Paths]
Plugins = PlugIns
Frameworks = Frameworks
QTCONF_EOF

# --- 6. Fix the main binary's references and rpath -------------------------
# Note: this only fixes the main executable, not each framework's own
# internal cross-references. That's intentional - Qt's official installer
# already builds its frameworks with @rpath-relative references to each
# other, and dyld resolves those using the whole process's inherited rpath
# stack, so fixing @executable_path/../Frameworks once on the main binary
# is sufficient.
echo "==> Fixing library references..."

for fw in "${QT_MODULES[@]}"; do
    install_name_tool -change \
        "$QT_ROOT/lib/$fw.framework/Versions/A/$fw" \
        "@executable_path/../Frameworks/$fw.framework/Versions/A/$fw" \
        "$BINARY" 2>/dev/null || true
done

install_name_tool -delete_rpath "$QT_ROOT/lib" "$BINARY" 2>/dev/null || true
install_name_tool -add_rpath "@executable_path/../Frameworks" "$BINARY" 2>/dev/null || true

# --- 7. Ad-hoc code-sign -----------------------------------------------------
# install_name_tool invalidates existing code signatures, and Apple Silicon
# requires at least an ad-hoc signature to run anything. No developer
# certificate needed for this - it's purely local/self-distribution signing.
echo "==> Ad-hoc code-signing the bundle..."
codesign --force --deep --sign - "$APP"

# --- 8. Package as a .dmg ----------------------------------------------------
echo "==> Creating DMG..."

DMG_PATH="$BUILD_DIR/$APP_NAME.dmg"
DMG_STAGING="$BUILD_DIR/dmg_staging"

rm -rf "$DMG_STAGING"
mkdir -p "$DMG_STAGING"
cp -R "$APP" "$DMG_STAGING/"
ln -sf /Applications "$DMG_STAGING/Applications"

rm -f "$DMG_PATH"
hdiutil create \
    -volname "$APP_NAME" \
    -srcfolder "$DMG_STAGING" \
    -ov \
    -format UDZO \
    "$DMG_PATH"

rm -rf "$DMG_STAGING"

mkdir -p "$INSTALL_DIR"
cp "$DMG_PATH" "$INSTALL_DIR/"

echo ""
echo "==> Done!"
echo "    App bundle: $APP"
echo "    DMG:        $INSTALL_DIR/$APP_NAME.dmg"
echo ""
echo "Launch it with:"
echo "  open \"$APP\""
