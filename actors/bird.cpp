#include "../actor.hpp"
#include "bird.hpp"

#include <math.h>
#include <vector>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <GL/glew.h>

float separation_distance = 3.0f;

void Bird::update(float current_time_seconds, float delta_seconds, vector<Actor*> actors) {
    // Main direction to move in (TODO: remove later)
    glm::vec3 movement_vector = glm::vec3(0.0f, 0.0f, 1.0f);

    // Crowd movement is a combination of three vectors
    glm::vec3 collision_avoidance = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 velocity_matching = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 flock_centering = glm::vec3(0.0f, 0.0f, 0.0f);

    // First, make sure we don't bump into obstacles or other crowd members
    // Get a list of all crows
    // Limit to crows within range
    // Find displacement vector between each crow
    // Invert displacement vectors and find their average to make the
    //  collision_avoidance vector

    // Second, find the average movement vector of the surrounding crowd members
    //  for the velocity_matching vector

    // Lastly, make a vector of the average point (centroid) of the surrounding
    //  crowd members to stay within the middle of the crowd

    // Save this speed
}