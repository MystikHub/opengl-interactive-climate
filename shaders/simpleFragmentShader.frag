#version 330

in vec3 normal;
in vec3 fragment_position;
in vec2 texture_coordinates;

uniform vec3 object_color;
uniform sampler2D texture1;

// Ambient variables
vec3 ambient_light_color = vec3(0.5, 0.5, 0.5);
float ambient_strength = 0.1;

// Diffuse variables
vec3 diffuse_light_color = vec3(0.5, 0.5, 0.5);

// Specular variables
vec3 specular_light_color = vec3(1.0, 1.0, 1.0);
float specular_strength = 1.0;
float specular_coefficient = 100.0;

uniform vec3 light_position;
vec3 light_intensity = vec3(1.0, 1.0, 1.0);

uniform vec3 camera_location;
out vec4 FragColor;

void main(){
	// Ambient light
	vec3 ambient = ambient_strength * ambient_light_color;

	// Diffuse light
	vec3 norm = normalize(normal);
	vec3 light_direction = normalize(light_position - fragment_position);
	float diff = max(dot(norm, light_direction), 0.0);
	vec3 diffuse = diff * diffuse_light_color;

	// Specularity
	vec3 view_dir = normalize(camera_location - fragment_position);
	vec3 reflection_direction = reflect(-light_direction, norm);
	float specular_dot = max(dot(view_dir, reflection_direction), 0.0);
	float spec = pow(specular_dot, specular_coefficient);
	vec3 specular = spec * specular_strength * specular_light_color;

	vec4 new_object_color = texture(texture1, texture_coordinates);
	FragColor = vec4((specular + ambient + diffuse), 1.0) * new_object_color;

	// FragColor = vec4(final_lighting, 1.0);
}