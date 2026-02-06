#ifndef WAGON_HPP
#define WAGON_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>

class Shader;
class TrackPath;

class Wagon
{
public:
    // Ride state
    enum class RideState
    {
        STOPPED,    // Not moving
        STARTING,   // Accelerating to cruise speed (chain lift)
        RUNNING     // Normal physics (gravity affects speed)
    };

    Wagon(float width = 10.0f, float height = 5.0f, float depth = 8.0f);
    ~Wagon();

    void init();
    void draw(Shader& shader);

    void setPosition(const glm::vec3& pos);
    glm::vec3 getPosition() const { return position; }

    // Set orientation using forward and up vectors
    void setOrientation(const glm::vec3& forward, const glm::vec3& up);

    // Update wagon position and orientation from track path at parameter t
    void updateFromTrackPath(const TrackPath& path, float t);

    // Physics-based update - call each frame when ride is active
    void updatePhysics(const TrackPath& path, float deltaTime);

    // Ride control
    void startRide();
    void stopRide();
    bool isRideRunning() const { return rideState != RideState::STOPPED; }
    RideState getRideState() const { return rideState; }

    // Get current track parameter
    float getTrackParameter() const { return trackT; }
    void setTrackParameter(float t) { trackT = t; }

    float getVelocity() const { return velocity; }

    void setColor(const glm::vec3& col) { color = col; }

    // Height offset above the track center line
    void setHeightOffset(float offset) { heightOffset = offset; }

    struct SeatTransform {
        glm::vec3 position;
        glm::vec3 forward;
        glm::vec3 up;
    };
    SeatTransform getSeatWorldTransform(int index) const;

private:
    void setupMesh();

    unsigned int VAO, VBO;
    int vertexCount;

    void setupSeatMesh();
    unsigned int seatVAO, seatVBO;
    // Helper to draw a single seat in local space
    void drawSingleSeat(Shader& shader, const glm::mat4& wagonModelMatrix, int index);

    unsigned int textureID;

    float width, height, depth;
    glm::vec3 position;
    glm::vec3 color;

    // Orientation vectors
    glm::vec3 forwardDir;
    glm::vec3 upDir;
    glm::vec3 rightDir;

    float trackT;        // Current parameter on track [0, 1]
    float heightOffset;  // Height above track center

    // Physics state
    RideState rideState;
    float velocity;      // Current velocity in track units per second

    // Physics constants
    static constexpr float CHAIN_LIFT_ACCEL = 0.2f;  // Acceleration during startup
    static constexpr float CRUISE_SPEED = 0.15f;      // Target speed after startup
    static constexpr float GRAVITY_EFFECT = 0.04f;    // How much slope affects acceleration
    static constexpr float FRICTION = 0.01f;          // Friction/drag coefficient
    static constexpr float MIN_VELOCITY = 0.02f;     // Minimum velocity
    static constexpr float MAX_VELOCITY = 0.12f;      // Maximum velocity cap
};

#endif
