//#define DEBUG
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include "common.h"
#include "file.h"
#include "data.h"
#include "stm.h"
#include "dio.h"
#include "scan.h"
#include "image.h"
#include "pal.h"
#include "sel.h"
#include "fft.h"
#include "spec.h"
#include "count.h"
#include "infospc.h"
#include "infodep.h"

BOOL FAR PASCAL ImageDlg(HWND,unsigned,WPARAM,LPARAM);
extern BOOL FAR PASCAL PalDlg(HWND,unsigned,WPARAM,LPARAM);
extern BOOL FAR PASCAL LogpalDlg(HWND,unsigned,WPARAM,LPARAM);
extern BOOL FAR PASCAL SelDlg(HWND,unsigned,WPARAM,LPARAM);
extern BOOL FAR PASCAL AnlDlg(HWND,unsigned,WPARAM,LPARAM);
extern BOOL FAR PASCAL InfodepDlg(HWND,unsigned,WPARAM,LPARAM);
extern BOOL FAR PASCAL InfospcDlg(HWND,unsigned,WPARAM,LPARAM);
extern BOOL FAR PASCAL InfoDlg(HWND,unsigned,WPARAM,LPARAM);
extern BOOL FAR PASCAL IPrintDlg(HWND,unsigned,WPARAM,LPARAM);
extern BOOL FAR PASCAL VscaleDlg(HWND,unsigned,WPARAM,LPARAM);
extern BOOL FAR PASCAL FourierDlg(HWND,unsigned,WPARAM,LPARAM);
extern BOOL FAR PASCAL SubDlg(HWND,unsigned,WPARAM,LPARAM);
extern BOOL FAR PASCAL CountDlg(HWND,unsigned,WPARAM,LPARAM);
extern BOOL FAR PASCAL Comment2Dlg(HWND,unsigned,WPARAM,LPARAM);
extern BOOL FAR PASCAL GridDlg(HWND,unsigned,WPARAM,LPARAM);
extern BOOL FAR PASCAL TraceDlg(HWND,unsigned,WPARAM,LPARAM);

extern HWND PalWnd;
extern HWND LogpalWnd;
extern BOOL PalOn;
extern BOOL LogpalOn;
extern HWND SelWnd;
extern HWND AnlWnd;
extern HWND VscaleWnd;
extern HWND FourierWnd;
extern HWND SubWnd;
extern HWND CountWnd;
extern HWND TraceWnd;
extern HWND Comment2Wnd;
extern HWND GridWnd;
extern BOOL SelOn;
extern BOOL AnlOn;
extern BOOL VscaleOn;
extern BOOL FourierOn;
extern BOOL SubOn;
extern BOOL CountOn;
extern BOOL TraceOn;
extern BOOL Comment2On;
extern BOOL GridOn;
extern HBITMAP  hCross;
extern HBITMAP  hXcross;

extern COUNT_DATA count_data[];
extern int count_mode;
extern int count_sloppy_erase;
HWND ImgDlg;

extern int infospc_caller;
extern int infodep_caller;

int image_caller;
int image_auto_illuminate=0;
int image_auto_info=0;
extern char     string[];
int current_image=0;
float gamma=1.0;
int smooth_setting=1;
//int smooth_scale=SMOOTH_SMALL;
int smooth_custom_s=0;
int smooth_custom_l=0;
int zoomed_in=0;
//extern float *smooth_cus_s,*smooth_cus_l;
extern float *smooth_s,*smooth_l;
extern datadef *data,*dep_oscill_data[],**glob_data;
extern BITMAPINFO *bitmapinfo;
extern LOGPALETTE              *pLogPal;
extern HPALETTE                hPal;
extern HANDLE hInst;
datadef *gendata[4]={NULL,NULL,NULL,NULL};
char *image2=NULL;
float drift_a=0,drift_b=0,drift_c=0;
extern int ColorLUT[256];
float cur_filter[9];
float cur_smooth_s[9];
float cur_smooth_l[9];
int im_src_x[4],im_src_y[4],im_screenlines[4];
unsigned char *im_bitmaps[4]={NULL,NULL,NULL,NULL};
int im_bitmap_size[4];
unsigned char *image_dacbox=NULL;
extern unsigned char *dacbox;
extern unsigned char *scan_dacbox;
DTRANS image_dtrans_l, image_dtrans_s;
extern SEL_POINT sel_p1,sel_p2,sel_p3,sel_p4;
extern SEL_REGION sel_r1,sel_r2;
extern SEL_REGION *cur_sel_region;
extern int cur_sel;
extern GRID grids[];
char *temp_file;

int sel1_on, sel2_on;
int last_button=-1;
int captured=0;
int constrain=0;
char *current_file;
extern char *current_file_spc;
extern int file_spc_count;
extern char *current_file_stm,*current_file_gif,*current_file_cut,
    *current_file_exp;
extern int file_stm_count,file_cut_count;
float image_higherv[4],image_lowerv[4];
float image_highert[4],image_lowert[4];
int image_first_time=1;
int image_use_current=0;
extern unsigned int trace_from, trace_to;


static void repaint_cut_graphics(HDC ,HPEN ,datadef *,
    int ,int,
    int ,int ,int ,int , int , int);

static void countit(HWND,int,int,SEL_POINT);
static void repaint_grids(HWND,int);
static int draw_grid_line(HWND,float,float,int);

void repaint_drift_values(HWND);
void read_smooth_vals(HWND, int);
void image_sel_cut(SEL_REGION *, datadef *);
void image_sel_copy(SEL_REGION *, datadef **, datadef **);
void project3D(int);
void take_deriv(HWND, int, int);
void invert(int);
void drift_comp(int);
void normalize(HWND);
void smooth(HWND, int);
void im_do_scrolls(HWND, int);
void free_dtrans(DTRANS *);
void read_dtrans(DTRANS *, char *);
void copy_filter(float [], float *);
void im_invert(HWND, SEL_REGION *);
void paint_ellipse(HWND, SEL_REGION *);
void find_clip_rect(HWND, RECT *);
void inc_pt_dist(int, int *, int);
void notify_all();
void image_set_title(HWND);
void new_image(HWND);
static void repaint_trace_limits(HWND, int);
static void image_cut_profile(SEL_REGION *);
void paint_circles(HWND, int, COUNT_EL *, int, COLORREF);

static HPEN hpen_black,hpen_red,hpen_green,hpen_blue,hpen_old,hpen_dash,hpen_white;

