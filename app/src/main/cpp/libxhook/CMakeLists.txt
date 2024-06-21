cmake_minimum_required(VERSION 3.22.1)

file(GLOB SOURCE *.c)
add_library(xhook SHARED ${SOURCE}) #=>libcount.so

find_library(log-lib log)

target_link_libraries(xhook ${log-lib})

target_include_directories(canary PUBLIC ${SOURCE})