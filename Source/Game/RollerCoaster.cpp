#include "../../Header/Game/RollerCoaster.hpp"
#include "../../Header/Game/Constants.hpp"
#include "../../Header/wagon.hpp"
#include "../../Header/trackpath.hpp"
#include <iostream>

RollerCoaster::RollerCoaster(Wagon& wagon, const TrackPath& trackPath)
    : wagon(wagon), trackPath(trackPath), gameState(GameState::ONBOARDING), cooldownTimer(0.0f), passedMidpoint(false)
{
    wagon.setTrackParameter(START_TRACK_T);
    wagon.updateFromTrackPath(trackPath, START_TRACK_T);
}

bool RollerCoaster::allPassengersBuckled() const {
    for (const auto& p : passengers) {
        if (!p.getHasSeatbelt()) {
            return false;
        }
    }
    return true;
}

bool RollerCoaster::hasPassengers() const {
    return !passengers.empty();
}

int RollerCoaster::findFirstEmptySeat() const {
    // Check seats 0-7, return first that's not occupied
    for (int i = 0; i < static_cast<int>(MAX_PASSENGERS); ++i) {
        bool occupied = false;
        for (const auto& p : passengers) {
            if (p.getSeatIndex() == i) {
                occupied = true;
                break;
            }
        }
        if (!occupied) {
            return i;
        }
    }
    return -1; // All seats full
}

Person* RollerCoaster::findPassengerBySeat(int seatIndex) {
    for (auto& p : passengers) {
        if (p.getSeatIndex() == seatIndex) {
            return &p;
        }
    }
    return nullptr;
}

const Person* RollerCoaster::findPassengerBySeat(int seatIndex) const {
    for (const auto& p : passengers) {
        if (p.getSeatIndex() == seatIndex) {
            return &p;
        }
    }
    return nullptr;
}

void RollerCoaster::update(float deltaTime) {
    switch (gameState) {
    case GameState::ONBOARDING:
        // Nothing to update, waiting for user input
        break;

    case GameState::TAKEOFF:
        // Wagon is accelerating via chain lift (handled by Wagon::updatePhysics in STARTING mode)
        if (wagon.getVelocity() >= MAX_START_VELOCITY) {
            // Transition to normal ride
            gameState = GameState::RIDE;
            std::cout << "STATE: RIDE" << std::endl;
        }
        break;

    case GameState::RIDE:
        {
            // Check if we've completed the loop (track t wraps around)
            float currentT = wagon.getTrackParameter();
            // When we pass END_TRACK_T coming from higher values, we've completed a loop
            // Since the track loops at 1.0, we check if we're close to where we started
            // and have been riding for a while (velocity > 0 means we're moving forward)

            // Simple approach: if we've gone past 0.5 and come back near START_TRACK_T
            // Track when we pass the midpoint to detect loop completion
            if (currentT > 0.5f) {
                passedMidpoint = true;
            }
            if (passedMidpoint && currentT < 0.3f && currentT >= END_TRACK_T) {
                // We've looped around
                passedMidpoint = false;
                wagon.setConstantVelocity(REVERSE_VELOCITY);
                gameState = GameState::REVERSE;
                std::cout << "STATE: REVERSE (loop complete)" << std::endl;
            }
        }
        break;

    case GameState::SLOWDOWN:
        if (wagon.getVelocity() <= 0.0f) {
            wagon.stop();
            cooldownTimer = COOLDOWN_DURATION;
            gameState = GameState::COOLDOWN;
            std::cout << "STATE: COOLDOWN" << std::endl;
        }
        break;

    case GameState::COOLDOWN:
        cooldownTimer -= deltaTime;
        if (cooldownTimer <= 0.0f) {
            wagon.setConstantVelocity(REVERSE_VELOCITY);
            gameState = GameState::REVERSE;
            std::cout << "STATE: REVERSE (after cooldown)" << std::endl;
        }
        break;

    case GameState::REVERSE:
        {
            float currentT = wagon.getTrackParameter();
            // Going backwards (negative velocity), so t decreases
            // Need to handle wrap-around: if t goes below 0, it wraps to ~1.0
            // We want to stop when we reach START_TRACK_T

            // If we're moving backwards and reach/pass the start position
            if (currentT <= START_TRACK_T || currentT > 0.9f) {
                // Check if we wrapped around (t > 0.9 means we went from ~0 to ~1)
                if (currentT > 0.9f) {
                    // We wrapped, keep going
                } else {
                    // We've reached the start
                    wagon.setTrackParameter(START_TRACK_T);
                    wagon.updateFromTrackPath(trackPath, START_TRACK_T);
                    wagon.stop();

                    // Unbuckle all passengers
                    for (auto& p : passengers) {
                        p.setHasSeatbelt(false);
                    }

                    gameState = GameState::OFFBOARDING;
                    std::cout << "STATE: OFFBOARDING" << std::endl;
                }
            }
        }
        break;

    case GameState::OFFBOARDING:
        // Nothing to update, waiting for user to remove passengers
        break;
    }
}

