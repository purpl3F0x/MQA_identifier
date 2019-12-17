cmake_minimum_required(VERSION 3.12)
project(MQA_identifier)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/libs)

if (NOT (MSVC))
    set(CMAKE_CXX_FLAGS "-O3 -Wall -march=native -ffast-math")

else (NOT (MSVC))
    set(CMAKE_CXX_FLAGS "/O2")
    STRING(REPLACE "/O2" "/Od" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
endif ()

set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG")

find_library(FLAC++_LIBRARIES NAMES FLAC++ FLAC)

add_executable(${PROJECT_NAME} main.cc)
target_link_libraries(${PROJECT_NAME} FLAC++ FLAC)