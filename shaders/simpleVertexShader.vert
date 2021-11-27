#version 330

in vec3 vertex_position;
in vec3 vertex_normal;
in vec2 vertex_texture;

out vec3 fragment_position;
out vec3 normal;
out mat3 normal_matrix;

out vec2 texture_coordinates;

uniform mat4 view;
uniform mat4 proj;
uniform mat4 model;

void main(){
	fragment_position = vec3(model * vec4(vertex_position, 1.0));

	mat4 ModelViewMatrix = view * model;
	mat3 normal_matrix = mat3(ModelViewMatrix);

	normal = vertex_normal;
	texture_coordinates = vertex_texture;

	gl_Position = proj * view * vec4(fragment_position, 1.0);
}