cmake_minimum_required(VERSION 3.10)
project(tuxniffer)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# Add include directories
include_directories(include)

# Add source directory
set(SRCDIR src)
file(GLOB SOURCES "${SRCDIR}/*.cpp")

# Add main source file (now inside src directory)
set(MAIN_SOURCE src/main.cpp)

# Check if DEBUG is set
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()

# Add the executable
add_executable(tuxniffer ${SOURCES} ${MAIN_SOURCE})

# Set output directories
set_target_properties(tuxniffer PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# Set the object files output directory (optional, usually CMake handles this internally)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
