# Server executable.
add_executable(server)

target_sources(server
    PRIVATE main.cpp
            connection.hpp
            console_printer.hpp
            listener.cpp
            listener.hpp
            logger.cpp
            logger.hpp
            session.cpp
            session.hpp
            message_engine.cpp
            message_engine.hpp
    )

target_link_libraries(server
    PRIVATE project_warnings
            messaging
            CONAN_PKG::boost
            CONAN_PKG::nlohmann_json
            CONAN_PKG::rang
            CONAN_PKG::spdlog
    )
