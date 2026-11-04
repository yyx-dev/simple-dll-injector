cmake_minimum_required(VERSION 3.16)

project(DLL-Injector VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(DLL-Injector
    src/main.cpp
    src/main_window.cpp
    src/main_window.h
    src/components.h
    src/render.h
    src/injector.h
    src/utils.h
    res/res.qrc
)

target_link_libraries(DLL-Injector PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
)

target_include_directories(DLL-Injector PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)