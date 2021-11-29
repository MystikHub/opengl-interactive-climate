#include "../actor.hpp"
#include "deer_leg.hpp"

#include <math.h>
#include <vector>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <GL/glew.h>

float leg_turn_range = 1.0f;
float leg_turn_speed = 2.0f;

void DeerLeg::update(float current_time_seconds, float delta_seconds, vector<Actor*> actors) {
    this->rotation.x = sin(current_time_seconds * leg_turn_speed) * leg_turn_range;
}
