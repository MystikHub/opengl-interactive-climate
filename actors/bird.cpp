#include "../actor.hpp"
#include "bird.hpp"

#include <math.h>
#include <vector>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <GL/glew.h>

float separation_distance = 0.5f;
float bird_speed = 2.0f;

void Bird::update(float current_time_seconds, float delta_seconds, vector<Actor*> actors) {
    // Main direction to move in
    glm::vec3 movement_vector = glm::vec3(0.0f, 0.0f, 1.0f);

    // Crowd movement is a combination of three vectors
    glm::vec3 collision_avoidance = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 velocity_matching = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 flock_centering = glm::vec3(0.0f, 0.0f, 0.0f);

    // First, make sure we don't bump into obstacles or other bird members
    // Get a list of all birds within a pre-defined distance
    vector<Actor*> close_birds;

    for(unsigned int i = 0; i < actors.size(); i++) {
        if(actors[i]->actor_type == ActorType::BirdCrowd) {
            if(glm::length(actors[i]->location - this->location)) {
                close_birds.push_back(actors[i]);
            }
        }
    }

    // Find displacement vector between each bird
    vector<glm::vec3> displacements;
    for(unsigned int i = 0; i < close_birds.size(); i++) {
        displacements.push_back(close_birds[i]->location - this->location);
    }

    // Add any obstacle actors within the separation distance
    // Also add inverted displacement vectors and find their average to make the
    //  collision_avoidance vector
    for(unsigned int i = 0; i < actors.size(); i++) {
        if(actors[i]->actor_type == ActorType::CrowdObstacle) {
            float obstacle_distance = glm::length(actors[i]->location - this->location);
            if(obstacle_distance <= separation_distance) {
                displacements.push_back(actors[i]->location - this->location);
            }
        }
    }
    for(unsigned int i = 0; i < displacements.size(); i++) {
        displacements[i] = -displacements[i];
    }

    glm::vec3 total_displacement = glm::vec3(0.0f, 0.0f, 0.0f);
    for(unsigned int i = 0; i < displacements.size(); i++) {
        total_displacement += displacements[i];
    }

    collision_avoidance = total_displacement / (float) close_birds.size();

    // Second, find the average movement vector of the surrounding bird members
    //  for the velocity_matching vector
    glm::vec3 total_surrounding_movement = glm::vec3(0.0f, 0.0f, 0.0f);
    for(unsigned int i = 0; i < close_birds.size(); i++) {
        total_surrounding_movement += close_birds[i]->prev_direction;
    }
    velocity_matching = total_surrounding_movement / (float) close_birds.size();

    // Lastly, make a vector of the average point (centroid) of the surrounding
    //  crowd members to stay within the middle of the crowd
    glm::vec3 total_positions = glm::vec3(0.0f, 0.0f, 0.0f);
    for(unsigned int i = 0; i < close_birds.size(); i++) {
        total_positions += close_birds[i]->location;
    }
    glm::vec3 flock_centroid = total_positions / (float) close_birds.size();
    glm::vec3 to_flock_centroid = flock_centroid - this->location;

    movement_vector += (0.3f * collision_avoidance) + (0.3f * velocity_matching) + (0.3f * to_flock_centroid);

    // Add in a random direction
	glm::vec3 random_direction = glm::vec3(0.0f, 0.0f, 0.0f);
	random_direction.x = rand();
	random_direction.y = rand();
	random_direction.z = rand();
	random_direction = glm::normalize(random_direction);
    movement_vector += random_direction * 2.0f;

    // Move in that direction
    this->location += movement_vector * delta_seconds * bird_speed;

    // Save this speed
    this->prev_direction = movement_vector;
}