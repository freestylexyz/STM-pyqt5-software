#ifndef _PAL_H
#define _PAL_H
#include <windows.h>
#define LOGPALSIZE 256
#define RAND_PAL_MAX 10
#define PAL_DATA_VERSION 1

#define SIZEOFMPAL (sizeof(unsigned char)*(LOGPALSIZE+1)*3)

void set_Plasma_palette(unsigned char *);
int spindac(int,int,unsigned char *);
void load_pal(char *);
void save_pal(char *);
void MakeColorLUT(int *,double);
void animate_delay(HWND,int);
void animate_pal(HWND,int);
void apply_gamma(unsigned char *);
void set_random_pal(unsigned char *);
void set_random_one_pal(unsigned char *);

#endif
