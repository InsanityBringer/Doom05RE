cmake_minimum_required(VERSION 3.11)
project(Doom05)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

#hastily adapted from Chocolate Descent. 

set(GAME_SOURCES
	portsrc/d_demo.c
	portsrc/d_main.c
	portsrc/d_misc.c
	portsrc/doomdef.h
	portsrc/g_game.h
	portsrc/g_game.c
	portsrc/g_world.c
	portsrc/hmap_c.c
	portsrc/hmaphc_c.c
	portsrc/i_local.h
	portsrc/i_sdl.c
	portsrc/ir_sdl.c
	portsrc/m_menu.h
	portsrc/m_menu.c
	portsrc/p_actor.h
	portsrc/p_enemy.c
	portsrc/p_local.h
	portsrc/p_play.c
	portsrc/p_spec.c
	portsrc/p_spec.h
	portsrc/p_status.c
	portsrc/p_things.c
	portsrc/p_trace.c
	portsrc/p_user.c
	portsrc/r_data.h
	portsrc/r_game.c
	portsrc/r_init.c
	portsrc/r_lines.c
	portsrc/r_local.h
	portsrc/r_planes.c
	portsrc/r_sector.c
	portsrc/r_textur.c
	portsrc/r_things.c
	portsrc/s_sound.c
	portsrc/s_sound.h
	portsrc/sdl_gl.c
	portsrc/sdl_gl.h
	portsrc/sdl_key.h
	portsrc/states.c
	portsrc/states.h
	portsrc/v_munged.c
	portsrc/v_video.h
	portsrc/vmaphc_c.c
	portsrc/vmaph_c.c
	portsrc/vmapl_c.c
	portsrc/w_wad.c
	portsrc/w_wad.h
	portsrc/z_zone.c
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
