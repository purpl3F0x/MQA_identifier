cmake_minimum_required(VERSION 3.12)
project(MQA_identifier)

set(CMAKE_CXX_STANDARD 17)

if (NOT (MSVC))
    set(CMAKE_CXX_FLAGS "-O2 -fpermissive")
else (NOT (MSVC))
    set(CMAKE_CXX_FLAGS "/O2")
    STRING(REPLACE "/O2" "/Od" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
endif ()

find_library(FLAC++_LIBRARIES NAMES FLAC++ FLAC)
find_library(ogg NAMES ogg)

add_executable(${PROJECT_NAME} main.cc)
target_link_libraries(${PROJECT_NAME} FLAC++ FLAC ogg)