BOOL FAR PASCAL ImageDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{
  int i,j,w;
  int dlg_result=FALSE;
  int id;
  int first_click;
  int delt,ddelt,mini,maxi;
  int this_image;
  POINT mouse_pos;
  float datasize;
  static FARPROC  PalDlgProc;
  static FARPROC  LogpalDlgProc;
  static FARPROC  SelDlgProc;
  static FARPROC  AnlDlgProc;
  static FARPROC  VscaleDlgProc;
  static FARPROC  FourierDlgProc;
  static FARPROC  SubDlgProc;
  static FARPROC  CountDlgProc;
  static FARPROC  TraceDlgProc;
  static FARPROC  Comment2DlgProc;
  static FARPROC  GridDlgProc;
  static FARPROC  lpfnDlgProc;
  PAINTSTRUCT ps;
//  HRGN small_rgn[4],large_rgn;
  RECT clip_rect;
  char *this_file;
  char *temp_char;
  double d;

  switch(Message) {
    case WM_INITDIALOG:
            ImgDlg=hDlg;
            PalDlgProc = MakeProcInstance(PalDlg, hInst);
            LogpalDlgProc = MakeProcInstance(LogpalDlg, hInst);
            SelDlgProc = MakeProcInstance(SelDlg, hInst);
            AnlDlgProc = MakeProcInstance(AnlDlg, hInst);
            VscaleDlgProc = MakeProcInstance(VscaleDlg, hInst);
            FourierDlgProc = MakeProcInstance(FourierDlg, hInst);
            SubDlgProc = MakeProcInstance(SubDlg, hInst);
            CountDlgProc = MakeProcInstance(CountDlg, hInst);
            TraceDlgProc = MakeProcInstance(TraceDlg, hInst);
            Comment2DlgProc = MakeProcInstance(Comment2Dlg, hInst);
            GridDlgProc = MakeProcInstance(GridDlg, hInst);
#ifdef WIN95TEST
mprintf("1");
#endif            
            sel1_on=sel2_on=0;
            hpen_black=CreatePen(PS_SOLID,1,RGB(0,0,0));
            hpen_red=CreatePen(PS_SOLID,1,RGB(255,0,0));
            hpen_green=CreatePen(PS_SOLID,1,RGB(0,255,0));
            hpen_blue=CreatePen(PS_SOLID,1,RGB(0,0,255));
            hpen_white=CreatePen(PS_SOLID,1,RGB(255,255,255));
            hpen_dash=CreatePen(PS_DASH,1,RGB(0,0,0));
#ifdef WIN95TEST
mprintf("2");
#endif            
            if (image_first_time)
            {
                for(i=0;i<4;i++)
                {
                    image_higherv[i]=IN_MAX;
                    image_lowerv[i]=IN_MIN;
                }
                init_grids();
                init_count();
                image_first_time=0; 
                temp_file=(char *)malloc(sizeof(char)*CURRENT_FILENAME_MAX);
                *temp_file='\0';
                
            }
#ifdef WIN95TEST
mprintf("3");
#endif            
            if (image_dacbox==NULL)
            {
                image_dacbox=(unsigned char *)malloc(SIZEOFMPAL);
                for(i=0;i<LOGPALSIZE+1;i++)
                    for(j=0;j<3;j++)
                        image_dacbox[i*(3)+j]=scan_dacbox[i*(3)+j];
            }
//            dacbox=image_dacbox;
            spindac(0,1,image_dacbox);
#ifdef WIN95TEST
mprintf("4");
#endif            
            
            zoomed_in=0;
            read_dtrans(&image_dtrans_l,DTRANS_L_INI);
            read_dtrans(&image_dtrans_s,DTRANS_S_INI);
            init_listbox(hDlg, FILTER_LIST, &(image_dtrans_s.listbox));
            copy_filter(cur_filter,
                image_dtrans_s.dtrans[image_dtrans_s.listbox.index[0]]);
            
            for(i=0;i<3;i++) for(j=0;j<3;j++) 
            {
              cur_smooth_s[i*3+j]=smooth_s[i*3+j];
              cur_smooth_l[i*3+j]=smooth_l[i*3+j];
            }
#ifdef WIN95TEST
mprintf("5");
#endif            
            CheckDlgButton(hDlg,SMOOTH_SMALL,1);
            CheckDlgButton(hDlg,SMOOTH_LARGE,0);
            CheckDlgButton(hDlg,IMAGE_AUTO_ILLUMINATE,image_auto_illuminate);
            CheckDlgButton(hDlg,IMAGE_AUTO_INFO,image_auto_info);
            CheckDlgButton(hDlg,IMAGE_USE_CURRENT,image_use_current);
//            large_rgn=CreateRectRgn(5,5,LARGE_IMAGE_SIZE+5,LARGE_IMAGE_SIZE+5);
            for(i=0;i<4;i++) 
                        {          
/*
                small_rgn[i]=CreateRectRgn(4+(i%2)*(SMALL_IMAGE_SIZE+5),
                    4+(i/2)*(SMALL_IMAGE_SIZE+5),
                    4+(i%2)*(SMALL_IMAGE_SIZE+5)+SMALL_IMAGE_SIZE+3,
                    4+(i/2)*(SMALL_IMAGE_SIZE+5)+SMALL_IMAGE_SIZE+3);
*/
                if (gendata[i]==NULL) alloc_data(&gendata[i],DATATYPE_3D,DATA_MAX,GEN2D_NONE,GEN2D_NONE,1);
                
            }
#ifdef WIN95TEST
mprintf("6");
#endif            
            
            switch(image_caller)
            {
                case IMAGE_CALLER_SCAN:
                    if (data->valid) 
                    {
                        copy_data(&gendata[0],&data);
                        im_src_x[0]=im_src_y[0]=0;
                        im_screenlines[0]=gendata[0]->size;
                        im_do_scrolls(hDlg,0);
                        calc_bitmap(0);
                        memcpy(gendata[0]->pal.dacbox,image_dacbox,SIZEOFMPAL);
                    }
                    break;
                case IMAGE_CALLER_DEP:
                    if ((*glob_data)!=NULL)
                    if ((*glob_data)->valid)
                    {
                        copy_data(&gendata[0],glob_data);
                        image_lowerv[0]=IN_MIN;
                        image_higherv[0]=IN_MAX;
                        image_lowert[0]=*(gendata[0]->time2d);
                        image_highert[0]=*(gendata[0]->time2d+gendata[0]->size-1);
                    }
                    break;
            }
#ifdef WIN95TEST
mprintf("7");
#endif            
//        if (!gendata[0]->valid) MessageBox(hDlg,"Invalid image","Warning",MB_OKCANCEL);
#ifdef OLD
        if (image2==NULL) image2=(char *) malloc(sizeof(char)*DATA_MAX*DATA_MAX);
#endif
        im_do_scrolls(hDlg,current_image);
        repaint_smooth(hDlg);
        repaint_gamma(hDlg);
        
//        repaint_anim_delay(hDlg);
        repaint_drift_values(hDlg);
//        repaint_pal(hDlg,PAL_X_POS2,PAL_Y_POS2);
        if (!zoomed_in) 
        {
//            for (i=0;i<4;i++) repaint_image(hDlg,i,NULL);
            repaint_rect(hDlg,1,current_image,SMALL_BORDER_SIZE,SMALL_IMAGE_SIZE);
        }
        else 
        {
            repaint_image(hDlg,current_image,NULL);
        }
        image_set_title(hDlg);
	dlg_result=TRUE;
#ifdef WIN95TEST
mprintf("8");
#endif            
      break;
    case WM_PALETTECHANGED:
        if ((HWND) wParam == hDlg)
            return 0;
            
    /* Otherwise, fall through to WM_QUERYNEWPALETTE. */

#ifdef OLD
    case WM_QUERYNEWPALETTE:

    /*
         * If realizing the palette causes the palette to change,
              * redraw completely.
                   */
                   
            i=spindac(0,1,image_dacbox);
#ifdef OLD1
        hdc = GetDC(hwnd);
        hpalT = SelectPalette (hdc, hpalApp, FALSE);
        

        i = RealizePalette(hdc); /* i == entries that changed  */
    
        SelectPalette (hdc, hpalT, FALSE);
            ReleaseDC(hwnd, hdc);
#endif        

    /* If any palette entries changed, repaint the window. */
    
#ifdef OLD1
        if (i > 0)
                InvalidateRect(hDlg, NULL, TRUE);
#endif
        return i;    
#endif            
	break;
    case WM_PAINT:
        BeginPaint(hDlg,&ps);
	dlg_result=TRUE;
        if (!zoomed_in) 
        {
            repaint_rect(hDlg,1,current_image,SMALL_BORDER_SIZE,SMALL_IMAGE_SIZE);
            for (i=0;i<4;i++) 
                if (rect_intersect(i,ps.rcPaint)) 
                {
                    repaint_image(hDlg,i,&(ps.rcPaint));
                }
        }
        else if (rect_intersect(0,ps.rcPaint)) 
        {
            repaint_image(hDlg,current_image,&(ps.rcPaint));
        }
        if (gendata[current_image]->valid)
        if (gendata[current_image]->type==DATATYPE_3D)
            repaint_pal(hDlg,PAL_X_POS2,PAL_Y_POS2,NULL,gendata[current_image]);
//        if (SelOn) SendMessage(SelWnd, Message, wParam, lParam);
        EndPaint(hDlg,&ps);
      break;
    case WM_LBUTTONUP:
        if (captured)
        {    
            ClipCursor(NULL);
            ReleaseCapture();
            captured=0;
	    dlg_result=TRUE;
        }
        break;
        
    case WM_RBUTTONUP:
        if (captured)
        {    
            ClipCursor(NULL);
            ReleaseCapture();
            captured=0;
	    dlg_result=TRUE;
        }
        break;
    case WM_MOUSEMOVE:
        if (captured)
        {
	    dlg_result=TRUE;
            repaint_sel(hDlg,cur_sel_region);
            if (find_coords(cur_sel_region->pt2,lParam)>=0)
            {
            switch(cur_sel_region->type)
            {
                case SEL_SQUARE:
                    if (!(wParam & MK_SHIFT))
                    {
                        square_coords(*(cur_sel_region->pt1),
                            cur_sel_region->pt2,cur_sel_region->image);
                    }
                    break;
                case SEL_CIRCLE:
                    i=sqrt((cur_sel_region->pt2->x-cur_sel_region->pt1->x)*
                        (cur_sel_region->pt2->x-cur_sel_region->pt1->x)+
                        (cur_sel_region->pt2->y-cur_sel_region->pt1->y)*
                        (cur_sel_region->pt2->y-cur_sel_region->pt1->y) );
                    cur_sel_region->pt2->x=abs(cur_sel_region->pt2->x-
                        cur_sel_region->pt1->x);
                    cur_sel_region->pt2->y=abs(cur_sel_region->pt2->y-
                        cur_sel_region->pt1->y);
                    if (!(wParam & MK_SHIFT))
                    {
                        cur_sel_region->pt2->x=cur_sel_region->pt2->y=i;
/*
                        circle_coords(*(cur_sel_region->pt1),
                            cur_sel_region->pt2,cur_sel_region->image);
*/
                    }
                    break;
            }
            SendMessage(SelWnd, WM_COMMAND, SEL_REPAINT_L2, 0);
            repaint_sel(hDlg,cur_sel_region);
            }
        }
        break;
    case WM_RBUTTONDOWN:
	dlg_result=TRUE;
        mouse_pos.x = LOWORD(lParam);
        mouse_pos.y = HIWORD(lParam);
        mouse_pos.x-=SMALL_BORDER_SIZE;
        mouse_pos.y-=SMALL_BORDER_SIZE;
        if (CountOn)
        {
            int temp_image;
            SEL_POINT temp_pt;
            
            temp_image=find_coords(&temp_pt,lParam);
            if (temp_image>=0)
            if (gendata[temp_image]->valid && 
                gendata[temp_image]->type==DATATYPE_3D &&
                !count_data[temp_image].hide && temp_image>=0)
            {
                if (!(wParam & MK_SHIFT))
                {
                    countit(hDlg,temp_image,1,temp_pt);
                }
                else
                {
                    countit(hDlg,temp_image,3,temp_pt);
                }
                    
            }
        }
        if (SelOn)
        {
            SEL_POINT temp_pt;
            int temp_image;
            cur_sel_region=&sel_r2;
            cur_sel=2;
            temp_image=find_coords(&temp_pt,lParam);
            if (temp_image>=0)
            {
            if (gendata[temp_image]->type==DATATYPE_2D ||
                gendata[temp_image]->type==DATATYPE_GEN2D)
                cur_sel_region->type=SEL_POINTS;
            SendMessage(SelWnd, WM_COMMAND, SEL_SEL_CHANGED, 0);
            if (sel2_on) repaint_sel(hDlg,cur_sel_region);
            cur_sel_region->flags &= ~(SEL_INVERTED);
            switch(cur_sel_region->type)
            {
                case SEL_POINTS:
                    cur_sel_region->image=
                        find_coords(cur_sel_region->pt1,lParam);
                    SendMessage(SelWnd, WM_COMMAND, SEL_REPAINT_P2, 0);
                    repaint_sel(hDlg,cur_sel_region);
                    sel2_on=1;
                    break;
                case SEL_LINE:
                    SetCapture(hDlg);
                    find_clip_rect(hDlg, &clip_rect);
                    ClipCursor(&clip_rect);
                    cur_sel_region->image=find_coords(cur_sel_region->pt1,lParam);
                    captured=1;
                    cur_sel_region->pt2->x=cur_sel_region->pt1->x;
                    cur_sel_region->pt2->y=cur_sel_region->pt1->y;
                    cur_sel_region->pt2->z=cur_sel_region->pt1->z;
                    SendMessage(SelWnd, WM_COMMAND, SEL_REPAINT_L1, 0);
                    SendMessage(SelWnd, WM_COMMAND, SEL_REPAINT_L2, 0);
                    paint_line(hDlg,cur_sel_region);
                    sel2_on=1;
                    break;
                case SEL_SQUARE:
 
                    SetCapture(hDlg);
                    find_clip_rect(hDlg, &clip_rect);
                    ClipCursor(&clip_rect);
                    cur_sel_region->image=find_coords(cur_sel_region->pt1,lParam);
                    captured=1;
                    cur_sel_region->pt2->x=cur_sel_region->pt1->x;
                    cur_sel_region->pt2->y=cur_sel_region->pt1->y;
                    cur_sel_region->pt2->z=cur_sel_region->pt1->z;
                    SendMessage(SelWnd, WM_COMMAND, SEL_REPAINT_L1, 0);
                    SendMessage(SelWnd, WM_COMMAND, SEL_REPAINT_L2, 0);
                    paint_rect(hDlg,cur_sel_region);
                    sel2_on=1;
                    break;
                case SEL_CIRCLE:
                    SetCapture(hDlg);
                    find_clip_rect(hDlg, &clip_rect);
                    ClipCursor(&clip_rect);
                    cur_sel_region->image=find_coords(cur_sel_region->pt1,lParam);
                    captured=1;
                    cur_sel_region->pt2->x=0;
                    cur_sel_region->pt2->y=0;
                    cur_sel_region->pt2->z=cur_sel_region->pt1->z;
                    SendMessage(SelWnd, WM_COMMAND, SEL_REPAINT_L1, 0);
                    SendMessage(SelWnd, WM_COMMAND, SEL_REPAINT_L2, 0);
                    paint_ellipse(hDlg,cur_sel_region);
                    sel2_on=1;
            
                    break;
            }
            }
            last_button=R_BUTTON;
        }
        break;
        
    case WM_LBUTTONDOWN:
	dlg_result=TRUE;
        mouse_pos.x = LOWORD(lParam);
        mouse_pos.y = HIWORD(lParam);
        mouse_pos.x-=SMALL_BORDER_SIZE;
        mouse_pos.y-=SMALL_BORDER_SIZE;
        first_click=1;
        this_image=current_image;
        if (!zoomed_in)
        {
            
            
            if (mouse_pos.x<SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE && 
                mouse_pos.y<(SMALL_IMAGE_SIZE*2+SMALL_BORDER_SIZE*3)) 
            {
                this_image=0;
                if (mouse_pos.y>SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE 
                    && mouse_pos.y<(SMALL_IMAGE_SIZE*2+SMALL_BORDER_SIZE*3)) 
                    this_image+=2;
            }
            else if (mouse_pos.x<(SMALL_IMAGE_SIZE*2+SMALL_BORDER_SIZE*3) &&
                mouse_pos.y<(SMALL_IMAGE_SIZE*2+SMALL_BORDER_SIZE*3)) 
            {
                this_image=1;
                if (mouse_pos.y>SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE 
                    && mouse_pos.y<(SMALL_IMAGE_SIZE*2+SMALL_BORDER_SIZE*3)) 
                    this_image+=2;
            }
/*
sprintf(string,"%d %d",current_image,this_image);
mprintf(string);
*/
            if (this_image!=current_image)
            {
                first_click=1;
                repaint_rect(hDlg,0,current_image,SMALL_BORDER_SIZE,SMALL_IMAGE_SIZE);
                repaint_rect(hDlg,1,this_image,SMALL_BORDER_SIZE,SMALL_IMAGE_SIZE);
                current_image=this_image;
                im_do_scrolls(hDlg,current_image);
                image_set_title(hDlg);
                if (gendata[current_image]->valid)
                if (gendata[current_image]->type==DATATYPE_3D)
                    repaint_pal(hDlg,PAL_X_POS2,PAL_Y_POS2,NULL,gendata[current_image]);
            }
            else first_click=0;
        } else first_click=0;
        if (CountOn && !first_click)
        {
            int temp_image;
            SEL_POINT temp_pt;
            
            temp_image=find_coords(&temp_pt,lParam);
            if (temp_image>=0)
            if (gendata[temp_image]->valid && 
                gendata[temp_image]->type==DATATYPE_3D &&
                !count_data[temp_image].hide && temp_image>=0)
            {
                if (!(wParam & MK_SHIFT))
                {
                    countit(hDlg,temp_image,0,temp_pt);
                }
                else
                {
                    countit(hDlg,temp_image,2,temp_pt);
                }
            }
        }
        if (SelOn && !first_click)
        {
            SEL_POINT temp_pt;
            int temp_image;
            
            cur_sel_region=&sel_r1;
            cur_sel=1;
            temp_image=find_coords(&temp_pt,lParam);
            if (temp_image>=0)
            {
            if (sel1_on) repaint_sel(hDlg,cur_sel_region);
            if (gendata[temp_image]->type==DATATYPE_2D
               || gendata[temp_image]->type==DATATYPE_GEN2D)
                cur_sel_region->type=SEL_POINTS;
            SendMessage(SelWnd, WM_COMMAND, SEL_SEL_CHANGED, 0);
            cur_sel_region->flags &= ~(SEL_INVERTED);
            switch(cur_sel_region->type)
            {
                case SEL_POINTS:
                    sel_r1.image=find_coords(sel_r1.pt1,lParam);
/*
    sprintf(string,"%d %d %f",sel_p1.x,sel_p1.y,sel_p1.z);
    MessageBox(hDlg,string,"Stop!",MB_ICONHAND);
*/
                    SendMessage(SelWnd, WM_COMMAND, SEL_REPAINT_P1, 0);
                    paint_cross(hDlg,&(sel_r1),hCross);
                    sel1_on=1;
                    break;
                case SEL_LINE:

                    SetCapture(hDlg);
                    find_clip_rect(hDlg, &clip_rect);
                    ClipCursor(&clip_rect);
                    cur_sel_region->image=find_coords(sel_r1.pt1,lParam);
                    captured=1;
                    cur_sel_region=&sel_r1;
                    sel_p2.x=sel_p1.x;
                    sel_p2.y=sel_p1.y;
                    sel_p2.z=sel_p1.z;
                    SendMessage(SelWnd, WM_COMMAND, SEL_REPAINT_L1, 0);
                    SendMessage(SelWnd, WM_COMMAND, SEL_REPAINT_L2, 0);
                    paint_line(hDlg,&sel_r1);
                    sel1_on=1;
                    break;
                case SEL_SQUARE:

                    SetCapture(hDlg);
                    find_clip_rect(hDlg, &clip_rect);
                    ClipCursor(&clip_rect);
                    sel_r1.image=find_coords(sel_r1.pt1,lParam);
                    captured=1;
                    cur_sel_region=&sel_r1;
                    sel_p2.x=sel_p1.x;
                    sel_p2.y=sel_p1.y;
                    sel_p2.z=sel_p1.z;
                    SendMessage(SelWnd, WM_COMMAND, SEL_REPAINT_L1, 0);
                    SendMessage(SelWnd, WM_COMMAND, SEL_REPAINT_L2, 0);
                    paint_rect(hDlg,cur_sel_region);
                    sel1_on=1;
                    break;
                case SEL_CIRCLE:
                    SetCapture(hDlg);
                    find_clip_rect(hDlg, &clip_rect);
                    ClipCursor(&clip_rect);
                    cur_sel_region->image=find_coords(cur_sel_region->pt1,lParam);
                    captured=1;
                    cur_sel_region->pt2->x=0;
                    cur_sel_region->pt2->y=0;
                    cur_sel_region->pt2->z=cur_sel_region->pt1->z;
                    SendMessage(SelWnd, WM_COMMAND, SEL_REPAINT_L1, 0);
                    SendMessage(SelWnd, WM_COMMAND, SEL_REPAINT_L2, 0);
                    paint_ellipse(hDlg,cur_sel_region);
                    sel1_on=1;
                    break;
            
            }
            }
            last_button=L_BUTTON;
        }
        notify_all();
      break;
    case WM_HSCROLL:
    case WM_VSCROLL:
	dlg_result=TRUE;
      id = getscrollid();
      switch(id) {      
/*
        case SCAN_Z_SCROLL:
          i = (int) scan_z;
          mini = scan_z_min;
          maxi = scan_z_max;
          delt = scan_z_delt;
          ddelt = scan_z_ddelt;
          break;
*/
      
        case IM_X_SCROLL:
          switch(gendata[current_image]->type)
          {
            case DATATYPE_3D:
              i = (int) im_src_x[current_image];
              mini = 0;
              maxi = gendata[current_image]->size-im_screenlines[current_image];
              delt = IM_SCROLL_DELT;
              ddelt = IM_SCROLL_DDELT;
              break;
            case DATATYPE_GEN2D:
            case DATATYPE_2D:
                break;
          }
          break;
              
        case IM_Y_SCROLL:
          switch(gendata[current_image]->type)
          {
            case DATATYPE_3D:
              i = (int) im_src_y[current_image];
              mini = 0;
              maxi = gendata[current_image]->size-im_screenlines[current_image];
              delt = -IM_SCROLL_DELT;
              ddelt = -IM_SCROLL_DDELT;
              break;
            case DATATYPE_GEN2D:
            case DATATYPE_2D:
                break;
          }
          break;
      }
      switch(getscrollcode())
      {
        case SB_LINELEFT:
          i -= delt;
          break;
        case SB_LINERIGHT:
          i += delt;
          break;
        case SB_PAGELEFT:
          i -= ddelt;
          break;
        case SB_PAGERIGHT:
          i += ddelt;
          break;
        case SB_THUMBPOSITION:
          i = getscrollpos();
          break;
        case SB_THUMBTRACK:
          i = getscrollpos();
          break;
        default:
          id = -1;
          break;
      }
      i = min(max(i,mini),maxi);
      switch(id)  {
        case IM_X_SCROLL:
          im_src_x[current_image] = (int) i;
          SetScrollPos(GetDlgItem(hDlg,IM_X_SCROLL),SB_CTL,im_src_x[current_image],TRUE);
          repaint_image(hDlg,current_image,NULL);
          break;
        case IM_Y_SCROLL:
          im_src_y[current_image] = (int) i;
          SetScrollPos(GetDlgItem(hDlg,IM_Y_SCROLL),SB_CTL,
            maxi-i,TRUE);
          repaint_image(hDlg,current_image,NULL);
          break;
/*
        case TIP_CURRENT_SCROLL:
          i_setpoint = (unsigned int) i;
          repaint_tip_current(hDlg);
          break;
*/
      }
      break;
 
   case WM_COMMAND:        /* received a command */
      switch(LOWORD(wParam)) {
        case IMAGE_REPAINT_TRACE_LIMITS:
            repaint_trace_limits(hDlg,current_image);
            break;
        case IM_NEW_VSCALE:
            repaint_image(hDlg,current_image,NULL);
            break;
    case IMAGE_INFO:
        switch(gendata[current_image]->type)
        {
            case DATATYPE_3D:
                lpfnDlgProc = MakeProcInstance(InfoDlg, hInst);
                DialogBox(hInst, "INFODLG", hDlg, lpfnDlgProc);
                FreeProcInstance(lpfnDlgProc);
                break;
            case DATATYPE_2D:
		infodep_caller = INFODEP_CALLER_IMAGE;
                lpfnDlgProc = MakeProcInstance(InfodepDlg, hInst);
                DialogBox(hInst, "INFODEPDLG", hDlg, lpfnDlgProc);
                FreeProcInstance(lpfnDlgProc);
                break;
            case DATATYPE_GEN2D:
                switch(gendata[current_image]->type2d)
                {
                    case TYPE2D_SPEC_I:
                    case TYPE2D_SPEC_Z:
                    case TYPE2D_SPEC_2:
                    case TYPE2D_SPEC_3:
                    case TYPE2D_MASSPEC:
			infospc_caller = INFOSPC_CALLER_IMAGE;
                        lpfnDlgProc = MakeProcInstance(InfospcDlg, hInst);
                        DialogBox(hInst, "INFOSPCDLG", hDlg, lpfnDlgProc);
                        FreeProcInstance(lpfnDlgProc);
                        break;
                    case TYPE2D_CUT:
                        break;
                }
                    
                break;
        }
        break;
    case IMAGE_PRINT:
        lpfnDlgProc = MakeProcInstance(IPrintDlg, hInst);
        DialogBox(hInst, "PRINTDLG", hDlg, lpfnDlgProc);
        FreeProcInstance(lpfnDlgProc);
        break;
    case FILTER_LIST:
/*
            sprintf(string,"%d",getcombomsg());
      MessageBox(hDlg,string,"Stop!",MB_ICONHAND);
*/
/*
                if (IsDlgButtonChecked(hDlg,SMOOTH_SMALL))
                {
                    copy_filter(cur_filter,image_dtrans_s.dtrans[
                        image_dtrans_s.listbox.index[SendDlgItemMessage(
                        hDlg,FILTER_LIST,CB_GETCURSEL,0,0)]]);
                }
                else
                {
                    copy_filter(cur_filter,image_dtrans_l.dtrans[
                        image_dtrans_l.listbox.index[SendDlgItemMessage(
                        hDlg,FILTER_LIST,CB_GETCURSEL,0,0)]]);
                }
                repaint_smooth(hDlg);
*/    
        switch (getcombomsg())
        {
            case CBN_SELCHANGE:
                if (IsDlgButtonChecked(hDlg,SMOOTH_SMALL))
                {
                    copy_filter(cur_filter,image_dtrans_s.dtrans[
                        image_dtrans_s.listbox.index[SendDlgItemMessage(
                        hDlg,FILTER_LIST,CB_GETCURSEL,0,0)]]);
                }
                else
                {
                    copy_filter(cur_filter,image_dtrans_l.dtrans[
                        image_dtrans_l.listbox.index[SendDlgItemMessage(
                        hDlg,FILTER_LIST,CB_GETCURSEL,0,0)]]);
                }
                repaint_smooth(hDlg);
                break;
        }
        break;            
    case IMAGE_ENHANCE:
        if (current_image!=3)
        {
            switch(gendata[current_image]->type)
            {
                case DATATYPE_3D:
                    if (gendata[current_image]->size<DATA_MAX)
                    {
                        copy_data(&gendata[3],&gendata[current_image]);
                        gendata[3]->size=DATA_MAX;
                        datasize=(float)gendata[current_image]->size/(float)DATA_MAX;
                        for(i=0;i<gendata[3]->size;i++) /* y */
                            for(j=0;j<gendata[3]->size;j++)  /* x */
                            {
                                *(gendata[3]->ptr+i*gendata[3]->size+j)=
                                    interp_z(gendata[current_image],datasize*(float)j,
                                        datasize*(float)i);
        /*
                                *(gendata[3]->ptr+i*gendata[3]->size+j)=
                                    linear_data(gendata[current_image],(float)i/(float)DATA_MAX,
                                        (float)j/(float)DATA_MAX);
        */
                            }
                        reset_image(hDlg,3);
                    }
                    else
                    {
                        MessageBox(hDlg,"Reached max size!","Done",MB_ICONHAND);
                    }
                    break;
                case DATATYPE_GEN2D:
                    switch(gendata[current_image]->type2d)
                    {
                        case TYPE2D_SPEC_I:
                        case TYPE2D_SPEC_Z:
                        case TYPE2D_SPEC_2:
                        case TYPE2D_SPEC_3:
                        case TYPE2D_MASSPEC:
                        
                        break;
                    }
                    break;
            }
        }
        break;
    case IM_SEL_REDRAW:
        repaint_sel(hDlg,&sel_r1);
        repaint_sel(hDlg,&sel_r2);
        break;
    case IM_REDRAW_IMAGE:
        repaint_image(hDlg,lParam,NULL);
        break;
    
    case IM_3D2TEXT:
	if( (gendata[current_image]->valid) && (gendata[current_image]->type == DATATYPE_3D)) {

	  // save current export file name and make a new one
	  strcpy( string, current_file_exp);
	  if( gendata[current_image]->saved) {
	    strcpy( current_file_exp, gendata[current_image]->filename);
	    w = 0;
	    while( current_file_exp[w] != '.') w++;
	    strcpy( &(current_file_exp[w]), ".exp");
	  }
	  else {
	    strcpy( current_file_exp, "3d2text.exp");
	  }
          glob_data=&gendata[current_image];
	  file_save_as( hDlg, hInst, FTYPE_3D2TEXT);

	  strcpy( current_file_exp, string);
	}
	break;

    case SEL_COPY:
        if (cur_sel_region->image!=3) image_sel_copy(cur_sel_region,
            &gendata[3],&gendata[cur_sel_region->image]);
        reset_image(hDlg,3);
        break;
    case SEL_CROP:
        switch(gendata[cur_sel_region->image]->type)
        {
            case DATATYPE_3D:
                switch(cur_sel_region->type)
                {
                    case SEL_SQUARE:
                    case SEL_CIRCLE:
                        image_sel_cut(cur_sel_region,
                            gendata[cur_sel_region->image]);
                        reset_image(hDlg,cur_sel_region->image);
                        break;
                    case SEL_LINE:
                        image_cut_profile(cur_sel_region);
                        reset_image(hDlg,3);
                        break;
                }
                break;
        }
        break;
        
    case IM_SEL_EXIT:
        if (sel1_on)
        {
            repaint_sel(hDlg,&sel_r1);
        }
        if (sel2_on) 
            repaint_sel(hDlg,&sel_r2);
        sel1_on=sel2_on=0;
        break;
    case IMAGE_AUTO_ILLUMINATE:
        image_auto_illuminate=IsDlgButtonChecked(hDlg,IMAGE_AUTO_ILLUMINATE);
        break;
    case IMAGE_AUTO_INFO:
        image_auto_info=IsDlgButtonChecked(hDlg,IMAGE_AUTO_INFO);
        break;
    case IMAGE_USE_CURRENT:
        image_use_current=IsDlgButtonChecked(hDlg,IMAGE_USE_CURRENT);
        break;
    case IMAGE_LOAD_NEXT:
      if (image_use_current && gendata[current_image]->valid) 
        strcpy(current_file,gendata[current_image]->full_filename);
      if (*current_file!='\0') 
      {
        strcpy(temp_file,current_file);
        moh_find_next(temp_file);
      }
      else *temp_file='\0';
      if (*temp_file!='\0')
      {
          strcpy(current_file,temp_file);
          glob_data=&gendata[current_image];
          smart_load(current_file,FTYPE_STM,1);
          new_image(hDlg);  
      }
      else
      {
          MessageBox(hDlg,"No more data!","Done",MB_ICONHAND);
      }
        
      break;      
    case IMAGE_LOAD_PREV:
      if (image_use_current && gendata[current_image]->valid) 
        strcpy(current_file,gendata[current_image]->full_filename);
      if (*current_file!='\0') 
      {
        strcpy(temp_file,current_file);
        moh_find_prev(temp_file);
      } else *temp_file='\0';
      if (*temp_file!='\0')
      {
          strcpy(current_file,temp_file);
          glob_data=&gendata[current_image];
          smart_load(current_file,FTYPE_STM,1);
          new_image(hDlg);
      }
      else
      {
          MessageBox(hDlg,"No more data!","Done",MB_ICONHAND);
      }
        
      
      break;      
        
    case SAVE_GIF:
      strcpy(current_file_gif,gendata[current_image]->filename);
      file_save_as(hDlg,hInst,FTYPE_GIF);
      break;
    case NORMALIZE:
          normalize(hDlg);
          repaint_smooth(hDlg);
        break;
    case SMOOTH:
        read_smooth_vals(hDlg,0);
        repaint_smooth(hDlg);
//        normalize(hDlg);
        if (current_image!=3) 
        {
            smooth(hDlg,current_image);
            reset_image(hDlg,3);
        }
        
        break;
    case SMOOTH_SMALL:
        init_listbox(hDlg, FILTER_LIST, &(image_dtrans_s.listbox));
        copy_filter(cur_filter,
            image_dtrans_s.dtrans[image_dtrans_s.listbox.index[0]]);
        repaint_smooth(hDlg);
        break;
        
    case SMOOTH_LARGE:
        init_listbox(hDlg, FILTER_LIST, &(image_dtrans_l.listbox));
        copy_filter(cur_filter,
            image_dtrans_s.dtrans[image_dtrans_l.listbox.index[0]]);
        repaint_smooth(hDlg);
        break;
#ifdef OLD
    case SMOOTH_1:
        if (IsDlgButtonChecked(hDlg,SMOOTH_SMALL))
        for(i=0;i<3;i++) for(j=0;j<3;j++) 
        {
          cur_smooth_s[i*3+j]=smooth_s[i*3+j];
        }
        else
        for(i=0;i<3;i++) for(j=0;j<3;j++) 
          cur_smooth_l[i*3+j]=smooth_l[i*3+j];
        repaint_smooth(hDlg);
        break;
    case SMOOTH_2:
        if (IsDlgButtonChecked(hDlg,SMOOTH_SMALL) )
        for(i=0;i<3;i++) for(j=0;j<3;j++) 
        {
          cur_smooth_s[i*3+j]=smooth_s[1*9+i*3+j];
        }
        else
        for(i=0;i<3;i++) for(j=0;j<3;j++) 
          cur_smooth_l[i*3+j]=smooth_l[1*9+i*3+j];
        repaint_smooth(hDlg);
        break;
    case SMOOTH_3:
        if (IsDlgButtonChecked(hDlg,SMOOTH_SMALL))
        for(i=0;i<3;i++) for(j=0;j<3;j++) 
        {
          cur_smooth_s[i*3+j]=smooth_s[2*9+i*3+j];
        }
        else
        for(i=0;i<3;i++) for(j=0;j<3;j++) 
          cur_smooth_l[i*3+j]=smooth_l[2*9+i*3+j];
        repaint_smooth(hDlg);
        break;
    case SMOOTH_4:
        if (IsDlgButtonChecked(hDlg,SMOOTH_SMALL))
        for(i=0;i<3;i++) for(j=0;j<3;j++) 
        {
          cur_smooth_s[i*3+j]=smooth_s[3*9+i*3+j];
        }
        else
        for(i=0;i<3;i++) for(j=0;j<3;j++) 
          cur_smooth_l[i*3+j]=smooth_l[3*9+i*3+j];
        repaint_smooth(hDlg);
        break;
#endif
    case ENTER:
//    MessageBox(hDlg,"Pressed enter IMAGE","Warning",MB_OKCANCEL);            
//      SetFocus(GetDlgItem(hDlg,IMAGE_EXIT));
          break;
        
    case DRIFT_A_VALUE:
      GetDlgItemText(hDlg,DRIFT_A_VALUE,string,9);
      drift_a = atof(string);
      break;
    case DRIFT_B_VALUE:
      GetDlgItemText(hDlg,DRIFT_B_VALUE,string,9);
      drift_b = atof(string);
      break;
    case DRIFT_C_VALUE:
      GetDlgItemText(hDlg,DRIFT_C_VALUE,string,9);
      drift_c = atof(string);
      break;
    case IMAGE_ILLUMINATE:
        if (current_image!=3 && gendata[current_image]->type==DATATYPE_3D &&
            gendata[current_image]->valid)
        {
            unequalize(gendata[current_image]);
            if (SelOn && constrain)
            {
              fit_plane(current_image,cur_sel_region);
            }
            else fit_plane(current_image,NULL);
            calc_bitmap(current_image);
            repaint_image(hDlg,current_image,NULL);
            if (gendata[current_image]->scan_dir>=SCAN_X_FIRST)
            {
              take_deriv(hDlg,current_image,X_DIR);
            }        
            else take_deriv(hDlg,current_image,Y_DIR);
      
            fit_plane(3,NULL);
            equalize(gendata[3],gendata[3]->min_z,gendata[3]->max_z,gendata[3]->fhist);
            gendata[3]->equalized=1;
            reset_image(hDlg,3);
        }
      break;
      
      
    case X_DERIV:
      take_deriv(hDlg,current_image,X_DIR);
      break;
    case Y_DERIV:
      take_deriv(hDlg,current_image,Y_DIR);
      break;
    case GRAD:
      take_deriv(hDlg,current_image,GRAD_DIR);
      break;
    case IMAGE_ROTATE:
        if (current_image!=3 && gendata[current_image]->valid &&
            gendata[current_image]->type==DATATYPE_3D)
        {
            int i,j;
            int size=gendata[current_image]->size;
            copy_data(&gendata[3],&gendata[current_image]);
            for(i=0;i<size;i++)
            {
                for(j=0;j<size;j++)
                    *(gendata[3]->ptr+i*size+j)=*(gendata[current_image]->ptr+
                        (size-j-1)*size+i);
            }
            reset_image(hDlg,3);
        }
        break;
    case IMAGE_FFT:
      if (!FourierOn)
      {
          FourierWnd = CreateDialog(hInst, "FOURIERDLG", hDlg, FourierDlgProc);
          FourierOn = TRUE;
      }
      else mprintf("it's on already!");
      break;
        

#ifdef OLD
        if (current_image!=3 && gendata[current_image]->valid &&
            gendata[current_image]->type==DATATYPE_3D)
        {
            copy_data(&gendata[3],&gendata[current_image]);
            gendata[3]->equalized=0;
            i=rev_size(gendata[3]->size);
//      sprintf(string,"%d",i);
//      MessageBox(hDlg,string,"log size",MB_ICONHAND);
            
//            if (i) fft_2d(gendata[3]->ptr,i,1);
            fht2d(gendata[3]->ptr,gendata[3]->ptr,1,gendata[3]->size);
            {
                float *filter;
                
                filter = (float *) malloc(sizeof(float)*gendata[3]->size*
                    gendata[3]->size);
                
                lowpass(filter, drift_a, drift_b, TRIANGLE, gendata[3]->size);
                fht2d(filter,filter,0,gendata[3]->size);
                mult_filter(gendata[3],filter);
                free(filter);
                fht2d(gendata[3]->ptr,gendata[3]->ptr,0,gendata[3]->size);
            }
            reset_image(hDlg,3);
        }
        break;
    case IMAGE_IFFT:
        if (current_image!=3 && gendata[current_image]->valid &&
            gendata[current_image]->type==DATATYPE_3D)
        {
            copy_data(&gendata[3],&gendata[current_image]);
            gendata[3]->equalized=0;
            i=rev_size(gendata[3]->size);
//      sprintf(string,"%d",i);
//      MessageBox(hDlg,string,"log size",MB_ICONHAND);
            
//            if (i) fft_2d(gendata[3]->ptr,i,-1);
            if (i) fht2d(gendata[3]->ptr,gendata[3]->ptr,0,gendata[3]->size);
            reset_image(hDlg,3);
        }
        break;
#endif
    case PAL_FCNS:
      if (!PalOn)
      {
          PalWnd = CreateDialog(hInst, "PALDLG", hDlg, PalDlgProc);
          PalOn = TRUE;
      }
      break;
    case IMAGE_LOGPAL:
      if (!LogpalOn)
      {
          LogpalWnd = CreateDialog(hInst, "LOGPALDLG", hDlg, LogpalDlgProc);
          LogpalOn = TRUE;
      }
      else
      {
        if (LogpalWnd!=NULL)
        {
            PostMessage(LogpalWnd, WM_COMMAND, LOGPAL_EXIT2, 0);
            LogpalWnd = CreateDialog(hInst, "LOGPALDLG", hDlg, LogpalDlgProc);
            LogpalOn = TRUE;
        }
      }
      break;
    case SEL_FCNS:
      if (!SelOn)
      {
          SelWnd = CreateDialog(hInst, "SELDLG", hDlg, SelDlgProc);
          SelOn = TRUE;
      }
      break;
    case ANL_FNCS:
      if (!AnlOn)
      {
          AnlWnd = CreateDialog(hInst, "ANLDLG", hDlg, AnlDlgProc);
          AnlOn = TRUE;
      }
      else
      {
//          MessageBox(hDlg,"Analysis already open!","Dope!",MB_ICONHAND);
      }
        
      break;
    case IMAGE_VSCALE:
      if (!VscaleOn)
      {
          VscaleWnd = CreateDialog(hInst, "VSCALEDLG", hDlg, VscaleDlgProc);
          VscaleOn = TRUE;
      }
      break;
    case IMAGE_SUBTRACT:
      if (!SubOn)
      {
          SubWnd = CreateDialog(hInst, "SUBTRACTDLG", hDlg, SubDlgProc);
          SubOn = TRUE;
      }
      break;
    case IM_COUNTING:
      if (!CountOn)
      {
          CountWnd = CreateDialog(hInst, "COUNTDLG", hDlg, CountDlgProc);
          CountOn = TRUE;
      }
      break;
    case IMAGE_TRACE:
      if (!TraceOn)
      {
          TraceWnd = CreateDialog(hInst, "TRACEDLG", hDlg, TraceDlgProc);
          TraceOn = TRUE;
      }
      break;
    case IMAGE_COMMENTS:
      if (!Comment2On)
      {
          Comment2Wnd = CreateDialog(hInst, "COMMENT2DLG", hDlg, Comment2DlgProc);
          Comment2On = TRUE;
      }
      break;
    case IMAGE_GRID:
      if (!GridOn)
      {
          GridWnd = CreateDialog(hInst, "GRIDDLG", hDlg, GridDlgProc);
          GridOn = TRUE;
      }
      break;

    case INVERT:
      if (gendata[current_image]->valid &&
            gendata[current_image]->type==DATATYPE_3D)
        {
          unequalize(gendata[current_image]);
          invert(current_image);
          calc_bitmap(current_image);
          repaint_image(hDlg,current_image,NULL);
          notify_all();
        }
      break;
    case PROJECT3D:
        if (current_image!=3 && gendata[current_image]->valid &&
            gendata[current_image]->type==DATATYPE_3D)
        {
          unequalize(gendata[current_image]);
          project3D(current_image);
          reset_image(hDlg,3);
          notify_all();
        }
      break;
    case IM_ZOOM_IN:
        if (gendata[current_image]->valid)
        {
            switch(gendata[current_image]->type)
            {
                case DATATYPE_3D:
                    if (im_screenlines[current_image]>3)
                    {
                        im_src_x[current_image]+=im_screenlines[current_image]/4;
                        im_src_y[current_image]+=im_screenlines[current_image]/4;
                        im_screenlines[current_image]/=2;
                    }
                    break;
                case DATATYPE_2D:
                    if (SelOn)
                    {
                        if (sel_r2.pt1->x>=0 && sel_r2.image==current_image
                            && sel_r1.pt1->x>=0 && sel_r1.image==current_image)
                        {
                            if (sel_r2.pt1->x>sel_r1.pt1->x)
                            {
                                image_lowert[current_image]=sel_r1.pt1->t;
                                image_highert[current_image]=sel_r2.pt1->t;
                            }
                            else if (sel_r2.pt1->x<sel_r1.pt1->x)
                            {
                                image_lowert[current_image]=sel_r2.pt1->t;
                                image_highert[current_image]=sel_r1.pt1->t;
                            }
                        }
                        else if (sel_r1.pt1->x>=0 && sel_r1.image==current_image)
                        {
                            image_lowert[current_image]=sel_r1.pt1->t;
                        }
                        else if (sel_r2.pt1->x>=0 && sel_r2.image==current_image)
                        {
                            image_highert[current_image]=sel_r2.pt1->t;
                        }
                    }
                    else
                    {
                        d=image_lowert[current_image];
                        image_lowert[current_image]+=(image_highert[current_image]-d)/4;
                        image_highert[current_image]-=(image_highert[current_image]-d)/4;
                    }
                    
                    break;
                case DATATYPE_GEN2D:
                    switch(gendata[current_image]->type2d)
                    {
                        case TYPE2D_SPEC_I:
                        case TYPE2D_SPEC_Z:
                        case TYPE2D_SPEC_2:
                        case TYPE2D_SPEC_3:
                        case TYPE2D_MASSPEC:
                            if (SelOn)
                            {
                                if (sel_r2.pt1->x>=0 && sel_r2.image==current_image
                                    && sel_r1.pt1->x>=0 && sel_r1.image==current_image)
                                {
                                    if (sel_r2.pt1->x>sel_r1.pt1->x)
                                    {
                                        image_lowert[current_image]=gendata[current_image]->start+
                                            sel_r1.pt1->x*gendata[current_image]->step;
                                        image_highert[current_image]=gendata[current_image]->start+
                                            sel_r2.pt1->x*gendata[current_image]->step;
                                    }
                                    else if (sel_r2.pt1->x<sel_r1.pt1->x)
                                    {
                                        image_lowert[current_image]=gendata[current_image]->start+
                                            sel_r2.pt1->x*gendata[current_image]->step;
                                        image_highert[current_image]=gendata[current_image]->start+
                                            sel_r1.pt1->x*gendata[current_image]->step;
                                    }
                                }
                                else if (sel_r1.pt1->x>=0 && sel_r1.image==current_image)
                                {
                                    image_lowert[current_image]=gendata[current_image]->start+
                                        sel_r1.pt1->x*gendata[current_image]->step;
                                }
                                else
                                {
                                    image_highert[current_image]=gendata[current_image]->start+
                                        sel_r2.pt1->x*gendata[current_image]->step;
                                }
                            }
                            else
                            {
                                d=image_lowert[current_image];
                                image_lowert[current_image]+=(image_highert[current_image]-d)/4;
                                image_highert[current_image]-=(image_highert[current_image]-d)/4;
                            }
                            break;
                        case TYPE2D_CUT:
                            if (SelOn)
                            {
                                if (sel_r2.pt1->x>=0 && sel_r2.image==current_image
                                    && sel_r1.pt1->x>=0 && sel_r1.image==current_image)
                                {
                                    if (sel_r2.pt1->x>sel_r1.pt1->x)
                                    {
                                        image_lowert[current_image]=
                                            sel_r1.pt1->x*gendata[current_image]->max_x/
                                            gendata[current_image]->size;
                                        image_highert[current_image]=
                                            sel_r2.pt1->x*gendata[current_image]->max_x/
                                            gendata[current_image]->size;
                                    }
                                    else if (sel_r2.pt1->x<sel_r1.pt1->x)
                                    {
                                        image_lowert[current_image]=
                                            sel_r2.pt1->x*gendata[current_image]->max_x/
                                            gendata[current_image]->size;
                                        image_highert[current_image]=
                                            sel_r1.pt1->x*gendata[current_image]->max_x/
                                            gendata[current_image]->size;
                                    } 
                                }
                                else if (sel_r1.pt1->x>=0 && sel_r1.image==current_image)
                                {
                                    image_lowert[current_image]=
                                        sel_r1.pt1->x*gendata[current_image]->max_x/
                                            gendata[current_image]->size;
                                }
                                else
                                {
                                    image_highert[current_image]=
                                        sel_r2.pt1->x*gendata[current_image]->max_x/
                                            gendata[current_image]->size;
                                }
                            }
                            else
                            {
                                d=image_lowert[current_image];
                                image_lowert[current_image]+=(image_highert[current_image]-d)/4;
                                image_highert[current_image]-=(image_highert[current_image]-d)/4;
                            }
                            break;
                    }
                    
                    break;
            }
            im_do_scrolls(hDlg,current_image);
            repaint_image(hDlg,current_image,NULL);
        }
        
        break;
    case IM_ZOOM_OUT:
        if (gendata[current_image]->valid)
        {
            switch(gendata[current_image]->type)
            {
                case DATATYPE_3D:
                    if (im_screenlines[current_image]<gendata[current_image]->size)
                    {
                        im_src_x[current_image]-=im_screenlines[current_image]/2;
                        im_src_y[current_image]-=im_screenlines[current_image]/2;
                        im_screenlines[current_image]*=2;
                        if (im_src_x[current_image]<0) im_src_x[current_image]= 0;
                        if (im_src_y[current_image]<0) im_src_y[current_image]=0;
                        if (im_screenlines[current_image]>gendata[current_image]->size)
                            im_screenlines[current_image]=gendata[current_image]->size;
                        if (im_src_x[current_image]+im_screenlines[current_image]>=
                            gendata[current_image]->size)
                        {
                            im_src_x[current_image]=
                                im_screenlines[current_image]-gendata[current_image]->size-1;
                        }
                        if (im_src_y[current_image]+im_screenlines[current_image]>=
                            gendata[current_image]->size)
                        {
                            im_src_y[current_image]=
                                im_screenlines[current_image]-gendata[current_image]->size-1;
                        }
                        if (im_src_x[current_image]<0) im_src_x[current_image]=0;
                        if (im_src_y[current_image]<0) im_src_y[current_image]=0;
                        if (im_src_x[current_image]>=gendata[current_image]->size) im_src_x[current_image]= 0;
                        if (im_src_y[current_image]>=gendata[current_image]->size) im_src_y[current_image]=0;
                    }
                    break;
                case DATATYPE_2D:
                    image_lowert[current_image]=*(gendata[current_image]->time2d);
                    image_highert[current_image]=*(gendata[current_image]->time2d+
                        gendata[current_image]->size-1);
                        
                    break;
                case DATATYPE_GEN2D:
                    switch(gendata[current_image]->type2d)
                    {
                        case TYPE2D_SPEC_I:
                        case TYPE2D_SPEC_Z:
                        case TYPE2D_SPEC_2:
                        case TYPE2D_SPEC_3:
                        case TYPE2D_MASSPEC:
                        case TYPE2D_CUT:
                        
                            image_lowert[current_image]=gendata[current_image]->min_x;
                            image_highert[current_image]=gendata[current_image]->max_x;
                            break;
                    }
                    break;
            }
            im_do_scrolls(hDlg,current_image);
            repaint_image(hDlg,current_image,NULL);
            
        }
        break;
                
            
    case IM_BIG_SMALL:
      zoomed_in=!zoomed_in;
      
      if (zoomed_in) 
      {
        clear_area(hDlg,4,4,12+LARGE_IMAGE_SIZE,12+LARGE_IMAGE_SIZE,RGB(255,255,255));
        repaint_image(hDlg,current_image,NULL);
      }
      else 
      {
        clear_area(hDlg,4,4,12+LARGE_IMAGE_SIZE,12+LARGE_IMAGE_SIZE,RGB(255,255,255));
        for (i=0;i<4;i++)  repaint_image(hDlg,i,NULL);
        repaint_rect(hDlg,1,current_image,SMALL_BORDER_SIZE,SMALL_IMAGE_SIZE);
      }
      break;
    case COPY_1:
      if (current_image!=0)
      {
        switch(gendata[current_image]->type)
        {
            case DATATYPE_2D:
                image_higherv[0]=IN_MAX;
                image_lowerv[0]=IN_MIN;
                image_highert[0]=*(gendata[current_image]->time2d+
                    gendata[current_image]->size-1);
                image_lowert[0]=*(gendata[current_image]->time2d);
                break;
            case DATATYPE_GEN2D:
                image_higherv[0]=IN_MAX;
                image_lowerv[0]=IN_MIN;
                image_highert[0]=gendata[current_image]->max_x;
                image_lowert[0]=gendata[current_image]->min_x;
                break;
            case DATATYPE_3D:
                im_src_x[0]=im_src_y[0]=0;
                im_screenlines[0]=gendata[current_image]->size;
                break;
                
        }    
        copy_data(&gendata[0],&gendata[current_image]);
        calc_bitmap(0);
        repaint_image(hDlg,0,NULL);
      }
      break;
    case COPY_2:
      if (current_image!=1)
      {
        switch(gendata[current_image]->type)
        {
            case DATATYPE_2D:
                image_higherv[1]=IN_MAX;
                image_lowerv[1]=IN_MIN;
                image_highert[1]=*(gendata[current_image]->time2d+
                    gendata[current_image]->size-1);
                image_lowert[1]=*(gendata[current_image]->time2d);
                break;
            case DATATYPE_GEN2D:
                image_higherv[1]=IN_MAX;
                image_lowerv[1]=IN_MIN;
                image_highert[1]=gendata[current_image]->max_x;
                image_lowert[1]=gendata[current_image]->min_x;
                break;
            case DATATYPE_3D:
                im_src_x[1]=im_src_y[1]=0;
                im_screenlines[1]=gendata[current_image]->size;
                break;
        }    
        copy_data(&gendata[1],&gendata[current_image]);
        calc_bitmap(1);
        repaint_image(hDlg,1,NULL);
      }
      break;
    case COPY_3:
      if (current_image!=2)
      {
        switch(gendata[current_image]->type)
        {
            case DATATYPE_2D:
                image_higherv[2]=IN_MAX;
                image_lowerv[2]=IN_MIN;
                image_highert[2]=*(gendata[current_image]->time2d+
                    gendata[current_image]->size-1);
                image_lowert[2]=*(gendata[current_image]->time2d);
                break;
            case DATATYPE_GEN2D:
                image_higherv[2]=IN_MAX;
                image_lowerv[2]=IN_MIN;
                image_highert[2]=gendata[current_image]->max_x;
                image_lowert[2]=gendata[current_image]->min_x;
                break;
            case DATATYPE_3D:
                im_src_x[2]=im_src_y[2]=0;
                im_screenlines[2]=gendata[current_image]->size;
                break;
        }    
        copy_data(&gendata[2],&gendata[current_image]);
        calc_bitmap(2);
        repaint_image(hDlg,2,NULL);
      }
      break;
    case COPY_4:
      if (current_image!=3)
      {
        switch(gendata[current_image]->type)
        {
            case DATATYPE_2D:
                image_higherv[3]=IN_MAX;
                image_lowerv[3]=IN_MIN;
                image_highert[3]=*(gendata[current_image]->time2d+
                    gendata[current_image]->size-1);
                image_lowert[3]=*(gendata[current_image]->time2d);
                break;
            case DATATYPE_GEN2D:
                image_higherv[3]=IN_MAX;
                image_lowerv[3]=IN_MIN;
                image_highert[3]=gendata[current_image]->max_x;
                image_lowert[3]=gendata[current_image]->min_x;
                break;
            case DATATYPE_3D:
                im_src_x[3]=im_src_y[3]=0;
                im_screenlines[3]=gendata[current_image]->size;
                break;
        }    
        copy_data(&gendata[3],&gendata[current_image]);
        calc_bitmap(3);
        repaint_image(hDlg,3,NULL);
      }
      break;
    case LINEARIZE:
      unequalize(gendata[current_image]);
      linearize(current_image);
      notify_all();
      break;
    case TAKE_LOG:
      unequalize(gendata[current_image]);
      take_log(current_image);
      im_src_x[3]=im_src_y[3]=0;
      im_screenlines[3]=gendata[3]->size;
      calc_bitmap(current_image);
      repaint_image(hDlg,current_image,NULL);
      notify_all();
      break;
    case FIT_PLANE:
      unequalize(gendata[current_image]);
      if (SelOn && constrain)
      {
        fit_plane(current_image,cur_sel_region);
      }
      else fit_plane(current_image,NULL);
      calc_bitmap(current_image);
      repaint_image(hDlg,current_image,NULL);
      notify_all();
      break;
    case DRIFT_COMP:
      unequalize(gendata[current_image]);
      drift_comp(current_image);
      im_src_x[3]=im_src_y[3]=0;
      im_screenlines[3]=gendata[3]->size;
      calc_bitmap(current_image);
      repaint_image(hDlg,3,NULL);
      break;
    case LOAD_IMAGE:
      glob_data=&gendata[current_image];
      i=FTYPE_STM;
      this_file=NULL;
      if (image_use_current && gendata[current_image]->valid)
        {
            i=guess_file_type(gendata[current_image]->full_filename);
            this_file=gendata[current_image]->full_filename;
        }
      else if (*current_file!='\0')
        {
            i=guess_file_type(current_file);
            this_file=current_file;
        }
        if (this_file) if (strrchr(this_file,'\\'))
        {
            this_file=strrchr(this_file,'\\')+1;
        }
      if (i==FTYPE_UNKNOWN) i=FTYPE_STM;
      file_open(hDlg,hInst,i,1,this_file);

      new_image(hDlg);      
      break;      
    case SAVE_IMAGE:
      if (gendata[current_image]->valid)
        switch(gendata[current_image]->type)
        {
            case DATATYPE_3D:
              glob_data=&gendata[current_image];
              if (file_save_as(hDlg,hInst,FTYPE_STM))
                  inc_file(current_file_stm,&file_stm_count);
              break;
            case DATATYPE_2D:
                glob_data=&gendata[current_image];
                if (file_save_as(hDlg,hInst,FTYPE_DEP))
                {
//                   inc_file(current_file_dep,&file_dep_count);
                }
			break;

            case DATATYPE_GEN2D:
                switch(gendata[current_image]->type2d)
                {
                    case TYPE2D_SPEC_I:
                    case TYPE2D_SPEC_Z:
                    case TYPE2D_SPEC_2:
                    case TYPE2D_SPEC_3:
                    case TYPE2D_MASSPEC:
                        glob_data=&gendata[current_image];
                        if (file_save_as(hDlg,hInst,FTYPE_SPC))
                        {
                            inc_file(current_file_spc,&file_spc_count);
                        }
                        break;
                    case TYPE2D_CUT:
                        glob_data=&gendata[current_image];
                        if (file_save_as(hDlg,hInst,FTYPE_CUT))
                            inc_file(current_file_cut,&file_cut_count);
                        
                        break;
                }
                break;
        }
      break;
      
	//pushkin
	case SAVE_AS_TOP:	//saves the topography in a nanotec format
		if(gendata[current_image]->type = DATATYPE_3D){
		}

	break;


    case IMAGE_DX:
        switch(gendata[current_image]->type)
        {
            case DATATYPE_3D:
                glob_data=&gendata[current_image];
                if (file_save_as(hDlg,hInst,FTYPE_STM_UNCOMPRESSED))
                    inc_file(current_file_stm,&file_stm_count);
                break;
            case DATATYPE_GEN2D:
	case DATATYPE_2D:
                strcpy(current_file_exp,
                    gendata[current_image]->filename);
                temp_char=strrchr(current_file_exp,'.');
                if (temp_char!=NULL)
                {
                    *(temp_char+1)='\0';
                    strcat(current_file_exp,"exp");
                }
                else strcat(current_file_exp,".exp");
                glob_data=&gendata[current_image];
                file_save_as(hDlg,hInst,FTYPE_GEN2D_EXPORT);
                break;
        }
        break;      
    case IMAGE_EXIT:
      
#ifdef OLD
      free(image2);
      image2=NULL;
#endif
/*
      for(i=0;i<4;i++) DeleteObject(small_rgn[i]);
      DeleteObject(large_rgn);
*/
      if (PalOn) SendMessage(PalWnd, WM_COMMAND, PAL_EXIT, 0);
      if (LogpalOn) SendMessage(LogpalWnd, WM_COMMAND, LOGPAL_EXIT, 0);
      if (SelOn) SendMessage(SelWnd, WM_COMMAND, SEL_EXIT, 0);
      if (AnlOn) SendMessage(AnlWnd, WM_COMMAND, ANL_EXIT, 0);
      if (VscaleOn) SendMessage(VscaleWnd, WM_COMMAND, VSCALE_EXIT, 0);
      if (FourierOn) SendMessage(FourierWnd, WM_COMMAND, FOURIER_EXIT, 0);
      if (SubOn) SendMessage(SubWnd, WM_COMMAND, SUB_EXIT, 0);
      if (CountOn) SendMessage(CountWnd, WM_COMMAND, COUNT_EXIT, 0);
      if (TraceOn) SendMessage(TraceWnd, WM_COMMAND, TRACE_EXIT, 0);
      if (GridOn) SendMessage(GridWnd, WM_COMMAND, GRID_EXIT, 0);
      if (Comment2On) SendMessage(Comment2Wnd, WM_COMMAND, COMMENT2_EXIT, 0);
      LogpalOn=FALSE;
      free_dtrans(&image_dtrans_l);
      free_dtrans(&image_dtrans_s);
      FreeProcInstance(PalDlgProc);
      FreeProcInstance(LogpalDlgProc);
      FreeProcInstance(SelDlgProc);
      FreeProcInstance(AnlDlgProc);
      FreeProcInstance(VscaleDlgProc);
      FreeProcInstance(FourierDlgProc);
      FreeProcInstance(SubDlgProc);
      FreeProcInstance(CountDlgProc);
      FreeProcInstance(TraceDlgProc);
      FreeProcInstance(Comment2DlgProc);
      FreeProcInstance(GridDlgProc);
        DeleteObject(hpen_black);
        DeleteObject(hpen_white);
        DeleteObject(hpen_red);
        DeleteObject(hpen_green);
        DeleteObject(hpen_blue);
        DeleteObject(hpen_dash);
      
          EndDialog(hDlg,TRUE);
          return(TRUE);
      }
      break;
  }
  return(dlg_result);
}

