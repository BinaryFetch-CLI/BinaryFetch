# CMakeLists.txt — BinaryFetch (VS Code + Clang friendly)
cmake_minimum_required(VERSION 3.16)
project(BinaryFetch LANGUAGES CXX)

################################################################################
# Project root (adjust if you placed CMakeLists elsewhere)
################################################################################
set(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "Project root: ${PROJECT_ROOT}")

################################################################################
# Options
################################################################################
option(BUILD_SHARED_LIBS "Build as shared library when supported" OFF)
option(ENABLE_NVAPI "Try to link NVAPI if available" OFF)
option(ENABLE_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

################################################################################
# Basic language / build settings
################################################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default output folders
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

################################################################################
# Collect sources
# - We use GLOB_RECURSE but exclude .history, .git and CMake dir to avoid noise.
################################################################################
file(GLOB_RECURSE ALL_CPP
    RELATIVE "${PROJECT_ROOT}"
    "${PROJECT_ROOT}/src/*.cpp"
    "${PROJECT_ROOT}/src/**/*.cpp"
)

file(GLOB_RECURSE ALL_H
    RELATIVE "${PROJECT_ROOT}"
    "${PROJECT_ROOT}/include/*.h"
    "${PROJECT_ROOT}/include/**/*.h"
)

# Filter out unwanted directories (simple filter)
set(SRC_FILES "")
foreach(f IN LISTS ALL_CPP)
    if(NOT f MATCHES "^\\.history" AND NOT f MATCHES "/\\.history/" AND NOT f MATCHES "^\\.git" AND NOT f MATCHES "/\\.git/")
        list(APPEND SRC_FILES "${PROJECT_ROOT}/${f}")
    endif()
endforeach()

set(HEADER_FILES "")
foreach(f IN LISTS ALL_H)
    if(NOT f MATCHES "^\\.history" AND NOT f MATCHES "/\\.history/" AND NOT f MATCHES "^\\.git" AND NOT f MATCHES "/\\.git/")
        list(APPEND HEADER_FILES "${PROJECT_ROOT}/${f}")
    endif()
endforeach()

# If present, resource files (Windows .rc)
set(RC_FILES "")
if(EXISTS "${PROJECT_ROOT}/resources.rc")
    list(APPEND RC_FILES "${PROJECT_ROOT}/resources.rc")
endif()
if(EXISTS "${PROJECT_ROOT}/version_info.rc")
    list(APPEND RC_FILES "${PROJECT_ROOT}/version_info.rc")
endif()

################################################################################
# Handle possible filename typo: DtailedGPUInfo.cpp vs DetailedGPUInfo.cpp
# (project had inconsistent names)
################################################################################
if(EXISTS "${PROJECT_ROOT}/src/DtailedGPUInfo.cpp")
    message(STATUS "Using src/DtailedGPUInfo.cpp (typoed file present)")
    list(APPEND SRC_FILES "${PROJECT_ROOT}/src/DtailedGPUInfo.cpp")
elseif(EXISTS "${PROJECT_ROOT}/src/DetailedGPUInfo.cpp")
    message(STATUS "Using src/DetailedGPUInfo.cpp")
    list(APPEND SRC_FILES "${PROJECT_ROOT}/src/DetailedGPUInfo.cpp")
endif()

################################################################################
# Non-source files (configs, docs) to show in IDE
################################################################################
set(OTHER_FILES
    "${PROJECT_ROOT}/CMake/Default_BinaryFetch_Config.json"
    "${PROJECT_ROOT}/AsciiArt_Documentation.md"
    "${PROJECT_ROOT}/DefaultAsciiArt.txt"
    "${PROJECT_ROOT}/Art_Collections.txt"
)

# Remove duplicates from SRC_FILES (safe)
list(REMOVE_DUPLICATES SRC_FILES)

################################################################################
# Create target
################################################################################
add_executable(${PROJECT_NAME}
    ${SRC_FILES}
    ${HEADER_FILES}
    ${RC_FILES}
    ${OTHER_FILES}
)

################################################################################
# Include directories (project-local)
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC
    "${PROJECT_ROOT}/include"
    "${PROJECT_ROOT}/nlohmann"        # your json.hpp lives here in tree
    "${PROJECT_ROOT}"                # sometimes headers are at project root
)

################################################################################
# Compiler flags for different compilers (MSVC vs Clang/GNU)
################################################################################
if(MSVC)
    # Keep MSVC flags conservative — this is a ported project originally for VS.
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W3
        $<$<CONFIG:Release>:/O2>
    )
    # Keep runtime choice default. If you want to force DLL runtime, change MSVC_RUNTIME_LIBRARY.
else()
    # For clang/gcc
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        $<$<BOOL:${ENABLE_WARNINGS_AS_ERRORS}>:-Werror>
    )

    # If using clang specifically, you may add extra clang-specific flags:
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(${PROJECT_NAME} PRIVATE
            -fno-exceptions   # only if project supports it — remove if not desired
        )
    endif()
endif()

################################################################################
# Platform-specific link libraries
################################################################################
if(WIN32)
    # Standard Win32 system libs
    target_link_libraries(${PROJECT_NAME} PRIVATE
        advapi32
        user32
        ole32
        gdi32
        shell32
    )
endif()

################################################################################
# NVAPI (optional)
# - If ENABLE_NVAPI is ON, try to locate nvapi64.lib (amd64) or nvapi.lib (x86)
# - You can pass -DENABLE_NVAPI=ON -DNVAPI_ROOT="C:/NVAPI/nvapi-main/amd64" when configuring
################################################################################
if(ENABLE_NVAPI)
    if(NOT DEFINED NVAPI_ROOT)
        set(NVAPI_SEARCH_PATHS
            "C:/NVAPI"
            "C:/NVAPI/nvapi-main/amd64"
            "${PROJECT_ROOT}/../nvapi"
            "${PROJECT_ROOT}/nvapi"
        )
    else()
        set(NVAPI_SEARCH_PATHS "${NVAPI_ROOT}")
    endif()

    find_library(NVAPI_LIB
        NAMES nvapi64 nvapi64.lib nvapi
        PATHS ${NVAPI_SEARCH_PATHS}
        NO_DEFAULT_PATH
    )

    if(NVAPI_LIB)
        message(STATUS "Found NVAPI library: ${NVAPI_LIB}")
        target_link_libraries(${PROJECT_NAME} PRIVATE "${NVAPI_LIB}")
    else()
        message(WARNING "ENABLE_NVAPI=ON but NVAPI library not found in: ${NVAPI_SEARCH_PATHS}")
    endif()
endif()

################################################################################
# Resource compile settings (if present)
################################################################################
if(RC_FILES)
    message(STATUS "Resource files included: ${RC_FILES}")
endif()

################################################################################
# Definitions that were in original CMake (kept minimal & portable)
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# If the project expects UNICODE macros for windows code:
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)
endif()

################################################################################
# Nice grouping for IDEs (Visual Studio / VS Code CMake Tools)
################################################################################
source_group("src" FILES ${SRC_FILES})
source_group("include" FILES ${HEADER_FILES})
source_group("resources" FILES ${RC_FILES})
source_group("config" FILES ${OTHER_FILES})

################################################################################
# Helpful messages / build hints
################################################################################
message(STATUS "Number of source files: ${SRC_FILES}")
message(STATUS "Number of header files: ${HEADER_FILES}")
if(ENABLE_NVAPI)
    message(STATUS "NVAPI support requested (ENABLE_NVAPI=ON).")
endif()

################################################################################
# Testing & packaging can be added later if needed
################################################################################
