#include <time.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include <i86.h>
#include <string.h>
#include "file.h"
#include "data.h"
#include "dio.h"
#include "stm.h"
#include "scan.h"
#include "image.h"
#include "sel.h"
#include "grid.h"
#include "print.h"
#include "random.h"

BOOL FAR PASCAL IPrintDlg(HWND,unsigned,WPARAM,LPARAM);
int intersection_pts(float,float,float *,float *,float *,float *,int);
void selection_squares(HDC,int,int,int,int);
int coord_in_rect(float,float,float,float,float,float);

static void remove_print_el(PRINT_EL **,PRINT_EL *);
static int place_bar(PRINT_EL *,FILE *);
static void write_colorbar(HWND,FILE *,PRINT_EL *,int);
static float pof_bw(float,float); 
static float pof_grey(float,float); 
static int pof_color(int,float); 
static void write_grid(PRINT_EL *,FILE *);
static int write_grid_line(float ,float ,int ,float,FILE *);
static void place_char(char *,float,float,int,int,STM_COLOR,FILE *);
static PRINT_EL *add_image(int);
static int cur_obj_type(int);
static void print_moveto_pos();
static void moveto_this_el( PRINT_EL *this_el, float x, float y);
static float max_print_x_origin();
static float max_print_y_origin();
static void print_do_scrolls(HWND);
static void print_do_x_scroll(HWND);
static void print_do_y_scroll(HWND);
static void write_circles_list(COUNT_DATA *,int,float,FILE *);
static void repaint_arrow_line(HDC ,PRINT_EL *,int ,float,float,float,float);
static void repaint_arrow_head(HDC,PRINT_EL *,int,float,float,int);
static void repaint_print_image(HWND,HRGN,PRINT_EL *);
static void repaint_print_image2(HDC,PRINT_EL *);
static void repaint_scalemoveto( HWND hDlg);

int print_save_links_as_added=0;
int device_res=300;
int output_color=PRINT_BW;
PRINT_EL *print_list=NULL;
PRINT_EL *print_clipboard=NULL;
char print_output_filename[FILE_NAME_SIZE];
CHOOSEFONT *lpcf;
int print_text_just=PRINT_TEXT_JUST_L;
int print_arrow_head_type=PRINT_ARROW_HEAD_DEF;
float print_text_angle=0;
STM_COLOR print_text_color,print_arrow_color;
float print_arrow_width=0.01;
float print_arrow_head=0.1;
float print_pixels_per_inch=PRINT_PIXELS_PER_INCH;
float print_x_origin=0,print_y_origin=0;
LISTBOX print_arrow_heads;

static int out_smart=0;
static int first_time=1;
static float print_scale_factor=1.0;
static float print_moveto_x=1.0;
static float print_moveto_y=1.0;
static int bit_pos=3;
static int bit_sum=0;
static int chars_this_line=0;
static int cycle_select;
static int pix;
static int print_object_mode=PRINT_OBJECT_SELECT;
static int print_obj_count=0;
static PRINT_EL *cur_obj;
static float image_gamma=1;
static int captured=0;
static POINT first_pt;
static int changed=0;
#ifdef OLD
static int print_image_count=0,print_arrow_count=0,print_text_count=0,print_eps_count=0;
static PRINT_EL *cur_obj,*cur_eps,*cur_arrow,*cur_text,*cur_obj;
#endif

extern char *current_file_fig;
extern HWND ImgDlg;
extern HANDLE hInst;
extern SEL_REGION sel_r1,sel_r2;
extern int sel2_on,sel1_on;
extern BOOL SelOn;
extern float image_lowerv[],image_higherv[],image_lowert[],image_highert[];
extern int current_image;
extern datadef *gendata[];
extern char string[];
extern BITMAPINFO *bitmapinfo;
extern unsigned char *im_bitmaps[];
extern int im_bitmap_size[];
extern GRID grids[];
extern COUNT_DATA count_data[];
extern char *pal_image;
extern HPALETTE hPal;

#define ONE_BYTE_HEX(A) if ((A)>9) fputc('a'+(A)-10,fp); \
            else fputc('0'+(A),fp);

#define PIXEL_X_TO_INCH(A) (((float)((A)-PRINT_AREA_X))/(float)print_pixels_per_inch+print_x_origin)
#define PIXEL_Y_TO_INCH(A) (((float)(PRINT_AREA_Y+PRINT_AREA_HEIGHT-\
                            (A)))/(float)print_pixels_per_inch+print_y_origin)
#define INCH_X_TO_PIXEL(A) (int)(((A)-print_x_origin)*print_pixels_per_inch+PRINT_AREA_X+0.5)
#define INCH_Y_TO_PIXEL(A) (int)(PRINT_AREA_HEIGHT-((A)-print_y_origin)*print_pixels_per_inch+PRINT_AREA_Y+0.5)

#ifdef OLD
#define PIXEL_X_TO_INCH(A) ((float)((A)-PRINT_AREA_X))/(float)print_pixels_per_inch
#define PIXEL_Y_TO_INCH(A) ((float)(PRINT_AREA_Y+PRINT_AREA_HEIGHT-\
                            (A)))/(float)print_pixels_per_inch
#define INCH_X_TO_PIXEL(A) (int)((A)*print_pixels_per_inch+PRINT_AREA_X+0.5)
#define INCH_Y_TO_PIXEL(A) (int)(PRINT_AREA_HEIGHT-(A)*print_pixels_per_inch+PRINT_AREA_Y+0.5)
#endif

void postscript_preamble(FILE *);    
void postscript_postamble(FILE *);
static void repaint_image_controls(HWND);
static void repaint_bar_controls(HWND);
static void repaint_all(HWND);
void write_image(HWND, PRINT_EL *, int, FILE *, int);
void print_bit(FILE *, int);
void flush_row(FILE *);
void finish_print_bit( /* FILE * */);
void start_print_bit( /* FILE *, nt */);
static void repaint_print_area(HWND);
void repaint_print_circles(HDC, PRINT_EL *);
void clear_maybe();
void select_next(int);
void clear_selection();
void selection_square(HDC, int, int);
void count_selected();
void print_status(HWND, char *);
static void write_circles(PRINT_EL *, FILE *);
void re_link();
void delete_selected();
static void copy_selected();
static void paste_selected();
void delete_all();
void print_char(FILE *, int);
static void repaint_text_controls(HWND);
static void repaint_local_text_controls(HWND);
static void repaint_print_text(HWND, HDC, PRINT_EL *);
static void write_text(HWND, PRINT_EL *, FILE *);
static void repaint_arrow_controls(HWND);
static void repaint_xor_arrow(HWND, PRINT_EL *);
static void repaint_arrow(HDC, PRINT_EL *, int, int);
static void write_arrow(HWND, PRINT_EL *, FILE *);
static void prune();
void init_arrow_heads();
static void reset_current();
static void repaint_grids(HDC, PRINT_EL *);
void print_scale_pos();
static void repaint_image_res_controls(HWND);

