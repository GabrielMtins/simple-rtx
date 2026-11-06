cmake_minimum_required(VERSION 3.16)

project(simple-rtx)

file(GLOB_RECURSE all_SRCS
		"${PROJECT_SOURCE_DIR}/src/*.c"
        )
	
file(GLOB cjson_SRCS "${PROJECT_SOURCE_DIR}/external/cjson/cJSON.c")

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(OpenMP QUIET COMPONENTS C)

# Executável
add_executable(${PROJECT_NAME}
	${all_SRCS}
	${cjson_SRCS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	m
)

if(OpenMP_C_FOUND)
	target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_C)
	message(STATUS "Linking OpenMP...")
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
	include/
	external/cglm/include/
	external/stb/
	external/cjson/
)

# Flags de compilação (opcional, mas recomendado)
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall -Wextra -Wpedantic
)
