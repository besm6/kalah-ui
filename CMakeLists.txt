cmake_minimum_required(VERSION 3.16)
project(kalah LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.2 COMPONENTS Core Quick QuickControls2 REQUIRED)

if(Qt6_VERSION VERSION_GREATER_EQUAL "6.5")
    qt_policy(SET QTP0001 NEW)
    qt_policy(SET QTP0004 NEW)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

qt_add_executable(kalah
    src/main.cpp
    src/kalah.h
    src/kalah.cpp
    src/gamecontroller.h
    src/gamecontroller.cpp
)

qt_add_qml_module(kalah
    URI Kalah
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    QML_FILES
        qml/Main.qml
        qml/WelcomeScreen.qml
        qml/NameScreen.qml
        qml/GenderScreen.qml
        qml/DifficultyScreen.qml
        qml/GameScreen.qml
        qml/Pit.qml
        qml/Store.qml
)

target_link_libraries(kalah PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::QuickControls2
)

enable_testing()
add_subdirectory(test)
