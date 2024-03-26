#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

uniform float aspect_ratio = 720.0f/1280.0f;

out vec2 tx_coord;

void main()
{
	tx_coord = uv;
 	gl_Position = vec4(position.x * aspect_ratio, position.yz, 1.0f);
}
