#include "../Header/util.hpp"

#include <iostream>
#include <thread>
#include <chrono>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../stb_image.h"

void limitFPS(double& lastTimeForRefresh, int targetFPS)
{
    double now = glfwGetTime();
    double targetFrameTime = 1.0 / targetFPS;
    double remaining = (lastTimeForRefresh + targetFrameTime) - now;

    if (remaining > 0.0)
    {
        std::this_thread::sleep_for(std::chrono::duration<double>(remaining));
    }

    lastTimeForRefresh = glfwGetTime();
}

unsigned int loadTexture(const char* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    // Flip image vertically to match OpenGL's coordinate system
    stbi_set_flip_vertically_on_load(true);

    int width, height, nrChannels;
    // Force load as RGBA to handle any color space issues
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 4);
    if (data)
    {
        std::cout << "Loaded texture: " << path << " (" << width << "x" << height
                  << ", original " << nrChannels << " channels, converted to RGBA)" << std::endl;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load texture: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void setupOverlayQuad(unsigned int& VAO, unsigned int& VBO)
{
    // Overlay quad in bottom-right corner (normalized device coordinates)
    // Position (x, y), UV (u, v)
    float overlayVertices[] = {
        // positions    // UVs
         0.65f, -0.65f,  0.0f, 1.0f,  // bottom-left
         0.95f, -0.65f,  1.0f, 1.0f,  // bottom-right
         0.95f, -0.95f,  1.0f, 0.0f,  // top-right

         0.65f, -0.65f,  0.0f, 1.0f,  // bottom-left
         0.95f, -0.95f,  1.0f, 0.0f,  // top-right
         0.65f, -0.95f,  0.0f, 0.0f   // top-left
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(overlayVertices), overlayVertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // UV attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
