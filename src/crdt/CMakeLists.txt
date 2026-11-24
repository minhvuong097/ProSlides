add_library(ps_crdt STATIC
    Operation.cpp
    Document.cpp
    SyncSession.cpp
)

target_include_directories(ps_crdt PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(ps_crdt PUBLIC
    Qt6::Core
)
