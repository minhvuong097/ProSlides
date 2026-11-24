add_library(ps_persistence STATIC
    Database.cpp
    ShowRepository.cpp
)

target_include_directories(ps_persistence PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(ps_persistence PUBLIC
    ps_core
    Qt6::Core
    Qt6::Sql
)
