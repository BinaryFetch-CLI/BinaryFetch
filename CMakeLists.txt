cmake_minimum_required(VERSION 3.20)

# ============================================================
# BinaryFetch
# Cross-platform CMake configuration
# ============================================================

project(BinaryFetch
    VERSION 1.0.0
    DESCRIPTION "A system information and hardware monitoring tool"
    LANGUAGES CXX
)

# ============================================================
# C++ Standard
# ============================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================
# Project Directories
# ============================================================

set(BINARYFETCH_ROOT
    ${CMAKE_CURRENT_SOURCE_DIR}/src/BinaryFetch
)

set(BINARYFETCH_CPP
    ${BINARYFETCH_ROOT}/cpp
)

set(BINARYFETCH_INCLUDE
    ${BINARYFETCH_ROOT}/include
)

set(BINARYFETCH_DEPENDENCIES
    ${BINARYFETCH_ROOT}/dependencies
)

# ------------------------------------------------------------
# NVAPI Root Directory
#
# Contains:
#
#   nvapi.h
#   nvapi_interface.h
#   nvapi_lite_*.h
#   amd64/nvapi64.lib
#
# ------------------------------------------------------------

set(BINARYFETCH_NVAPI
    ${BINARYFETCH_DEPENDENCIES}/NVAPI/nvapi-main
)

set(BINARYFETCH_RESOURCES
    ${BINARYFETCH_ROOT}/resources
)

# ============================================================
# Platform Detection
# ============================================================

if(WIN32)

    set(BINARYFETCH_PLATFORM "Windows")

elseif(UNIX AND NOT APPLE)

    set(BINARYFETCH_PLATFORM "Linux")

elseif(APPLE)

    set(BINARYFETCH_PLATFORM "macOS")

else()

    set(BINARYFETCH_PLATFORM "Unknown")

endif()

# ============================================================
# Common Header Files
#
# These headers are shared by the platform implementations.
#
# Example:
#
#   Windows/CPUInfo.cpp ─────┐
#                            ├──> include/detailed/CPUInfo.h
#   Linux/CPUInfo.cpp ───────┘
#
# ============================================================

set(BINARYFETCH_HEADERS

    # --------------------------------------------------------
    # Core
    # --------------------------------------------------------

    ${BINARYFETCH_INCLUDE}/core/AsciiArt.h
    ${BINARYFETCH_INCLUDE}/core/json.hpp
    ${BINARYFETCH_INCLUDE}/core/resource.h

    # --------------------------------------------------------
    # Compact
    # --------------------------------------------------------

    ${BINARYFETCH_INCLUDE}/compact/CompactAudio.h
    ${BINARYFETCH_INCLUDE}/compact/CompactCPU.h
    ${BINARYFETCH_INCLUDE}/compact/CompactGPU.h
    ${BINARYFETCH_INCLUDE}/compact/CompactMemory.h
    ${BINARYFETCH_INCLUDE}/compact/CompactNetwork.h
    ${BINARYFETCH_INCLUDE}/compact/CompactOS.h
    ${BINARYFETCH_INCLUDE}/compact/CompactPerformance.h
    ${BINARYFETCH_INCLUDE}/compact/CompactScreen.h
    ${BINARYFETCH_INCLUDE}/compact/CompactSystem.h
    ${BINARYFETCH_INCLUDE}/compact/CompactUser.h
    ${BINARYFETCH_INCLUDE}/compact/compact_disk_info.h
    ${BINARYFETCH_INCLUDE}/compact/TimeInfo.h

    # --------------------------------------------------------
    # Detailed
    # --------------------------------------------------------

    ${BINARYFETCH_INCLUDE}/detailed/CPUInfo.h
    ${BINARYFETCH_INCLUDE}/detailed/DetailedGPUInfo.h
    ${BINARYFETCH_INCLUDE}/detailed/DisplayInfo.h
    ${BINARYFETCH_INCLUDE}/detailed/ExtraInfo.h
    ${BINARYFETCH_INCLUDE}/detailed/GPUInfo.h
    ${BINARYFETCH_INCLUDE}/detailed/MemoryInfo.h
    ${BINARYFETCH_INCLUDE}/detailed/NetworkInfo.h
    ${BINARYFETCH_INCLUDE}/detailed/OSInfo.h
    ${BINARYFETCH_INCLUDE}/detailed/PerformanceInfo.h
    ${BINARYFETCH_INCLUDE}/detailed/StorageInfo.h
    ${BINARYFETCH_INCLUDE}/detailed/SystemInfo.h
    ${BINARYFETCH_INCLUDE}/detailed/UserInfo.h
)

# ============================================================
# Platform Sources
# ============================================================

