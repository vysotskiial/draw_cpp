cmake_minimum_required(VERSION 3.1)
project(name VERSION 1.0)

find_package(wxWidgets REQUIRED COMPONENTS core base adv propgrid)
find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
include(${wxWidgets_USE_FILE})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(test src/main.cpp src/main_window.cpp src/picture_panel.cpp src/control_panel.cpp)

target_include_directories(test PRIVATE include /usr/include/klfbackend /usr/include/klftools)
target_link_libraries(test ${wxWidgets_LIBRARIES} Qt5::Core Qt5::Gui Qt5::Widgets klfbackend)

add_library(draw_lib src/main_window.cpp src/picture_panel.cpp src/control_panel.cpp)
target_include_directories(draw_lib PUBLIC /usr/include/klfbackend /usr/include/klftools include)
target_link_libraries(draw_lib ${wxWidgets_LIBRARIES} Qt5::Core Qt5::Gui Qt5::Widgets klfbackend)
