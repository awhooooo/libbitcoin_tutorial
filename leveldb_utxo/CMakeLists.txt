# macOS Ventura
cmake_minimum_required(VERSION 3.19)

# Set the project name
project(UTXO)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(
    /opt/homebrew/Cellar/leveldb/1.22.0/include
)

# Link directories
link_directories(
    /opt/homebrew/Cellar/leveldb/1.22.0/lib
)

# Source files
set(SOURCES
    utxo_set_navigation.cpp
    utxo_set_navigation_main.cpp
)

# Header files (not strictly necessary for CMake, but good for organization)
set(HEADERS
    utxo_set_navigation.hpp
)

# Add the executable
add_executable(UTXO ${SOURCES} ${HEADERS})

# Specify rpath to ensure the dynamic linker can find the LevelDB library at runtime
set_target_properties(UTXO PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "/opt/homebrew/Cellar/leveldb/1.22.0/lib"
    INSTALL_RPATH_USE_LINK_PATH TRUE
)

# Link the libraries
target_link_libraries(UTXO
    leveldb  # Link against LevelDB directly
    ${CMAKE_DL_LIBS}  # Link the dynamic library for dlopen
)

# Add compiler warnings
target_compile_options(UTXO PRIVATE 
    -Wno-deprecated-declarations)
