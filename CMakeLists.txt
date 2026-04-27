cmake_minimum_required(VERSION 3.10)
project("giraphics" VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#include helper scripts
include(cmake/ImportDependencies.cmake)
importDependencies()

#Engine Static Library
add_subdirectory(Engine)

#Sandbox test Application
add_subdirectory(Sandbox)
