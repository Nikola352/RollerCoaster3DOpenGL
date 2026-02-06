#include "../Header/wagon.hpp"
#include "../Header/shader.hpp"
#include "../Header/trackpath.hpp"
#include "../Header/util.hpp"
#include <glm/gtc/matrix_transform.hpp>

Wagon::Wagon(float width, float height, float depth)
    : VAO(0), VBO(0), vertexCount(0),
      width(width), height(height), depth(depth),
      position(0.0f), color(0.2f, 0.9f, 0.2f),
      forwardDir(0.0f, 0.0f, 1.0f),
      upDir(0.0f, 1.0f, 0.0f),
      rightDir(1.0f, 0.0f, 0.0f),
      trackT(0.0f),
      heightOffset(1.0f),
      rideState(RideState::STOPPED),
      velocity(0.0f),
      textureID(0)
{
}

Wagon::~Wagon()
{
    if (VAO != 0)
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
    if (textureID != 0)
    {
        glDeleteTextures(1, &textureID);
    }
}

void Wagon::init()
{
    setupMesh();
    setupSeatMesh();
    textureID = loadTexture("res/textures/wagon_texture.jpg");
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

void Wagon::startRide()
{
    if (rideState == RideState::STOPPED)
    {
        rideState = RideState::STARTING;
        velocity = 0.0f;
    }
}

void Wagon::stopRide()
{
    rideState = RideState::STOPPED;
    velocity = 0.0f;
}

void Wagon::updatePhysics(const TrackPath& path, float deltaTime)
{
    if (rideState == RideState::STOPPED || !path.isInitialized())
    {
        return;
    }

    if (rideState == RideState::STARTING)
    {
        // Chain lift phase: accelerate steadily until reaching cruise speed
        velocity += CHAIN_LIFT_ACCEL * deltaTime;

        if (velocity >= CRUISE_SPEED)
        {
            velocity = CRUISE_SPEED;
            rideState = RideState::RUNNING;
        }
    }
    else // RideState::RUNNING
    {
        // Get forward direction to calculate slope
        glm::vec3 forward = path.getForward(trackT);

        // Slope is the Y component of the forward direction
        // Positive = going uphill, Negative = going downhill
        float slope = forward.y;

        // Apply gravity effect: decelerate uphill, accelerate downhill
        float acceleration = -GRAVITY_EFFECT * slope;

        // Apply friction (always opposes motion)
        acceleration -= FRICTION * velocity;

        // Update velocity
        velocity += acceleration * deltaTime;

        // Clamp velocity to reasonable bounds
        if (velocity < MIN_VELOCITY)
        {
            velocity = MIN_VELOCITY;
        }
        if (velocity > MAX_VELOCITY)
        {
            velocity = MAX_VELOCITY;
        }
    }

    // Update track position
    trackT += velocity * deltaTime;

    // Loop back to start when reaching the end
    if (trackT >= 1.0f)
    {
        trackT -= 1.0f;
    }

    // Update wagon transform
    updateFromTrackPath(path, trackT);
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

    // Bind wagon texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    shader.setInt("uDiffMap1", 0);
    shader.setBool("uUseTexture", true);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);

    // Draw Seats (solid color, no texture)
    shader.setBool("uUseTexture", false);
    glBindVertexArray(seatVAO);
    shader.setVec3("uMaterialColor", color * 0.5f); // Make seats slightly darker
    for (int i = 0; i < 8; ++i) {
        drawSingleSeat(shader, model, i);
    }
    glBindVertexArray(0);
}

