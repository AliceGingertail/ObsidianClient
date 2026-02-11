cmake_minimum_required(VERSION 3.16)

project(ObsidianClient VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Quick
    QuickControls2
    Network
)

# Sources
set(SOURCES
    src/main.cpp
    src/ApiClient.cpp
    src/WireGuardKeys.cpp
    src/ConfigManager.cpp
    src/VpnConnection.cpp
)

# Headers
set(HEADERS
    include/ApiClient.h
    include/WireGuardKeys.h
    include/ConfigManager.h
    include/VpnConnection.h
    include/KeyGenerator.h
)

# QML Resources
qt_add_resources(QML_RESOURCES qml/qml.qrc)

# Executable
qt_add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${QML_RESOURCES}
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
)

# Install
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
