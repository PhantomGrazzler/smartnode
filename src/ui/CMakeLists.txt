# UI executable
add_executable(ui)

target_sources(ui
    PRIVATE main.cpp
            session.hpp
            session.cpp
)

target_link_libraries(ui
    PRIVATE project_warnings
            messaging

            CONAN_PKG::imgui-sfml
            CONAN_PKG::boost
)

if(MSVC)
    target_compile_options(ui
        PRIVATE /bigobj
    )
endif()
