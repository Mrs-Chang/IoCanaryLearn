cmake_minimum_required(VERSION 3.22.1)

file(GLOB COMM_SOURCE comm/*.cpp)
file(GLOB CORE_SOURCE core/*.cpp)
file(GLOB DETECTOR_SOURCE detector/*.cpp)
file(GLOB MD5_SOURCE md5/*.cpp)

set(SOURCE_FILES ${COMM_SOURCE} ${CORE_SOURCE} ${DETECTOR_SOURCE} ${MD5_SOURCE})

add_library(canary SHARED ${SOURCE_FILES}) #=>libcanary.so

target_include_directories(canary PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/comm ${CMAKE_CURRENT_SOURCE_DIR}/core ${CMAKE_CURRENT_SOURCE_DIR}/detector ${CMAKE_CURRENT_SOURCE_DIR}/md5)