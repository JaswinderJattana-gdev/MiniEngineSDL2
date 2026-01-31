# MiniEngineSDL2 🎮

A lightweight 2D mini-engine built from scratch in **C++** using **SDL2**, focused on core engine and gameplay fundamentals.

This project was created as a **portfolio engine demo**.

---

## 🎥 Demo

👉 **Demo video:**  
(https://github.com/JaswinderJattana-gdev/MiniEngineSDL2/tree/main/media)

(Click the link to watch the engine demo: movement, animation, dash, particles, collision, and camera.)

---

## ✨ Features

### Engine & Architecture
- Fixed timestep game loop
- Scene system (Menu / Demo / Pause)
- Action-based input mapping
- Debug HUD & overlay toggle (F1)

### Rendering & World
- Logical resolution: **800 × 450**
- Camera tracking in world space
- Grid-based world background
- World boundaries

### Gameplay & Animation
- 8-direction sprite sheet animation
- Dash mechanic (Shift) with camera shake
- Particle system (dust while moving)
- Procedurally scattered obstacles

### Collision
- Feet-based collision collider
- Axis-separated collision resolution

### Assets
- BMP asset loading (no SDL_image dependency)
- Menu and Pause scenes rendered using images

---

## 🎮 Controls

| Action | Key |
|------|-----|
| Move | WASD / Arrow Keys |
| Dash | Left Shift / Right Shift |
| Pause | P |
| Back / Menu | ESC |
| Debug Toggle | F1 |

---

## 🧱 Build (Windows)

**Requirements**
- Windows 11
- CMake
- Visual Studio (C++ workload)
- vcpkg
- SDL2

**Build (Release example)**

```bash
cmake -S . -B out/build/release -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build out/build/release
