cmake_minimum_required(VERSION 3.31)
project(Mandelbrot)

set(CMAKE_CXX_STANDARD 20)

find_package(SFML 2.6.2 COMPONENTS system window graphics network REQUIRED)

add_executable(Mandelbrot src/main.cpp)
target_link_libraries(Mandelbrot sfml-system sfml-window sfml-graphics sfml-network)