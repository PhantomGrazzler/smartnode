# data_model library
add_library(data_model)

target_include_directories(data_model
    PUBLIC  include
)

target_sources(data_model
    PRIVATE include/data_types.hpp
)

target_link_libraries(data_model
    PRIVATE project_warnings
)

set_target_properties(data_model PROPERTIES LINKER_LANGUAGE CXX)
