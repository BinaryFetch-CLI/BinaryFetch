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
# Source Files
# ============================================================

set(BINARYFETCH_SOURCES

    # --------------------------------------------------------
    # Main
    # --------------------------------------------------------

    ${BINARYFETCH_CPP}/main.cpp

    # --------------------------------------------------------
    # Core
    # --------------------------------------------------------

    ${BINARYFETCH_CPP}/windows/core/AsciiArt.cpp

    # --------------------------------------------------------
    # Detailed Information
    # --------------------------------------------------------

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
    ${BINARYFETCH_CPP}/windows/detailed/TimeInfo.cpp
    ${BINARYFETCH_CPP}/windows/detailed/UserInfo.cpp

    # --------------------------------------------------------
    # Compact Information
    # --------------------------------------------------------

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
)

# ============================================================
# Header Files
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
    ${BINARYFETCH_INCLUDE}/detailed/TimeInfo.h
    ${BINARYFETCH_INCLUDE}/detailed/UserInfo.h
)

# ============================================================
# Validate Main Source
# ============================================================

if(NOT EXISTS "${BINARYFETCH_CPP}/main.cpp")

    message(FATAL_ERROR
        "BinaryFetch source file not found:\n"
        "  ${BINARYFETCH_CPP}/main.cpp\n\n"
        "Expected structure:\n"
        "  ${BINARYFETCH_ROOT}/cpp/main.cpp"
    )

endif()

# ============================================================
# Windows-Specific Sources
# ============================================================

if(WIN32)

    message(STATUS "Configuring Windows build")

    # --------------------------------------------------------
    # Windows Resource Files
    # --------------------------------------------------------

    set(BINARYFETCH_RESOURCE_FILES

        ${BINARYFETCH_RESOURCES}/binary_fetch_v1.rc
        ${BINARYFETCH_RESOURCES}/resources.rc
        ${BINARYFETCH_RESOURCES}/version_info.rc
    )

    # --------------------------------------------------------
    # Windows Resource Compiler
    # --------------------------------------------------------

    enable_language(RC)

    list(APPEND BINARYFETCH_SOURCES
        ${BINARYFETCH_RESOURCE_FILES}
    )

endif()

# ============================================================
# Linux / Unix Sources
# ============================================================

if(UNIX AND NOT APPLE)

    message(STATUS "Configuring Linux build")

    # Windows-specific implementation files are currently
    # excluded from Linux compilation.
    #
    # Future Linux implementations can be placed under:
    #
    # cpp/linux/
    #
    # and added here.

endif()

# ============================================================
# Executable
# ============================================================

add_executable(BinaryFetch
    ${BINARYFETCH_SOURCES}
    ${BINARYFETCH_HEADERS}
)

# ============================================================
# Include Directories
# ============================================================

target_include_directories(BinaryFetch
    PRIVATE

        ${BINARYFETCH_INCLUDE}

        ${BINARYFETCH_INCLUDE}/core
        ${BINARYFETCH_INCLUDE}/compact
        ${BINARYFETCH_INCLUDE}/detailed

        ${BINARYFETCH_DEPENDENCIES}

        ${BINARYFETCH_NVAPI}
)

# ============================================================
# Windows Configuration
# ============================================================

if(WIN32)

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

        # ----------------------------------------------------
        # Fix MSVC / Windows SDK std::byte ambiguity
        # ----------------------------------------------------
        _HAS_STD_BYTE=0
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

    if(EXISTS "${NVAPI_LIBRARY}")

        message(STATUS
            "NVAPI found: ${NVAPI_LIBRARY}"
        )

        target_link_libraries(BinaryFetch
            PRIVATE
            "${NVAPI_LIBRARY}"
        )

        target_compile_definitions(BinaryFetch
            PRIVATE
            BINARYFETCH_HAS_NVAPI
        )

    else()

        message(WARNING
            "NVAPI library was not found:\n"
            "  ${NVAPI_LIBRARY}\n"
            "NVIDIA GPU functionality may fail to link."
        )

    endif()

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
message(STATUS " Source Root:   ${BINARYFETCH_ROOT}")
message(STATUS " C++ Directory: ${BINARYFETCH_CPP}")
message(STATUS " Include Dir:   ${BINARYFETCH_INCLUDE}")
message(STATUS " Dependencies:  ${BINARYFETCH_DEPENDENCIES}")
message(STATUS " NVAPI:         ${BINARYFETCH_NVAPI}")
message(STATUS " Platform:      ${BINARYFETCH_PLATFORM}")
message(STATUS " C++ Standard:  C++17")
message(STATUS " Compiler:      ${CMAKE_CXX_COMPILER_ID}")
message(STATUS " Version:       ${PROJECT_VERSION}")
message(STATUS "======================================")
message(STATUS "")