#version 420

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec3 a_normal;
layout(location = 3) in int index;

uniform float aspect_ratio = 720.0f/1280.0f;

uniform mat4 view;
uniform mat4 perspective;
uniform vec3 transform[256];
uniform int collision[256];

out vec3 FragPos;
out vec4 color;
out vec3 normal;

void main()
{
	int i = index % 256;
	vec4 collision_color = vec4(1.0f, 0.0f, 0.0f, a_color.w);
	color = collision[i] * collision_color + (1 - collision[i]) * a_color;
 	gl_Position = perspective * view * vec4(position + transform[i].xyz, 1.0f);
	normal = a_normal;
	FragPos = position + transform[i].xyz;
}