void repaint_drift_values(HWND hDlg)
{
    sprintf(string,"%0.2f", drift_a);
    SetDlgItemText(hDlg,DRIFT_A_VALUE,string); 
    sprintf(string,"%0.2f", drift_b);
    SetDlgItemText(hDlg,DRIFT_B_VALUE,string); 
    sprintf(string,"%0.2f", drift_c);
    SetDlgItemText(hDlg,DRIFT_C_VALUE,string); 
}

void read_smooth_vals(HWND hDlg,int reverse)
{
    float *array;
    
#ifdef OLD
    if (reverse)
    {
        if (IsDlgButtonChecked(hDlg,SMOOTH_SMALL)) array=cur_smooth_l;
        else array=cur_smooth_s;
    }
    else
    {
        if (IsDlgButtonChecked(hDlg,SMOOTH_SMALL)) array=cur_smooth_s;
        else array=cur_smooth_l;
    }
#endif
    array=cur_filter;    
    
    GetDlgItemText(hDlg,SMOOTH_1_1,string,9);
    array[0]=atof(string);
    GetDlgItemText(hDlg,SMOOTH0_1,string,9);
    array[1]=atof(string);
    GetDlgItemText(hDlg,SMOOTH1_1,string,9);
    array[2]=atof(string);
    GetDlgItemText(hDlg,SMOOTH_10,string,9);
    array[3]=atof(string);
    GetDlgItemText(hDlg,SMOOTH00,string,9);
    array[4]=atof(string);
    GetDlgItemText(hDlg,SMOOTH10,string,9);
    array[5]=atof(string);
    GetDlgItemText(hDlg,SMOOTH_11,string,9);
    array[6]=atof(string);
    GetDlgItemText(hDlg,SMOOTH01,string,9);
    array[7]=atof(string);
    GetDlgItemText(hDlg,SMOOTH11,string,9);
    array[8]=atof(string);
}    
    

