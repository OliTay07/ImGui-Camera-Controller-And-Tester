# ==========================
# Project and General Settings
# ==========================
cmake_minimum_required(VERSION 3.30)
project(imguiWinApp VERSION 0.1 LANGUAGES CXX)

# Set version variables
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)
set(COMPANY_NAME "Jones AV Ltd")
set(PRODUCT_NAME "imguiWinApp")
set(PRODUCT_INTERNAL_NAME "imguiWinApp")
set(COPYRIGHT_YEAR 2025)

# Set the C++ Standard and target platform
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_GENERATOR_PLATFORM x64)  # Target x64 architecture
set(CMAKE_CXX_EXTENSIONS OFF)      # Disable compiler-specific extensions

# Enable global module scanning
set(CMAKE_CXX_SCAN_FOR_MODULES ON)  

# Set Windows SDK version
set(CMAKE_SYSTEM_VERSION 10.0.26100.0)

# ==========================
# Conan Integration
# ==========================
# Include the Conan-generated toolchain file
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_BINARY_DIR}/generators/conan_toolchain.cmake" CACHE STRING "")
include(${CMAKE_BINARY_DIR}/generators/conan_toolchain.cmake)


# ==========================
# Dependencies from Conan
# ==========================
find_package(OpenSSL REQUIRED)
find_package(Boost 1.86.0 REQUIRED) # COMPONENTS System Asio Beast Thread
find_package(imgui REQUIRED)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)

# ==========================
# Source Files (Target Definition)
# ==========================
# Set up the source files
set(SOURCES
    src/modules/ui/ui.ixx
    src/modules/logging/imguiSink.ixx
    src/modules/ui/ui.cpp
    src/main.cpp

    src/modules/restClient/restClient.cpp
    src/modules/restClient/restClient.hpp
    src/modules/wsClient/wsClient.cpp
    src/modules/wsClient/wsClient.hpp

    src/modules/presets/presets.cpp
    src/modules/presets/presets.hpp

    src/headers/Resource.h
    src/headers/framework.h
    src/headers/targetver.h
    src/imgui-backend/imgui_impl_dx11.h
    src/imgui-backend/imgui_impl_dx11.cpp
    src/imgui-backend/imgui_impl_win32.h
    src/imgui-backend/imgui_impl_win32.cpp
)

# Mark ixx files as module interfaces
set_source_files_properties(
    src/ui.ixx
    src/imguiSink.ixx
    PROPERTIES
        CXX_MODULE_INTERFACE ON
)

# Define the executable
add_executable(imguiWinApp WIN32 ${SOURCES})

# Add this after your add_executable command
if(MSVC)
    set_target_properties(imguiWinApp PROPERTIES
        LINK_FLAGS "/ENTRY:mainCRTStartup"
    )
endif()

# ==========================
# Function to auto-create filters based on file paths
# ==========================
function(create_source_groups)
    foreach(_source IN ITEMS ${ARGN})
        get_filename_component(_source_path "${_source}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${_source_path_msvc}" FILES "${_source}")
    endforeach()
endfunction()

# Call the function with your source files
create_source_groups(${SOURCES})

# ==========================
# Set Module Scanning for the Target
# ==========================
set_target_properties(imguiWinApp PROPERTIES CXX_SCAN_FOR_MODULES ON)

# ==========================
# Include Directories (after target definition)
# ==========================
# Function to extract unique directories from source files and add them to include directories
function(add_include_directories_from_sources target)
    set(directories)
    
    # Iterate through each source file and extract its directory
    foreach(source_file ${ARGN})
        get_filename_component(source_dir "${source_file}" PATH)
        list(APPEND directories "${source_dir}")
    endforeach()

    # Remove duplicate directories
    list(REMOVE_DUPLICATES directories)

    # Add each unique directory to the include path for the specified target
    foreach(dir ${directories})
        target_include_directories(${target} PRIVATE ${dir})
    endforeach()
endfunction()

# Call this function with the target and the sources
add_include_directories_from_sources(imguiWinApp ${SOURCES})

# libs
target_include_directories(imguiWinApp PRIVATE ${CMAKE_SOURCE_DIR}/libs)

# ==========================
# Visual Studio Filters for Resources
# ==========================
# Add non-build files to the source list for the imguiWinApp target
# Add non-build files to the source list for the imguiWinApp target
set(RESOURCE_FILES    
    ${CMAKE_SOURCE_DIR}/README.md
)

