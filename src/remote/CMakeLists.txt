add_library(ps_remote STATIC
    Protocol.cpp
    RemoteServer.cpp
    RemoteDiscovery.cpp
)

target_include_directories(ps_remote PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(ps_remote PUBLIC
    ps_core
    ps_crdt
    Qt6::Core
    Qt6::Network
    Qt6::WebSockets
)
