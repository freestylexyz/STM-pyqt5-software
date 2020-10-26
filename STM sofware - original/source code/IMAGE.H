#include "common.h"

#define IMAGE_CUT_XFACTOR    3		// multiply this by the selection length to
				 	// determine the number of data points in a 2d cut 
#define FIT_PLANE_MIN_REGION 25
#define IMAGE_2D_SKIP   10
#define R_BUTTON 1
#define L_BUTTON 0
#define IM_SCROLL_DELT  1
#define IM_SCROLL_DDELT (im_screenlines[current_image]/2)
#define PAL_X_POS2 2
#define PAL_Y_POS2 558
#define SMALL_IMAGE_SIZE 256
#define LARGE_IMAGE_SIZE 512
#define SMALL_BORDER_SIZE 5
#define LARGE_BORDER_SIZE 7
#define X_DIR 0
#define Y_DIR 1
#define GRAD_DIR 2
#define NEG_X_DIR 3
#define NEG_Y_DIR 4
#define DTRANS_SMALL     0
#define DTRANS_LARGE     1
#define DTRANS_BUF_MAX   500


#define DTRANS_MAX      100

void repaint_smooth(HWND);
void repaint_smooth_values(HWND,float *);
void repaint_smooth_radios(HWND);
void repaint_gamma(HWND);
void repaint_anim_delay(HWND);
void repaint_image(HWND,int,RECT *);
void linearize(int);
void take_log(int);
void calc_bitmap(int);
void calc_bitmap_gen(datadef *,unsigned char **,int *);
int floatcmp(float *,float *);
void save_gif(char *);
int time_index(int,float);
int image2pixel(float,float,int *,int *,int);
int color_index(datadef *,int, int);
void reset_image(HWND,int);

#define INDEX2Z(I,D) (float)(I)/(float)(LOGPAL_NUM_INDICES-1)*((D)->max_z-\
                (D)->min_z)+(D)->min_z


typedef int (* ifunptr)();
void GIFEncode ( FILE* , int , int , int , int , int , unsigned char *, 
        ifunptr );

void init_grids();

typedef struct DTRANSDEF
{
        float *dtrans[DTRANS_MAX];
        LISTBOX listbox;
} DTRANS;


