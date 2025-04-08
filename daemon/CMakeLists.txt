file(GLOB_RECURSE SOURCES_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_executable(RyzenAdjDaemon ${SOURCES_FILES})
target_include_directories(RyzenAdjDaemon PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../external/RyzenAdj/lib)
target_link_libraries(RyzenAdjDaemon PRIVATE libryzenadj nlohmann_json fmt::fmt)