BOOL FAR PASCAL IPrintDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{
    int i;
    int id;
    int delt,ddelt,mini,maxi;
    double d,minid,maxid,deltd,ddeltd;
    PRINT_EL *this_el;
    PAINTSTRUCT ps;
    POINT mouse_pos;
    int delta_x,delta_y;
    int maybe,shifted;

    switch(Message)
    {
        case WM_INITDIALOG:
            SetScrollRange(GetDlgItem(hDlg, PRINT_X_SCROLL), SB_CTL, 0,32767, TRUE);
            SetScrollRange(GetDlgItem(hDlg, PRINT_Y_SCROLL), SB_CTL, 0,32767, TRUE);
            
            if (first_time)
            {
                init_arrow_heads();
//                init_listbox(hDlg, PRINT_ARROW_TYPE, &(print_arrow_heads));
                strcpy(print_output_filename,"c:\\test.ps");
                first_time=0;
                rmarin(13,147);
                lpcf=(CHOOSEFONT *)malloc(sizeof(CHOOSEFONT));
                lpcf->lStructSize=sizeof(CHOOSEFONT);
                lpcf->hwndOwner=NULL;
                lpcf->hDC=NULL;
                lpcf->lpLogFont=(LOGFONT *)malloc(sizeof(LOGFONT));
                lpcf->lpLogFont->lfFaceName[0]='\0';
                lpcf->Flags=CF_TTONLY | CF_BOTH;
//                lpcf->Flags=CF_TTONLY | CF_NOVECTORFONTS | CF_NOOEMFONTS;
                lpcf->lpszStyle=(char *)malloc(100);
                print_text_color.r=0;
                print_text_color.g=0;
                print_text_color.b=0;
                print_arrow_color.r=0;
                print_arrow_color.g=0;
                print_arrow_color.b=0;
                print_text_angle=0;
                print_text_just=PRINT_TEXT_JUST_L;
            }
            init_listbox(hDlg, PRINT_ARROW_TYPE, &(print_arrow_heads));
            reset_current();
            re_link();
            repaint_all(hDlg);
            break;
        case WM_PAINT:
            BeginPaint(hDlg,&ps);
            if (rect_intersect_gen(PRINT_AREA_X,PRINT_AREA_Y,
                PRINT_AREA_X+PRINT_AREA_WIDTH,PRINT_AREA_Y+PRINT_AREA_HEIGHT,
                ps.rcPaint.left,ps.rcPaint.bottom,ps.rcPaint.right,
                ps.rcPaint.top))
            {
                repaint_print_area(hDlg);
            }
            EndPaint(hDlg,&ps);
        break;
    case WM_MOUSEMOVE:
        if (captured)
        {
            mouse_pos.x = LOWORD(lParam);
            mouse_pos.y = HIWORD(lParam);
/*
            mouse_pos.x-=PRINT_AREA_X;
            mouse_pos.y-=PRINT_AREA_Y;
*/
            delta_x=mouse_pos.x-first_pt.x;
            delta_y=first_pt.y-mouse_pos.y;
            first_pt.x=mouse_pos.x;
            first_pt.y=mouse_pos.y;

            switch(print_object_mode)
            {
                case PRINT_OBJECT_MOVE:
                    if (print_list!=NULL)
                    {
                        this_el=print_list;
                        do
                        {
                            if (this_el->selected)
                            {
                                switch(this_el->type)
                                {
                                    case PRINT_TYPE_IMAGE:
                                    case PRINT_TYPE_IMAGE_LINK:
                                    case PRINT_TYPE_EPS:
                                    case PRINT_TYPE_BAR:
                                        this_el->x1+=(float)delta_x/(float) print_pixels_per_inch;
                                        this_el->y1+=(float)delta_y/(float) print_pixels_per_inch;
                                        break;
                                    case PRINT_TYPE_TEXT:
                                    case PRINT_TYPE_LINE:
                                        this_el->x1+=(float)delta_x/(float) print_pixels_per_inch;
                                        this_el->y1+=(float)delta_y/(float) print_pixels_per_inch;
                                        this_el->x2+=(float)delta_x/(float) print_pixels_per_inch;
                                        this_el->y2+=(float)delta_y/(float) print_pixels_per_inch;
                                        break;
                                }
                            }       
                            this_el=this_el->next;
                        } while(this_el!=print_list);
                        repaint_all(hDlg);
                        repaint_print_area(hDlg);
                    }
                    break;
                case PRINT_ADD_ARROW:
                    repaint_xor_arrow(hDlg,cur_obj);
                    cur_obj->x2=PIXEL_X_TO_INCH(mouse_pos.x);
                    cur_obj->y2=PIXEL_Y_TO_INCH(mouse_pos.y);
                    repaint_arrow_controls(hDlg);
                    repaint_xor_arrow(hDlg,cur_obj);
                    break;
            }
            
        }
        cycle_select=0;
        clear_maybe();
        break;
    case WM_LBUTTONUP:
        if (captured)
        {    
/*
            ClipCursor(NULL);
*/
            ReleaseCapture();
            switch(print_object_mode)
            {
                case PRINT_ADD_ARROW:
                    repaint_print_area(hDlg);
//                    print_object_mode=PRINT_OBJECT_SELECT;
                    break;
                case PRINT_ADD_TEXT:
//                    print_object_mode=PRINT_OBJECT_SELECT;
                    break;
            }
            captured=0;
        }
        break;
        
    case WM_RBUTTONUP:
        if (captured)
        {    
/*
            ClipCursor(NULL);
*/
            ReleaseCapture();
            captured=0;
        }
        break;
    case WM_RBUTTONDOWN:
        mouse_pos.x = LOWORD(lParam);
        mouse_pos.y = HIWORD(lParam);
        mouse_pos.x-=PRINT_AREA_X;
        mouse_pos.y-=PRINT_AREA_Y;
        break;    
    case WM_LBUTTONDOWN:
        mouse_pos.x = LOWORD(lParam);
        mouse_pos.y = HIWORD(lParam);
/*
        mouse_pos.x-=PRINT_AREA_X;
        mouse_pos.y-=PRINT_AREA_Y;
*/
        switch(print_object_mode)
        {
            case PRINT_OBJECT_SELECT:
                shifted=(wParam & MK_SHIFT);
                maybe=maybe_select(mouse_pos);
                if (maybe>1)
                {
                    if (cycle_select)
                    {
                        select_next(shifted);
                    }
                    else 
                    {
                        if (!shifted) clear_selection();
                        select_next(shifted);
                        cycle_select=1;
                    }
                }
                else if (maybe)
                {
            
                    if (!shifted) clear_selection();
                    this_el=print_list;
                    if (this_el!=NULL) do
                    {
                        if (this_el->maybe_selected && this_el->selected)
                        {
                            this_el->maybe_selected=this_el->selected=0;
                            break;
                        }
                        if (this_el->maybe_selected)
                        {
                            this_el->maybe_selected=0;
                            this_el->selected=1;
                            break;
                        }
                        this_el=this_el->next;
                    } while(this_el!=print_list);
                }
                count_selected();
                repaint_all(hDlg);
                repaint_print_area(hDlg);
                break;
            case PRINT_OBJECT_MOVE:
                first_pt.x=mouse_pos.x;
                first_pt.y=mouse_pos.y;
                SetCapture(hDlg);
                captured=1;
                break;
            case PRINT_ADD_ARROW:
                if (pt_in_rect(PRINT_AREA_X,PRINT_AREA_Y,
                    PRINT_AREA_X+PRINT_AREA_WIDTH,
                    PRINT_AREA_Y+PRINT_AREA_HEIGHT,mouse_pos))
                {
                    first_pt.x=mouse_pos.x;
                    first_pt.y=mouse_pos.y;
                    SetCapture(hDlg);
                    captured=1;
                    clear_selection();
                    repaint_print_area(hDlg);
                    
                    this_el= new_print_el(&print_list,PRINT_TYPE_LINE);
                    this_el->selected=1;
                    cur_obj=this_el;
                    print_obj_count=1;
                    this_el->x1=PIXEL_X_TO_INCH(mouse_pos.x);
                    this_el->y1=PIXEL_Y_TO_INCH(mouse_pos.y);
                    this_el->color=print_arrow_color;
                    this_el->x2=PIXEL_X_TO_INCH(mouse_pos.x);
                    this_el->y2=PIXEL_Y_TO_INCH(mouse_pos.y);
                    this_el->arrow_width=print_arrow_width;
                    this_el->arrow_head=print_arrow_head;
                    this_el->arrow_head_type=print_arrow_head_type;
                    repaint_arrow_controls(hDlg);
                }
                break;
            case PRINT_ADD_TEXT:
                if (pt_in_rect(PRINT_AREA_X,PRINT_AREA_Y,
                    PRINT_AREA_X+PRINT_AREA_WIDTH,
                    PRINT_AREA_Y+PRINT_AREA_HEIGHT,mouse_pos))
                {
                    if (!(lpcf->lpLogFont->lfFaceName[0]))
                    {
                        if (!ChooseFont(lpcf)) break;
                    }
                    lpcf->Flags|=CF_INITTOLOGFONTSTRUCT;

                    SetFocus(GetDlgItem(hDlg, PRINT_TEXT));
                    clear_selection();
                    repaint_print_area(hDlg);
                    
                    this_el= new_print_el(&print_list,PRINT_TYPE_TEXT);
                    this_el->selected=1;
                    cur_obj=this_el;
                    print_obj_count=1;
                    this_el->x1=PIXEL_X_TO_INCH(mouse_pos.x);
                    this_el->y1=PIXEL_Y_TO_INCH(mouse_pos.y);
                    this_el->color=print_text_color;
                    this_el->print_font=*(lpcf->lpLogFont);
                    this_el->font_size=((float)lpcf->iPointSize)/10;
                    this_el->text_just=print_text_just;
                    this_el->angle=print_text_angle;
                    repaint_local_text_controls(hDlg);
                    SetDlgItemText(hDlg,PRINT_TEXT,"");
                    SetFocus(GetDlgItem(hDlg, PRINT_TEXT));
                    
                }
                
                
                break;

          case PRINT_PALETTE:
                if (pt_in_rect(PRINT_AREA_X,PRINT_AREA_Y,
                    PRINT_AREA_X+PRINT_AREA_WIDTH,
                    PRINT_AREA_Y+PRINT_AREA_HEIGHT,mouse_pos))
                {
                  if( gendata[current_image]->valid) {
                          clear_selection();
                          repaint_print_area(hDlg);

                          this_el = new_print_el( &print_list, PRINT_TYPE_BAR);
    			  copy_data(&(this_el->print_image),&(gendata[current_image]));
                          this_el->selected = 1;
                          cur_obj = this_el;
                          print_obj_count=1;
                          this_el->x1=PIXEL_X_TO_INCH(mouse_pos.x);
                          this_el->y1=PIXEL_Y_TO_INCH(mouse_pos.y);
                          this_el->sizex = PRINT_DEF_BAR_LENGTH; // 6 inches
                          this_el->sizey = PRINT_DEF_BAR_HEIGHT; // .5 inches

                          this_el->bar_pal.type = this_el->print_image->pal.type;

			  calc_print_pal_bitmap( this_el);

                          repaint_print_area( hDlg);
                          repaint_bar_controls( hDlg);
                  }
                  else {
                        MessageBox( hDlg, "No image selected in data manipulation", "Warning", MB_OK);
                  }               
                }
                break;  
        }
        
        break;    
    case WM_HSCROLL:
    case WM_VSCROLL:
        if (!out_smart)
        {
            out_smart=1;
            id = getscrollid();
            switch (id)
            {
                case PRINT_X_SCROLL:
                    d=(double) print_x_origin;
                    minid=0;
                    maxid=max_print_x_origin();
                    deltd=(maxid-minid)/100;
                    ddeltd=(maxid-minid)/10;
                    break;
                case PRINT_Y_SCROLL:
                    d=(double) print_y_origin;
                    minid=0;
                    maxid=max_print_y_origin();
                    deltd=(maxid-minid)/100;
                    ddeltd=(maxid-minid)/10;
                    break;
/*
                case VSCALE_MIN_SC:
                    d=(double) image_lowerv[current_image];
                    minid=min(gendata[current_image]->min_z,IN_ZERO);
                    maxid=image_higherv[current_image];
                    deltd=(maxid-minid)/100;
                    ddeltd=(maxid-minid)/10;
                    break;
                case VSCALE_MAX_SC:
                    d=(double) image_higherv[current_image];
                    minid=image_lowerv[current_image];
                    maxid=max(gendata[current_image]->max_z,IN_MAX);
                    deltd=(maxid-minid)/100;
                    ddeltd=(maxid-minid)/10;
                    break;
*/
    
            }
            switch (getscrollcode())
            {       
                case SB_LINELEFT:
                    i -= delt;
                    d -= deltd;
                    break;
                case SB_LINERIGHT:
                    i += delt;
                    d += deltd;
                    break;
                case SB_PAGELEFT:
                    i -= ddelt;
                    d -= ddeltd;
                    break;
                case SB_PAGERIGHT:
                    i += ddelt;
                    d += ddeltd;
                    break;
                case SB_THUMBPOSITION:
                    i = getscrollpos();
                    d = ITOD(getscrollpos(),minid,maxid);
                    break;
                case SB_THUMBTRACK:
                    i = getscrollpos();
                    d = ITOD(getscrollpos(),minid,maxid);
                    break;
            }
            i = min(max(i, mini), maxi);
//            d=floor(d+0.5);
            d = min(max(d, minid), maxid);
            if (getscrollcode()!=SB_ENDSCROLL) switch (id)
            {
                case PRINT_X_SCROLL:
                    print_x_origin=d;
                    print_do_x_scroll(hDlg);
                    repaint_print_area(hDlg);
                    break;
                case PRINT_Y_SCROLL:
                    print_y_origin=d;
                    print_do_y_scroll(hDlg);
                    repaint_print_area(hDlg);
                    break;
/*
                case VSCALE_MIN_SC:
                    image_lowerv[current_image]=d;
                    repaint_all(hDlg);
                    SendMessage(ImgDlg, WM_COMMAND, IM_NEW_VSCALE, 0);
                    break;
                    
                case VSCALE_MAX_SC:
                    image_higherv[current_image]=d;
                    repaint_all(hDlg);
                    SendMessage(ImgDlg, WM_COMMAND, IM_NEW_VSCALE, 0);
                    break;
*/
            }
            out_smart=0;
        }
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
	    case PRINT_MOVE_POS:
		print_moveto_pos();
                repaint_print_area( hDlg);
		repaint_image_controls( hDlg);
		break;
	    case PRINT_MOVE_X:
                  out_smart=1;
                  GetDlgItemText(hDlg,LOWORD(wParam),string,10);
                  print_moveto_x=atof(string);
                  out_smart=0;
		break;
	    case PRINT_MOVE_Y:
                  out_smart=1;
                  GetDlgItemText(hDlg,LOWORD(wParam),string,10);
                  print_moveto_y=atof(string);
                  out_smart=0;
		break;
            case PRINT_SCALE_POS:
                print_scale_pos();
                repaint_print_area(hDlg);
		repaint_image_controls( hDlg);
                break;
            case PRINT_SCALE_FACTOR:
                out_smart=1;
                GetDlgItemText(hDlg,LOWORD(wParam),string,10);
                print_scale_factor=atof(string);
                out_smart=0;
                break;
            case PRINT_ZOOM_IN:
                print_pixels_per_inch*=2;
                print_do_scrolls(hDlg);
                
                repaint_print_area(hDlg);
                break;
            case PRINT_ZOOM_OUT:
                print_pixels_per_inch/=2;
                print_pixels_per_inch=max(print_pixels_per_inch,
                    PRINT_PIXELS_PER_INCH);
                    
                if (print_pixels_per_inch>PRINT_PIXELS_PER_INCH-1 &&
                    print_pixels_per_inch<PRINT_PIXELS_PER_INCH+1)
                {
                    print_pixels_per_inch=PRINT_PIXELS_PER_INCH;
                }
                print_do_scrolls(hDlg);
                repaint_print_area(hDlg);
                break;
            case PRINT_SAVE_LINKS_AS_ADDED:
                print_save_links_as_added=IsDlgButtonChecked(hDlg,
                    PRINT_SAVE_LINKS_AS_ADDED);
                break;
            case PRINT_GET_LINE:
                if (cur_obj_type(PRINT_TYPE_ANY_IMAGE) && print_obj_count==1 
                    && SelOn && sel1_on)
                {
                    
                    this_el= new_print_el(&print_list,PRINT_TYPE_LINE);
                    this_el->x1=(((float)sel_r1.pt1->x+0.5)/(float)
                        gendata[current_image]->size)*cur_obj->sizex+
                            cur_obj->x1;
                    this_el->y1=(((float)sel_r1.pt1->y+0.5)/(float)
                        gendata[current_image]->size)*cur_obj->sizex+
                            cur_obj->y1;

                    this_el->color=print_arrow_color;
                    this_el->x2=(((float)sel_r1.pt2->x+0.5)/(float)
                        gendata[current_image]->size)*cur_obj->sizex+
                            cur_obj->x1;

                    this_el->y2=(((float)sel_r1.pt2->y+0.5)/(float)
                        gendata[current_image]->size)*cur_obj->sizex+
                            cur_obj->y1;

                    this_el->arrow_width=print_arrow_width;
                    this_el->arrow_head=print_arrow_head;
                    this_el->arrow_head_type=print_arrow_head_type;
                    clear_selection();
                    this_el->selected=1;
                    cur_obj=this_el;
                    print_obj_count=1;
                    repaint_arrow_controls(hDlg);
                    repaint_print_area(hDlg);
                }
                break;
                
            case PRINT_ARROW_TYPE:
                if ((cur_obj_type(PRINT_TYPE_LINE) && print_obj_count==1))
                switch (getcombomsg())
                {
                    case CBN_SELCHANGE:
                        cur_obj->arrow_head_type=print_arrow_heads.index[
                            SendDlgItemMessage(hDlg,PRINT_ARROW_TYPE,
                                CB_GETCURSEL,0,0)];
                        print_arrow_head_type=cur_obj->arrow_head_type;
                        repaint_print_area(hDlg);
                        break;
                }
                break;            
            case PRINT_TEXT_JUST_L:
            case PRINT_TEXT_JUST_C:
            case PRINT_TEXT_JUST_R:
                if ((cur_obj_type(PRINT_TYPE_TEXT) && print_obj_count==1))
                {
                    print_text_just=LOWORD(wParam);
                    cur_obj->text_just=print_text_just;
                    repaint_print_area(hDlg);
                }
                break;
            case PRINT_TEXT:
                if ((cur_obj_type(PRINT_TYPE_TEXT) && print_obj_count==1))
                {
                    GetDlgItemText(hDlg,PRINT_TEXT,cur_obj->text,COMMENTMAXSIZE-1);
                    repaint_print_area(hDlg);
                }
                break;
            case PRINT_TEXT_FONT:
//                lpcf->hDC=hDlg;
                if ((cur_obj_type(PRINT_TYPE_TEXT) && print_obj_count==1))
                {
                    *(lpcf->lpLogFont)=cur_obj->print_font;
//                    if (!(lpcf->lpLogFont->lfFaceName[0])) 
                        lpcf->Flags|=CF_INITTOLOGFONTSTRUCT;
                    ChooseFont(lpcf);
                    SetDlgItemText(hDlg,PRINT_TEXT_FONT_NAME,
                        (lpcf->lpLogFont)->lfFaceName);
                    sprintf(string,"%.3f",(float)lpcf->iPointSize/10.0);
                    SetDlgItemText(hDlg,PRINT_TEXT_SIZE,string);
                    cur_obj->print_font=*(lpcf->lpLogFont);
                    cur_obj->font_size=(float)lpcf->iPointSize/10.0;
                    SetFocus(GetDlgItem(hDlg, PRINT_TEXT));
                    
                    repaint_print_area(hDlg);
                }
                break;
#ifdef OLD
            case PRINT_ARROW_R:
                if ((cur_obj_type(PRINT_TYPE_LINE) && print_obj_count==1) && !out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,10);
                    i=atoi(string);
                    i=min(max(i,0),255);
                    if (i!=cur_obj->color.r)
                    {
                        changed=1;
                        cur_obj->color.r=i;
                    }                  
                    out_smart=0;
                }
                break;
            case PRINT_ARROW_G:
                if ((cur_obj_type(PRINT_TYPE_LINE) && print_obj_count==1) && !out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,10);
                    i=atoi(string);
                    i=min(max(i,0),255);
                    if (i!=cur_obj->color.g)
                    {
                        changed=1;
                        cur_obj->color.g=i;
                    }
                    out_smart=0;
                }
                break;
            case PRINT_ARROW_B:
                if ((cur_obj_type(PRINT_TYPE_LINE) && print_obj_count==1) && !out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,10);
                    i=atoi(string);
                    i=min(max(i,0),255);
                    if (i!=cur_obj->color.b)
                    {
                        changed=1;
                        cur_obj->color.b=i;
                    }
                    out_smart=0;
                }
                break;
            case PRINT_TEXT_R:
                if ((cur_obj_type(PRINT_TYPE_TEXT) && print_obj_count==1) && !out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,10);
                    i=atoi(string);
                    i=min(max(i,0),255);
                    if (i!=cur_obj->color.r)
                    {
                        changed=1;
                        cur_obj->color.r=i;
                    }                  
                    out_smart=0;
                }
                break;
            case PRINT_TEXT_G:
                if ((cur_obj_type(PRINT_TYPE_TEXT) && print_obj_count==1) && !out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,10);
                    i=atoi(string);
                    i=min(max(i,0),255);
                    if (i!=cur_obj->color.g)
                    {
                        changed=1;
                        cur_obj->color.g=i;
                    }
                    out_smart=0;
                }
                break;
            case PRINT_TEXT_B:
                if ((cur_obj_type(PRINT_TYPE_TEXT) && print_obj_count==1) && !out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,10);
                    i=atoi(string);
                    i=min(max(i,0),255);
                    if (i!=cur_obj->color.b)
                    {
                        changed=1;
                        cur_obj->color.b=i;
                    }
                    out_smart=0;
                }
                break;
#endif
            case PRINT_GRID_R:
                if (print_obj_count==1 && !out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,10);
                    i=atoi(string);
                    i=min(max(i,0),255);
                    if (i!=cur_obj->color.r)
                    {
                        changed=1;
                        cur_obj->color.r=i;
                    }                  
                    out_smart=0;
                }
                break;
            case PRINT_GRID_G:
                if (print_obj_count==1 && !out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,10);
                    i=atoi(string);
                    i=min(max(i,0),255);
                    if (i!=cur_obj->color.g)
                    {
                        changed=1;
                        cur_obj->color.g=i;
                    }
                    out_smart=0;
                }
                break;
            case PRINT_GRID_B:
                if (print_obj_count==1 && !out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,10);
                    i=atoi(string);
                    i=min(max(i,0),255);
                    if (i!=cur_obj->color.b)
                    {
                        changed=1;
                        cur_obj->color.b=i;
                    }
                    out_smart=0;
                }
                break;
	    case PRINT_IMAGE_RES_RAW:
	    case PRINT_IMAGE_RES_SMOOTH:
		if( cur_obj_type(PRINT_TYPE_ANY_IMAGE)) {
		  cur_obj->image_resolution_type = LOWORD(wParam);
		  repaint_image_res_controls(hDlg);
		}
		break;
            case PRINT_BW:
            case PRINT_GREY:
            case PRINT_COLOR:
                output_color=LOWORD(wParam);
                break;
            case PRINT_SAVE:
                prune();
                file_save_as(hDlg,hInst,FTYPE_FIG);
                break;    
            case PRINT_LOAD:
                delete_all();
                file_open(hDlg,hInst,FTYPE_FIG,0,current_file_fig);
                reset_current();
                re_link();
                print_pixels_per_inch=PRINT_PIXELS_PER_INCH;
                print_x_origin=print_y_origin=0;
                repaint_all(hDlg);
                repaint_print_area(hDlg);
                break;    
            
            case PRINT_DELETE_SELECTED:
                delete_selected();
                reset_current();
                repaint_all(hDlg);
                repaint_print_area(hDlg);
                break;
            case PRINT_PASTE:
                paste_selected();
                reset_current();
                repaint_all(hDlg);
                repaint_print_area(hDlg);
                break;
            case PRINT_COPY:
                copy_selected();
                break;
            case PRINT_DEVICE_GAMMA:
                if ((cur_obj_type(PRINT_TYPE_ANY_IMAGE) && print_obj_count==1) && !out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,10);
                    d=atof(string);
                    d=min(max(d,PRINT_DEVICE_GAMMA_MIN),PRINT_DEVICE_GAMMA_MAX);
                    if (d!=cur_obj->gamma)
                    {
                        changed=1;
                        cur_obj->gamma=image_gamma=d;
                    }
                    out_smart=0;
                }
                break;
            case PRINT_IMAGE_SIZE:
                if ((cur_obj_type(PRINT_TYPE_ANY_IMAGE) && print_obj_count==1) && !out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,10);
                    d=atof(string);
                    d=min(max(d,PRINT_IMAGE_SIZE_MIN),PRINT_IMAGE_SIZE_MAX);
                    if (d!=cur_obj->sizex)
                    {
                        changed=1;
                        cur_obj->sizex=d;
                    }
                    out_smart=0;
                }
                break;
            case PRINT_ARROW_WIDTH:
                if ((cur_obj_type(PRINT_TYPE_LINE) && print_obj_count==1) && !out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,10);
                    d=atof(string);
/*
                    d=min(max(d,PRINT_ARROW_X_MIN),
                        PRINT_ARROW_X_MAX);
*/ 
                    if (d!=cur_obj->arrow_width)
                    {
                        changed=1;
                        cur_obj->arrow_width=d;
                    }
                    out_smart=0;
                }
                break;
            case PRINT_ARROW_HEAD:
                if ((cur_obj_type(PRINT_TYPE_LINE) && print_obj_count==1) && !out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,10);
                    d=atof(string);
/*
                    d=min(max(d,PRINT_ARROW_X_MIN),
                        PRINT_ARROW_X_MAX);
*/ 
                    if (d!=cur_obj->arrow_head)
                    {
                        changed=1;
                        cur_obj->arrow_head=d;
                    }
                    out_smart=0;
                }
                break;
#ifdef OLD
            case PRINT_ARROW_X1:
                if ((cur_obj_type(PRINT_TYPE_LINE) && print_obj_count==1) && !out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,10);
                    d=atof(string);
/*
                    d=min(max(d,PRINT_ARROW_X_MIN),
                        PRINT_ARROW_X_MAX);
*/ 
                    if (d!=cur_obj->x1)
                    {
                        changed=1;
                        cur_obj->x1=d;
                    }
                    out_smart=0;
                }
                break;
            case PRINT_ARROW_Y1:
                if ((cur_obj_type(PRINT_TYPE_LINE) && print_obj_count==1) && !out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,10);
                    d=atof(string);
/*
                    d=min(max(d,PRINT_ARROW_Y_MIN),
                        PRINT_ARROW_Y_MAX);
*/
                    if (d!=cur_obj->x1)
                    {
                        changed=1;
                        cur_obj->y1=d;
                    }
                    out_smart=0;
                }
                break;
#endif
            case PRINT_ARROW_X2:
                if ((cur_obj_type(PRINT_TYPE_LINE) && print_obj_count==1) && !out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,10);
                    d=atof(string);
/*
                    d=min(max(d,PRINT_ARROW_X_MIN),
                        PRINT_ARROW_X_MAX);
*/ 
                    if (d!=cur_obj->x2)
                    {
                        changed=1;
                        cur_obj->x2=d;
                    }
                    out_smart=0;
                }
                break;
            case PRINT_ARROW_Y2:
                if ((cur_obj_type(PRINT_TYPE_LINE) && print_obj_count==1) && !out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,10);
                    d=atof(string);
