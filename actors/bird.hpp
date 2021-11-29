#ifndef BIRD_HPP
#define BIRD_HPP

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <GL/glew.h>

#include "../actor.hpp"

using namespace std;

class Bird: public Actor {

public:
    using Actor::Actor;

    // Used by other crowd members to find the next place to move
    glm::vec3 prev_direction;
    void update(float current_time_seconds, float delta_seconds, vector<Actor*> actors);
};

#endif