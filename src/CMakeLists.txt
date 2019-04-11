cmake_minimum_required(VERSION 3.8)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
project(cellbowl)
set(SRCS main.c cell.c draw.c graph.c)
find_package(SDL2 REQUIRED)
add_executable(cellbowl ${SRCS})
target_link_libraries(cellbowl ${SDL2_LIBRARIES} SDL2_ttf -lm)

