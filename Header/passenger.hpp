#ifndef PASSENGER_HPP
#define PASSENGER_HPP

#include <string>
#include <glm/glm.hpp>

class Model;
class Shader;
class Wagon;

class Passenger
{
public:
    Passenger(const std::string& modelPath, int seatIndex);
    ~Passenger();

    void draw(Shader& shader, const Wagon& wagon);

    int getSeatIndex() const { return seatIndex; }

    // Seatbelt state
    bool isBuckled() const { return buckled; }
    void setBuckled(bool value) { buckled = value; }
    void toggleBuckled() { buckled = !buckled; }

    // Sick state
    bool isSick() const { return sick; }
    void setSick(bool value) { sick = value; }

private:
    Model* model;
    int seatIndex;
    bool buckled = false;
    bool sick = false;

    // Seatbelt rendering
    unsigned int seatbeltVAO = 0;
    unsigned int seatbeltVBO = 0;
    static unsigned int seatbeltTextureID;
    static bool seatbeltTextureLoaded;
    void setupSeatbeltMesh();
    void drawSeatbelt(Shader& shader, const Wagon& wagon);

    // Tuning parameters
    static constexpr float SCALE = 5.0f;
    static constexpr float Y_OFFSET = 3.0f;

    glm::mat4 calculateModelMatrix(const Wagon& wagon) const;
};

#endif
