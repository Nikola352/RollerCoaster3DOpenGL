#include "../Header/trackpath.hpp"
#include "../Header/model.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

TrackPath::TrackPath()
{
}

void TrackPath::extractFromModel(const Model& trackModel, int numSegments, int verticesPerSegment)
{
    centerPoints.clear();
    upVectors.clear();

    // Collect all vertices from all meshes
    std::vector<glm::vec3> allVertices;
    for (const auto& mesh : trackModel.meshes)
    {
        for (const auto& vertex : mesh.vertices)
        {
            allVertices.push_back(vertex.Position);
        }
    }

    std::cout << "TrackPath: Total vertices in model: " << allVertices.size() << std::endl;

    if (allVertices.size() < static_cast<size_t>(numSegments * verticesPerSegment))
    {
        std::cerr << "TrackPath: Not enough vertices! Expected "
                  << (numSegments * verticesPerSegment) << ", got " << allVertices.size() << std::endl;
        // Adjust if needed
        numSegments = static_cast<int>(allVertices.size()) / verticesPerSegment;
        if (numSegments < 2)
        {
            std::cerr << "TrackPath: Cannot extract path - too few vertices" << std::endl;
            return;
        }
    }

    // For each segment, calculate the center point (mean of vertices)
    centerPoints.reserve(numSegments);
    for (int seg = 0; seg < numSegments; ++seg)
    {
        glm::vec3 center(0.0f);
        int startIdx = seg * verticesPerSegment;

        for (int v = 0; v < verticesPerSegment; ++v)
        {
            center += allVertices[startIdx + v];
        }
        center /= static_cast<float>(verticesPerSegment);
        centerPoints.push_back(center);
    }

    std::cout << "TrackPath: Extracted " << centerPoints.size() << " center points" << std::endl;

    // Smooth the center points to remove noise from mesh averaging
    smoothPoints(centerPoints, 3, 5);  // 3 passes with window size 5

    std::cout << "TrackPath: Smoothed center points" << std::endl;

    // Calculate up vectors based on track geometry
    // For each point, estimate up as perpendicular to forward and a reference right
    upVectors.reserve(numSegments);
    for (int i = 0; i < numSegments; ++i)
    {
        // Get forward direction using neighbors for smoother result
        glm::vec3 forward;
        int prev = (i - 1 + numSegments) % numSegments;
        int next = (i + 1) % numSegments;
        forward = glm::normalize(centerPoints[next] - centerPoints[prev]);

        // Use world up as reference, compute right, then recompute up
        glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
        glm::vec3 right = glm::cross(forward, worldUp);

        // If forward is nearly parallel to worldUp, use a different reference
        float rightLen = glm::length(right);
        if (rightLen < 0.001f)
        {
            right = glm::vec3(1.0f, 0.0f, 0.0f);
        }
        else
        {
            right = right / rightLen;
        }

        glm::vec3 up = glm::normalize(glm::cross(right, forward));
        upVectors.push_back(up);
    }

    // Smooth the up vectors to prevent wobbling
    smoothDirections(upVectors, 5, 7);  // 5 passes with window size 7

    std::cout << "TrackPath: Smoothed up vectors" << std::endl;

    // Print some debug info about the track bounds
    glm::vec3 minBounds(FLT_MAX), maxBounds(-FLT_MAX);
    for (const auto& p : centerPoints)
    {
        minBounds = glm::min(minBounds, p);
        maxBounds = glm::max(maxBounds, p);
    }
    std::cout << "TrackPath: Bounds - Min(" << minBounds.x << ", " << minBounds.y << ", " << minBounds.z
              << ") Max(" << maxBounds.x << ", " << maxBounds.y << ", " << maxBounds.z << ")" << std::endl;
}

void TrackPath::getSegmentInfo(float t, int& index, float& localT) const
{
    if (centerPoints.empty())
    {
        index = 0;
        localT = 0.0f;
        return;
    }

    // Clamp t to [0, 1]
    t = glm::clamp(t, 0.0f, 1.0f);

    // Map t to segment index
    float scaledT = t * (centerPoints.size() - 1);
    index = static_cast<int>(scaledT);
    localT = scaledT - index;

    // Handle edge case at t = 1
    if (index >= static_cast<int>(centerPoints.size()) - 1)
    {
        index = static_cast<int>(centerPoints.size()) - 2;
        localT = 1.0f;
    }
}

