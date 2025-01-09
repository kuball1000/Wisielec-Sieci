cmake_minimum_required(VERSION 3.10)

# Set project name
project(SFMLApplication)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find SFML library
find_package(SFML 2.5 COMPONENTS system window graphics REQUIRED)

# Add sources
set(SOURCES
    main.cpp
    Application.cpp
    Resources.cpp
    Widok_Login.cpp
    Widok_Choice.cpp
    Widok_Game.cpp
)

set(HEADERS
    Application.h
    Resources.h
    Widok_Login.h
    Widok_Choice.h
    Widok_Game.h
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link SFML libraries
target_link_libraries(${PROJECT_NAME} sfml-system sfml-window sfml-graphics)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})