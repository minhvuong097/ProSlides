add_executable(tst_proslides tst_proslides.cpp)
target_link_libraries(tst_proslides PRIVATE
    ps_core ps_persistence ps_crdt ps_presentation
    Qt6::Core Qt6::Test Qt6::Sql
)
add_test(NAME tst_proslides COMMAND tst_proslides)
