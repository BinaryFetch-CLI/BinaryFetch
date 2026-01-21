cmake_minimum_required(VERSION 3.16)
project(BinaryFetch VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Platform detection
if(WIN32)
    set(PLATFORM_WINDOWS TRUE)
    add_definitions(-DPLATFORM_WINDOWS=1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(PLATFORM_LINUX TRUE)
    add_definitions(-DPLATFORM_LINUX=1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    set(PLATFORM_FREEBSD TRUE)
    add_definitions(-DPLATFORM_FREEBSD=1)
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

# Static linking options
option(BUILD_STATIC "Build statically linked binary" ON)

if(BUILD_STATIC)
    if(PLATFORM_LINUX OR PLATFORM_FREEBSD)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    elseif(PLATFORM_WINDOWS)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

# Common source files (platform-agnostic logic)
set(COMMON_SOURCES
    main.cpp
    AsciiArt.cpp
    ConfigReader.cpp
    Helpers.cpp
    TimeInfo.cpp
    personalization_info.cpp
    DistroDetector.cpp
)

# Windows-specific sources
set(WINDOWS_SOURCES
    OSInfo.cpp
    CPUInfo.cpp
    MemoryInfo.cpp
    GPUInfo.cpp
    StorageInfo.cpp
    NetworkInfo.cpp
    DisplayInfo.cpp
    UserInfo.cpp
    SystemInfo.cpp
    PerformanceInfo.cpp
    ExtraInfo.cpp
    DtailedGPUInfo.cpp
    DetailedScreen.cpp
    ConsoleUtils.cpp
    CompactOS.cpp
    CompactCPU.cpp
    CompactMemory.cpp
    CompactGPU.cpp
    CompactScreen.cpp
    CompactSystem.cpp
    CompactUser.cpp
    CompactNetwork.cpp
    CompactPerformance.cpp
    CompactAudio.cpp
    compact_disk_info.cpp
)

# POSIX/Unix sources (Linux + FreeBSD shared)
set(POSIX_SOURCES
    platform/posix/UserInfoPosix.cpp
    platform/posix/CompactOSPosix.cpp
    platform/posix/CompactCPUPosix.cpp
    platform/posix/CompactMemoryPosix.cpp
    platform/posix/CompactGPUPosix.cpp
    platform/posix/CompactScreenPosix.cpp
    platform/posix/CompactNetworkPosix.cpp
    platform/posix/CompactUserPosix.cpp
    platform/posix/CompactPerformancePosix.cpp
    platform/posix/CompactSystemPosix.cpp
    platform/posix/compact_disk_infoPosix.cpp
    platform/posix/CompactAudioPosix.cpp
)

# Linux-specific sources
set(LINUX_SOURCES
    platform/linux/OSInfoLinux.cpp
    platform/linux/CPUInfoLinux.cpp
    platform/linux/MemoryInfoLinux.cpp
    platform/linux/GPUInfoLinux.cpp
    platform/linux/StorageInfoLinux.cpp
    platform/linux/NetworkInfoLinux.cpp
    platform/linux/DisplayInfoLinux.cpp
    platform/linux/SystemInfoLinux.cpp
    platform/linux/PerformanceInfoLinux.cpp
)

# FreeBSD-specific sources
set(FREEBSD_SOURCES
    platform/freebsd/OSInfoFreeBSD.cpp
    platform/freebsd/CPUInfoFreeBSD.cpp
    platform/freebsd/MemoryInfoFreeBSD.cpp
    platform/freebsd/GPUInfoFreeBSD.cpp
    platform/freebsd/StorageInfoFreeBSD.cpp
    platform/freebsd/NetworkInfoFreeBSD.cpp
    platform/freebsd/DisplayInfoFreeBSD.cpp
    platform/freebsd/SystemInfoFreeBSD.cpp
    platform/freebsd/PerformanceInfoFreeBSD.cpp
    platform/freebsd/AudioInfoFreeBSD.cpp
)

# POSIX-specific compact sources (replaces Windows versions)
set(POSIX_COMPACT_SOURCES
    platform/posix/ExtraInfoPosix.cpp
    platform/posix/DetailedGPUInfoPosix.cpp
    platform/posix/DetailedScreenPosix.cpp
    platform/posix/ConsoleUtilsPosix.cpp
)

# Windows compact-related sources
set(WINDOWS_COMPACT_SOURCES
    ExtraInfo.cpp
    DtailedGPUInfo.cpp
    DetailedScreen.cpp
    ConsoleUtils.cpp
)

# Select platform sources
if(PLATFORM_WINDOWS)
    set(PLATFORM_SOURCES ${WINDOWS_SOURCES} ${WINDOWS_COMPACT_SOURCES})
    set(PLATFORM_LIBS
        wbemuuid
        pdh
        iphlpapi
        ws2_32
        wlanapi
        winhttp
        dxgi
        d3d12
        netapi32
        setupapi
        cfgmgr32
        Shcore
        ole32
        oleaut32
    )
elseif(PLATFORM_LINUX)
    set(PLATFORM_SOURCES ${POSIX_SOURCES} ${LINUX_SOURCES} ${POSIX_COMPACT_SOURCES})
    set(PLATFORM_LIBS pthread)
elseif(PLATFORM_FREEBSD)
    set(PLATFORM_SOURCES ${POSIX_SOURCES} ${FREEBSD_SOURCES} ${POSIX_COMPACT_SOURCES})
    set(PLATFORM_LIBS pthread)
endif()

# Create executable
add_executable(binaryfetch ${COMMON_SOURCES} ${PLATFORM_SOURCES})

# Include directories
target_include_directories(binaryfetch PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/nlohmann
    ${CMAKE_CURRENT_SOURCE_DIR}/platform
)

# Link libraries
target_link_libraries(binaryfetch PRIVATE ${PLATFORM_LIBS})

# Compiler warnings
if(MSVC)
    target_compile_options(binaryfetch PRIVATE /W4)
else()
    target_compile_options(binaryfetch PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Install
install(TARGETS binaryfetch RUNTIME DESTINATION bin)

# Platform-specific install paths for config
if(PLATFORM_LINUX)
    install(FILES Default_BinaryFetch_Config.json DESTINATION share/binaryfetch RENAME BinaryFetch_Config.json)
    install(FILES Default_Ascii_Art.txt DESTINATION share/binaryfetch RENAME BinaryArt.txt)
elseif(PLATFORM_FREEBSD)
    install(FILES Default_BinaryFetch_Config.json DESTINATION share/binaryfetch RENAME BinaryFetch_Config.json)
    install(FILES Default_Ascii_Art.txt DESTINATION share/binaryfetch RENAME BinaryArt.txt)
endif()

message(STATUS "Building BinaryFetch for: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Static linking: ${BUILD_STATIC}")
