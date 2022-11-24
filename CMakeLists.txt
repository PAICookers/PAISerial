cmake_minimum_required(VERSION 3.12)

project(PAISerial_in_C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

include_directories(include)
aux_source_directory(./lib src)

add_executable(serial main.cpp ${src})