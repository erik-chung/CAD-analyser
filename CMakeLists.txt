cmake_minimum_required(VERSION 3.14)
project(cad-analyser LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBREDWG REQUIRED IMPORTED_TARGET libredwg)

add_executable(cad-analyser
    src/main.cpp
    src/dwg_parser.cpp
    src/entity_extractors.cpp
    src/json_builder.cpp
    src/text_utils.cpp
)

target_include_directories(cad-analyser PRIVATE include)
target_link_libraries(cad-analyser PkgConfig::LIBREDWG)

if(WIN32)
    target_compile_definitions(cad-analyser PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

install(TARGETS cad-analyser RUNTIME DESTINATION bin)