void  repaint_smooth(HWND hDlg)
{

        repaint_smooth_values(hDlg,cur_filter);
    
}

void repaint_smooth_values(HWND hDlg, float *smooth_array)
{
    sprintf(string,"%0.3f",*(smooth_array+3*0+0));
    SetDlgItemText(hDlg,SMOOTH_1_1,string); 
    sprintf(string,"%0.3f",*(smooth_array+3*0+1));
    SetDlgItemText(hDlg,SMOOTH0_1,string); 
    sprintf(string,"%0.3f",*(smooth_array+3*0+2));
    SetDlgItemText(hDlg,SMOOTH1_1,string); 
    sprintf(string,"%0.3f",*(smooth_array+3*1+0));
    SetDlgItemText(hDlg,SMOOTH_10,string); 
    sprintf(string,"%0.3f",*(smooth_array+3*1+1));
    SetDlgItemText(hDlg,SMOOTH00,string); 
    sprintf(string,"%0.3f",*(smooth_array+3*1+2));
    SetDlgItemText(hDlg,SMOOTH10,string); 
    sprintf(string,"%0.3f",*(smooth_array+3*2+0));
    SetDlgItemText(hDlg,SMOOTH_11,string); 
    sprintf(string,"%0.3f",*(smooth_array+3*2+1));
    SetDlgItemText(hDlg,SMOOTH01,string); 
    sprintf(string,"%0.3f",*(smooth_array+3*2+2));
    SetDlgItemText(hDlg,SMOOTH11,string); 
}

void repaint_smooth_radios(HWND hDlg)
{
/*
          CheckDlgButton(hDlg,SCAN_FREQ_10,0);
*/
}
 
void repaint_gamma(HWND hDlg)
{
    sprintf(string,"%0.2f",gamma);
    SetDlgItemText(hDlg,GAMMA_VALUE,string); 
}
#ifdef OLD
void repaint_anim_delay(HWND hDlg)
{
    sprintf(string,"%d",animate_delay);
    SetDlgItemText(hDlg,ANIMATE_DELAY,string); 
}
#endif

void repaint_image(HWND hDlg,int number,RECT *area)
{
    HDC hDC;
    LOGBRUSH lplb;
    HBRUSH hBrush,hBrushOld;
    HPEN hPen,hPenOld;
    int xpos,ypos;
    int y_max,y_min;
    int x1,x2,y1,y2;
    int i,j;
    int screen_size;
    
    switch(gendata[number]->type)
    {
        case DATATYPE_3D:
            if (!zoomed_in)
            {
                xpos=SMALL_BORDER_SIZE+(number % 2)*(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE);
                ypos=SMALL_BORDER_SIZE+(number/2)*(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE);
                screen_size=SMALL_IMAGE_SIZE;
            }
            else
            {
                xpos=LARGE_BORDER_SIZE;
                ypos=LARGE_BORDER_SIZE;
                screen_size=LARGE_IMAGE_SIZE;
                if (number!=current_image) return;
            }
            hDC=GetDC(hDlg);
            SelectPalette(hDC,hPal,0);
            RealizePalette(hDC);
            SetMapMode(hDC,MM_TEXT);
            if (gendata[number]->valid)
            {
                repaint_pal(hDlg,PAL_X_POS2,PAL_Y_POS2,NULL,gendata[number]);
                spindac(0,1,gendata[number]->pal.dacbox);
                bitmapinfo->bmiHeader.biWidth=im_bitmap_size[number];
                bitmapinfo->bmiHeader.biHeight=im_bitmap_size[number];
                switch(gendata[number]->pal.type)
                {
                    case PALTYPE_MAPPED:
                        bitmapinfo->bmiHeader.biBitCount = 8;
                        StretchDIBits(hDC, xpos, ypos, screen_size, screen_size, 
                            im_src_x[number], im_src_y[number], im_screenlines[number],
                            im_screenlines[number],
                            im_bitmaps[number], bitmapinfo, DIB_PAL_COLORS,SRCCOPY);
                        break;
                    case PALTYPE_LOGICAL:
                        bitmapinfo->bmiHeader.biBitCount = 24;
                        StretchDIBits(hDC, xpos, ypos, screen_size, screen_size, 
                            im_src_x[number], im_src_y[number], im_screenlines[number],
                            im_screenlines[number],
                            im_bitmaps[number], bitmapinfo, DIB_RGB_COLORS,SRCCOPY);
                        break;
                }
                bitmapinfo->bmiHeader.biBitCount = 8;
                    
                repaint_grids(hDlg,number);
                if (!count_data[number].hide)
                {
                    for(i=0;i<COUNT_COLORS;i++)
                    {
                        paint_circles(hDlg,number,count_data[number].list[i],
                            count_data[number].size,
                            stm2colorref(count_data[number].color[i]));
                    }
                }
                if (SelOn) 
                {
                    if (sel_r1.image==number) repaint_sel(hDlg,&sel_r1);
                    if (sel_r2.image==number) repaint_sel(hDlg,&sel_r2);
                }
                    
            }
            else
            {
                lplb.lbStyle = BS_SOLID;
                lplb.lbColor = RGB(0,0,0);
                hBrush = CreateBrushIndirect(&lplb);
                hBrushOld = SelectObject(hDC, hBrush);
                hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
                hPenOld = SelectObject(hDC, hPen);
                Rectangle(hDC, xpos , ypos, xpos+screen_size, 
                    ypos+screen_size);
                SelectObject(hDC, hBrushOld);
                SelectObject(hDC, hPenOld);
                DeleteObject(hBrush);
                DeleteObject(hPen);
            }
                
                
            ReleaseDC(hDlg,hDC);
        break;
        case DATATYPE_2D:
            hDC=GetDC(hDlg);
            SelectPalette(hDC,hPal,0);
            RealizePalette(hDC);
            SetMapMode(hDC,MM_TEXT);
            if (!zoomed_in)
            {
                x1=SMALL_BORDER_SIZE+(number % 2)*(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE);
                y1=SMALL_BORDER_SIZE+(number/2)*(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE);
                x2=x1+SMALL_IMAGE_SIZE-1;
                y2=y1+SMALL_IMAGE_SIZE-1;
                screen_size=SMALL_IMAGE_SIZE;
            }
           else
           {
                x1=LARGE_BORDER_SIZE;
                y1=LARGE_BORDER_SIZE;
                x2=x1+LARGE_IMAGE_SIZE-1;
                y2=y1+LARGE_IMAGE_SIZE-1;
                screen_size=LARGE_IMAGE_SIZE;
            }
            if (gendata[number]->valid)
            {
                float start_time;
                float total_time;
                int first_pt=0;
                int skip=1;
                int start,end;
                int old_xs,old_ys;
            

                old_xs=-1;old_ys=-1;
                
                start_time=*(gendata[number]->time2d);
                total_time=image_highert[number]-image_lowert[number];
                start=-1;
                end=-1;
/* this can be done faster with a binary search! */
                for(j=0;j<gendata[number]->size && end<0;j++)
                {
                    if (start<0 && 
                        *(gendata[number]->time2d+j)>=image_lowert[number])
                    {
                        start=j;
                    }
                    if (end<0 && 
                        *(gendata[number]->time2d+j)>image_highert[number])
                    {
                        end=j;
                    }
                }
                if (start<0) start=0;
                if (end<0) end=gendata[number]->size;
                    
                hPenOld=SelectObject(hDC,hpen_white);
                Rectangle(hDC,x1,y1,x2,y2);
                repaint_grid(hDlg,x1,x2,y1,y2);
                SelectObject(hDC, hpen_green);
                
                first_pt=0;
                for(j=start;j<end;j++)
                {
                    int xs,ys;
                    if (first_pt)
                    {
                        xs=(*((gendata[number])->time2d+j)-image_lowert[number]
                                )*(x2-x1+1)/total_time+
                                x1;
                        ys=-((float)*(gendata[number]->data2d+j)
                            -image_higherv[number])*(y2-y1+1)/(image_higherv[number]-
                            image_lowerv[number]+1)+y1;
                        xs=max(min(xs,x2),x1);
                        ys=max(min(ys,y2),y1);
                        if (xs!=old_xs)
                        {
                            LineTo(hDC,xs,ys);
                            y_max=y_min=ys;
                        }
                        else
                        {
                            if (ys>y_max)
                            {
                                LineTo(hDC,xs,ys);
                                y_max=ys;
                            }
                            if (ys<y_min)
                            {
                                LineTo(hDC,xs,ys);
                                y_min=ys;
                            }
                        }
                        old_xs=xs;
                        old_ys=ys;
                    }
                    else
                    {
                        xs=(*((gendata[number])->time2d+j-1)-image_lowert[number]
                                )*(x2-x1+1)/total_time+
                                x1;
                        ys=-((float)*(gendata[number]->data2d+j-1)
                            -image_higherv[number])*(y2-y1+1)/(image_higherv[number]-
                            image_lowerv[number]+1)+y1;
                        xs=max(min(xs,x2),x1);
                        ys=max(min(ys,y2),y1);
//                        MoveTo(hDC,xs,ys);
                        MoveToEx(hDC,xs,ys,NULL);
                        first_pt=1;
                    }   
                }
                if (SelOn) 
                {
                    if (sel_r1.image==number) repaint_sel(hDlg,&sel_r1);
                    if (sel_r2.image==number) repaint_sel(hDlg,&sel_r2);
                }       
                if (TraceOn)
                {
                    repaint_trace_limits(hDlg,number);
                }
            }
            else    
            {
                lplb.lbStyle = BS_SOLID;
                lplb.lbColor = RGB(0,0,0);
                hBrush = CreateBrushIndirect(&lplb);
                hBrushOld = SelectObject(hDC, hBrush);
                hPenOld = SelectObject(hDC, hpen_black);
                Rectangle(hDC, xpos , ypos, xpos+LARGE_IMAGE_SIZE, ypos+LARGE_IMAGE_SIZE);
                SelectObject(hDC, hBrushOld);
                DeleteObject(hBrush);
            }

            SelectObject(hDC,hPenOld);            
            ReleaseDC(hDlg,hDC);
            break;
        case DATATYPE_GEN2D:
            hDC=GetDC(hDlg);
            SelectPalette(hDC,hPal,0);
            RealizePalette(hDC);
            SetMapMode(hDC,MM_TEXT);
            if (!zoomed_in)
            {
                x1=SMALL_BORDER_SIZE+(number % 2)*(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE);
                y1=SMALL_BORDER_SIZE+(number/2)*(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE);
                x2=x1+SMALL_IMAGE_SIZE-1;
                y2=y1+SMALL_IMAGE_SIZE-1;
                screen_size=SMALL_IMAGE_SIZE;
            }
           else
           {
                x1=LARGE_BORDER_SIZE;
                y1=LARGE_BORDER_SIZE;
                x2=x1+LARGE_IMAGE_SIZE-1;
                y2=y1+LARGE_IMAGE_SIZE-1;
                screen_size=LARGE_IMAGE_SIZE;
            }
            if (gendata[number]->valid)
            {
                hPenOld=SelectObject(hDC,hpen_white);
                Rectangle(hDC,x1,y1,x2,y2);
                repaint_grid(hDlg,x1,x2,y1,y2);
                SelectObject(hDC,hPenOld);            
                switch(gendata[number]->type2d)
                {
                    case TYPE2D_SPEC_I:
                    case TYPE2D_SPEC_Z:
                    case TYPE2D_SPEC_2:
                    case TYPE2D_SPEC_3:
                    case TYPE2D_MASSPEC:
                        repaint_spec_graphics(hDC,hpen_red,gendata[number] ,
                            image_lowert[number],image_highert[number],
                            x1,y1,x2,y2,image_higherv[number],image_lowerv[number]);
                        break;
                    case TYPE2D_CUT:
                        repaint_cut_graphics(hDC,hpen_red,gendata[number] ,
                            image_lowert[number],image_highert[number],
                            x1,y1,x2,y2,image_higherv[number],image_lowerv[number]);
                        break;
                }
                
                if (SelOn) 
                {
                    if (sel_r1.image==number) repaint_sel(hDlg,&sel_r1);
                    if (sel_r2.image==number) repaint_sel(hDlg,&sel_r2);
                }       
/*
                if (TraceOn)
                {
                    repaint_trace_limits(hDlg,number);
                }
*/
            }
            else    
            {
                lplb.lbStyle = BS_SOLID;
                lplb.lbColor = RGB(0,0,0);
                hBrush = CreateBrushIndirect(&lplb);
                hBrushOld = SelectObject(hDC, hBrush);
                hPenOld = SelectObject(hDC, hpen_black);
                Rectangle(hDC, xpos , ypos, xpos+LARGE_IMAGE_SIZE, ypos+LARGE_IMAGE_SIZE);
                SelectObject(hDC,hPenOld);            
                SelectObject(hDC, hBrushOld);
                DeleteObject(hBrush);
            }

            ReleaseDC(hDlg,hDC);
            break;
    }
}

