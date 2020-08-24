cmake_minimum_required(VERSION 3.8)

if((${CMAKE_VERSION} VERSION_EQUAL 3.9) OR (${CMAKE_VERSION} VERSION_GREATER 3.9))
	cmake_policy(SET CMP0069 NEW)
endif()

###########
# Options #
###########

option(JAPANESE "Enable the Japanese-language build (instead of the unofficial Aeon Genesis English translation)" OFF)
option(FIX_BUGS "Fix various bugs in the game" OFF)
option(DEBUG_SAVE "Re-enable the ability to drag-and-drop save files onto the window" OFF)
option(DOCONFIG "Compile a DoConfig clone tool - not useful for console ports" ON)

set(BACKEND_RENDERER "SDLTexture" CACHE STRING "Which renderer the game should use: 'OpenGL3' for an OpenGL 3.2 renderer, 'OpenGLES2' for an OpenGL ES 2.0 renderer, 'SDLTexture' for SDL2's hardware-accelerated Texture API, 'SDLSurface' for SDL2's software-rendered Surface API, 'Wii U' for the Wii U's hardware-accelerated GX2 API, or 'Software' for a handwritten software renderer")
set(BACKEND_AUDIO "SDL2" CACHE STRING "Which audio backend the game should use: 'SDL2', 'miniaudio', 'WiiU-Hardware', 'WiiU-Software', or 'Null'")
set(BACKEND_PLATFORM "SDL2" CACHE STRING "Which platform backend the game should use: 'SDL2', 'GLFW3', 'WiiU', or 'Null'")

option(LTO "Enable link-time optimisation" OFF)
option(PKG_CONFIG_STATIC_LIBS "On platforms with pkg-config, static-link the dependencies (good for Windows builds, so you don't need to bundle DLL files)" OFF)
option(MSVC_LINK_STATIC_RUNTIME "Link the static MSVC runtime library (Visual Studio only)" OFF)

option(FORCE_LOCAL_LIBS "Compile the built-in versions of SDL2, GLFW3, and FreeType instead of using the system-provided ones" OFF)


#########
# Setup #
#########

project(CSE2 LANGUAGES C CXX)

add_executable(CSE2 WIN32
	"src/ArmsItem.cpp"
	"src/ArmsItem.h"
	"src/Back.cpp"
	"src/Back.h"
	"src/Bitmap.cpp"
	"src/Bitmap.h"
	"src/Boss.cpp"
	"src/Boss.h"
	"src/BossAlmo1.cpp"
	"src/BossAlmo1.h"
	"src/BossAlmo2.cpp"
	"src/BossAlmo2.h"
	"src/BossBallos.cpp"
	"src/BossBallos.h"
	"src/BossFrog.cpp"
	"src/BossFrog.h"
	"src/BossIronH.cpp"
	"src/BossIronH.h"
	"src/BossLife.cpp"
	"src/BossLife.h"
	"src/BossOhm.cpp"
	"src/BossOhm.h"
	"src/BossPress.cpp"
	"src/BossPress.h"
	"src/BossTwinD.cpp"
	"src/BossTwinD.h"
	"src/BossX.cpp"
	"src/BossX.h"
	"src/BulHit.cpp"
	"src/BulHit.h"
	"src/Bullet.cpp"
	"src/Bullet.h"
	"src/Caret.cpp"
	"src/Caret.h"
	"src/CommonDefines.h"
	"src/Config.cpp"
	"src/Config.h"
	"src/Draw.cpp"
	"src/Draw.h"
	"src/Ending.cpp"
	"src/Ending.h"
	"src/Escape.cpp"
	"src/Escape.h"
	"src/Fade.cpp"
	"src/Fade.h"
	"src/File.cpp"
	"src/File.h"
	"src/Flags.cpp"
	"src/Flags.h"
	"src/Flash.cpp"
	"src/Flash.h"
	"src/Font.cpp"
	"src/Font.h"
	"src/Frame.cpp"
	"src/Frame.h"
	"src/Game.cpp"
	"src/Game.h"
	"src/Generic.cpp"
	"src/Generic.h"
	"src/GenericLoad.cpp"
	"src/GenericLoad.h"
	"src/Input.cpp"
	"src/Input.h"
	"src/KeyControl.cpp"
	"src/KeyControl.h"
	"src/Main.cpp"
	"src/Main.h"
	"src/Map.cpp"
	"src/Map.h"
	"src/MapName.cpp"
	"src/MapName.h"
	"src/MiniMap.cpp"
	"src/MiniMap.h"
	"src/Music.cpp"
	"src/Music.h"
	"src/MyChar.cpp"
	"src/MyChar.h"
	"src/MycHit.cpp"
	"src/MycHit.h"
	"src/MycParam.cpp"
	"src/MycParam.h"
	"src/NpcAct.h"
	"src/NpcAct000.cpp"
	"src/NpcAct020.cpp"
	"src/NpcAct040.cpp"
	"src/NpcAct060.cpp"
	"src/NpcAct080.cpp"
	"src/NpcAct100.cpp"
	"src/NpcAct120.cpp"
	"src/NpcAct140.cpp"
	"src/NpcAct160.cpp"
	"src/NpcAct180.cpp"
	"src/NpcAct200.cpp"
	"src/NpcAct220.cpp"
	"src/NpcAct240.cpp"
	"src/NpcAct260.cpp"
	"src/NpcAct280.cpp"
	"src/NpcAct300.cpp"
	"src/NpcAct320.cpp"
	"src/NpcAct340.cpp"
	"src/NpChar.cpp"
	"src/NpChar.h"
	"src/NpcHit.cpp"
	"src/NpcHit.h"
	"src/NpcTbl.cpp"
	"src/NpcTbl.h"
	"src/PixTone.cpp"
	"src/PixTone.h"
	"src/Profile.cpp"
	"src/Profile.h"
	"src/Random.cpp"
	"src/Random.h"
	"src/SelStage.cpp"
	"src/SelStage.h"
	"src/Shoot.cpp"
	"src/Shoot.h"
	"src/Sound.cpp"
	"src/Sound.h"
	"src/Stage.cpp"
	"src/Stage.h"
	"src/Star.cpp"
	"src/Star.h"
	"src/TextScr.cpp"
	"src/TextScr.h"
	"src/Title.cpp"
	"src/Title.h"
	"src/Triangle.cpp"
	"src/Triangle.h"
	"src/ValueView.cpp"
	"src/ValueView.h"
	"src/WindowsWrapper.h"
	"src/Music/Organya.cpp"
	"src/Music/Organya.h"
	"src/Backends/Audio.h"
	"src/Backends/Controller.h"
	"src/Backends/Misc.h"
	"src/Backends/Rendering.h"
)

