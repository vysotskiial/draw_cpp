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

set(IMAGES_INSTALLATION_PATH "${CMAKE_INSTALL_PREFIX}/share/draw_cpp")

set(INCLUDES_PATH $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/draw_cpp>)

add_library(symbolic_math src/formula_processor.cpp)
target_include_directories(symbolic_math PUBLIC ${INCLUDES_PATH})
set_target_properties(symbolic_math PROPERTIES PUBLIC_HEADER "include/formula_processor.h")

add_library(drawing src/picture_panel.cpp src/control_panel.cpp include/widgets.h src/main_window.cpp src/chart_dialog.cpp)
target_compile_definitions(drawing PRIVATE IMAGES_PATH="${IMAGES_INSTALLATION_PATH}")
set_target_properties(drawing PROPERTIES PUBLIC_HEADER "include/widgets.h;include/solver.h")
target_include_directories(drawing PUBLIC ${INCLUDES_PATH} /usr/include/klftools /usr/include/klfbackend)
target_link_libraries(drawing PUBLIC Qt5::Widgets Qt5::Charts klfbackend nlohmann_json::nlohmann_json symbolic_math)


add_executable(drawcpp src/main.cpp)
target_link_libraries(drawcpp PRIVATE drawing symbolic_math)

add_executable(symbolic_math_test test/symbols_test.cpp)
target_link_libraries(symbolic_math_test symbolic_math GTest::GTest)

install(TARGETS drawing symbolic_math drawcpp
        EXPORT drawcpp
        ARCHIVE DESTINATION lib/draw_cpp
        PUBLIC_HEADER DESTINATION include/draw_cpp
        RUNTIME DESTINATION bin
)
install(EXPORT drawcpp DESTINATION lib/cmake)
install(FILES cmake/drawcppConfig.cmake DESTINATION lib/cmake)
install(DIRECTORY images DESTINATION ${IMAGES_INSTALLATION_PATH})
