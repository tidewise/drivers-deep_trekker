rock_gtest(test_deep_trekker
    suite.cpp
    test_CommandAndStateMessageParser.cpp
    DEPS deep_trekker)

set_tests_properties(test-test_deep_trekker-cxx PROPERTIES ENVIRONMENT
                     "DEEP_TREKKER_SNAPSHOT_DIR=${CMAKE_CURRENT_SOURCE_DIR}/snapshots")