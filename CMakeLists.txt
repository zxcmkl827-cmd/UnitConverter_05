cmake_minimum_required(VERSION 3.16)

project(UnitConverter_05 LANGUAGES CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(APPROVAL_TESTS_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(APPROVAL_TESTS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(APPROVAL_TESTS_BUILD_THIRD_PARTY_CATCH2 ON CACHE BOOL "" FORCE)
FetchContent_Declare(
    ApprovalTestsCpp
    GIT_REPOSITORY https://github.com/approvals/ApprovalTests.cpp.git
    GIT_TAG master
)
FetchContent_MakeAvailable(ApprovalTestsCpp)

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

add_executable(UnitConverter_test tests/test_golden_master.cpp)
target_include_directories(UnitConverter_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/catch2
    ${approvaltestscpp_SOURCE_DIR}/third_party/catch2/include
)
target_compile_definitions(UnitConverter_test PRIVATE
    UNIT_CONVERTER_EXE="$<TARGET_FILE:UnitConverter>"
)
target_link_libraries(UnitConverter_test PRIVATE ApprovalTests::ApprovalTests)
add_dependencies(UnitConverter_test UnitConverter)

add_test(NAME GoldenMaster COMMAND UnitConverter_test)
set_tests_properties(GoldenMaster PROPERTIES
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
