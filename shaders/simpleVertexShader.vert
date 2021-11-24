#version 330

in vec3 vertex_position;
in vec3 vertex_normal;

out vec3 n_eye;

uniform mat4 view;
uniform mat4 proj;
uniform mat4 model;

void main(){
	n_eye = (view * vec4 (vertex_normal, 0.0)).xyz;
	gl_Position =  proj * view * model * vec4 (vertex_position, 1.0);
}