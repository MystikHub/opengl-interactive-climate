#ifndef ACTOR_HPP
#define ACTOR_HPP

#include <chrono>
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

    unsigned int vertex_positions_vbo_id;
    unsigned int vertex_normals_vbo_id;
    unsigned int vertex_texture_vbo_id;
    unsigned int texture_id;
    unsigned int attr_positions;
    unsigned int attr_normals;
    unsigned int attr_texture;
    unsigned int vao_id;

    string diffuse_texture;
    string normal_texture;
    float specularity;

    Actor(Camera* camera);
    Camera* camera;

    void update(chrono::time_point current_time, float delta_seconds);
    void loadMesh(string mesh_path);
    void setupBufferObjects();
    void renderMesh();
    glm::mat4 getTransform();
};

#endif