/*
                    d=min(max(d,PRINT_ARROW_Y_MIN),
                        PRINT_ARROW_Y_MAX);
*/
                    if (d!=cur_obj->x2)
                    {
                        changed=1;
                        cur_obj->y2=d;
                    }
                    out_smart=0;
                }
                break;
#ifdef OLD
            case PRINT_TEXT_X:
                if ((cur_obj_type(PRINT_TYPE_TEXT) && print_obj_count==1) && !out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,10);
                    d=atof(string);
/*
                    d=min(max(d,PRINT_TEXT_X_MIN),
                        PRINT_TEXT_X_MAX);
*/ 
                    if (d!=cur_obj->x1)
                    {
                        changed=1;
                        cur_obj->x1=d;
                    }
                    out_smart=0;
                }
                break;
            case PRINT_TEXT_Y:
                if ((cur_obj_type(PRINT_TYPE_TEXT) && print_obj_count==1) && !out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,10);
                    d=atof(string);
/*
                    d=min(max(d,PRINT_TEXT_Y_MIN),
                        PRINT_TEXT_Y_MAX);
*/
                    if (d!=cur_obj->x1)
                    {
                        changed=1;
                        cur_obj->y1=d;
                    }
                    out_smart=0;
                }
                break;
#endif
            case PRINT_IMAGE_X:
                if (print_obj_count==1 && !out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,10);
                    d=atof(string);
/*
                    d=min(max(d,PRINT_IMAGE_X_MIN),
                        PRINT_IMAGE_X_MAX-cur_obj->sizex);
*/
                    if (d!=cur_obj->x1)
                    {
                        changed=1;
                        cur_obj->x1=d;
                    }
                    out_smart=0;
                }
                break;
            case PRINT_IMAGE_Y:
                if (print_obj_count==1 && !out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,10);
                    d=atof(string);
/*
                    d=min(max(d,PRINT_IMAGE_Y_MIN),
                        PRINT_IMAGE_Y_MAX-cur_obj->sizex);
*/
                    if (d!=cur_obj->x1)
                    {
                        changed=1;
                        cur_obj->y1=d;
                    }
                    out_smart=0;
                }
                break;
            case PRINT_DEVICE_RES:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,10);
                    i=atoi(string);
                    i=min(max(i,PRINT_DEVICE_RES_MIN),
                        PRINT_DEVICE_RES_MAX);
                    if (i!=device_res)
                    {
                        changed=1;
                        device_res=i;
                    }
                    out_smart=0;
                }
                break;
                    
            case PRINT_OBJECT_SELECT:
            case PRINT_OBJECT_MOVE:
            case PRINT_ADD_ARROW:
            case PRINT_ADD_TEXT:
            case PRINT_ADD_EPS:
            case PRINT_PALETTE:
                print_object_mode=LOWORD(wParam);
                break;
            
            case PRINT_LINK_1:
                if (gendata[0]->valid)
                {
                    this_el=new_print_el(&print_list,PRINT_TYPE_IMAGE_LINK);
                    this_el->link=0;
                    this_el->gamma=image_gamma;
                    re_link();
                    this_el->color.r=0;
                    this_el->color.g=0;
                    this_el->color.b=0;
                    repaint_print_image(hDlg,NULL,this_el);
                    
                }
                break;
            case PRINT_LINK_2:
                if (gendata[1]->valid)
                {
                    this_el=new_print_el(&print_list,PRINT_TYPE_IMAGE_LINK);
                    this_el->link=1;
                    this_el->gamma=image_gamma;
                    re_link();
                    this_el->color.r=0;
                    this_el->color.g=0;
                    this_el->color.b=0;
                    repaint_print_image(hDlg,NULL,this_el);
                    
                }
                break;
            case PRINT_LINK_3:
                if (gendata[2]->valid)
                {
                    this_el=new_print_el(&print_list,PRINT_TYPE_IMAGE_LINK);
                    this_el->link=2;
                    this_el->gamma=image_gamma;
                    re_link();
                    this_el->color.r=0;
                    this_el->color.g=0;
                    this_el->color.b=0;
                    repaint_print_image(hDlg,NULL,this_el);
                    
                }
                break;
            case PRINT_LINK_4:
                if (gendata[3]->valid)
                {
                    this_el=new_print_el(&print_list,PRINT_TYPE_IMAGE_LINK);
                    this_el->gamma=image_gamma;
                    this_el->link=3;
                    re_link();
                    this_el->color.r=0;
                    this_el->color.g=0;
                    this_el->color.b=0;
                    repaint_print_image(hDlg,NULL,this_el);
                    
                }
                break;
            case PRINT_ADD_1:
                if (gendata[0]->valid)
                {
                    this_el=add_image(0);
                    repaint_print_image(hDlg,NULL,this_el);
                    
                }
                break;
            case PRINT_ADD_2:
                if (gendata[1]->valid)
                {
                    this_el=add_image(1);
                    repaint_print_image(hDlg,NULL,this_el);
                    
                }
                break;
            case PRINT_ADD_3:
                if (gendata[2]->valid)
                {
                    this_el=add_image(2);
                    repaint_print_image(hDlg,NULL,this_el);
                    
                }
                break;
            case PRINT_ADD_4:
                if (gendata[3]->valid)
                {
                    this_el=add_image(3);
                    repaint_print_image(hDlg,NULL,this_el);
                    
                }
                break;
            case PRINT_PRINT:
    	      if (print_list!=NULL) {
                prune();
                file_save_as(hDlg,hInst,FTYPE_PS);
//                 print(hDlg);
	      }
	      else {
		MessageBox( hDlg, "No objects in layout.", "Unable to print.", MB_OK);
	      }
                break;
            case ENTER:
                repaint_all(hDlg);
                if (changed) repaint_print_area(hDlg);
                break;
                            
            case PRINT_EXIT:
//                PostMessage(ImgDlg,WM_COMMAND,IM_VSCALE_EXIT,0);
          
                EndDialog(hDlg,TRUE);
                return(TRUE);
                break;

    }
    break;
  }
  return(FALSE);
}

void print_file(char *filename,HWND hDlg)
{
    FILE *fp;
    PRINT_EL *this_el;
    int this_output_color;
    int image_number = 1;
    
    fp=fopen(filename,"wb");
    if (fp==NULL) 
    {
        mprintf("can't open output file");
        return;
    }
    postscript_preamble(fp);
    this_el=print_list;
    do
    {
        switch (this_el->type)
        {
            case PRINT_TYPE_IMAGE:
            case PRINT_TYPE_IMAGE_LINK:
                this_output_color=place_image(this_el, fp);
                write_image(hDlg,this_el,image_number++,fp,this_output_color);
                break;

            case PRINT_TYPE_BAR:
                this_output_color=place_bar( this_el, fp);
                write_colorbar( hDlg, fp, this_el, this_output_color);
                break;
        }
        this_el=this_el->next;
    } while (this_el!=print_list);
    this_el=print_list;
    do
    {
        switch (this_el->type)
        {
            case PRINT_TYPE_TEXT:
                write_text(hDlg,this_el,fp);
                break;
            case PRINT_TYPE_LINE:
                write_arrow(hDlg,this_el,fp);
                break;        
        }
        
        this_el=this_el->next;
    } while (this_el!=print_list);
    postscript_postamble(fp);
    fclose(fp);
}

static int place_bar( PRINT_EL *this_el, FILE *fp)
{
    int size_x, size_y;
    int result;
    int bits_per_sample;

    size_x = (int) ( (float)this_el->sizex*(float)device_res);
    size_y = (int) ( (float)this_el->sizey*(float)device_res);

    // save the current ps graphics state
    fprintf(fp,"gsave\n");

    // translate and scale the user coordinate system
    fprintf(fp,"%f inch %f inch translate\n",this_el->x1,this_el->y1);
    fprintf(fp,"%f inch %f inch scale\n",this_el->sizex,this_el->sizey);

    switch(output_color)
    {
        case PRINT_BW:
            result=PRINT_BW;
            bits_per_sample = 1;

            // add a line to ps file to create a string that holds one row of image data
            pix = size_x/8;
            if (size_x%8) pix++;
            fprintf(fp,"/pix %d string def\n",pix);

            // add lines for call to ps image function and its parameters
            fprintf(fp,"%d %d ", size_x, size_y);
            fprintf(fp,"%d [%d 0 0 %d 0 0] {currentfile pix readhexstring pop} image\n",
                        bits_per_sample, size_x, size_y);
            break;

        case PRINT_GREY:
            result=PRINT_GREY;
            bits_per_sample = 8;

            // add a line to ps file to create a string that holds one row of image data
            pix=size_x;
            fprintf(fp,"/pix %d string def\n",pix);

            // add lines for call to ps image function and its parameters
            fprintf(fp,"%d %d ", size_x, size_y);
            fprintf(fp,"%d [%d 0 0 %d 0 0] {currentfile pix readhexstring pop} image\n",
                        bits_per_sample, size_x, size_y);

            break;
        case PRINT_COLOR:
            if ( color_pal( this_el->bar_pal))
            {
                result=PRINT_COLOR;
                bits_per_sample = 8;

               // add a line to ps file to create a string that holds one row of image data
                pix=size_x;
                fprintf(fp,"/pix %d string def\n",pix*3);

                // add lines for call to ps colorimage function and its parameters
                fprintf(fp, "%d %d ", size_x, size_y);
                fprintf(fp,"%d [%d 0 0 %d 0 0] {currentfile pix readhexstring pop} false 3 colorimage\n",
                        bits_per_sample, size_x, size_y);
            }
            else
            {
            result=PRINT_GREY;
              bits_per_sample = 8;

              // add a line to ps file to create a string that holds one row of image data
              pix=size_x;
              fprintf(fp,"/pix %d string def\n",pix);

              // add lines for call to ps image function and its parameters
              fprintf(fp,"%d %d ", size_x, size_y);
              fprintf(fp,"%d [%d 0 0 %d 0 0] {currentfile pix readhexstring pop} image\n",
                        bits_per_sample, size_x, size_y);
            }
            break;
    }
    return (result);

}

int place_image(PRINT_EL *this_el, /* int image_number,*/ FILE *fp)
{
    int size;
    int result;
    
    size=(int) ((float)this_el->sizex*(float)device_res);
//    fprintf(fp,"save\n");
    fprintf(fp,"gsave\n");
    fprintf(fp,"%f inch %f inch translate\n",this_el->x1,this_el->y1);
    fprintf(fp,"%f inch %f inch scale\n",this_el->sizex,this_el->sizex);
    switch(output_color)
    {
        case PRINT_BW:
            result=PRINT_BW;
            pix=size/8;
            if (size%8) pix++;
            fprintf(fp,"/pix %d string def\n",pix);
            fprintf(fp,"%d %d ",size,size);
            fprintf(fp,"1 [%d 0 0 %d 0 0] {currentfile pix readhexstring pop} image\n",size,size);
            break;
        case PRINT_GREY:
            result=PRINT_GREY;
            pix=size;
            fprintf(fp,"/pix %d string def\n",pix);
            fprintf(fp,"%d %d ",size,size);
            fprintf(fp,"8 [%d 0 0 %d 0 0] {currentfile pix readhexstring pop} image\n",size,size);
            break;
        case PRINT_COLOR:
            if (color_pal(this_el->print_image->pal))
            {
                result=PRINT_COLOR;
                pix=size;
                fprintf(fp,"/pix %d string def\n",pix*3);
                fprintf(fp,"%d %d ",size,size);
                fprintf(fp,"8 [%d 0 0 %d 0 0] {currentfile pix readhexstring pop} false 3 colorimage\n",size,size);
            }
            else
            {
                result=PRINT_GREY;
                pix=size;
                fprintf(fp,"/pix %d string def\n",pix);
                fprintf(fp,"%d %d ",size,size);
                fprintf(fp,"8 [%d 0 0 %d 0 0] {currentfile pix readhexstring pop} image\n",size,size);
            }
            break;
    }
    return (result);
}

void postscript_preamble(FILE *fp)    
{
    char buf[1000];
    FILE *pre_fp;
    
    pre_fp=fopen(POSTSCRIPT_PREAMBLE,"rb");
    while(fgets(buf,1000,pre_fp))
    {
        fprintf(fp,"%s",buf);
    }
    fclose(pre_fp);
    fprintf(fp,"\n");
    POSTSCRIPT_INCH_SCALE;
    fprintf(fp,"/inch {%d mul} def\n",device_res);
}    

void postscript_postamble(FILE *fp)
{
    fprintf(fp,"showpage\n");
    fprintf(fp,"%%%% End\n");
}

static void repaint_image_controls(HWND hDlg)
{
    if ((cur_obj_type(PRINT_TYPE_ANY_IMAGE) && print_obj_count==1))
    {
        out_smart=1;
        switch(cur_obj->type)
        {
            case PRINT_TYPE_IMAGE:
                strcpy(string,"Image");
                break;
            case PRINT_TYPE_IMAGE_LINK:
                strcpy(string,"Image Link");
                break;
        }
        SetDlgItemText(hDlg,PRINT_OBJECT,string);
        SetDlgItemText(hDlg,PRINT_OBJECT_COLOR,"Grid");
        image_gamma=cur_obj->gamma;
        sprintf(string,"%0.2f",image_gamma);
        SetDlgItemText(hDlg,PRINT_DEVICE_GAMMA,string);
        sprintf(string,"%0.2f",cur_obj->sizex);
        SetDlgItemText(hDlg, PRINT_IMAGE_SIZE, string);
        sprintf(string,"%0.2f",cur_obj->x1);
        SetDlgItemText(hDlg, PRINT_IMAGE_X, string);
        sprintf(string,"%0.2f",cur_obj->y1);
        SetDlgItemText(hDlg, PRINT_IMAGE_Y, string);
        if (cur_obj->type==PRINT_TYPE_IMAGE_LINK)
            sprintf(string,"Link %d",cur_obj->link+1);
        else sprintf(string,"%s",cur_obj->print_image->filename);
        SetDlgItemText(hDlg, PRINT_IMAGE_FILENAME, string);
        sprintf(string,"%d",cur_obj->color.r);
        SetDlgItemText(hDlg, PRINT_GRID_R, string);
        sprintf(string,"%d",cur_obj->color.g);
        SetDlgItemText(hDlg, PRINT_GRID_G, string);
        sprintf(string,"%d",cur_obj->color.b);
        SetDlgItemText(hDlg, PRINT_GRID_B, string);
        
        out_smart=0;
    }
        
}

static void repaint_device_options(HWND hDlg)
{
    out_smart=1;
    CheckDlgButton(hDlg,PRINT_BW,0);
    CheckDlgButton(hDlg,PRINT_GREY,0);
    CheckDlgButton(hDlg,PRINT_COLOR,0);
    CheckDlgButton(hDlg,output_color,1);
    sprintf(string,"%d",device_res);
    SetDlgItemText(hDlg, PRINT_DEVICE_RES, string);
    SetDlgItemText(hDlg,PRINT_DEVICE_FILENAME,print_output_filename);
    
    out_smart=0;
}    

static void repaint_bar_controls( HWND hDlg)
{
    if( (cur_obj_type( PRINT_TYPE_BAR) && print_obj_count==1))
    {
        out_smart=1;
        SetDlgItemText(hDlg,PRINT_OBJECT,"Color Bar");
        SetDlgItemText(hDlg,PRINT_OBJECT_COLOR,"");
        sprintf(string,"%0.2f",cur_obj->x1);
        SetDlgItemText(hDlg, PRINT_IMAGE_X, string);
        sprintf(string,"%0.2f",cur_obj->y1);
        SetDlgItemText(hDlg, PRINT_IMAGE_Y, string);
        SetDlgItemText(hDlg, PRINT_ARROW_X2, "");
        SetDlgItemText(hDlg, PRINT_ARROW_Y2, "");
        out_smart=0;
    }
}

static void repaint_scalemoveto( HWND hDlg)
{
    sprintf(string,"%.3f",print_scale_factor);
    SetDlgItemText(hDlg,PRINT_SCALE_FACTOR,string);
    sprintf(string,"%.3f",print_moveto_x);
    SetDlgItemText(hDlg,PRINT_MOVE_X,string);
    sprintf(string,"%.3f",print_moveto_y);
    SetDlgItemText(hDlg,PRINT_MOVE_Y,string);
}

static void repaint_all(HWND hDlg)
{
    CheckDlgButton(hDlg,PRINT_OBJECT_SELECT,0);
    CheckDlgButton(hDlg,PRINT_OBJECT_MOVE,0);
    CheckDlgButton(hDlg,PRINT_ADD_ARROW,0);
    CheckDlgButton(hDlg,PRINT_PALETTE,0);
    CheckDlgButton(hDlg,PRINT_ADD_TEXT,0);
    CheckDlgButton(hDlg,PRINT_ADD_EPS,0);
    CheckDlgButton(hDlg,print_object_mode,1);
    CheckDlgButton(hDlg,PRINT_SAVE_LINKS_AS_ADDED,print_save_links_as_added);
    repaint_image_res_controls( hDlg);
    repaint_scalemoveto( hDlg);

    repaint_device_options(hDlg);
    if (print_obj_count==1)
    {
        repaint_image_controls(hDlg);
        repaint_arrow_controls(hDlg);
        repaint_text_controls(hDlg);
        repaint_bar_controls(hDlg);
    }
    else
    {
        if (print_obj_count) 
            SetDlgItemText(hDlg,PRINT_OBJECT,"Multiple Selection");
        else SetDlgItemText(hDlg,PRINT_OBJECT,"None");
        SetDlgItemText(hDlg,PRINT_OBJECT_COLOR,"");
    }
    print_do_scrolls(hDlg);
    
}

