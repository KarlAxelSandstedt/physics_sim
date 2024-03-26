#version 420

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec4 background_color;
layout(location = 3) in vec4 border_color;
layout(location = 4) in vec4 text_color;
layout(location = 5) in vec2 r_center;
layout(location = 6) in float c_radius;
layout(location = 7) in float e_softness;
layout(location = 8) in float border_thickness;


uniform float aspect_ratio = 720.0f/1280.0f;

out vec2 rect_center;
out vec2 rect_halfsize;
out vec2 uv_frag;
out vec4 bg_color;
out vec4 br_color;
out float br_thickness;
out vec4 tx_color;
out float corner_radius;
out float edge_softness;

void main()
{
	corner_radius = c_radius;
	edge_softness = e_softness;
	rect_center = vec2(720.0f/1280.0f, 1.0f) * r_center;
	vec2 hlfsz = abs(position - r_center);
	rect_halfsize = vec2(720.0f/1280.0f, 1.0f) * hlfsz;
	uv_frag = uv;
	bg_color = background_color;
	br_color = border_color;
	tx_color = text_color;
	br_thickness = border_thickness;

 	gl_Position = vec4(position.x * aspect_ratio, position.y, 0.0f, 1.0f);
}