void image_sel_cut(SEL_REGION *region, datadef *dest)
{
    int i,j;
    
    for(i=0;i<dest->size;i++)
        for(j=0;j<dest->size;j++)
        {
            if (pt_in_region(i,j,region))
            {
                *(dest->ptr+j*dest->size+i)=0;
            }
        }
}

void image_sel_copy(SEL_REGION *region, datadef **dest, datadef **source)
{
    
    int i,j,x1,x2,y1,y2,temp;
        
    copy_data(dest,source);
    
    x1=sel_p1.x;
    x2=sel_p2.x;
    y1=sel_p1.y;
    y2=sel_p2.y;
    if (x2<x1)
    {
        temp=x2;
        x2=x1;
        x1=temp;
    }
    if (y2<y1)
    {
        temp=y2;
        y2=y1;
        y1=temp;
    }
    (*dest)->size=y2-y1+1;
    for(i=0;i<x2-x1+1;i++)
        for(j=0;j<y2-y1+1;j++)
        {
            *((*dest)->ptr+j*(*dest)->size+i)=*((*source)->ptr+(j+y1)*(*source)->size+i+x1);
        }
}   
void linearize(int num)
{   
    int i,j;
    float data_size=gendata[num]->max_z-gendata[num]->min_z+1;
    if (data_size==0) data_size=1;
    
    if (gendata[num]->type==DATATYPE_3D)
    {
        for (i=0;i<gendata[num]->size;i++)
            for(j=0;j<gendata[num]->size;j++)
            {
                *(gendata[num]->ptr+i*gendata[num]->size+j)=
                 (*(gendata[num]->ptr+i*gendata[num]->size+j)-gendata[num]->min_z)/data_size*256;
            
            }            
        sprintf(string,"\r\nLinearized; Minimum was %f, Maximum was %f",
            gendata[num]->min_z,gendata[num]->max_z);
        strcat(gendata[num]->comment.ptr,string);
        gendata[num]->comment.size+=strlen(string);
        gendata[num]->min_z=0;
        gendata[num]->max_z=255;
        gendata[num]->clip_min=0;
        gendata[num]->clip_max=255;
    }
}    

int fit_plane_region(int num,SEL_REGION *region,double *a,double *b,double *c)
{
    int i,j;
    double gamma,sz,sxz,syz,z,sx,sy,sxy,sx2,sy2;
    int result=0;
 
    float n=0;
    sz=sxz=syz=sx2=sy2=sx=sy=sxy=0;
    if (gendata[num]->type==DATATYPE_3D)
    {
            for (i=0;i<gendata[num]->size;i++)
            for(j=0;j<gendata[num]->size;j++)
            if (pt_in_region(i,j,region))
            {
                z=*(gendata[num]->ptr+gendata[num]->size*i+j);
                sz+=z;
                sxz+= i*z;
                syz+= j*z;
                sx+=i;
                sy+=j;
                sx2+=i*i;
                sy2+=j*j;
                sxy+= i*j;
                n++;
            }
        if (n>=FIT_PLANE_MIN_REGION)
        {
        gamma=-n * sxy*sxy+2*sx*sxy*sy-sx*sx*sy2-sy*sy*sx2+n*sx2*sy2;
        *a=(-sxz*sy*sy-n*sxy*syz+sx*sy*syz+n*sxz*sy2+sxy*sy*sz-sx*sy2*sz)/gamma;
        *b=(-n*sxy*sxz+sx*sxz*sy-sx*sx*syz+n*sx2*syz+sx*sxy*sz-sx2*sy*sz)/gamma;
        *c=(sxy*sxz*sy+sx*sxy*syz-sx2*sy*syz-sx*sxz*sy2-sxy*sxy*sz+sx2*sy2*sz)/
            gamma;
        result=1;
        }
    }
    return(result);
            
}    
void fit_plane(int num,SEL_REGION *region)
{
    int i,j;
    double z,a,b,c;
    float min_z,max_z;
 
    if (gendata[num]->type==DATATYPE_3D)
    {
        if (region==NULL || num!=region->image) 
        {
            fit_plane_simple(gendata[num],&a,&b,&c);
        }
        else 
        {
            if (!fit_plane_region(num,region,&a,&b,&c))
                fit_plane_simple(gendata[num],&a,&b,&c);
        }
            
        for (i=0;i<gendata[num]->size;i++)
            for(j=0;j<gendata[num]->size;j++)
            {
                z=*(gendata[num]->ptr+gendata[num]->size*i+j)-a*i-b*j-c+IN_ZERO;
                *(gendata[num]->ptr+i*gendata[num]->size+j)=(float) z;
            }
        sprintf(string,"\r\nFitted Plane: a x+ b y + c ; a=%f b=%f c=%f",a,b,c);
        strcat(gendata[num]->comment.ptr,string);
        gendata[num]->comment.size+=strlen(string);
    
        find_min_max(gendata[num],&min_z,&max_z);
//    linearize(num);
    }

}
void take_log(int num)
{
    int i,j;
    float z;
    float data_size=gendata[num]->max_z-gendata[num]->min_z+1;
    if (data_size==0) data_size=1;
    
    switch(gendata[num]->type)
    {
        case DATATYPE_3D:
            for (i=0;i<gendata[num]->size;i++)
                for(j=0;j<gendata[num]->size;j++)
                {
                    z=(*(gendata[num]->ptr+i*gendata[num]->size+j)-gendata[num]->min_z)/
                        (data_size)*9+1;
                    *(gendata[num]->ptr+i*gendata[num]->size+j)=log10((double) z)*256;
                
                }            
            sprintf(string,"\r\nTook Log10; Minimum was %f, Maximum was %f",
                gendata[num]->min_z,gendata[num]->max_z);
            strcat(gendata[num]->comment.ptr,string);
            gendata[num]->comment.size+=strlen(string);
    
            gendata[num]->min_z=0;
            gendata[num]->max_z=255;
            gendata[num]->clip_min=0;
            gendata[num]->clip_max=255;
            break;
    }
}    

void project3D(int num)
{
    int i,j;
    float min_z,max_z,dx,dy;
    
    if (gendata[num]->type==DATATYPE_3D)
    {
        copy_data(&gendata[3],&gendata[num]);
        if (num!=3)
        {
            gendata[3]->size--;
            for(i=0;i<gendata[num]->size-1;i++)
            {
                for(j=0;j<gendata[num]->size-1;j++)
                {
                    dx=*(gendata[num]->ptr+j+i*gendata[num]->size) -
                        *(gendata[num]->ptr+j+1+i*gendata[num]->size);
                    dy=*(gendata[num]->ptr+j+i*gendata[num]->size) -
                        *(gendata[num]->ptr+j+(1+i)*gendata[num]->size);
                    *(gendata[3]->ptr+j+i*gendata[3]->size)=1.0/sqrt(1+dx*dx+dy*dy);
                        
                }
            }
        }
        find_min_max(gendata[3],&min_z,&max_z);
        linearize(3);
        unequalize(gendata[3]);
    }
}    

void take_deriv(HWND hDlg,int num,int dir)
{
    int i,j;
    float min_z,max_z,dx,dy;
    
    switch(gendata[num]->type)
    {
        case DATATYPE_3D:
            copy_data(&gendata[3],&gendata[num]);
            
            switch(dir)
            {
                case X_DIR:
                    for(i=0;i<gendata[3]->size;i++)
                    {
                        for(j=0;j<gendata[3]->size-1;j++)
                        {
                            *(gendata[3]->ptr+j+i*gendata[3]->size) -=
                                *(gendata[3]->ptr+j+1+i*gendata[3]->size);
                        }
                        *(gendata[3]->ptr+gendata[3]->size-1+i*gendata[3]->size) =
                         *(gendata[3]->ptr+gendata[3]->size-2+i*gendata[3]->size);
                    }
                    break;
                case Y_DIR:
                    for(i=0;i<gendata[3]->size;i++)
                    {
                        for(j=0;j<gendata[3]->size-1;j++)
                        {
                            *(gendata[3]->ptr+i+j*gendata[3]->size) -=
                                *(gendata[num]->ptr+i+(1+j)*gendata[num]->size);
                        }
                        *(gendata[3]->ptr+(gendata[3]->size-1)*gendata[3]->size+i) =
                            *(gendata[3]->ptr+(gendata[num]->size-2)*gendata[num]->size+i);
                    }
                    break;
                case NEG_Y_DIR:
                    for(i=0;i<gendata[3]->size;i++)
                    {
                        for(j=0;j<gendata[3]->size-1;j++)
                        {
                            *(gendata[3]->ptr+i+(j+1)*gendata[3]->size) -=
                                *(gendata[num]->ptr+i+j*gendata[num]->size);
                        }
                        *(gendata[3]->ptr+i) =
                            *(gendata[3]->ptr+gendata[num]->size+i);
                    }
                    break;
                case GRAD_DIR:
                    if (num!=3)
                    {
                        gendata[3]->size--;
                        for(i=0;i<gendata[num]->size-1;i++)
                        {
                            for(j=0;j<gendata[num]->size-1;j++)
                            {
                                dx=*(gendata[num]->ptr+j+i*gendata[num]->size) -
                                    *(gendata[num]->ptr+j+1+i*gendata[num]->size);
                                dy=*(gendata[num]->ptr+j+i*gendata[num]->size) -
                                    *(gendata[num]->ptr+j+(1+i)*gendata[num]->size);
                                *(gendata[3]->ptr+j+i*gendata[3]->size)=sqrt(dx*dx+dy*dy);
                                    
                            }
                        }
                    }
                    break;
              }
            im_src_x[3]=im_src_y[3]=0;
            im_screenlines[3]=gendata[3]->size;
            find_min_max(gendata[3],&min_z,&max_z);
            linearize(3);
            unequalize(gendata[3]);
            reset_image(hDlg,3);
            break;
    }
        
    
}

void invert(int num)
{
    int i,j;
    float temp=gendata[num]->min_z;
    
    switch(gendata[num]->type)
    {
        case DATATYPE_3D:
            for(i=0;i<gendata[num]->size;i++)
                for(j=0;j<gendata[num]->size;j++)
                    *(gendata[num]->ptr+i+j*gendata[num]->size) =
                        -*(gendata[num]->ptr+i+j*gendata[num]->size);
            gendata[num]->min_z=gendata[num]->clip_min=-gendata[num]->max_z;
            gendata[num]->max_z=gendata[num]->clip_max=-temp;
            break;
    }
}            

void drift_comp(int number)
{
    int new_max_x,new_max_y,old_x,old_y,i,j;
    float zoom;
    float S,Z,drift_a_rad,drift_b_rad,drift_c_rad,A,B,C;
    
    if (gendata[number]->type==DATATYPE_3D)
    {
        copy_data(&gendata[3],&gendata[number]);
        drift_a_rad=drift_a/360*2*PI;
        drift_b_rad=drift_b/360*2*PI;
        drift_c_rad=drift_c/360*2*PI;
        A=tan(drift_a_rad);
        B=tan(drift_b_rad);
        C=tan(drift_c_rad);
        zoom=1;
        Z=2*(A-B)*(B-C)*(C-A)/(sqrt(3)*B*B*(C-A)*(C-A)+(A*B+B*C-2*A*C)*
            (A*B+B*C-2*A*C)/sqrt(3));
        S=(Z*(A*B+B*C-2*A*C)-sqrt(3)*(C-A))/(sqrt(3)*Z*B*(C-A));
        new_max_x=(int) ((float) gendata[number]->size*(1+S));
        new_max_y=(int) ((float)gendata[number]->size*Z);
        if (new_max_x<gendata[number]->size)
        {
            zoom=zoom*(float) gendata[number]->size/(float) new_max_x;
            new_max_x=gendata[number]->size;
            new_max_y=new_max_y*zoom;
        }
        if (new_max_y<gendata[number]->size)
        {
            zoom=zoom*(float) gendata[number]->size/(float) new_max_y;
            new_max_y=gendata[number]->size;
            new_max_x=new_max_x*zoom;
        }
        if (new_max_x> new_max_y) new_max_y=new_max_x;
        else new_max_x=new_max_y;
        if (new_max_x>DATA_MAX) 
        {
            zoom=zoom*DATA_MAX/new_max_x;
            new_max_x=new_max_y=DATA_MAX;
        }
        gendata[3]->size=new_max_x;
        for(i=0;i<new_max_x;i++)
            for(j=0;j<new_max_y;j++)
            {
                old_x=(i-S*j/Z)/zoom;
                old_y=j/Z/zoom;
                if (old_x<0 || old_y<0 || old_x>=gendata[number]->size
                      || old_y>=gendata[number]->size)
                      *(gendata[3]->ptr+i*new_max_x+j)=IN_ZERO;
                else *(gendata[3]->ptr+i*new_max_x+j)=
                        *(gendata[number]->ptr+old_x*gendata[number]->size+old_y);
            }
        sprintf(string,"\r\nDrift compensated with: a=%f b=%f c=%f zoom=%f",
            drift_a,drift_b,drift_c,zoom);
        strcat(gendata[3]->comment.ptr,string);
        gendata[3]->comment.size+=strlen(string);
        unequalize(gendata[3]);
    }
}

int rect_intersect(int image, RECT area)
{
    int x1,x2,y1,y2;
    int result=0;
    
    if (zoomed_in)
    {
        x1=y1=SMALL_BORDER_SIZE;
        x2=y2=x1+LARGE_IMAGE_SIZE;
    }
    else
    {
        x1=SMALL_BORDER_SIZE+(image % 2)*(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE);
        y1=SMALL_BORDER_SIZE+(image/2)*(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE);
        x2=x1+SMALL_IMAGE_SIZE;
        y2=y1+SMALL_IMAGE_SIZE;
    }
    return(rect_intersect_gen(x1,y1,x2,y2,area.left,area.bottom,area.right,
        area.top));
#ifdef OLD
    
    result=((x1>=area.left && x1<=area.right) && (y1>=area.top && y1<=area.bottom));
    if (!result)
        result=((x1>=area.left && x1<=area.right) && (y2>=area.top && y2<=area.bottom));
    if (!result)
        result=((area.left>=x1 && area.left<=x2) && (area.top>=y1 && area.top<=y2));
    if (!result)
        result=((area.left>=x1 && area.left<=x2) && (area.bottom>=y1 && area.bottom<=y2));
    if (!result)
        result=((area.right>=x1 && area.right<=x2) && (area.top>=y1 && area.top<=y2));
    if (!result)
        result=(area.left<x1 && area.right>x2 && area.top>y1 && area.bottom<y2);
    if (!result)
        result=(area.left>x1 && area.right<x2 && area.top<y1 && area.bottom>y2);
    if (!result)
        result=(area.left>x1 && area.left<x2 && area.top<y1 && area.bottom>y2);
#endif
    return(result);
}        

