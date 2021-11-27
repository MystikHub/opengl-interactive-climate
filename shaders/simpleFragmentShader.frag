#version 330

in vec3 n_eye;

uniform vec3 object_color;
float ambient_strength = 0.75;
vec3 light_intensity = vec3(1.0, 1.0, 1.0);

void main(){
	vec3 ambient = ambient_strength * light_intensity;

	vec3 final_lighting = ambient * object_color;

	gl_FragColor = vec4(final_lighting, 1.0);
}