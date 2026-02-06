#pragma once

enum class GameState {
    ONBOARDING,  // Adding passengers, buckling seatbelts
    TAKEOFF,     // Accelerating to cruise speed
    RIDE,        // Normal physics ride
    SLOWDOWN,    // Decelerating after sick signal
    COOLDOWN,    // Waiting before reverse
    REVERSE,     // Going back to start
    OFFBOARDING  // Removing passengers
};
