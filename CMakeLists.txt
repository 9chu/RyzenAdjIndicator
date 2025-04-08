cmake_minimum_required(VERSION 3.30)
project(RyzenAdjIndicator CXX)

##### 依赖

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(PkgConfig REQUIRED)
find_package(APPINDICATOR REQUIRED)

pkg_check_modules(GTK3 REQUIRED gtk+-3.0 gtkmm-3.0>=3.24.0 glibmm-2.4>=2.60.0)
if(NOT GTK3_FOUND)
    message(FATAL_ERROR "GTK3 is required but wasn't found")
endif()

add_subdirectory(external/fmt)
add_subdirectory(external/nlohmann_json)
add_subdirectory(external/RyzenAdj)

##### 工程

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(daemon)
add_subdirectory(app)
