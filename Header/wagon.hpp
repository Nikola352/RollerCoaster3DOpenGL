#ifndef WAGON_HPP
#define WAGON_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>

class Shader;
class TrackPath;

class Wagon
{
public:
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

    // Get current track parameter
    float getTrackParameter() const { return trackT; }
    void setTrackParameter(float t) { trackT = t; }

    void setColor(const glm::vec3& col) { color = col; }

    // Height offset above the track center line
    void setHeightOffset(float offset) { heightOffset = offset; }

private:
    void setupMesh();

    unsigned int VAO, VBO;
    int vertexCount;

    float width, height, depth;
    glm::vec3 position;
    glm::vec3 color;

    // Orientation vectors
    glm::vec3 forwardDir;
    glm::vec3 upDir;
    glm::vec3 rightDir;

    float trackT;        // Current parameter on track [0, 1]
    float heightOffset;  // Height above track center
};

#endif
