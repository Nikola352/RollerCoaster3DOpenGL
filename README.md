# RollerCoaster 3D

A 3D roller coaster simulation built with OpenGL 3.3+ and C++.

Passengers board a wagon, buckle their seatbelts, and ride along a track with hills. The simulation features physics-based acceleration (faster downhill, slower uphill), first-person and orbit cameras, and a sickness mechanic with a green screen filter.

## Features

- Roller coaster track loaded as a 3D model with multiple hills
- Textured wagon with 8 seats
- 8 unique humanoid passenger models
- Physics-driven wagon movement along the track
- Orbit camera and first-person camera from the front seat
- Seatbelt rendering per passenger
- Passenger sickness mechanic with green tint effect
- Toggleable depth testing, face culling, and winding order
- Fullscreen mode at 75 FPS

## Controls

| Key | Action |
|---|---|
| Mouse (LMB drag) | Rotate camera |
| Space | Add passenger to next empty seat |
| 1-8 | Buckle seatbelt / signal sickness / remove passenger |
| Enter | Start ride (all passengers must be buckled) |
| V | Toggle orbit / first-person camera |
| F1 | Toggle depth test |
| F2 | Toggle face culling |
| F3 | Toggle back/front face culling |
| F4 | Toggle winding order (CCW/CW) |
| Esc | Quit |

## Building

Requires Visual Studio with C++ and NuGet package restore.

1. Open `RollerCoaster3D.sln` in Visual Studio
2. Restore NuGet packages (should happen automatically)
3. Build with **F7** (or Build > Build Solution)
4. Run with **F5** (or Debug > Start Debugging)

## Dependencies

All managed via NuGet:

- **GLFW 3.4** - Window and input management
- **GLEW 2.2** - OpenGL extension loading
- **GLM 1.0** - OpenGL mathematics
- **Assimp 3.0** - 3D model loading
- **stb_image** - Texture loading

## Project Structure

```
Source/          C++ implementation files
Header/          Header files
Shader/          GLSL vertex and fragment shaders
res/             3D models and textures
```
