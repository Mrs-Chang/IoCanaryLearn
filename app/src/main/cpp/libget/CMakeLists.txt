cmake_minimum_required(VERSION 3.22.1)

file(GLOB SOURCE *.cpp)
add_library(get STATIC ${SOURCE}) #=>libget.a