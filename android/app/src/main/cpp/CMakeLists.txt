cmake_minimum_required(VERSION 3.22.1)

project("regroove")

# Speed up builds with parallel compilation
include(ProcessorCount)
ProcessorCount(N)
if(NOT N EQUAL 0)
    set(CMAKE_BUILD_PARALLEL_LEVEL ${N})
endif()

# Try to use ccache if available
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    message(STATUS "Using ccache for faster rebuilds")
endif()

# Shared source directories
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../)
set(SDL3_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../SDL3)
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../regroovelizer/imgui)
set(RFX_ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../rfx/engine)
set(RFX_SYNTH_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../rfx/synth)
set(RFX_UI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../rfx/ui)
set(RFX_MIDI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../rfx/midi)
set(RFX_COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../rfx/common)
set(RFX_EFFECTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../rfx/effects)
set(LIBOPENMPT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libopenmpt)

# SDL3 configuration
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
add_subdirectory(${SDL3_DIR} SDL3 EXCLUDE_FROM_ALL)

# LibOpenMPT configuration
set(LIBOPENMPT_INCLUDE_DIR ${LIBOPENMPT_DIR}/include)
set(LIBOPENMPT_LIB_DIR ${LIBOPENMPT_DIR}/${ANDROID_ABI})

# Check if libopenmpt exists
if(NOT EXISTS ${LIBOPENMPT_LIB_DIR}/libopenmpt.so)
    message(WARNING "
================================================================================
LibOpenMPT NOT FOUND for ${ANDROID_ABI}!
Expected location: ${LIBOPENMPT_LIB_DIR}/libopenmpt.so

Please download or build libopenmpt for Android and place it in:
  ${LIBOPENMPT_DIR}/arm64-v8a/libopenmpt.so
  ${LIBOPENMPT_DIR}/armeabi-v7a/libopenmpt.so
  ${LIBOPENMPT_DIR}/include/libopenmpt/libopenmpt.h

See ${LIBOPENMPT_DIR}/README.md for instructions.
================================================================================
")
endif()

# Add libopenmpt as imported library
add_library(openmpt SHARED IMPORTED)
set_target_properties(openmpt PROPERTIES
    IMPORTED_LOCATION ${LIBOPENMPT_LIB_DIR}/libopenmpt.so
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SRC_DIR}
    ${SDL3_DIR}/include
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${RFX_ENGINE_DIR}
    ${RFX_SYNTH_DIR}
    ${RFX_UI_DIR}
    ${RFX_MIDI_DIR}
    ${RFX_COMMON_DIR}
    ${RFX_EFFECTS_DIR}
    ${LIBOPENMPT_INCLUDE_DIR}
)

# ImGui sources
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# RFX synth library sources (envelope generator)
set(RFX_SYNTH_SOURCES
    ${RFX_SYNTH_DIR}/synth_envelope.c
    ${RFX_SYNTH_DIR}/synth_utils.c
)

# RFX effects library sources
set(RFX_EFFECTS_SOURCES
    ${RFX_EFFECTS_DIR}/fx_delay.c
    ${RFX_EFFECTS_DIR}/fx_distortion.c
    ${RFX_EFFECTS_DIR}/fx_filter.c
    ${RFX_EFFECTS_DIR}/fx_eq.c
    ${RFX_EFFECTS_DIR}/fx_compressor.c
    ${RFX_EFFECTS_DIR}/fx_reverb.c
)

find_library(log-lib log)
find_library(android-lib android)

# Main library for Android
add_library(main SHARED
    ${SRC_DIR}/main-gui.cpp
    ${SRC_DIR}/midi_handler_android.cpp
    ${SRC_DIR}/midi_android_compat.cpp
    ${RFX_ENGINE_DIR}/regroove_engine.c
    ${SRC_DIR}/regroove_common.c
    ${SRC_DIR}/regroove_metadata.c
    ${SRC_DIR}/regroove_performance.c
    ${SRC_DIR}/regroove_phrase.c
    ${SRC_DIR}/regroove_effects.c
    ${SRC_DIR}/audio_input.c
    ${SRC_DIR}/input_mappings.c
    ${SRC_DIR}/midi_sysex.c
    ${SRC_DIR}/midi_mmc.c
    ${SRC_DIR}/lcd.c
    ${IMGUI_SOURCES}
    ${RFX_SYNTH_SOURCES}
    ${RFX_EFFECTS_SOURCES}
)

target_compile_definitions(main PRIVATE
    __ANDROID__
    ANDROID_APP_PACKAGE=nl_gbraad_regroove
    MIDI_LOG_TAG="Regroove-Native"
)

set_target_properties(main PROPERTIES
    LINK_FLAGS "-Wl,--export-dynamic"
)

target_link_libraries(main
    SDL3::SDL3
    openmpt
    ${log-lib}
    GLESv3
    ${android-lib}
    m
)
