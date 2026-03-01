cmake_minimum_required(VERSION 3.21)
project(MiniEngineSDL2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Main Executable
add_executable(MiniEngineSDL2
    src/main.cpp

    src/engine/Engine.cpp
    src/engine/Engine.h
    src/engine/Input.cpp
    src/engine/Input.h
    src/engine/Renderer.cpp
    src/engine/Renderer.h
    src/engine/Log.cpp
    src/engine/Log.h
    src/engine/Assert.h
    src/engine/IScene.h
    src/engine/SceneManager.cpp
    src/engine/SceneManager.h
    src/engine/Actions.h
    src/engine/InputMap.cpp
    src/engine/InputMap.h
    src/engine/Texture2D.cpp
    src/engine/Texture2D.h
    src/engine/SpriteSheet.cpp
    src/engine/SpriteSheet.h
    src/engine/Animator.h
    src/engine/Camera2D.h
    src/engine/Camera2D.cpp
    src/engine/WorldRender2D.h 
    src/engine/WorldRender2D.cpp
    src/engine/ParticleSystem2D.h 
    src/engine/ParticleSystem2D.cpp
    src/engine/CollisionWorld2D.h
    src/engine/CollisionWorld2D.cpp

    src/engine/math/Vec2.h

    src/game/MenuScene.cpp
    src/game/MenuScene.h
    src/game/DemoScene.cpp
    src/game/DemoScene.h
    src/game/PauseScene.cpp
    src/game/PauseScene.h

 "src/engine/EngineContext.h" "src/engine/Actions.h" "src/engine/InputMap.h" "src/engine/InputMap.cpp" "src/game/PauseScene.h" "src/game/PauseScene.cpp" "src/engine/math/Vec2.h" "src/engine/Texture2D.h" "src/engine/Texture2D.cpp" "src/engine/SpriteSheet.h" "src/engine/SpriteSheet.cpp" "src/engine/Animator.h" "src/engine/CollisionWorld2D.h" "src/engine/CollisionWorld2D.cpp")

target_include_directories(MiniEngineSDL2 PRIVATE
    src
)

# Copy assets to output directory
add_custom_command(TARGET MiniEngineSDL2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:MiniEngineSDL2>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/assets/player.bmp
        $<TARGET_FILE_DIR:MiniEngineSDL2>/assets/player.bmp
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/assets/player_sheet.bmp
        $<TARGET_FILE_DIR:MiniEngineSDL2>/assets/player_sheet.bmp
)

add_custom_command(TARGET MiniEngineSDL2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:MiniEngineSDL2>/assets
)

# SDL2 from vcpkg provides CONFIG packages + targets
find_package(SDL2 CONFIG REQUIRED)

target_link_libraries(MiniEngineSDL2 PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

# copy runtime DLLs next to the exe
add_custom_command(TARGET MiniEngineSDL2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:MiniEngineSDL2>
        $<TARGET_FILE_DIR:MiniEngineSDL2>
    COMMAND_EXPAND_LISTS
)

option(ME_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

if (ME_WARNINGS_AS_ERRORS)
    if (MSVC)
        target_compile_options(MiniEngineSDL2 PRIVATE /WX)
    else()
        target_compile_options(MiniEngineSDL2 PRIVATE -Werror)
    endif()
endif()

# Warnings
if (MSVC)
    target_compile_options(MiniEngineSDL2 PRIVATE /W4 /permissive-)
else()
    target_compile_options(MiniEngineSDL2 PRIVATE -Wall -Wextra -Wpedantic)
endif()

