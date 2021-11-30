#version 330

#define MAX_LIGHTS 128

in vec3 normal;
in vec3 fragment_position;
in vec2 texture_coordinates;

uniform sampler2D texture1;

// Ambient variables
vec3 light_color = vec3(1.0, 1.0, 1.0);
float ambient_strength = 0.1;

// Diffuse variables
float diffuse_strength = 0.5;

// Specular variables
vec3 specular_light_color = vec3(1.0, 1.0, 1.0);
float specular_strength = 1;
uniform float specularity;

// Lights
uniform int n_lights;
uniform vec3 light_positions[MAX_LIGHTS];

uniform vec3 camera_location;
out vec4 FragColor;

// Attenuation
float attenuation_constant	= 1.0;
float attenuation_linear	= 0.09;
float attenuation_quadratic	= 0.032;

// Sun direction
vec3 sun_direction = vec3(-0.3, -1.0, 0.3);
vec3 sun_color = vec3(1.0, 1.0, 1.0);
float sun_diffuse_strength = 0.5;
float sun_specularity = 4.0;

// Enable/disable point lights
uniform int point_lights_on;

void main(){
	// Need to calculate lighting for all light sources
	vec3 total_lighting = vec3(0.0, 0.0, 0.0);

	// Ambient light
	vec3 ambient = ambient_strength * light_color;

	// Sun (directional) light
	// Same as point light, but with a fixed direction
	// Diffuse light
	vec3 sun_norm = normalize(normal);
	vec3 sun_light_direction = normalize(-sun_direction);
	float sun_diff = max(dot(sun_norm, sun_light_direction), 0.0);
	vec3 sun_diffuse = sun_diff * (sun_diffuse_strength * sun_color);

	// Specularity
	vec3 sun_view_dir = normalize(camera_location - fragment_position);
	vec3 sun_reflection_direction = reflect(-sun_light_direction, sun_norm);
	float sun_specular_dot = max(dot(sun_view_dir, sun_reflection_direction), 0.0);
	float sun_spec = pow(sun_specular_dot, sun_specularity);
	vec3 sun_specular = sun_spec  * sun_color;

	if(point_lights_on == 1) {
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
			
			float distance = length(light_positions[i] - fragment_position);
			float attenuation = 1.0 / (attenuation_constant + attenuation_linear * distance + 
						attenuation_quadratic * (distance * distance));

			total_lighting += (specular + diffuse) * attenuation;
		}
	}


	vec4 texture_color = texture(texture1, texture_coordinates);
	FragColor = vec4((total_lighting + sun_specular + sun_diffuse) + ambient, 1.0) * texture_color;
}