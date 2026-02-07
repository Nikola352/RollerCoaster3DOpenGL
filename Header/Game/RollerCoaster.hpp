#pragma once

#include <vector>
#include "GameState.hpp"
#include "Person.hpp"

class Wagon;
class TrackPath;

class RollerCoaster {
private:
    Wagon& wagon;
    const TrackPath& trackPath;
    GameState gameState;
    float cooldownTimer;
    bool passedMidpoint;  // Track loop detection
    std::vector<Person> passengers;

    bool allPassengersBuckled() const;
    bool hasPassengers() const;
    int findFirstEmptySeat() const;
    Person* findPassengerBySeat(int seatIndex);
    const Person* findPassengerBySeat(int seatIndex) const;

public:
    RollerCoaster(Wagon& wagon, const TrackPath& trackPath);

    void update(float deltaTime);

    // Input handlers
    void handleAddPassenger();         // SPACE key
    void handleSeatAction(int index);  // Keys 1-8 (0-indexed internally)
    void handleStartRide();            // ENTER key

    // Queries
    GameState getState() const;
    size_t getPassengerCount() const;
    const std::vector<Person>& getPassengers() const;
    bool isSeatOccupied(int seatIndex) const;
    const Person* getPassengerBySeat(int seatIndex) const;  // Public accessor for camera passenger check
};
