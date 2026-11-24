add_library(ps_cloud STATIC
    CloudSyncClient.cpp
)

target_include_directories(ps_cloud PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(ps_cloud PUBLIC
    ps_crdt
    Qt6::Core
    Qt6::Network
    Qt6::WebSockets
)