void write_image(HWND hDlg,PRINT_EL *this_image,int image_number,FILE *fp, int this_output_color)
{

#define FS_SCALE 1024
#define HALF_FS_SCALE 512

    bit* bitrow;
    register bit* bP;
    int rows, cols, row, col, limitcol;
    float fthreshval;
    int threshval, sum;
    int* thiserr;
    int* nexterr;
    int* temperr;
    int fs_direction;
    int i;
    int color32;
    char *color;

    fthreshval = 0.5;


    rows=cols=(int) ((float)this_image->sizex*(float)device_res);
    switch(this_output_color)
    {
        case PRINT_BW:
            start_print_bit( /* fp,image_number */);
            bitrow = (bit *)malloc(sizeof(bit)* cols );
        
            /* Initialize Floyd-Steinberg error vectors. */
            thiserr = (int*) malloc( (cols + 2)*sizeof(int) );
            nexterr = (int*) malloc( (cols + 2)*sizeof(int) );
            for ( col = 0; col < cols + 2; ++col )
            {
                thiserr[col] = ( mohrand()*(float)FS_SCALE - HALF_FS_SCALE ) / 4;
                if (thiserr[col]<-FS_SCALE/8 || thiserr[col]>FS_SCALE/8)
                {
                    sprintf(string,"bad err: %d",thiserr[col]);
                    mprintf(string);
                    break;
                }
            }
            /* (random errors in [-FS_SCALE/8 .. FS_SCALE/8]) */
            fs_direction = 1;
            threshval = HALF_FS_SCALE;
        
            for ( row = 0; row < rows; ++row )
            {
                for ( col = 0; col < cols + 2; ++col )
                    nexterr[col] = 0;
                if ( fs_direction )
                {
                    col = 0;
                    limitcol = cols;
                    bP = bitrow;
                }
                else
                {
                    col = cols - 1;
                    limitcol = -1;
                    bP = &(bitrow[col]);
                }
                do
                {
/*    
                sum = ((float)row/(float)rows*(float)FS_SCALE);
                sum = floor(linear_data(gendata[0],
                    (float)col/(float)cols,
                    (float)row/(float) rows) *  (float)FS_SCALE ); 
*/    
                    sum = (int) (pof_bw(linear_data(this_image->print_image,
                        (float)col/(float)cols,
                        (float)row/(float) rows),this_image->gamma) 
                        *  (float)FS_SCALE +0.5)
                        +thiserr[col + 1];
                    if ( sum >= threshval )
                    {
                        *bP = PBM_WHITE;
                        sum = sum - threshval - HALF_FS_SCALE;
                    }
                    else
                        *bP = PBM_BLACK;
                    
                    if ( fs_direction )
                    {
                        thiserr[col + 2] += ( sum * 7 ) / 16;
                        nexterr[col    ] += ( sum * 3 ) / 16;
                        nexterr[col + 1] += ( sum * 5 ) / 16;
                        nexterr[col + 2] += ( sum     ) / 16;
        
                        ++col;
                        ++bP;
                    }
                    else
                    {
                        thiserr[col    ] += ( sum * 7 ) / 16;
                        nexterr[col + 2] += ( sum * 3 ) / 16;
                        nexterr[col + 1] += ( sum * 5 ) / 16;
                        nexterr[col    ] += ( sum     ) / 16;
                
                        --col;
                        --bP;
                    }
                }
                while ( col != limitcol );
                temperr = thiserr;
                thiserr = nexterr;
                nexterr = temperr;
                fs_direction = ! fs_direction;
        
                for(i=0;i<cols;i++) print_bit(fp,bitrow[i]);
                flush_row(fp);
                sprintf(string,"image %d, printed row %d/%d",image_number,row+1,rows);
                print_status(hDlg,string);
            }
            finish_print_bit();
            free(bitrow);
            break;
        case PRINT_GREY:
            chars_this_line=0;
            for ( row = 0; row < rows; ++row )
            {
                for(col=0;col<cols;col++)
                {
                    if( this_image->image_resolution_type == PRINT_IMAGE_RES_SMOOTH) {
                    	sum = (int) (pof_grey(linear_data(this_image->print_image,
                        	(float)col/(float)cols,
                        	(float)row/(float) rows),this_image->gamma) 
                        	*  (float)255);
		    }
		    else {
                    	sum = (int) (pof_grey(raw_linear_data(this_image->print_image,
                        	(float)col/(float)cols,
                        	(float)row/(float) rows),this_image->gamma) 
                        	*  (float)255);
		    }
                    print_char(fp,sum);
                }
                sprintf(string,"image %d, printed row %d/%d",image_number,row+1,rows);
                print_status(hDlg,string);
            }
            break;
        case PRINT_COLOR:
            chars_this_line=0;
            color = (char * )&color32;
            for ( row = 0; row < rows; ++row )
            {
                for(col=0;col<cols;col++)
                {
                    if( this_image->image_resolution_type == PRINT_IMAGE_RES_SMOOTH) {
		        color32 = pof_color(linear_data3(this_image->print_image,
                            (float)col/(float)cols,
                            (float)row/(float) rows),this_image->gamma);
		    }
		    else {
		        color32 = pof_color(raw_linear_data3(this_image->print_image,
                            (float)col/(float)cols,
                            (float)row/(float) rows),this_image->gamma);
		    }
					print_char(fp,color[2]);
                    print_char(fp,color[1]);
                    print_char(fp,color[0]);
                }
                sprintf(string,"image %d, printed row %d/%d",image_number,row+1,rows);
                print_status(hDlg,string);
            }
            break;
    }    
    fprintf(fp,"\ngrestore\n");
    write_grid(this_image,fp);
    write_circles(this_image,fp);    

}

void print_bit(FILE *fp,int this_bit)
{
    bit_sum+=(this_bit<<bit_pos);
    bit_pos--;
    if (bit_pos<0)
    {
        bit_pos=3;
        ONE_BYTE_HEX(bit_sum);
        chars_this_line++;
        if (chars_this_line==CHARS_PER_LINE)
        {
            fputc('\n',fp);
            chars_this_line=0;
        }
        bit_sum=0;
    }
}

void flush_row(FILE *fp)
{
        if (bit_pos!=3)
        {
            ONE_BYTE_HEX(bit_sum);
            chars_this_line++;
        }
        if (chars_this_line%2) fprintf(fp,"0\n");
        else fprintf(fp,"\n");
        bit_pos=3;
        chars_this_line=0;
        bit_sum=0;
}
    
void finish_print_bit( /* FILE *fp */)
{
//    fprintf(fp,"grestore\n");
//    fprintf(fp,"restore\n");
}    

void start_print_bit( /* FILE *fp,int num */)
{
    chars_this_line=0;
}

static void repaint_print_area(HWND hDlg)
{
    HDC hDC;
    HPALETTE hPal_Old;
    LOGBRUSH      lplb;
    HBRUSH        hBrush,hBrushOld;
    HPEN          hPen,hPenOld;
    HRGN          hRgn;
    int x,y,x2,y2;
    PRINT_EL *this_el;
    int palx, paly, pal_screenheight, pal_screenlength;
        
    hDC = GetDC(hDlg);
    
    lplb.lbStyle = BS_HOLLOW;
    hBrush = CreateBrushIndirect(&lplb);
    hBrushOld = SelectObject(hDC,hBrush);
    x=PRINT_AREA_X;
    y=PRINT_AREA_Y;
    x2=x+PRINT_AREA_WIDTH;
    y2=y+PRINT_AREA_HEIGHT;  
    clear_area(hDlg,x,y,x2,y2,RGB(255,255,255));
    hPen = CreatePen(PS_SOLID,1,RGB(0,0,0));
    hPenOld = SelectObject(hDC,hPen);
    SelectPalette(hDC,hPal,0);
    RealizePalette(hDC);
    SetMapMode(hDC,MM_TEXT);
    Rectangle(hDC,x-1,y-1,x2+1,y2+1);
    SelectObject(hDC,hBrushOld);
    SelectObject(hDC,hPenOld);
    DeleteObject(hBrush);
    DeleteObject(hPen);
    
    hRgn = CreateRectRgn(x, y, 
            x2,y2);
    SelectClipRgn(hDC, hRgn);
    
    this_el=print_list;
    if (this_el!=NULL) do
    {
        switch(this_el->type)
        {
            case PRINT_TYPE_IMAGE:
            case PRINT_TYPE_IMAGE_LINK:
/*
                if (coord_in_rect(this_el->x1,this_el->y1,
                    print_x_origin,print_y_origin,
                    print_x_origin+PRINT_AREA_WIDTH*print_pixels_per_inch,
                    print_y_origin+PRINT_AREA_HEIGHT*print_pixels_per_inch) ||
                    coord_in_rect(this_el->x1+this_el->sizex,
                    this_el->y1+this_el->sizex,
                    print_x_origin,print_y_origin,
                    print_x_origin+PRINT_AREA_WIDTH*print_pixels_per_inch,
                    print_y_origin+PRINT_AREA_HEIGHT*print_pixels_per_inch))
*/
                if (rect_intersect_gen(this_el->x1,this_el->y1,
                    this_el->x1+this_el->sizex,
                    this_el->y1+this_el->sizex,
                    print_x_origin,print_y_origin,
                    print_x_origin+PRINT_AREA_WIDTH*print_pixels_per_inch,
                    print_y_origin+PRINT_AREA_HEIGHT*print_pixels_per_inch))
                {
                    repaint_print_image2(hDC,this_el);
                }
                break;

            case PRINT_TYPE_BAR:

                pal_screenlength = this_el->sizex*print_pixels_per_inch;
                pal_screenheight = this_el->sizey*print_pixels_per_inch;
                palx=INCH_X_TO_PIXEL(this_el->x1);
                paly=INCH_Y_TO_PIXEL(this_el->y1)-pal_screenheight;

                switch( this_el->bar_pal.type) {
                    case PALTYPE_MAPPED:
                      spindac( 0, 1, this_el->bar_pal.dacbox);
                      hPal_Old = SelectPalette( hDC, hPal, 0);
                      RealizePalette( hDC);
                
                      bitmapinfo->bmiHeader.biBitCount = 8;
                      bitmapinfo->bmiHeader.biWidth = PAL_IM_WIDTH;
                      bitmapinfo->bmiHeader.biHeight = PAL_IM_HEIGHT;
                      StretchDIBits( hDC, palx, paly, pal_screenlength, pal_screenheight,
                                0, 0, PAL_IM_WIDTH, PAL_IM_HEIGHT, this_el->print_bitmap,
                                bitmapinfo,  DIB_PAL_COLORS, SRCCOPY);
                      SelectPalette( hDC, hPal_Old, 0);
                      RealizePalette( hDC);
                      break;

                    case PALTYPE_LOGICAL:
                      bitmapinfo->bmiHeader.biBitCount = 24;
                      bitmapinfo->bmiHeader.biWidth = PAL_IM_WIDTH;
                      bitmapinfo->bmiHeader.biHeight = PAL_IM_HEIGHT;
                      StretchDIBits( hDC, palx, paly, pal_screenlength, pal_screenheight,
                                0, 0, PAL_IM_WIDTH, PAL_IM_HEIGHT, this_el->print_bitmap,
                                bitmapinfo,  DIB_RGB_COLORS, SRCCOPY);
                      break;
                }

                if( this_el->selected) {
                  selection_squares(hDC, palx, paly, palx+pal_screenlength, paly+pal_screenheight);
                }
                break;

        }
        this_el=this_el->next;
    } while (this_el!=print_list);
    this_el=print_list;
    if (this_el!=NULL) do
    {
        switch(this_el->type)
        {
            case PRINT_TYPE_TEXT:
                repaint_print_text(hDlg,hDC,this_el);
                break;
            case PRINT_TYPE_LINE:
                repaint_arrow(hDC,this_el,R2_COPYPEN,1);
                break;
                
        }
        this_el=this_el->next;
    } while (this_el!=print_list);
  
    SelectClipRgn(hDC,NULL);
    
    DeleteObject(hRgn);
    SelectObject(hDC,hBrushOld);
    SelectObject(hDC,hPenOld);
    DeleteObject(hBrush);
    DeleteObject(hPen);
    ReleaseDC(hDlg,hDC);
  
}    

static void repaint_print_image(HWND hDlg,HRGN hRgn,PRINT_EL *this_el)
{
    HDC             hDC;
    int x,y,x2,y2;
    
    hDC = GetDC(hDlg);
    
    x=PRINT_AREA_X;
    y=PRINT_AREA_Y;
    x2=x+PRINT_AREA_WIDTH;
    y2=y+PRINT_AREA_HEIGHT;  
    SelectPalette(hDC,hPal,0);
    RealizePalette(hDC);
    SetMapMode(hDC,MM_TEXT);
    hRgn = CreateRectRgn(x, y, 
            x2,y2);
    SelectClipRgn(hDC, hRgn);
    
    
    repaint_print_image2(hDC,this_el);
        
    SelectClipRgn(hDC,NULL);
    ReleaseDC(hDlg,hDC);
}    

static void repaint_print_image2(HDC hDC,PRINT_EL *this_el)
{
    int xpos,ypos,screen_size;
    
    screen_size=this_el->sizex*print_pixels_per_inch;
    xpos=INCH_X_TO_PIXEL(this_el->x1);
    ypos=INCH_Y_TO_PIXEL(this_el->y1)-screen_size;
    
    SelectPalette(hDC,hPal,0);
    RealizePalette(hDC);
    SetMapMode(hDC,MM_TEXT);
/*
    SelectClipRgn(hDC, hRgn);
    SelectClipRgn(hDC,NULL);
*/

    if (this_el->print_image->valid && this_el->print_image->type==DATATYPE_3D)
    {
        bitmapinfo->bmiHeader.biWidth=*this_el->print_bitmap_size;
        bitmapinfo->bmiHeader.biHeight=*this_el->print_bitmap_size;
        switch(this_el->print_image->pal.type)
        {
            case PALTYPE_MAPPED:
                spindac(0,1,this_el->print_image->pal.dacbox);
                bitmapinfo->bmiHeader.biBitCount = 8;

                StretchDIBits(hDC, xpos, ypos, screen_size, screen_size, 
                    0, 0, this_el->print_image->size,
                    this_el->print_image->size,
                    this_el->print_bitmap, bitmapinfo, DIB_PAL_COLORS,SRCCOPY);

/*		// old way uses bitmap size... but bitmap size is limited to powers of two
                StretchDIBits(hDC, xpos, ypos, screen_size, screen_size, 
                    0, 0, *this_el->print_bitmap_size,
                    *this_el->print_bitmap_size,
                    this_el->print_bitmap, bitmapinfo, DIB_PAL_COLORS,SRCCOPY);
*/
                break;
            case PALTYPE_LOGICAL:
                bitmapinfo->bmiHeader.biBitCount = 24;
                StretchDIBits(hDC, xpos, ypos, screen_size, screen_size, 
                    0, 0, this_el->print_image->size,
                    this_el->print_image->size,
                    this_el->print_bitmap, bitmapinfo, DIB_RGB_COLORS,SRCCOPY);

/*		// old way uses bitmap size... but bitmap size is limited to powers of two
                StretchDIBits(hDC, xpos, ypos, screen_size, screen_size, 
                    0, 0, *this_el->print_bitmap_size,
                    *this_el->print_bitmap_size,
                    this_el->print_bitmap, bitmapinfo, DIB_RGB_COLORS,SRCCOPY);
*/
                break;
        }
    }
/*
    else
        clear_area(hDlg,xpos,ypos,xpos+screen_size,ypos+screen_size,RGB(0,0,0));
*/

    repaint_grids(hDC,this_el);
    repaint_print_circles(hDC,this_el);    
    
    if (this_el->selected)
    {
        selection_squares(hDC,xpos,ypos,xpos+screen_size,ypos+screen_size);
    }
        
  
}    

void repaint_print_circles(HDC hDC,PRINT_EL *this_el)
{
    int i;
    int x_offset,y_offset;
    float pixel_size;
    
    x_offset=INCH_X_TO_PIXEL(this_el->x1);
    y_offset=INCH_Y_TO_PIXEL(this_el->y1);
    
    pixel_size=this_el->sizex*print_pixels_per_inch/
        (float)this_el->print_image->size;
    
    for(i=0;i<COUNT_COLORS;i++)
    {
        if (this_el->count_data->list[i])
            paint_circles_gen(hDC,x_offset,y_offset,pixel_size,
                this_el->count_data->list[i],this_el->count_data->size,
                stm2colorref(this_el->count_data->color[i]));
    }
}

void clear_maybe()
{
    PRINT_EL *this_el;
    
    if (print_list!=NULL)
    {
        this_el=print_list;
        do
        {
            this_el->maybe_selected=0;
            this_el=this_el->next;
        } while(this_el!=print_list);
    }
}

void select_next(int multi_select)
{
    
    PRINT_EL *last_el,*this_el;

    if (print_list!=NULL)
    {
        this_el=last_el=print_list;
        do
        {
            if (this_el->selected) last_el=this_el;
            this_el=this_el->next;
        } while(this_el!=print_list);
        
        if (last_el->selected)
        {
            this_el=last_el->next;
            while(this_el!=last_el)
            {
                if (this_el->maybe_selected)
                {
                    this_el->selected=1;
                    last_el->selected=multi_select;
                }
                this_el=this_el->next;
            }
            
        }
        else /* nothing selected yet */
        {
            last_el=print_list;
            do
            {
                if (last_el->maybe_selected) 
                {
                    last_el->selected=1;
                    break;
                }
                last_el=last_el->next;
            } while(last_el!=print_list);
        }
            
    }
}

