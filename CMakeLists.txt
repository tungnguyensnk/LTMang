cmake_minimum_required(VERSION 3.22)
project(LTMang C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS -pthread)

add_executable(LTMang lab10/main.c)
