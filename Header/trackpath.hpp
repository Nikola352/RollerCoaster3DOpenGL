#ifndef TRACKPATH_HPP
#define TRACKPATH_HPP

#include <glm/glm.hpp>
#include <vector>

class Model;

class TrackPath
{
public:
    TrackPath();
    ~TrackPath() = default;

    // Extract center line from track model
    // Assumes track has segments of verticesPerSegment vertices each
    void extractFromModel(const Model& trackModel, int numSegments = 300, int verticesPerSegment = 384);

    // Get position along the track at parameter t [0, 1]
    // Uses Catmull-Rom spline interpolation for smooth movement
    glm::vec3 getPosition(float t) const;

    // Get forward direction (tangent) at parameter t [0, 1]
    glm::vec3 getForward(float t) const;

    // Get up vector at parameter t [0, 1]
    // Approximates the "up" based on track banking
    glm::vec3 getUp(float t) const;

    // Get the right vector at parameter t [0, 1]
    glm::vec3 getRight(float t) const;

    // Get total number of center points extracted
    int getNumPoints() const { return static_cast<int>(centerPoints.size()); }

    // Get raw center point by index
    const glm::vec3& getCenterPoint(int index) const { return centerPoints[index]; }

    // Check if path has been initialized
    bool isInitialized() const { return !centerPoints.empty(); }

private:
    // Catmull-Rom spline interpolation
    glm::vec3 catmullRom(const glm::vec3& p0, const glm::vec3& p1,
                         const glm::vec3& p2, const glm::vec3& p3, float t) const;

    // Get index and local t for a global t value
    void getSegmentInfo(float t, int& index, float& localT) const;

    // Smooth a vector of points using multiple passes of moving average
    void smoothPoints(std::vector<glm::vec3>& points, int passes, int windowSize);

    // Smooth and normalize a vector of direction vectors
    void smoothDirections(std::vector<glm::vec3>& directions, int passes, int windowSize);

    std::vector<glm::vec3> centerPoints;  // Center line points (300 points)
    std::vector<glm::vec3> upVectors;     // Approximate up vectors for banking
};

#endif
