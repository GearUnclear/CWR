# PoseidonGame - Game client executable

set(GAME_SOURCES
    WinMain.cpp
    GameApplication.cpp
    HighPerformanceGpuHint.cpp
)


if(WIN32)
    list(APPEND GAME_SOURCES resource.rc)
    add_executable(PoseidonGame WIN32 ${GAME_SOURCES})
else()
    add_executable(PoseidonGame ${GAME_SOURCES})
endif()

# Executable-specific compile definitions
target_compile_definitions(PoseidonGame PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>     # Enable debug mode (affects Assert/Fail macros in debugLog.hpp)
)
if(WIN32)
    target_compile_definitions(PoseidonGame PRIVATE _WINDOWS)
endif()

# Match MSVC compiler settings
if(MSVC OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"))
    target_compile_options(PoseidonGame PRIVATE
        /W3                    # Warning level 3
        /GR                    # Enable RTTI
        $<$<CONFIG:Debug>:/Od>     # Disable optimization (Debug only)
        $<$<CONFIG:Debug>:/MDd>    # Runtime library: MultiThreadedDebugDLL (Debug only)
    )
    
    # Disable SafeSEH for compatibility with external .obj files.
    target_link_options(PoseidonGame PRIVATE /SAFESEH:NO)
endif()

# Include paths: only need root and Poseidon (all libs/externals handled by Poseidon.lib)
target_include_directories(PoseidonGame PRIVATE
    ${CMAKE_SOURCE_DIR}/engine
    # resource.rc references resrc1.h/resource.h (Core) and versionNo.h
    # (Foundation/Platform) by bare name; expose those dirs to the RC compiler only.
    $<$<COMPILE_LANGUAGE:RC>:${CMAKE_SOURCE_DIR}/engine/Poseidon/Core>
    $<$<COMPILE_LANGUAGE:RC>:${CMAKE_SOURCE_DIR}/engine/Poseidon/Foundation/Platform>
)

# Link against libraries (Poseidon transitively brings in folded support/runtime code and externals)
if(WIN32)
    target_link_libraries(PoseidonGame PRIVATE
        GameBase
        Poseidon
        PoseidonGL33
        PoseidonOpenAL
        # Windows system libraries
        winmm.lib
        version.lib
        ws2_32.lib
        legacy_stdio_definitions.lib
    )
elseif(APPLE)
    target_link_libraries(PoseidonGame PRIVATE
        GameBase
        PoseidonGL33
        Poseidon
        PoseidonOpenAL
        pthread
        dl
    )
else()
    target_link_libraries(PoseidonGame PRIVATE
        -Wl,--start-group
        GameBase
        PoseidonGL33
        Poseidon
        PoseidonOpenAL
        -Wl,--end-group
        pthread
        dl
    )
endif()

# Use WINDOWS subsystem (WIN32 flag); AttachConsole at runtime handles CLI output
if(MSVC OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"))
    set_target_properties(PoseidonGame PROPERTIES
        LINK_FLAGS "/IGNORE:4099"
    )
endif()

set_target_properties(PoseidonGame PROPERTIES
    OUTPUT_NAME "PoseidonGame"
)

include(${CMAKE_SOURCE_DIR}/cmake/DistCopy.cmake)
dist_copy(PoseidonGame)

# The fire spread's flame flipbooks, one a style (POSEIDON_XP_FIRESPREAD;
# engine/PoseidonGL33/EngineGL33_FireFlames.cpp loads fx/flame_<style>.png
# beside the executable, a missing style takes another's, and it draws
# procedural flames without any). Made by tools/firespread/gen_flame_flipbook.py.
foreach(_flame_style front tongues creep bushy)
    set(_flame_book "${CMAKE_SOURCE_DIR}/tools/firespread/flame_${_flame_style}.png")
    if(EXISTS "${_flame_book}")
        add_custom_command(TARGET PoseidonGame POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:PoseidonGame>/fx ${DIST_DIR}/fx
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_flame_book}
                    $<TARGET_FILE_DIR:PoseidonGame>/fx/flame_${_flame_style}.png
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_flame_book} ${DIST_DIR}/fx/flame_${_flame_style}.png
            COMMENT "Copying the ${_flame_style} flame flipbook beside PoseidonGame"
            VERBATIM
        )
    endif()
endforeach()

# The fire spread's sounds (POSEIDON_XP_FIRESPREAD; FireSpreadWorld.cpp
# FireSound loads fx/fire_*.ogg beside the executable, and the fire is silent
# without them). Made by tools/firespread/gen_fire_sounds.py.
foreach(_fire_sound fire_crackle_loop fire_roar_loop fire_pop_1 fire_pop_2 fire_pop_3 fire_pop_4 fire_pop_5 fire_pop_6)
    set(_fire_ogg "${CMAKE_SOURCE_DIR}/tools/firespread/${_fire_sound}.ogg")
    if(EXISTS "${_fire_ogg}")
        add_custom_command(TARGET PoseidonGame POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:PoseidonGame>/fx ${DIST_DIR}/fx
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_fire_ogg} $<TARGET_FILE_DIR:PoseidonGame>/fx/${_fire_sound}.ogg
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_fire_ogg} ${DIST_DIR}/fx/${_fire_sound}.ogg
            COMMENT "Copying the ${_fire_sound} fire sound beside PoseidonGame"
            VERBATIM
        )
    endif()
endforeach()
