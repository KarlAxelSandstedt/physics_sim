#version 450

out vec4 output_color;

in vec3 view_pos;
in vec2 tx_coord;

uniform sampler2D tx;
uniform vec3 text_color;
uniform float transparency;

void main()
{
	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(tx, tx_coord).r);
	output_color = vec4(text_color, transparency) * sampled;
}
