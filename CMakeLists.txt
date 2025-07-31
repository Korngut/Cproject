cmake_minimum_required(VERSION 3.20)
project(C)

set(CMAKE_WIN32_EXECUTABLE OFF)

add_executable(C
        main.c
        preproc/preproc.c
        firstPass/firstPass.c)

