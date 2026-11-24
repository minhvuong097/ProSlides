add_library(ps_templates STATIC
    TemplateLibrary.cpp
)

target_include_directories(ps_templates PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(ps_templates PUBLIC
    ps_core
    Qt6::Core
)
