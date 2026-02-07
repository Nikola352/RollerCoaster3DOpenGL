//Opis: 3D RollerCoaster projekat
//OpenGL 3.3+ sa programabilnim pajplajnom

#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../Header/shader.hpp"
#include "../Header/model.hpp"
#include "../Header/util.hpp"
#include "../Header/wagon.hpp"
#include "../Header/trackpath.hpp"
#include "../Header/passenger.hpp"
#include "../Header/Game/RollerCoaster.hpp"
#include "../Header/Game/Constants.hpp"

#include <vector>
#include <map>

const int FPS = 75;

// Global state for toggles (consistent with Aquarium project)
bool depthTestEnabled = true;
bool faceCullingEnabled = false;
bool cullBackFaces = true;
bool isCCWWinding = true;

// Camera mode
enum class CameraMode {
    ORBIT,       // Ground perspective, orbiting around track
    FIRST_PERSON // Sitting in front seat of wagon
};

CameraMode cameraMode = CameraMode::ORBIT;

// Orbit camera state
float cameraYaw = -45.0f;    // Horizontal angle
float cameraPitch = 20.0f;   // Vertical angle
float cameraDistance = 130.0f;
glm::vec3 cameraTarget(30.0f, 10.0f, 10.0f);

// First-person camera state (look offset from wagon forward)
float fpYaw = 0.0f;
float fpPitch = 0.0f;

// Mouse tracking
double lastMouseX = 0, lastMouseY = 0;
bool firstMouse = true;

// Wagon pointer for keyboard callback access
Wagon* g_wagon = nullptr;

// Game logic
RollerCoaster* g_game = nullptr;

// Passenger models (pre-loaded, keyed by seat index)
std::map<int, Passenger*> passengerModels;

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    // Only rotate camera when left mouse button is pressed
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS)
    {
        lastMouseX = xpos;
        lastMouseY = ypos;
        return;
    }

    float xoffset = (float)(xpos - lastMouseX) * 0.3f;
    float yoffset = (float)(lastMouseY - ypos) * 0.3f;
    lastMouseX = xpos;
    lastMouseY = ypos;

    if (cameraMode == CameraMode::ORBIT) {
        cameraYaw += xoffset;
        cameraPitch += yoffset;

        // Clamp pitch to avoid flipping
        if (cameraPitch > 89.0f) cameraPitch = 89.0f;
        if (cameraPitch < -89.0f) cameraPitch = -89.0f;
    } else {
        // First-person mode - look around from seat
        fpYaw += xoffset;
        fpPitch += yoffset;

        // Limit look range (can't look too far behind or up/down)
        if (fpYaw > 120.0f) fpYaw = 120.0f;
        if (fpYaw < -120.0f) fpYaw = -120.0f;
        if (fpPitch > 60.0f) fpPitch = 60.0f;
        if (fpPitch < -60.0f) fpPitch = -60.0f;
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS) return;

    switch (key)
    {
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, true);
        break;

    // Game controls
    case GLFW_KEY_SPACE:
        if (g_game) {
            g_game->handleAddPassenger();
        }
        break;

    case GLFW_KEY_ENTER:
        if (g_game) {
            g_game->handleStartRide();
        }
        break;

    // Seat actions (keys 1-8 for seats 0-7)
    case GLFW_KEY_1:
    case GLFW_KEY_2:
    case GLFW_KEY_3:
    case GLFW_KEY_4:
    case GLFW_KEY_5:
    case GLFW_KEY_6:
    case GLFW_KEY_7:
    case GLFW_KEY_8:
        if (g_game) {
            int seatIndex = key - GLFW_KEY_1;  // Convert key to 0-7 index
            g_game->handleSeatAction(seatIndex);
        }
        break;

    // Debug/toggle controls (use F keys to avoid conflict with seat keys)
    case GLFW_KEY_F1:
        depthTestEnabled = !depthTestEnabled;
        if (depthTestEnabled)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);
        std::cout << (depthTestEnabled ? "DEPTH TEST ENABLED" : "DEPTH TEST DISABLED") << std::endl;
        break;

    case GLFW_KEY_F2:
        faceCullingEnabled = !faceCullingEnabled;
        if (faceCullingEnabled)
            glEnable(GL_CULL_FACE);
        else
            glDisable(GL_CULL_FACE);
        std::cout << (faceCullingEnabled ? "FACE CULLING ENABLED" : "FACE CULLING DISABLED") << std::endl;
        break;

    case GLFW_KEY_F3:
        cullBackFaces = !cullBackFaces;
        glCullFace(cullBackFaces ? GL_BACK : GL_FRONT);
        std::cout << (cullBackFaces ? "CULLING BACK" : "CULLING FRONT") << std::endl;
        break;

    case GLFW_KEY_F4:
        isCCWWinding = !isCCWWinding;
        glFrontFace(isCCWWinding ? GL_CCW : GL_CW);
        std::cout << (isCCWWinding ? "CCW WINDING" : "CW WINDING") << std::endl;
        break;

    case GLFW_KEY_V:
        if (cameraMode == CameraMode::ORBIT) {
            // Only allow FPV if there are passengers
            if (g_game && !g_game->getPassengers().empty()) {
                cameraMode = CameraMode::FIRST_PERSON;
                fpYaw = 0.0f;
                fpPitch = 0.0f;
                std::cout << "CAMERA: FIRST PERSON" << std::endl;
            } else {
                std::cout << "CAMERA: Cannot switch to FPV - no passengers!" << std::endl;
            }
        } else {
            cameraMode = CameraMode::ORBIT;
            std::cout << "CAMERA: ORBIT" << std::endl;
        }
        break;
    }
}

