#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <vector>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <GL/glew.h>

using namespace std;

class Camera {
public:
    glm::vec3 location;
    glm::vec3 rotation;

    int width;
    int height;
    float aspect_ratio;

    bool project_orthographic;
    float orthographic_size;
    float orthographic_height;
    float orthographic_width;

    int horizontal_center;
    int vertical_center;

    vector<GLfloat*> lights;

    Camera();
    Camera(glm::vec3 location, glm::vec3 rotation, int width, int height);
};

#endif