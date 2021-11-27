#include "actor.hpp"
#include "camera.hpp"

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

Actor::Actor(Camera* camera) {
	this->parent = NULL;
	this->camera = camera;

	this->location = glm::vec3(0.0f, 0.0f, 0.0f);
	this->rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	this->scale = glm::vec3(1.0f, 1.0f, 1.0f);
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
		this->mesh = modelData;
	}

	printf("  %i materials\n", scene->mNumMaterials);
	printf("  %i meshes\n", scene->mNumMeshes);
	printf("  %i textures\n", scene->mNumTextures);

	for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
		const aiMesh* mesh = scene->mMeshes[m_i];
		printf("    %i vertices in mesh\n", mesh->mNumVertices);
		modelData.mPointCount += mesh->mNumVertices;
		for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
			if (mesh->HasPositions()) {
				const aiVector3D* vp = &(mesh->mVertices[v_i]);
				modelData.mVertices.push_back(glm::vec3(vp->x, vp->y, vp->z));
			}
			if (mesh->HasNormals()) {
				const aiVector3D* vn = &(mesh->mNormals[v_i]);
				modelData.mNormals.push_back(glm::vec3(vn->x, vn->y, vn->z));
			}
			if (mesh->HasTextureCoords(0)) {
				const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
				modelData.mTextureCoords.push_back(glm::vec2(vt->x, vt->y));
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
		aiVector3D l = scene->mLights[0]->mPosition;
		this->camera->lights.push_back(glm::vec3(l.x, l.y, l.z));
	}

	this->mesh = modelData;
}

void Actor::setupBufferObjects() {
	unsigned int vertex_positions_vbo_id = 0;
	GLuint loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
	GLuint loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
	GLuint loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");

    glGenBuffers(1, &vertex_positions_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_positions_vbo_id);
	glBufferData(GL_ARRAY_BUFFER, this->mesh.mPointCount * sizeof(glm::vec3), &this->mesh.mVertices[0], GL_STATIC_DRAW);

    GLuint vertex_normals_vbo_id = 0;
    glGenBuffers(1, &vertex_normals_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_normals_vbo_id);
	glBufferData(GL_ARRAY_BUFFER, this->mesh.mPointCount * sizeof(glm::vec3), &this->mesh.mNormals[0], GL_STATIC_DRAW);

    // unsigned int vertex_texture_vbo = 0;
	// glGenBuffers(1, &vt_vbo);
	// glBindBuffer(GL_ARRAY_BUFFER, vt_vbo);
	// glBufferData(GL_ARRAY_BUFFER, this->mesh.mTextureCoords * sizeof (glm::vec2), &this->mesh.mTextureCoords[0], GL_STATIC_DRAW);
    
	unsigned int vao = 0;
	glBindVertexArray(vao);

    glEnableVertexAttribArray(loc1);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_positions_vbo_id);
    glVertexAttribPointer (loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(loc2);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_normals_vbo_id);
    glVertexAttribPointer (loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	// Texturessssssssssssss
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// Set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Load and generate the texture
	int width, height, nrChannels;
	unsigned char *data = stbi_load("materials/textures/blue_floor_tiles_01_diff_4k.jpg", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		printf("Failed to load texture %s\n", "blue_floor_tiles_01_diff_4k.jpg");
	}
	stbi_image_free(data);

	glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc3);


    // glEnableVertexAttribArray (loc3);
	// glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	// glVertexAttribPointer (loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);
}

void Actor::renderMesh() {
    // Declare your uniform variables that will be used in your shader
    int matrix_location = glGetUniformLocation(this->shaderProgramID, "model");
    int view_mat_location = glGetUniformLocation(this->shaderProgramID, "view");
    int proj_mat_location = glGetUniformLocation(this->shaderProgramID, "proj");

	// Set the object color
	int object_color = glGetUniformLocation(this->shaderProgramID, "object_color");
	glUniform3f(object_color, 1.0f, 0.5f, 0.2f);

	// Set the light position
	int light_position = glGetUniformLocation(this->shaderProgramID, "light_position");
	glUniform3f(light_position, this->camera->lights[0].x, this->camera->lights[0].y, this->camera->lights[0].z);

	// Pass the camera location
	int camera_location = glGetUniformLocation(this->shaderProgramID, "camera_location");
	glUniform3f(camera_location, this->camera->location.x, this->camera->location.y, this->camera->location.z);
    
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
    glDrawArrays(GL_TRIANGLES, 0, this->mesh.mPointCount);
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
	
	if(this->parent) {
		glm::mat4 parentTransform = this->parent->getTransform();
		model = parentTransform * model;
	}

	return model;
}