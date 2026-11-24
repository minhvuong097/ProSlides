add_library(ps_backup STATIC
    BackupManager.cpp
)

target_include_directories(ps_backup PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(ps_backup PUBLIC
    Qt6::Core
)
