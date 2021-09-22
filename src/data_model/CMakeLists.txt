# data_model library
add_library(data_model INTERFACE)

target_include_directories(data_model
    INTERFACE   include
)

target_sources(data_model
    PRIVATE include/data_types.hpp
)
