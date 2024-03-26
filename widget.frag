#version 420

uniform sampler2D font_atlas;
in vec2 rect_center;
in vec2 rect_halfsize;
in vec2 uv_frag;
in vec4 bg_color;
in vec4 br_color;
in vec4 tx_color;
in float corner_radius;
in float edge_softness;
in float br_thickness;

out vec4 output_color;

float rect_sgf(in vec2 center, in vec2 halfsize, in vec2 position)
{
	const vec2 d = (abs(center - position) - halfsize) / halfsize; /* [ (-1.0, infty) x (-1.0, infty) ] */
	const float s_distance = -min(max(d.x, d.y), 0.0f); /* Longest axis distance from center, bound at 0.0f */
	return s_distance;
}

uniform vec2 res = vec2(1280.0f, 720.0f);

void main()
{
	const vec2 fragment_position = ((2.0f * gl_FragCoord.xy / res) - vec2(1.0f, 1.0f));
	const float s_distance = rect_sgf(rect_center, rect_halfsize, fragment_position);
	if (s_distance < 0.05f) {
		output_color = br_color;
	} else {
		output_color = bg_color;
	}

	vec4 sampled = vec4(1.0f, 1.0f, 1.0f, texture(font_atlas, uv_frag).a);
	output_color = output_color + (tx_color * sampled);
}