void Wagon::setupSeatMesh() {
    float v[] = {
        // Position (3)     // Normal (3)
        -0.5f,-0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
         0.5f, 0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
         0.5f,-0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
         0.5f, 0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
        -0.5f,-0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
        -0.5f, 0.5f,-0.5f,  0.0f, 0.0f,-1.0f,

        -0.5f,-0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
         0.5f,-0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
         0.5f, 0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
         0.5f, 0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f,-0.5f, 0.5f,  0.0f, 0.0f, 1.0f,

        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f,-0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f,-0.5f,-0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f,-0.5f,-0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f,-0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,

         0.5f, 0.5f, 0.5f,  1.0f, 0.0f, 0.0f,
         0.5f,-0.5f,-0.5f,  1.0f, 0.0f, 0.0f,
         0.5f, 0.5f,-0.5f,  1.0f, 0.0f, 0.0f,
         0.5f,-0.5f,-0.5f,  1.0f, 0.0f, 0.0f,
         0.5f, 0.5f, 0.5f,  1.0f, 0.0f, 0.0f,
         0.5f,-0.5f, 0.5f,  1.0f, 0.0f, 0.0f,

        -0.5f,-0.5f,-0.5f,  0.0f,-1.0f, 0.0f,
         0.5f,-0.5f,-0.5f,  0.0f,-1.0f, 0.0f,
         0.5f,-0.5f, 0.5f,  0.0f,-1.0f, 0.0f,
         0.5f,-0.5f, 0.5f,  0.0f,-1.0f, 0.0f,
        -0.5f,-0.5f, 0.5f,  0.0f,-1.0f, 0.0f,
        -0.5f,-0.5f,-0.5f,  0.0f,-1.0f, 0.0f,

        -0.5f, 0.5f,-0.5f,  0.0f, 1.0f, 0.0f,
         0.5f, 0.5f, 0.5f,  0.0f, 1.0f, 0.0f,
         0.5f, 0.5f,-0.5f,  0.0f, 1.0f, 0.0f,
         0.5f, 0.5f, 0.5f,  0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f,-0.5f,  0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f,  0.0f, 1.0f, 0.0f
    };

    glGenVertexArrays(1, &seatVAO);
    glGenBuffers(1, &seatVBO);
    glBindVertexArray(seatVAO);
    glBindBuffer(GL_ARRAY_BUFFER, seatVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void Wagon::drawSingleSeat(Shader& shader, const glm::mat4& wagonModel, int index) {
    // Layout: 4 rows (Z), 2 columns (X)
    int row = index / 2; // 0 to 3
    int col = index % 2; // 0 to 1

    // Calculate local positions within the wagon
    // Reversed Z logic: Row 0 starts at the front (+Z) and goes back (-Z)
    float xPos = (col == 0) ? -width * 0.25f : width * 0.25f;
    float zPos = depth * 0.35f - (row * (depth * 0.23f));
    float yPos = -height * 0.3f; // Set on the floor

    glm::vec3 seatLocalPos(xPos, yPos, zPos);

    // 1. Cushion (The base)
    glm::mat4 cushionModel = wagonModel;
    cushionModel = glm::translate(cushionModel, seatLocalPos);
    cushionModel = glm::scale(cushionModel, glm::vec3(width * 0.35f, 0.4f, depth * 0.15f));
    shader.setMat4("uM", cushionModel);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // 2. Backrest
    glm::mat4 backModel = wagonModel;
    // Move slightly back (-Z) and up from cushion center
    backModel = glm::translate(backModel, seatLocalPos + glm::vec3(0.0f, height * 0.25f, -depth * 0.075f));
    backModel = glm::scale(backModel, glm::vec3(width * 0.35f, height * 0.5f, 0.2f));
    shader.setMat4("uM", backModel);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

Wagon::SeatTransform Wagon::getSeatWorldTransform(int index) const {
    int row = index / 2;
    int col = index % 2;

    // Local offset (matches the draw logic)
    float xOffset = (col == 0) ? -width * 0.25f : width * 0.25f;
    float zOffset = depth * 0.35f - (row * (depth * 0.23f));
    float yOffset = -height * 0.2f; // Offset slightly up so they sit ON the cushion

    // Combine local offset with wagon orientation
    // Position = WagonPos + (Right * xOffset) + (Up * yOffset) + (Forward * zOffset)
    glm::vec3 worldPos = position + (rightDir * xOffset) + (upDir * yOffset) + (forwardDir * zOffset);

    return { worldPos, forwardDir, upDir };
}
