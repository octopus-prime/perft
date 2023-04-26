cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

project(HelloWorld CXX)

 if(MSVC)
	message(STATUS "compiler=msvc")
	add_compile_options(/Ox /arch:AVX2 /Gy)
else()
	message(STATUS "compiler=gcc")
	add_compile_options(-O3 -march=native)
endif()

file (GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ./src/*.cpp ./src/*.hpp)

add_executable(HelloWorld ${SOURCES})
