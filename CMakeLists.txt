# Project Info

cmake_minimum_required(VERSION 3.6 FATAL_ERROR)
cmake_policy(VERSION 3.6)
project(CrossWindowImGui
        VERSION 1.0.0.0
        LANGUAGES C CXX
)

# =============================================================

# Options

set(XGFX_API AUTO CACHE STRING "The graphics API you're targeting, defaults to VULKAN, can be VULKAN, OPENGL, DIRECTX12, DIRECTX11, METAL, or NONE.")
set_property(
    CACHE
    XGFX_API PROPERTY
    STRINGS VULKAN OPENGL DIRECTX12 DIRECTX11 METAL
)

if(XGFX_API STREQUAL "VULKAN")
    set(XGFX_API_PATH "Vulkan")
elseif(XGFX_API STREQUAL "OPENGL")
    set(XGFX_API_PATH "OpenGL")
elseif(XGFX_API STREQUAL "DIRECTX12")
    set(XGFX_API_PATH "DirectX12")
elseif(XGFX_API STREQUAL "DIRECTX11")
    set(XGFX_API_PATH "DirectX11")
elseif(XGFX_API STREQUAL "METAL")
    set(XGFX_API_PATH "Metal")
else()
    message( SEND_ERROR "XGFX_API can only be either VULKAN, OPENGL, DIRECTX12, DIRECTX11, METAL.")
endif()

# =============================================================

# ImGui
message(STATUS "Installing ImGUI via submodule")
set(
  IMGUI_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/imgui.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/imgui_draw.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/imgui_tables.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/imgui_widgets.cpp
)
add_library(
  ImGui
  "${IMGUI_SOURCES}"
)

target_include_directories(
  ImGui
  PUBLIC "external/imgui"
)
target_compile_definitions(ImGui PUBLIC IMGUI_DISABLE_OBSOLETE_FUNCTIONS)
target_compile_definitions(ImGui PUBLIC IMGUI_DISABLE_DEMO_WINDOWS)

# =============================================================

# Sources

file(GLOB_RECURSE FILE_SOURCES RELATIVE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CrossWindow/ImGui/ImGui.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CrossWindow/ImGui/ImGuiManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CrossWindow/ImGui/ImGuiManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CrossWindow/ImGui/${XGFX_API_PATH}.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CrossWindow/ImGui/${XGFX_API_PATH}.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CrossWindow/ImGui/${XGFX_API_PATH}.h
)

# Solution Filters
foreach(source IN LISTS FILE_SOURCES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    string(REPLACE "src" "" source_path_final "${source_path_msvc}")
    source_group("${source_path_final}" FILES "${source}")
endforeach()

# =============================================================

# Finalize Library

add_library(
    CrossWindowImGui
    ${FILE_SOURCES}
)

target_link_libraries(
    CrossWindowImGui
    ImGui
)

add_dependencies(
    CrossWindowImGui
    ImGui
)

target_include_directories(
    CrossWindowImGui PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/)


if(NOT( XGFX_API STREQUAL "NONE" ))
    message( STATUS "Using the " ${XGFX_API_PATH} " graphics API with CrossWindow")
    target_compile_definitions(CrossWindowImGui PUBLIC XGFX_${XGFX_API}=1)
else()
    message( SEND_ERROR "ImGui requires a graphics API and does not run in headless mode. This project is empty.")
endif()

