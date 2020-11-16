cmake_minimum_required(VERSION 3.11)
project(Doom05)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

#hastily adapted from Chocolate Descent. 

set(GAME_SOURCES
	src/d_demo.c
	src/d_main.c
	src/d_misc.c
	src/doomdata.h
	src/doomdef.h
	src/g_game.c
	src/g_world.c
	src/hmapc.c
	src/hmaph.c
	src/i_local.h
	src/i_sdl.c
	src/ir_sdl.c
	src/m_menu.c
	src/p_enemy.c
	src/p_local.h
	src/p_play.c
	src/p_spec.c
	src/p_status.c
	src/p_things.c
	src/p_trace.c
	src/p_user.c
	src/r_data.h
	src/r_game.c
	src/r_init.c
	src/r_lines.c
	src/r_local.h
	src/r_planes.c
	src/r_sector.c
	src/r_textur.c
	src/r_things.c
	src/s_sound.c
	src/s_sound.h
	src/sdl_gl.c
	src/sdl_gl.h
	src/sdl_key.h
	src/states.c
	src/states.h
	src/v_munged.c
	src/vmapc.c
	src/vmaph.c
	src/vmapl.c
	src/w_wad.c
	src/z_zone.c
)

set(GAME_LIBS
)

find_package(SDL2 REQUIRED)

include_directories(.)

if (SDL2_FOUND)
	include_directories(${SDL2_INCLUDE_DIRS})
	set(GAME_LIBS ${GAME_LIBS} ${SDL2_LIBRARIES})
    set(GAME_LIBS ${GAME_LIBS} ${SDL2_MAIN_LIBRARIES})
endif()

# Set up flags for MSVC
if (MSVC)
	set(CMAKE_CXX_FLAGS "/MP ${CMAKE_CXX_FLAGS}") # Use all cores for compilation
	set(CMAKE_CXX_FLAGS "/wd4267 ${CMAKE_CXX_FLAGS}") #warning C4267 conversion from 'size_t' to 'int', possible loss of data
	set(CMAKE_CXX_FLAGS "/wd4244 ${CMAKE_CXX_FLAGS}") #warning C4244: 'return': conversion from '__int64' to 'int', possible loss of data
	set(CMAKE_CXX_FLAGS "/wd4390 ${CMAKE_CXX_FLAGS}") #warning C4390: ';': empty controlled statement found; is this the intent?
	set(CMAKE_CXX_FLAGS "/wd4101 ${CMAKE_CXX_FLAGS}") #warning C4101: 'org_gamma': unreferenced local variable
	set(CMAKE_CXX_FLAGS "/wd4102 ${CMAKE_CXX_FLAGS}") #warning C4102: 'RePaintNewmenu4': unreferenced label
	set(CMAKE_CXX_FLAGS "/wd4018 ${CMAKE_CXX_FLAGS}") #warning C4018: '>': signed/unsigned mismatch
	set(CMAKE_CXX_FLAGS "/wd4715 ${CMAKE_CXX_FLAGS}") #warning C4715: 'get_num_faces': not all control paths return a value
	set(CMAKE_CXX_FLAGS "/wd4838 ${CMAKE_CXX_FLAGS}") #warning C4838: conversion from 'unsigned int' to 'long' requires a narrowing conversion
	set(CMAKE_CXX_FLAGS "/wd4309 ${CMAKE_CXX_FLAGS}") #warning C4309: 'argument': truncation of constant value
	add_definitions(-D_CRT_SECURE_NO_WARNINGS=1) # Suppress warning about insecure strxxx functions
endif()

add_executable(Doom05 WIN32 MACOSX_BUNDLE ${GAME_SOURCES})

target_link_libraries(Doom05 ${GAME_LIBS})

#source_group("2d" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/2d/.+")
