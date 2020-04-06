cmake_minimum_required(VERSION 3.12)
project(MQA_identifier_gui)

# Required Qt settings
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(QT_USE_QTMAIN TRUE)

if (NOT (MSVC))
    set(CMAKE_CXX_FLAGS "-O2 -Wall -static -static-libstdc++ -static-libgcc")

else (NOT (MSVC))
    set(CMAKE_CXX_FLAGS "/O2")
    STRING(REPLACE "/O2" "/Od" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
endif ()

include_directories(${CMAKE_PARENT_LIST_FILE}/../mainwindow.h)

if (WIN32)
    set(GUI_TYPE WIN32)
elseif (APPLE)
    set(GUI_TYPE MACOSX_BUNDLE)
endif ()

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)


add_executable(${PROJECT_NAME} ${GUI_TYPE} ${CMAKE_PARENT_LIST_FILE}/../main.cpp ${CMAKE_PARENT_LIST_FILE}/../mainwindow.cpp ${CMAKE_PARENT_LIST_FILE}/../mainwindow.ui)
target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::Core)
