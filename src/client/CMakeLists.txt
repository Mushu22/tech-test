add_executable(client client.cpp)

target_link_libraries(client PRIVATE cppzmq nlohmann_json::nlohmann_json)
