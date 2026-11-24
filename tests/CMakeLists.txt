add_executable(tst_proslides tst_proslides.cpp ${PS_RESOURCES})
target_link_libraries(tst_proslides PRIVATE
    ps_core ps_persistence ps_crdt ps_presentation ps_timeline ps_scripture ps_timers ps_templates ps_overlays ps_audio ps_triggers ps_backup ps_providers ps_cloud ps_chords
    Qt6::Core Qt6::Test Qt6::Sql
)
add_test(NAME tst_proslides COMMAND tst_proslides)