void clear_selection()
{
    PRINT_EL *this_el;
    
    if (print_list!=NULL)
    {
        this_el=print_list;
        do
        {
            this_el->selected=0;
            this_el=this_el->next;
        } while(this_el!=print_list);
    }
}
        

int maybe_select(POINT pt)
{
    int result=0;
    PRINT_EL *this_el;
    int x1,y1,x2,y2;
    
    if (print_list!=NULL)
    {
        this_el=print_list;
        do
        {
            switch(this_el->type)
            {
                case PRINT_TYPE_IMAGE:
                case PRINT_TYPE_IMAGE_LINK:
                case PRINT_TYPE_BAR:
                    if (pt_in_image(this_el,pt))
                    {
                        result++;
                        this_el->maybe_selected=1;
                    }
                    break;
                case PRINT_TYPE_TEXT:
                    switch(this_el->text_just)
                    {
                        case PRINT_TEXT_JUST_L:
                            x1=INCH_X_TO_PIXEL(this_el->x1);
                            y1=INCH_Y_TO_PIXEL(this_el->y1);
                            x2=INCH_X_TO_PIXEL(this_el->x1+this_el->sizex);
                            y2=INCH_Y_TO_PIXEL(this_el->y1+this_el->sizey);
                            break;
                        case PRINT_TEXT_JUST_C:
                            x1=INCH_X_TO_PIXEL(this_el->x1-this_el->sizex/2);
                            y1=INCH_Y_TO_PIXEL(this_el->y1);
                            x2=INCH_X_TO_PIXEL(this_el->x1+this_el->sizex/2);
                            y2=INCH_Y_TO_PIXEL(this_el->y1+this_el->sizey);
                            break;
                        case PRINT_TEXT_JUST_R:
                            x1=INCH_X_TO_PIXEL(this_el->x1-this_el->sizex);
                            y1=INCH_Y_TO_PIXEL(this_el->y1);
                            x2=INCH_X_TO_PIXEL(this_el->x1);
                            y2=INCH_Y_TO_PIXEL(this_el->y1+this_el->sizey);
                            break;
                    }
                            
                    if (pt_in_rect(x1,y1,x2,y2,pt))
                    {
                        result++;
                        this_el->maybe_selected=1;
                    }
                    break;
                case PRINT_TYPE_LINE:
                    if (pt_in_rect(INCH_X_TO_PIXEL(this_el->x1),
                        INCH_Y_TO_PIXEL(this_el->y1),
                        INCH_X_TO_PIXEL(this_el->x2),
                        INCH_Y_TO_PIXEL(this_el->y2),
                        pt))
                    {
                        result++;
                        this_el->maybe_selected=1;
                    }
                    break;
            }
            this_el=this_el->next;
        } while(this_el!=print_list);
    }
    
    return result;
}
            
int pt_in_image(PRINT_EL *this_el,POINT pt)
{
    int xpos,ypos,screen_size,screen_y_size;   

    switch( this_el->type) {

      case PRINT_TYPE_IMAGE:
      case PRINT_TYPE_IMAGE_LINK:
        screen_size=this_el->sizex*print_pixels_per_inch;
        xpos=INCH_X_TO_PIXEL(this_el->x1);
        ypos=INCH_Y_TO_PIXEL(this_el->y1)-screen_size;
        if (pt.x>=xpos && pt.x<=xpos+screen_size && pt.y>=ypos &&
          pt.y<=ypos+screen_size) return 1;
        break;

      case PRINT_TYPE_BAR:
        screen_size=this_el->sizex*print_pixels_per_inch;
        screen_y_size=this_el->sizey*print_pixels_per_inch;
        xpos=INCH_X_TO_PIXEL(this_el->x1);
        ypos=INCH_Y_TO_PIXEL(this_el->y1)-screen_y_size;
        if (pt.x>=xpos && pt.x<=xpos+screen_size && pt.y>=ypos &&
          pt.y<=ypos+screen_y_size) return 1;
        break;
    }

    return 0;
}

int pt_in_rect(int x1,int y1,int x2,int y2,POINT pt)
{
    int tmp;
    
    if (x2<x1)
    {
        tmp=x2;
        x2=x1;
        x1=tmp;
    }
    if (y2<y1)
    {
        tmp=y2;
        y2=y1;
        y1=tmp;
    }

    if (pt.x>=x1 && pt.x<=x2 && pt.y>=y1 &&
        pt.y<=y2) return 1;

    return 0;
}

int coord_in_rect(float x,float y,float x1,float y1,float x2,float y2)
{
    float tmp;
    
    if (x2<x1)
    {
        tmp=x2;
        x2=x1;
        x1=tmp;
    }
    if (y2<y1)
    {
        tmp=y2;
        y2=y1;
        y1=tmp;
    }

    if (x>=x1 && x<=x2 && y>=y1 &&
        y<=y2) return 1;

    return 0;
}
    
void selection_squares(HDC hDC,int x1,int y1,int x2,int y2)
{
    int OldROP;
        
    SelectPalette(hDC, hPal, 0);
    RealizePalette(hDC);
    OldROP = SetROP2(hDC, R2_NOTXORPEN);
//    MoveTo(hDC, (int) (x1) , (int) (y1));
    MoveToEx(hDC, (int) (x1) , (int) (y1), NULL);
            
    LineTo(hDC, (int) x2, (int) y1);
    LineTo(hDC, (int) x2, (int) y2);
    LineTo(hDC, (int) x1, (int) y2);
    LineTo(hDC, (int) x1, (int) y1);
    SetROP2(hDC, OldROP);
/*
            SelectClipRgn(hDC,NULL);
            DeleteObject(hRgn);
*/
        
    selection_square(hDC,x1,y1);
    selection_square(hDC,x2,y1);
    selection_square(hDC,x1,y2);
    selection_square(hDC,x2,y2);
}

void selection_square(HDC hDC,int x,int y)
{
    
    int OldROP;
   
    SelectPalette(hDC, hPal, 0);
    RealizePalette(hDC);
    OldROP = SetROP2(hDC, R2_NOTXORPEN);
/*
            hRgn = CreateRectRgn(x_offset, y_offset, 
                    x_offset+image_size,y_offset+image_size);
            SelectClipRgn(hDC, hRgn);
*/
    x-=PRINT_SELECTION_SQUARE/2;
    y-=PRINT_SELECTION_SQUARE/2;
    PatBlt(hDC,(int) x,(int) y,
                (int) PRINT_SELECTION_SQUARE,  (int) PRINT_SELECTION_SQUARE,
                DSTINVERT);
                
    SetROP2(hDC, OldROP);
    
}

void count_selected()
{
    PRINT_EL *this_el;
    
    if (print_list!=NULL)
    {
        this_el=print_list;
        print_obj_count=0;
#ifdef OLD
        print_image_count=print_arrow_count=print_text_count=print_eps_count=0;
#endif
        do
        {
            if (this_el->selected) 
            {
                cur_obj=this_el;
                print_obj_count++;
            }
#ifdef OLD
            switch(this_el->type)
            {
                case PRINT_TYPE_IMAGE:
                case PRINT_TYPE_IMAGE_LINK:
                    cur_obj=this_el;
                    print_image_count++;
                    break;
                case PRINT_TYPE_LINE:
                    cur_obj=this_el;
                    print_arrow_count++;
                    break;
                case PRINT_TYPE_TEXT:
                    cur_obj=this_el;
                    print_text_count++;
                    break;
                case PRINT_TYPE_EPS:
                    cur_obj=this_el;
                    print_eps_count++;
                    break;
            }
#endif
            this_el=this_el->next;
        } while(this_el!=print_list);
    }
}
          
void print_status(HWND hDlg,char *mes)
{
    SetDlgItemText(hDlg,PRINT_STATUS,mes);
    CheckMsgQ();
}

static float pof_bw(float z,float gamma)
{
    return(pow(z,gamma));
}
static float pof_grey(float z,float gamma)
{
    return(pow(z,gamma));
}
static int pof_color(int color32,float gamma)
{
    char *color,*res_color;
    int res_color32;
    
    color = & color32;
    res_color= & res_color32;
    
    res_color[0]=pow((float)color[0]/(float)255,gamma)*255;
    res_color[1]=pow((float)color[1]/(float)255,gamma)*255;
    res_color[2]=pow((float)color[2]/(float)255,gamma)*255;
    
    return(res_color32);
}
int line_side(GRID this_grid,int x,int y) /* return 1 if (x,y) is above line */
{
    float slope,intercept,line_y;
    
    if (this_grid.x[0]==this_grid.x[1])
    {
        if (x<this_grid.x[0]) return 0;
        else return 1;
    }
    else
    {
        slope=((float)(this_grid.y[1]-this_grid.y[0]))/
            (float)(this_grid.x[1]-this_grid.x[0]);
        intercept=((float)this_grid.y[1])-((float)(this_grid.y[1]-this_grid.y[0]))/
            (float)(this_grid.x[1]-this_grid.x[0])*((float)this_grid.x[1]);
        line_y=slope*(float)x+intercept;
    }
    return(line_y>y);        
}

static void write_circles(PRINT_EL *this_el,FILE *fp)
{
    int i;
    float pixel_size;
    
    pixel_size=this_el->sizex/(float)this_el->print_image->size;
     
    
    fprintf(fp,"gsave\n");
    fprintf(fp,"%f inch %f inch translate\n",this_el->x1,this_el->y1);
    fprintf(fp,"/circrad %f def\n",pixel_size/(float)2*
        (float)(this_el->count_data->size+1));
    
    for(i=0;i<COUNT_COLORS;i++)
    {
        if (this_el->count_data->list[i])
        {
            write_circles_list(this_el->count_data,i,pixel_size,fp);
        }
    }


    fprintf(fp,"\nstroke\n");
    fprintf(fp,"grestore\n");
}
static void write_circles_list(COUNT_DATA *count_data,int num,float pixel_size,
    FILE *fp)
{
    COUNT_EL *count_el;
    
    fprintf(fp,"%f %f %f setrgbcolor\n",
        (float)count_data->color[num].r/(float)255,
        (float)count_data->color[num].g/(float)255,
        (float)count_data->color[num].b/(float)255);
        
    count_el=count_data->list[num];
    do
    {
        fprintf(fp,"%f inch %f inch cd\n",((float)count_el->x+0.5)*pixel_size,
            ((float)count_el->y+0.5)*pixel_size);
        count_el=count_el->next;
    } while (count_el!=count_data->list[num]);
}


static void write_grid(PRINT_EL *this_el,FILE *fp)
{
    int i,j;
    int count;
    float x[5],y[5];
    float d[3];
    
     
    float r,theta,factor;
    int sofar;
    float temp,temp_x,temp_y;
    
    fprintf(fp,"gsave\n");
    fprintf(fp,"%f inch %f inch translate\n",this_el->x1,this_el->y1);
    fprintf(fp,"0.00333 inch setlinewidth\n");
    fprintf(fp,"%f %f %f setrgbcolor\n",(float)this_el->color.r/(float)255,
        (float)this_el->color.g/(float)255,
        (float)this_el->color.b/(float)255);
    if ((*(this_el->print_grid)).clip)
    {
        calc_r_theta(*(this_el->print_grid),&r,&theta);
/*
        fprintf(fp,"%% r: %f theta %f\n",r,theta);
*/
        if (intersection_pts(r,theta,x,y,x+1,y+1,(this_el->print_image)->size)==2)
        {
            sofar=2;
/*
            fprintf(fp,"%% x[0]: %f y[0]: %f x[1]: %f y[1]: %f\n",
                x[0],y[0],x[1],y[1]);
*/
            if (x[0]<x[1])
            {
                temp=x[0];
                x[0]=x[1];
                x[1]=temp;
                temp=y[0];
                y[0]=y[1];
                y[1]=temp;
            }
            if ((*(this_el->print_grid)).clip_y_less)
            {
                if (!line_side(*(this_el->print_grid),0,0))
                {
                    x[sofar]=0;
                    y[sofar]=0;
                    sofar++;
                }
                if (!line_side(*(this_el->print_grid),(this_el->print_image)->size-1,0))
                {
                    x[sofar]=(this_el->print_image)->size-1;
                    y[sofar]=0;
                    sofar++;
                }
                if (!line_side(*(this_el->print_grid),(this_el->print_image)->size-1,(this_el->print_image)->size-1))
                {
                    x[sofar]=(this_el->print_image)->size-1;
                    y[sofar]=(this_el->print_image)->size-1;
                    sofar++;
                }
                if (!line_side(*(this_el->print_grid),0,(this_el->print_image)->size-1))
                {
                    x[sofar]=0;
                    y[sofar]=(this_el->print_image)->size-1;
                    sofar++;
                }
            }
            else
            {
                if (line_side(*(this_el->print_grid),0,0))
                {
                    x[sofar]=0;
                    y[sofar]=0;
                    sofar++;
                }
                if (line_side(*(this_el->print_grid),0,(this_el->print_image)->size-1))
                {
                    x[sofar]=0;
                    y[sofar]=(this_el->print_image)->size-1;
                    sofar++;
                }
                if (line_side(*(this_el->print_grid),(this_el->print_image)->size-1,(this_el->print_image)->size-1))
                {
                    x[sofar]=(this_el->print_image)->size-1;
                    y[sofar]=(this_el->print_image)->size-1;
                    sofar++;
                }
                if (line_side(*(this_el->print_grid),(this_el->print_image)->size-1,0))
                {
                    x[sofar]=(this_el->print_image)->size-1;
                    y[sofar]=0;
                    sofar++;
                }
            }
            for (i=2;i<sofar;i++)
            {
                d[i-2]=sqrt((x[i]-x[1])*(x[i]-x[1])+(y[i]-y[1])*
                    (y[i]-y[1]));
            }
            j=2;
            for(i=3;i<sofar;i++)
            {
                if (d[i-2]<d[j-2]) j=i;
            }
            while(j>2)
            {
                temp_x=x[2];
                temp_y=y[2];
                for(i=2;i<sofar-1;i++)
                {
                    x[i]=x[i+1];
                    y[i]=y[i+1];
                }
                x[sofar-1]=temp_x;
                y[sofar-1]=temp_y;
                j--;
            }
            factor=((float)(this_el->sizex))/(float)(this_el->print_image)->size;
            fprintf(fp,"%f inch %f inch moveto\n",x[0]*factor,
                y[0]*factor);
            for(i=1;i<sofar;i++)
            {
                fprintf(fp,"%f inch %f inch lineto\n",x[i]*factor,
                     y[i]*factor);
            }
            fprintf(fp,"closepath clip newpath\n");
                
        }
    }
    for(i=0;i<GRID_MAX_LINES;i++)
    {
        if (!(*(this_el->print_grid)).line[i].hide)
        {
            r=(*(this_el->print_grid)).line[i].r;
            count=0;
            while (count<(this_el->print_image)->size)
            {
                write_grid_line(r,(*(this_el->print_grid)).line[i].theta,
                    (this_el->print_image)->size,this_el->sizex,fp);
                r+=(*(this_el->print_grid)).line[i].dist;
                count++;
            }
            if ((*(this_el->print_grid)).line[i].theta>0)
            {
                r=(*(this_el->print_grid)).line[i].r;
                count=0;
                while (count<(this_el->print_image)->size)
                {
                    write_grid_line(r,(*(this_el->print_grid)).line[i].theta,
                        (this_el->print_image)->size,this_el->sizex,fp);
                    r-=(*(this_el->print_grid)).line[i].dist;
                    count++;
                }
            }
        }
    }
    fprintf(fp,"\nstroke\n");
    fprintf(fp,"grestore\n");
        
}

int intersection_pts(float r,float theta,float *x1,float *y1,float *x2,
    float *y2,int image_size)
{
    int sofar=0;
    float line_x,line_y;
    float *x,*y;
    
    sofar=0;
    line_x=r*
        cos(RADIAN(theta+90));
    line_y=r*
        sin(RADIAN(theta+90));
    x=x1;
    y=y1;
    if (horiz_int(line_x,line_y,theta,x,0))
    {
        if (*x>=0 && 
            *x<=image_size-1)
        {
            *y=0;
            sofar++;
            x=x2;
            y=y2;
        }
    }
    if (horiz_int(line_x,line_y,theta,x,
        image_size-1))
    {
        if (*x>=0 && 
            *x<=image_size-1)
        {
            *y=image_size-1;
            sofar++;
            x=x2;
            y=y2;
        }
    }
    if (sofar<2)
    {
        if (vert_int(line_x,line_y,theta,0,y))
        {
            if (*y>=0 && 
                *y<=image_size-1)
            {
                *x=0;
                sofar++;
                x=x2;
                y=y2;
            }
        }
    }
    if (sofar<2)
    {
        if (vert_int(line_x,line_y,theta,
            image_size-1,y))
        {
            if (*y>=0 && 
                *y<=image_size-1)
            {
                *x=image_size-1;
                sofar++;
                x=x2;
                y=y2;
            }
        }
    }
    return(sofar);
}
static int write_grid_line(float r,float theta,int image_size,float print_size,
    FILE *fp)
{
    
    float temp_x1,temp_y1,temp_x2,temp_y2;
    int result=0;
    
    if (intersection_pts(r,theta,&temp_x1,&temp_y1,
        &temp_x2,&temp_y2,image_size)==2)
    {

        fprintf(fp,"%f inch %f inch moveto\n",
            (float) temp_x1/(float)image_size*print_size,
            (float) temp_y1/(float) image_size*print_size);
        fprintf(fp,"%f inch %f inch lineto\n",
            (float) temp_x2/(float)image_size*print_size,
            (float) temp_y2/(float) image_size*print_size);
        result=1;
    }
    return(result);
        
}

