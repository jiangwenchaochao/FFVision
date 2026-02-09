cmake_minimum_required(VERSION 3.16)
project(FFVision LANGUAGES CXX)

#include(qt.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


# QtNodes path
#set(QtNodes_DIR "${CMAKE_SOURCE_DIR}/3rd/nodeeditor")

# QtNodes
#add_subdirectory(${QtNodes_DIR} QtNodes_build)

# QtNodes subdirectory
#add_subdirectory("${CMAKE_SOURCE_DIR}/test" FFtest)

#find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
#find_package(Qt${QT_VERSION_MAJOR}
#    COMPONENTS
#        Core
#        Gui
#        Widgets
#)
#qt_standard_project_setup()


set(PROJECT_SOURCES
    src/flow/FlowExample.cpp
    src/flow/Edge.h
    src/flow/ExecutionContext.h
    src/flow/Flow.h
    src/flow/FlowEngine.h
    src/flow/Graph.h
    src/flow/Node.h
    src/flow/Nodes.h
)

# Create executable (console application)
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

# Set as console application (show console window)
set_target_properties(${PROJECT_NAME}
    PROPERTIES
        WIN32_EXECUTABLE FALSE
)

target_link_libraries(${PROJECT_NAME}

)
#    PRIVATE QtNodes
#    PRIVATE
#        Qt::Core
#        Qt::Gui
#        Qt::Widgets
