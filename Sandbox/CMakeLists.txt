include(${CMAKE_SOURCE_DIR}/cmake/BuildTests.cmake)
add_executable(Sandbox main.cpp)
linkEngineLibraries(Sandbox)

if(WIN32)
    set_target_properties(Sandbox PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()