#ifndef __MG_FONT_H__
#define __MG_FONT_H__

#include "mg_common.h"
#include "mg_mempool.h"

struct read_content {
	uint8_t *data;
	size_t stride;
	size_t size;	
};

struct bitmap_info {
	i32 width;
	i32 height;
	u8 *mem;
};

struct read_content debug_read_file_into_memory(struct arena *mem, const char *filepath);

struct kerning_table {
	int x;
};

struct codepoint_info {
	int32_t advance;
	int32_t lsb;
	int32_t y_offset;
	uint32_t width;
	uint32_t height;
	float a_x;
	float a_y;
	float a_width;
	float a_height;
};

struct mg_font {
	uint32_t first_char;
	uint32_t num_chars;
	uint32_t glyph_height;
	int32_t baseline;
	struct codepoint_info *codepoint_table;
	int32_t *kerning_table;
	struct bitmap_info atlas;
};

struct mg_font 	read_ttf(const char *filepath);
float 		mg_font_text_width(const struct mg_font *font, const u32 *str, const u32 len, const float aspect_ratio, const uint32_t screen_width);

#endif
