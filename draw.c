#include <libk/stdarg.h>
#include <libk/stdio.h>
#include <libk/string.h>

#include <psp2/display.h>
#include <psp2/types.h>

#include "draw.h"
#include "font.h"


unsigned int* vram32;
int pwidth, pheight, bufferwidth, pixelformat;

uint32_t fcolor = 0x00ffffff;
uint32_t bcolor = 0xff000000;


/*
*	This function updates the frame buffer.
*/
void updateFrameBuf(const SceDisplayFrameBuf *param) {
	
	pwidth = param->width;
	pheight = param->height;
	vram32 = param->base;
	bufferwidth = param->pitch;
	pixelformat = param->pixelformat;

	if ((bufferwidth == 0) || (pixelformat != 0)) 
		return;

	fcolor = 0x00ffffff;
	bcolor = 0xff000000;
}

/*
*	This function sets the string color, as well as the background color.
*/
void setColor(int fg_col, int bg_col) {
	fcolor = fg_col;
	bcolor = bg_col;
}

/*
*	This function draws a string onto the screen.
*/
void drawString(int sx, int sy, const char *format) {
	
	if ((bufferwidth == 0) || (pixelformat != 0))
		return;

	for (int x = 0; format[x] && x < (pwidth / 16); x++) {
		
		int code = format[x] & 0x7f; // 7bit ANK
		
		for (int y = 0; y < 8; y++) {
			int offset = (sy + (y * 2)) * bufferwidth + sx + x * 16;
			unsigned char font = (y >= 7) ? 0x00 : msx[code * 8 + y];
			
			for (int p = 0; p < 8; p++) {
				uint32_t color = (font & 0x80) ? fcolor : bcolor;
				vram32[offset] = color;
				vram32[offset + 1] = color;
				vram32[offset + bufferwidth] = color;
				vram32[offset + bufferwidth + 1] = color;

				font <<= 1;
				offset+=2;
			}
		}
	}
}

/*
*	This function draws a string onto the screen with string specifier formats.
*/
void drawStringF(int sx, int sy, const char *format, ...) {
	
	char string[512];
	va_list list;

	va_start(list, format);
	vsnprintf(string, 512, format, list);
	va_end(list);

	drawString(sx, sy, string);
}