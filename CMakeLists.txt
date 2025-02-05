cmake_minimum_required(VERSION 3.28)
project(TechnicalTest LANGUAGES CXX)

# Global CXX settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Werror)

# Use local install directory if no prefix provided
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "Default install path" FORCE)
endif()

# Display path configuration
message("PROJECT SOURCE DIR:" ${CMAKE_CURRENT_SOURCE_DIR})
message("PROJECT BINARY DIR:" ${CMAKE_CURRENT_BINARY_DIR})
message("INSTALL PREFIX DIR:" ${CMAKE_INSTALL_PREFIX})

# Use FetchContent for dependencies
include(FetchContent)

# Get cppzmq headers and disable build and install
# libzmq not added here, have to be installed on the system
FetchContent_Declare(
    cppzmq
    GIT_REPOSITORY https://github.com/zeromq/cppzmq.git
    GIT_TAG        v4.10.0
    EXCLUDE_FROM_ALL
)
set(CPPZMQ_BUILD_TESTS OFF CACHE BOOL "Disable cppzmq tests" FORCE)
set(CPPZMQ_BUILD_EXAMPLES OFF CACHE BOOL "Disable cppzmq examples" FORCE)
set(CPPZMQ_INSTALL OFF CACHE BOOL "Disable cppzmq installation" FORCE)
FetchContent_MakeAvailable(cppzmq)

# Get nlohmann json library
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

# Add source subdirectories
add_subdirectory(src/monitor)
add_subdirectory(src/server)
add_subdirectory(src/client)

# Install client and server binary 
install(TARGETS client server
    RUNTIME DESTINATION bin
)

