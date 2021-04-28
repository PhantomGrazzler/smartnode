cmake_minimum_required(VERSION 3.12)

project(smartnode
        VERSION 0.1.0
        DESCRIPTION "A SmartNode server and a SmartNode UI written in C++"
        HOMEPAGE_URL "https://github.com/PhantomGrazzler/smartnode"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Let Conan do it's magic.
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

# Enable folders for IDEs.
set_property(GLOBAL PROPERTY USE_FOLDERS YES)

# Generate compile_commands.json to make it easier to work with clang based tools.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Copy relevant configuration files to the build directory.
configure_file(.editorconfig .editorconfig COPYONLY)
configure_file(.clang-format .clang-format COPYONLY)

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake.
include(cmake/CompilerWarnings.cmake)
add_library(project_warnings INTERFACE)
set_project_warnings(project_warnings)

# Compile definitions for MSVC.
if(MSVC)
    add_compile_definitions(
        _WIN32_WINNT=0x0602
        WIN32_LEAN_AND_MEAN
    )
endif()

# Now add the sub-directory that actually contains our executables.
add_subdirectory(src)
