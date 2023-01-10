cmake_minimum_required(VERSION 3.15)
project(nanovg)

file(
    GLOB
    SRC
    "*.h"
    "*.c"
)

add_library(${PROJECT_NAME} ${SRC})
