cmake_minimum_required(VERSION 3.16)
project(FFVision LANGUAGES CXX)

include(qt.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置 QtNodes 路径
set(QtNodes_DIR " ${PROJECT_SOURCES}/3rd/QtNodes")

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
    PRIVATE
        Qt::Core
        Qt::Gui
        Qt::Widgets
     PRIVATE QtNodes
)

