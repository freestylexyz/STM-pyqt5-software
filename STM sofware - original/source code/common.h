#ifndef _COMMON_H
#define _COMMON_H
#include <windows.h>
#include "data.h"
#if defined(__NT__) || defined(WIN32)
//#ifdef __NT__
//#define MoveTo(_a,_b,_c) MoveToEx(_a,_b,_c,NULL)
#define MK_FP32
#define getscrollid() GetDlgCtrlID((HWND) lParam)
#define getscrollcode() ((int) LOWORD(wParam))
#define getscrollpos() (HIWORD(wParam))
#define getcombomsg()   (HIWORD(wParam))
#else
#define getscrollid() GetDlgCtrlID((HWND) HIWORD(lParam))
#define getscrollcode() (wParam)
#define getscrollpos() (LOWORD(lParam))
#define getcombomsg()   (HIWORD(lParam))
#endif
#define LISTBOXMAX  100
#define IMAGE_CALLER_SCAN       0
#define IMAGE_CALLER_DEP        1
#define IMAGE_CALLER_STM        2
#define SQRT2   1.414213562373
#define PI 3.14159
#define PRINT_DEF_SIZE          3
#define PRINT_DEF_X_POS         1
#define PRINT_DEF_Y_POS         1
#define PRINT_DEF_BAR_LENGTH    6
#define PRINT_DEF_BAR_HEIGHT    0.5

//#define PRINT_DATA_VERSION 3
// inherited from Moh

#define PRINT_DATA_VERSION 4
// added image_resolution_type field

#define GRID_ZERO_TOLERANCE  1e-3

#define PRINT_ARROW_HEAD_NONE   0
#define PRINT_ARROW_HEAD_START  1
#define PRINT_ARROW_HEAD_END    2
#define PRINT_ARROW_HEAD_BOTH   3
#define PRINT_ARROW_HEAD_DEF    2

#define SCAN_X_FIRST 100


#define RADIAN(T) ((float)(T)*3.14159/180.0)
#define DEGREE(T) ((float)(T)*180.0/3.14159)

#define wprint(A) SetDlgItemText(hDlg,A,string)
#define mprintf(A) MessageBox(GetFocus(),A,"Debug",MB_OKCANCEL)

#define stm2colorref(A) RGB((A).r,(A).g,(A).b)

typedef struct LISTBOXDEF
{
    int num;
    int index[LISTBOXMAX];
    char *str[LISTBOXMAX];
} LISTBOX;


void CheckMsgQ();
void init_listbox(HWND,int,LISTBOX *);
char *scan_freq_str(char *,unsigned int,unsigned int);
void repaint_grid(HWND,int,int,int,int);
void fit_plane_simple(datadef *,double *,double *,double *);
void copy_data(datadef **,datadef **);
void repaint_rect(HWND,int,int,int,int);
void clear_area(HWND,int,int,int,int,COLORREF);
float raw_z(datadef *,float,float);
float interp_z(datadef *,float,float);
float raw_linear_data( datadef *,float,float);
float linear_data(datadef *,float,float);
int raw_linear_data3(datadef *,float,float);
int linear_data3(datadef *,float,float);
int vert_int(float,float,float,float,float *);
int horiz_int(float,float,float,float *,float);
int new_count_el(COUNT_EL **,int,int);
int remove_count_el(COUNT_EL **,int,int);
PRINT_EL *new_print_el(PRINT_EL **,int);
LOGPAL_EL *new_logpal_el(LOGPAL_EL **,int);
LOGPAL_EL *sort_logpal_els(LOGPAL_EL **);
void destroy_logpal(LOGPAL_EL **);
void copy_logpal(LOGPAL_EL **,LOGPAL_EL *);
float float_bin_find(float *,int,int,float);
float point_line_dist(float ,float ,float ,float ,float ,float );
int bin_find(float *, int , int , float );
char *bgr(datadef *,float,int *);
void calc_r_theta(GRID,float *,float*);
void wait_cursor();
void arrow_cursor();
void find_min_max(datadef *, float *,float *);
int color_pal(GENPAL);
double calc_i_set(unsigned int, int,unsigned int);
void destroy_count(COUNT_DATA *);
void copy_count(COUNT_DATA **,COUNT_DATA **);
void free_count(COUNT_DATA **);
void copy_step(read_step *,read_step *);

int index2d(datadef *,int);

int rect_intersect_gen(float,float,float,float,
        float,float,float,float);
void paint_circles_gen(HDC ,int ,int ,float ,
    COUNT_EL *,int ,COLORREF );

void init_count_data(COUNT_DATA *);
#define stm2colorref(A) RGB((A).r,(A).g,(A).b)


#endif

/* common.h */