void RollerCoaster::handleAddPassenger() {
    if (gameState != GameState::ONBOARDING) {
        return;
    }

    if (passengers.size() >= MAX_PASSENGERS) {
        std::cout << "All seats are full!" << std::endl;
        return;
    }

    int seatIndex = findFirstEmptySeat();
    if (seatIndex < 0) {
        std::cout << "No empty seat found!" << std::endl;
        return;
    }

    passengers.emplace_back(seatIndex);
    std::cout << "Passenger added to seat " << (seatIndex + 1) << std::endl;
}

void RollerCoaster::handleSeatAction(int index) {
    // index is 0-7 (from keys 1-8)
    if (index < 0 || index >= static_cast<int>(MAX_PASSENGERS)) {
        return;
    }

    Person* passenger = findPassengerBySeat(index);

    if (gameState == GameState::ONBOARDING) {
        // Toggle seatbelt
        if (passenger) {
            passenger->setHasSeatbelt(!passenger->getHasSeatbelt());
            std::cout << "Passenger " << (index + 1) << " seatbelt: "
                      << (passenger->getHasSeatbelt() ? "ON" : "OFF") << std::endl;
        }
    }
    else if (gameState == GameState::OFFBOARDING) {
        // Remove passenger
        if (passenger) {
            // Find and erase the passenger
            for (auto it = passengers.begin(); it != passengers.end(); ++it) {
                if (it->getSeatIndex() == index) {
                    passengers.erase(it);
                    std::cout << "Passenger " << (index + 1) << " removed" << std::endl;
                    break;
                }
            }

            // Check if all passengers removed
            if (passengers.empty()) {
                gameState = GameState::ONBOARDING;
                std::cout << "STATE: ONBOARDING" << std::endl;
            }
        }
    }
    else if (gameState == GameState::RIDE || gameState == GameState::TAKEOFF) {
        // Signal passenger is sick
        if (passenger) {
            passenger->setIsSick(true);
            std::cout << "Passenger " << (index + 1) << " is SICK!" << std::endl;

            // TODO: Render sick passenger with green color
            // TODO: If this is the camera passenger, apply green screen filter

            // Start slowing down
            wagon.setDeceleration(SLOWDOWN_DECELERATION);
            gameState = GameState::SLOWDOWN;
            std::cout << "STATE: SLOWDOWN" << std::endl;
        }
    }
}

void RollerCoaster::handleStartRide() {
    if (gameState != GameState::ONBOARDING) {
        return;
    }

    if (!hasPassengers()) {
        std::cout << "Cannot start: No passengers!" << std::endl;
        return;
    }

    if (!allPassengersBuckled()) {
        std::cout << "Cannot start: Not all passengers buckled!" << std::endl;
        return;
    }

    wagon.startRide();
    passedMidpoint = false;  // Reset loop detection
    gameState = GameState::TAKEOFF;
    std::cout << "STATE: TAKEOFF" << std::endl;
}

GameState RollerCoaster::getState() const {
    return gameState;
}

size_t RollerCoaster::getPassengerCount() const {
    return passengers.size();
}

const std::vector<Person>& RollerCoaster::getPassengers() const {
    return passengers;
}

bool RollerCoaster::isSeatOccupied(int seatIndex) const {
    return findPassengerBySeat(seatIndex) != nullptr;
}