void normalize(HWND hDlg)
{
    float smooth_sum;
    float *temp_array;
    int i,j;
            
        read_smooth_vals(hDlg,0);
        smooth_sum=0;
        temp_array=cur_filter;
        if (!IsDlgButtonChecked(hDlg,SMOOTH_SMALL)) 
        {
#ifdef OLD
            temp_array=cur_smooth_l;
#endif
            smooth_sum+=(temp_array[0]+temp_array[3*1]+temp_array[2*3+1]+
                temp_array[2*3+2])+3*(temp_array[3*1+1]+temp_array[3*1+2]+
                temp_array[1]+temp_array[2]);
        }
        for(i=0;i<3;i++) for(j=0;j<3;j++)
        {
            smooth_sum+=temp_array[i*3+j];
        }
        if (smooth_sum==0) smooth_sum=1;
        for(i=0;i<3;i++) for(j=0;j<3;j++)
        {
            temp_array[i*3+j]/=smooth_sum;
        }
        repaint_smooth(hDlg);
}        

void smooth(HWND hDlg,int num)
{
    float *sr=gendata[num]->ptr;
    float *ds;
    int size=gendata[num]->size;
    float *sa;
    int i,j;
//    float min_z,max_z;

    sa=cur_filter;
        
    switch(gendata[num]->type)
    {
        case DATATYPE_3D:
            copy_data(&gendata[3],&gendata[num]);
            unequalize(gendata[num]);
            ds=gendata[3]->ptr;
            if (IsDlgButtonChecked(hDlg,SMOOTH_SMALL))
            {
                for(i=1;i<size-1;i++) for(j=1;j<size-1;j++)
                {
                    ds[size*j+i]=sr[size*j+i]*sa[4]+
                        sr[size*(j-1)+i-1]*sa[8]+
                        sr[size*j+i-1]*sa[5]+
                        sr[size*(j+1)+i-1]*sa[2]+
                        sr[size*(j-1)+i]*sa[7]+
                        sr[size*(j+1)+i]*sa[1]+
                        sr[size*(j-1)+i+1]*sa[6]+
                        sr[size*j+i+1]*sa[3]+
                        sr[size*(j+1)+i]*sa[0];
                }
                /* bounding rows */
                for(i=1;i<size-1;i++) /*bottom row */
                {
                    ds[i]=ds[i+size];
                }
                for(i=1;i<size-1;i++) /*top row */
                {
                    ds[i+(size-1)*size]=ds[i+(size-2)*size];
                }
                /* bounding columns */
                for(i=1;i<size-1;i++) /*left col */
                {
                    ds[i*size]=ds[i*size+1];
                }
                for(i=1;i<size-1;i++) /*right col */
                {
                    ds[i*size+size-1]=ds[i*size+size-2];
                }
		/* corners */
                ds[0]=ds[1]; 
                ds[size-1]=ds[size-2];
                ds[-1+size*size]=ds[-2+size*size];
                ds[(size-1)*size]=ds[(size-1)*size+1];
            }
            break;
    }
//    find_min_max(gendata[3],&min_z,&max_z);
    reset_image(hDlg,3);
}
void calc_bitmap(int num)
{
    calc_bitmap_gen(gendata[num],&(im_bitmaps[num]),&(im_bitmap_size[num]));
}

void calc_bitmap_gen(datadef *this_image,unsigned char **this_bitmap,int *size)
{
    int newi,newj,data_size;
    float z;
    int bitmap_factor;
    int color32;
    
    if (this_image->valid && this_image->type==DATATYPE_3D)
    {
        switch(this_image->pal.type)
        {
            case PALTYPE_MAPPED:
                bitmap_factor=1;
                break;
            case PALTYPE_LOGICAL:
                bitmap_factor=3;
                break;
        }
        data_size=this_image->clip_max-this_image->clip_min+1;
        *size=
            find_size(this_image->size);
        if (!data_size)
            data_size = 1;
        if (*this_bitmap == NULL)
            *this_bitmap = (unsigned char *) malloc(bitmap_factor*(*size)* (*size)* sizeof(unsigned char));
        else
        {
            *this_bitmap = (unsigned char *) 
                realloc((void *) *this_bitmap, bitmap_factor*(*size)* (*size)* sizeof(unsigned char));
        }
    
        switch (this_image->pal.type)
        {
            case PALTYPE_MAPPED:
                for(newi=0;newi<this_image->size;newi++)
                    for(newj=0;newj<this_image->size;newj++)
                    {
                        *(*this_bitmap+newi*(*size)+newj)=
                            color_index(this_image,newi,newj);
                    }
                break;
            case PALTYPE_LOGICAL:
                for(newi=0;newi<this_image->size;newi++)
                    for(newj=0;newj<this_image->size;newj++)
                    {
                        z=*(this_image->ptr+newi*this_image->size+newj);
                        memcpy(*this_bitmap+(newi*(*size)+newj)
                            *bitmap_factor,bgr(this_image,z,&color32),3);
                    }
                break;
        }
                
    }
}
    
int time_index(int number, float t)
{
    int i;
    
    for(i=0;i<gendata[number]->size-1;i++)
    {
        if (*(gendata[number]->time2d+i)<=t && 
            *(gendata[number]->time2d+i+1)>t)
            return i;
    }
    if (t==*(gendata[number]->time2d+i)) return i;
    else return 0;
}

void im_do_scrolls(HWND hDlg,int number)
{
    if (gendata[number]->valid)
    {
        switch(gendata[number]->type)
        {
            case DATATYPE_3D:
                if (im_screenlines[number]<gendata[number]->size)
                {
                    EnableWindow(GetDlgItem(hDlg,IM_X_SCROLL),TRUE);
                    EnableWindow(GetDlgItem(hDlg,IM_Y_SCROLL),TRUE);
                    SetScrollRange(GetDlgItem(hDlg,IM_X_SCROLL),SB_CTL, 0,
                        gendata[number]->size-im_screenlines[number],FALSE);
                    SetScrollRange(GetDlgItem(hDlg,IM_Y_SCROLL),SB_CTL, 0,
                        gendata[number]->size-im_screenlines[number],FALSE);
                    SetScrollPos(GetDlgItem(hDlg,IM_X_SCROLL),SB_CTL,im_src_x[number],TRUE);
                    SetScrollPos(GetDlgItem(hDlg,IM_Y_SCROLL),SB_CTL,
                        gendata[number]->size-im_screenlines[number]-im_src_y[number],TRUE);
                }
                else
                {
                    EnableWindow(GetDlgItem(hDlg,IM_X_SCROLL),FALSE);
                    EnableWindow(GetDlgItem(hDlg,IM_Y_SCROLL),FALSE);
                }
                break;
            case DATATYPE_2D:
                EnableWindow(GetDlgItem(hDlg,IM_Y_SCROLL),FALSE);
                if (image_lowert[number]>*(gendata[number]->time2d) ||
                    image_highert[number]<*(gendata[number]->time2d+
                        gendata[number]->size-1))
                {
                    EnableWindow(GetDlgItem(hDlg,IM_X_SCROLL),TRUE);
                    SetScrollRange(GetDlgItem(hDlg,IM_X_SCROLL),SB_CTL, 0,
                        gendata[number]->size-time_index(number,image_highert[number])+
                            time_index(number,image_lowert[number]),FALSE);
                    SetScrollPos(GetDlgItem(hDlg,IM_X_SCROLL),SB_CTL,
                        time_index(number,(image_lowert[number]+image_highert[number])/2),TRUE);
                }
                else
                {
                    EnableWindow(GetDlgItem(hDlg,IM_X_SCROLL),FALSE);
                }
                
                break;
        }
    }
    else
    {
        EnableWindow(GetDlgItem(hDlg,IM_X_SCROLL),FALSE);
        EnableWindow(GetDlgItem(hDlg,IM_Y_SCROLL),FALSE);
    }
            
}


int floatcmp(float *f1,float *f2)
{
    if (*f1<*f2) return -1;
    else if (*f1>*f2) return 1;
    else return 0;
}
    
void free_dtrans(DTRANS *dtrans)
{
    int i;
    for(i=0;i<dtrans->listbox.num;i++)
    {
        free(dtrans->listbox.str[i]);
        free(dtrans->dtrans[i]);
    }
}    
    
void read_dtrans( DTRANS * dtrans , char * filename)
{
    FILE *fp;
    char buf[DTRANS_BUF_MAX];
    int i,j;
    
    fp=fopen(filename,"r");
    buf[0]=eatspace(fp);
    fgets(buf+1,DTRANS_BUF_MAX-1,fp);
    dtrans->listbox.num=atoi(buf);
//      sprintf(string,"%d",dtrans->listbox.num);
//      MessageBox(hDlg,string,"test",MB_ICONHAND);
    for(i=0;i<dtrans->listbox.num;i++)
    {
        /* get name */
        buf[0]=eatspace(fp);
        fgets(buf+1,DTRANS_BUF_MAX-1,fp);
        buf[strlen(buf)-1]='\0';
        dtrans->listbox.str[i]=(char *)malloc(sizeof(char)*(strlen(buf)+1));
        strcpy(dtrans->listbox.str[i],buf);
//      MessageBox(hDlg,buf,"test",MB_ICONHAND);
        
        dtrans->dtrans[i]=(float *)malloc(sizeof(float)*9);
        /* get rows */
        for(j=0;j<3;j++)
        {
            buf[0]=eatspace(fp);
            fgets(buf+1,DTRANS_BUF_MAX-1,fp);
            buf[strlen(buf)-1]='\0';
            *(dtrans->dtrans[i]+3*j+0)=atof(strtok(buf," "));
            *(dtrans->dtrans[i]+3*j+1)=atof(strtok(NULL," "));
            *(dtrans->dtrans[i]+3*j+2)=atof(strtok(NULL," "));
        }
    }
/*
    dtrans->listbox.num++;
    dtrans->listbox.str[dtrans->listbox.num-1]=(char *)malloc(sizeof(char)*10);
    strcpy(dtrans->listbox.str[i],"Custom");
*/
    fclose(fp);
}
            
void copy_filter(float dest[], float *src)
{
    int i,j;
    
            
        
    for(i=0;i<3;i++) for(j=0;j<3;j++) 
    {
        dest[i*3+j]=src[i*3+j];
    }
}

void repaint_sel(HWND hDlg,SEL_REGION *region)
{
/*
    if (sel_r1.image==this_image)
*/
    if (region->pt1->x<0 || region->pt1->y <0) return;
    switch(region->type)
    {
        case SEL_POINTS:
            
            if (region->flags & SEL_SECONDSEL)
            {
                paint_cross(hDlg,region,hXcross);
                sel2_on=1;
            }
            else
            {
                paint_cross(hDlg,region,hCross);
                sel1_on=1;
            }
                
            break;
        case SEL_LINE:
            paint_line(hDlg,region);
            break;
        case SEL_SQUARE:
            paint_rect(hDlg,region);
            if (region->flags & SEL_INVERTED) im_invert(hDlg,region);
            break;
        case SEL_CIRCLE:
            paint_ellipse(hDlg,region);
            if (region->flags & SEL_INVERTED) im_invert(hDlg,region);
            break;
    }
}

void paint_line(HWND hDlg,SEL_REGION *region)
{
    HDC             hDC;
    HDC             hMemDC;
/*
    LOGBRUSH        lplb;
    HBRUSH          hBrush,
                    hBrushOld;
    HPEN            hPen,
                    hPenOld;
*/
    HRGN            hRgn;
    int OldROP;
    int x,y,x_offset,y_offset,image_size,pixel_size,x2,y2;
    SEL_POINT pt1=*(region->pt1);
    SEL_POINT pt2=*(region->pt2);
    int this_image=region->image;
    
    if (pt1.x>=0 && pt1.y>=0 && gendata[region->image]->valid
        && pt2.x>=0 && pt2.y>=0)
    {
        if (zoomed_in)
        {
            x_offset=y_offset=LARGE_BORDER_SIZE;
            image_size=LARGE_IMAGE_SIZE;
        }
        else
        {
            x_offset=SMALL_BORDER_SIZE+
                (this_image%2)*(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE);
            y_offset=SMALL_BORDER_SIZE+
                (this_image/2)*(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE);
            
            image_size=SMALL_IMAGE_SIZE;
        }
        pixel_size=image_size/im_screenlines[this_image];
        x_offset-=7-pixel_size/2;
        y_offset-=7+pixel_size/2;
        hDC = GetDC(hDlg);
        hMemDC = CreateCompatibleDC(hDC);
        SelectPalette(hDC, hPal, 0);
        RealizePalette(hDC);
        SelectPalette(hMemDC, hPal, 0);
        RealizePalette(hMemDC);
        if (region->flags & SEL_SECONDSEL)
            SelectObject(hMemDC, hXcross);
        else
            SelectObject(hMemDC, hCross);
                    

        x = (pt1.x-im_src_x[this_image])*image_size/im_screenlines[this_image]; 
        y = image_size-
            (pt1.y-im_src_y[this_image])*image_size/im_screenlines[this_image]; 
        x2 = (pt2.x-im_src_x[this_image])*image_size/im_screenlines[this_image]; 
        y2 = image_size-
            (pt2.y-im_src_y[this_image])*image_size/im_screenlines[this_image]; 
        if (x >= 0 && y >= 0 && x < image_size+15 && y < image_size+15)
        {
            OldROP = SetROP2(hDC, R2_NOTXORPEN);
            BitBlt(hDC, (int) (x + x_offset), (int) (y + y_offset), 
                15, 15, hMemDC, 0, 0, SRCINVERT);
            x_offset+=7;
            y_offset+=7;
            hRgn = CreateRectRgn(x_offset, y_offset, 
                    x_offset+image_size,y_offset+image_size);
            SelectClipRgn(hDC, hRgn);
//            MoveTo(hDC, (int) (x+x_offset) , (int) (y+y_offset));
            MoveToEx(hDC, (int) (x+x_offset) , (int) (y+y_offset),NULL);
            
            LineTo(hDC, (int) (x2+x_offset), (int) (y2+y_offset));
            SetROP2(hDC, OldROP);
            SelectClipRgn(hDC,NULL);
            DeleteObject(hRgn);
        }
        
        DeleteDC(hMemDC);
        ReleaseDC(hDlg, hDC);
    }
        
        
}
void paint_rect(HWND hDlg,SEL_REGION *region)
{
    HDC             hDC;
    HDC             hMemDC;
/*
    LOGBRUSH        lplb;
    HBRUSH          hBrush,
                    hBrushOld;
    HPEN            hPen,
                    hPenOld;
*/
    HRGN            hRgn;
    int OldROP;
    int x,y,x_offset,y_offset,image_size,pixel_size,x2,y2,temp;
    int im_x1,im_x2,im_y1,im_y2,screenlines; 
    SEL_POINT pt1=*(region->pt1);
    SEL_POINT pt2=*(region->pt2);
    int this_image=region->image;
   
    if (pt1.x>=0 && pt1.y>=0 && gendata[this_image]->valid 
        && pt2.x>=0 && pt2.y>=0)
    {
        if (zoomed_in)
        {
            x_offset=y_offset=LARGE_BORDER_SIZE;
            image_size=LARGE_IMAGE_SIZE;
        }
        else
        {
            x_offset=SMALL_BORDER_SIZE+
                (this_image%2)*(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE);
            y_offset=SMALL_BORDER_SIZE+
                (this_image/2)*(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE);
            
            image_size=SMALL_IMAGE_SIZE;
        }
        screenlines=im_screenlines[this_image];
        pixel_size=image_size/screenlines;
        x_offset-=7-pixel_size/2;
        y_offset-=7+pixel_size/2;
        hDC = GetDC(hDlg);
        hMemDC = CreateCompatibleDC(hDC);
        SelectPalette(hDC, hPal, 0);
        RealizePalette(hDC);
        SelectPalette(hMemDC, hPal, 0);
        RealizePalette(hMemDC);
        if (region->flags & SEL_SECONDSEL)
            SelectObject(hMemDC, hXcross);
        else
            SelectObject(hMemDC, hCross);
        im_x1=pt1.x-im_src_x[this_image];
        im_y1=pt1.y-im_src_y[this_image];
        im_x2=pt2.x-im_src_x[this_image];
        im_y2=pt2.y-im_src_y[this_image];
        x = (im_x1)*image_size/screenlines; 
        y = image_size-
            im_y1*image_size/screenlines; 
        OldROP = SetROP2(hDC, R2_NOTXORPEN);
        if (x >= 0 && y >= 0 && x < image_size+15 && y < image_size+15)
            BitBlt(hDC, (int) (x + x_offset), (int) (y + y_offset), 
                15, 15, hMemDC, 0, 0, SRCINVERT);
        x_offset+=7-pixel_size/2;
        y_offset+=7+pixel_size/2;
        if (im_x2<im_x1)
        {
            temp=im_x2;
            im_x2=im_x1;
            im_x1=temp;
        }
        if (im_y2>im_y1)
        {
            temp=im_y2;
            im_y2=im_y1;
            im_y1=temp;
        }
        x = (im_x1)*image_size/screenlines; 
        y = image_size-
            im_y1*image_size/screenlines-pixel_size; 
        x2 = im_x2*image_size/screenlines+pixel_size; 
        y2 = image_size-
            im_y2*image_size/screenlines; 
        {
            
            hRgn = CreateRectRgn(x_offset, y_offset, 
                    x_offset+image_size,y_offset+image_size);
            SelectClipRgn(hDC, hRgn);
            if (x<0) x=0;
            if (x>=image_size) x=image_size-1;
            if (y<0) y=0;
            if (y>=image_size) y=image_size-1;
            if (x2<0) x2=0;
            if (x2>=image_size) x2=image_size-1;
            if (y2<0) y2=0;
            if (y2>=image_size) y2=image_size-1;
            PatBlt(hDC,(int) (x+x_offset),(int) (y+y_offset),
                (int) (x2-x+1),  (int) (y2-y+1),DSTINVERT);
                
            SetROP2(hDC, OldROP);
            SelectClipRgn(hDC,NULL);
            DeleteObject(hRgn);
        }
        
        DeleteDC(hMemDC);
        ReleaseDC(hDlg, hDC);
    }
        
        
}

void im_invert(HWND hDlg,SEL_REGION *region)
{
    HDC             hDC;
    int OldROP;
    int x_offset,y_offset,image_size;
    SEL_POINT pt1=*(region->pt1);
    SEL_POINT pt2=*(region->pt2);
    int this_image=region->image;
   
    if (pt1.x>=0 && pt1.y>=0 && gendata[this_image]->valid 
        && pt2.x>=0 && pt2.y>=0)
    {
        if (zoomed_in)
        {
            x_offset=y_offset=LARGE_BORDER_SIZE;
            image_size=LARGE_IMAGE_SIZE;
        }
        else
        {
            x_offset=SMALL_BORDER_SIZE+
                (this_image%2)*(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE);
            y_offset=SMALL_BORDER_SIZE+
                (this_image/2)*(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE);
            
            image_size=SMALL_IMAGE_SIZE;
        }
/*
        screenlines=im_screenlines[this_image];
        pixel_size=image_size/screenlines;
        x_offset-=7-pixel_size/2;
        y_offset-=7+pixel_size/2;
*/
        hDC = GetDC(hDlg);
        SelectPalette(hDC, hPal, 0);
        RealizePalette(hDC);
        OldROP = SetROP2(hDC, R2_NOTXORPEN);
        PatBlt(hDC,(int) (x_offset),(int) (y_offset),
            (int) (image_size),  (int) (image_size),DSTINVERT);
        SetROP2(hDC, OldROP);
        ReleaseDC(hDlg, hDC);
    }
        
        
}

