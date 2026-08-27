cmake_minimum_required(VERSION 3.10)
project(ipad_zygisk)

add_library(ipad_zygisk SHARED jni/module.cpp)

find_library(log-lib log)
target_link_libraries(ipad_zygisk ${log-lib})