# Associate these files with filters in Visual Studio
source_group("" FILES ${CMAKE_SOURCE_DIR}/README.md)  # Empty filter means root

# Now make sure these resource files are visible in the imguiWinApp target, but not part of the build
target_sources(imguiWinApp PRIVATE ${RESOURCE_FILES})

# ==========================
# Set Subsystem Properties (Windows or Console)
# ==========================
# If your application is a Windows GUI application:
# set_target_properties(imguiWinApp PROPERTIES
#     LINK_FLAGS "/SUBSYSTEM:WINDOWS"
# )

# Set the directory for PDB and IDB files for Debug builds
if(MSVC)
    set_target_properties(imguiWinApp PROPERTIES
        COMPILE_PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/pdb"
        PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/pdb"
        INTERPROCEDURAL_OPTIMIZATION_RELEASE ON
    )

    # Place intermediate object files in a separate folder
    set_target_properties(imguiWinApp PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/lib/Debug"
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/Release"
        LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/lib/Debug"
        LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/Release"
    )
endif()

# ==========================
# Compiler and Language Options
# ==========================
if(MSVC)
    add_compile_options(/std:c++20 /experimental:module /MP)  # Enable C++20 standard, modules, and parallel build in MSVC

    # Set Debug options
    target_compile_options(imguiWinApp PRIVATE
        $<$<CONFIG:Debug>:/ZI /Od /Oi- /MTd /JMC>  # Edit and Continue, no optimization, multithreaded debug
    )

    # Set Release options
    target_compile_options(imguiWinApp PRIVATE
        $<$<CONFIG:Release>:/MT /O2 /GL /Ob2 /Oi /Zi /Gy>  # Multithreaded release, maximum optimization, link-time code generation
    )
    
    # Global compile options
    target_compile_options(imguiWinApp PRIVATE /Zc:inline /fp:precise /W3 /sdl)  # Inline handling, floating-point model, and warning level
    # Enable language conformance mode for Debug and Release
    target_compile_options(imguiWinApp PRIVATE /permissive-)
    
    ## LINKER
    # Linker options for both Debug and Release
    target_link_options(imguiWinApp PRIVATE
        $<$<CONFIG:Debug>:/DYNAMICBASE /NXCOMPAT>  # Security features for Debug
        $<$<CONFIG:Release>:/DYNAMICBASE /NXCOMPAT /LTCG /OPT:REF /OPT:ICF>  # Link-time code generation, optimization references and identical COMDAT folding
    )

    # Enable incremental linking for Debug, disable for Release
    target_link_options(imguiWinApp PRIVATE
        $<$<CONFIG:Debug>:/INCREMENTAL>         # Incremental linking in Debug
        $<$<CONFIG:Release>:/INCREMENTAL:NO>    # Disable incremental linking in Release
    )

endif()

# ==========================
# Preprocessor Definitions
# ==========================
set(CORE_DEFINES_DEBUG   "_WINDOWS;WIN32;UNICODE;_UNICODE;_DEBUG;M_LOGGER")
set(CORE_DEFINES_RELEASE "_WINDOWS;WIN32;UNICODE;_UNICODE;NDEBUG;M_LOGGER")

target_compile_definitions(imguiWinApp PRIVATE UNICODE _UNICODE
    $<$<CONFIG:Debug>:${CORE_DEFINES_DEBUG}>
    $<$<CONFIG:Release>:${CORE_DEFINES_RELEASE}>
)

# ==========================
# Linker and Library Settings
# ==========================

# Linking conan libs to imguiWinApp
target_link_libraries(imguiWinApp PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
    Boost::boost
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    imgui::imgui    
    d3d11.lib
    dxgi.lib
    d3dcompiler.lib
)

# Set module interface properties
set_source_files_properties(
    ${CMAKE_SOURCE_DIR}/src/ui.ixx
    ${CMAKE_SOURCE_DIR}/src/imguiSink.ixx
    PROPERTIES
    VS_SETTINGS "GenerateXMLDocumentationFiles=false"
    CXX_MODULE_INTERFACE ON
    CXX_MODULE_NAME ui
)

# Add module dependencies
set_source_files_properties(
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    PROPERTIES
    CXX_MODULE_DEPENDS ui
    CXX_MODULE_DEPENDS imguiSink
)