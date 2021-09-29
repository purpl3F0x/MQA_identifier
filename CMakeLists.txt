cmake_minimum_required(VERSION 3.12)
project(MQA_identifier)

# for android build, unhide this              
# set(Boost_USE_STATIC_LIBS OFF)
# set(Boost_USE_DEBUG_LIBS OFF)   
# set(Boost_USE_RELEASE_LIBS ON)
# set(Boost_USE_MULTITHREADED ON)
# set(Boost_USE_STATIC_RUNTIME OFF)
# find_package(Boost 1.75.0 COMPONENTS date_time filesystem system )

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

# for android build, hide this
target_link_libraries(${PROJECT_NAME} FLAC++ FLAC ogg)

# for android build, unhide this
# target_link_libraries(${PROJECT_NAME} FLAC++ FLAC ogg ${Boost_LIBRARIES})
