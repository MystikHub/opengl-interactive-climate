#ifndef ACTOR_HPP
#define ACTOR_HPP

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <GL/glew.h>

#include "camera.hpp"

using namespace std;

typedef struct {
	size_t mPointCount = 0;
	std::vector<glm::vec3> mVertices;
	std::vector<glm::vec3> mNormals;
	std::vector<glm::vec2> mTextureCoords;
} ModelData;

class Actor {

public:
    Actor* parent;

    glm::vec3 location;
    glm::vec3 rotation;
    glm::vec3 scale;

    ModelData mesh;
    GLuint shaderProgramID;

    Actor(Camera* camera);
    Camera* camera;

    void loadMesh(string mesh_path);
    void setupBufferObjects();
    void renderMesh();
    glm::mat4 getTransform();
};

#endif