cmake_minimum_required(VERSION 3.1)
project(drawing VERSION 1.0)

find_package(Qt5 REQUIRED COMPONENTS Widgets Charts)
find_package(GTest REQUIRED)
find_package(nlohmann_json REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra -Wshadow")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(IMAGES_INSTALLATION_PATH "/usr/local/share/draw_cpp")
message(${IMAGES_INSTALLATION_PATH})

add_library(symbolic_math symbolic_math/formula_processor.cpp)
target_include_directories(symbolic_math PUBLIC include)
set_target_properties(symbolic_math PROPERTIES PUBLIC_HEADER "include/formula_processor.h")

add_library(drawing src/picture_panel.cpp src/control_panel.cpp include/widgets.h src/main_window.cpp src/chart_dialog.cpp)
target_compile_definitions(drawing PRIVATE IMAGES_PATH="${IMAGES_INSTALLATION_PATH}")
set_target_properties(drawing PROPERTIES PUBLIC_HEADER "include/widgets.h;include/solver.h")
target_include_directories(drawing PUBLIC include /usr/include/klftools /usr/include/klfbackend)
target_link_libraries(drawing PUBLIC Qt5::Widgets Qt5::Charts klfbackend nlohmann_json::nlohmann_json)


add_executable(graph_test test/main.cpp)
target_link_libraries(graph_test PRIVATE drawing symbolic_math)

add_executable(symbolic_math_test test/symbols_test.cpp)
target_link_libraries(symbolic_math_test symbolic_math GTest::GTest)

install(TARGETS drawing symbolic_math
        ARCHIVE DESTINATION /usr/local/lib/draw_cpp
        PUBLIC_HEADER DESTINATION /usr/local/include/draw_cpp
)
install(DIRECTORY images DESTINATION ${IMAGES_INSTALLATION_PATH}
)
