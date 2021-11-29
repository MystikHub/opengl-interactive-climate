#include "actor.hpp"
#include "camera.hpp"

#include <chrono>
#include <stdlib.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <GL/glew.h>

// Assimp includes
#include <assimp/Importer.hpp> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

// Texture loading
#define STB_IMAGE_IMPLEMENTATION
#include "headers/stb_image.h"

unsigned int load_cubemap(vector<std::string> faces);

Actor::Actor(Camera* camera) {
	this->parent = NULL;
	this->camera = camera;

	this->location = glm::vec3(0.0f, 0.0f, 0.0f);
	this->rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	this->scale = glm::vec3(1.0f, 1.0f, 1.0f);

	this->specularity = 64;
	this->diffuse_texture = "";
	this->actor_type = ActorType::Default;

	this->prev_direction = glm::vec3(0.0f, 0.0f, 0.0f);
	this->prev_direction.x = rand();
	this->prev_direction.y = rand();
	this->prev_direction.z = rand();
	this->prev_direction = glm::normalize(this->prev_direction);
}

void Actor::loadMesh(string file_name) {
	ModelData modelData;

	/* Use assimp to read the model file, forcing it to be read as    */
	/* triangles. The second flag (aiProcess_PreTransformVertices) is */
	/* relevant if there are multiple meshes in the model file that   */
	/* are offset from the origin. This is pre-transform them so      */
	/* they're in the right position.                                 */
    Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(
		file_name.c_str(), 
		aiProcess_Triangulate
        | aiProcess_PreTransformVertices
	); 

	if (!scene) {
		fprintf(stderr, "ERROR: reading mesh %s\n", file_name.c_str());
		this->mesh = new ModelData();
	}

	printf("  %i materials\n", scene->mNumMaterials);
	printf("  %i meshes\n", scene->mNumMeshes);
	printf("  %i textures\n", scene->mNumTextures);

	this->mesh = new ModelData;
	for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
		const aiMesh* mesh = scene->mMeshes[m_i];
		printf("    %i vertices in mesh\n", mesh->mNumVertices);
		this->mesh->mPointCount += mesh->mNumVertices;
		for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
			if (mesh->HasPositions()) {
				const aiVector3D* vp = &(mesh->mVertices[v_i]);
				this->mesh->mVertices.push_back(glm::vec3(vp->x, vp->y, vp->z));
			}
			if (mesh->HasNormals()) {
				const aiVector3D* vn = &(mesh->mNormals[v_i]);
				this->mesh->mNormals.push_back(glm::vec3(vn->x, vn->y, vn->z));
			}
			if (mesh->HasTextureCoords(0)) {
				const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
				this->mesh->mTextureCoords.push_back(glm::vec2(vt->x, vt->y));
			}
			if (mesh->HasTangentsAndBitangents()) {
				/* You can extract tangents and bitangents here              */
				/* Note that you might need to make Assimp generate this     */
				/* data for you. Take a look at the flags that aiImportFile  */
				/* can take.                                                 */
			}
		}
	}

	// Add lights
	for(unsigned int i = 0; i < scene->mNumLights; i++) {
		aiVector3D l = scene->mLights[i]->mPosition;
		GLfloat* light_coordinates = new GLfloat[3];
		light_coordinates[0] = l.x;
		light_coordinates[1] = l.y;
		light_coordinates[2] = l.z;
		this->camera->lights.push_back(light_coordinates);
	}
}

void Actor::setupBufferObjects() {
	unsigned int vertex_positions_vbo_id = 0;
	GLuint loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
	GLuint loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
	GLuint loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");
	this->attr_positions = loc1;
	this->attr_normals = loc2;
	this->attr_texture = loc3;

    glGenBuffers(1, &vertex_positions_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_positions_vbo_id);
	glBufferData(GL_ARRAY_BUFFER, this->mesh->mPointCount * sizeof(glm::vec3), &this->mesh->mVertices[0], GL_STATIC_DRAW);
	this->vertex_positions_vbo_id = vertex_positions_vbo_id;

    unsigned int vertex_normals_vbo_id = 0;
    glGenBuffers(1, &vertex_normals_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_normals_vbo_id);
	glBufferData(GL_ARRAY_BUFFER, this->mesh->mPointCount * sizeof(glm::vec3), &this->mesh->mNormals[0], GL_STATIC_DRAW);
	this->vertex_normals_vbo_id = vertex_normals_vbo_id;

    unsigned int vertex_texture_vbo_id = 0;
	glGenBuffers(1, &vertex_texture_vbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_texture_vbo_id);
	glBufferData(GL_ARRAY_BUFFER, this->mesh->mPointCount * sizeof (glm::vec2), &this->mesh->mTextureCoords[0], GL_STATIC_DRAW);
	this->vertex_texture_vbo_id = vertex_texture_vbo_id;
    
	unsigned int vao = 0;
	glBindVertexArray(vao);
	this->vao_id = vao;

    glEnableVertexAttribArray(loc1);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_positions_vbo_id);
    glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(loc2);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_normals_vbo_id);
    glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	// Texturessssssssssssss
	unsigned int texture;
	glGenTextures(1, &texture);
	this->texture_id = texture;
	glBindTexture(GL_TEXTURE_2D, texture);
	// Set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Load and generate the texture
	int width, height, nrChannels;
	unsigned char *data = stbi_load(this->diffuse_texture.c_str(), &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		printf("Failed to load texture %s\n", this->diffuse_texture.c_str());
	}
	stbi_image_free(data);

	glEnableVertexAttribArray(loc3);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_texture_vbo_id);
	glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	// Skybox
	vector<string> faces = {
		"materials/cubemap/posx.jpg",
		"materials/cubemap/negx.jpg",
		"materials/cubemap/posy.jpg",
		"materials/cubemap/negy.jpg",
		"materials/cubemap/posz.jpg",
		"materials/cubemap/negz.jpg"
	};
	// unsigned int cubemap_texture = load_cubemap(faces);
}

