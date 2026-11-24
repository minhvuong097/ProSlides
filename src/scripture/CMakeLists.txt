add_library(ps_scripture STATIC
    Bible.cpp
    ScriptureProvider.cpp
)

target_include_directories(ps_scripture PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(ps_scripture PUBLIC
    ps_core
    ps_persistence
    Qt6::Core
    Qt6::Sql
)
