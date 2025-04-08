file(GLOB_RECURSE SOURCES_FILES src/*.cpp)
add_executable(RyzenAdjIndicator ${SOURCES_FILES})
target_include_directories(RyzenAdjIndicator PRIVATE ${GTK3_INCLUDE_DIRS} ${APPINDICATOR_INCLUDE_DIR})
target_link_directories(RyzenAdjIndicator PRIVATE ${GTK3_LIBRARY_DIRS})
target_link_libraries(RyzenAdjIndicator PRIVATE ${GTK3_LIBRARIES} ${APPINDICATOR_LIBRARY} nlohmann_json fmt::fmt)

add_custom_target(CopyAssets COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets)
add_custom_target(CopyDefaultConfig COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/config.json ${CMAKE_CURRENT_BINARY_DIR}/config.json)
add_custom_target(CopyDaemon COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:RyzenAdjDaemon>
        ${CMAKE_CURRENT_BINARY_DIR}/RyzenAdjDaemon DEPENDS RyzenAdjDaemon)
add_dependencies(RyzenAdjIndicator CopyAssets)
add_dependencies(RyzenAdjIndicator CopyDefaultConfig)
add_dependencies(RyzenAdjIndicator CopyDaemon)