glm::vec3 TrackPath::catmullRom(const glm::vec3& p0, const glm::vec3& p1,
                                 const glm::vec3& p2, const glm::vec3& p3, float t) const
{
    float t2 = t * t;
    float t3 = t2 * t;

    // Catmull-Rom spline formula
    glm::vec3 result = 0.5f * (
        (2.0f * p1) +
        (-p0 + p2) * t +
        (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
        (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3
    );

    return result;
}

glm::vec3 TrackPath::getPosition(float t) const
{
    if (centerPoints.empty())
    {
        return glm::vec3(0.0f);
    }

    if (centerPoints.size() == 1)
    {
        return centerPoints[0];
    }

    int index;
    float localT;
    getSegmentInfo(t, index, localT);

    int n = static_cast<int>(centerPoints.size());

    // Get 4 control points for Catmull-Rom (handle boundaries)
    int i0 = (index - 1 + n) % n;
    int i1 = index;
    int i2 = (index + 1) % n;
    int i3 = (index + 2) % n;

    return catmullRom(centerPoints[i0], centerPoints[i1],
                      centerPoints[i2], centerPoints[i3], localT);
}

glm::vec3 TrackPath::getForward(float t) const
{
    if (centerPoints.size() < 2)
    {
        return glm::vec3(0.0f, 0.0f, 1.0f);
    }

    // Calculate tangent by getting positions slightly before and after
    float delta = 0.001f;
    float t1 = glm::clamp(t - delta, 0.0f, 1.0f);
    float t2 = glm::clamp(t + delta, 0.0f, 1.0f);

    glm::vec3 p1 = getPosition(t1);
    glm::vec3 p2 = getPosition(t2);

    glm::vec3 forward = p2 - p1;
    float len = glm::length(forward);

    if (len > 0.0001f)
    {
        return forward / len;
    }

    return glm::vec3(0.0f, 0.0f, 1.0f);
}

glm::vec3 TrackPath::getUp(float t) const
{
    if (upVectors.empty())
    {
        return glm::vec3(0.0f, 1.0f, 0.0f);
    }

    if (upVectors.size() == 1)
    {
        return upVectors[0];
    }

    int index;
    float localT;
    getSegmentInfo(t, index, localT);

    int n = static_cast<int>(upVectors.size());

    // Use Catmull-Rom interpolation for smooth up vector transitions
    int i0 = (index - 1 + n) % n;
    int i1 = index;
    int i2 = (index + 1) % n;
    int i3 = (index + 2) % n;

    glm::vec3 up = catmullRom(upVectors[i0], upVectors[i1],
                               upVectors[i2], upVectors[i3], localT);
    return glm::normalize(up);
}

glm::vec3 TrackPath::getRight(float t) const
{
    glm::vec3 forward = getForward(t);
    glm::vec3 up = getUp(t);
    return glm::normalize(glm::cross(forward, up));
}

void TrackPath::smoothPoints(std::vector<glm::vec3>& points, int passes, int windowSize)
{
    if (points.size() < 3) return;

    int n = static_cast<int>(points.size());
    int halfWindow = windowSize / 2;

    for (int pass = 0; pass < passes; ++pass)
    {
        std::vector<glm::vec3> smoothed(n);

        for (int i = 0; i < n; ++i)
        {
            glm::vec3 sum(0.0f);
            float totalWeight = 0.0f;

            for (int j = -halfWindow; j <= halfWindow; ++j)
            {
                // Use modulo for closed loop track
                int idx = (i + j + n) % n;

                // Gaussian-like weight (closer points have more influence)
                float weight = 1.0f / (1.0f + std::abs(j) * 0.5f);
                sum += points[idx] * weight;
                totalWeight += weight;
            }

            smoothed[i] = sum / totalWeight;
        }

        points = smoothed;
    }
}

void TrackPath::smoothDirections(std::vector<glm::vec3>& directions, int passes, int windowSize)
{
    if (directions.size() < 3) return;

    int n = static_cast<int>(directions.size());
    int halfWindow = windowSize / 2;

    for (int pass = 0; pass < passes; ++pass)
    {
        std::vector<glm::vec3> smoothed(n);

        for (int i = 0; i < n; ++i)
        {
            glm::vec3 sum(0.0f);
            float totalWeight = 0.0f;

            for (int j = -halfWindow; j <= halfWindow; ++j)
            {
                // Use modulo for closed loop track
                int idx = (i + j + n) % n;

                // Gaussian-like weight
                float weight = 1.0f / (1.0f + std::abs(j) * 0.5f);
                sum += directions[idx] * weight;
                totalWeight += weight;
            }

            // Normalize since we're averaging direction vectors
            smoothed[i] = glm::normalize(sum / totalWeight);
        }

        directions = smoothed;
    }
}
