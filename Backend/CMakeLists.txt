cmake_minimum_required(VERSION 3.16)

# Build universal (Intel + Apple Silicon) binaries on macOS by default when
# configured standalone. See the top-level CMakeLists.txt for the full
# rationale - the short version is that Eigen (header-only) and Qt's
# official installer both support this cleanly now that OpenCV is no longer
# a dependency.
if(APPLE AND NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for macOS" FORCE)
endif()

# See the top-level CMakeLists.txt for why this is set explicitly.
if(APPLE AND NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum macOS version" FORCE)
endif()

project(EmailClusteringBackend)

# Default to a Release build if the user didn't specify one. Debug builds
# disable compiler optimizations, which is brutal for Eigen-heavy numerical
# code (matrix operations, k-means iterations, eigendecomposition) - a
# realistic dataset can be 30-40x slower unoptimized.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Eigen3 REQUIRED)

include_directories(include)
include_directories(src)

add_library(pipeline
    src/pipeline.cpp
    src/csv_reader.cpp
    src/kmeans_processor.cpp
    src/gru_processor.cpp
)

target_link_libraries(pipeline
    Eigen3::Eigen
)

add_executable(email_clustering_backend src/main.cpp)
target_link_libraries(email_clustering_backend pipeline)