void Actor::renderMesh() {
	// Enable the data we need
	glEnableVertexAttribArray(this->attr_positions);
	glBindBuffer(GL_ARRAY_BUFFER, this->vertex_positions_vbo_id);
	glVertexAttribPointer(this->attr_positions, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(this->attr_normals);
	glBindBuffer(GL_ARRAY_BUFFER, this->vertex_normals_vbo_id);
	glVertexAttribPointer(this->attr_normals, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindTexture(GL_TEXTURE_2D, this->texture_id);
	glEnableVertexAttribArray(this->attr_texture);
	glBindBuffer(GL_ARRAY_BUFFER, this->vertex_texture_vbo_id);
	glVertexAttribPointer(this->attr_texture, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindVertexArray(this->vao_id);

    // Declare your uniform variables that will be used in your shader
    int matrix_location = glGetUniformLocation(this->shaderProgramID, "model");
    int view_mat_location = glGetUniformLocation(this->shaderProgramID, "view");
    int proj_mat_location = glGetUniformLocation(this->shaderProgramID, "proj");

	// Set the light positions
	// TODO: Move this to the model loading stage
	GLfloat* lights = new GLfloat[this->camera->lights.size() * 3];
	for(unsigned int i = 0; i < this->camera->lights.size(); i++) {
		lights[i * 3] = this->camera->lights[i][0];
		lights[(i * 3) + 1] = this->camera->lights[i][1];
		lights[(i * 3) + 2] = this->camera->lights[i][2];
	}
	int light_positions = glGetUniformLocation(this->shaderProgramID, "light_positions");
	glUniform3fv(light_positions, this->camera->lights.size(), &lights[0]);

	// Pass the light positions
	int n_lights = glGetUniformLocation(this->shaderProgramID, "n_lights");
	GLint light_count = this->camera->lights.size();
	glUniform1i(n_lights, light_count);

	// Pass the camera location
	int camera_location = glGetUniformLocation(this->shaderProgramID, "camera_location");
	glUniform3f(camera_location, -this->camera->location.x, -this->camera->location.y, -this->camera->location.z);

	// Pass the material's specularity
	float specularity = glGetUniformLocation(this->shaderProgramID, "specularity");
	glUniform1f(specularity, this->specularity);
    
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::rotate(view, this->camera->rotation.y, glm::vec3(1.0f, 0.0f, 0.0f));
    view = glm::rotate(view, this->camera->rotation.x, glm::vec3(0.0f, 1.0f, 0.0f));
    view = glm::translate(view, this->camera->location);

    // Different projection types
    glm::mat4 projection;
    if(this->camera->project_orthographic) {
        projection = glm::ortho(this->camera->orthographic_width / -2,
			this->camera->orthographic_width / 2,
			this->camera->orthographic_height / -2,
			this->camera->orthographic_width / 2,
			(float) 0.1, (float) pow(10, 20));
	} else {
        projection = glm::perspective((float) 45.0, this->camera->aspect_ratio, (float) 0.1, (float) pow(10, 20));
	}

	// Follow hierarchies
	glm::mat4 model = this->getTransform();

    glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(matrix_location, 1, GL_FALSE, &model[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, this->mesh->mPointCount);
}

glm::mat4 Actor::getTransform() {
	// (reverse?) recursive function
	// Get the accumulated transform of all parents and apply this actor's
	//	transformation
	//
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, this->location);
    model = glm::rotate(model, this->rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, this->rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, this->rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, this->scale);
	
	if(this->parent != NULL) {
		glm::mat4 parentTransform = this->parent->getTransform();
		model = parentTransform * model;
	}

	return model;
}

// Empty function, made useful by subclasses
void Actor::update(float current_time_seconds, float delta_seconds, vector<Actor*> actors) {

}

unsigned int load_cubemap(vector<std::string> faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        } else {
            printf("Cubemap tex failed to load at path: %s\n", faces[i]);
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

float skybox_vertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};