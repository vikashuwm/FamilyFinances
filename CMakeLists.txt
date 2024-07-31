cmake_minimum_required(VERSION 3.10)
project(FamilyFinances VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6
find_package(Qt6 COMPONENTS Widgets REQUIRED)

# Find nlohmann_json
find_package(nlohmann_json REQUIRED)

# Add subdirectories
add_subdirectory(bank)
add_subdirectory(ui)

# Include the resource file
qt6_add_resources(RESOURCES resources/resources.qrc)

# FamilyFinances executable
add_executable(FamilyFinances
    main.cpp
    ${RESOURCES}
)

target_include_directories(FamilyFinances PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/bank/include
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/include
)

target_link_libraries(FamilyFinances PRIVATE
    Bank
    UI
    Qt6::Widgets
    nlohmann_json::nlohmann_json
)

# Copy the QSS file to the build directory
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/FamilyFinances.qss ${CMAKE_CURRENT_BINARY_DIR}/FamilyFinances.qss COPYONLY)

# Ensure the necessary directory structure exists
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/resources)

# Create a custom target to copy resources
add_custom_target(copy_resources ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources
    ${CMAKE_CURRENT_BINARY_DIR}/resources
    COMMENT "Copying resources into binary directory"
)

# Ensure FamilyFinances depends on the copy_resources target
add_dependencies(FamilyFinances copy_resources)