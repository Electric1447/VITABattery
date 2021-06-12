#ifndef __DRAW_H__
#define __DRAW_H__


#define BLACK   0x00000000
#define WHITE   0x00FFFFFF
#define RED     0x000000FF
#define GREEN   0x0000FF00
#define BLUE    0x00FF0000
#define CYAN    0x00FFFF00
#define MAGENTA 0x00FF00FF
#define YELLOW  0x0000FFFF
#define ORANGE  0x00008CFF
#define PURPLE  0x00FC006D


void updateFrameBuf(const SceDisplayFrameBuf *param);
void setColor(int fg_col, int bg_col);
void drawString(int sx, int sy, const char *format);
void drawStringF(int sx, int sy, const char *format, ...);


#endif