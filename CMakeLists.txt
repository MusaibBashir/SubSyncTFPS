cmake_minimum_required(VERSION 3.15)
project(alass_cpp VERSION 1.0.0 LANGUAGES CXX)

# Enforce modern C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compiler warnings to catch bugs early
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic -O3)
endif()

# Tell CMake where our header files live
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party)

# For now, we will compile everything into a single executable
# As we build the src/ files, we will add them here
add_executable(alass-cpp 
    app/main.cpp
    src/audio_tap.cpp
    src/srt_parser.cpp
    src/fft_math.cpp
)