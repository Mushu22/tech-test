add_library(monitor STATIC monitor.cpp)

target_include_directories(monitor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )
target_link_libraries(monitor PUBLIC nlohmann_json::nlohmann_json)