list(APPEND CMAKE_MODULE_PATH
	"${CMAKE_SOURCE_DIR}/cmake"
)


###################
# Option handling #
###################

if(JAPANESE)
	set(BUILD_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/game_japanese")
	list(APPEND RESOURCES "BITMAP/pixel_jp.bmp")
	target_compile_definitions(CSE2 PRIVATE JAPANESE)
else()
	set(BUILD_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/game_english")
	list(APPEND RESOURCES "BITMAP/pixel.bmp")
endif()

if(FIX_BUGS)
	target_compile_definitions(CSE2 PRIVATE FIX_BUGS)
endif()

if(DEBUG_SAVE)
	target_compile_definitions(CSE2 PRIVATE DEBUG_SAVE)
endif()

if(PKG_CONFIG_STATIC_LIBS)
	target_link_options(CSE2 PRIVATE "-static")
endif()

if(LTO)
	include(CheckIPOSupported)

	check_ipo_supported(RESULT result)

	if(result)
		set_target_properties(CSE2 PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
	endif()
endif()

# This is messy as hell, and has been replaced by CMAKE_MSVC_RUNTIME_LIBRARY,
# but that's a very recent CMake addition, so we're still doing it this way for now
if(MSVC AND MSVC_LINK_STATIC_RUNTIME)
	# Statically-link the CRT (vcpkg static libs do this)
	foreach(flag_var CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
		if(${flag_var} MATCHES "/MD")
			string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
		endif()
	endforeach()
endif()

if(BACKEND_RENDERER MATCHES "SDLTexture")
	target_sources(CSE2 PRIVATE "src/Backends/Rendering/SDLTexture.cpp")
else()
	message(FATAL_ERROR "Invalid BACKEND_RENDERER selected")
endif()

if(BACKEND_AUDIO MATCHES "SDL2")
	target_sources(CSE2 PRIVATE
		"src/Backends/Audio/SDL2.cpp"
		"src/Backends/Audio/SoftwareMixer.cpp"
		"src/Backends/Audio/SoftwareMixer.h"
	)
elseif(BACKEND_AUDIO MATCHES "Null")
	target_sources(CSE2 PRIVATE
		"src/Backends/Audio/Null.cpp"
	)
else()
	message(FATAL_ERROR "Invalid BACKEND_AUDIO selected")
endif()

if(BACKEND_PLATFORM MATCHES "SDL2")
	target_sources(CSE2 PRIVATE
		"src/Backends/Controller/SDL2.cpp"
		"src/Backends/Platform/SDL2.cpp"
		"src/Backends/Shared/SDL2.h"
	)
elseif(BACKEND_PLATFORM MATCHES "Null")
	target_sources(CSE2 PRIVATE
		"src/Backends/Controller/Null.cpp"
		"src/Backends/Platform/Null.cpp"
	)
endif()

##########
# Tweaks #
##########

# Make some tweaks if we're using MSVC
if(MSVC)
	# Disable warnings that normally fire up on MSVC when using "unsafe" functions instead of using MSVC's "safe" _s functions
	target_compile_definitions(CSE2 PRIVATE _CRT_SECURE_NO_WARNINGS)

	# Make it so source files are recognized as UTF-8 by MSVC
	target_compile_options(CSE2 PRIVATE "/utf-8")

	# Use `main` instead of `WinMain`
	set_target_properties(CSE2 PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
endif()

# On Windows, we use native icons instead
if(NOT WIN32)
	list(APPEND RESOURCES "ICON/ICON_MINI.bmp")
endif()


##################
# Misc. settings #
##################

# Force strict C90
set_target_properties(CSE2 PROPERTIES
	C_STANDARD 90
	C_STANDARD_REQUIRED ON
	C_EXTENSIONS OFF
)

# Force strict C++11
set_target_properties(CSE2 PROPERTIES
	CXX_STANDARD 11
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

# Name debug builds "CSE2_debug", to distinguish them
set_target_properties(CSE2 PROPERTIES DEBUG_OUTPUT_NAME "CSE2_debug")

# Send executable to the build_en/build_jp directory
set_target_properties(CSE2 PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIRECTORY}
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BUILD_DIRECTORY}
	RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${BUILD_DIRECTORY}
	RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${BUILD_DIRECTORY}
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BUILD_DIRECTORY}
)


################
# Dependencies #
################

if(NOT FORCE_LOCAL_LIBS)
	find_package(PkgConfig QUIET)
endif()

if(BACKEND_PLATFORM MATCHES "GLFW3")
	if(NOT FORCE_LOCAL_LIBS)
		find_package(glfw3)

		if (PKG_CONFIG_FOUND)
			pkg_check_modules(glfw3 QUIET IMPORTED_TARGET glfw3)
		endif()
	endif()

	if(TARGET PkgConfig::glfw3)
		# pkg-config
		if (PKG_CONFIG_STATIC_LIBS)
			message(STATUS "Using system GLFW3 (pkg-config, static)")
			target_compile_options(CSE2 PRIVATE ${glfw3_STATIC_CFLAGS})
			target_link_libraries(CSE2 PRIVATE ${glfw3_STATIC_LDFLAGS})
		else()
			message(STATUS "Using system GLFW3 (pkg-config, dynamic)")
			target_compile_options(CSE2 PRIVATE ${glfw3_CFLAGS})
			target_link_libraries(CSE2 PRIVATE ${glfw3_LDFLAGS})
		endif()
	elseif(TARGET glfw)
		# CMake
		message(STATUS "Using system GLFW3 (CMake)")
		target_link_libraries(CSE2 PRIVATE glfw)
	else()
		# Compile it ourselves
		message(STATUS "Using local GLFW3")
		set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "" FORCE)
		set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
		set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "" FORCE)
		set(GLFW_INSTALL OFF CACHE INTERNAL "" FORCE)
		add_subdirectory("external/glfw" EXCLUDE_FROM_ALL)
		target_link_libraries(CSE2 PRIVATE glfw)
	endif()
endif()

if(BACKEND_PLATFORM MATCHES "SDL2" OR BACKEND_AUDIO MATCHES "SDL2")
	if(NOT FORCE_LOCAL_LIBS)
		find_package(SDL2 2.0.6)

		if (PKG_CONFIG_FOUND)
			pkg_check_modules(sdl2 QUIET IMPORTED_TARGET sdl2)
		endif()
	endif()

	if(TARGET PkgConfig::sdl2)
		# pkg-config
		if (PKG_CONFIG_STATIC_LIBS)
			message(STATUS "Using system SDL2 (pkg-config, static)")
			target_compile_options(CSE2 PRIVATE ${sdl2_STATIC_CFLAGS})
			target_link_libraries(CSE2 PRIVATE ${sdl2_STATIC_LDFLAGS})
		else()
			message(STATUS "Using system SDL2 (pkg-config, dynamic)")
			target_compile_options(CSE2 PRIVATE ${sdl2_CFLAGS})
			target_link_libraries(CSE2 PRIVATE ${sdl2_LDFLAGS})
		endif()
	elseif(TARGET SDL2::SDL2)
		# CMake-generated config (Arch, vcpkg, Raspbian)
		message(STATUS "Using system SDL2 (CMake, dynamic)")
		target_link_libraries(CSE2 PRIVATE SDL2::SDL2)
	elseif(TARGET SDL2::SDL2-static)
		# CMake-generated config (Arch, vcpkg, Raspbian)
		message(STATUS "Using system SDL2 (CMake, static)")
		target_link_libraries(CSE2 PRIVATE SDL2::SDL2-static)
	elseif(SDL2_FOUND)
		# Autotools-generated config (MSYS2)
		message(STATUS "Using system SDL2 (Autotools)")
		target_include_directories(CSE2 PRIVATE ${SDL2_INCLUDE_DIRS})
		target_link_libraries(CSE2 PRIVATE ${SDL2_LIBRARIES})
	else()
		# Compile it ourselves
		message(STATUS "Using local SDL2")
		set(SDL_SHARED_ENABLED_BY_DEFAULT OFF)
		if(MSVC)
				set(LIBC ON CACHE INTERNAL "" FORCE)	# Needed to prevent possible 'symbol already defined' errors
		endif()
		add_subdirectory("external/SDL2" EXCLUDE_FROM_ALL)
		target_link_libraries(CSE2 PRIVATE SDL2-static)
	endif()
endif()

if(NOT FORCE_LOCAL_LIBS)
	find_package(Freetype)

	if (PKG_CONFIG_FOUND)
		pkg_check_modules(freetype2 QUIET IMPORTED_TARGET freetype2)
	endif()
endif()

if(TARGET PkgConfig::freetype2)
	# pkg-config
	if (PKG_CONFIG_STATIC_LIBS)
		message(STATUS "Using system FreeType (pkg-config, static)")
		target_compile_options(CSE2 PRIVATE ${freetype2_STATIC_CFLAGS})
		target_link_libraries(CSE2 PRIVATE ${freetype2_STATIC_LDFLAGS})
	else()
		message(STATUS "Using system FreeType (pkg-config, dynamic)")
		target_compile_options(CSE2 PRIVATE ${freetype2_CFLAGS})
		target_link_libraries(CSE2 PRIVATE ${freetype2_LDFLAGS})
	endif()
elseif(FREETYPE_FOUND)
	message(STATUS "Using system FreeType (CMake)")
	target_include_directories(CSE2 PRIVATE ${FREETYPE_INCLUDE_DIRS})
	target_link_libraries(CSE2 PRIVATE ${FREETYPE_LIBRARIES})
else()
	# Compile it ourselves
	message(STATUS "Using local FreeType")
	if(FORCE_LOCAL_LIBS)
		set(CMAKE_DISABLE_FIND_PACKAGE_HarfBuzz ON CACHE INTERNAL "" FORCE)
		set(CMAKE_DISABLE_FIND_PACKAGE_ZLIB ON CACHE INTERNAL "" FORCE)
		set(CMAKE_DISABLE_FIND_PACKAGE_PNG ON CACHE INTERNAL "" FORCE)
		set(CMAKE_DISABLE_FIND_PACKAGE_BZip2 ON CACHE INTERNAL "" FORCE)
	endif()
	add_subdirectory("external/freetype" EXCLUDE_FROM_ALL)
	target_link_libraries(CSE2 PRIVATE freetype)
endif()

if(BACKEND_RENDERER MATCHES "OpenGL3")
	add_subdirectory("external/glad" EXCLUDE_FROM_ALL)
	target_link_libraries(CSE2 PRIVATE glad)
endif()

if(BACKEND_RENDERER MATCHES "OpenGLES2")
	find_package(OpenGLES2 REQUIRED)
	target_include_directories(CSE2 PRIVATE ${OPENGLES2_INCLUDE_DIR})
	target_link_libraries(CSE2 PRIVATE ${OPENGLES2_LIBRARIES})
endif()

if(BACKEND_RENDERER MATCHES "OpenGL3" OR (BACKEND_PLATFORM MATCHES "GLFW3" AND BACKEND_RENDERER MATCHES "Software"))
	if (CMAKE_VERSION GREATER_EQUAL 3.11)
		cmake_policy(SET CMP0072 NEW)
	endif()

	find_package(OpenGL REQUIRED)
	target_link_libraries(CSE2 PRIVATE OpenGL::GL)
endif()