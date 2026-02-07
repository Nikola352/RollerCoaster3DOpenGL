#include "../Header/passenger.hpp"
#include "../Header/model.hpp"
#include "../Header/shader.hpp"
#include "../Header/wagon.hpp"
#include "../Header/util.hpp"

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

// Static members for shared seatbelt texture
unsigned int Passenger::seatbeltTextureID = 0;
bool Passenger::seatbeltTextureLoaded = false;

Passenger::Passenger(const std::string& modelPath, int seatIndex)
    : seatIndex(seatIndex)
{
    model = new Model(modelPath);
    setupSeatbeltMesh();

    // Load seatbelt texture once (shared by all passengers)
    if (!seatbeltTextureLoaded) {
        seatbeltTextureID = loadTexture("res/textures/seatbelt_texture.jpg");
        seatbeltTextureLoaded = true;
    }
}

Passenger::~Passenger()
{
    delete model;
    if (seatbeltVAO) glDeleteVertexArrays(1, &seatbeltVAO);
    if (seatbeltVBO) glDeleteBuffers(1, &seatbeltVBO);
}

void Passenger::setupSeatbeltMesh()
{
    // Seatbelt as a diagonal strip across chest (from left shoulder to right hip)
    // Coordinates relative to passenger center, will be transformed with passenger matrix
    const float beltWidth = 0.12f;  // Width of the belt
    const float beltDepth = 0.15f;  // How far it sticks out from body

    // Belt goes diagonally: left shoulder (top) to right hip (bottom)
    // In local space: Y is up, X is right, Z is forward
    float vertices[] = {
        // Front face of belt (visible from front)
        // Position (x, y, z), Normal (nx, ny, nz), UV (u, v)
        // Left shoulder area (top-left of belt)
        -0.15f,  0.45f, beltDepth,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,
        -0.15f - beltWidth, 0.45f, beltDepth,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
        // Right hip area (bottom-right of belt)
         0.12f,  0.15f, beltDepth,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
         0.12f + beltWidth, 0.15f, beltDepth,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,

        // Back face of belt (for backface visibility)
         0.12f + beltWidth, 0.15f, -beltDepth,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f,
         0.12f,  0.15f, -beltDepth,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f,
        -0.15f - beltWidth, 0.45f, -beltDepth,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f,
        -0.15f,  0.45f, -beltDepth,   0.0f, 0.0f, -1.0f,   1.0f, 1.0f,
    };

    glGenVertexArrays(1, &seatbeltVAO);
    glGenBuffers(1, &seatbeltVBO);

    glBindVertexArray(seatbeltVAO);
    glBindBuffer(GL_ARRAY_BUFFER, seatbeltVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    int stride = 8 * sizeof(float);
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // UV attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Passenger::drawSeatbelt(Shader& shader, const Wagon& wagon)
{
    // Use seatbelt texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, seatbeltTextureID);
    shader.setInt("uDiffMap1", 0);
    shader.setBool("uUseTexture", true);

    // Use same transform as passenger but no extra scaling for belt
    // (belt coordinates are already in passenger's local space relative to SCALE)
    glm::mat4 beltMatrix = calculateModelMatrix(wagon);
    shader.setMat4("uM", beltMatrix);

    glBindVertexArray(seatbeltVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);  // Front face
    glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);  // Back face
    glBindVertexArray(0);
}

glm::mat4 Passenger::calculateModelMatrix(const Wagon& wagon) const
{
    Wagon::SeatTransform seat = wagon.getSeatWorldTransform(seatIndex);

    glm::vec3 forward = seat.forward;
    glm::vec3 up = seat.up;
    glm::vec3 right = glm::normalize(glm::cross(forward, up));
    up = glm::normalize(glm::cross(right, forward));

    // Build rotation matrix from orientation vectors
    glm::mat4 rotation(1.0f);
    rotation[0] = glm::vec4(right, 0.0f);
    rotation[1] = glm::vec4(up, 0.0f);
    rotation[2] = glm::vec4(forward, 0.0f);
    rotation[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    // Build model matrix: translate -> rotate -> scale
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::vec3 adjustedPos = seat.position + up * Y_OFFSET;
    modelMatrix = glm::translate(modelMatrix, adjustedPos);
    modelMatrix = modelMatrix * rotation;
    // Lean passenger backwards by 10 degrees (rotate around local X-axis)
    modelMatrix = glm::rotate(modelMatrix, glm::radians(-10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(SCALE));

    return modelMatrix;
}

void Passenger::draw(Shader& shader, const Wagon& wagon)
{
    // Set tint color: green if sick, white (no tint) otherwise
    if (sick) {
        shader.setVec3("uTintColor", 0.3f, 1.0f, 0.3f);  // Green tint
    } else {
        shader.setVec3("uTintColor", 1.0f, 1.0f, 1.0f);  // No tint
    }

    shader.setBool("uUseTexture", true);
    shader.setMat4("uM", calculateModelMatrix(wagon));
    model->Draw(shader);

    // Draw seatbelt if buckled
    if (buckled) {
        drawSeatbelt(shader, wagon);
    }
}
