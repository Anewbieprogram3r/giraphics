set(ENGINE_NAME MyEngine)

file(GLOB_RECURSE GRAPHICS_ABSTRACTION_LAYER_SRC_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
    ${CMAKE_CURRENT_LIST_DIR}/Graphics/*.[ch]pp
)

set(ALL_FILES
    ${GRAPHICS_ABSTRACTION_LAYER_SRC_FILES} 
    ${VULKAN_SRC_FILES}
)

# Create the executable (or library)
add_library(${ENGINE_NAME} ${ALL_FILES})

set_property(TARGET ${ENGINE_NAME}  PROPERTY CXX_STANDARD 20) 
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${ENGINE_NAME} PRIVATE -Wall ${DisableWarnings})
elseif (MSVC)
    target_compile_options(${ENGINE_NAME} PRIVATE /W3 /WX-)
endif()

target_include_directories(${ENGINE_NAME} SYSTEM PRIVATE ${PROJECT_SOURCE_DIR})

target_include_directories(${ENGINE_NAME} PUBLIC
    ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(${ENGINE_NAME} 
    ${Vulkan_LIBRARY}
)
