cmake_minimum_required(VERSION 3.12)

project(SmartNodeServer
        VERSION 0.1.0
        DESCRIPTION "Websocket server for SmartNodes and SmartNode UIs"
        HOMEPAGE_URL "https://github.com/PhantomGrazzler/SmartNodeServer"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Let Conan do it's magic.
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

# Options to add compiler warning flags, and turn on warnings as errors
if(MSVC)
    add_compile_options(/W4 /WX)
    add_compile_definitions(
        _WIN32_WINNT=0x0602
        WIN32_LEAN_AND_MEAN
    )
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

# Main application executable
add_executable(smartnode-server)

target_include_directories(smartnode-server
    PRIVATE include
    )

target_sources(smartnode-server
    PRIVATE include/Listener.h
            include/Session.h
            include/FailPrinters.h

            src/Connection.h
            src/MessageTypes.h
            src/main.cpp
            src/Listener.cpp
            src/Session.cpp
            src/FailPrinters.cpp
            src/MessageEngine.h
            src/MessageEngine.cpp
            src/PeerIdTypes.hpp
    )

target_link_libraries(smartnode-server
    PRIVATE CONAN_PKG::boost
            CONAN_PKG::nlohmann_json
            CONAN_PKG::rang
    )
