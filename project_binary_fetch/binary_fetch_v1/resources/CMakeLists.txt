cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

project(binary_fetch_v1 CXX)

################################################################################
# Set target arch type if empty. Visual studio solution generator provides it.
################################################################################
if(NOT CMAKE_VS_PLATFORM_NAME)
    set(CMAKE_VS_PLATFORM_NAME "x64")
endif()
message("${CMAKE_VS_PLATFORM_NAME} architecture in use")

if(NOT ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64"
     OR "${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86"))
    message(FATAL_ERROR "${CMAKE_VS_PLATFORM_NAME} arch is not supported!")
endif()

################################################################################
# Global configuration types
################################################################################
set(CMAKE_CONFIGURATION_TYPES
    "Debug"
    "Release"
    CACHE STRING "" FORCE
)

################################################################################
# Global compiler options
################################################################################
if(MSVC)
    # remove default flags provided with CMake for MSVC
    set(CMAKE_CXX_FLAGS "")
    set(CMAKE_CXX_FLAGS_DEBUG "")
    set(CMAKE_CXX_FLAGS_RELEASE "")
endif()

################################################################################
# Global linker options
################################################################################
if(MSVC)
    # remove default flags provided with CMake for MSVC
    set(CMAKE_EXE_LINKER_FLAGS "")
    set(CMAKE_MODULE_LINKER_FLAGS "")
    set(CMAKE_SHARED_LINKER_FLAGS "")
    set(CMAKE_STATIC_LINKER_FLAGS "")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS}")
endif()

################################################################################
# Nuget packages function stub.
################################################################################
function(use_package TARGET PACKAGE VERSION)
    message(WARNING "No implementation of use_package. Create yours. "
                    "Package \"${PACKAGE}\" with version \"${VERSION}\" "
                    "for target \"${TARGET}\" is ignored!")
endfunction()

################################################################################
# Common utils
################################################################################
include(CMake/Utils.cmake)

################################################################################
# Additional Global Settings(add specific info there)
################################################################################
include(CMake/GlobalSettingsInclude.cmake OPTIONAL)

################################################################################
# Use solution folders feature
################################################################################
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################################################################################
# Sub-projects
################################################################################



























set(PROJECT_NAME BinaryFetch)

################################################################################
# Source groups
################################################################################
set(no_group_source_files
    "Documentation/BinaryFetch documentation.md"
    "nlohmann/json.hpp"
    "text infos/better output format.txt"
    "text infos/Commit_Info.txt"
    "text infos/Info about main.cpp.txt"
    "text infos/locations.md"
    "text infos/main 8.txt"
    "text infos/main-4 copy.txt"
    "text infos/main-5 copy.txt"
    "text infos/main-6 copy.txt"
    "text infos/main-7 copy.txt"
    "text infos/main.cpp with out josn.txt"
    "text infos/project structure.txt"
    "text infos/temp_output.txt"
)
source_group("" FILES ${no_group_source_files})

set(Config
    "Default_BinaryFetch_Config.json"
)
source_group("Config" FILES ${Config})

set(Resources
    "Art_Collections.txt"
    "AsciiArt_Documentation.md"
    "DefaultAsciiArt.txt"
    "Standalone Json File management.md"
)
source_group("Resources" FILES ${Resources})

set(include
    "AsciiArt.h"
    "compact_disk_info.h"
    "CompactAudio.h"
    "CompactCPU.h"
    "CompactGPU.h"
    "CompactMemory.h"
    "CompactNetwork.h"
    "CompactOS.h"
    "CompactPerformance.h"
    "CompactScreen.h"
    "CompactSystem.h"
    "CompactUser.h"
    "CPUInfo.h"
    "DetailedGPUInfo.h"
    "DisplayInfo.h"
    "ExtraInfo.h"
    "GPUInfo.h"
    "json.hpp"
    "MemoryInfo.h"
    "NetworkInfo.h"
    "OSInfo.h"
    "PerformanceInfo.h"
    "personalization_info.h"
    "resource.h"
    "StorageInfo.h"
    "SystemInfo.h"
    "TimeInfo.h"
    "UserInfo.h"
)
source_group("include" FILES ${include})

set(include__Utlis
    "ConfigReader.cpp"
    "ConfigReader.h"
    "ConsoleUtils.cpp"
    "Helpers.cpp"
)
source_group("include\\Utlis" FILES ${include__Utlis})

set(src
    "AsciiArt.cpp"
    "compact_disk_info.cpp"
    "CompactAudio.cpp"
    "CompactCPU.cpp"
    "CompactGPU.cpp"
    "CompactMemory.cpp"
    "CompactNetwork.cpp"
    "CompactOS.cpp"
    "CompactPerformance.cpp"
    "CompactScreen.cpp"
    "CompactSystem.cpp"
    "CompactUser.cpp"
    "CPUInfo.cpp"
    "DisplayInfo.cpp"
    "DtailedGPUInfo.cpp"
    "ExtraInfo.cpp"
    "GPUInfo.cpp"
    "main.cpp"
    "MemoryInfo.cpp"
    "NetworkInfo.cpp"
    "OSInfo.cpp"
    "PerformanceInfo.cpp"
    "personalization_info.cpp"
    "StorageInfo.cpp"
    "SystemInfo.cpp"
    "TimeInfo.cpp"
    "UserInfo.cpp"
)
source_group("src" FILES ${src})

set(ALL_FILES
    ${no_group_source_files}
    ${Config}
    ${Resources}
    ${include}
    ${include__Utlis}
    ${src}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE binaryfetchv1)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    )
endif()
################################################################################
# MSVC runtime library
################################################################################
get_property(MSVC_RUNTIME_LIBRARY_DEFAULT TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY)
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
        MultiThreadedDebugDLL
    )
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})

################################################################################
# Include directories
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/"H:/programming/projects/project_binary_fetch/binary_fetch_v1/json.hpp""
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/"H:/programming/projects/project_binary_fetch/binary_fetch_v1/json.hpp""
    )
endif()

################################################################################
# Compile definitions
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "_DEBUG"
        ">"
        "$<$<CONFIG:Release>:"
            "NDEBUG"
        ">"
        "_CONSOLE;"
        "UNICODE;"
        "_UNICODE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "_DEBUG"
        ">"
        "$<$<CONFIG:Release>:"
            "NDEBUG"
        ">"
        "WIN32;"
        "_CONSOLE;"
        "UNICODE;"
        "_UNICODE"
    )
endif()

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /Oi;
                /Gy
            >
            /permissive-;
            /sdl;
            /W3;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /Oi;
                /Gy
            >
            /permissive-;
            /sdl;
            /W3;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    endif()
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_link_options(${PROJECT_NAME} PRIVATE
            /DEBUG;
            /SUBSYSTEM:CONSOLE
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        target_link_options(${PROJECT_NAME} PRIVATE
            /DEBUG;
            /SUBSYSTEM:CONSOLE
        )
    endif()
endif()

################################################################################
# Dependencies
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "$<$<CONFIG:Debug>:"
            "advapi32;"
            "user32;"
            "ole32;"
            "Gdi32"
            ""
        ">"
        "$<$<CONFIG:Release>:"
            "$(CoreLibraryDependencies)"
        ">"
        "nvapi64;"
        "Shell32"
    )
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_link_directories(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "C:/Users/OBITO/Downloads/nvapi-main/nvapi-main/amd64/"
        ">"
        "C:/NVAPI/nvapi-main/amd64"
    )
endif()

