cmake_minimum_required(VERSION 3.22.1)

file(GLOB SOURCE comm/*.cpp)
file(GLOB SOURCE core/*.cpp)
add_library(canary SHARED ${SOURCE}) #=>libcanary.so