int main()
{
    if (!glfwInit())
    {
        std::cout << "GLFW fail!\n" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Get primary monitor for full screen
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    // Create full screen window
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "RollerCoaster 3D", monitor, NULL);
    if (window == NULL)
    {
        std::cout << "Window fail!\n" << std::endl;
        glfwTerminate();
        return -2;
    }
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
    {
        std::cout << "GLEW fail! :(\n" << std::endl;
        return -3;
    }

    // Set callbacks
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouseCallback);

    // Load models and shaders
    Model track("res/track.obj");
    Shader sceneShader("Shader/basic.vert", "Shader/basic.frag");
    Shader overlayShader("Shader/texture.vert", "Shader/texture.frag");

    // Extract track center line for wagon positioning
    TrackPath trackPath;
    trackPath.extractFromModel(track, 300, 384);

    // Create wagon and place it at the beginning of the track
    Wagon wagon(8.0f, 5.0f, 14.0f);
    wagon.init();
    wagon.setHeightOffset(3.5f);  // Height above track center line
    g_wagon = &wagon;  // Set global pointer for keyboard callback

    // Create game logic (will set wagon position to START_TRACK_T)
    RollerCoaster game(wagon, trackPath);
    g_game = &game;

    // Load student info texture
    unsigned int studentTexture = loadTexture("res/student.png");

    // Pre-load all 8 passenger models (will be displayed dynamically based on game state)
    std::cout << "Loading passenger models..." << std::endl;
    for (int i = 0; i < static_cast<int>(MAX_PASSENGERS); ++i) {
        std::string path = "res/person" + std::to_string(i + 1) + "/model_mesh.obj";
        passengerModels[i] = new Passenger(path, i);
    }
    std::cout << "Passenger models loaded." << std::endl;

    // Setup overlay quad
    unsigned int overlayVAO, overlayVBO;
    setupOverlayQuad(overlayVAO, overlayVBO);

    // Setup green overlay for sick camera passenger
    unsigned int greenOverlayVAO, greenOverlayVBO;
    setupFullscreenQuad(greenOverlayVAO, greenOverlayVBO);
    unsigned int greenTexture = createGreenTexture();

    // Setup 3D scene
    sceneShader.use();
    sceneShader.setVec3("uViewPos", 0, 30, 100);
    sceneShader.setVec3("uLightColor", 1, 1, 1);
    sceneShader.setFloat("uLightIntensity", 1.5f);
    sceneShader.setVec3("uMaterialColor", 0.6f, 0.3f, 0.1f);  // Brown track color
    sceneShader.setBool("uUseTexture", false);  // Track has no texture
    sceneShader.setVec3("uTintColor", 1.0f, 1.0f, 1.0f);  // Default: no tint

    float aspectRatio = (float)mode->width / (float)mode->height;
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 500.0f);
    sceneShader.setMat4("uP", projection);

    // View matrix will be calculated each frame based on mouse input

    glm::mat4 model = glm::mat4(1.0f);

    // Setup overlay shader (orthographic projection)
    overlayShader.use();
    glm::mat4 orthoProjection = glm::mat4(1.0f); // Identity for NDC
    overlayShader.setMat4("uP", orthoProjection);
    overlayShader.setInt("uTexture", 0);

    // Set blue background color
    glClearColor(0.245f, 0.6f, 0.85f, 1.0f);

    // Enable depth test by default
    glEnable(GL_DEPTH_TEST);

    // Enable blending for transparent overlay
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Face culling setup
    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    std::cout << "Controls:" << std::endl;
    std::cout << "  Mouse  - Orbit camera around track" << std::endl;
    std::cout << "  SPACE  - Add passenger" << std::endl;
    std::cout << "  1-8    - Seatbelt (onboarding) / Sick (riding) / Remove (offboarding)" << std::endl;
    std::cout << "  ENTER  - Start ride (all passengers must be buckled)" << std::endl;
    std::cout << "  V      - Toggle camera (orbit / first-person)" << std::endl;
    std::cout << "  ESC    - Quit" << std::endl;
    std::cout << "  F1     - Toggle depth test" << std::endl;
    std::cout << "  F2     - Toggle face culling" << std::endl;
    std::cout << "  F3     - Toggle back/front face culling" << std::endl;
    std::cout << "  F4     - Toggle winding order (CCW/CW)" << std::endl;

    double lastTimeForRefresh = glfwGetTime();
    double lastTime = glfwGetTime();
    size_t prevPassengerCount = 0;

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Calculate delta time
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        glfwPollEvents();

        // Update game logic (handles wagon physics internally)
        game.update(deltaTime);
        wagon.updatePhysics(trackPath, deltaTime);

        // Auto-switch camera on passenger count changes
        size_t currentPassengerCount = game.getPassengers().size();
        if (prevPassengerCount == 0 && currentPassengerCount > 0) {
            // First passenger added → switch to FPV
            cameraMode = CameraMode::FIRST_PERSON;
            fpYaw = 0.0f;
            fpPitch = 0.0f;
        } else if (currentPassengerCount == 0 && prevPassengerCount > 0) {
            // Last passenger removed → switch to orbit
            cameraMode = CameraMode::ORBIT;
        }
        prevPassengerCount = currentPassengerCount;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Calculate view matrix based on camera mode
        glm::mat4 view;
        glm::vec3 cameraPos;

        if (cameraMode == CameraMode::ORBIT) {
            // Orbit camera: spherical coordinates around target
            float camX = cameraDistance * cos(glm::radians(cameraPitch)) * cos(glm::radians(cameraYaw));
            float camY = cameraDistance * sin(glm::radians(cameraPitch));
            float camZ = cameraDistance * cos(glm::radians(cameraPitch)) * sin(glm::radians(cameraYaw));
            cameraPos = cameraTarget + glm::vec3(camX, camY, camZ);
            view = glm::lookAt(cameraPos, cameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));
        } else {
            // First-person camera from front seat (seat 0)
            Wagon::SeatTransform seat = wagon.getSeatWorldTransform(0);
            cameraPos = seat.position + seat.up * 6.0f + seat.forward * 0.5f; // Eye height above seat + in front of person model

            // Calculate look direction with mouse offset
            glm::vec3 right = glm::normalize(glm::cross(seat.forward, seat.up));

            // Apply yaw rotation (around up axis)
            glm::mat4 yawRot = glm::rotate(glm::mat4(1.0f), glm::radians(fpYaw), seat.up);
            glm::vec3 lookDir = glm::vec3(yawRot * glm::vec4(seat.forward, 0.0f));

            // Apply pitch rotation (around right axis)
            glm::mat4 pitchRot = glm::rotate(glm::mat4(1.0f), glm::radians(fpPitch), right);
            lookDir = glm::vec3(pitchRot * glm::vec4(lookDir, 0.0f));

            view = glm::lookAt(cameraPos, cameraPos + lookDir, seat.up);
        }

        // Render 3D scene
        sceneShader.use();
        sceneShader.setMat4("uV", view);
        sceneShader.setVec3("uViewPos", cameraPos);
        sceneShader.setVec3("uLightPos", cameraPos + glm::vec3(0.0f, 50.0f, 0.0f));
        sceneShader.setMat4("uM", model);

        sceneShader.setBool("uUseTexture", false);
        sceneShader.setVec3("uMaterialColor", 0.6f, 0.3f, 0.1f);  // Brown track color
        sceneShader.setVec3("uTintColor", 1.0f, 1.0f, 1.0f);  // Reset tint for track/wagon
        track.Draw(sceneShader);

        // Draw wagon
        wagon.draw(sceneShader);

        // Draw passengers based on game state
        for (const Person& person : game.getPassengers()) {
            int seatIndex = person.getSeatIndex();
            Passenger* passengerModel = passengerModels[seatIndex];

            // Sync rendering state with game logic
            passengerModel->setBuckled(person.getHasSeatbelt());
            passengerModel->setSick(person.getIsSick());

            passengerModel->draw(sceneShader, wagon);
        }

        // Green screen filter when camera passenger (seat 0) is sick
        if (cameraMode == CameraMode::FIRST_PERSON) {
            const Person* frontPassenger = game.getPassengerBySeat(0);
            if (frontPassenger && frontPassenger->getIsSick()) {
                glDepthFunc(GL_ALWAYS);
                overlayShader.use();
                glBindTexture(GL_TEXTURE_2D, greenTexture);
                glBindVertexArray(greenOverlayVAO);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                glDepthFunc(GL_LESS);
            }
        }

        // Render 2D overlay (student info)
        glDepthFunc(GL_ALWAYS); // Always pass depth test for overlay
        glDisable(GL_CULL_FACE); // Disable culling for overlay
        overlayShader.use();
        glBindTexture(GL_TEXTURE_2D, studentTexture);
        glBindVertexArray(overlayVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDepthFunc(GL_LESS); // Reset depth function
        // Reset culling settings
        if (faceCullingEnabled)
            glEnable(GL_CULL_FACE);
        else
            glDisable(GL_CULL_FACE);

        glfwSwapBuffers(window);
        limitFPS(lastTimeForRefresh, FPS);
    }

    // Cleanup
    g_wagon = nullptr;
    g_game = nullptr;

    // Cleanup passenger models
    for (auto& pair : passengerModels) {
        delete pair.second;
    }
    passengerModels.clear();

    glDeleteVertexArrays(1, &overlayVAO);
    glDeleteBuffers(1, &overlayVBO);
    glDeleteVertexArrays(1, &greenOverlayVAO);
    glDeleteBuffers(1, &greenOverlayVBO);
    glDeleteTextures(1, &studentTexture);
    glDeleteTextures(1, &greenTexture);

    glfwTerminate();
    return 0;
}
