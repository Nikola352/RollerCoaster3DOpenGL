#include "../../Header/Game/Constants.hpp"

const size_t MAX_PASSENGERS = 8;
const float START_TRACK_T = 0.25f;          // Starting position on track
const float END_TRACK_T = 0.24f;            // Loop completion (just before start)
const float MAX_START_VELOCITY = 0.15f;     // Cruise speed threshold
const float START_ACCELERATION = 0.2f;      // Chain lift acceleration
const float SLOWDOWN_DECELERATION = -0.1f;  // Deceleration when sick
const float REVERSE_VELOCITY = -0.05f;      // Reverse speed (negative)
const float COOLDOWN_DURATION = 2.0f;       // Seconds to wait before reverse
