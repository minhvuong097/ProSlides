cmake_minimum_required(VERSION 3.25)

project(ProSlides
    VERSION 0.9.0
    DESCRIPTION "Professional, lightweight worship presentation software"
    LANGUAGES CXX)

# ----------------------------------------------------------------------------
# Build configuration
# ----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

option(PS_BUILD_TESTS "Build unit tests" OFF)
option(PS_ENABLE_NDI "Enable NewTek NDI output (requires NDI SDK)" OFF)
option(PS_ENABLE_FFMPEG "Enable FFmpeg hardware-accelerated decoding" ON)

if(MSVC)
    add_compile_options(/W4 /permissive- /utf-8 /EHsc /Zc:__cplusplus)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS NOMINMAX WIN32_LEAN_AND_MEAN)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ----------------------------------------------------------------------------
# Qt 6 — QML, SQL, WebSockets, Multimedia
# ----------------------------------------------------------------------------
find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core Gui Qml Quick QuickControls2 Sql WebSockets Network Concurrent Multimedia Svg
)
qt_standard_project_setup(REQUIRES 6.5)

# ----------------------------------------------------------------------------
# FFmpeg (hardware decoding: D3D11VA / DXVA2 / VideoToolbox / VAAPI)
# ----------------------------------------------------------------------------
if(PS_ENABLE_FFMPEG)
    find_package(FFmpeg QUIET COMPONENTS avcodec avformat avutil swscale)
    if(FFmpeg_FOUND)
        message(STATUS "FFmpeg found: hardware decoding enabled")
        add_compile_definitions(PS_HAS_FFMPEG=1)
    else()
        message(WARNING "FFmpeg not found — using Qt Multimedia backend only")
        set(PS_ENABLE_FFMPEG OFF)
    endif()
endif()

if(PS_ENABLE_NDI)
    add_compile_definitions(PS_ENABLE_NDI=1)
    # Set NDI_SDK_DIR in your environment / CMake cache.
    find_path(NDI_INCLUDE_DIR Processing.NDI.Lib.h HINTS "$ENV{NDI_SDK_DIR}/Include")
    if(NDI_INCLUDE_DIR)
        include_directories(${NDI_INCLUDE_DIR})
        find_library(NDI_LIBRARY NDI HINTS "$ENV{NDI_SDK_DIR}/Lib/x64")
    else()
        message(WARNING "NDI SDK not found — disabling NDI output")
        set(PS_ENABLE_NDI OFF)
    endif()
endif()

# ----------------------------------------------------------------------------
# Embedded resources are generated per-target (app and tests) via
# qt_add_resources in their own CMakeLists.txt.
# ----------------------------------------------------------------------------

# ----------------------------------------------------------------------------
# Project libraries
# ----------------------------------------------------------------------------
add_subdirectory(src/core)
add_subdirectory(src/persistence)
add_subdirectory(src/crdt)
add_subdirectory(src/remote)
add_subdirectory(src/media)
add_subdirectory(src/output)
add_subdirectory(src/timeline)
add_subdirectory(src/scripture)
add_subdirectory(src/timers)
add_subdirectory(src/templates)
add_subdirectory(src/overlays)
add_subdirectory(src/audio)
add_subdirectory(src/triggers)
add_subdirectory(src/recording)
add_subdirectory(src/chords)
add_subdirectory(src/backup)
add_subdirectory(src/providers)
add_subdirectory(src/cloud)
add_subdirectory(src/presentation)
add_subdirectory(src/app)

if(PS_BUILD_TESTS)
    enable_testing()
    find_package(Qt6 REQUIRED COMPONENTS Test)
    add_subdirectory(tests)
endif()

# Windows deployment is configured in src/app/CMakeLists.txt, where the
# proslides_app target is created (CMake requires TARGET commands in the
# same directory as the target).
