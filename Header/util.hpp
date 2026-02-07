#ifndef UTIL_HPP
#define UTIL_HPP

// FPS Limiting
void limitFPS(double& lastTimeForRefresh, int targetFPS = 75);

// Texture Loading
unsigned int loadTexture(const char* path);

// Overlay Setup
void setupOverlayQuad(unsigned int& VAO, unsigned int& VBO);

// Green screen overlay for sick camera passenger
void setupFullscreenQuad(unsigned int& VAO, unsigned int& VBO);
unsigned int createGreenTexture();

// Ground cuboid (textured, uses scene shader vertex format: pos3 normal3 uv2)
void setupGroundMesh(unsigned int& VAO, unsigned int& VBO, int& vertexCount,
                     float sizeX, float sizeZ, float height, float uvTile);

#endif
