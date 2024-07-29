# Create the test executable
add_executable(run_tests test_bank.cpp)
target_link_libraries(run_tests PRIVATE bank_lib)

# Add include directories for tests
target_include_directories(run_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)