#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "vector.h"
#include "mg_font.h"

struct read_content debug_read_file_into_memory(struct arena *mem, const char *filepath)
{
	struct read_content rc;

	FILE *file = fopen(filepath, "r");
	
	if (mem)
	{
		fseek(file, 0, SEEK_END);
		rc.size = ftell(file);
		fseek(file, 0, SEEK_SET);
		arena_push(mem, NULL, sizeof(rc.size));
		fread(rc.data, 1, rc.size, file);
	}
	else
	{
		fseek(file, 0, SEEK_END);
		rc.size = ftell(file);
		fseek(file, 0, SEEK_SET);
		rc.data = malloc(rc.size);
		fread(rc.data, 1, rc.size, file);
	}

	fclose(file);

	return rc;
}

//struct mg_font mg_font_new(struct arena *mem, const char *filepath, const i32 atlas_w, const i32 atlas_h, const i32 glyph_w, const i32 glyph_h)
//{
//	struct read_content rc = debug_read_file_into_memory(mem, filepath);
//
//	struct mg_font font;
//	font.bmp.mem = rc.data;
//	font.bmp.width =  atlas_w;
//	font.bmp.height = atlas_h;
//	font.glyph_w = glyph_w;
//	font.glyph_h = glyph_h;
//
//	//arena_pop(stack, rc.size);
//}

//void mg_font_push_glyph(const struct mg_font *font, const char glyph, const vec4 color)
//{
//
//}

/***************************************************************************/

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#define SAFETY_PIXELS 4
struct mg_font read_ttf(const char *filepath)
{
	struct read_content rc = debug_read_file_into_memory(NULL, filepath);

	stbtt_fontinfo font;
	int ret = stbtt_GetFontOffsetForIndex(rc.data, 0);
	if (ret == -1)
	{
		printf("Error: could not return font offset\n");
	}
	stbtt_InitFont(&font, rc.data, ret);

	struct bitmap_info bmp;
	bmp.width =  1024;
	bmp.height = 1024;
	bmp.mem = malloc(4 * bmp.width * bmp.height);
	memset(bmp.mem, 0, 4 * bmp.width * bmp.height);

	const int32_t num_chars = 96;
	const int32_t first_char = 32;
	const int32_t glyph_pixel_height = 32;
	
	struct mg_font mg_font;
	mg_font.codepoint_table = malloc(num_chars * sizeof(struct codepoint_info));
	mg_font.num_chars = num_chars;
	mg_font.first_char = first_char;
	mg_font.glyph_height = glyph_pixel_height;
	mg_font.kerning_table = malloc(num_chars * num_chars * sizeof(int32_t));
	//TODO Get kerning table (NEED FONT WITH KERNING VALUES!)

	float scale = stbtt_ScaleForPixelHeight(&font, glyph_pixel_height);

	uint32_t *bmp_pix = (uint32_t *) bmp.mem;
	int32_t cur_row = SAFETY_PIXELS;
	int32_t cur_col = SAFETY_PIXELS;
	for (int i = 0; i < num_chars; ++i)
	{
		int32_t width, height, x_offset, y_offset, advance, lsb;
		/* Left-to-right, Top-to-bottom (reverse) */
		uint8_t *glyph_bitmap = stbtt_GetCodepointBitmap(&font, 0, scale, first_char + i, &width, &height, &x_offset, &y_offset);
		stbtt_GetCodepointHMetrics(&font, first_char + i, &advance, &lsb);

		int32_t x0,x1,y0,y1;
		stbtt_GetCodepointBitmapBox(&font, first_char + i, 0, scale, &x0, &y0, &x1, &y1);


		if (cur_col + width - SAFETY_PIXELS >= bmp.width)
		{
			cur_col = SAFETY_PIXELS;
			cur_row += glyph_pixel_height + SAFETY_PIXELS;
			if (cur_row + glyph_pixel_height - SAFETY_PIXELS >= bmp.height)
			{
				break;
			}
		}

		mg_font.codepoint_table[i].y_offset = -y1;
		mg_font.codepoint_table[i].advance = (scale * advance);
		mg_font.codepoint_table[i].lsb = scale * lsb;
		mg_font.codepoint_table[i].width = width;
		mg_font.codepoint_table[i].height = height;
		mg_font.codepoint_table[i].a_x = (float) cur_col / bmp.width; 
		mg_font.codepoint_table[i].a_y = (float) cur_row / bmp.height; 
		mg_font.codepoint_table[i].a_width = (float) width / bmp.width;
		mg_font.codepoint_table[i].a_height = (float) height / bmp.height;

		uint8_t *glyph_source = glyph_bitmap;
		for (int y = 0; y < height; ++y)
		{
			
			for (int x = 0; x < width; ++x)
			{

				uint8_t alpha = *glyph_source++;
				bmp_pix[bmp.width * (cur_row + (height - 1 - y)) + (cur_col + x)] = (
							(((uint32_t) alpha) << 24) | 
					      		(((uint32_t) alpha) << 16) |
					      		(((uint32_t) alpha) << 8 ) |
					      		(((uint32_t) alpha))
							);
				
			}
		}
		cur_col += width + SAFETY_PIXELS;
		stbtt_FreeBitmap(glyph_bitmap, NULL);
	}
	
	/* Get Baseline */
	int32_t ascent, descent, line_gap;
	stbtt_GetFontVMetrics(&font, &ascent, &descent, &line_gap);
	mg_font.baseline = (int32_t) (ascent * scale);
	printf("BASELINE: %i\n", mg_font.baseline);
	printf("SCALE: %f\n", scale);
	printf("HEIGHT: %i\n", mg_font.glyph_height);
	printf("VERTICAL: %i\n", (int32_t) (scale * (ascent - descent + line_gap)));

	printf("Kerning table size: %i\n", stbtt_GetKerningTableLength(&font));
	mg_font.atlas = bmp;
		
	return mg_font;
}

float mg_font_text_width(const struct mg_font *font, const u32 *str, const u32 len, const float aspect_ratio, const uint32_t screen_width)
{
	uint32_t pixel_width = 0;
	assert(len > 0);

	const u32 first_char = font->first_char;
	for (uint32_t i = 0; i < len; ++i) {
		const struct codepoint_info *cinfo = &font->codepoint_table[str[i] - first_char];
		pixel_width += cinfo->advance;
	}
	pixel_width -= font->codepoint_table[str[len-1] - first_char].advance;

	return ((float) pixel_width / screen_width) * aspect_ratio;
}
