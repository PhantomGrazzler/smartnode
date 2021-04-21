# UI executable
add_library(messaging)

target_include_directories(messaging
    PUBLIC  include)

target_sources(messaging
    PRIVATE include/parser.hpp
            include/messages.hpp
            include/id_types.hpp

            parser.cpp
            id_types.cpp
    )

target_link_libraries(messaging
    PRIVATE project_warnings
            CONAN_PKG::boost
    )
