cmake_minimum_required(VERSION 3.16)
project(mancala LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.2 COMPONENTS Core Quick QuickControls2 REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

qt_add_executable(mancala
    src/main.cpp
    src/kalah.h
    src/kalah.cpp
    src/gamecontroller.h
    src/gamecontroller.cpp
)

qt_add_qml_module(mancala
    URI Mancala
    VERSION 1.0
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

target_link_libraries(mancala PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::QuickControls2
)
