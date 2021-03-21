cmake_minimum_required(VERSION 3.1)
project(name VERSION 1.0)

find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(test src/main.cpp)

target_include_directories(test PRIVATE include)
target_link_libraries(test ${wxWidgets_LIBRARIES})
