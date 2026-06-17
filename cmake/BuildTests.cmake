MACRO(SETUP_EXAMPLE APP_NAME)

set(GAME_ENGINE_NAME MyEngine)

project(${APP_NAME} VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB_RECURSE ${APP_NAME}_source_files
    ${CMAKE_CURRENT_LIST_DIR}/${APP_NAME}/*.[ch]pp
)

add_executable(${APP_NAME}
    ${${APP_NAME}_source_files}
)
target_include_directories(${APP_NAME} PUBLIC
    ${Vulkan_INCLUDE_DIRS}
)

target_include_directories(${APP_NAME} PRIVATE ${CMAKE_SOURCE_DIR})
target_compile_definitions(${APP_NAME} PRIVATE ASSETS_DIR_PATH="${CMAKE_SOURCE_DIR}/Engine/Assets/")
message(STATUS "Using MyEngine target for ${APP_NAME}....")

target_link_libraries(${APP_NAME} PRIVATE
    ${GAME_ENGINE_NAME}
    glfw
    ${Vulkan_LIBRARY}
)

ENDMACRO(SETUP_EXAMPLE)