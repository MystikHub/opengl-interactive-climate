#version 330

#define MAX_LIGHTS 128

in vec3 normal;
in vec3 fragment_position;
in vec2 texture_coordinates;

uniform sampler2D texture1;

// Ambient variables
vec3 light_color = vec3(0.5, 0.5, 0.5);
float ambient_strength = 0.5;

// Diffuse variables
float diffuse_strength = 0.5;

// Specular variables
vec3 specular_light_color = vec3(1.0, 1.0, 1.0);
float specular_strength = 1.0;
uniform float specularity;

// Lights
uniform int n_lights;
uniform vec3 light_positions[MAX_LIGHTS];
vec3 light_intensity = vec3(1.0, 1.0, 1.0);

uniform vec3 camera_location;
out vec4 FragColor;

void main(){
	// Need to calculate lighting for all light sources
	vec3 total_lighting = vec3(0.0, 0.0, 0.0);

	// Ambient light
	vec3 ambient = ambient_strength * light_color;

	for(int i = 0; i < n_lights; i++) {
		// Diffuse light
		vec3 norm = normalize(normal);
		vec3 light_direction = normalize(light_positions[i] - fragment_position);
		float diff = max(dot(norm, light_direction), 0.0);
		vec3 diffuse = diff * (diffuse_strength * light_color);

		// Specularity
		vec3 view_dir = normalize(camera_location - fragment_position);
		vec3 reflection_direction = reflect(-light_direction, norm);
		float specular_dot = max(dot(view_dir, reflection_direction), 0.0);
		float spec = pow(specular_dot, specularity);
		vec3 specular = spec * specular_strength * light_color;

		total_lighting += (specular + diffuse);
	}

	vec4 texture_color = texture(texture1, texture_coordinates);
	FragColor = vec4(total_lighting + ambient, 1.0) * texture_color;

	// FragColor = vec4(final_lighting + ambient , 1.0);
}