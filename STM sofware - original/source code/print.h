#include "data.h"

#define PRINT_ARROW_HEADS_MAX   30

#define PRINT_ARROW_HEAD_ANGLE  40

#define PRINT_DEVICE_GAMMA_MIN  0.1
#define PRINT_DEVICE_GAMMA_MAX  4

#define PRINT_IMAGE_X_MIN       0
#define PRINT_IMAGE_X_MAX       8.5
#define PRINT_IMAGE_Y_MIN       0
#define PRINT_IMAGE_Y_MAX       11

#define PRINT_DEVICE_RES_MIN    100
#define PRINT_DEVICE_RES_MAX    2500

#define PRINT_IMAGE_SIZE_MIN    0.1
#define PRINT_IMAGE_SIZE_MAX    8.5

#define PRINT_SELECTION_SQUARE 5

#define PRINT_AREA_X   5
#define PRINT_AREA_Y   5
#define PRINT_AREA_WIDTH        425
#define PRINT_AREA_HEIGHT       550
#define PRINT_PIXELS_PER_INCH   50

#define PBM_WHITE 1
#define PBM_BLACK 0
#define CHARS_PER_LINE 60

#define POSTSCRIPT_INCH_SCALE fprintf(fp,"%f %f scale\n",(float)72/(float)device_res,(float)72/(float)device_res);

typedef unsigned char bit;

typedef struct tagfontbitmapinfo
{
        int font_x;
        int font_y;
        int font_descent;
        int x;
        int y;
        int del_x;
        int del_y;
} FONTBITMAPINFO;

void print_file(char *,HWND);
void calc_print_pal_bitmap( PRINT_EL *);
int pt_in_rect(int,int,int,int,POINT);

