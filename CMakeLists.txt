cmake_minimum_required(VERSION 3.15)
project(regroove VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add /usr/local to search paths (only for native Linux builds)
if(NOT CMAKE_CROSSCOMPILING AND NOT WIN32 AND NOT ANDROID)
    set(CMAKE_PREFIX_PATH "/usr/local;${CMAKE_PREFIX_PATH}")
    list(APPEND CMAKE_LIBRARY_PATH /usr/local/lib)
    list(APPEND CMAKE_INCLUDE_PATH /usr/local/include)
    set(ENV{PKG_CONFIG_PATH} "/usr/local/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
endif()

# Windows cross-compilation: set PKG_CONFIG_LIBDIR to prevent finding Linux packages
if(WIN32 AND CMAKE_CROSSCOMPILING)
    set(ENV{PKG_CONFIG_LIBDIR} "/usr/x86_64-w64-mingw32/lib/pkgconfig:/usr/x86_64-w64-mingw32/sys-root/mingw/lib/pkgconfig")
endif()

# RFX library directories
set(RFX_SYNTH_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../rfx/synth)
set(RFX_UI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../rfx/ui)
set(RFX_MIDI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../rfx/midi)
set(RFX_COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../rfx/common)
set(RFX_EFFECTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../rfx/effects)
set(RFX_TINYFILEDIALOGS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../rfx/tinyfiledialogs)

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

# Platform detection
if(ANDROID)
    set(PLATFORM_ANDROID ON)
elseif(CMAKE_CROSSCOMPILING AND WIN32)
    set(PLATFORM_WINDOWS_CROSS ON)
elseif(WIN32)
    set(PLATFORM_WINDOWS ON)
else()
    set(PLATFORM_NATIVE ON)
endif()

# Find required packages
find_package(PkgConfig REQUIRED)

# SDL3 configuration
if(ANDROID)
    # Android: Build SDL3 from source (using regroovelizer's SDL3 submodule)
    set(SDL3_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SDL3)
    set(SDL_SHARED ON CACHE BOOL "" FORCE)
    set(SDL_STATIC OFF CACHE BOOL "" FORCE)
    add_subdirectory(${SDL3_DIR} SDL3_build EXCLUDE_FROM_ALL)
else()
    # Desktop (Linux/Windows): Use pkg-config to find SDL3
    pkg_check_modules(SDL3 REQUIRED sdl3)
    message(STATUS "Using system SDL3: ${SDL3_LIBRARIES}")
endif()

# Find OpenGL
if(NOT PLATFORM_ANDROID)
    find_package(OpenGL REQUIRED)
else()
    # Android uses GLES
    set(OPENGL_LIBRARIES GLESv2)
endif()

# Find OpenMPT and RtMIDI (not on Android)
if(NOT PLATFORM_ANDROID)
    pkg_check_modules(OPENMPT REQUIRED libopenmpt)
    pkg_check_modules(RTMIDI REQUIRED rtmidi)
endif()

# ImGui sources
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl2.cpp
)

# Console version with MIDI (Unix only - uses termios)
if(NOT WIN32 AND NOT PLATFORM_ANDROID)
    add_executable(regroove-tui
        main-tui.c
        regroove_engine.c
        regroove_common.c
        regroove_metadata.c
        regroove_performance.c
        regroove_phrase.c
        regroove_effects.c
        midi.c
        midi_output.c
        input_mappings.c
        midi_sysex.c
        midi_mmc.c
    )

    target_include_directories(regroove-tui PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SDL3_INCLUDE_DIRS}
        ${OPENMPT_INCLUDE_DIRS}
        ${RTMIDI_INCLUDE_DIRS}
        ${RFX_COMMON_DIR}
    )

    target_link_libraries(regroove-tui PRIVATE
        ${SDL3_LDFLAGS}
        ${OPENMPT_LIBRARIES}
        ${RTMIDI_LIBRARIES}
        m
    )

    target_compile_options(regroove-tui PRIVATE
        ${OPENMPT_CFLAGS_OTHER}
        ${RTMIDI_CFLAGS_OTHER}
    )
endif()

# Desktop-only sources (file dialogs)
if(NOT ANDROID)
    set(DESKTOP_SOURCES ${RFX_TINYFILEDIALOGS_DIR}/tinyfiledialogs.c)
endif()

# GUI version (desktop and Android)
add_executable(regroove-gui
    main-gui.cpp
    regroove_engine.c
    regroove_common.c
    regroove_metadata.c
    regroove_performance.c
    regroove_phrase.c
    regroove_effects.c
    audio_input.c
    midi.c
    midi_output.c
    input_mappings.c
    midi_sysex.c
    midi_mmc.c
    lcd.c
    ${IMGUI_SOURCES}
    ${RFX_SYNTH_SOURCES}
    ${RFX_EFFECTS_SOURCES}
    ${DESKTOP_SOURCES}
)

target_include_directories(regroove-gui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${RFX_SYNTH_DIR}
    ${RFX_UI_DIR}
    ${RFX_MIDI_DIR}
    ${RFX_COMMON_DIR}
    ${RFX_EFFECTS_DIR}
    ${RFX_TINYFILEDIALOGS_DIR}
)

# Add SDL3 include directories (platform-specific)
if(ANDROID)
    target_include_directories(regroove-gui PRIVATE ${SDL3_DIR}/include)
else()
    target_include_directories(regroove-gui PRIVATE ${SDL3_INCLUDE_DIRS})
endif()

# Platform-specific configuration
if(PLATFORM_ANDROID)
    target_compile_definitions(regroove-gui PRIVATE
        IMGUI_IMPL_OPENGL_ES2
        __ANDROID__
    )
    target_link_libraries(regroove-gui PRIVATE
        SDL3::SDL3
        ${OPENGL_LIBRARIES}
        log
        android
        m
    )
else()
    target_include_directories(regroove-gui PRIVATE
        ${OPENMPT_INCLUDE_DIRS}
        ${RTMIDI_INCLUDE_DIRS}
    )
    target_link_libraries(regroove-gui PRIVATE
        ${SDL3_LDFLAGS}
        ${OPENMPT_LIBRARIES}
        ${RTMIDI_LIBRARIES}
    )
endif()

# Platform-specific libraries
if(WIN32)
    target_link_libraries(regroove-gui PRIVATE
        opengl32
        winmm
        imm32
        version
        setupapi
        z
    )
    # Force console subsystem on Windows so app returns to console when closed
    if(MSVC)
        set_target_properties(regroove-gui PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
    elseif(MINGW)
        set_target_properties(regroove-gui PROPERTIES LINK_FLAGS "-mconsole")
    endif()
elseif(NOT PLATFORM_ANDROID)
    target_link_libraries(regroove-gui PRIVATE
        GL
        dl
        pthread
        m
    )
endif()

# Installation rules
if(WIN32)
    install(TARGETS regroove-gui
        RUNTIME DESTINATION bin
    )
elseif(NOT PLATFORM_ANDROID)
    install(TARGETS regroove-tui regroove-gui
        RUNTIME DESTINATION bin
    )
endif()
