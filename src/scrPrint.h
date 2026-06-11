#pragma once

#include "screen_util.h"

typedef enum {
    FNT6x9=1,
} FontIndex;

void scrGotoXY(int x, int y);
void scrInk(color_t ink);
void scrPaper(color_t ink);
void scrInkPaper(color_t ink, color_t paper);

void scrLF();
void scrPutC(char c);
void scrTab(int pos);
void scrPutS(const char *string);
void scrPutSLen(const char *string, size_t len);
void scrPrintf(const char *format, ...);