if(WIN32)

    # ========================================================
    # WINDOWS BUILD
    # ========================================================

    message(STATUS "Configuring BinaryFetch for Windows")

    set(BINARYFETCH_SOURCES

        # ----------------------------------------------------
        # Main
        # ----------------------------------------------------

        ${BINARYFETCH_CPP}/main.cpp

        # ----------------------------------------------------
        # Core
        # ----------------------------------------------------

        ${BINARYFETCH_CPP}/windows/core/AsciiArt.cpp

        # ----------------------------------------------------
        # Detailed Information
        # ----------------------------------------------------

        ${BINARYFETCH_CPP}/windows/detailed/CPUInfo.cpp
        ${BINARYFETCH_CPP}/windows/detailed/DisplayInfo.cpp
        ${BINARYFETCH_CPP}/windows/detailed/DtailedGPUInfo.cpp
        ${BINARYFETCH_CPP}/windows/detailed/ExtraInfo.cpp
        ${BINARYFETCH_CPP}/windows/detailed/GPUInfo.cpp
        ${BINARYFETCH_CPP}/windows/detailed/Helpers.cpp
        ${BINARYFETCH_CPP}/windows/detailed/MemoryInfo.cpp
        ${BINARYFETCH_CPP}/windows/detailed/NetworkInfo.cpp
        ${BINARYFETCH_CPP}/windows/detailed/OSInfo.cpp
        ${BINARYFETCH_CPP}/windows/detailed/PerformanceInfo.cpp
        ${BINARYFETCH_CPP}/windows/detailed/personalization_info.cpp
        ${BINARYFETCH_CPP}/windows/detailed/StorageInfo.cpp
        ${BINARYFETCH_CPP}/windows/detailed/SystemInfo.cpp
        ${BINARYFETCH_CPP}/windows/detailed/UserInfo.cpp

        # ----------------------------------------------------
        # Compact Information
        # ----------------------------------------------------

        ${BINARYFETCH_CPP}/windows/compact/CompactAudio.cpp
        ${BINARYFETCH_CPP}/windows/compact/CompactCPU.cpp
        ${BINARYFETCH_CPP}/windows/compact/CompactGPU.cpp
        ${BINARYFETCH_CPP}/windows/compact/CompactMemory.cpp
        ${BINARYFETCH_CPP}/windows/compact/CompactNetwork.cpp
        ${BINARYFETCH_CPP}/windows/compact/CompactOS.cpp
        ${BINARYFETCH_CPP}/windows/compact/CompactPerformance.cpp
        ${BINARYFETCH_CPP}/windows/compact/CompactScreen.cpp
        ${BINARYFETCH_CPP}/windows/compact/CompactSystem.cpp
        ${BINARYFETCH_CPP}/windows/compact/CompactUser.cpp
        ${BINARYFETCH_CPP}/windows/compact/compact_disk_info.cpp
        ${BINARYFETCH_CPP}/windows/compact/TimeInfo.cpp
    )

    # --------------------------------------------------------
    # Windows Resource Files
    # --------------------------------------------------------

    set(BINARYFETCH_RESOURCE_FILES

        ${BINARYFETCH_RESOURCES}/binary_fetch_v1.rc
        ${BINARYFETCH_RESOURCES}/resources.rc
        ${BINARYFETCH_RESOURCES}/version_info.rc
    )

    enable_language(RC)

    list(APPEND BINARYFETCH_SOURCES
        ${BINARYFETCH_RESOURCE_FILES}
    )

