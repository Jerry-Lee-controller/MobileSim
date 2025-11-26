cmake_minimum_required(VERSION 3.10)
project(flightsim_console LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(flightsim src/main.cpp)

if (MSVC)
  target_compile_options(flightsim PRIVATE /W4)
else()
  target_compile_options(flightsim PRIVATE -Wall -Wextra -pedantic)
endif()