void paint_ellipse(HWND hDlg,SEL_REGION *region)
{
    HDC             hDC;
    HDC             hMemDC;
    LOGBRUSH        lplb;
    HBRUSH          hBrush,
                    hBrushOld;
    HRGN            hRgn;
    int OldROP;
    int x,y,x_offset,y_offset,image_size,pixel_size,x2,y2;
    int im_x1,im_x2,im_y1,im_y2,screenlines; 
    SEL_POINT pt1=*(region->pt1);
    SEL_POINT pt2=*(region->pt2);
    int this_image=region->image;
   
    if (pt1.x>=0 && pt1.y>=0 && gendata[this_image]->valid 
        && pt2.x>=0 && pt2.y>=0)
    {
        if (zoomed_in)
        {
            x_offset=y_offset=LARGE_BORDER_SIZE;
            image_size=LARGE_IMAGE_SIZE;
        }
        else
        {
            x_offset=SMALL_BORDER_SIZE+
                (this_image%2)*(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE);
            y_offset=SMALL_BORDER_SIZE+
                (this_image/2)*(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE);
            
            image_size=SMALL_IMAGE_SIZE;
        }
        screenlines=im_screenlines[this_image];
        pixel_size=image_size/screenlines;
        x_offset-=7-pixel_size/2;
        y_offset-=7+pixel_size/2;
        hDC = GetDC(hDlg);
        hMemDC = CreateCompatibleDC(hDC);
        SelectPalette(hDC, hPal, 0);
        RealizePalette(hDC);
        SelectPalette(hMemDC, hPal, 0);
        RealizePalette(hMemDC);
        if (region->flags & SEL_SECONDSEL)
            SelectObject(hMemDC, hXcross);
        else
            SelectObject(hMemDC, hCross);
        im_x1=pt1.x-im_src_x[this_image];
        im_y1=pt1.y-im_src_y[this_image];
        im_x2=pt2.x-im_src_x[this_image];
        im_y2=pt2.y-im_src_y[this_image];
        x = (im_x1)*image_size/screenlines; 
        y = image_size-
            im_y1*image_size/screenlines; 
        OldROP = SetROP2(hDC, R2_NOTXORPEN);
        if (x >= 0 && y >= 0 && x < image_size+15 && y < image_size+15)
            BitBlt(hDC, (int) (x + x_offset), (int) (y + y_offset), 
                15, 15, hMemDC, 0, 0, SRCINVERT);
        x_offset+=7-pixel_size/2;
        y_offset+=7+pixel_size/2;
        
        im_x1-=pt2.x;
        im_y1-=pt2.y;
        im_x2=pt1.x-im_src_x[this_image]+pt2.x;
        im_y2=pt1.y-im_src_y[this_image]+pt2.y;
        
        x = (im_x1)*image_size/screenlines; 
        y = image_size-
            im_y1*image_size/screenlines; 
        x2 = im_x2*image_size/screenlines+pixel_size; 
        y2 = image_size-
            im_y2*image_size/screenlines-pixel_size; 
        {
            
            hRgn = CreateRectRgn(x_offset, y_offset, 
                    x_offset+image_size,y_offset+image_size);
            SelectClipRgn(hDC, hRgn);
            lplb.lbStyle = BS_SOLID;
            lplb.lbColor = RGB(0,0,0);
            hBrush = CreateBrushIndirect(&lplb);
            hBrushOld = SelectObject(hDC, hBrush);
            Ellipse(hDC,(int) (x+x_offset-1), (int) (y+y_offset-1),
                (int) (x2+x_offset+1), (int) (y2+y_offset+1));                
            SetROP2(hDC, OldROP);
            SelectClipRgn(hDC,NULL);
            DeleteObject(hRgn);
            SelectObject(hDC, hBrushOld);
            DeleteObject(hBrush);
        }
        
        DeleteDC(hMemDC);
        ReleaseDC(hDlg, hDC);
    }
        
        
}

void paint_cross(HWND hDlg,SEL_REGION *region,HBITMAP cross)
{
    HDC             hDC;
    HDC             hMemDC;
/*
    LOGBRUSH        lplb;
    HBRUSH          hBrush,
                    hBrushOld;
    HPEN            hPen,
                    hPenOld;
*/
    int x,y,x_offset,y_offset,image_size,pixel_size;
    SEL_POINT pt=*(region->pt1);
    int this_image=region->image;
    
    if (pt.x>=0 && pt.y>=0 && gendata[this_image]->valid)
    {
        if (zoomed_in)
        {
            x_offset=y_offset=LARGE_BORDER_SIZE;
            image_size=LARGE_IMAGE_SIZE;
        }
        else
        {
            x_offset=SMALL_BORDER_SIZE+
                (this_image%2)*(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE);
            y_offset=SMALL_BORDER_SIZE+
                (this_image/2)*(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE);
            
            image_size=SMALL_IMAGE_SIZE;
        }
        hDC = GetDC(hDlg);
        hMemDC = CreateCompatibleDC(hDC);
        SelectPalette(hDC, hPal, 0);
        RealizePalette(hDC);
        SelectPalette(hMemDC, hPal, 0);
        RealizePalette(hMemDC);
        SelectObject(hMemDC, cross);
        /* cross is 15x15 */
        x_offset-=7;
        y_offset-=7;
        switch(gendata[this_image]->type)
        {
            case DATATYPE_3D:
                pixel_size=image_size/im_screenlines[this_image];
                x_offset+=pixel_size/2;
                y_offset-=pixel_size/2;
                x = (pt.x-im_src_x[this_image])*image_size/im_screenlines[this_image]; 
                y = image_size-
                    (pt.y-im_src_y[this_image])*image_size/im_screenlines[this_image]; 
                break;
            case DATATYPE_2D:
                x=(*(gendata[this_image]->time2d+pt.x)-image_lowert[this_image])
                            *image_size/(image_highert[this_image]-
                            image_lowert[this_image]);
                y=-((float)*(gendata[this_image]->data2d+pt.x)
                        -image_higherv[this_image])*(image_size)/
                        (image_higherv[this_image]-image_lowerv[this_image]+1);
                break;
            case DATATYPE_GEN2D:
                switch(gendata[this_image]->type2d)
                {
                    case TYPE2D_SPEC_I:
                    case TYPE2D_SPEC_Z:
                    case TYPE2D_SPEC_2:
                    case TYPE2D_SPEC_3:
                    case TYPE2D_MASSPEC:
                        x=(gendata[this_image]->start+pt.x*gendata[this_image]->step-
                            image_lowert[this_image])*(image_size)/
                            (image_highert[this_image]-image_lowert[this_image]+1);
                        y=((float)image_higherv[this_image]-
                            gendata[this_image]->yf[pt.x])*image_size/
                            (image_higherv[this_image]-image_lowerv[this_image]+1);
                        break;
                    case TYPE2D_CUT:
                        x=(gendata[this_image]->min_x+pt.x*
                            gendata[this_image]->max_x/gendata[this_image]->size-
                            image_lowert[this_image])*(image_size)/
                            (image_highert[this_image]-image_lowert[this_image]+1);
                        y=((float)image_higherv[this_image]-
                            gendata[this_image]->yf[pt.x])*image_size/
                            (image_higherv[this_image]-image_lowerv[this_image]+1);
                        break;
                }
                break;
        }
        if (x >= 0 && y >= 0 && x < image_size+15 && y < image_size+15)
        {
            BitBlt(hDC, (int) (x +x_offset), (int) (y +y_offset ), 
                15, 15, hMemDC, 0, 0, SRCINVERT);
        }
        
        DeleteDC(hMemDC);
        ReleaseDC(hDlg, hDC);
    }
        
        
}

void find_clip_rect(HWND hDlg,RECT *r)
{
    int xpos,ypos,x_offset,y_offset;
    RECT offset;
    
    GetWindowRect(hDlg,&offset);
    x_offset=offset.left+GetSystemMetrics(SM_CXBORDER)+
        GetSystemMetrics(SM_CXDLGFRAME);
    y_offset=offset.top;
    y_offset+=GetSystemMetrics(SM_CYCAPTION)+
        GetSystemMetrics(SM_CYDLGFRAME);
    
    if (!zoomed_in)
    {
        xpos=x_offset+
            SMALL_BORDER_SIZE+(current_image % 2)*(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE);
        ypos=y_offset+
            SMALL_BORDER_SIZE+(current_image/2)*(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE);
        r->left=xpos;
        r->right=xpos+SMALL_IMAGE_SIZE;
        r->top=ypos;
        r->bottom=ypos+SMALL_IMAGE_SIZE;
    
    }
    else
    {
        xpos=x_offset+LARGE_BORDER_SIZE;
        ypos=y_offset+LARGE_BORDER_SIZE;
        r->left=xpos;
        r->right=xpos+LARGE_IMAGE_SIZE;
        r->top=ypos;
        r->bottom=ypos+LARGE_IMAGE_SIZE;
    }
}    
int
GifGetPixel( x, y )
int x, y;
    {
    int color;
    color=*(im_bitmaps[current_image]+
        (gendata[current_image]->size-y-1)*im_bitmap_size[current_image]+x);
//        (y)*im_bitmap_size[current_image]+x);
    return color;
    }

void save_gif(char *fname)
{
    FILE *fp;
    
    fp=fopen(fname,"wb");
    GIFEncode(
        fp, 
        gendata[current_image]->size,gendata[current_image]->size, 
        0, 0, 8,
        image_dacbox, 
        GifGetPixel );
        
//    fclose(fp);
}

void square_coords(SEL_POINT pt1,SEL_POINT *pt2,int image)
{
    int delta_x,delta_y;
    
    delta_x=abs(pt1.x-pt2->x);
    delta_y=abs(pt1.y-pt2->y);
    if (delta_x > delta_y)
    {
        inc_pt_dist(pt1.y, &(pt2->y),delta_x-delta_y);
        if (pt2->y <0)
        {
            inc_pt_dist(pt1.x,&(pt2->x),pt2->y);
            pt2->y=0;
        }
        else if (pt2->y >= gendata[image]->size)
        {
            inc_pt_dist(pt1.x,&(pt2->x),gendata[image]->size-pt2->y-1);
            pt2->y=gendata[image]->size-1;
        }
    }
    else
    {
        inc_pt_dist(pt1.x, &(pt2->x),delta_y-delta_x);
        if (pt2->x <0)
        {
            inc_pt_dist(pt1.y,&(pt2->y),pt2->x);
            pt2->x=0;
        }
        else if (pt2->x >= gendata[image]->size)
        {
            inc_pt_dist(pt1.y,&(pt2->y),gendata[image]->size-pt2->x-1);
            pt2->x=gendata[image]->size-1;
        }
    }        
            
    
}
void circle_coords(SEL_POINT pt1,SEL_POINT *pt2,int image)
{
    int delta_x,delta_y;
    
    delta_x=abs(pt1.x-pt2->x);
    delta_y=abs(pt1.y-pt2->y);
    if (delta_x > delta_y)
    {
        inc_pt_dist(pt1.y, &(pt2->y),delta_x-delta_y);
        if (pt2->y <0)
        {
            inc_pt_dist(pt1.x,&(pt2->x),pt2->y);
            pt2->y=0;
        }
        else if (pt2->y >= gendata[image]->size)
        {
            inc_pt_dist(pt1.x,&(pt2->x),gendata[image]->size-pt2->y-1);
            pt2->y=gendata[image]->size-1;
        }
    }
    else
    {
        inc_pt_dist(pt1.x, &(pt2->x),delta_y-delta_x);
        if (pt2->x <0)
        {
            inc_pt_dist(pt1.y,&(pt2->y),pt2->x);
            pt2->x=0;
        }
        else if (pt2->x >= gendata[image]->size)
        {
            inc_pt_dist(pt1.y,&(pt2->y),gendata[image]->size-pt2->x-1);
            pt2->x=gendata[image]->size-1;
        }
    }        
            
    
}

void inc_pt_dist(int x1,int *x2,int dist)
{
    if (x1<*x2)
        *x2+=dist;
    else *x2-=dist;
}

int pt_in_region(int x,int y, SEL_REGION *region)
{
    double x1,y1,x2,y2,temp;
    int result=0;
    
    if (region==NULL) return 1;
    switch(region->type)
    {
        case SEL_POINTS:
        case SEL_LINE:
            return 1;
            break;
        case SEL_SQUARE:
            x1=region->pt1->x;
            x2=region->pt2->x;
            y1=region->pt1->y;
            y2=region->pt2->y;
            if (x1>x2)
            {
                temp=x1;
                x1=x2;
                x2=temp;
            }
            if (y1>y2)
            {
                temp=y1;
                y1=y2;
                y2=temp;
            }
            if (x>=x1 && x<=x2 && y>=y1 && y<=y2) result=1;
            else result= 0;
            if (region->flags && SEL_INVERTED) result=!result;
            break;
        case SEL_CIRCLE:
            x1=region->pt1->x-x;
            x2=region->pt2->x;
            y1=region->pt1->y-y;
            y2=region->pt2->y;
            if (x1*x1/x2/x2+y1*y1/y2/y2<=1) result= 1;
            else result=0;
            if (region->flags && SEL_INVERTED) result=!result;
            break;
    }
    return result;
}
        
    
int image2pixel(float image_x,float image_y,int *pixel_x,int *pixel_y,
    int image)
{
    int screen_size,x_offset,y_offset,pixel_size;
    
    

    if (gendata[image]->type!=DATATYPE_3D)
        return 0;    
    if (!zoomed_in)
    {
        x_offset=SMALL_BORDER_SIZE+(image % 2)*(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE);
        y_offset=SMALL_BORDER_SIZE+(image/2)*(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE);
        screen_size=SMALL_IMAGE_SIZE;
    }
    else 
    {
        if (image!=current_image) return 0;
        screen_size=LARGE_IMAGE_SIZE;
        x_offset=LARGE_BORDER_SIZE;
        y_offset=LARGE_BORDER_SIZE;
    }
    pixel_size=screen_size/im_screenlines[image];
    x_offset+=pixel_size/2;
    y_offset-=pixel_size/2;
    if (image_x<im_src_x[image] || 
        image_x>im_src_x[image]+im_screenlines[image]-1 ||
        image_y<im_src_y[image] ||
        image_y>im_src_y[image]+im_screenlines[image]-1)
        return 0;
    *pixel_x=(int)(((float)image_x-(float)im_src_x[image])*(float)screen_size/
        (float)im_screenlines[image]);
    *pixel_y=(int)((float)screen_size-1-((float)image_y-(float)im_src_y[image])*
        (float)screen_size/(float)im_screenlines[image]);
    *pixel_x+=x_offset;
    *pixel_y+=y_offset;
    return 1;
        
}


static void repaint_grids(HWND hDlg,int this_image)
{
    int i;
    int count;
    
    float r;
    
    for(i=0;i<GRID_MAX_LINES;i++)
    {
        if (!grids[this_image].line[i].hide)
        {
            r=grids[this_image].line[i].r;
            count=0;
/*
draw_grid_line(hDlg,r,grids[this_image].line[i].theta,this_image);
*/
/*
            while(draw_grid_line(hDlg,r,grids[this_image].line[i].theta,this_image)
                && count<gendata[this_image]->size)
*/
            while (count<gendata[this_image]->size)
            {
                draw_grid_line(hDlg,r,grids[this_image].line[i].theta,this_image);
                r+=grids[this_image].line[i].dist;
                count++;
            }
            if (grids[this_image].line[i].theta>0)
            {
                r=grids[this_image].line[i].r;
                count=0;
/*
                while(draw_grid_line(hDlg,r,grids[this_image].line[i].theta,this_image)
                    && count<gendata[this_image]->size)
*/
                while (count<gendata[this_image]->size)
                {
                    draw_grid_line(hDlg,r,grids[this_image].line[i].theta,this_image);
                    r-=grids[this_image].line[i].dist;
                    count++;
                }
            }
        }
    }
    
        
}

static int draw_grid_line(HWND hDlg,float r,float theta,int this_image)
{
    int sofar;
    
    float *x,*y;
    float temp_x1,temp_y1,temp_x2,temp_y2;
    int xs,ys;
    float line_x,line_y;
    int result=0;
    HDC hDC;
    LOGBRUSH      lplb;
    HBRUSH        hBrush,hBrushOld;
    HPEN          hPen,hPenOld;
    
    hDC = GetDC(hDlg);

    SelectPalette(hDC,hPal,0);
    RealizePalette(hDC);

    lplb.lbStyle = BS_HOLLOW;
    hBrush = CreateBrushIndirect(&lplb);
    hBrushOld = SelectObject(hDC,hBrush);
    hPen = CreatePen(PS_SOLID,1,RGB(255,0,255));
    hPenOld = SelectObject(hDC,hPen);

    sofar=0;
    line_x=r*
        cos(RADIAN(theta+90));
    line_y=r*
        sin(RADIAN(theta+90));
    x=&temp_x1;
    y=&temp_y1;
    if (horiz_int(line_x,line_y,theta,x,im_src_y[this_image]))
    {
        if (*x>=im_src_x[this_image] && 
            *x<=im_src_x[this_image]+im_screenlines[this_image]-1)
        {
            *y=im_src_y[this_image];
            sofar++;
            x=&temp_x2;
            y=&temp_y2;
        }
    }
    if (horiz_int(line_x,line_y,theta,x,
        im_src_y[this_image]+im_screenlines[this_image]-1))
    {
        if (*x>=im_src_x[this_image] && 
            *x<=im_src_x[this_image]+im_screenlines[this_image]-1)
        {
            *y=im_src_y[this_image]+im_screenlines[this_image]-1;
            sofar++;
            x=&temp_x2;
            y=&temp_y2;
        }
    }
    if (sofar<2)
    {
        if (vert_int(line_x,line_y,theta,im_src_x[this_image],y))
        {
            if (*y>=im_src_y[this_image] && 
                *y<=im_src_y[this_image]+im_screenlines[this_image]-1)
            {
                *x=im_src_x[this_image];
                sofar++;
                x=&temp_x2;
                y=&temp_y2;
            }
        }
    }
    if (sofar<2)
    {
        if (vert_int(line_x,line_y,theta,
            im_src_x[this_image]+im_screenlines[this_image]-1,y))
        {
            if (*y>=im_src_y[this_image] && 
                *y<=im_src_y[this_image]+im_screenlines[this_image]-1)
            {
                *x=im_src_x[this_image]+im_screenlines[this_image]-1;
                sofar++;
                x=&temp_x2;
                y=&temp_y2;
            }
        }
    }
    if (sofar==2)
    {

          
        if (image2pixel(temp_x1,temp_y1,&xs,&ys,this_image))
        {
//            MoveTo(hDC,xs,ys);
            MoveToEx(hDC,xs,ys,NULL);
/*
        sprintf(string,"%f %f %f %f %d %d %d",temp_x1,temp_y1,temp_x2,temp_y2,this_image,xs,ys);
        MessageBox(hDlg,string,"Debug",MB_ICONHAND);
*/
            if (image2pixel(temp_x2,temp_y2,&xs,&ys,this_image))
            {
                LineTo(hDC,xs,ys);
                result=1;
            }
        }
    }
    SelectObject(hDC,hBrushOld);
    SelectObject(hDC,hPenOld);
    DeleteObject(hBrush);
    DeleteObject(hPen);
    ReleaseDC(hDlg,hDC);
    return(result);
        
}

void notify_all()
{
    if (VscaleOn)
    {
        SendMessage(VscaleWnd, WM_COMMAND, VSCALE_NEW_IMAGE, 0);
    }
    if (FourierOn)
    {
        SendMessage(FourierWnd, WM_COMMAND, FOURIER_NEW_IMAGE, 0);
    }
    if (GridOn)
    {
        SendMessage(GridWnd, WM_COMMAND, GRID_NEW_IMAGE, 0);
    }
    if (Comment2On)
    {
        SendMessage(Comment2Wnd, WM_COMMAND, COMMENT2_NEW_IMAGE, 0);
    }
    if (LogpalOn)
    {
        SendMessage(LogpalWnd, WM_COMMAND, LOGPAL_NEW_IMAGE, 0);
    }
    if (CountOn)
    {
        SendMessage(CountWnd, WM_COMMAND, COUNT_NEW_IMAGE, 0);
    }
        
}

