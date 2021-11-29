#ifndef DEER_LEG_HPP
#define DEER_LEG_HPP

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <GL/glew.h>

#include "../actor.hpp"

using namespace std;

class DeerLeg: public Actor {

public:
    using Actor::Actor;
    void update(float current_time_seconds, float delta_seconds, vector<Actor*> actors);
};

#endif