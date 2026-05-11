# Projects Settings

cmake_minimum_required(VERSION 3.4...3.28 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project("giraphics" VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the directory where your custom modules are located
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(ImportDependencies)

importDependencies()

ADD_SUBDIRECTORY(Engine)

include(BuildTests)
ADD_SUBDIRECTORY(Sandbox)
