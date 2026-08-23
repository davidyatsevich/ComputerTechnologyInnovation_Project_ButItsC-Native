cmake_minimum_required(VERSION 3.20)

# Build universal (Intel + Apple Silicon) binaries on macOS by default.
# Both remaining dependencies support this cleanly: Eigen is header-only (no
# compiled binary, so architecture is a non-issue), and Qt's official
# installer already ships universal frameworks. This was previously
# restricted to host-arch-only because Homebrew's OpenCV was single-arch and
# silently broke universal builds - that's no longer a concern since this
# project doesn't use OpenCV at all. Override with
# -DCMAKE_OSX_ARCHITECTURES=arm64 (or =x86_64) for a single-arch build.
if(APPLE AND NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for macOS" FORCE)
endif()

# Set a minimum supported macOS version explicitly. Without this, CMake
# defaults to whatever the build machine's SDK implies (often quite recent),
# which can produce a binary that builds fine as x86_64 but refuses to launch
# on an older Intel Mac that hasn't been updated to a newer macOS release.
# Apple ended Intel Mac support as of macOS 27 (2026) - macOS 26 (Tahoe) was
# the last Intel-compatible release - so 12.0 comfortably covers the
# remaining supported Intel Mac population without dropping very old,
# already-unsupported hardware.
if(APPLE AND NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum macOS version" FORCE)
endif()

project(MyProject)

enable_testing()

# Default to a Release build if the user didn't specify one (see Backend/CMakeLists.txt
# for why this matters - unoptimized Eigen math is dramatically slower).
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_subdirectory(Backend)
add_subdirectory(Frontend)