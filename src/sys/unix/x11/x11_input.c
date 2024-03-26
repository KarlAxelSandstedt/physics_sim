#include "mg_common.h"
#include "x11_public.h"
#include "x11_local.h"

enum mg_keycode X11_mg_keycode_lookup(const KeySym symbol)
{
	enum mg_keycode keycode = MG_NO_SYMBOL;
	if ((symbol >= 'a' && symbol <= 'z'))
	{
		keycode = (1 - g_input_state->shift_pressed) * symbol 
			+ g_input_state->shift_pressed * ('A' - 'a' + symbol);
	} 
	else if (symbol >= '0' && symbol <= '9') 
	{	
		keycode = symbol; 
	} 
	else if (symbol >= XK_F1 && symbol <= XK_F12) 
	{
		keycode = MG_F1 + (symbol - XK_F1);
	} 
	else 
	{
		switch (symbol)
		{
			case XK_Shift_L: keycode = MG_SHIFT; break;
			case XK_space: keycode = MG_SPACE; break;
			case XK_Escape: keycode = MG_ESCAPE; break;
			default: break;
		}
	}
	
	return keycode;
}

enum mg_button X11_mg_button_lookup(const i32 state)
{
	enum mg_button button;
	switch (state)
	{
		case Button1Mask: button = MG_BUTTON_LEFT; break;
		case Button2Mask: button = MG_BUTTON_RIGHT; break;
		case Button3Mask: button = MG_BUTTON_SCROLL; break;
		default: button = MG_BUTTON_NONMAPPED; break;
	}

	return button;
}