elseif(UNIX AND NOT APPLE)

    # ========================================================
    # LINUX DEVELOPMENT BUILD
    # ========================================================

    message(STATUS "Configuring BinaryFetch for Linux")

    message(STATUS
        "Linux development entry point: Linux_test_main.cpp"
    )

    set(BINARYFETCH_SOURCES

        # ----------------------------------------------------
        # Temporary Linux Test Entry Point
        #
        # This is intentionally NOT main.cpp.
        #
        # It allows Linux implementations to be tested
        # independently before merging the final Linux logic
        # into the production main.cpp.
        # ----------------------------------------------------

        ${BINARYFETCH_CPP}/Linux_test_main.cpp

        # ----------------------------------------------------
        # Core
        # ----------------------------------------------------

        ${BINARYFETCH_CPP}/Linux/core/AsciiArt.cpp

        # ----------------------------------------------------
        # Detailed Information
        # ----------------------------------------------------

        ${BINARYFETCH_CPP}/Linux/detailed/CPUInfo.cpp
        ${BINARYFETCH_CPP}/Linux/detailed/DisplayInfo.cpp
        ${BINARYFETCH_CPP}/Linux/detailed/DtailedGPUInfo.cpp
        ${BINARYFETCH_CPP}/Linux/detailed/ExtraInfo.cpp
        ${BINARYFETCH_CPP}/Linux/detailed/GPUInfo.cpp
        ${BINARYFETCH_CPP}/Linux/detailed/Helpers.cpp
        ${BINARYFETCH_CPP}/Linux/detailed/MemoryInfo.cpp
        ${BINARYFETCH_CPP}/Linux/detailed/NetworkInfo.cpp
        ${BINARYFETCH_CPP}/Linux/detailed/OSInfo.cpp
        ${BINARYFETCH_CPP}/Linux/detailed/PerformanceInfo.cpp
        ${BINARYFETCH_CPP}/Linux/detailed/StorageInfo.cpp
        ${BINARYFETCH_CPP}/Linux/detailed/SystemInfo.cpp
        ${BINARYFETCH_CPP}/Linux/detailed/UserInfo.cpp
        ${BINARYFETCH_CPP}/Linux/detailed/personalization_info.cpp

        # ----------------------------------------------------
        # Compact Information
        # ----------------------------------------------------

        ${BINARYFETCH_CPP}/Linux/compact/CompactAudio.cpp
        ${BINARYFETCH_CPP}/Linux/compact/CompactCPU.cpp
        ${BINARYFETCH_CPP}/Linux/compact/CompactGPU.cpp
        ${BINARYFETCH_CPP}/Linux/compact/CompactMemory.cpp
        ${BINARYFETCH_CPP}/Linux/compact/CompactNetwork.cpp
        ${BINARYFETCH_CPP}/Linux/compact/CompactOS.cpp
        ${BINARYFETCH_CPP}/Linux/compact/CompactPerformance.cpp
        ${BINARYFETCH_CPP}/Linux/compact/CompactScreen.cpp
        ${BINARYFETCH_CPP}/Linux/compact/CompactSystem.cpp
        ${BINARYFETCH_CPP}/Linux/compact/CompactUser.cpp
        ${BINARYFETCH_CPP}/Linux/compact/compact_disk_info.cpp
        ${BINARYFETCH_CPP}/Linux/compact/TimeInfo.cpp
    )

endif()

# ============================================================
# Validate Platform Entry Point
# ============================================================

if(WIN32)

    if(NOT EXISTS "${BINARYFETCH_CPP}/main.cpp")

        message(FATAL_ERROR
            "Windows main.cpp was not found:\n"
            "  ${BINARYFETCH_CPP}/main.cpp"
        )

    endif()

elseif(UNIX AND NOT APPLE)

    if(NOT EXISTS "${BINARYFETCH_CPP}/Linux_test_main.cpp")

        message(FATAL_ERROR
            "Linux_test_main.cpp was not found:\n"
            "  ${BINARYFETCH_CPP}/Linux_test_main.cpp\n\n"
            "This file is currently the temporary Linux development entry point."
        )

    endif()

endif()

# ============================================================
# Executable
# ============================================================

add_executable(BinaryFetch
    ${BINARYFETCH_SOURCES}
    ${BINARYFETCH_HEADERS}
)

# ============================================================
# Common Include Directories
# ============================================================

target_include_directories(BinaryFetch

    PRIVATE

        # ----------------------------------------------------
        # Project headers
        # ----------------------------------------------------

        ${BINARYFETCH_INCLUDE}

        ${BINARYFETCH_INCLUDE}/core
        ${BINARYFETCH_INCLUDE}/compact
        ${BINARYFETCH_INCLUDE}/detailed

        # ----------------------------------------------------
        # General dependencies
        # ----------------------------------------------------

        ${BINARYFETCH_DEPENDENCIES}
)

# ============================================================
# Windows Configuration
# ============================================================

