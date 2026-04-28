cmake_minimum_required(VERSION 3.14)
project(cad-analyser LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_path(LIBREDWG_INCLUDE_DIR dwg_api.h)
find_library(LIBREDWG_LIBRARY NAMES redwg libredwg)

if(NOT LIBREDWG_INCLUDE_DIR OR NOT LIBREDWG_LIBRARY)
    message(FATAL_ERROR "LibreDWG not found. Install it or set CMAKE_PREFIX_PATH.")
endif()

message(STATUS "LibreDWG include: ${LIBREDWG_INCLUDE_DIR}")
message(STATUS "LibreDWG library: ${LIBREDWG_LIBRARY}")

add_executable(cad-analyser
    src/main.cpp
    src/dwg_parser.cpp
    src/entity_extractors.cpp
    src/json_builder.cpp
    src/text_utils.cpp
)

target_include_directories(cad-analyser PRIVATE include ${LIBREDWG_INCLUDE_DIR})
target_link_libraries(cad-analyser ${LIBREDWG_LIBRARY})

if(WIN32)
    target_compile_definitions(cad-analyser PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

install(TARGETS cad-analyser RUNTIME DESTINATION bin)