void re_link()
{
    PRINT_EL *this_el;
    
    if (print_list!=NULL)
    {
        this_el=print_list;
        do
        {
            if (this_el->type==PRINT_TYPE_IMAGE_LINK)
            {
                this_el->print_image=gendata[(this_el->link)];
                this_el->print_bitmap_size=&(im_bitmap_size[(this_el->link)]);
                this_el->print_bitmap=im_bitmaps[(this_el->link)];
                this_el->print_grid=&(grids[(this_el->link)]);
                this_el->count_data=&(count_data[this_el->link]);
            }
            this_el=this_el->next;
        } while(this_el!=print_list);
    }
    if (print_clipboard!=NULL)
    {
        this_el=print_clipboard;
        do
        {
            if (this_el->type==PRINT_TYPE_IMAGE_LINK)
            {
                this_el->print_image=gendata[(this_el->link)];
                this_el->print_bitmap_size=&(im_bitmap_size[(this_el->link)]);
                this_el->print_bitmap=im_bitmaps[(this_el->link)];
                this_el->print_grid=&(grids[(this_el->link)]);
                this_el->count_data=&(count_data[this_el->link]);
            }
            this_el=this_el->next;
        } while(this_el!=print_clipboard);
    }
}

static void remove_print_el(PRINT_EL **remove_list,PRINT_EL *remove_el)
{
    // frees all pointer members of remove_el and removes remove_el from remove_list

    PRINT_EL *this_el;
    
    if ((*remove_list)!=NULL)
    {
        this_el=*remove_list;
        do
        {
            if (this_el==remove_el)
            {
                switch(this_el->type)
                {
                    case PRINT_TYPE_IMAGE:
                        free_data(&(this_el->print_image));
                        free_count(&(this_el->count_data));
                        free(this_el->print_grid);
                        free(this_el->print_bitmap);
                        free(this_el->print_bitmap_size);
                        break;

                    case PRINT_TYPE_BAR:
                        free_data(&(this_el->print_image));
                        free(this_el->print_bitmap);
                        free(this_el->print_bitmap_size);
			if( this_el->bar_pal.type == PALTYPE_MAPPED) {
			    free( this_el->bar_pal.dacbox);
			}
			else {
			    destroy_logpal(&(this_el->bar_pal.logpal));
			}
                        break;
                }
                if (this_el->next==this_el) *remove_list=NULL;
                if (this_el==(*remove_list)) (*remove_list)=(*remove_list)->next;
                this_el->next->prev=this_el->prev;
                this_el->prev->next=this_el->next;
                free(this_el);
                break;
            }
            this_el=this_el->next;
        }while (this_el!=(*remove_list));
    }
}
                
void delete_selected()
{
    // deletes all selected PRINT_EL's from print_list

    PRINT_EL *this_el;
    
  delete_selected_top:
    if (print_list!=NULL)
    {
        this_el=print_list;
        do
        {
            if (this_el->selected)
            {
                remove_print_el(&print_list,this_el);
                goto delete_selected_top;
            }
            this_el=this_el->next;
        } while(this_el!=print_list);
    }
}

static void copy_print_el(PRINT_EL **this_list,PRINT_EL *this_el)
{
    PRINT_EL *new_el;
    int bitmap_factor = 1; // initialization, mw
    struct print_eldef *next;
    struct print_eldef *prev;
    
    new_el=new_print_el(this_list,this_el->type);
    next=new_el->next;
    prev=new_el->prev;
    
    memcpy(new_el,this_el,sizeof(PRINT_EL));
    new_el->next=next;
    new_el->prev=prev;
    
    switch (this_el->type)
    {
        case PRINT_TYPE_IMAGE:
            new_el->print_image=NULL;
            new_el->count_data=NULL;
            copy_data(&(new_el->print_image),&(this_el->print_image));
            if (this_el->print_image->pal.type==PALTYPE_LOGICAL) bitmap_factor=3;
            new_el->print_bitmap_size=(int *)malloc(sizeof(int));
            *new_el->print_bitmap_size=*this_el->print_bitmap_size;
            new_el->print_bitmap=(unsigned char *)
                malloc(sizeof(unsigned char)*(*this_el->print_bitmap_size)*bitmap_factor*
                (*this_el->print_bitmap_size));
            memcpy(new_el->print_bitmap,this_el->print_bitmap,
                sizeof(unsigned char)*(*this_el->print_bitmap_size)*bitmap_factor*
                (*this_el->print_bitmap_size));
            new_el->print_grid=(GRID *) malloc(sizeof(GRID));
            new_el->count_data=(COUNT_DATA *) malloc(sizeof(COUNT_DATA));
            init_count_data(new_el->count_data);
            copy_grid(new_el->print_grid,this_el->print_grid);
            copy_count(&(new_el->count_data),&(this_el->count_data));
            break;
	case PRINT_TYPE_BAR:
            new_el->print_image=NULL;
            copy_data(&(new_el->print_image),&(this_el->print_image));
            this_el->bar_pal.type = this_el->print_image->pal.type;
	    calc_print_pal_bitmap( new_el);
	    break;
    }
}

static void copy_selected()
{
    PRINT_EL *this_el;
    
    while(print_clipboard!=NULL)
    {
        remove_print_el(&print_clipboard,print_clipboard);
    }
    if (print_list!=NULL)
    {
        this_el=print_list;
        do
        {
            if (this_el->selected)
            {
                copy_print_el(&print_clipboard,this_el);
            }
            this_el=this_el->next;
        } while(this_el!=print_list);
    }
}

static void paste_selected()
{
    PRINT_EL *this_el;
    
    this_el=print_clipboard;
    
    if (this_el!=NULL)
    {
        do
        {
            copy_print_el(&print_list,this_el);
            this_el=this_el->next;
        } while (this_el!=print_clipboard);
    }
}

void delete_all()
{
    while(print_list!=NULL)
    {
        remove_print_el(&print_list,print_list);
    }

}
    
//void print_char(FILE *fp,int c)
void print_char(FILE *fp,unsigned char c)
{
    unsigned char high,low;
    
    high=c>>4;
    low=c & 0xF;
    ONE_BYTE_HEX(high);
    ONE_BYTE_HEX(low);
    chars_this_line+=2;
    if (chars_this_line==CHARS_PER_LINE)
    {
        fputc('\n',fp);
        chars_this_line=0;
    }
    
}

void get_char_bitmap(HWND hDlg,LOGFONT *font,char chr,char **bitmap,
    FONTBITMAPINFO *fbinfo)
{
    HDC             hDC;
    HDC             hMemDC;
    HFONT   hfont,hfont_old;
    HBITMAP hbm,hbm_old;
    ABC widths;
    NEWTEXTMETRIC textmetric;
    int angle;
    int x;
        
    hDC = GetDC(hDlg);
        
    hMemDC = CreateCompatibleDC(hDC);
/*
    SelectPalette(hMemDC,hPal,0);
    RealizePalette(hMemDC);
*/
    SetMapMode(hMemDC,MM_TEXT);
    angle=font->lfOrientation;
    font->lfOrientation=0;
    hfont=CreateFontIndirect(font);
    hfont_old=SelectObject(hMemDC,hfont);
    GetCharABCWidths(hMemDC,chr,chr,&widths);
    GetTextMetrics(hMemDC,&textmetric);
    fbinfo->font_x=widths.abcB;
    fbinfo->y=fbinfo->font_y=textmetric.tmHeight;
    fbinfo->font_descent=textmetric.tmDescent;
    fbinfo->del_x=widths.abcA+widths.abcB+widths.abcC;
    
    fbinfo->del_y=0;
    fbinfo->x=x=abs(widths.abcA)+widths.abcB+abs(widths.abcC);
    while(x>=32) x-=32;
    if (x) fbinfo->x+=32-x;
    if (!(fbinfo->x)) fbinfo->x=32;
    bitmapinfo->bmiHeader.biBitCount = 8;
    bitmapinfo->bmiHeader.biWidth=fbinfo->x ;
    bitmapinfo->bmiHeader.biHeight=fbinfo->y;
    *bitmap=(char *) malloc(sizeof(char)*fbinfo->x*fbinfo->y/8);
    memset(*bitmap,0,fbinfo->x*fbinfo->y/8);
    
    hbm = CreateCompatibleBitmap(hMemDC, fbinfo->x, fbinfo->y);
    hbm_old = SelectObject(hMemDC,hbm);
    BitBlt(hMemDC,0,0,fbinfo->x,fbinfo->y,hMemDC,0,0,WHITENESS);
    
    SetTextAlign(hMemDC,TA_BOTTOM | TA_LEFT);
    
    SetTextColor(hMemDC,RGB(0,0,0));
    SetBkColor(hMemDC,RGB(255,255,255));
    
    sprintf(string,"%c",chr);
    TextOut(hMemDC,0,fbinfo->y-1,string,1);

    
    SelectObject(hMemDC,hbm_old);
    GetBitmapBits(hbm,fbinfo->x*fbinfo->y/8,*bitmap);

    DeleteObject(hbm);
    
    SelectObject(hMemDC,hfont_old);
     
    
    DeleteObject(hfont);
    DeleteDC(hMemDC);
    ReleaseDC(hDlg,hDC);
}

static void repaint_text_controls(HWND hDlg)
{
    if ((cur_obj_type(PRINT_TYPE_TEXT) && print_obj_count==1))
    {
        SetDlgItemText(hDlg,PRINT_OBJECT,"Text");
        SetDlgItemText(hDlg,PRINT_OBJECT_COLOR,"");
        print_text_color=cur_obj->color;
        *(lpcf->lpLogFont)=cur_obj->print_font;
        lpcf->iPointSize=cur_obj->font_size*10;
        print_text_just=cur_obj->text_just;
        print_text_angle=cur_obj->angle;
        SetDlgItemText(hDlg,PRINT_TEXT,cur_obj->text);
        repaint_local_text_controls(hDlg);
    }
        
}

static void repaint_local_text_controls(HWND hDlg)
{
        out_smart=1;
        sprintf(string,"%0.2f",cur_obj->x1);
        SetDlgItemText(hDlg, PRINT_IMAGE_X, string);
        sprintf(string,"%0.2f",cur_obj->y1);
        SetDlgItemText(hDlg, PRINT_IMAGE_Y, string);
        sprintf(string,"%d",print_text_color.r);
        SetDlgItemText(hDlg, PRINT_GRID_R, string);
        sprintf(string,"%d",print_text_color.g);
        SetDlgItemText(hDlg, PRINT_GRID_G, string);
        sprintf(string,"%d",print_text_color.b);
        SetDlgItemText(hDlg, PRINT_GRID_B, string);
        SetDlgItemText(hDlg,PRINT_TEXT_FONT_NAME,(lpcf->lpLogFont)->lfFaceName);
        sprintf(string,"%.3f",(float)lpcf->iPointSize/10.0);
        SetDlgItemText(hDlg,PRINT_TEXT_SIZE,string);
        sprintf(string,"%.1f",print_text_angle);
        SetDlgItemText(hDlg,PRINT_TEXT_ANGLE,string);
        CheckDlgButton(hDlg,PRINT_TEXT_JUST_L,0);
        CheckDlgButton(hDlg,PRINT_TEXT_JUST_C,0);
        CheckDlgButton(hDlg,PRINT_TEXT_JUST_R,0);
        CheckDlgButton(hDlg,print_text_just,1);
        
        out_smart=0;
}

static void repaint_print_text(HWND hDlg,HDC hDC,PRINT_EL *this_el)
{
    char *this_char;
    char *bitmap;
    HDC             hMemDC;
    BITMAP bm;
    HBITMAP hbm,hbm_old;
    HBRUSH hbrush,hbrush_old;
    FONTBITMAPINFO fbinfo;
    int x,y;
    int sel_x,sel_y;
    int font_height;
    int font_scale=1;
    float temp_size;
    
    this_char=this_el->text;
    x=INCH_X_TO_PIXEL(this_el->x1);
    y=INCH_Y_TO_PIXEL(this_el->y1);
    
    hMemDC = CreateCompatibleDC(hDC);
    SelectPalette(hDC,hPal,0);
    RealizePalette(hDC);
    SetMapMode(hDC,MM_TEXT);
    SetMapMode(hMemDC,MM_TEXT);
    this_el->print_font.lfOrientation=10*this_el->angle;
    this_el->print_font.lfWidth=0;
    font_height=this_el->print_font.lfHeight;
    this_el->print_font.lfHeight=(int) (((float)this_el->print_font.lfHeight)*
        ((float)print_pixels_per_inch)/((float)PRINT_PIXELS_PER_INCH));
    temp_size=this_el->font_size*print_pixels_per_inch/PRINT_PIXELS_PER_INCH;
    while(temp_size<8)
    {
        temp_size*=2;
        this_el->print_font.lfHeight*=2;
        font_scale*=2;
    }
    while(*this_char)
    {
        get_char_bitmap(hDlg,&(this_el->print_font),*this_char,&bitmap,
            &fbinfo);
    
        bm.bmType=0;
        bm.bmWidth=fbinfo.x;
        bm.bmHeight=fbinfo.y;
        bm.bmWidthBytes=fbinfo.x/8;
        bm.bmPlanes=1;
        bm.bmBitsPixel=1;
        bm.bmBits=bitmap;
        hbm=CreateBitmapIndirect(&bm);
        hbm_old=SelectObject(hMemDC,hbm);
/*
        BitBlt(hDC, x + gm.gmptGlyphOrigin.x, y + (ascent - gm.gmptGlyphOrigin.y), 
        gm.gmBlackBoxX, gm.gmBlackBoxY, hdcMem, 0, 0, SRCCOPY);
*/
        
        x+=fbinfo.del_x/font_scale;
        y+=fbinfo.del_y/font_scale;
        
        SelectObject(hMemDC,hbm_old);
        DeleteObject(hbm);
        
        free(bitmap);
        
        this_char++;
    }
    this_el->x2=PIXEL_X_TO_INCH(x);
    this_el->y2=PIXEL_Y_TO_INCH(y-fbinfo.y/font_scale);
    this_el->sizex=this_el->x2-this_el->x1;
    this_el->sizey=this_el->y2-this_el->y1;
    switch(this_el->text_just)
    {
        case PRINT_TEXT_JUST_L:
            x=INCH_X_TO_PIXEL(this_el->x1);
            y=INCH_Y_TO_PIXEL(this_el->y1);
            
            break;
        case PRINT_TEXT_JUST_R:
            x=INCH_X_TO_PIXEL(2*this_el->x1-this_el->x2);
            y=INCH_Y_TO_PIXEL(this_el->y1);
            break;
        case PRINT_TEXT_JUST_C:
            x=INCH_X_TO_PIXEL(this_el->x1-(this_el->x2-this_el->x1)/2);
            y=INCH_Y_TO_PIXEL(this_el->y1);
            break;
    }
    sel_x=x;
    sel_y=y;
    this_char=this_el->text;
    while(*this_char)
    {
        get_char_bitmap(hDlg,&(this_el->print_font),*this_char,&bitmap,
            &fbinfo);
    
        bm.bmType=0;
        bm.bmWidth=fbinfo.x;
        bm.bmHeight=fbinfo.y;
        bm.bmWidthBytes=fbinfo.x/8;
        bm.bmPlanes=1;
        bm.bmBitsPixel=1;
        bm.bmBits=bitmap;
        hbm=CreateBitmapIndirect(&bm);
        hbm_old=SelectObject(hMemDC,hbm);
        
        hbrush=CreateSolidBrush(RGB(this_el->color.r,this_el->color.g,
            this_el->color.b));
        hbrush_old=SelectObject(hDC,hbrush);
        
        StretchBlt(hDC, x, y-fbinfo.y/font_scale, 
            fbinfo.x/font_scale, fbinfo.y/font_scale, hMemDC, 0, 0, 
            fbinfo.x,fbinfo.y,SRCAND);
        StretchBlt(hDC, x, y-fbinfo.y/font_scale, 
            fbinfo.x/font_scale, fbinfo.y/font_scale, hMemDC, 0, 0, 
            fbinfo.x,fbinfo.y,(DWORD)0xBA0B09);
        x+=fbinfo.del_x/font_scale;
        y+=fbinfo.del_y/font_scale;
        
        SelectObject(hMemDC,hbm_old);
        DeleteObject(hbm);
        SelectObject(hDC,hbrush_old);
        DeleteObject(hbrush);
        
        free(bitmap);
        
        this_char++;
    }
    this_el->x2=PIXEL_X_TO_INCH(x);
    this_el->y2=PIXEL_Y_TO_INCH(y-fbinfo.y/font_scale);
    if (this_el->selected)
    {
        selection_squares(hDC,sel_x,
            sel_y,x,y-fbinfo.y/font_scale);
    }
    this_el->print_font.lfHeight=font_height;

    DeleteDC(hMemDC);
    
}

