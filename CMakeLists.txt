cmake_minimum_required(VERSION 3.16)
project(kalah LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_executable(kalah
    src/main.cpp
    src/kalah.h
    src/kalah.cpp
    src/gamecontroller.h
    src/gamecontroller.cpp
    ui/mainwindow.h
    ui/mainwindow.cpp
    ui/welcomescreen.h
    ui/welcomescreen.cpp
    ui/namescreen.h
    ui/namescreen.cpp
    ui/genderscreen.h
    ui/genderscreen.cpp
    ui/difficultyscreen.h
    ui/difficultyscreen.cpp
    ui/gamescreen.h
    ui/gamescreen.cpp
    ui/pitwidget.h
    ui/pitwidget.cpp
    ui/storewidget.h
    ui/storewidget.cpp
)

target_include_directories(kalah PRIVATE
    ${GTKMM_INCLUDE_DIRS}
    src
    ui
)
target_link_libraries(kalah PRIVATE ${GTKMM_LIBRARIES})
target_compile_options(kalah PRIVATE ${GTKMM_CFLAGS_OTHER})

install(TARGETS kalah DESTINATION bin)

enable_testing()
add_subdirectory(test)
