#
# Download GoogleTest
#
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

enable_testing()
add_definitions(-DTEST_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
add_definitions(-DBUILD_DIR="${CMAKE_BINARY_DIR}")

# ── unit_tests: KalahGame pure C++17 logic, no Qt ────────────────────────
set(TEST_SOURCES
    kalah_test.cpp
    ../src/kalah.cpp
)

add_executable(unit_tests EXCLUDE_FROM_ALL ${TEST_SOURCES})
target_include_directories(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(unit_tests PRIVATE gtest_main)
gtest_discover_tests(unit_tests EXTRA_ARGS --gtest_repeat=1 PROPERTIES TIMEOUT 10)

# ── controller_tests: GameController + Qt6::Core signals ─────────────────
add_executable(controller_tests EXCLUDE_FROM_ALL
    controller_test.cpp
    ../src/kalah.cpp
    ../src/gamecontroller.cpp
)
target_include_directories(controller_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(controller_tests PRIVATE gtest Qt6::Core)
set_target_properties(controller_tests PROPERTIES AUTOMOC ON)
gtest_discover_tests(controller_tests EXTRA_ARGS --gtest_repeat=1 PROPERTIES TIMEOUT 30)
