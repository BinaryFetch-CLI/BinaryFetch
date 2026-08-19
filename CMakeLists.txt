cmake_minimum_required(VERSION 3.20)

project(BinaryFetch VERSION 1.3 LANGUAGES CXX RC)

# ============================================================
# C++ Configuration
# ============================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================
# BinaryFetch source directory
# ============================================================

set(BINARYFETCH_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/project_binary_fetch/binary_fetch_v1"
)

set(INCLUDE_DIR
    "${BINARYFETCH_DIR}/include"
)

# ============================================================
# Source files
# ============================================================

set(SOURCES

    "${BINARYFETCH_DIR}/main.cpp"

    "${BINARYFETCH_DIR}/AsciiArt.cpp"
    "${BINARYFETCH_DIR}/CompactAudio.cpp"
    "${BINARYFETCH_DIR}/CompactCPU.cpp"
    "${BINARYFETCH_DIR}/CompactGPU.cpp"
    "${BINARYFETCH_DIR}/CompactMemory.cpp"
    "${BINARYFETCH_DIR}/CompactNetwork.cpp"
    "${BINARYFETCH_DIR}/CompactOS.cpp"
    "${BINARYFETCH_DIR}/CompactPerformance.cpp"
    "${BINARYFETCH_DIR}/CompactScreen.cpp"
    "${BINARYFETCH_DIR}/CompactSystem.cpp"
    "${BINARYFETCH_DIR}/CompactUser.cpp"

    "${BINARYFETCH_DIR}/compact_disk_info.cpp"

    "${BINARYFETCH_DIR}/CPUInfo.cpp"
    "${BINARYFETCH_DIR}/DisplayInfo.cpp"
    "${BINARYFETCH_DIR}/DtailedGPUInfo.cpp"
    "${BINARYFETCH_DIR}/ExtraInfo.cpp"
    "${BINARYFETCH_DIR}/GPUInfo.cpp"
    "${BINARYFETCH_DIR}/Helpers.cpp"
    "${BINARYFETCH_DIR}/MemoryInfo.cpp"
    "${BINARYFETCH_DIR}/NetworkInfo.cpp"
    "${BINARYFETCH_DIR}/OSInfo.cpp"
    "${BINARYFETCH_DIR}/PerformanceInfo.cpp"
    "${BINARYFETCH_DIR}/personalization_info.cpp"
    "${BINARYFETCH_DIR}/StorageInfo.cpp"
    "${BINARYFETCH_DIR}/SystemInfo.cpp"
    "${BINARYFETCH_DIR}/TimeInfo.cpp"
    "${BINARYFETCH_DIR}/UserInfo.cpp"
)

# ============================================================
# Windows Resources
# ============================================================

if(WIN32)

    list(APPEND SOURCES

        "${BINARYFETCH_DIR}/resources.rc"
        "${BINARYFETCH_DIR}/version_info.rc"

    )

endif()

# ============================================================
# BinaryFetch executable
# ============================================================

add_executable(BinaryFetch ${SOURCES})

# ============================================================
# Include directories
# ============================================================

target_include_directories(BinaryFetch PRIVATE

    "${INCLUDE_DIR}"

    "${BINARYFETCH_DIR}"

    "${BINARYFETCH_DIR}/nlohmann"

)

# ============================================================
# Windows configuration
# ============================================================

if(WIN32)

    target_compile_definitions(BinaryFetch PRIVATE

        UNICODE
        _UNICODE
        _CONSOLE

    )

    target_link_libraries(BinaryFetch PRIVATE

        advapi32
        user32
        ole32
        Gdi32
        Shell32

    )

endif()

# ============================================================
# NVIDIA NVAPI
# ============================================================
#
# NVAPI is Windows-only.
#
# Repository location:
#
# project_binary_fetch/
# └── binary_fetch_v1/
#     └── dependencies/
#         └── NVAPI/
#             └── nvapi-main/
#
# ============================================================

if(WIN32)

    set(NVAPI_ROOT
        "${BINARYFETCH_DIR}/dependencies/NVAPI/nvapi-main"
    )

    if(EXISTS "${NVAPI_ROOT}")

        message(STATUS "NVAPI found: ${NVAPI_ROOT}")

        target_include_directories(BinaryFetch PRIVATE

            "${NVAPI_ROOT}"

        )

        target_link_directories(BinaryFetch PRIVATE

            "${NVAPI_ROOT}/amd64"

        )

        target_link_libraries(BinaryFetch PRIVATE

            nvapi64

        )

    else()

        message(WARNING
            "NVAPI was not found at: ${NVAPI_ROOT}. "
            "NVIDIA GPU functionality may fail to link."
        )

    endif()

endif()

# ============================================================
# MSVC configuration
# ============================================================

if(MSVC)

    # --------------------------------------------------------
    # Warning level
    # --------------------------------------------------------

    target_compile_options(BinaryFetch PRIVATE

        /W3
        /permissive-

    )

    # --------------------------------------------------------
    # Fix Windows SDK / std::byte ambiguity
    # --------------------------------------------------------

    target_compile_definitions(BinaryFetch PRIVATE

        _HAS_STD_BYTE=0

    )

    # --------------------------------------------------------
    # Static MSVC runtime
    #
    # Debug   -> /MTd
    # Release -> /MT
    #
    # This statically links the MSVC runtime into BinaryFetch.
    # --------------------------------------------------------

    set_property(TARGET BinaryFetch PROPERTY

        MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>"

    )

    # --------------------------------------------------------
    # Release optimization
    # --------------------------------------------------------

    target_compile_options(BinaryFetch PRIVATE

        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:Release>:/GL>

    )

    target_link_options(BinaryFetch PRIVATE

        $<$<CONFIG:Release>:/LTCG>

    )

endif()

# ============================================================
# Output directories
# ============================================================

set_target_properties(BinaryFetch PROPERTIES

    RUNTIME_OUTPUT_DIRECTORY
        "${CMAKE_BINARY_DIR}/bin"

)

# ============================================================
# Build information
# ============================================================

message(STATUS "======================================")
message(STATUS " BinaryFetch")
message(STATUS "======================================")
message(STATUS " Source: ${BINARYFETCH_DIR}")
message(STATUS " Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS " C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS " System: ${CMAKE_SYSTEM_NAME}")
message(STATUS " MSVC: ${MSVC}")
message(STATUS " Windows: ${WIN32}")

if(WIN32)

    message(STATUS " NVAPI: ${NVAPI_ROOT}")

endif()

message(STATUS "======================================")