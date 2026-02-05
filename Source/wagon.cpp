#include "../Header/wagon.hpp"
#include "../Header/shader.hpp"
#include "../Header/trackpath.hpp"
#include <glm/gtc/matrix_transform.hpp>

Wagon::Wagon(float width, float height, float depth)
    : VAO(0), VBO(0), vertexCount(0),
      width(width), height(height), depth(depth),
      position(0.0f), color(0.2f, 0.9f, 0.2f),
      forwardDir(0.0f, 0.0f, 1.0f),
      upDir(0.0f, 1.0f, 0.0f),
      rightDir(1.0f, 0.0f, 0.0f),
      trackT(0.0f),
      heightOffset(1.0f)  // Default height above track center
{
}

Wagon::~Wagon()
{
    if (VAO != 0)
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
}

void Wagon::init()
{
    setupMesh();
}

void Wagon::setPosition(const glm::vec3& pos)
{
    position = pos;
}

void Wagon::setOrientation(const glm::vec3& forward, const glm::vec3& up)
{
    forwardDir = glm::normalize(forward);
    upDir = glm::normalize(up);
    rightDir = glm::normalize(glm::cross(forwardDir, upDir));
    // Recalculate up to ensure orthogonality
    upDir = glm::normalize(glm::cross(rightDir, forwardDir));
}

void Wagon::updateFromTrackPath(const TrackPath& path, float t)
{
    if (!path.isInitialized())
    {
        return;
    }

    trackT = t;

    // Get position from track and offset upward
    glm::vec3 trackPos = path.getPosition(t);
    glm::vec3 up = path.getUp(t);

    // Position wagon above the track center
    position = trackPos + up * heightOffset;

    // Set orientation to follow track
    glm::vec3 forward = path.getForward(t);
    setOrientation(forward, up);
}

void Wagon::setupMesh()
{
    // Half dimensions for easier vertex calculations
    float hw = width / 2.0f;   // half width (X)
    float hh = height / 2.0f;  // half height (Y)
    float hd = depth / 2.0f;   // half depth (Z)

    // Hollow cuboid - 5 faces (no top face)
    // Each vertex: position (3), normal (3), UV (2)
    float vertices[] = {
        // Front face (+Z) - Looking from +Z towards -Z
        // CCW: BL -> BR -> TR, TR -> TL -> BL
        -hw, -hh,  hd,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f, // Bottom-Left
         hw, -hh,  hd,   0.0f,  0.0f,  1.0f,   1.0f, 0.0f, // Bottom-Right
         hw,  hh,  hd,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f, // Top-Right
         hw,  hh,  hd,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f, // Top-Right
        -hw,  hh,  hd,   0.0f,  0.0f,  1.0f,   0.0f, 1.0f, // Top-Left
        -hw, -hh,  hd,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f, // Bottom-Left

        // Back face (-Z) - Looking from -Z towards +Z
        // CCW: BL (hw) -> BR (-hw) -> TR (-hw), TR -> TL -> BL
         hw, -hh, -hd,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f, // Bottom-Left (from back)
        -hw, -hh, -hd,   0.0f,  0.0f, -1.0f,   1.0f, 0.0f, // Bottom-Right
        -hw,  hh, -hd,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f, // Top-Right
        -hw,  hh, -hd,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f, // Top-Right
         hw,  hh, -hd,   0.0f,  0.0f, -1.0f,   0.0f, 1.0f, // Top-Left
         hw, -hh, -hd,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f, // Bottom-Left

         // Left face (-X) - Looking from -X towards +X
         // CCW: BL (-zd) -> BR (+zd) -> TR (+zd), TR -> TL -> BL
         -hw, -hh, -hd,  -1.0f,  0.0f,  0.0f,   0.0f, 0.0f, // Bottom-Left
         -hw, -hh,  hd,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f, // Bottom-Right
         -hw,  hh,  hd,  -1.0f,  0.0f,  0.0f,   1.0f, 1.0f, // Top-Right
         -hw,  hh,  hd,  -1.0f,  0.0f,  0.0f,   1.0f, 1.0f, // Top-Right
         -hw,  hh, -hd,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f, // Top-Left
         -hw, -hh, -hd,  -1.0f,  0.0f,  0.0f,   0.0f, 0.0f, // Bottom-Left

         // Right face (+X) - Looking from +X towards -X
         // CCW: BL (+zd) -> BR (-zd) -> TR (-zd), TR -> TL -> BL
          hw, -hh,  hd,   1.0f,  0.0f,  0.0f,   0.0f, 0.0f, // Bottom-Left
          hw, -hh, -hd,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f, // Bottom-Right
          hw,  hh, -hd,   1.0f,  0.0f,  0.0f,   1.0f, 1.0f, // Top-Right
          hw,  hh, -hd,   1.0f,  0.0f,  0.0f,   1.0f, 1.0f, // Top-Right
          hw,  hh,  hd,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f, // Top-Left
          hw, -hh,  hd,   1.0f,  0.0f,  0.0f,   0.0f, 0.0f, // Bottom-Left

          // Bottom face (-Y) - Looking from -Y towards +Y
          // CCW: BL (-hw, -hd) -> BR (hw, -hd) -> TR (hw, hd)
          -hw, -hh, -hd,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f, // Bottom-Left
           hw, -hh, -hd,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f, // Bottom-Right
           hw, -hh,  hd,   0.0f, -1.0f,  0.0f,   1.0f, 1.0f, // Top-Right
           hw, -hh,  hd,   0.0f, -1.0f,  0.0f,   1.0f, 1.0f, // Top-Right
          -hw, -hh,  hd,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f, // Top-Left
          -hw, -hh, -hd,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f, // Bottom-Left
    };

    vertexCount = 30; // 5 faces * 6 vertices each

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Stride: 8 floats per vertex (pos:3 + normal:3 + uv:2)
    int stride = 8 * sizeof(float);

    // Position - location 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // Normal - location 1
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // UV - location 2
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Wagon::draw(Shader& shader)
{
    shader.use();

    // Create model matrix with position and orientation
    // The wagon's local Z axis points forward along the track
    // Build rotation matrix from orientation vectors
    glm::mat4 rotation(1.0f);
    rotation[0] = glm::vec4(rightDir, 0.0f);    // X axis = right
    rotation[1] = glm::vec4(upDir, 0.0f);       // Y axis = up
    rotation[2] = glm::vec4(forwardDir, 0.0f);  // Z axis = forward
    rotation[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = model * rotation;

    shader.setMat4("uM", model);
    shader.setBool("uUseTexture", false);
    shader.setVec3("uMaterialColor", color);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);
}
