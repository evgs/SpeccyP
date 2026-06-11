#include "screen_util.h"
#include <stdarg.h>
#include <stdio.h>

static color_t inkColor;
static color_t paperColor;

static int cursorStartLineX;
static int cursorX;
static int cursorY;
static int cursorL;

//TODO: менять размер символа при выборе шрифта
static int fontCharWidth = FONT_W;
static int fontCharHeight = FONT_H;

#define PRINTF_BUFSZ (64)

/**
 * @brief Переместить позицию печати указанные координаты экрана
 * @param x горизонтальная координата печати в пикселях
 * @param y вертикальная координата печати в пикселях
 */
void scrGotoXY(int x, int y) {
    cursorStartLineX = x;
    cursorX = x;
    cursorY = y;
    cursorL = 0;
}

/**
 * @brief Установить цвет текста
 * @param ink цвет текста (чернила)
 */
void scrInk(color_t ink) {
    inkColor = ink;
}


/**
 * @brief Установить цвет фона текста
 * @param paper цвет фона (бумага) 
 */
void scrPaper(color_t paper) {
    paperColor = paper;
}


/**
 * @brief Установить цвета вывода текста
 * @param ink цвет текста (чернила)
 * @param paper цвет фона (бумага)
 */
void scrInkPaper(color_t ink, color_t paper) {
    scrInk(ink);
    scrPaper(paper);
} 

/**
 * @brief Перевод строки (LineFeed)
 */
void scrLF() {
    cursorY += fontCharHeight;  // обработка выхода за пределы экрана - в draw_symbol()
    cursorX = cursorStartLineX;
    cursorL = 0;
}

void scrTab8() {
    int tpos = cursorL % 8;
    cursorL += 8 - tpos;
    cursorX = cursorStartLineX + cursorL * fontCharWidth; 
}

/**
 * @brief Вывести символ на экран и сместить позицию печати вправо
 * @param c - символ или управляющий код для вывода
 */
void scrPutC(char c) {
    switch (c)
    {
    case 0x0a:  //обрабатывается как возврат каретки и перевод строки (unix style)
        scrLF();
        break;

    case 0x0d:  //возврат каретки
        cursorX = cursorStartLineX;
        cursorL = 0;
        break;

    case 0x08:  //backspace
        cursorX -= fontCharWidth;
        cursorL--;
        if (cursorX < 0) { cursorX = 0; cursorL = 0; }
        break;

    case 0x09:  //tab
        scrTab8();
        break;

    default:
        draw_symbol(cursorX, cursorY, c, inkColor, paperColor);
        cursorX += fontCharWidth;
        cursorL++;
        break;
    }
}

void scrTab(int pos) {
    if (pos == 0) {
        scrTab8();
    }

    while (cursorL < pos) {
        scrPutC(' ');
    }
}


void scrPutS(const char *string) {
    char c;
    while ( c = *string++) {
        scrPutC(c);
    }
}


void scrPutSLen(const char *string, size_t len) {
    char c;
    while ( c = *string++) {
        if (len == 0) break;
        scrPutC(c);
        len--;
    }

    while (len--) {
        scrPutC(' ');
    }
}


void scrPrintf(const char *format, ...) {
    char buf[PRINTF_BUFSZ];

    va_list args;
    va_start (args, format);

    vsnprintf(buf, sizeof(buf), format, args);

    va_end(args);
    
    scrPutS(buf);
}
