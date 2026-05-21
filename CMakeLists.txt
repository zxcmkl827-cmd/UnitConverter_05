cmake_minimum_required(VERSION 3.16)

project(UnitConverter_05 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(UnitConverter cpp/UnitConverter.cpp)

enable_testing()

add_executable(unit_converter_red_tests tests/test_unit_converter_red.cpp)
target_include_directories(unit_converter_red_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/catch2
)
add_dependencies(unit_converter_red_tests UnitConverter)

add_test(NAME unit_converter_red_tests COMMAND unit_converter_red_tests)
set_tests_properties(unit_converter_red_tests PROPERTIES
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
