include(CMakeFindDependencyMacro)
find_dependency(Qt5 REQUIRED COMPONENTS Widgets Charts)
find_dependency(nlohmann_json REQUIRED)

include("${CMAKE_INSTALL_PREFIX}/lib/cmake/drawcpp.cmake")
