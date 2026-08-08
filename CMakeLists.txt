cmake_minimum_required(VERSION 3.20)

# Build for the host architecture by default on macOS. Universal (Intel +
# Apple Silicon) builds are possible via -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64",
# but are NOT the default here because it requires every linked dependency to
# also be universal. Qt's official installer is. Homebrew's OpenCV is NOT
# (Homebrew only builds single-arch, matching whatever Mac it was installed
# on) - attempting a universal build with a single-arch OpenCV causes the
# linker to silently drop OpenCV from the non-matching slice (a *warning*,
# not an error), producing a binary that reports a successful build but
# crashes on launch on the architecture that got dropped. It has also been
# observed to make `otool -L` require otool-classic (for parsing the
# resulting fat binary), which newer Xcode releases no longer ship.
if(APPLE AND NOT CMAKE_OSX_ARCHITECTURES)
    execute_process(COMMAND uname -m OUTPUT_VARIABLE HOST_ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(CMAKE_OSX_ARCHITECTURES "${HOST_ARCH}" CACHE STRING "Build architectures for macOS" FORCE)
endif()

project(MyProject)

# Default to a Release build if the user didn't specify one (see Backend/CMakeLists.txt
# for why this matters - unoptimized Eigen math is dramatically slower).
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_subdirectory(Backend)
add_subdirectory(Frontend)