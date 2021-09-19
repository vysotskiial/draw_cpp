cmake_minimum_required(VERSION 3.1)
project(modeling VERSION 1.0)

find_package(Eigen3 REQUIRED)
find_package(wxWidgets REQUIRED COMPONENTS core base adv propgrid)
include(${wxWidgets_USE_FILE})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(modeling src/modeling.cpp)

target_include_directories(modeling PRIVATE include)
target_link_libraries(modeling draw_lib Eigen3::Eigen)
