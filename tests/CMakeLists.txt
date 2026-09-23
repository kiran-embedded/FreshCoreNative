add_executable(test_config test_config.cpp ../config/config.cpp ../logging/logger.cpp)
add_test(NAME config_tests COMMAND test_config)

add_executable(test_state test_state_storage.cpp ../cache/state_storage.cpp ../logging/logger.cpp)
add_test(NAME state_tests COMMAND test_state)
