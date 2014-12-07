CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_C_FLAGS "-Wall")
PROJECT(sudoku)

# Recursively build components
ADD_SUBDIRECTORY(solver)
