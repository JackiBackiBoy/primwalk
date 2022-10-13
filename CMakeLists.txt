cmake_minimum_required(VERSION 3.20.0)

if (WIN32)
  set(CMAKE_CXX_COMPILER "C:/Users/TheGa/mingw64/bin/g++.exe")
  set(CMAKE_C_COMPILER "C:/Users/TheGa/mingw64/bin/gcc.exe")
endif()

project(fz)

add_subdirectory(fzui)
add_subdirectory(fzcoach)