cmake_minimum_required(VERSION 3.16)

# Find required packages
find_package(Crow REQUIRED)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Create library target for telemetry core
add_library(telemetry-core
    # Models
    src/models/TelemetryData.cpp
    src/models/ErrorMessage.cpp
    
    # Storage
    src/storage/InMemoryStorage.cpp
    
    # Services
    src/services/WebSocketService.cpp
    src/services/TelemetryService.cpp
    src/services/ErrorService.cpp
    
    # API
    src/api/RestApi.cpp
    src/api/WebSocketApi.cpp
    
    # Server
    src/TelemetryServer.cpp
)

# Include directories
target_include_directories(telemetry-core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Compiler warnings
if(MSVC)
    target_compile_options(telemetry-core PRIVATE /W4)
else()
    target_compile_options(telemetry-core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Link libraries
target_link_libraries(telemetry-core PUBLIC
    Crow::Crow
    pthread
)

# Installation
install(TARGETS telemetry-core
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Install headers
install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# Create telemetry-server executable
add_executable(telemetry-server
    src/main.cpp
)

target_link_libraries(telemetry-server PRIVATE
    telemetry-core
)

# Install telemetry-server executable
install(TARGETS telemetry-server
    RUNTIME DESTINATION bin
)

# Print configuration summary
message(STATUS "=== Telemetry Core Configuration ===")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Crow Library: ${Crow_FOUND}")
message(STATUS "====================================")