static void write_text(HWND hDlg,PRINT_EL *this_el,FILE *fp)
{
    char *this_char;
    char *bitmap;
    FONTBITMAPINFO fbinfo;
    float x,y;
    int font_height;
    
    this_char=this_el->text;
    x=this_el->x1;
    y=this_el->y1;
        
    
    this_el->print_font.lfOrientation=10*this_el->angle;
    this_el->print_font.lfWidth=0;
    font_height=this_el->print_font.lfHeight;
    this_el->print_font.lfHeight=(int) (((float)this_el->print_font.lfHeight)*
        ((float)device_res)/((float)PRINT_PIXELS_PER_INCH));
    while(*this_char)
    {
        get_char_bitmap(hDlg,&(this_el->print_font),*this_char,&bitmap,
            &fbinfo);
    
//        place_char(bitmap,x,y,fbinfo.x,fbinfo.y,this_el->color,fp);

        x+=((float)fbinfo.del_x)/(float)device_res;
        y+=((float)fbinfo.del_y)/(float)device_res;
        
        free(bitmap);
        
        this_char++;
    }
    this_char=this_el->text;
    switch(this_el->text_just)
    {
        case PRINT_TEXT_JUST_L:
            x=this_el->x1;
            y=this_el->y1;
            
            break;
        case PRINT_TEXT_JUST_R:
            x=2*this_el->x1-x;
            y=this_el->y1;
            break;
        case PRINT_TEXT_JUST_C:
            x=this_el->x1-(x-this_el->x1)/2;
            y=this_el->y1;
            break;
    }
    while(*this_char)
    {
        fprintf(fp,"%% %c (%d) of %s\n",*this_char,this_char-this_el->text,
            this_el->text);
        get_char_bitmap(hDlg,&(this_el->print_font),*this_char,&bitmap,
            &fbinfo);
    
        place_char(bitmap,x,y,fbinfo.x,fbinfo.y,this_el->color,fp);

        x+=((float)fbinfo.del_x)/(float)device_res;
        y+=((float)fbinfo.del_y)/(float)device_res;
        
        free(bitmap);
        
        this_char++;
    }
    this_el->print_font.lfHeight=font_height;

    
}

static void place_char(char *bitmap,float x,float y,int bitmap_x,int bitmap_y,
    STM_COLOR color,FILE *fp)
{
    int i,j;
    
    fprintf(fp,"gsave\n");
    fprintf(fp,"%f inch %f inch translate\n",x,y);
    fprintf(fp,"%f inch %f inch scale\n",((float)bitmap_x)/(float)device_res,
        ((float)bitmap_y)/(float)device_res);
    fprintf(fp,"%f %f %f setrgbcolor\n",((float)color.r)/255.0,
        ((float)color.g)/255.0,((float)color.b)/255.0);
    pix=bitmap_x/8;
    fprintf(fp,"/pix %d string def\n",pix);
    fprintf(fp,"%d %d ",bitmap_x,bitmap_y);
    fprintf(fp,"true [%d 0 0 %d 0 0] {currentfile pix readhexstring pop} imagemask\n",bitmap_x,bitmap_y);
    chars_this_line=0;
    for(i=bitmap_y-1;i>=0;i--)
    {
        for(j=0;j<bitmap_x/8;j++)
        {
            print_char(fp,255-*(bitmap+i*bitmap_x/8+j));
        }
    }
    fprintf(fp,"\ngrestore\n");
}

static void repaint_arrow_controls(HWND hDlg)
{
    int i;
    if ((cur_obj_type(PRINT_TYPE_LINE) && print_obj_count==1))
    {
        SetDlgItemText(hDlg,PRINT_OBJECT,"Arrow");
        SetDlgItemText(hDlg,PRINT_OBJECT_COLOR,"");
        print_arrow_color=cur_obj->color;
        print_arrow_head=cur_obj->arrow_head;
        print_arrow_width=cur_obj->arrow_width;
        print_arrow_head_type=cur_obj->arrow_head_type;
        for(i=0;i<LISTBOXMAX;i++)
        {
            if (print_arrow_heads.index[i]==cur_obj->arrow_head_type)
                break;
        }
        SendDlgItemMessage(hDlg,PRINT_ARROW_TYPE,
            CB_SETCURSEL,i,0);
        
        sprintf(string,"%0.2f",cur_obj->arrow_head);
        SetDlgItemText(hDlg, PRINT_ARROW_HEAD, string);
        sprintf(string,"%0.2f",cur_obj->arrow_width);
        SetDlgItemText(hDlg, PRINT_ARROW_WIDTH, string);
        sprintf(string,"%0.2f",cur_obj->x1);
        SetDlgItemText(hDlg, PRINT_IMAGE_X, string);
        sprintf(string,"%0.2f",cur_obj->y1);
        SetDlgItemText(hDlg, PRINT_IMAGE_Y, string);
        sprintf(string,"%0.2f",cur_obj->x2);
        SetDlgItemText(hDlg, PRINT_ARROW_X2, string);
        sprintf(string,"%0.2f",cur_obj->y2);
        SetDlgItemText(hDlg, PRINT_ARROW_Y2, string);
        sprintf(string,"%d",print_arrow_color.r);
        SetDlgItemText(hDlg, PRINT_GRID_R, string);
        sprintf(string,"%d",print_arrow_color.g);
        SetDlgItemText(hDlg, PRINT_GRID_G, string);
        sprintf(string,"%d",print_arrow_color.b);
        SetDlgItemText(hDlg, PRINT_GRID_B, string);
        out_smart=0;
    }
        
}

static void repaint_xor_arrow(HWND hDlg,PRINT_EL *this_el)
{
    HDC hDC;
    
    hDC=GetDC(hDlg);
    repaint_arrow(hDC,this_el,R2_NOTXORPEN,0);
    
    ReleaseDC(hDlg,hDC);
}

static void repaint_arrow(HDC hDC,PRINT_EL *this_el,int rasterop,
    int sel_box)
{
    float theta;
    float length;
    
    if (this_el->y1==this_el->y2 && this_el->x1==this_el->x2) return;
    
    
    
    theta=atan2(this_el->y2-this_el->y1,this_el->x2-this_el->x1);
    length=sqrt((this_el->y2-this_el->y1)*(this_el->y2-this_el->y1)+
        (this_el->x2-this_el->x1)*(this_el->x2-this_el->x1));
    switch(this_el->arrow_head_type)
    {
        case PRINT_ARROW_HEAD_NONE:
            repaint_arrow_line(hDC,this_el,rasterop,this_el->x1,
                this_el->y1,this_el->x2,this_el->y2);
            break;
        case PRINT_ARROW_HEAD_END:
            repaint_arrow_line(hDC,this_el,rasterop,this_el->x1,
                this_el->y1,
                this_el->x1+cos(theta)*(length-this_el->arrow_head),
                this_el->y1+sin(theta)*(length-this_el->arrow_head));
            repaint_arrow_head(hDC,this_el,rasterop,this_el->x2,this_el->y2,
                1);
            break;
        case PRINT_ARROW_HEAD_START:
            repaint_arrow_line(hDC,this_el,rasterop,this_el->x2,
                this_el->y2,
                this_el->x2-cos(theta)*(length-this_el->arrow_head),
                this_el->y2-sin(theta)*(length-this_el->arrow_head));
            repaint_arrow_head(hDC,this_el,rasterop,this_el->x1,this_el->y1,
                -1);
            break;
        case PRINT_ARROW_HEAD_BOTH:
            repaint_arrow_line(hDC,this_el,rasterop,
                this_el->x1+cos(theta)*(length-this_el->arrow_head),
                this_el->y1+sin(theta)*(length-this_el->arrow_head),
                this_el->x2-cos(theta)*(length-this_el->arrow_head),
                this_el->y2-sin(theta)*(length-this_el->arrow_head));
            repaint_arrow_head(hDC,this_el,rasterop,this_el->x2,this_el->y2,
                1);
            repaint_arrow_head(hDC,this_el,rasterop,this_el->x1,this_el->y1,
                -1);
            break;
    }
    
    if (sel_box && this_el->selected)
    {
        selection_squares(hDC,INCH_X_TO_PIXEL(this_el->x1),
            INCH_Y_TO_PIXEL(this_el->y1),INCH_X_TO_PIXEL(this_el->x2),
            INCH_Y_TO_PIXEL(this_el->y2)); 
    }
            
            
}
static void repaint_arrow_head(HDC hDC,PRINT_EL *this_el,int rasterop,
    float x1,float y1,int polarity)
{
    int OldROP;
    HPEN            hPen,hPenOld;
    float theta;
    float length;
    POINT head[3];
    float x,y;
    LOGBRUSH        lplb;
    HBRUSH          hBrush,
                    hBrushOld;
    
    
    OldROP = SetROP2(hDC, rasterop);
    if (this_el->y1==this_el->y2 && this_el->x1==this_el->x2) return;
    
    
    theta=atan2(this_el->y2-this_el->y1,this_el->x2-this_el->x1);
    
    hPen = CreatePen(PS_SOLID,1,RGB(this_el->color.r,this_el->color.g,
        this_el->color.b));
    hPenOld = SelectObject(hDC,hPen);
    lplb.lbStyle = BS_SOLID;
    lplb.lbColor = RGB(this_el->color.r,this_el->color.g,this_el->color.b);
    hBrush = CreateBrushIndirect(&lplb);
    hBrushOld = SelectObject(hDC, hBrush);

    head[0].x=INCH_X_TO_PIXEL(x1);
    head[0].y=INCH_Y_TO_PIXEL(y1);
    length=(this_el->arrow_head)/cos(RADIAN(PRINT_ARROW_HEAD_ANGLE)/2);
    theta=theta-RADIAN(PRINT_ARROW_HEAD_ANGLE)/2+PI;
    x=x1+length*cos(theta)*polarity;
    y=y1+length*sin(theta)*polarity;
    head[1].x=INCH_X_TO_PIXEL(x);
    head[1].y=INCH_Y_TO_PIXEL(y);
    theta+=RADIAN(PRINT_ARROW_HEAD_ANGLE);
    x=x1+length*cos(theta)*polarity;
    y=y1+length*sin(theta)*polarity;
    head[2].x=INCH_X_TO_PIXEL(x);
    head[2].y=INCH_Y_TO_PIXEL(y);
    Polygon(hDC,head,3);
    
    
    
    SelectObject(hDC, hBrushOld);
    SelectObject(hDC, hPenOld);
    DeleteObject(hBrush);
    DeleteObject(hPen);
            
    SetROP2(hDC, OldROP);
            
}

static void repaint_arrow_line(HDC hDC,PRINT_EL *this_el,int rasterop,
    float x1,float y1,float x2,float y2)
{
    int OldROP;
    HPEN            hPen,hPenOld;
    int thickness;
    
    
    OldROP = SetROP2(hDC, rasterop);
    thickness=(int)(this_el->arrow_width*(float)print_pixels_per_inch+0.5);
    if (!thickness) thickness=1;
    hPen = CreatePen(PS_SOLID,thickness,RGB(this_el->color.r,this_el->color.g,
        this_el->color.b));
    hPenOld = SelectObject(hDC,hPen);
    
//    MoveTo(hDC,INCH_X_TO_PIXEL(x1),INCH_Y_TO_PIXEL(y1));
    MoveToEx(hDC,INCH_X_TO_PIXEL(x1),INCH_Y_TO_PIXEL(y1),NULL);
    LineTo(hDC,INCH_X_TO_PIXEL(x2),INCH_Y_TO_PIXEL(y2));
    SelectObject(hDC,hPenOld);
    DeleteObject(hPen);
    
    SetROP2(hDC, OldROP);
}

static void write_arrow(HWND hDlg,PRINT_EL *this_el,FILE *fp)
{
    float theta,length;
    
    fprintf(fp,"gsave\n");
    fprintf(fp,"%f %f %f setrgbcolor\n",((float)this_el->color.r)/255.0,
        ((float)this_el->color.g)/255.0,((float)this_el->color.b)/255.0);
    fprintf(fp,"newpath\n");
    switch(this_el->arrow_head_type)
    {
        case PRINT_ARROW_HEAD_NONE:
            fprintf(fp,"%f inch setlinewidth\n",this_el->arrow_width);
            fprintf(fp,"%f inch %f inch moveto %f inch %f inch lineto stroke\n",
            this_el->x1,this_el->y1,this_el->x2,this_el->y2);            
            break;
        case PRINT_ARROW_HEAD_END:
            fprintf(fp,"%f inch %f inch %f inch %f inch\n",this_el->x1,
                this_el->y1,this_el->x2,this_el->y2);
            fprintf(fp,"%f inch %f inch %f inch arrow\n",this_el->arrow_width,
                tan(RADIAN(PRINT_ARROW_HEAD_ANGLE/2))*this_el->arrow_head*2.0,
                this_el->arrow_head);
            break;
        case PRINT_ARROW_HEAD_START:
            fprintf(fp,"%f inch %f inch %f inch %f inch\n",this_el->x2,
                this_el->y2,this_el->x1,this_el->y1);
            fprintf(fp,"%f inch %f inch %f inch arrow\n",this_el->arrow_width,
                tan(RADIAN(PRINT_ARROW_HEAD_ANGLE/2))*this_el->arrow_head*2.0,
                this_el->arrow_head);
            break;
        case PRINT_ARROW_HEAD_BOTH:
            theta=atan2(this_el->y2-this_el->y1,this_el->x2-this_el->x1);
            length=sqrt((this_el->y2-this_el->y1)*(this_el->y2-this_el->y1)+
                (this_el->x2-this_el->x1)*(this_el->x2-this_el->x1));
            fprintf(fp,"%f inch %f inch %f inch %f inch\n",
                this_el->x1+cos(theta)*(length-this_el->arrow_head),
                this_el->y1+sin(theta)*(length-this_el->arrow_head),
                this_el->x1,this_el->y1);
            fprintf(fp,"%f inch %f inch %f inch arrow\n",this_el->arrow_width,
                tan(RADIAN(PRINT_ARROW_HEAD_ANGLE/2))*this_el->arrow_head*2.0,
                this_el->arrow_head);
            fprintf(fp,"%f inch %f inch %f inch %f inch\n",
                this_el->x2-cos(theta)*(length-this_el->arrow_head),
                this_el->y2-sin(theta)*(length-this_el->arrow_head),
                this_el->x2,this_el->y2);
            fprintf(fp,"%f inch %f inch %f inch arrow\n",this_el->arrow_width,
                tan(RADIAN(PRINT_ARROW_HEAD_ANGLE/2))*this_el->arrow_head*2.0,
                this_el->arrow_head);
            break;
    }
        
    fprintf(fp,"fill\n");
    fprintf(fp,"grestore\n");
}

static void prune()
{   
    PRINT_EL *this_el;
    int removed;
    
    this_el=print_list;
    do
    {
        removed=0;
        switch(this_el->type)
        {
            case PRINT_TYPE_LINE:
                if (this_el->x1==this_el->x2 && this_el->y1==this_el->y2)
                {
                    remove_print_el(&print_list,this_el);
                    this_el=print_list;
                    removed=1;
                }
                break;
            case PRINT_TYPE_TEXT:
                if (!this_el->text[0])
                {
                    remove_print_el(&print_list,this_el);
                    this_el=print_list;
                    removed=1;
                }
                break;
        }
        this_el=this_el->next;
    }
    while(this_el!=print_list || removed);
}

void init_arrow_heads()
{
    int i;
    print_arrow_heads.num=4;
    
    for(i=0;i<print_arrow_heads.num;i++)
    {
        print_arrow_heads.str[i]=(char *) malloc(sizeof(char)*PRINT_ARROW_HEADS_MAX);
    }
    strcpy(print_arrow_heads.str[PRINT_ARROW_HEAD_NONE],"None");
    strcpy(print_arrow_heads.str[PRINT_ARROW_HEAD_START],"Start");
    strcpy(print_arrow_heads.str[PRINT_ARROW_HEAD_END],"End");
    strcpy(print_arrow_heads.str[PRINT_ARROW_HEAD_BOTH],"Both");
}
    
static PRINT_EL *add_image(int num)
{
    // creates and returns a PRINT_EL of type PRINT_TYPE_IMAGE
    // num is the index of the image in gendata[] (0 = topleft in data manip, etc)

    PRINT_EL *this_el;
    int bitmap_factor=1; 
    COUNT_DATA *this_count_data;
    
    this_count_data=&(count_data[num]);
    
    this_el= new_print_el(&print_list,PRINT_TYPE_IMAGE);
    copy_data(&(this_el->print_image),&(gendata[num]));
    if (gendata[num]->pal.type==PALTYPE_LOGICAL) bitmap_factor=3;
    this_el->print_bitmap_size=(int *)malloc(sizeof(int));
    *this_el->print_bitmap_size=im_bitmap_size[num];
    this_el->print_bitmap=(unsigned char *)
        malloc(sizeof(unsigned char)*(*this_el->print_bitmap_size)*bitmap_factor*
        (*this_el->print_bitmap_size));
    memcpy(this_el->print_bitmap,im_bitmaps[num],
        sizeof(unsigned char)*(*this_el->print_bitmap_size)*bitmap_factor*
        (*this_el->print_bitmap_size));
    this_el->print_grid=(GRID *) malloc(sizeof(GRID));
    copy_grid(this_el->print_grid,&(grids[num]));
    this_el->count_data=(COUNT_DATA *) malloc(sizeof(COUNT_DATA));
    init_count_data(this_el->count_data);
    copy_count(&(this_el->count_data),&(this_count_data));
    this_el->gamma=image_gamma;
    this_el->color.r=0;
    this_el->color.g=0;
    this_el->color.b=0;
    
    return(this_el);
}

