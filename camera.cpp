#include "camera.hpp"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

Camera::Camera() {
    this->location = glm::vec3(0.0f, 0.0f, -30.0f);
    this->rotation = glm::vec3(0.0f, 0.0f, -0.0f);

    this->width = 1920;
    this->height = 1080;
    // this->width = 1920;
    // this->height = 1080;

    this->aspect_ratio = (float) width / (float) height;
    this->horizontal_center = width / 2;
    this->vertical_center = height / 2;

    this->project_orthographic = false;
    this->orthographic_size = 30;
    this->orthographic_height = orthographic_size;
    this->orthographic_width = orthographic_size * this->aspect_ratio;
}

Camera::Camera(glm::vec3 location, glm::vec3 rotation, int width, int height) {
    this->location = location;
    this->rotation = rotation;

    this->width = width;
    this->height = height;

    this->aspect_ratio = (float) width / (float) height;
    this->horizontal_center = width / 2;
    this->vertical_center = height / 2;

    this->project_orthographic = false;
    this->orthographic_size = 30;
    this->orthographic_height = orthographic_size;
    this->orthographic_width = orthographic_size * this->aspect_ratio;
}