cmake_minimum_required(VERSION 3.15)
project(gui)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(
	GLOB
	SRC
	"*.h"
	"*.cpp"

	"texture_nodes/*.h"
	"texture_nodes/*.cpp"
)

file(
	GLOB
	FONTS
	"../fonts/*.ttf"
)

MESSAGE(STATUS ${FONTS})

# Dependencies
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glad)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/nanovg)

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} PRIVATE opengl32 glad nanovg)

# Copy fonts to output dir
file(COPY ${FONTS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/fonts)
