# Setup Notes

## Prerequisites (macOS)

- **CMake** (3.16+)
- **Qt6** — install via the [Qt Online Installer](https://www.qt.io/download-qt-installer), including the **Charts** module (not part of the default component set — expand your Qt version's tree and check it under "Additional Libraries")
- **Eigen3** — `brew install eigen`

Do **not** `brew install opencv` or `qt`/`qtbase` for this project — see "Known gotchas" below for why.

## Building

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

`CMAKE_BUILD_TYPE` defaults to `Release` automatically (see comments in `CMakeLists.txt`) — debug builds are dramatically slower for the Eigen-heavy matrix math in the K-Means/PCA pipeline.

## Packaging a distributable .app (macOS)

```bash
./deploy.sh                        # uses Frontend/assets/icon.png as the app icon
./deploy.sh path/to/icon.png       # or specify a different source image
```

Produces `build/Frontend/EmailClusteringFrontend.app` and a `.dmg` in `Installation/`.

The app is **ad-hoc signed, not notarized** (no paid Apple Developer account used). First launch requires right-click → **Open** rather than double-click, to get past Gatekeeper's quarantine warning. This is expected — see `xattr <app>` if curious; `com.apple.quarantine` is normal here, not a bug.

## Known gotchas

**Do not link OpenCV, even partially, unless you actually use it.** This project doesn't call a single OpenCV function anywhere — an earlier version linked `${OpenCV_LIBS}` (every OpenCV module) for no reason, which pulled in `opencv_highgui`. Homebrew's OpenCV build has `highgui` linked against Homebrew's *own* separate Qt install, which loads a second, conflicting copy of Qt into the process alongside the official Qt install this project actually links against — causing a `Symbol not found` crash at launch. If you ever need OpenCV again, link only the specific modules you use (e.g. `opencv_core`), never the full `${OpenCV_LIBS}` sweep.

**`otool` may be broken on newer Xcode installs.** Some recent Xcode releases dropped an internal `otool-classic` helper that `otool -L` (and therefore `macdeployqt`) depends on for certain binaries, causing a `can't find or exec: otool-classic` error. `deploy.sh` avoids this entirely by bundling Qt manually with `install_name_tool` and a hardcoded module list rather than dynamically walking dependencies via `otool`. If a future dependency change needs the module list expanded, edit `QT_MODULES` near the top of `deploy.sh`.

**Pin `CMAKE_PREFIX_PATH` to your intended Qt install.** `deploy.sh` does this automatically (`-DCMAKE_PREFIX_PATH="$QT_ROOT"`, pointed at whichever `~/Qt/<version>/macos` it finds). Without this, `find_package(Qt6 ...)` can silently resolve against an unintended second Qt install if one exists anywhere else on the system (e.g. pulled in transitively by some other Homebrew formula).

**Universal (Intel + Apple Silicon) builds are opt-in, not default**, since Homebrew only builds single-architecture packages — a universal build needs every dependency to also be universal. Default is your Mac's native architecture. To attempt universal:
```bash
cmake -S . -B build -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" -DCMAKE_BUILD_TYPE=Release
```
