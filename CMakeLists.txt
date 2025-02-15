cmake_minimum_required(VERSION 3.15)

# ensure dependencies are rebuilt whenever URL changes
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

cmake_policy(SET CMP0091 NEW)

project("gmdinput" C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include(GenerateExportHeader)
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

set(MAX_GAMEPADS 16 CACHE STRING "Max gamepad count")

if(NOT WIN32)
	message(FATAL_ERROR "only targets windows")
endif()

add_library(sdlmapping "sdlmapping/sdlmapping.c")
target_include_directories(sdlmapping PUBLIC "sdlmapping")

add_executable(sdlmapping-gen "tool/sdlmapping-gen.c")
target_link_libraries(sdlmapping-gen PUBLIC sdlmapping)

FetchContent_Declare(sdlgamecontrollerdb URL https://github.com/mdqinc/SDL_GameControllerDB/archive/refs/heads/master.zip)
FetchContent_MakeAvailable(sdlgamecontrollerdb)

FetchContent_Declare(minhook URL https://github.com/TsudaKageyu/minhook/archive/c1a7c3843bd1a5fe3eb779b64c0d823bca3dc339.zip)
FetchContent_MakeAvailable(minhook)

add_custom_command(
  OUTPUT "${CMAKE_BINARY_DIR}/include_gen/sdlmappings.inc.h"
  COMMAND "$<TARGET_FILE:sdlmapping-gen>" header "${sdlgamecontrollerdb_SOURCE_DIR}/gamecontrollerdb.txt" "${CMAKE_BINARY_DIR}/include_gen/sdlmappings.inc.h"
  DEPENDS "$<TARGET_FILE:sdlmapping-gen>" "${sdlgamecontrollerdb_SOURCE_DIR}/gamecontrollerdb.txt"
)

add_library("${PROJECT_NAME}" SHARED "gmdinput/gmdinput.c" "${CMAKE_BINARY_DIR}/include_gen/sdlmappings.inc.h")
target_include_directories("${PROJECT_NAME}" PUBLIC "gmdinput" "${CMAKE_BINARY_DIR}/include_gen")
target_link_libraries("${PROJECT_NAME}" PUBLIC dinput8 dxguid sdlmapping minhook)
target_compile_definitions(${PROJECT_NAME} PUBLIC MAX_GAMEPADS=${MAX_GAMEPADS})
set_property(TARGET "${PROJECT_NAME}" PROPERTY PREFIX "")
generate_export_header("${PROJECT_NAME}" EXPORT_FILE_NAME "${CMAKE_BINARY_DIR}/include_gen/${PROJECT_NAME}_export.h")

add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy 
     "$<TARGET_FILE:${PROJECT_NAME}>"
    "${CMAKE_CURRENT_SOURCE_DIR}/tool/gms/extensions/${PROJECT_NAME}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
)
