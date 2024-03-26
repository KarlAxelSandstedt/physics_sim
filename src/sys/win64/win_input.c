#include "win_public.h"
#include "win_local.h"

enum mg_keycode win_mg_keycode_lookup(const u32 symbol)
{
	enum mg_keycode keycode = MG_NO_SYMBOL;
	if ((symbol >= 'A' && symbol <= 'Z'))
	{
		keycode = g_input_state->shift_pressed * symbol 
			+ (1 - g_input_state->shift_pressed) * (symbol - 'A' + 'a');
	} 
	else if (symbol >= '0' && symbol <= '9') 
	{	
		keycode = symbol; 
	} 
	else
	{
		switch (symbol)
		{
			case VK_SHIFT: keycode = MG_SHIFT; break;
			case VK_SPACE: keycode = MG_SPACE; break;
			case VK_ESCAPE: keycode = MG_ESCAPE; break;
			default: break;
		}
	}
	
	return keycode;
}