if(WIN32)

    # --------------------------------------------------------
    # NVAPI Header Include Directory
    #
    # nvapi.h is located directly inside:
    #
    # dependencies/NVAPI/nvapi-main/
    #
    # This directory MUST be added to the compiler's
    # include search path so:
    #
    #     #include <nvapi.h>
    #
    # can be resolved.
    # --------------------------------------------------------

    if(EXISTS "${BINARYFETCH_NVAPI}/nvapi.h")

        message(STATUS
            "NVAPI headers found:"
        )

        message(STATUS
            "  ${BINARYFETCH_NVAPI}"
        )

        target_include_directories(BinaryFetch

            PRIVATE

                ${BINARYFETCH_NVAPI}
        )

    else()

        message(FATAL_ERROR
            "NVAPI header was not found:\n"
            "  ${BINARYFETCH_NVAPI}/nvapi.h\n\n"
            "Expected NVAPI structure:\n"
            "  dependencies/NVAPI/nvapi-main/nvapi.h"
        )

    endif()

    # --------------------------------------------------------
    # Windows Definitions
    # --------------------------------------------------------

    target_compile_definitions(BinaryFetch

        PRIVATE

            NOMINMAX
            WIN32_LEAN_AND_MEAN
            _CRT_SECURE_NO_WARNINGS
            UNICODE
            _UNICODE

            # ------------------------------------------------
            # Fix MSVC / Windows SDK std::byte ambiguity
            # ------------------------------------------------

            _HAS_STD_BYTE=0

            # ------------------------------------------------
            # Tell the source code that NVAPI is available
            # ------------------------------------------------

            BINARYFETCH_HAS_NVAPI
    )

    # --------------------------------------------------------
    # Windows Libraries
    # --------------------------------------------------------

    target_link_libraries(BinaryFetch

        PRIVATE

            user32
            shell32
            advapi32
            psapi
            iphlpapi
            wbemuuid
            ole32
            oleaut32
            version
    )

    # ========================================================
    # NVAPI
    # ========================================================

    set(NVAPI_LIBRARY
        ${BINARYFETCH_NVAPI}/amd64/nvapi64.lib
    )

    # --------------------------------------------------------
    # Verify NVAPI library
    # --------------------------------------------------------

    if(EXISTS "${NVAPI_LIBRARY}")

        message(STATUS
            "NVAPI library found:"
        )

        message(STATUS
            "  ${NVAPI_LIBRARY}"
        )

        # ----------------------------------------------------
        # Link NVAPI
        # ----------------------------------------------------

        target_link_libraries(BinaryFetch

            PRIVATE

                "${NVAPI_LIBRARY}"
        )

    else()

        message(FATAL_ERROR
            "NVAPI library was not found:\n"
            "  ${NVAPI_LIBRARY}\n\n"
            "Expected NVAPI structure:\n"
            "  dependencies/NVAPI/nvapi-main/amd64/nvapi64.lib"
        )

    endif()

endif()

# ============================================================
# Linux Configuration
# ============================================================

if(UNIX AND NOT APPLE)

    message(STATUS
        "Linux-specific dependencies enabled"
    )

    # --------------------------------------------------------
    # Linux libraries
    #
    # Add platform libraries here only when the Linux
    # implementations actually require them.
    # --------------------------------------------------------

    find_package(Threads REQUIRED)

    target_link_libraries(BinaryFetch

        PRIVATE

            Threads::Threads
    )

endif()

# ============================================================
# Compiler Warnings
# ============================================================

if(MSVC)

    target_compile_options(BinaryFetch

        PRIVATE

            /W4
            /permissive-
    )

else()

    target_compile_options(BinaryFetch

        PRIVATE

            -Wall
            -Wextra
            -Wpedantic
    )

endif()

# ============================================================
# Output Directories
# ============================================================

set_target_properties(BinaryFetch

    PROPERTIES

    RUNTIME_OUTPUT_DIRECTORY
        ${CMAKE_BINARY_DIR}/bin

    RUNTIME_OUTPUT_DIRECTORY_DEBUG
        ${CMAKE_BINARY_DIR}/bin/Debug

    RUNTIME_OUTPUT_DIRECTORY_RELEASE
        ${CMAKE_BINARY_DIR}/bin/Release
)

# ============================================================
# Visual Studio Configuration
# ============================================================

if(MSVC)

    set_property(

        TARGET BinaryFetch

        PROPERTY

        VS_DEBUGGER_WORKING_DIRECTORY
        "${CMAKE_CURRENT_SOURCE_DIR}"
    )

endif()

# ============================================================
# Project Information
# ============================================================

message(STATUS "")

message(STATUS "======================================")
message(STATUS " BinaryFetch")
message(STATUS "======================================")

message(STATUS
    " Source Root:   ${BINARYFETCH_ROOT}"
)

message(STATUS
    " C++ Directory: ${BINARYFETCH_CPP}"
)

message(STATUS
    " Include Dir:   ${BINARYFETCH_INCLUDE}"
)

message(STATUS
    " Dependencies:  ${BINARYFETCH_DEPENDENCIES}"
)

message(STATUS
    " Platform:      ${BINARYFETCH_PLATFORM}"
)

message(STATUS
    " C++ Standard:  C++17"
)

message(STATUS
    " Compiler:      ${CMAKE_CXX_COMPILER_ID}"
)

message(STATUS
    " Version:       ${PROJECT_VERSION}"
)

if(WIN32)

    message(STATUS
        " Entry Point:   main.cpp"
    )

    message(STATUS
        " NVAPI:         Enabled"
    )

    message(STATUS
        " NVAPI Headers: ${BINARYFETCH_NVAPI}"
    )

    message(STATUS
        " NVAPI Library: ${NVAPI_LIBRARY}"
    )

    message(STATUS
        " Resources:     Windows RC enabled"
    )

elseif(UNIX AND NOT APPLE)

    message(STATUS
        " Entry Point:   Linux_test_main.cpp"
    )

    message(STATUS
        " Mode:           Linux development/testing"
    )

endif()

message(STATUS "======================================")
message(STATUS "")