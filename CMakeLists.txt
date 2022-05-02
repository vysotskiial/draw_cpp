cmake_minimum_required(VERSION 3.1)
project(drawing VERSION 1.0)

find_package(Qt5 REQUIRED COMPONENTS Widgets Charts)
find_package(GTest REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(drawing src/picture_panel.cpp src/control_panel.cpp include/widgets.h src/graph_choice_panel.cpp src/main_window.cpp)
target_include_directories(drawing PUBLIC include /usr/include/klftools /usr/include/klfbackend)
target_link_libraries(drawing PUBLIC Qt5::Widgets Qt5::Charts klfbackend)

add_library(symbolic_math symbolic_math/formula_processor.cpp)
target_include_directories(symbolic_math PUBLIC include)

add_executable(graph_test test/main.cpp)
target_link_libraries(graph_test PRIVATE drawing symbolic_math)

add_executable(symbolic_math_test test/symbols_test.cpp)
target_link_libraries(symbolic_math_test symbolic_math GTest::GTest)
