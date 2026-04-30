cmake_minimum_required(VERSION 3.16)
project(telemetry-frontend)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 packages
# No Charts module needed - using Canvas for lightweight charting
find_package(Qt6 REQUIRED COMPONENTS Core Qml Quick WebSockets)

# Set up include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Collect source files
set(SOURCES
    src/main.cpp
    src/models/TelemetryPoint.cpp
    src/models/ErrorEntry.cpp
    src/models/DeviceData.cpp
    src/websocket/WebSocketClient.cpp
    src/viewmodels/TelemetryViewModel.cpp
)

# Collect header files
set(HEADERS
    include/frontend/models/TelemetryPoint.h
    include/frontend/models/ErrorEntry.h
    include/frontend/models/DeviceData.h
    include/frontend/websocket/WebSocketClient.h
    include/frontend/viewmodels/TelemetryViewModel.h
)

# Enable automatic MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Create executable target
add_executable(telemetry-dashboard
    ${SOURCES}
    ${HEADERS}
)

# Link Qt libraries
target_link_libraries(telemetry-dashboard PRIVATE
    Qt6::Core
    Qt6::Qml
    Qt6::Quick
    Qt6::WebSockets
)

# Set output directory
set_target_properties(telemetry-dashboard PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/frontend
)

# Configure QML resources
set(QML_FILES
    qml/Main.qml
    qml/DeviceView.qml
    qml/TelemetryChartView.qml
    qml/ErrorView.qml
)

qt_add_resources(telemetry-dashboard "qml_resources"
    PREFIX "/"
    FILES ${QML_FILES}
)

# Install target
install(TARGETS telemetry-dashboard
    RUNTIME DESTINATION bin
)

# Print configuration summary
message(STATUS "==========================================")
message(STATUS "Telemetry Frontend Configuration:")
message(STATUS "  Qt Version: ${Qt6_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Output Directory: ${CMAKE_BINARY_DIR}/frontend")
message(STATUS "==========================================")