static void reset_current()
{
    clear_selection();
    print_obj_count=0;
    cur_obj=NULL;
}    

static int cur_obj_type(int type)
{
    int result=0;
     
    if (cur_obj)
    {
        if (type==PRINT_TYPE_ANY_IMAGE)
            result=((cur_obj->type==PRINT_TYPE_IMAGE || 
                cur_obj->type==PRINT_TYPE_IMAGE_LINK));
        else result=(cur_obj->type==type);
    }
    return result;
}

static float max_print_x_origin()
{
    return((float)PRINT_IMAGE_X_MAX-((float)PRINT_AREA_WIDTH)/print_pixels_per_inch);
}

static float max_print_y_origin()
{
    return((float)PRINT_IMAGE_Y_MAX-((float)PRINT_AREA_HEIGHT)/print_pixels_per_inch);
}

static void print_do_scrolls(HWND hDlg)
{
    print_do_x_scroll(hDlg);
    print_do_y_scroll(hDlg);
}

static void print_do_x_scroll(HWND hDlg)
{

    print_x_origin=min(print_x_origin,max_print_x_origin());
    SetScrollPos(GetDlgItem(hDlg, PRINT_X_SCROLL), 
            SB_CTL,DTOI(print_x_origin,
            0,max_print_x_origin()),
            TRUE);
}
static void print_do_y_scroll(HWND hDlg)
{

    print_y_origin=min(print_y_origin,max_print_y_origin());
    SetScrollPos(GetDlgItem(hDlg, PRINT_Y_SCROLL), 
            SB_CTL,DTOI(print_y_origin,
            0,max_print_y_origin()),
            TRUE);
}

static int draw_grid_line(HDC hDC,float r,float theta,int 
    left,int bottom,int right,int top,int x_offset,int y_offset,
    float pixel_size)
{
    int sofar;
    
    float *x,*y;
    float temp_x1,temp_y1,temp_x2,temp_y2;
    int xs,ys;
    float line_x,line_y;
    int result=0;
    LOGBRUSH      lplb;
    HBRUSH        hBrush,hBrushOld;
    HPEN          hPen,hPenOld;
    
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

    if (horiz_int(line_x,line_y,theta,x,bottom))
    {
        if (*x>=left && 
            *x<=right)
        {
            *y=bottom;
            sofar++;
            x=&temp_x2;
            y=&temp_y2;
        }
    }
    if (horiz_int(line_x,line_y,theta,x,top))
    {
        if (*x>=left && 
            *x<=right)
        {
            *y=top;
            sofar++;
            x=&temp_x2;
            y=&temp_y2;
        }
    }
    if (sofar<2)
    {
        if (vert_int(line_x,line_y,theta,left,y))
        {
            if (*y>=bottom && 
                *y<=top)
            {
                *x=left;
                sofar++;
                x=&temp_x2;
                y=&temp_y2;
            }
        }
    }
    if (sofar<2)
    {
        if (vert_int(line_x,line_y,theta,
            right,y))
        {
            if (*y>=bottom && 
                *y<=top)
            {
                *x=right;
                sofar++;
                x=&temp_x2;
                y=&temp_y2;
            }
        }
    }
    if (sofar==2)
    {

        xs=x_offset+temp_x1*pixel_size;
        ys=y_offset-temp_y1*pixel_size;
//        MoveTo(hDC,xs,ys);
        MoveToEx(hDC,xs,ys,NULL);

/*
        sprintf(string,"%f %f %f %f %d %d %d",temp_x1,temp_y1,temp_x2,temp_y2,this_image,xs,ys);
        MessageBox(hDlg,string,"Debug",MB_ICONHAND);
*/

        xs=x_offset+temp_x2*pixel_size;
        ys=y_offset-temp_y2*pixel_size;
        
        LineTo(hDC,xs,ys);
    }
    SelectObject(hDC,hBrushOld);
    SelectObject(hDC,hPenOld);
    DeleteObject(hBrush);
    DeleteObject(hPen);
    return(result);
        
}

static void repaint_grids(HDC hDC,PRINT_EL *this_el)
{
    int i;
    int count;
    
    float r;
    int left,right,top,bottom;
    int x_offset,y_offset;
    float pixel_size;
    
    x_offset=INCH_X_TO_PIXEL(this_el->x1);
    y_offset=INCH_Y_TO_PIXEL(this_el->y1)-1;
    pixel_size=(float)this_el->sizex/(float)this_el->print_image->size*
        print_pixels_per_inch;


    if (this_el->x1>=print_x_origin)
    {
        left=0;
    }
    else
    {
        left=(print_x_origin-this_el->x1)/this_el->sizex*
            this_el->print_image->size;
    }
    if (this_el->x1+this_el->sizex<=print_x_origin+PRINT_AREA_WIDTH*print_pixels_per_inch)
    {
        right=this_el->print_image->size-1;
    }
    else
    {
        right=(print_x_origin+PRINT_AREA_WIDTH*print_pixels_per_inch- 
            this_el->x1)/this_el->sizex*
            this_el->print_image->size;
    }
    if (this_el->y1>=print_y_origin)
    {
        bottom=0;
    }
    else
    {
        bottom=(print_y_origin-this_el->y1)/this_el->sizex*
            this_el->print_image->size;
    }
    if (this_el->y1+this_el->sizex<=print_y_origin+PRINT_AREA_HEIGHT*print_pixels_per_inch)
    {
        top=this_el->print_image->size-1;
    }
    else
    {
        top=(print_y_origin+PRINT_AREA_WIDTH*print_pixels_per_inch- 
            this_el->y1)/this_el->sizex*
            this_el->print_image->size;
    }
    
/*
sprintf(string,"%d %d %d %d",left,bottom,right,top);
mprintf(string);
*/
    
    for(i=0;i<GRID_MAX_LINES;i++)
    {
        if (!(this_el->print_grid)->line[i].hide)
        {
            r=(this_el->print_grid)->line[i].r;
            count=0;
            while (count<this_el->print_image->size)
            {
                draw_grid_line(hDC,r,(this_el->print_grid)->line[i].theta,
                    left,bottom,right,top,x_offset,y_offset,pixel_size);
                r+=(this_el->print_grid)->line[i].dist;
                count++;
            }
            if ((this_el->print_grid)->line[i].theta>0)
            {
                r=(this_el->print_grid)->line[i].r;
                count=0;
                
                while (count<this_el->print_image->size)
                {
                    draw_grid_line(hDC,r,(this_el->print_grid)->line[i].theta,
                        left,bottom,right,top,x_offset,y_offset,pixel_size);
                    r-=(this_el->print_grid)->line[i].dist;
                    count++;
                }
            }
        }
    }
    
        
}

void scale_this_el(PRINT_EL *this_el,float factor)
{
    this_el->x1*=factor;
    this_el->y1*=factor;
    this_el->sizex*=factor;
    this_el->sizey*=factor;
    switch(this_el->type)
    {
        case PRINT_TYPE_IMAGE:
        case PRINT_TYPE_IMAGE_LINK:
        case PRINT_TYPE_EPS:
            break;
        case PRINT_TYPE_TEXT:
            this_el->x2*=factor;
            this_el->y2*=factor;
            this_el->font_size*=factor;
            this_el->print_font.lfHeight*=factor;
            break;
        case PRINT_TYPE_LINE:
            this_el->x2*=factor;
            this_el->y2*=factor;
            this_el->arrow_width*=factor;
            this_el->arrow_head*=factor;
            break;
    }
}
        
static void write_colorbar( HWND hDlg, FILE *fp, PRINT_EL *this_bar, int this_output_color)
{
    int rows, cols, current_row, current_col, result;
    float value;
    HDC hDC;
    HPALETTE hPal_Old;
    PALETTEENTRY palEntry;
    int pal_index, interp_index;
    char red_level, green_level, blue_level, grey_level;

    hDC = GetDC( hDlg);

    rows = (int)( (float)this_bar->sizey * (float)device_res);
    cols = (int)( (float)this_bar->sizex * (float)device_res);

    switch( this_output_color)
    {
        case PRINT_BW:
            chars_this_line=0;

            for( current_row = 0; current_row < rows; ++current_row )
            {
                for( current_col=0; current_col < cols; current_col++)
                {
		  /* under construction */
                   value = 0.0;
                   result = (char) pof_grey( value, this_bar->gamma);
                   print_char( fp, result);
                }

                sprintf(string,"color bar: printed row %d/%d",current_row+1,rows);
                print_status(hDlg,string);
            }
          break;
        case PRINT_GREY:
            chars_this_line=0;

            switch( this_bar->bar_pal.type) {
               case PALTYPE_MAPPED:
                    spindac( 0, 1, this_bar->bar_pal.dacbox);
                    hPal_Old = SelectPalette( hDC, hPal, 0);
                    RealizePalette( hDC);
	            for( current_row = 0; current_row < rows; ++current_row )
	            {
	                for( current_col=0; current_col < cols; current_col++)
	                {
				/*
				   Y = 0.212671 * R + 0.715160 * G + 0.072169 * B;
				*/
			   
			   interp_index = (int) floor( (float)current_col * (float)PAL_IM_WIDTH / (float)cols);
			   pal_index = *(pal_image + interp_index);
			
			   GetPaletteEntries( hPal, pal_index, 1 /*get one entry*/, &palEntry);
			   grey_level = (char) ( pow ( ( 0.212671*(float)palEntry.peRed +
						       0.715160*(float)palEntry.peGreen +
						       0.072169*(float)palEntry.peBlue )
							/(float)255,this_bar->gamma)*255);
			 
                   	   print_char( fp, grey_level);
                	}

                        sprintf(string,"color bar: printed row %d/%d",current_row+1,rows);
                        print_status(hDlg,string);
            	   }
                   SelectPalette( hDC, hPal_Old, 0);
                   RealizePalette( hDC);
		   break;
		case PALTYPE_LOGICAL:
		  for( current_row = 0; current_row < rows; ++current_row )
                  {
                     for( current_col=0; current_col < cols; current_col++)
                     {
			 interp_index = (int) floor( (float)current_col * (float)PAL_IM_WIDTH / (float)cols);
			 red_level = (char) *(this_bar->print_bitmap + 3*interp_index + 2);
			 green_level = (char) *(this_bar->print_bitmap + 3*interp_index + 1);
			 blue_level = (char) *(this_bar->print_bitmap + 3*interp_index);

			 grey_level = (char) ( pow ( ( 0.212671*(float)red_level +
						     0.715160*(float)green_level +
						     0.072169*(float)blue_level )
							/(float)255,this_bar->gamma)*255);

                   	 print_char( fp, grey_level);
                     }

                     sprintf(string,"color bar: printed row %d/%d",current_row+1,rows);
                     print_status(hDlg,string);
            	  }
		   break;
	   }
           break;
        case PRINT_COLOR:
            chars_this_line=0;

            switch( this_bar->bar_pal.type) {
               case PALTYPE_MAPPED:
                  spindac( 0, 1, this_bar->bar_pal.dacbox);
                  hPal_Old = SelectPalette( hDC, hPal, 0);
                  RealizePalette( hDC);
		  for( current_row = 0; current_row < rows; ++current_row )
                  {
                     for( current_col=0; current_col < cols; current_col++)
                     {
			 interp_index = (int) floor( (float)current_col * (float)PAL_IM_WIDTH / (float)cols);
			 pal_index = *(this_bar->print_bitmap + interp_index);
			
			 GetPaletteEntries( hPal, pal_index, 1 /*get one entry*/, &palEntry);
			 red_level = (char) (pow((float)palEntry.peRed/(float)255,this_bar->gamma)*255);
    			 green_level = (char) (pow((float)palEntry.peGreen/(float)255,this_bar->gamma)*255);
    			 blue_level = (char) (pow((float)palEntry.peBlue/(float)255,this_bar->gamma)*255);
			 
                   	 print_char( fp, red_level);
                   	 print_char( fp, green_level);
                    	 print_char( fp, blue_level);
                     }

                     sprintf(string,"color bar: printed row %d/%d",current_row+1,rows);
                     print_status(hDlg,string);
            	  }
                  SelectPalette( hDC, hPal_Old, 0);
                  RealizePalette( hDC);
	          break;

	       case PALTYPE_LOGICAL:
		  for( current_row = 0; current_row < rows; ++current_row )
                  {
                     for( current_col=0; current_col < cols; current_col++)
                     {
			 interp_index = (int) floor( (float)current_col * (float)PAL_IM_WIDTH / (float)cols);
			 red_level = (char) *(this_bar->print_bitmap + 3*interp_index + 2);
			 green_level = (char) *(this_bar->print_bitmap + 3*interp_index + 1);
			 blue_level = (char) *(this_bar->print_bitmap + 3*interp_index);

			 red_level = (char) (pow((float)red_level/(float)255,this_bar->gamma)*255);
    			 green_level = (char) (pow((float)green_level/(float)255,this_bar->gamma)*255);
    			 blue_level = (char) (pow((float)blue_level/(float)255,this_bar->gamma)*255);

                   	 print_char( fp, red_level);
                   	 print_char( fp, green_level);
                    	 print_char( fp, blue_level);
                     }

                     sprintf(string,"color bar: printed row %d/%d",current_row+1,rows);
                     print_status(hDlg,string);
            	  }
		  break;          
	    }

	    break;
    }

    // restore the most recently saved (see place_bar) ps graphics state
    fprintf(fp,"\ngrestore\n");
}

void print_scale_pos()
{
    PRINT_EL *this_el;
    
    if (print_list!=NULL)
    {
        this_el=print_list;
        do
        {
            if (this_el->selected)
            {
                scale_this_el(this_el,print_scale_factor);
            }
            this_el=this_el->next;
        } while(this_el!=print_list);
    }
}

static void print_moveto_pos()
{
    PRINT_EL *this_el;
    float minimum_x = MAX;
    float minimum_y = MAX;
    
    if (print_list!=NULL)
    {
        this_el=print_list;
        do
        {
            if (this_el->selected)
            {
		if( this_el->x1 < minimum_x) {
		  minimum_x = this_el->x1;
		  minimum_y = this_el->y1;
		}
            }
            this_el=this_el->next;
        } while(this_el!=print_list);
        do
        {
            if (this_el->selected)
            {
                moveto_this_el( this_el, print_moveto_x + (this_el->x1-minimum_x), 
				print_moveto_y + (this_el->y1-minimum_y));
            }
            this_el=this_el->next;
        } while(this_el!=print_list);
    }
}

static void moveto_this_el( PRINT_EL *this_el, float x, float y)
{
  this_el->x1 = x;
  this_el->y1 = y;
}

void calc_print_pal_bitmap( PRINT_EL *this_el)
{
    // if this_el is a palette (bar) print element, calculate its bitmap
    // copy the bitmap into this_el->print_bitmap

    int bitmap_size;
    int i,j;

    if( this_el->type == PRINT_TYPE_BAR) {
	  switch( this_el->bar_pal.type) {

	    case PALTYPE_MAPPED:
	      bitmap_size = PAL_IM_WIDTH * PAL_IM_HEIGHT * sizeof(char);

	      // copy the palette (dacbox) to the print element's palette
	      this_el->bar_pal.dacbox = (unsigned char *) malloc( sizeof(unsigned char)*SIZEOFMPAL);
	      memcpy( this_el->bar_pal.dacbox, this_el->print_image->pal.dacbox, SIZEOFMPAL);

	      this_el->print_bitmap_size = (int*) malloc( sizeof( int));
	      this_el->print_bitmap_size = &bitmap_size;
	      this_el->print_bitmap = malloc( *(this_el->print_bitmap_size));

	      // the pal_image is set up in stm.c; copy it to the print_bitmap
	      memcpy( this_el->print_bitmap, pal_image, bitmap_size);
	      break;

	    case PALTYPE_LOGICAL:
	      {
	        float data_size;
	        int color32;

	        data_size=this_el->print_image->max_z-this_el->print_image->min_z;
	        if (data_size<=0) data_size=1;

		// copy the logpal elements (not really necessary?)
		copy_logpal(&(this_el->bar_pal.logpal),this_el->print_image->pal.logpal);

	        bitmap_size = 3 * PAL_IM_WIDTH * PAL_IM_HEIGHT * sizeof(char);
	        this_el->print_bitmap_size = (int*) malloc( sizeof( int));
	        this_el->print_bitmap_size = &bitmap_size;
	        this_el->print_bitmap=(char *) malloc( *(this_el->print_bitmap_size));

		// fill in the print_bitmap
	        for (i = 0; i < PAL_IM_HEIGHT; i++)
	          for (j = 0; j < PAL_IM_WIDTH; j++)
	            memcpy( this_el->print_bitmap + (j + i * PAL_IM_WIDTH)*3,
	              bgr(this_el->print_image,this_el->print_image->min_z+
	                (float)(j) / (float)PAL_IM_WIDTH*data_size, &color32), 3);
	      }
	      break;
	  }
    }

}

static void repaint_image_res_controls( HWND hDlg)
{
    if( cur_obj_type(PRINT_TYPE_ANY_IMAGE)) {
        CheckDlgButton(hDlg,PRINT_IMAGE_RES_RAW,0);
    	CheckDlgButton(hDlg,PRINT_IMAGE_RES_SMOOTH,0);
        CheckDlgButton(hDlg,cur_obj->image_resolution_type,1);
    }
}

