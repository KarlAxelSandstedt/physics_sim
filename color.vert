#version 420

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

uniform float aspect_ratio = 720.0f/1280.0f;

uniform mat4 view;
uniform mat4 perspective;

out vec4 out_color;

void main()
{
	out_color = color;
 	gl_Position = perspective * view * vec4(position, 1.0f);
}
