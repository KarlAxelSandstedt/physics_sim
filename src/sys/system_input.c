#include "mg_common.h"
#include "system_public.h"
#include "system_local.h"

struct mg_input_state *g_input_state;

#ifdef MG_DEBUG
#include "mg_string.h"

const char *mg_button_string_map[] =
{
	"MG_BUTTON_LEFT",
	"MG_BUTTON_RIGHT",
	"MG_BUTTON_SCROLL",
	"MG_BUTTON_NONMAPPED",
};

const char *mg_keycode_string_map[] = 
{
	"MG_SHIFT", "MG_SPACE", "MG_ESCAPE", "MG_F1", "MG_F2", "MG_F3", "MG_F4", "MG_F5", "MG_F6", "MG_F7",
	"MG_F8", "MG_F9", "MG_F10", "MG_F11", "MG_F12", NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "MG_0", "MG_1",
	"MG_2", "MG_3", "MG_4", "MG_5", "MG_6", "MG_7", "MG_8", "MG_9", NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, "MG_A", "MG_B", "MG_C", "MG_D", "MG_E", 
	"MG_F", "MG_G",  "MG_H",  "MG_I",  "MG_J",  "MG_K",  "MG_L",  "MG_M",  "MG_N",  "MG_O", 
	"MG_P", "MG_Q",  "MG_R",  "MG_S",  "MG_T",  "MG_U",  "MG_V",  "MG_W",  "MG_X",  "MG_Y", 
	"MG_Z", NULL, NULL, NULL, NULL, NULL, NULL, "MG_a", "MG_b",   "MG_c",  
	"MG_d",  "MG_e",  "MG_f",  "MG_g",  "MG_h",  "MG_i", "MG_j", "MG_k",  "MG_l",  "MG_m",  
	"MG_n",  "MG_o",  "MG_p",  "MG_q",  "MG_r",  "MG_s", "MG_t", "MG_u",  "MG_v",  "MG_w",
      	"MG_x",  "MG_y",  "MG_z", "MG_NO_SYMBOL",
};

const char *mg_keycode_to_string(const i32 key)
{
	return mg_keycode_string_map[key - (i32) MG_SHIFT];
}

const char *mg_button_to_string(const i32 button)
{
	return mg_button_string_map[button];
}
#endif

void input_state_init(struct arena *mem)
{
	assert(MG_NO_SYMBOL <= 255 && "MG_NO_SYMBOL should fit in a character");

	if (mem) {
		struct mg_input_state state = { 0 };
		const u64 num_bits = MG_NO_SYMBOL+1;
		u64 num_blocks = num_bits / BIT_VECTOR_BLOCK_SIZE;
		if (num_bits % BIT_VECTOR_BLOCK_SIZE)
		{
			num_blocks += 1;
		}
		u64 *bits = arena_push(mem, NULL, num_blocks * BIT_VECTOR_BLOCK_SIZE/8);

		g_input_state = (struct mg_input_state *) arena_push(mem, &state, sizeof(struct mg_input_state));
		g_input_state->key_pressed = bit_vec_init(num_bits, num_blocks, 0, bits);
	} else {
		g_input_state = calloc(1, sizeof(struct mg_input_state));
		g_input_state->key_pressed = bit_vec_new(MG_NO_SYMBOL+1, 0);
	}
}
