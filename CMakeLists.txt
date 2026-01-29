cmake_minimum_required(VERSION 3.16)
project(FFVision LANGUAGES CXX)

include(qt.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


# 设置 QtNodes 路径
set(QtNodes_DIR "${CMAKE_SOURCE_DIR}/3rd/nodeeditor")

# 添加 QtNodes 子目录
add_subdirectory(${QtNodes_DIR} QtNodes_build)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR}
    COMPONENTS
        Core
        Gui
        Widgets
)
qt_standard_project_setup()

set(PROJECT_SOURCES
    src/main.cpp
    src/models.cpp
    src/models.hpp
    src/FFVision.ui
    src/FFVision.h
    src/FFVision.cpp
)

qt_add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        WIN32_EXECUTABLE TRUE
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE QtNodes
    PRIVATE
        Qt::Core
        Qt::Gui
        Qt::Widgets
)

