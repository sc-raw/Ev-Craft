cmake_minimum_required(VERSION 3.21)
project(EV VERSION 0.1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.9.3 EXACT REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets)

qt_add_executable(EV WIN32
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/TimelineWidget.cpp
    src/TimelineWidget.h
)

target_link_libraries(EV PRIVATE Qt6::Widgets Qt6::Multimedia Qt6::MultimediaWidgets)

if(MINGW)
    target_compile_options(EV PRIVATE -Wall -Wextra -Wpedantic)
endif()
