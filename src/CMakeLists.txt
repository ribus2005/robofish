cmake_minimum_required(VERSION 3.22)
project(CMakeSFMLProject LANGUAGES CXX)

include(FetchContent)
include(GoogleTest)



FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

#find_package(googletest REQUIRED)
FetchContent_MakeAvailable(googletest)
enable_testing()


add_executable(main main.cpp candle.cpp)

add_executable(is_green is_green.cpp candle.cpp)
target_link_libraries(is_green GTest::gtest_main)

add_executable(is_red is_red.cpp candle.cpp)
target_link_libraries(is_red GTest::gtest_main)

add_executable(body_size body_size.cpp candle.cpp)
target_link_libraries(body_size GTest::gtest_main)

add_executable(full_size full_size.cpp candle.cpp)
target_link_libraries(full_size GTest::gtest_main)

add_executable(contains contains.cpp candle.cpp)
target_link_libraries(contains GTest::gtest_main)

add_executable(body_contains body_contains.cpp candle.cpp)
target_link_libraries(body_contains GTest::gtest_main)



add_test(NAME green COMMAND is_green)
add_test(NAME red COMMAND is_red)
add_test(NAME contain COMMAND contains)
add_test(NAME body COMMAND body_size)
add_test(NAME full COMMAND full_size)