void image_set_title(HWND hDlg)
{
      if(gendata[current_image]->valid)
      {
        sprintf(string,"Data Manipulation: %s",gendata[current_image]->filename);
        SetWindowText(hDlg,string);
      }
      else SetWindowText(hDlg,"Data Manipulation");
}

int color_index(datadef *this_data,int row,int col)
{
    int z;
    int data_size;
    
    data_size=this_data->clip_max-this_data->clip_min+1;

    if (this_data->equalized)
    {
        z=bin_find(&(this_data->fhist[0]),0,LOGPALUSED-1,
            *(this_data->ptr+row*this_data->size+col));
    }
    else
    {    
        z=((*(this_data->ptr+
            row*this_data->size+col) -this_data->clip_min)*
            LOGPALUSED/data_size);
        if (z<0 || z>LOGPALUSED) z=0;
    }
    return z;
}

#ifdef OLD
char *bgr(datadef *this_data,float this_z,int *color32)
{
    char *color;
    int z;
    float data_size;
    LOGPAL_EL *this_logpal;
    int pos;
    float fpos;
    
    color=(char *) color32;
    switch(this_data->pal.type)
    {
#ifdef OLD
        case PALTYPE_MAPPED:
            data_size=this_data->clip_max-this_data->clip_min+1;
    
            if (this_data->equalized)
            {
                z=bin_find(&(this_data->fhist[0]),0,LOGPALUSED-1,this_z);
            }
            else
            {    
                z=((this_z-this_data->clip_min)*
                    LOGPALUSED/data_size);
                if (z<0 || z>LOGPALUSED) z=0;
            }
            break;
#endif
        case PALTYPE_LOGICAL:
            data_size=this_data->max_z-this_data->min_z;
            if (data_size==0) data_size=1;
            fpos=((this_z -this_data->min_z)*
                    ((float)LOGPAL_NUM_INDICES)/data_size);
            pos=floor(fpos);
            if (pos>=LOGPAL_NUM_INDICES) pos=LOGPAL_NUM_INDICES-1;
            if (pos<0) pos=0;
            this_logpal=this_data->pal.logpal;
            while(pos<this_logpal->index || pos>this_logpal->next->index)
            {
                this_logpal=this_logpal->next;
            }
            if (this_logpal->equalized)
            {
                z=bin_find(&(this_logpal->fhist[0]),0,LOGPALUSED-1,this_z);
                fpos=((float)z)/(float)(LOGPALUSED-1);
            }
            else
            {
                fpos=(fpos-this_logpal->index)/(float)(this_logpal->next->index-
                    this_logpal->index+1);
            }
            fpos=pow(fpos,this_logpal->gamma);
            *color=this_logpal->color.b+(this_logpal->next->color.b-
                this_logpal->color.b)*fpos;
            *(color+1)=this_logpal->color.g+(this_logpal->next->color.g-
                this_logpal->color.g)*fpos;
            *(color+2)=this_logpal->color.r+(this_logpal->next->color.r-
                this_logpal->color.r)*fpos;            
            break;
    }
            
    return color;
}
#endif
    
void new_image(HWND hDlg)
{
    float min_z,max_z;
    
    if (sel_r1.image==current_image)
    {
      sel_r1.image=0;
      sel_p1.x=-1;
      sel_p1.y=-1;
      sel_p2.x=-1;
      sel_p2.y=-1;
      sel1_on=0;
    } 
    if (sel_r2.image==current_image)
    {
      sel_r2.image=0;
      sel_p3.x=-1;
      sel_p3.y=-1;
      sel_p4.x=-1;
      sel_p4.y=-1;
      sel2_on=0;
    } 
    switch(gendata[current_image]->type)
    {
        case DATATYPE_3D:          
          reset_image(hDlg,current_image);
          if (image_auto_illuminate)
          {
            SendMessage(hDlg, WM_COMMAND, IMAGE_ILLUMINATE,0);
          } 
          break;
        case DATATYPE_2D:
            find_min_max(gendata[current_image],&min_z,&max_z);
            image_lowerv[current_image]=IN_MIN;
            image_higherv[current_image]=IN_MAX;
            image_lowert[current_image]=*(gendata[current_image]->time2d);
            image_highert[current_image]=*(gendata[current_image]->time2d+gendata[current_image]->size-1);
            im_do_scrolls(hDlg,current_image);
            repaint_image(hDlg,current_image,NULL);
            image_set_title(hDlg);
            notify_all();
            break;
        case DATATYPE_GEN2D:
//mprintf("new image");
            find_min_max(gendata[current_image],&min_z,&max_z);
            image_lowerv[current_image]=IN_MIN;
            image_higherv[current_image]=IN_MAX;
            image_lowert[current_image]=gendata[current_image]->min_x;
            image_highert[current_image]=gendata[current_image]->max_x;
            im_do_scrolls(hDlg,current_image);
            repaint_image(hDlg,current_image,NULL);
            image_set_title(hDlg);
            notify_all();
            break;
      }
      if (image_auto_info)
      {
        SendMessage(hDlg, WM_COMMAND, IMAGE_INFO,0);      
      } 
        
}       

void reset_image(HWND hDlg,int num)
{
    float min_z,max_z;
    
    switch(gendata[num]->type)
    {
        case DATATYPE_2D:
            image_higherv[num]=IN_MAX;
            image_lowerv[num]=IN_MIN;
            image_highert[num]=*(gendata[current_image]->time2d+
                gendata[current_image]->size-1);
            image_lowert[num]=*(gendata[current_image]->time2d);
            break;
        case DATATYPE_GEN2D:
            image_higherv[num]=IN_MAX;
            image_lowerv[num]=IN_MIN;
            image_highert[num]=gendata[num]->max_x;
            image_lowert[num]=gendata[num]->min_x;
            break;
        case DATATYPE_3D:
            im_src_x[num]=im_src_y[num]=0;
            im_screenlines[num]=gendata[num]->size;
            break;
            
    }    
    find_min_max(gendata[num],&min_z,&max_z);
    im_src_x[num]=im_src_y[num]=0;
    im_screenlines[num]=gendata[num]->size;
    im_do_scrolls(hDlg,num);
    if (gendata[num]->type==DATATYPE_3D)
    {
        if (gendata[num]->pal.type==PALTYPE_LOGICAL)
        {
          destroy_logpal(&(gendata[num]->pal.logpal));
        }
        gendata[num]->pal.type=PALTYPE_MAPPED;
        memcpy(gendata[num]->pal.dacbox,image_dacbox,SIZEOFMPAL);
    }
    calc_bitmap(num);
    repaint_image(hDlg,num,NULL);
    notify_all();
    image_set_title(hDlg);
}  

static void repaint_trace_limits(HWND hDlg,int number)
{
    int ys;
    int OldROP;
    HDC hDC;
    int x1,x2,y1,y2,screen_size;
    
    hDC=GetDC(hDlg);
    SelectPalette(hDC,hPal,0);
    RealizePalette(hDC);
    SetMapMode(hDC,MM_TEXT);
    if (!zoomed_in)
    {
        x1=SMALL_BORDER_SIZE+(number % 2)*(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE);
        y1=SMALL_BORDER_SIZE+(number/2)*(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE);
        x2=x1+SMALL_IMAGE_SIZE-1;
        y2=y1+SMALL_IMAGE_SIZE-1;
        screen_size=SMALL_IMAGE_SIZE;
    }
   else
   {
        x1=LARGE_BORDER_SIZE;
        y1=LARGE_BORDER_SIZE;
        x2=x1+LARGE_IMAGE_SIZE-1;
        y2=y1+LARGE_IMAGE_SIZE-1;
        screen_size=LARGE_IMAGE_SIZE;
    }
    OldROP = SetROP2(hDC, R2_NOT);
    ys=-(trace_from
        -image_higherv[number])*(y2-y1+1)/(image_higherv[number]-
        image_lowerv[number]+1)+y1;
//    MoveTo(hDC, (int) (x1) , (int) (ys));
    MoveToEx(hDC, (int) (x1) , (int) (ys),NULL);

    LineTo(hDC, (int) (x2), (int) (ys));
    ys=-(trace_to
        -image_higherv[number])*(y2-y1+1)/(image_higherv[number]-
        image_lowerv[number]+1)+y1;
//    MoveTo(hDC, (int) (x1) , (int) (ys));
    MoveToEx(hDC, (int) (x1) , (int) (ys), NULL);

    LineTo(hDC, (int) (x2), (int) (ys));
    
    SetROP2(hDC, OldROP);
    ReleaseDC(hDlg,hDC);
}

static void image_cut_profile(SEL_REGION *region)
{
    // region->type is assumed to be a SEL_LINE

    int this_image=region->image;
    int size,i;
    float x,y,length;
    
    
    if (this_image!=3)
    {
        copy_data(&gendata[3],&gendata[this_image]);
        free_data_ptrs(&gendata[3]);
        gendata[3]->pal.type=PALTYPE_MAPPED; // I have no idea why Moh did this. -mw
        gendata[3]->type2d=TYPE2D_CUT;

        gendata[3]->cut_x1=region->pt1->x;
        gendata[3]->cut_x2=region->pt2->x;
        gendata[3]->cut_y1=region->pt1->y;
        gendata[3]->cut_y2=region->pt2->y;

        gendata[3]->min_x=0.0;
	gendata[3]->max_x = length = sqrt( (gendata[3]->cut_x2-gendata[3]->cut_x1)*(gendata[3]->cut_x2-gendata[3]->cut_x1) + 
					   (gendata[3]->cut_y2-gendata[3]->cut_y1)*(gendata[3]->cut_y2-gendata[3]->cut_y1)   );

        size = length*IMAGE_CUT_XFACTOR + 1; // +1 is for endpoint
        alloc_data_ptrs(&gendata[3],DATATYPE_GEN2D,size,GEN2D_NONE,GEN2D_FLOAT,1);

        for(i=0;i<size;i++)
        {
            x=gendata[3]->cut_x1+
                ((float)i)/((float)(size-1))*(gendata[3]->cut_x2-gendata[3]->cut_x1);
            y=gendata[3]->cut_y1+
                ((float)i)/((float)(size-1))*(gendata[3]->cut_y2-gendata[3]->cut_y1);

            if (x<0 || x>=gendata[this_image]->size || y<0 ||
                y>=gendata[this_image]->size)
                mprintf("x or y out of range");

            *(gendata[3]->yf+i)=interp_z(gendata[this_image],x,y);
        }
    }

}

static void repaint_cut_graphics(HDC hdc,HPEN hpen,datadef *this_data,
    int screen_min,int screen_max,
    int x_min,int y_min,int x_max,int y_max, int vert_max, int vert_min)
{    
#define YFS(Y)      (((float)vert_max-(float)Y)* \
                            (float)(y_max-y_min+1)/ \
                            (float)(vert_max-vert_min))+y_min
    int start,end;
    HPEN hpen_old;
    int xs,ys,i;
    float yfs;
    float image_cut_factor;
        
    hpen_old=SelectObject(hdc,hpen);
    image_cut_factor=this_data->size/this_data->max_x;
        
    if (screen_min>this_data->min_x) 
    {
        start=(screen_min-this_data->min_x)*image_cut_factor;
    }
    else
    {
        start=0;
    }
    if (screen_max<this_data->max_x)
    {
        end=(screen_max-this_data->min_x)*image_cut_factor+1;
        if (end>this_data->size) end=this_data->size;
    }
    else 
    {
        end=this_data->size;
    }
    
#ifdef OLD
    start=0;
    end=this_data->size;
#endif

    xs=(this_data->min_x-screen_min)*(x_max-x_min+1)/
        (screen_max-screen_min+1)+x_min;
    yfs=YFS(this_data->yf[start]);
    xs=max(min(xs,x_max),x_min);
    yfs=max(min(yfs,y_max),y_min);
    ys=(int) yfs;
//    MoveTo(hdc,xs,ys);
    MoveToEx(hdc,xs,ys,NULL);
    for(i=start+1;i<end;i++)
    {
        xs=((float)(i-start)/(float)(end-start+1))*(x_max-x_min+1)+x_min;
        yfs=YFS(this_data->yf[i]);
        xs=max(min(xs,x_max),x_min);
        yfs=max(min(yfs,y_max),y_min);
        ys=(int) yfs;
        LineTo(hdc,xs,ys);
    }
        
    SelectObject(hdc,hpen_old);
}

#ifdef OLD
void paint_circles(HWND hDlg,int this_image,COUNT_EL *this_list,int size,
    COLORREF color)
{
    HDC             hDC;
    LOGBRUSH        lplb;
    HBRUSH          hBrush,
                    hBrushOld;
    HRGN            hRgn;
    int OldROP;
    int x,y,x_offset,y_offset,image_size,pixel_size,x2,y2,temp;
    int im_x1,im_x2,im_y1,im_y2,screenlines; 
    COUNT_EL *this_el;
   
    if (gendata[this_image]->valid && gendata[this_image]->type==DATATYPE_3D
     && this_list)
    {
        if (zoomed_in)
        {
            x_offset=y_offset=LARGE_BORDER_SIZE;
            image_size=LARGE_IMAGE_SIZE;
        }
        else
        {
            x_offset=SMALL_BORDER_SIZE+
                (this_image%2)*(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE);
            y_offset=SMALL_BORDER_SIZE+
                (this_image/2)*(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE);
            
            image_size=SMALL_IMAGE_SIZE;
        }
        screenlines=im_screenlines[this_image];
        pixel_size=image_size/screenlines;
        hDC = GetDC(hDlg);
        SelectPalette(hDC, hPal, 0);
        RealizePalette(hDC);
        OldROP = SetROP2(hDC, R2_COPYPEN);
        hRgn = CreateRectRgn(x_offset, y_offset, 
                x_offset+image_size,y_offset+image_size);
        SelectClipRgn(hDC, hRgn);
        lplb.lbStyle = BS_SOLID;
        lplb.lbColor = color;
        hBrush = CreateBrushIndirect(&lplb);
        hBrushOld = SelectObject(hDC, hBrush);

        x_offset-=im_src_x[this_image]*image_size/screenlines;
        y_offset+=im_src_y[this_image]*image_size/screenlines+image_size;
        this_el=this_list;
        do
        {
            im_x1=this_el->x-size/2;
            im_y1=this_el->y-size/2;
            im_x2=this_el->x+size/2;
            im_y2=this_el->y+size/2;
            
            x = (im_x1)*image_size/screenlines; 
            y = -im_y1*image_size/screenlines+1; 
            x2 = im_x2*image_size/screenlines+pixel_size-1; 
            y2 = -im_y2*image_size/screenlines-pixel_size-1; 
            Ellipse(hDC,(int) (x+x_offset-1), (int) (y+y_offset-1),
                (int) (x2+x_offset+1), (int) (y2+y_offset+1));                
            this_el=this_el->next;
        } while(this_el!=this_list);
            
            
            
        SetROP2(hDC, OldROP);
        SelectClipRgn(hDC,NULL);
        DeleteObject(hRgn);
        SelectObject(hDC, hBrushOld);
        DeleteObject(hBrush);
    
        ReleaseDC(hDlg, hDC);
    }
        
        
}
#endif

void paint_circles(HWND hDlg,int this_image,COUNT_EL *this_list,int size,
    COLORREF color)
{
    HDC hDC;
    HRGN hRgn;
    int x_offset,y_offset,image_size,pixel_size;
    int screenlines; 
   
    if (gendata[this_image]->valid && gendata[this_image]->type==DATATYPE_3D
     && this_list)
    {
        if (zoomed_in)
        {
            x_offset=y_offset=LARGE_BORDER_SIZE;
            image_size=LARGE_IMAGE_SIZE;
        }
        else
        {
            x_offset=SMALL_BORDER_SIZE+
                (this_image%2)*(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE);
            y_offset=SMALL_BORDER_SIZE+
                (this_image/2)*(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE);
            
            image_size=SMALL_IMAGE_SIZE;
        }
        screenlines=im_screenlines[this_image];
        pixel_size=image_size/screenlines;
        hDC = GetDC(hDlg);
        hRgn = CreateRectRgn(x_offset, y_offset, 
                x_offset+image_size,y_offset+image_size);
        SelectClipRgn(hDC, hRgn);
        x_offset-=im_src_x[this_image]*image_size/screenlines;
        y_offset+=im_src_y[this_image]*image_size/screenlines+image_size;
        paint_circles_gen(hDC,x_offset,y_offset,
            (float)((float)image_size/(float)screenlines),this_list,size,color);
        
        SelectClipRgn(hDC,NULL);
        DeleteObject(hRgn);
        ReleaseDC(hDlg, hDC);
    }
        
        
}


static void countit(HWND hDlg,int this_image,int num,SEL_POINT temp_pt)
{
    int i,j,k;
    
    switch(count_mode)
    {
        case COUNT_ADD:
            if (new_count_el(&(count_data[this_image].list[num]),
                temp_pt.x,temp_pt.y))
            {
                count_data[this_image].total[num]++;
                paint_circles(hDlg,this_image,
                    count_data[this_image].list[num],count_data[this_image].size,
                    stm2colorref(count_data[this_image].color[num]));
                SendMessage(CountWnd, WM_COMMAND, COUNT_NEW_IMAGE, 0);
                
            }
            break;
        case COUNT_ERASE: 
            for(i=temp_pt.x-count_data[this_image].size/2;i<=temp_pt.x+count_data[this_image].size;i++)
                for(j=temp_pt.y-count_data[this_image].size/2;j<=temp_pt.y+count_data[this_image].size;j++)
                {
                                    
                    if (remove_count_el(&(count_data[this_image].list[num]),
                        i,j))
                    {
                        count_data[this_image].total[num]--;
                        repaint_image(hDlg,this_image,NULL);
                        SendMessage(CountWnd, WM_COMMAND, COUNT_NEW_IMAGE, 0);
                        goto COUNT_ERASE_DONE;
                    }
                }
            if (count_sloppy_erase)
            {
                for(k=0;k<COUNT_COLORS;k++)
                {
                    if (k!=num)
                    {
                        for(i=temp_pt.x-count_data[this_image].size/2;i<=temp_pt.x+count_data[this_image].size;i++)
                            for(j=temp_pt.y-count_data[this_image].size/2;j<=temp_pt.y+count_data[this_image].size;j++)
                            {
                                                
                                if (remove_count_el(&(count_data[this_image].list[k]),
                                    i,j))
                                {
                                    count_data[this_image].total[k]--;
                                    repaint_image(hDlg,this_image,NULL);
                                    SendMessage(CountWnd, WM_COMMAND, COUNT_NEW_IMAGE, 0);
                                    goto COUNT_ERASE_DONE;
                                }
                            }
                    }
                }
            }
COUNT_ERASE_DONE:
            break;
        case COUNT_CONVERT:
            for(k=0;k<COUNT_COLORS;k++)
            {
                if (k!=num)
                {
                    for(i=temp_pt.x-count_data[this_image].size/2;i<=temp_pt.x+count_data[this_image].size;i++)
                        for(j=temp_pt.y-count_data[this_image].size/2;j<=temp_pt.y+count_data[this_image].size;j++)
                        {
                                            
                            if (remove_count_el(&(count_data[this_image].list[k]),
                                i,j))
                            {
                                count_data[this_image].total[k]--;
                                if (new_count_el(&(count_data[this_image].list[num]),
                                    i,j))
                                {
                                    count_data[this_image].total[num]++;
                                }
                                repaint_image(hDlg,this_image,NULL);
                                SendMessage(CountWnd, WM_COMMAND, COUNT_NEW_IMAGE, 0);
                                goto COUNT_CONVERT_DONE;
                            }
                        }
                }
            }
                    
COUNT_CONVERT_DONE:
            break;
            
    }
}


