#include <windows.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>
#include <sys\timeb.h>
#include <sys\types.h>
#include <string.h>
#include <math.h>
#include <direct.h>
//#include <toolhelp.h>
#include "common.h"
#include "stm.h"
#include "dio.h"
#include "data.h"
#include "file.h"
#include "scan.h"
#include "hop.h"
#include "pal.h"
#include "clock.h"
#include "pre_scan.h"
//#include "scan2.h"
#include "latman.h"
#include "serial.h"
#include "Spec.h"

//#define NA
#define MOH
//#define MOH2
//#define MOH3
//#define TEST
#define SCAN_TIME_TEST
extern int z_offset_crash_protect;
extern int feedback;
extern datadef *data,  // data we are currently showing in scan window
	   **glob_data,
	   *all_data[];
extern unsigned int dac_data[];
extern unsigned int out1;
extern int z_offset_ch;
extern int x_range,
           y_range,z_range,z_offset_range,
           x_offset_range,
           y_offset_range;
extern int datapro;
extern int i_set_range,sample_bias_range;
extern double i_set;
extern int track_limit_x_min,track_limit_x_max;
extern int track_limit_y_min,track_limit_y_max;
//extern int sharp_lines;
//extern int sharp_cycles;
//extern int sharp_bias1;
//extern int sharp_bias2;
extern int digital_abort;

extern char *current_file_stm;
extern HANDLE hInst;
extern char string[];
extern int image_caller;
extern struct commentdef *gcomment;
extern double a_per_v, z_a_per_v;
extern HBITMAP hCross;
extern HBITMAP hXcross;
extern LOGPALETTE *pLogPal;
extern HPALETTE hPal;
//extern unsigned char *dacbox;
extern BITMAPINFO *bitmapinfo;
extern char *pal_image;
extern char cur_rs_type[];
extern int current_step;

extern BOOL FAR PASCAL PrescanDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL Prescan2Dlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL ImageDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL DepDlg(HWND, unsigned, WPARAM, LPARAM);
//extern BOOL FAR PASCAL NewDepDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL CommentDlg(HWND, unsigned, WPARAM, LPARAM);
//extern BOOL FAR PASCAL SharpDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL SpecDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL TrackDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL HopTrackDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL LatManDlg(HWND, unsigned, WPARAM, LPARAM);	//LATMAN!

extern HWND TrackWnd;
extern BOOL TrackOn;
//extern HWND Prescan2Wnd;
//extern BOOL Prescan2On;

unsigned int scan_x,
             scan_y,
             scan_z,
             scan_fine_x,
             scan_fine_y;
float scan_x_gain;
float scan_y_gain;
float scan_z_gain;
float min_z,max_z;
int scan_now = 0;
unsigned int scan_step;
unsigned int scan_size;
unsigned int scan_scale;
int scan_freq;
unsigned int sample_bias;
unsigned int tip_gain;
unsigned int i_setpoint;
static int dither0_before;
static int dither1_before;

int scan_end_fb=1; //Shaowei Post Scan
int scan_end_auto = 0;
int scan_end_retract = 0;

int bitmap_size;
int image_orig_x,
    image_orig_y;
int screen_orig_x = -409600;
int screen_orig_y = 409600;
int screen_lines = 819200;
//char sample_type[SAMPLETYPESIZE] = "";
unsigned char *scan_dacbox;
//int mypal[PALETTESIZE];
HWND ScnDlg;
unsigned char *image_data;
unsigned char *bitmap_data;
datadef *scan_defaults_data[SCAN_NUM_DEFAULTS];
int scan_current_default = 0;
int scan_current_palette = 0;
int scan_current_data = 0; // index of current data that we are looking at (0 if only 1 image)
int scan_max_data;
int image_x_min,
    image_y_min,
    image_x_max,
    image_y_max;
unsigned int old_x_offset,
             old_y_offset,
             old_z_offset;
int scan_fine = 0;
int scan_plane_fit = 1;
int scan_display_factor = 1;
//int scan_sharpen;

static int out_smart = 0;

//Triggering Shaowei
int scan_trigger_ch;
unsigned int scan_trigger_low = 2048;//Mod. Shaowei default trigger low = 0V
unsigned int scan_trigger_high = 3072;//Mod. Shaowei default trigger high = 5V
unsigned int scan_trigger_time = 50;//Mod. Shaowei default trigger time 50ms
unsigned int scan_trigger_ch = 7;//Mod. Shaowei, default trigger channel 7
unsigned int scan_trigger_now = 2048;//Mod. Shaowei default trigger ch. at low
static void repaint_scantrigger_ch(HWND);

static void repaint_track_limits(HWND);
static void enable_clock_users(HWND,int);
static void scan_set_title(HWND);
void repaint_scan_dir(HWND);
void repaint_scan_time(HWND);
void repaint_scan_cursor(HWND);
void repaint_image_scrolls(HWND);
void calc_step_delay();
//void sharpen(HWND);
void init_bitmap_data();
static void repaint_data(HWND);
static void repaint_scan_current_data(HWND);
void enable_x_gain(HWND,int);
void enable_y_gain(HWND,int);

BOOL FAR PASCAL ScanDlg(HWND,unsigned,WORD,LONG);
// Scan Dialog. For imaging topography, vibrational intensity, or any other
// input as a function of position. Also, used to Send tip offsets over large
// separations. Many scan parameters must be set using PreScan or Prescan 2.

BOOL FAR PASCAL ScanDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{
  int i,j;
  int id;
  int /*mini,maxi,*/delt,ddelt;
  unsigned int x_gain,y_gain,z_gain,z2_gain;
  static FARPROC TrackDlgProc;
  double d,minid,maxid,deltd,ddeltd;
  float x,y,z;
  HDC hDC;
  POINT mouse_pos;
  float point1 = (float)0.1;
  int xa,ya;
  BOOL result;
//  datadef *temp_data;
  char filename[256];

  static FARPROC  lpfnDlgProc;
  static FARPROC  Prescan2DlgProc;

  switch(Message)
  {
    case WM_INITDIALOG:

#ifdef OLD  // moved to stm.c
	  for(i = 0;i < SCAN_NUM_DEFAULTS;i++) 
      {
        scan_defaults_data[i] = NULL;
        alloc_data(&(scan_defaults_data[i]),DATATYPE_3D_PARAMETERS,0,GEN2D_NONE,GEN2D_NONE,1);
        strcpy(filename,SCAN_INI);
        sprintf(filename + strlen(filename),"%d.ini",i);
        glob_data = &scan_defaults_data[i];
        if(!load_image(filename))
        {
          set_scan_defaults(scan_defaults_data[i]);
        }
      }

      for(i = 0;i < PALETTESIZE;i++)
      {
        pLogPal->palPalEntry[i].peRed = (BYTE)i;
        pLogPal->palPalEntry[i].peGreen = (BYTE)i;
        pLogPal->palPalEntry[i].peBlue = (BYTE)i;
        pLogPal->palPalEntry[i].peFlags = NULL;
      }
#endif

	  ScnDlg = hDlg;
      TrackDlgProc = MakeProcInstance(TrackDlg,hInst);
	  Prescan2DlgProc = MakeProcInstance(Prescan2Dlg,hInst);
      calc_step_delay();
      scan_fine_x = scan_fine_y = ZERO;
//      scan_dacbox = (unsigned char *)malloc(SIZEOFMPAL);

      SetScrollRange(GetDlgItem(hDlg,IMAGE_X_SCROLL),SB_CTL,0,32767,FALSE);
      SetScrollRange(GetDlgItem(hDlg,IMAGE_Y_SCROLL),SB_CTL,0,32767,FALSE);
      SetScrollRange(GetDlgItem(hDlg,SCAN_X_SCROLL),SB_CTL,scan_x_min,scan_x_max,FALSE);
      SetScrollRange(GetDlgItem(hDlg,SCAN_Y_SCROLL),SB_CTL,scan_y_min,scan_y_max,FALSE);
//      SetScrollRange(GetDlgItem(hDlg,SCAN_Z_SCROLL),SB_CTL,scan_z_min,scan_z_max,FALSE);
      SetScrollRange(GetDlgItem(hDlg,SCAN_STEP_SCROLL),SB_CTL,scan_step_min,scan_step_max,FALSE);
      SetScrollRange(GetDlgItem(hDlg,SCAN_SIZE_SCROLL),SB_CTL,scan_size_min,scan_size_max,FALSE);
//      SetScrollRange(GetDlgItem(hDlg,SCAN_NUM_SCROLL),SB_CTL,scan_num_min,scan_num_max,FALSE);
      SetScrollRange(GetDlgItem(hDlg,SCAN_CURRENT_DATA_SCROLL),SB_CTL,0,scan_max_data-1,FALSE);
      CheckDlgButton(hDlg,SCAN_FINE,scan_fine);
      CheckDlgButton(hDlg,SCAN_PLANE_FIT,scan_plane_fit);
      CheckDlgButton(hDlg,SCAN_1,0);
      CheckDlgButton(hDlg,SCAN_2,0);
      CheckDlgButton(hDlg,SCAN_3,0);
      CheckDlgButton(hDlg,SCAN_4,0);

// datapro mode Mod. Shaowei
	  if(datapro)
	  {	  
	  EnableWindow(GetDlgItem(hDlg,SCAN_START),FALSE);
	  EnableWindow(GetDlgItem(hDlg,SCAN_OFFSET_AUTO_BELOW),FALSE);
	  EnableWindow(GetDlgItem(hDlg,SEND_X_Y_OFFSETS),FALSE);
	  EnableWindow(GetDlgItem(hDlg,PRESCAN_BUTTON),FALSE);
	  EnableWindow(GetDlgItem(hDlg,SCAN_PRESCAN2),FALSE);
      EnableWindow(GetDlgItem(hDlg,SCAN_TRACK),FALSE);
	  EnableWindow(GetDlgItem(hDlg,SCAN_HOP),FALSE);
	  EnableWindow(GetDlgItem(hDlg,DEPOSITION),FALSE);
	  EnableWindow(GetDlgItem(hDlg,SCAN_TRIGGER_HIGH),FALSE);
	  EnableWindow(GetDlgItem(hDlg,SCAN_TRIGGER_LOW),FALSE);
	  EnableWindow(GetDlgItem(hDlg,SCAN_TRIGGER_PULSE),FALSE);
	  }



      switch(scan_current_default)
      {
        case 0:
          CheckDlgButton(hDlg,SCAN_1,1);
          break;
        case 1:
          CheckDlgButton(hDlg,SCAN_2,1);
          break;
        case 2:
          CheckDlgButton(hDlg,SCAN_3,1);
          break;
        case 3:
          CheckDlgButton(hDlg,SCAN_4,1);
          break;
      }
		//palette initialization
	  CheckDlgButton(hDlg,SCAN_R,0);	//rainbow
      CheckDlgButton(hDlg,SCAN_F,0);	//flipped
      CheckDlgButton(hDlg,SCAN_G,0);	//gray
      switch(scan_current_palette)
      {
        case 0:
          CheckDlgButton(hDlg,SCAN_R,1);
          break;
        case 1:
          CheckDlgButton(hDlg,SCAN_F,1);
          break;
        case 2:
          CheckDlgButton(hDlg,SCAN_G,1);
          break;
      }

      if(scan_x_gain == point1)
        CheckDlgButton(hDlg,SCAN_X_GAIN_TENTH,1);
      else if(scan_x_gain == 1)
        CheckDlgButton(hDlg,SCAN_X_GAIN_ONE,1);
      else if(scan_x_gain == 10)
        CheckDlgButton(hDlg,SCAN_X_GAIN_TEN,1);
      
	  if(scan_y_gain == point1)
        CheckDlgButton(hDlg,SCAN_Y_GAIN_TENTH,1);
      else if(scan_y_gain == 1)
        CheckDlgButton(hDlg,SCAN_Y_GAIN_ONE,1);
      else if(scan_y_gain == 10)
        CheckDlgButton(hDlg,SCAN_Y_GAIN_TEN,1);
        
	  SetFocus(GetDlgItem(hDlg,SCAN_EXIT));

      hDC = GetDC(hDlg);
      set_Plasma_palette(scan_dacbox);
      SelectPalette(hDC,hPal,0);
      RealizePalette(hDC);

//      hImage = CreateBitmap(1,1,1,8,image_data);
      image_data = (unsigned char *)malloc(IMAGESIZE * IMAGESIZE * sizeof(unsigned char));
      bitmap_data = NULL;     /* Resets pointer */

      init_bitmap_data();
//      UpdateBitmap(hDlg,0,IMAGESIZE,0,IMAGESIZE);
      repaint_image_scrolls(hDlg);
      repaint_scan_x(hDlg);
      repaint_scan_y(hDlg);
//      repaint_scan_z(hDlg);
      repaint_scan_step(hDlg);
      repaint_scan_size(hDlg);
//      repaint_scan_num(hDlg);
//      repaint_scan_freq(hDlg);
      repaint_scan_bitmap(hDlg);
//      repaint_sample_bias(hDlg);
      repaint_scan_dir(hDlg);
      repaint_pal(hDlg,PAL_X_POS,PAL_Y_POS,scan_dacbox,NULL);
      repaint_scan_time(hDlg);
      repaint_scan_current_data(hDlg);
	  repaint_scantrigger_ch(hDlg);//Triggering
      ReleaseDC(hDlg,hDC);
      break;
    case WM_PAINT:
      repaint_scan_bitmap(hDlg);
      repaint_pal(hDlg,PAL_X_POS,PAL_Y_POS,scan_dacbox,NULL);
	  repaint_scan_x(hDlg);
	  repaint_scan_y(hDlg);
      break;

    case WM_LBUTTONDOWN:
	  // left mouse button chooses x and y offset (but does not SEND)
      mouse_pos.x = LOWORD(lParam);
      mouse_pos.y = HIWORD(lParam);
      mouse_pos.x -= IMAGE_X_POS;
      mouse_pos.y -= IMAGE_Y_POS;
      if(mouse_pos.x >= 0 && mouse_pos.x < IMAGESIZE &&
            mouse_pos.y >= 0 && mouse_pos.y < IMAGESIZE)
      {
        x = (float)mouse_pos.x * (float)screen_lines / (float)IMAGESIZE + screen_orig_x;
        x = x / (float)100.0 / (float)x_offset_range + 2048;
        scan_x = (unsigned int)(x + 0.5);
        y = -(float)mouse_pos.y * (float)screen_lines / (float)IMAGESIZE + screen_orig_y;
        y = y / (float)100.0 / (float)y_offset_range + 2048;
        scan_y = (unsigned int)(y + 0.5);
        repaint_scan_bitmap(hDlg);
      }
      repaint_scan_x(hDlg);
      repaint_scan_y(hDlg);
      break;

    case WM_RBUTTONDOWN:
	  // left mouse button chooses x and y offset (but does not SEND)
      mouse_pos.x = LOWORD(lParam);
      mouse_pos.y = HIWORD(lParam);
      mouse_pos.x -= IMAGE_X_POS;
      mouse_pos.y -= IMAGE_Y_POS;
      if(mouse_pos.x >= 0 && mouse_pos.x < IMAGESIZE &&
            mouse_pos.y >= 0 && mouse_pos.y < IMAGESIZE)
      {
        repaint_scan_cursor(hDlg);

        x = (float)mouse_pos.x * (float)screen_lines / (float)IMAGESIZE + screen_orig_x;
        x -= (float)((float)scan_x - ZERO) * (float)100.0 * (float)x_offset_range;
        x /= (float)10.0 * (float)x_range * (float)scan_x_gain;
        x += ZERO;
        if(x < 0) x = 0;
        if(x > MAX) x = MAX;
        scan_fine_x = (unsigned int)(x + 0.5);
        y = -(float)mouse_pos.y * (float) screen_lines / (float)IMAGESIZE + screen_orig_y;
        y -= (float)((float)scan_y - ZERO) * (float)100.0 * (float)y_offset_range;
        y /= (float)10.0 * (float)y_range * (float)scan_y_gain;
        y += ZERO;
        if(y < 0) y = 0;
        if(y > MAX) y = MAX;
        scan_fine_y = (unsigned int)(y + 0.5);
        repaint_scan_cursor(hDlg);
      }
      repaint_scan_x(hDlg);
      repaint_scan_y(hDlg);
      break;

    case WM_HSCROLL:
    case WM_VSCROLL:
      if(!out_smart)
      {
            
        id = getscrollid();
        switch(id)
        {
          case IMAGE_X_SCROLL:
            j = screen_orig_x;
            d = (double)screen_orig_x;
            deltd = (double)screen_lines / 10.0;
            if(deltd == 0) deltd = 1;
            ddeltd = (double)screen_lines / 2.0;
            if(ddeltd == 0) ddeltd = 1;
            minid = image_x_min;
            maxid = image_x_max;
            
/*
            i = j = screen_orig_x;
            delt = screen_lines / 10;
            if(!delt)
                delt = 1;
            ddelt = screen_lines / 2;
            if(!ddelt)
                ddelt = 1;
*/
            break;
          case IMAGE_Y_SCROLL:
            j = screen_orig_y;
            d = (double)screen_orig_y;
            deltd = -(double)screen_lines / 10.0;
            if(deltd == 0) deltd = -1;
            ddeltd =- (double)screen_lines / 2.0;
            if(ddeltd == 0) ddeltd = -1;
            minid = image_y_min;
            maxid = image_y_max;
/*
            i = j = screen_orig_y;
            delt = -screen_lines / 10;
            if(!delt)
                delt = -1;
            ddelt = -screen_lines / 2;
            if(!ddelt)
                ddelt = -1;
*/
            break;
          case SCAN_X_SCROLL:
            if(scan_fine) i = (int)scan_fine_x;
            else i = (int)scan_x;
            delt = scan_x_delt;
            ddelt = scan_x_ddelt;
            break;
          case SCAN_Y_SCROLL:
            if(scan_fine) i= (int)scan_fine_y;
            else i = (int)scan_y;
            delt = -scan_y_delt;
            ddelt = -scan_y_ddelt;
            break;
          case SCAN_STEP_SCROLL:
            i = (int)scan_step;
            delt = scan_step_delt;
            ddelt = scan_step_ddelt;
            break;
          case SCAN_SIZE_SCROLL:
            i = (int)scan_size;
            delt = scan_size_delt;
            ddelt = scan_size_ddelt;
            break;
          case SCAN_CURRENT_DATA_SCROLL:
            i = (int)scan_current_data;
            delt = SCAN_CURRENT_DATA_DELT;
            ddelt = SCAN_CURRENT_DATA_DDELT;
            break;

        
/*
          case SCAN_Z_SCROLL:
            i = (int)scan_z;
            delt = scan_z_delt;
            ddelt = scan_z_ddelt;
            break;
*/
/*
          case SCAN_NUM_SCROLL:
            i = (int) scan_num;
            delt = scan_num_delt;
            ddelt = scan_num_ddelt;
            break;
*/
		}
        switch(getscrollcode())
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
            if(id == SCAN_Y_SCROLL){
                d = scan_y_max - d;
                i = scan_y_max - getscrollpos(); //added by joon  11/04/2003
			}
            break;
          case SB_THUMBTRACK:
            i = getscrollpos();
            d = ITOD(getscrollpos(),minid,maxid);
            if(id == SCAN_Y_SCROLL){
                d = scan_y_max - d;
			    i = scan_y_max - getscrollpos(); //added by joon  11/04/2003
			}
            break;
          default:
            id = -1;
            break;
        }
//        i = min(max(i,mini),maxi);
        d = min(max(d,minid),maxid);
        switch (id)
        {
          case IMAGE_X_SCROLL:
            screen_orig_x = (int)(floor(d + 0.5));
            repaint_image_scrolls(hDlg);
            if(screen_orig_x != j)
            {
//              UpdateBitmap(hDlg,0,IMAGESIZE,0,IMAGESIZE);
              repaint_scan_bitmap(hDlg);
            }
            break;
          case IMAGE_Y_SCROLL:
            screen_orig_y = (int)(floor(d + 0.5));
            repaint_image_scrolls(hDlg);
            if(screen_orig_y != j)
            {
//              UpdateBitmap(hDlg,0,IMAGESIZE,0,IMAGESIZE);
              repaint_scan_bitmap(hDlg);
            }
            break;
          case SCAN_X_SCROLL:
            if(scan_fine) 
            {
              repaint_scan_cursor(hDlg);
              scan_fine_x = (unsigned int)i;
              repaint_scan_cursor(hDlg);
            }
            else 
            {
              scan_x = (unsigned int)i;
              repaint_scan_bitmap(hDlg);
            }
            repaint_scan_x(hDlg);
            break;
          case SCAN_Y_SCROLL:
            if(scan_fine) 
            {
              repaint_scan_cursor(hDlg);
              scan_fine_y = (unsigned int)i;
              repaint_scan_cursor(hDlg);
            }
            else
            {
              scan_y = (unsigned int)i;
              repaint_scan_bitmap(hDlg);
            }
            repaint_scan_y(hDlg);
            break;
          case SCAN_STEP_SCROLL:
            scan_step = (unsigned int)i;
            repaint_scan_step(hDlg);
            repaint_scan_size(hDlg);
            repaint_scan_bitmap(hDlg);
            break;
          case SCAN_SIZE_SCROLL:
            scan_size = (unsigned int)i;
            repaint_scan_size(hDlg);
            repaint_scan_bitmap(hDlg);
            break;
          case SCAN_CURRENT_DATA_SCROLL:
            scan_current_data = min(max(i,0),scan_max_data - 1);
            data = all_data[scan_current_data];
//            if(scan_current_data != i) 
            {
              repaint_scan_current_data(hDlg);
            }
            repaint_data(hDlg);
            break;
/*
          case SCAN_Z_SCROLL:
            scan_z = (unsigned int)i;
            repaint_scan_z(hDlg);
            break;
*/
/*        case SCAN_NUM_SCROLL:
            scan_num = (unsigned int) i;
            repaint_scan_num(hDlg);
            break;
*/
		}
      }
      break;
    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
	    //opening other dialogs
        case PRESCAN_BUTTON:
          lpfnDlgProc = MakeProcInstance(PrescanDlg,hInst);
          DialogBox(hInst,"PRESCANDLG",hDlg,lpfnDlgProc);
          FreeProcInstance(lpfnDlgProc);
          if(scan_now) SendMessage(hDlg,WM_COMMAND,SCAN_START,0);
          else repaint_scan_time(hDlg);
          scan_now = 0;
          break;
        case SCAN_PRESCAN2:
/*
      	  if(!Prescan2On)
      	  {
            Prescan2Wnd = CreateDialog(hInst,"PRESCAN2DLG",hDlg,Prescan2DlgProc);
          	Prescan2On = TRUE;
      	  }
*/
          lpfnDlgProc = MakeProcInstance(Prescan2Dlg,hInst);
          DialogBox(hInst,"PRESCAN2DLG",hDlg,lpfnDlgProc);
          FreeProcInstance(lpfnDlgProc);
          repaint_scan_time(hDlg);
          scan_now = 0;
           break;
	    case SCAN_TRACK:
          if(!TrackOn)
          {     
            TrackWnd = CreateDialog(hInst,"TRACKDLG",hDlg,TrackDlgProc);
            TrackOn = TRUE;
          }
          break;
        case SCAN_HOP:
          lpfnDlgProc = MakeProcInstance(HopTrackDlg,hInst);
          DialogBox(hInst,"HOPDLG",hDlg,lpfnDlgProc);
          FreeProcInstance(lpfnDlgProc);
          break;
        case SCAN_LATMAN:
          lpfnDlgProc = MakeProcInstance(LatManDlg,hInst);
          DialogBox(hInst,"LATMANDLG",hDlg,lpfnDlgProc);
          FreeProcInstance(lpfnDlgProc);
          break;
        case SCAN_SPEC:
          lpfnDlgProc = MakeProcInstance(SpecDlg,hInst);
          DialogBox(hInst,"SPECDLG",hDlg,lpfnDlgProc);
          FreeProcInstance(lpfnDlgProc);
          scan_fine_x = dac_data[x_ch];
          scan_fine_y = dac_data[y_ch];
          repaint_scan_x(hDlg);
          repaint_scan_y(hDlg);            
          break;
        case IMAGE_MANIPULATION:
          image_caller = IMAGE_CALLER_SCAN;
          lpfnDlgProc = MakeProcInstance(ImageDlg,hInst);
          DialogBox(hInst,"IMAGEDLG",hDlg,lpfnDlgProc);
          FreeProcInstance(lpfnDlgProc);
//          dacbox = scan_dacbox;
          spindac(0,1,scan_dacbox);
          break;
        case DEPOSITION:
          lpfnDlgProc = MakeProcInstance(DepDlg,hInst);
          DialogBox(hInst,"DEPDLG",hDlg,lpfnDlgProc);
		  //lpfnDlgProc = MakeProcInstance(NewDepDlg,hInst);
          //DialogBox(hInst,"NEWDEPDLG",hDlg,lpfnDlgProc);
          FreeProcInstance(lpfnDlgProc);
//          dacbox = scan_dacbox;
          spindac(0,1,scan_dacbox);
          break;
        case SCAN_COMMENT:
            gcomment=&(data->comment);
            lpfnDlgProc = MakeProcInstance(CommentDlg, hInst);
            DialogBox(hInst, "COMMENTDLG", hDlg, lpfnDlgProc);
            FreeProcInstance(lpfnDlgProc);                  
/*
            for(i = 0;i < scan_max_data;i++)
            {
              if(!all_data[i]->saved && all_data[i]->valid && i != scan_current_data)
              {
                strcpy(all_data[i]->comment.ptr,data->comment.ptr);
                all_data[i]->comment.size=data->comment.size;
              }
            }
*/
            break;
/*
        case SCAN_SHARPENB:
          lpfnDlgProc = MakeProcInstance(SharpDlg,hInst);
          DialogBox(hInst,"SHARPDLG",hDlg,lpfnDlgProc);
          FreeProcInstance(lpfnDlgProc);
          break;
        case SCAN_SHARPEN:
          scan_sharpen = IsDlgButtonChecked(hDlg,SCAN_SHARPEN);
          break;
*/

	    //messages used by other dialogs to update scan dialog
	    case SCAN_TRACKING_START:
          enable_clock_users(hDlg,0);
          break;
        case SCAN_TRACKING_STOP:
          enable_clock_users(hDlg,1);
          break;
	    case SCAN_REPAINT_SIZES:
	      repaint_scan_size(hDlg);
	      repaint_scan_step(hDlg);
	      break;
        case SCAN_REPAINT_STATUS:
          scan_status(hDlg,string,-1);
          break;
        case SCAN_REPAINT_CURSOR:
          repaint_scan_cursor(hDlg);
          scan_fine_x = dac_data[x_ch];
          scan_fine_y = dac_data[y_ch];
          repaint_scan_cursor(hDlg);
          if(scan_fine)
          {
            repaint_scan_x(hDlg);
            repaint_scan_y(hDlg);
          }
          break;
        case SCAN_REPAINT_TRACK_LIMITS:
          repaint_track_limits(hDlg);
          break;
        case SCAN_REPAINT_FINE_POS:
          repaint_scan_cursor(hDlg);
          scan_fine_x = dac_data[x_ch];
          scan_fine_y = dac_data[y_ch];
          repaint_scan_x(hDlg);
          repaint_scan_y(hDlg);            
          repaint_scan_cursor(hDlg);
          break;

	    // scan geometry
        case SCAN_STEP_BITS_EDIT:
          GetDlgItemText(hDlg,SCAN_STEP_BITS_EDIT,string,9);
          i = atoi(string);
          scan_step = (unsigned int)(i + 0.5);
          break;
        case SCAN_SIZE_STEPS_EDIT:
          GetDlgItemText(hDlg,SCAN_SIZE_STEPS_EDIT,string,9);
          i = atoi(string);
          scan_size = (unsigned int)(i + 0.5);
          break;
        case SCAN_SIZE_64:
          if(IsDlgButtonChecked(hDlg,SCAN_SIZE_64))
          {
            scan_size = 64;
            repaint_scan_size(hDlg);
            repaint_scan_bitmap(hDlg);
          }
          break;
        case SCAN_SIZE_128:
          if(IsDlgButtonChecked(hDlg,SCAN_SIZE_128))
          {
            scan_size = 128;
            repaint_scan_size(hDlg);
            repaint_scan_bitmap(hDlg);
          }
          break;
        case SCAN_SIZE_256:
          if(IsDlgButtonChecked(hDlg,SCAN_SIZE_256))
          {
            scan_size = 256;
            repaint_scan_size(hDlg);
            repaint_scan_bitmap(hDlg);
          }
          break;
        case SCAN_SIZE_512:
          if(IsDlgButtonChecked(hDlg,SCAN_SIZE_512))
          {
            scan_size = 512;
            repaint_scan_size(hDlg);
            repaint_scan_bitmap(hDlg);
          }
          break;
        case READ_FORWARD:
          if(IsDlgButtonChecked(hDlg,READ_FORWARD))
          {
            SD->scan_dir = FORWARD_DIR;
            repaint_scan_dir(hDlg);
          }
          break;
        case READ_BACKWARD:
          if(IsDlgButtonChecked(hDlg,READ_BACKWARD))
          {
            SD->scan_dir = BACKWARD_DIR;
            repaint_scan_dir(hDlg);
          }
          break;
        case READ_1_IMAGE:
          if(IsDlgButtonChecked(hDlg,READ_1_IMAGE))
          {
            SD->scan_dir = BOTH_DIR1;
            repaint_scan_dir(hDlg);
          }
          break;
        case READ_2_IMAGES:
          if(IsDlgButtonChecked(hDlg,READ_2_IMAGES))
          {
            SD->scan_dir = BOTH_DIR2;
            repaint_scan_dir(hDlg);
          }
          break;
        case READ_2_POLAR_IMAGES:
          if(IsDlgButtonChecked(hDlg,READ_2_POLAR_IMAGES))
          {
            SD->scan_dir = BOTH_DIR2_POLAR;
            repaint_scan_dir(hDlg);
          }
          break;
        case X_FIRST:
          if(IsDlgButtonChecked(hDlg,X_FIRST))
          {
            SD->x_first = 1;
            repaint_scan_dir(hDlg);
			}
          break;
        case Y_FIRST:
          if(IsDlgButtonChecked(hDlg,Y_FIRST))
          {
            SD->x_first = 0;
            repaint_scan_dir(hDlg);
			}
          break;
		  
		// default scan parameters
        case SCAN_1:
          if(scan_current_default != 0)
          {
            strcpy(filename,SCAN_INI);
            sprintf(filename+strlen(filename),"%d.ini",scan_current_default);
            glob_data = &scan_defaults_data[scan_current_default];
            save_image(filename,0);
            scan_current_default = 0;
	        current_step = 0;
            strcpy(cur_rs_type,SD->sequence[0].step_type_string);
            repaint_scan_dir(hDlg);
            repaint_scan_time(hDlg);
          }
          break;
        case SCAN_2:
          if(scan_current_default != 1)
          {
            strcpy(filename,SCAN_INI);
            sprintf(filename+strlen(filename),"%d.ini",scan_current_default);
            glob_data = &scan_defaults_data[scan_current_default];
            save_image(filename,0);
            scan_current_default = 1;
	        current_step = 0;
            strcpy(cur_rs_type,SD->sequence[0].step_type_string);
            repaint_scan_dir(hDlg);
            repaint_scan_time(hDlg);
          }
          break;
        case SCAN_3:
          if(scan_current_default != 2)
          {
            strcpy(filename,SCAN_INI);
            sprintf(filename+strlen(filename),"%d.ini",scan_current_default);
            glob_data = &scan_defaults_data[scan_current_default];
            save_image(filename,0);
            scan_current_default = 2;
	        current_step = 0;
            strcpy(cur_rs_type,SD->sequence[0].step_type_string);
            repaint_scan_dir(hDlg);
            repaint_scan_time(hDlg);
          }
          break;
        case SCAN_4:
          if(scan_current_default != 3)
          {
            strcpy(filename,SCAN_INI);
            sprintf(filename+strlen(filename),"%d.ini",scan_current_default);
            glob_data = &scan_defaults_data[scan_current_default];
            save_image(filename,0);
            scan_current_default = 3;
	        current_step = 0;
            strcpy(cur_rs_type,SD->sequence[0].step_type_string);
            repaint_scan_dir(hDlg);
            repaint_scan_time(hDlg);
          }
          break;
		//palette choice
		case SCAN_R:
          if(scan_current_palette != 0)
          {
            scan_current_palette = 0;
			set_Plasma_palette(scan_dacbox);
			repaint_scan_bitmap(hDlg);
			repaint_pal(hDlg,PAL_X_POS,PAL_Y_POS,scan_dacbox,NULL);
          }
          break;
        case SCAN_F:
          if(scan_current_palette != 1)
          {
             scan_current_palette = 1;
			 set_Plasma_palette(scan_dacbox);
			 repaint_scan_bitmap(hDlg);
			 repaint_pal(hDlg,PAL_X_POS,PAL_Y_POS,scan_dacbox,NULL);
          }
          break;
        case SCAN_G:
          if(scan_current_palette != 2)
          {
             scan_current_palette = 2;
			 set_Plasma_palette(scan_dacbox);
			 repaint_scan_bitmap(hDlg);
			 repaint_pal(hDlg,PAL_X_POS,PAL_Y_POS,scan_dacbox,NULL);
          }
          break;

        case SCAN_SAVE_PARAMETERS:
          glob_data = &scan_defaults_data[scan_current_default];
          file_save_as(hDlg, hInst,FTYPE_STP);
          break;
        case SCAN_LOAD_PARAMETERS:
		  if(scan_current_default == 0)
          {
            sprintf(string,"You are overwriting sequence #1, continue?");
             if(MessageBox(hDlg,string, 
                    "Warning",MB_OKCANCEL | MB_ICONEXCLAMATION) == IDCANCEL)
            break;
		  }
          glob_data = &scan_defaults_data[scan_current_default];
          file_open(hDlg,hInst,FTYPE_STP,0,"*.stp");
          break;

		//changing x,y output voltages
        case SCAN_X_BITS_EDIT:
          if(!out_smart)
          {
            GetDlgItemText(hDlg,SCAN_X_BITS_EDIT,string,5);
            i = atoi(string);
            scan_x = (unsigned int)i;
          }
          break;
        case SCAN_X_VOLTS_EDIT:
          if(!out_smart)
          {
            GetDlgItemText(hDlg,SCAN_X_VOLTS_EDIT,string,9);
            i = vtod(atof(string) / 10,x_offset_range);
            scan_x = (unsigned int)i;
          }
          break;
        case SCAN_X_ANGS_EDIT:
          if(!out_smart)
          {
            GetDlgItemText(hDlg,SCAN_X_ANGS_EDIT,string,9);
            i = (int)vtod((atof(string) / (double)A_PER_V + 0.5) / 10,x_offset_range);
            scan_x = (unsigned int)(i + 0.5);
          }
          break;
        case SCAN_Y_BITS_EDIT:
          if(!out_smart)
          {
            GetDlgItemText(hDlg,SCAN_Y_BITS_EDIT,string,5);
            i = atoi(string);
            scan_y = (unsigned int)(i + 0.5);
          }
          break;
        case SCAN_Y_VOLTS_EDIT:
          if(!out_smart)
          {
            GetDlgItemText(hDlg,SCAN_Y_VOLTS_EDIT,string,9);
            i = vtod(atof(string) / 10,y_offset_range);
            scan_y = (unsigned int)(i + 0.5);
		  }
          break;
        case SCAN_Y_ANGS_EDIT:
          if(!out_smart)
          {
            GetDlgItemText(hDlg,SCAN_Y_ANGS_EDIT,string,9);
            i = (int) vtod((atof(string) / (double) A_PER_V + 0.5) / 10,y_offset_range);
            scan_y = (unsigned int)(i + 0.5);
          }
          break;
        case SCAN_X_ZERO:
          if(scan_fine) scan_fine_x = ZERO;
          else scan_x = ZERO;
          repaint_scan_x(hDlg);
          repaint_scan_bitmap(hDlg);
          break;
        case SCAN_Y_ZERO:
          if(scan_fine) scan_fine_y = ZERO;
          else scan_y = ZERO;
          repaint_scan_y(hDlg);
          repaint_scan_bitmap(hDlg);
          break;
	    case SCAN_FINE:
          scan_fine = IsDlgButtonChecked(hDlg,SCAN_FINE);
          repaint_scan_x(hDlg);
          repaint_scan_y(hDlg);
          repaint_scan_bitmap(hDlg);
          break;
        case SEND_X_Y_OFFSETS:
		  //send_x_y_old();
		  // Offsets
			
		  i = 10 / scan_x_gain * SD->inter_step_delay * SCAN_OFFSET_FUDGE * (
                abs(scan_x - dac_data[x_offset_ch])) + 10 / scan_y_gain *
                SD->inter_step_delay * SCAN_OFFSET_FUDGE *
                (abs(scan_y-dac_data[y_offset_ch]));
		  if(i > SCAN_WARN_TIME)
		  {           
		    sprintf(string,"It will take %d seconds to move!\nDo it anyway?",
                    i / 1000000);
			i = (MessageBox(hDlg,string, 
                    "Warning",MB_YESNO | MB_ICONEXCLAMATION) == IDNO);
		  }
          else i = 0;
		  if(i) break;
          enable_clock_users(hDlg,0);              
		  dac_data[x_offset_ch] = move_to_protect_serial(x_offset_ch,
                dac_data[x_offset_ch],
                scan_x,(int)(10 / scan_x_gain * SD->inter_step_delay * SCAN_OFFSET_FUDGE),
                SD->crash_protection,SD->z_limit_percent);
		  if(dac_data[x_offset_ch] != scan_x)
		  {
		    scan_x = dac_data[x_offset_ch];
			repaint_scan_x(hDlg);
			repaint_scan_bitmap(hDlg);
			i = (MessageBox(hDlg,"Crash Protection activated while moving X.\nContinue with Y move?", 
                    "Warning",MB_YESNO | MB_ICONEXCLAMATION) == IDYES);
		  }
		  else i = 1;
		  if(i)
		  {
			dac_data[y_offset_ch] = move_to_protect_serial(y_offset_ch,
                    dac_data[y_offset_ch],scan_y,
                    (int)(10 / scan_y_gain) * SD->inter_step_delay * SCAN_OFFSET_FUDGE,
                    SD->crash_protection,SD->z_limit_percent);
			if(dac_data[y_offset_ch] != scan_y)
			{
              scan_y = dac_data[y_offset_ch];
              repaint_scan_y(hDlg);
              repaint_scan_bitmap(hDlg);
              MessageBox(hDlg, "Crash Protection activated while moving Y. No fine movement!", 
                 "Warning", MB_OKCANCEL | MB_ICONEXCLAMATION);
			  enable_clock_users(hDlg,1);
              break;
                    
			}
		  }

		  // Fine coordinates
		  i = SD->inter_step_delay * SCAN_OFFSET_FUDGE * (
                abs(scan_fine_x - dac_data[x_ch])) +
                SD->inter_step_delay * SCAN_OFFSET_FUDGE * 
                (abs(scan_fine_y - dac_data[y_ch]));
		  if(i > SCAN_WARN_TIME)
		  {
			sprintf(string,"It will take %d seconds to move!\nDo it anyway?",
                     i / 1000000);
			i = (MessageBox(hDlg,string, 
                    "Warning",MB_YESNO | MB_ICONEXCLAMATION) == IDNO);
		  }
		  else i = 0;
		  if(i) 
		  {enable_clock_users(hDlg,1);
			  break;
		  }
                        
		  dac_data[x_ch] = move_to_protect_serial(x_ch,
                dac_data[x_ch],
                scan_fine_x,SD->inter_step_delay * SCAN_OFFSET_FUDGE,
                SD->crash_protection,SD->z_limit_percent);
		  if(dac_data[x_ch] != scan_fine_x)
		  {
			scan_fine_x = dac_data[x_ch];
			repaint_scan_x(hDlg);
			repaint_scan_bitmap(hDlg);
			i = (MessageBox(hDlg,"Crash Protection activated while moving Fine X.\nContinue with Y move?", 
                    "Warning", MB_YESNO | MB_ICONEXCLAMATION) == IDYES);
		  }
		  else i = 1;
		  if(i)
		  {
		    dac_data[y_ch] = move_to_protect_serial(y_ch,
                    dac_data[y_ch],scan_fine_y,
                    SD->inter_step_delay * SCAN_OFFSET_FUDGE,
                    SD->crash_protection,SD->z_limit_percent);
		    if(dac_data[y_ch] != scan_fine_y)
			{
		      scan_fine_y = dac_data[y_ch];
			  repaint_scan_y(hDlg);
			  repaint_scan_bitmap(hDlg);
			  MessageBox(hDlg, "Crash Protection activated while moving Fine Y.", 
                        "Warning", MB_OKCANCEL | MB_ICONEXCLAMATION);
			}
		  }
		  scan_z = dac_data[z_offset_ch];
		  repaint_scan_x(hDlg);
		  repaint_scan_y(hDlg);
          enable_clock_users(hDlg,1);
		  break;

        // gain settings
        case SCAN_X_GAIN_TENTH:
          if(IsDlgButtonChecked(hDlg,SCAN_X_GAIN_TENTH))
          {
            scan_x_gain = point1;
            repaint_scan_x(hDlg);
            repaint_scan_bitmap(hDlg);
            repaint_scan_step(hDlg);
            repaint_scan_size(hDlg);
            calc_gains(&x_gain,&y_gain,&z_gain,&z2_gain);
            //set_gain(x_gain,y_gain,z_gain,z2_gain);
            set_gain_serial(x_gain,y_gain,z_gain,z2_gain);
          }
          break;
        case SCAN_X_GAIN_ONE:
          if(IsDlgButtonChecked(hDlg,SCAN_X_GAIN_ONE))
          {
            scan_x_gain = 1;
            repaint_scan_x(hDlg);
            repaint_scan_bitmap(hDlg);
            repaint_scan_step(hDlg);
            repaint_scan_size(hDlg);
            calc_gains(&x_gain,&y_gain,&z_gain,&z2_gain);
            //set_gain(x_gain,y_gain,z_gain,z2_gain);
            set_gain_serial(x_gain,y_gain,z_gain,z2_gain);
          }
          break;
        case SCAN_X_GAIN_TEN:
          if(IsDlgButtonChecked(hDlg,SCAN_X_GAIN_TEN))
          {
            scan_x_gain = 10;
            repaint_scan_x(hDlg);
            repaint_scan_bitmap(hDlg);
            repaint_scan_step(hDlg);
            repaint_scan_size(hDlg);
            calc_gains(&x_gain,&y_gain,&z_gain,&z2_gain);
            //set_gain(x_gain,y_gain,z_gain,z2_gain);
            set_gain_serial(x_gain,y_gain,z_gain,z2_gain);
          }
          break;
        case SCAN_Y_GAIN_TENTH:
          if(IsDlgButtonChecked(hDlg,SCAN_Y_GAIN_TENTH))
          {
            scan_y_gain = point1;
            repaint_scan_y(hDlg);
            repaint_scan_bitmap(hDlg);
            repaint_scan_step(hDlg);
            repaint_scan_size(hDlg);
            calc_gains(&x_gain,&y_gain,&z_gain,&z2_gain);
            //set_gain(x_gain,y_gain,z_gain,z2_gain);
            set_gain_serial(x_gain,y_gain,z_gain,z2_gain);
          }
          break;
        case SCAN_Y_GAIN_ONE:
          if(IsDlgButtonChecked(hDlg,SCAN_Y_GAIN_ONE))
          {
            scan_y_gain = 1;
            repaint_scan_y(hDlg);
            repaint_scan_bitmap(hDlg);
            repaint_scan_step(hDlg);
            repaint_scan_size(hDlg);
            calc_gains(&x_gain,&y_gain,&z_gain,&z2_gain);
            //set_gain(x_gain,y_gain,z_gain,z2_gain);
            set_gain_serial(x_gain,y_gain,z_gain,z2_gain);
          }
          break;
        case SCAN_Y_GAIN_TEN:
          if(IsDlgButtonChecked(hDlg,SCAN_Y_GAIN_TEN))
          {
            scan_y_gain = 10;
            repaint_scan_y(hDlg);
            repaint_scan_bitmap(hDlg);
            repaint_scan_step(hDlg);
            repaint_scan_size(hDlg);
            calc_gains(&x_gain,&y_gain,&z_gain,&z2_gain);
            //set_gain(x_gain,y_gain,z_gain,z2_gain);
            set_gain_serial(x_gain,y_gain,z_gain,z2_gain);
          }
          break;

		// on-screen image scaling and centering
        case SCAN_ZOOM_IN:
          i = screen_lines;
          xa = (scan_x - 2048) * 100 * x_offset_range;
          ya = (scan_y - 2048) * 100 * y_offset_range;
          screen_orig_x = xa - screen_lines / 2;
          screen_orig_y = ya + screen_lines / 2;
          screen_lines = screen_lines / 2;
          if(screen_lines < 2)
            screen_lines = 2;
          screen_orig_x += (i - screen_lines) / 2;
          screen_orig_y -= (i - screen_lines) / 2;
          repaint_image_scrolls(hDlg);
//          UpdateBitmap(hDlg,0,IMAGESIZE,0,IMAGESIZE);
          repaint_scan_bitmap(hDlg);
          break;
        case SCAN_ZOOM_OUT:
          i = screen_lines;
          xa = (scan_x - 2048) * 100 * x_offset_range;
          ya = (scan_y - 2048) * 100 * y_offset_range;
          screen_orig_x = xa - screen_lines / 2;
          screen_orig_y = ya + screen_lines / 2;
          screen_lines = screen_lines * 2;
          if(screen_lines > 819200)
            screen_lines = 819200;
          screen_orig_x += (i - screen_lines) / 2;
          screen_orig_y -= (i - screen_lines) / 2;
          repaint_image_scrolls(hDlg);
//          UpdateBitmap(hDlg, 0, IMAGESIZE, 0, IMAGESIZE);
          repaint_scan_bitmap(hDlg);
          break;
        case SCAN_CENTER:
          xa = (scan_x - 2048) * 100 * x_offset_range;
          ya = (scan_y - 2048) * 100 * y_offset_range;
          screen_orig_x = (int) ((float)xa - (float)screen_lines / 2.0);
          screen_orig_y = (int) ((float)ya + (float)screen_lines / 2.0);
          repaint_image_scrolls(hDlg);
          repaint_scan_bitmap(hDlg);
          break;
        case SCAN_DETAIL_VIEW:
          xa = (data->x - 2048) * 100 * x_offset_range;
          ya = (data->y - 2048) * 100 * y_offset_range;
          image_orig_x = (int)(-(float)data->size * (float)data->step * 5.0 *
                                  data->x_gain * (float)data->x_range);
          image_orig_y = (int)(-(float) data->size * (float)data->step * 5.0 *
                              (float)data->y_gain * (float)data->y_range);
          screen_lines = (int)((float)data->size * 10.0 * data->x_gain *
                                (float)data->step * (float)data->x_range);
          screen_orig_x = xa + image_orig_x;
          screen_orig_y = ya - image_orig_y;
          image_orig_x += xa;
          image_orig_y += ya;
          repaint_image_scrolls(hDlg);
          repaint_scan_bitmap(hDlg);
          break;
        case SCAN_FULL_VIEW:
          screen_orig_x = -409600;
          screen_orig_y = 409600;
          screen_lines = 819200;
          repaint_image_scrolls(hDlg);
          repaint_scan_bitmap(hDlg);
          break;

        // file save & load
		case SCAN_SAVE_ALL:
          data = all_data[0];
          SendMessage(GetParent(hDlg),WM_COMMAND,FILE_SAVE_AS,0);
          for(i = 1;i < scan_max_data;i++)
          {
            if(all_data[i]->valid && !all_data[i]->saved) 
            {
              sprintf(string,"Save image %d too?",i + 1);
              if(MessageBox(hDlg, string, 
                        "Notice", MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK)
              {
                data = all_data[i];
                SendMessage(GetParent(hDlg),WM_COMMAND,FILE_SAVE_AS,0);
                all_data[i]->saved = 1;
              }
              else all_data[i]->saved = 1;
            }
          }
          data = all_data[scan_current_data];
          scan_set_title(hDlg);
          break;
        case SCAN_SAVE:
          SendMessage(GetParent(hDlg),WM_COMMAND,FILE_SAVE_AS,0);
          scan_set_title(hDlg);
          break;
        case SCAN_LOAD:
//          SendMessage(GetParent(hDlg),WM_COMMAND,FILE_LOAD,0);
          glob_data = &data;
          if(data->valid && !data->saved)
          {
            glob_data = &data;
            if(MessageBox(hDlg,"Image not saved!\nLoad will overwrite image data...","Warning",
			              MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK)
               if(result = file_open(hDlg,hInst,FTYPE_STM,0,current_file_stm))
                        data->saved = TRUE;
          } else
                result = file_open(hDlg,hInst,FTYPE_STM,0,current_file_stm);
          if(result)  // data successfully loaded. stuff all parameters into our local vars.
          {
//            strcpy(sample_type,data->sample_type.ptr);
            scan_x = data->x;
            scan_y = data->y;
            scan_z = data->z;
            scan_step = data->step;
            scan_size = data->size;
            scan_x_gain = data->x_gain;
            scan_y_gain = data->y_gain;
            find_min_max(data,&min_z,&max_z);
            xa = (scan_x - 2048) * 100 * x_offset_range;
            ya = (scan_y - 2048) * 100 * y_offset_range;
            image_orig_x = (int)(-(float)data->size * (float)data->step * 5.0 *
                                      data->x_gain * (float)data->x_range);
            image_orig_y = (int)(-(float)data->size * (float)data->step * 5.0 *
                              (float)data->y_gain * (float)data->y_range);
            screen_lines = (int)((float)data->size * 10.0 * data->x_gain *
                                (float)data->step * (float)data->x_range);
            screen_orig_x = xa + image_orig_x;
            screen_orig_y = ya - image_orig_y;
            image_orig_x += xa;
            image_orig_y += ya;
            repaint_image_scrolls(hDlg);

          }
          CheckDlgButton(hDlg,SCAN_X_GAIN_TENTH,0);
          CheckDlgButton(hDlg,SCAN_Y_GAIN_TENTH,0);
          CheckDlgButton(hDlg,SCAN_X_GAIN_ONE,0);
          CheckDlgButton(hDlg,SCAN_Y_GAIN_ONE,0);
          CheckDlgButton(hDlg,SCAN_X_GAIN_TEN,0);
          CheckDlgButton(hDlg,SCAN_Y_GAIN_TEN,0);

          if(scan_x_gain == point1)
              CheckDlgButton(hDlg,SCAN_X_GAIN_TENTH,1);
          else if(scan_x_gain == 1)
              CheckDlgButton(hDlg,SCAN_X_GAIN_ONE,1);
          else if(scan_x_gain == 10)
              CheckDlgButton(hDlg,SCAN_X_GAIN_TEN,1);

          if(scan_y_gain == point1)
              CheckDlgButton(hDlg,SCAN_Y_GAIN_TENTH,1);
          else if(scan_y_gain == 1)
              CheckDlgButton(hDlg,SCAN_Y_GAIN_ONE,1);
          else if(scan_y_gain == 10)
              CheckDlgButton(hDlg,SCAN_Y_GAIN_TEN,1);

		  repaint_scan_x(hDlg);
          repaint_scan_y(hDlg);
//          repaint_scan_z(hDlg);
          repaint_scan_step(hDlg);
          repaint_scan_size(hDlg);
//          repaint_scan_num(hDlg);
//          repaint_scan_freq(hDlg);
//          repaint_sample_bias(hDlg);
          repaint_scan_dir(hDlg);
          if(result)
          {
            init_bitmap_data();
//            UpdateBitmap(hDlg,0,IMAGESIZE,0,IMAGESIZE);
          }
          repaint_scan_bitmap(hDlg);
          repaint_pal(hDlg,PAL_X_POS,PAL_Y_POS,scan_dacbox,NULL);
          scan_set_title(hDlg);
          break;

        case SCAN_START:
			//check setting.sw Mod 5
			if(scan_current_default != 0)
          {
            sprintf(string,"You are not taking topography image, continue?");
             if(MessageBox(hDlg,string, 
                    "Warning",MB_OKCANCEL | MB_ICONEXCLAMATION) == IDCANCEL)
            break;
			}
                    
			//check setting
			//Check Scan Fine Shaowei Mod. 13
			if (scan_fine)
			{
				scan_fine = 0;
				CheckDlgButton(hDlg, SCAN_FINE, scan_fine);
				repaint_scan_x(hDlg);
				repaint_scan_y(hDlg);
				repaint_scan_bitmap(hDlg);
			}

		   i = 0;
			         
          for(j = 0;j < scan_max_data;j++)
          {
            if(all_data[j]->valid && !all_data[j]->saved) i++;
          }
          if(i)
          {
            sprintf(string,"%d image(s) not saved!\nScan will overwrite/free these images",i);
            if(MessageBox(hDlg,string, 
                    "Warning",MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK)
            i = 1;
            else i = 0;
          }
          else i = 1;
           
            
          if (i)
          {
            if(pre_scan(hDlg))
            {
              //if(scan_sharpen) sharpen(hDlg);
              //else
			  
			  scan(hDlg);

              xa = (data->x - 2048) * 100 * x_offset_range;
              ya = (data->y - 2048) * 100 * y_offset_range;
              image_orig_x = (int)(-(float)data->size * (float)data->step * 5.0 *
                                      data->x_gain * (float)data->x_range);
              image_orig_y = (int)(-(float)data->size * (float)data->step * 5.0 *
                              (float)data->y_gain * (float)data->y_range);
              screen_lines = (int)((float)data->size * 10.0 * data->x_gain *
                                (float)data->step * (float)data->x_range);
              screen_orig_x = xa + image_orig_x;
              screen_orig_y = ya - image_orig_y;
              image_orig_x += xa;
              image_orig_y += ya;
              repaint_image_scrolls(hDlg);			 
            }
            post_scan();
            init_bitmap_data();
//            UpdateBitmap(hDlg,0,IMAGESIZE,0,IMAGESIZE);
            repaint_scan_bitmap(hDlg);
            repaint_scan_time(hDlg);
          }
          scan_set_title(hDlg);
          break;

	    case SCAN_CURRENT_DATA:
          if(!out_smart)
          {
            GetDlgItemText(hDlg,LOWORD(wParam),string,9);
            i = atoi(string);
            scan_current_data = min(max(i,0),scan_max_data - 1);
            data = all_data[scan_current_data];
            repaint_scan_current_data(hDlg);
            repaint_data(hDlg);
          }
          break;
            
        case SCAN_PLANE_FIT:
          scan_plane_fit = IsDlgButtonChecked(hDlg,SCAN_PLANE_FIT);
          init_bitmap_data();
          repaint_scan_bitmap(hDlg);
          break;

        case ENTER:
          SetFocus(GetDlgItem(hDlg,SCAN_EXIT));
          repaint_scan_x(hDlg);
          repaint_scan_y(hDlg);
//          repaint_scan_z(hDlg);
          repaint_scan_step(hDlg);
          repaint_scan_size(hDlg);
//          repaint_scan_num(hDlg);
          repaint_scan_dir(hDlg);
          repaint_scan_bitmap(hDlg);
		  repaint_scantrigger_ch(hDlg);
          break;

//Mod. Auto- Shaowei
        case SCAN_OFFSET_AUTO_BELOW:
        auto_z_below(IN_ZERO);
		break;
// end auto-

//Triggering Shaowei

		case SCAN_TRIGGER_CH:
			if(!out_smart)
			{
				out_smart = 1;
				GetDlgItemText(hDlg,SCAN_TRIGGER_CH,string,9);
				scan_trigger_ch = (int)atoi(string);
				scan_trigger_ch = min(max(scan_trigger_ch,SP_TRIGGER_CH_MIN),SP_TRIGGER_CH_MAX);
				out_smart = 0;
			}
		break;

		case SCAN_TRIGGER_HIGH://Trigger to high
		    scan_trigger_now = ramp_serial(scan_trigger_ch,scan_trigger_now,
		                                 scan_trigger_high,10,0);
		break;
		
		case SCAN_TRIGGER_LOW: //Trigger to low
		   scan_trigger_now = ramp_serial(scan_trigger_ch,scan_trigger_now,
		                                 scan_trigger_low,10,0);
		break;
		case SCAN_TRIGGER_PULSE: //Trigger to low
		   scan_trigger_now = ramp_serial(scan_trigger_ch,scan_trigger_now,
		                                 scan_trigger_low,10,0);//set channel to low
		   scan_trigger_now = ramp_serial(scan_trigger_ch,scan_trigger_now,
		                                 scan_trigger_high,10,0);
		   Sleep(500);
		   scan_trigger_now = ramp_serial(scan_trigger_ch,scan_trigger_now,
		                                 scan_trigger_low,10,0);
		break;


// End Triggering

        case SCAN_EXIT:
	      if(MessageBox(hDlg,"Really exit?","Warning",MB_OKCANCEL | MB_ICONEXCLAMATION) ==
				IDCANCEL)
		  {
	        break;
		  }

		  if (MessageBox(hDlg, "Are you really sure?", "Warning", MB_OKCANCEL | MB_ICONEXCLAMATION) ==
			  IDCANCEL)
		  {
			  break;
		  }


          if(!datapro)//Mod. Shaowei datapro mode
		  {
		  strcpy(filename,SCAN_INI);
          sprintf(filename + strlen(filename),"%d.ini",scan_current_default);
          glob_data = &scan_defaults_data[scan_current_default];
          save_image(filename,0);
          if(TrackOn) SendMessage(TrackWnd,WM_COMMAND,TRACK_EXIT,0);
          //if(Prescan2On) SendMessage(Prescan2Wnd,WM_COMMAND,PRESCAN2_EXIT,0);
          //dio_feedback(1);
          dio_feedback_serial(1);
          switch(scan_scale)
          {
            case 0:
              scan_freq -= 2;
              if(scan_freq < 0) scan_freq = 0;
			  auto_z_above(IN_ZERO);
              scan_scale = 1;
              calc_gains(&x_gain,&y_gain,&z_gain,&z2_gain);
              //set_gain(x_gain,y_gain,z_gain,z2_gain);
              set_gain_serial(x_gain,y_gain,z_gain,z2_gain);
              auto_z_above(IN_ZERO);
              break;
            case 1:
              scan_freq -= 1;
              if(scan_freq < 0) scan_freq = 0;
              auto_z_above(IN_ZERO);
              break;
          }
          scan_scale = 2;
          calc_gains(&x_gain,&y_gain,&z_gain,&z2_gain);
          //set_gain(x_gain, y_gain, z_gain, z2_gain);
          set_gain_serial(x_gain, y_gain, z_gain, z2_gain);
            
          //move_to_timed(z_offset_ch,dac_data[z_offset_ch],ZERO,DEFAULT_Z_OFFSET_TIME);
          //move_to_timed(y_offset_ch,dac_data[y_offset_ch],ZERO,10 / scan_y_gain * SD->inter_step_delay);
          //move_to_timed(x_offset_ch,dac_data[x_offset_ch],ZERO,10 / scan_x_gain * SD->inter_step_delay);
          //move_to_timed(y_ch,dac_data[y_ch],ZERO,10 * SD->inter_step_delay);
          //move_to_timed(x_ch,dac_data[x_ch],ZERO,10 * SD->inter_step_delay);
          ramp_serial(z_offset_ch,dac_data[z_offset_ch],ZERO,DEFAULT_Z_OFFSET_TIME,0);
          ramp_serial(y_offset_ch,dac_data[y_offset_ch],ZERO,(int)(10 / scan_y_gain * SD->inter_step_delay),0);
          ramp_serial(x_offset_ch,dac_data[x_offset_ch],ZERO,(int)(10 / scan_x_gain * SD->inter_step_delay),0);
          ramp_serial(y_ch,dac_data[y_ch],ZERO,10 * SD->inter_step_delay,0);
          ramp_serial(x_ch,dac_data[x_ch],ZERO,10 * SD->inter_step_delay,0);

          scan_x = scan_y = ZERO;
          scan_fine_x = scan_fine_y = ZERO;
          dac_data[x_ch] = ZERO;
          dac_data[y_ch] = ZERO;
          dac_data[x_offset_ch] = ZERO;
          dac_data[y_offset_ch] = ZERO;
          dac_data[z_offset_ch] = ZERO;
          scan_z = ZERO;
            
//          free(scan_dacbox);
          free(image_data);
          free(bitmap_data);

		}
//          for(i = 0;i < SCAN_NUM_DEFAULTS;i++) free_data(&scan_defaults_data[i]);
          EndDialog(hDlg,TRUE);
            return(TRUE);
      }
      break;
  }
  return (FALSE);
}

int pre_scan(HWND hDlg)
{
// Performs a variety of actions to prepare for a scan. This includes Initialization of
// variables and fundamental parameters (bias, current, etc) as well as moving the tip
// to the scan's starting location (lower left corner on screen).
// Returns 0 if the tip can't be moved to the start location. Otherwise, returns 1.
	
  int i;
  unsigned int x_gain,
               y_gain,
               z_gain,
               z2_gain;
  float time,first_gain,second_gain;
  unsigned int first_ch,second_ch,first_offset,second_offset;
  unsigned int moved_value;
  int mini,overshoot_offset_fast,overshoot_offset_slow;

  // I. Initialization
  wait_cursor();
  save_init(SCAN_SETTINGS);
  calc_gains(&x_gain,&y_gain,&z_gain,&z2_gain);
  //  set_gain(x_gain,y_gain,z_gain,z2_gain);
  set_gain_serial(x_gain,y_gain,z_gain,z2_gain);
  time = calc_total_time();
  if(SD->x_first)
  {
    first_ch = x_ch;
    first_offset = x_offset_ch;
    second_ch = y_ch;
    second_offset = y_offset_ch;
    first_gain = scan_x_gain;
    second_gain = scan_y_gain;
  }
  else
  {
    first_ch = y_ch;
    first_offset = y_offset_ch;
    second_ch = x_ch;
    second_offset = x_offset_ch;
    first_gain = scan_y_gain;
    second_gain = scan_x_gain;
  }
  if(num_data() > 1 || scan_max_data > 1)
  {
    if(num_data() > scan_max_data)
    {
      for(i = scan_max_data;i < num_data();i++)
      {
        alloc_data(&(all_data[i]),DATATYPE_3D,
                    DATA_MAX,GEN2D_NONE,GEN2D_NONE,SD->read_seq_num);
      }
      scan_max_data = num_data();
    }
    else
    {
      for(i = max(num_data(),1);i < scan_max_data;i++) free_data(&(all_data[i]));
      scan_max_data = max(num_data(),1);
    }
  }
  data = all_data[0];
  scan_current_data = 0;
  repaint_scan_current_data(hDlg);

  // II. Paranoia - re-set some fundamental parameters
  //  bias(sample_bias);
  bias_serial(sample_bias);
  //  tip_current(i_setpoint);
  tip_current_serial(i_setpoint);
  //  mode(fine_mode);
  mode_serial(fine_mode);
  //  mode(translate_mode);
  mode_serial(translate_mode);
  //dio_feedback(SD->scan_feedback);
  dio_feedback_serial(SD->scan_feedback);
  dither0_before = dio_dither_status(0);
  dither1_before = dio_dither_status(1);
  //dio_dither(0,SD->scan_dither0);
  dio_dither_serial(0,SD->scan_dither0);
  //dio_dither(1,SD->scan_dither1);
  dio_dither_serial(1,SD->scan_dither1);

  // III. Move Fine X and Y to center of region (ZERO,ZERO)
  // NOTE: use "Wait for tip spacing..." option here, if implemented
  scan_status(hDlg,"Zeroing Fine X and Y",-1.0);
  dac_data[x_ch] = move_to_protect_serial(x_ch,dac_data[x_ch],ZERO,
      SD->inter_step_delay,SD->crash_protection,SD->z_limit_percent);
  if(dac_data[x_ch] != ZERO)
  {
    MessageBox(hDlg,"Could not zero X. Aborting.","Warning", 
            MB_ICONEXCLAMATION);
    return(0);
  }
  scan_fine_x = ZERO;
  repaint_scan_x(hDlg);
  dac_data[y_ch] = move_to_protect_serial(y_ch,dac_data[y_ch],ZERO,
      SD->inter_step_delay,SD->crash_protection,SD->z_limit_percent);
  if(dac_data[y_ch] != ZERO)
  {
    MessageBox(hDlg,"Could not zero Y. Aborting.","Warning", 
            MB_ICONEXCLAMATION);
    return(0);
  }
  scan_fine_y = ZERO;
  repaint_scan_y(hDlg);

  // IV. Sync offsets. Ramp x, y, z offset to last clicked position in case
  // the user hasn't hit "SEND".
  // NOTE: use "Wait for tip spacing..." option here, if implemented
  scan_status(hDlg,"Synching offsets",-1.0);
  dac_data[x_offset_ch] = move_to_protect_serial(x_offset_ch,dac_data[x_offset_ch],scan_x,
      (int)(10 / scan_x_gain) * SD->inter_step_delay,SD->crash_protection,SD->z_limit_percent);
  if(dac_data[x_offset_ch] != scan_x)
  {
    MessageBox(hDlg,"Could not sync X Offset. Aborting.","Warning", 
      MB_ICONEXCLAMATION);
    return(0);
  }
  dac_data[y_offset_ch] = move_to_protect_serial(y_offset_ch,dac_data[y_offset_ch],scan_y,
      (int)(10 / scan_y_gain) * SD->inter_step_delay,SD->crash_protection,SD->z_limit_percent);
  if(dac_data[y_offset_ch] != scan_y)
  {
    MessageBox(hDlg, "Could not sync Y Offset. Aborting.","Warning", 
            MB_ICONEXCLAMATION);
    return(0);
  }
  ramp_serial(z_offset_ch,dac_data[z_offset_ch],scan_z,DEFAULT_Z_OFFSET_TIME,0);
  dac_data[z_offset_ch] = scan_z;

  // V. Move slow direction (second_ch) to starting location.
  // NOTE: use "Wait for tip spacing..." option here, if implemented
  scan_status(hDlg,"Moving in the slow direction.",-1.0);
  if((moved_value = move_to_protect_serial(second_ch,dac_data[second_ch],
        ZERO - scan_step * scan_size / 2,SD->inter_step_delay,
        SD->crash_protection,SD->z_limit_percent)) !=
        ZERO - scan_step * scan_size / 2)
  {
    MessageBox(hDlg,"Could not move in slow direction!\nForcing it back and aborting.", 
        "Warning",MB_ICONEXCLAMATION);
    move_to_protect_serial(second_ch,moved_value,dac_data[second_ch], 
            SD->inter_step_delay,SD->crash_protection,SD->z_limit_percent);
    return(0);
  }

  time -= calc_initial_time() / 2;
  sprintf(string,"%0.2f",time);
  SetDlgItemText(hDlg,SCAN_TIME,string);

  // VI. Overshoot in slow direction.
  // NOTE: use "Wait for tip spacing..." option here, if implemented
  if ((SD->overshoot & OVERSHOOT_SLOW))
  {
/*    mini = (int)floor(scan_step * scan_size / 2 * second_gain / OFFSET_GAIN + 0.5);
    overshoot_offset_slow = max((int)(dac_data[second_offset] -
            scan_step * scan_size / 2 * 
            second_gain / OFFSET_GAIN * SD->overshoot_percent) / 100,mini);
*/
    mini = floor(scan_step * scan_size / 2 * second_gain / OFFSET_GAIN + 0.5);
    overshoot_offset_slow = max(dac_data[second_offset] -
            scan_step * scan_size / 2 * 
            second_gain / OFFSET_GAIN * SD->overshoot_percent / 100,mini);

    if (overshoot_offset_slow < dac_data[second_offset])
    {
      scan_status(hDlg,"Overshooting by %0.2f%% in the slow dir",
                SD->overshoot_percent);
      moved_value = move_to_protect_serial(second_offset,dac_data[second_offset], 
                overshoot_offset_slow,(int)(10 / second_gain) * SD->inter_step_delay,
                SD->crash_protection,SD->z_limit_percent);
      if(moved_value != overshoot_offset_slow)
      {
        MessageBox(hDlg,"Could not overshoot in slow direction!\nForcing it back and aborting.", 
                "Warning",MB_ICONEXCLAMATION);
        // undo offset
		move_to_protect_serial(second_offset,moved_value,
                dac_data[second_offset],(int)(10 / second_gain) * SD->inter_step_delay,
                SD->crash_protection,SD->z_limit_percent);
		// undo fine
        move_to_protect_serial(second_ch,ZERO - scan_step * scan_size / 2 ,
                dac_data[second_ch],SD->inter_step_delay,SD->crash_protection,
                SD->z_limit_percent);
        return(0);
      }
    }
    time -= calc_overshoot_slow_time();
    sprintf(string,"%0.2f",time);
    SetDlgItemText(hDlg,SCAN_TIME,string);
    scan_status(hDlg,"Waiting %0.0f seconds at overshot location",
            (float)SD->overshoot_wait1);
    for(i = 0;i < 5 * SD->overshoot_wait1;i++)
	{
	  // five * maximum delay (FFFFFF) = about 1 sec
	  serial_soft_delay(0xFFFFFF);
      time -= (float)0.2;
      sprintf(string,"%0.2f",time);
      SetDlgItemText(hDlg,SCAN_TIME,string);
	}
    if(overshoot_offset_slow < dac_data[second_offset])
    {
      scan_status(hDlg,"Moving overshot slow direction back...",-1.0);
      moved_value = move_to_protect_serial(second_offset,overshoot_offset_slow, 
                dac_data[second_offset],(int)(10 / second_gain) * SD->inter_step_delay,
                SD->crash_protection,SD->z_limit_percent);
      if(moved_value != dac_data[second_offset])
      {
        MessageBox(hDlg,"Could not return overshoot in slow direction!\nForcing it back and aborting.", 
          "Warning",MB_ICONEXCLAMATION);
		// undo offset
        move_to_protect_serial(second_offset,moved_value,
          dac_data[second_offset],(int)(10 / second_gain) * SD->inter_step_delay,
          SD->crash_protection,SD->z_limit_percent);
		// undo fine
		move_to_protect_serial(second_ch,ZERO - scan_step * scan_size / 2,
          dac_data[second_ch], SD->inter_step_delay,SD->crash_protection,
          SD->z_limit_percent);
        return(0);
      }
                
      time -= calc_overshoot_slow_time();
      sprintf(string,"%0.2f",time);
      SetDlgItemText(hDlg,SCAN_TIME,string);
	}
    scan_status(hDlg,"Waiting %0.0f seconds at image edge",
      (float)SD->overshoot_wait2);
    for(i = 0;i < 5 * SD->overshoot_wait2;i++)
    {
	  // five * maximum delay (FFFFFF) = about 1 sec
	  serial_soft_delay(0xFFFFFF);
      time -= (float)0.2;
      sprintf(string,"%0.2f",time);
      SetDlgItemText(hDlg,SCAN_TIME,string);
        }
  }

  // VII. Move fast direction (first_ch) to starting location.
  // NOTE: use "Wait for tip spacing..." option here, if implemented
  scan_status(hDlg,"Moving in the fast direction.",-1.0);
  if((moved_value = move_to_protect_serial(first_ch,dac_data[first_ch], 
        ZERO - scan_step * scan_size / 2,SD->inter_step_delay,
        SD->crash_protection,SD->z_limit_percent))!=
        ZERO - scan_step * scan_size / 2)
  {
    MessageBox(hDlg,"Could not move in fast direction!\nForcing it back and aborting.", 
      "Warning", MB_ICONEXCLAMATION);
	// undo first ch fine
    move_to_protect_serial(first_ch,moved_value,dac_data[first_ch], 
      SD->inter_step_delay,SD->crash_protection,SD->z_limit_percent);
	// undo second ch fine
    move_to_protect_serial(second_ch,ZERO - scan_step * scan_size / 2,
      dac_data[second_ch], SD->inter_step_delay,SD->crash_protection,
      SD->z_limit_percent);
    return(0);
  }

  // VIII. Overshoot in fast direction.
  // NOTE: use "Wait for tip spacing..." option here, if implemented
  if((SD->overshoot & OVERSHOOT_FAST))
  {
/*
	mini = (int)floor(scan_step * scan_size / 2 * first_gain / OFFSET_GAIN + 0.5);
    overshoot_offset_fast = max((int)(dac_data[first_offset] -
            scan_step * scan_size / 2 * 
            first_gain / OFFSET_GAIN * SD->overshoot_percent) / 100,mini);
*/
    mini = floor(scan_step * scan_size / 2 * first_gain / OFFSET_GAIN + 0.5);
    overshoot_offset_fast = max((dac_data[first_offset] -
            scan_step * scan_size / 2 * 
            first_gain / OFFSET_GAIN * SD->overshoot_percent) / 100,mini);
    if(overshoot_offset_fast < dac_data[first_offset])
    {
      scan_status(hDlg,"Overshooting by %0.2f%% in the fast dir",
        (float)(dac_data[first_offset] - overshoot_offset_fast) *
        100 / (float) mini);
      moved_value = move_to_protect_serial(first_offset,dac_data[first_offset], 
        overshoot_offset_fast,(int)(10 / first_gain) * SD->inter_step_delay,
        SD->crash_protection,SD->z_limit_percent);
      if(moved_value != overshoot_offset_fast)
      {
        MessageBox(hDlg,"Could not overshoot in fast direction!\nForcing it back and aborting.", 
          "Warning",MB_ICONEXCLAMATION);
		// undo offset
        move_to_protect_serial(first_offset,moved_value,
          dac_data[first_offset],(int)(10 / first_gain) * SD->inter_step_delay,
          SD->crash_protection,SD->z_limit_percent);
        // undo first ch fine
		move_to_protect_serial(first_ch,ZERO - scan_step * scan_size / 2,
          dac_data[first_ch],SD->inter_step_delay,SD->crash_protection,
          SD->z_limit_percent);
        // undo second ch fine
        move_to_protect_serial(second_ch,ZERO - scan_step * scan_size / 2,
          dac_data[second_ch],SD->inter_step_delay,SD->crash_protection,
          SD->z_limit_percent);
        return(0);
      }
    }
    time -= calc_overshoot_fast_time();
    sprintf(string,"%0.2f",time);
    SetDlgItemText(hDlg, SCAN_TIME, string);
    scan_status(hDlg,"Waiting %0.0f seconds at overshot location",
            (float)SD->overshoot_wait1);

    for(i = 0;i < 5 * SD->overshoot_wait1;i++)
    {
	  // five * maximum delay (FFFFFF) = about 1 sec
	  serial_soft_delay(0xFFFFFF);
      time -= (float)0.2;
      sprintf(string,"%0.2f",time);
      SetDlgItemText(hDlg,SCAN_TIME,string);
    }
    if(overshoot_offset_fast < dac_data[first_offset])
    {
      scan_status(hDlg,"Moving overshot fast direction back...",-1.0);
      moved_value = move_to_protect_serial(first_offset,overshoot_offset_fast,
         dac_data[first_offset],(int)(10 / first_gain) * SD->inter_step_delay,
         SD->crash_protection,SD->z_limit_percent);
      if(moved_value != dac_data[first_offset])
      {
        MessageBox(hDlg, "Could not return in overshot fast direction!\nForcing it back and aborting.", 
          "Warning", MB_ICONEXCLAMATION);
		// undo offset
        move_to_protect_serial(first_offset,moved_value,
          dac_data[first_offset],(int)(10 / first_gain) * SD->inter_step_delay,
          SD->crash_protection,SD->z_limit_percent);
		// undo first ch fine
        move_to_protect_serial(first_ch,ZERO - scan_step * scan_size / 2 ,
          dac_data[first_ch],SD->inter_step_delay,SD->crash_protection,
          SD->z_limit_percent);
		// undo second ch fine
        move_to_protect_serial(second_ch,ZERO - scan_step * scan_size / 2 ,
          dac_data[second_ch], SD->inter_step_delay,SD->crash_protection,
          SD->z_limit_percent);
        return(0);
      }
      time -= calc_overshoot_fast_time();
      sprintf(string,"%0.2f",time);
      SetDlgItemText(hDlg, SCAN_TIME, string);
    }
    scan_status(hDlg,"Waiting %0.0f seconds at image edge",
            (float)SD->overshoot_wait2);
    for(i = 0;i < 5 * SD->overshoot_wait2;i++)
    {
	  // five * maximum delay (FFFFFF) = about 1 sec
	  serial_soft_delay(0xFFFFFF);
      time -= (float)0.2;
      sprintf(string,"%0.2f",time);
      SetDlgItemText(hDlg,SCAN_TIME,string);
    }
  }

  return(1); /* success */
}

void post_scan()
{
//  unsigned int first_ch,second_ch;

  //dio_feedback(1);
  // Shaowei Post Scan
  dio_feedback_serial(scan_end_fb);
  //dio_dither(0,dither0_before);
  if (scan_end_retract)
  {
	  dio_feedback_serial(1);
	  retract_serial(1);
  }
  
  if (scan_end_auto)
  {
	  dio_feedback_serial(1);
	  auto_z_below(IN_ZERO);
  }
  // end Shaowei Post Scan

  dio_dither_serial(0,dither0_before);
  //dio_dither(1,dither1_before);
  dio_dither_serial(1,dither1_before);
  Beep( 750, 300 );
  glob_data = &data;
  save_image(SCAN_IMAGE,0);
  arrow_cursor();
}

void init_bitmap_data()
{
  int i,j;
  int data_size;
  double a,b,c;
  datadef *this_data = NULL;
  float z,min_z,max_z;
    
  if(data->valid)
  {
    if(scan_plane_fit)
    {
      alloc_data(&this_data,DATA_MAX,0,GEN2D_NONE,GEN2D_NONE,1);
      copy_data(&this_data,&data);
      fit_plane_simple(this_data,&a,&b,&c);
      for(i = 0;i < this_data->size;i++)
        for(j = 0;j < this_data->size;j++)
        {
          z = *(this_data->ptr + this_data->size * i + j) - a * i - b * j - c + IN_ZERO;
          *(this_data->ptr + i * this_data->size + j) = (float)z;
        }
        find_min_max(this_data,&min_z,&max_z);
    }
    else this_data = data;

    bitmap_size = find_size(this_data->size);
    if(bitmap_data == NULL)
      bitmap_data = (unsigned char *)malloc(bitmap_size * bitmap_size *
									 sizeof(unsigned char));
    else
      bitmap_data = (unsigned char *)realloc((void *)bitmap_data,bitmap_size * 
									 bitmap_size * sizeof(unsigned char));
        
    data_size = (int)(this_data->max_z - this_data->min_z + 1);

    if(!data_size) data_size = 1;
    for(j = 0; j < this_data->size; j++)
      for (i = 0; i < this_data->size; i++)
            *(bitmap_data + j * bitmap_size + i) = 
                (unsigned char) ((unsigned long) (*(this_data->ptr + j *
				this_data->size + i) - this_data->min_z) * LOGPALUSED / data_size);
    if (scan_plane_fit) free_data(&this_data);
  }
}

void set_scan_defaults(datadef *this_data)
{
  this_data->valid = 1; 
  strcpy(this_data->filename,"Unsaved"); 
  strcpy(this_data->full_filename,"Unsaved"); 
  this_data->x = scan_x; 
  this_data->y = scan_y; 
  this_data->z = scan_z; 
  this_data->x_range = x_range; 
  this_data->y_range = y_range; 
  this_data->step = scan_step; 
  this_data->size = scan_size; 
  this_data->x_gain = scan_x_gain; 
  this_data->y_gain = scan_y_gain; 
  this_data->z_gain = calc_z_gain(scan_scale); 
  this_data->z_freq = scan_freq; 
  this_data->x_offset_range = x_offset_range; 
  this_data->y_offset_range = y_offset_range; 
  this_data->bias = sample_bias; 
  this_data->bias_range = sample_bias_range; 
  this_data->i_setpoint = i_setpoint; 
  this_data->i_set_range = i_set_range; 
  this_data->amp_gain = tip_gain; 
    
  free_data_seq(&this_data);
  alloc_data_seq(&this_data,1);
  this_data->read_seq_num = 1;
    
  this_data->digital_feedback_max = 1000;
  this_data->digital_feedback_reread = 1;
  this_data->z_limit_percent = 5; 
  this_data->x_first = 0; 
  this_data->step_delay = 500; 
  this_data->step_delay_fixed = 1; 
  this_data->inter_step_delay = 50; 
  this_data->digital_feedback = 1; 
  this_data->tip_spacing = 0.5; 
  this_data->inter_line_delay = 20000; 
  this_data->scan_feedback = 1; 
  this_data->crash_protection = CRASH_MINMAX_Z; 
  this_data->overshoot = OVERSHOOT_SLOW; 
  this_data->overshoot_percent = 20; 
  this_data->overshoot_wait1 = 3; 
  this_data->overshoot_wait2 = 1; 
  this_data->scan_dither0 = 0; 
  this_data->scan_dither1 = 0; 
  this_data->version = STM_DATA_VERSION;

  this_data->read_seq[0].num_samples = 5; 
  this_data->read_seq[0].record = 1; 
  this_data->read_seq[0].read_ch = 1; 
  this_data->read_seq[0].feedback = 1; 
  this_data->read_seq[0].dither0 = 0; 
  this_data->read_seq[0].dither1 = 0; 
  this_data->read_seq[0].wait1 = 0; 
  this_data->read_seq[0].wait2 = 0; 
  this_data->read_seq[0].wait3 = 0; 
  this_data->current_read_seq = 0;
            
  this_data->scan_dir = FORWARD_DIR; 
}

int num_data()
{
// returns the number of data sets (scans) for a given read sequence
  int count = 0;
  int i;

  for(i = 0;i < SD->total_steps;i++)
  {
    if(SD->sequence[i].type == READ_TYPE) count++;
  }
  
  if(SD->scan_dir == BOTH_DIR2 || SD->scan_dir == BOTH_DIR2_POLAR)
    count *= 2;
  return count;

}

int find_size(int size)
{
// Use this function to find the size of an appropriate bitmap,
// given that the size of the data is size (in units of steps).
// Note that the return value is restricted to powers of two.

  int i;
  int result;
      
  for(i = 10;i > 0;i--)
  {
    result = 1L << i;
    if(size == result || (size < result && size > (result >> 1))) break;
  }
  return(result);    
}

void calc_gains(unsigned int *x_gain,unsigned int *y_gain,unsigned int *z_gain,
				unsigned int *z2_gain)
{
    float point1 = (float)0.1;

    if(scan_x_gain == point1)
      *x_gain = X_TENTH_GAIN;
    if(scan_x_gain == 1)
      *x_gain = X_ONE_GAIN;
    if(scan_x_gain == 10)
      *x_gain = X_TEN_GAIN;
    if(scan_y_gain == point1)
      *y_gain = Y_TENTH_GAIN;
    if(scan_y_gain == 1)
      *y_gain = Y_ONE_GAIN;
    if(scan_y_gain == 10)
      *y_gain = Y_TEN_GAIN;
    switch(scan_scale)
    {
      case 0:
        *z2_gain = Z2_TENTH_GAIN;
        break;
      case 1:
        *z2_gain = Z2_ONE_GAIN;
        break;
      case 2:
        *z2_gain = Z2_TEN_GAIN;
        break;
    }
    
    switch (scan_freq)
    {
      case 0:
        *z_gain = Z_TENTH_GAIN;
        break;
      case 1:
        *z_gain = Z_ONE_GAIN;
        break;
      case 2:
        *z_gain = Z_TEN_GAIN;
        break;
    }
    if(!SD->step_delay_fixed) calc_step_delay();
}

float calc_z_gain(unsigned int scan_scale)
{
  switch(scan_scale)
  {
    case 0:
      return (float)0.1;
      break;
    case 1:
      return 1;
      break;
    case 2:
      return 10;
      break;
	default:
	  return -1;	// error
	  break;
  }
}

void calc_step_delay()
{
    switch(scan_scale)
    {
      case 0:
        switch(scan_freq)
        {
          case 0:
            if(!SD->step_delay_fixed)
                SD->step_delay = 40000;
            break;
          case 1:
            if(!SD->step_delay_fixed)
                SD->step_delay = 4000;
            break;
          case 2:
            if(!SD->step_delay_fixed)
                SD->step_delay = 400;
            break;
        }
        break;
      case 1:
        switch(scan_freq)
        {
          case 0:
            if(!SD->step_delay_fixed)
                SD->step_delay = 40000;
            break;
          case 1:
            if(!SD->step_delay_fixed)
                SD->step_delay = 4000;
            break;
          case 2:
            if(!SD->step_delay_fixed)
                SD->step_delay = 400;
            break;
        }
        break;
      case 2:
        switch(scan_freq)
        {
          case 0:
            if(!SD->step_delay_fixed)
                SD->step_delay = 40000;
            break;
          case 1:
            if(!SD->step_delay_fixed)
                SD->step_delay = 4000;
            break;
          case 2:
            if(!SD->step_delay_fixed)
                SD->step_delay = 400;
            break;
		}
        break;
    }
}

float calc_initial_time()
{
  return(((float)SD->inter_step_delay * (float)(scan_step - 1) * 
	  (float)(scan_size - 1)) / (float)1000000.0);
}

float calc_overshoot_fast_time()
{
  float time,first_gain,second_gain;
  int mini,overshoot_offset_fast;
  unsigned int first_ch,second_ch,first_offset,second_offset;
  if(SD->x_first)
  {
    first_ch = x_ch;
    first_offset = scan_x;
    second_ch = y_ch;
    second_offset = scan_y;
    first_gain = scan_x_gain;
    second_gain = scan_y_gain;
  }
  else
  {
    first_ch = y_ch;
    first_offset = scan_y;
    second_ch = x_ch;
    second_offset = scan_x;
    first_gain = scan_y_gain;
    second_gain = scan_x_gain;
  }
  if((SD->overshoot & OVERSHOOT_FAST))
  {
    mini = (int)(scan_step * scan_size / 2 * first_gain / OFFSET_GAIN);
    overshoot_offset_fast = max((int)(first_offset-
            scan_step * scan_size / 2 * 
            first_gain / OFFSET_GAIN*SD->overshoot_percent / 100),mini);
    if(overshoot_offset_fast < first_offset)
    {
      time = (first_offset - overshoot_offset_fast) *
                10 / first_gain * SD->inter_step_delay / 1000000;
    }
    else time=0;
  }
  else time = 0;
  return(time);
}            

float calc_overshoot_slow_time()
{
  float time,first_gain,second_gain;
  int mini,overshoot_offset_slow;
  unsigned int first_ch,second_ch,first_offset,second_offset;
  if(SD->x_first)
  {
    first_ch = x_ch;
    first_offset = scan_x;
    second_ch = y_ch;
    second_offset = scan_y;
    first_gain = scan_x_gain;
    second_gain = scan_y_gain;
  } 
  else
  {
    first_ch = y_ch;
    first_offset = scan_y;
    second_ch = x_ch;
    second_offset = scan_x;
    first_gain = scan_y_gain;
    second_gain = scan_x_gain;
  }
  if((SD->overshoot & OVERSHOOT_SLOW))
  {
    mini = (int)(scan_step * scan_size / 2 * second_gain / OFFSET_GAIN);
        overshoot_offset_slow = max((int)(second_offset-
        scan_step * scan_size / 2 * 
        second_gain / OFFSET_GAIN * SD->overshoot_percent / 100),mini);
    if(overshoot_offset_slow < second_offset)
    {
      time = (second_offset - overshoot_offset_slow) *
                10 / second_gain * SD->inter_step_delay / 1000000;
    }
    else time = 0;
                        
  }
  else time = 0;
  return(time);
}            

float calc_total_time()
{
  float time;
    
  time = calc_time(scan_size);
  time += calc_initial_time();
  if((SD->overshoot & OVERSHOOT_FAST)) time += calc_overshoot_fast_time() * 2;
  if((SD->overshoot & OVERSHOOT_SLOW)) time += calc_overshoot_slow_time() * 2;
  if((SD->overshoot & OVERSHOOT_SLOW)) time += SD->overshoot_wait1 + SD->overshoot_wait2;
  if((SD->overshoot & OVERSHOOT_FAST)) time += SD->overshoot_wait1 + SD->overshoot_wait2;
  return(time);
}

float calc_seq_time()
{
  int i;
  float time = (float)SD->step_delay;

  for(i = 0;i < SD->total_steps;i++)
  {
	switch(SD->sequence[i].type)
	{
	  case WAIT_TYPE:
		time += (float)(SD->sequence[i].wait);
		break;
	  case READ_TYPE:
        time += one_input_time * SD->sequence[i].num_reads;
		break;
	  case STEP_OUTPUT_TYPE:
        time += dio_out_time;
		break;
	  case SERIAL_OUT_TYPE:
		time += 1000.0; // at least 1 ms min for serial comm
		break;
	  case DITHER0_TYPE:
	  case DITHER1_TYPE:
	  case FEEDBACK_TYPE:
        time += dio_out_time;
		break;
	}
  }

  return(time);
}
    
float calc_time(int lines_left)
{
  float time;
//    float read_line,slow_line;
  float move_time,read_time;
                              
  move_time = (float)SD->inter_step_delay;
  if(SD->digital_feedback) move_time += dio_out_time + one_input_time;
  move_time *= (float)scan_step * (float)scan_size;
  read_time = calc_seq_time();
  read_time *= (float)scan_size;
    
  if(SD->scan_dir == BOTH_DIR1)
  {
    time = read_time + move_time;
    time *= (float)lines_left;
  }
  else if(SD->scan_dir == BOTH_DIR2 || SD->scan_dir == BOTH_DIR2_POLAR)
  {
    time = read_time + move_time;
    time *= 2.0;
    time *= (float)lines_left;
  }
  else 
  {
    time = read_time + move_time * (float)2.0;
    if(SD->crash_protection != CRASH_DO_NOTHING)
    {
      time += (float)((one_input_time + dio_out_time) * scan_step * scan_size);
    }
    time *= lines_left;
  }
  time += SD->inter_line_delay * lines_left * (float)2.0;
  time /= 1000000.0;
    
#ifdef OLD
  read_line = ((float)SD->inter_step_delay * (float)(scan_step - 1) * (float)
        (scan_size - 1) + (float)scan_size * ((float)scan_num * 40.0 +
        SD->step_delay)) / 1000000.0;
  slow_line = ((float)SD->inter_step_delay * (float) scan_step) / 1000000.0;
  time = ((float)SD->inter_line_delay) / 1000000.0;
  if(SD->scan_dir == BOTH_DIR1)
  {
    time += read_line + slow_line;
    time *= (float)lines_left;
  }
  else if (SD->scan_dir==BOTH_DIR2 || SD->scan_dir==BOTH_DIR2_POLAR)
  {
    time *= 2.0;
    time += 2.0 * read_line + slow_line;
    time *= (float)lines_left;
  }
  else 
  {
    time *= 2.0;
    time += (float)SD->inter_step_delay * (float)scan_step * (float)(scan_size - 1) /
            1000000.0 + read_line + slow_line;
    time *= lines_left;
  }
#endif
  return(time);
}

static void scan_set_title(HWND hDlg)
{
  if(data->valid)
  {
    sprintf(string,"Scan: %s",data->filename);
    SetWindowText(hDlg,string);
  }
  else SetWindowText(hDlg,"Scan");
}

void scan_status(HWND hDlg,char *text,float num)
{
  if(num < 0) SetDlgItemText(hDlg,SCAN_STATUS,text);
  else
  {
    sprintf(string,text,num);
    SetDlgItemText(hDlg,SCAN_STATUS,string);
  }
}

static void enable_clock_users(HWND hDlg,int status)
{
  //EnableWindow(GetDlgItem(hDlg,SCAN_SHARPENB),status);


  EnableWindow(GetDlgItem(hDlg,DEPOSITION),status);
  EnableWindow(GetDlgItem(hDlg,SCAN_SPEC),status);
  EnableWindow(GetDlgItem(hDlg,SCAN_TRACK),status);
  EnableWindow(GetDlgItem(hDlg,PRESCAN_BUTTON),status);
  EnableWindow(GetDlgItem(hDlg,SCAN_PRESCAN2),status);
  EnableWindow(GetDlgItem(hDlg,SEND_X_Y_OFFSETS),status);
  EnableWindow(GetDlgItem(hDlg,SCAN_START),status);
  EnableWindow(GetDlgItem(hDlg,SCAN_OFFSET_AUTO_BELOW),status);//.Shaowei
  enable_x_gain(hDlg,status);
  enable_y_gain(hDlg,status);
}

void enable_x_gain(HWND hDlg,int status)
{
  EnableWindow(GetDlgItem(hDlg,SCAN_X_GAIN_TENTH),status);
  EnableWindow(GetDlgItem(hDlg,SCAN_X_GAIN_ONE),status);
  EnableWindow(GetDlgItem(hDlg,SCAN_X_GAIN_TEN),status);
    
}

void enable_y_gain(HWND hDlg,int status)
{
  EnableWindow(GetDlgItem(hDlg,SCAN_Y_GAIN_TENTH),status);
  EnableWindow(GetDlgItem(hDlg,SCAN_Y_GAIN_ONE),status);
  EnableWindow(GetDlgItem(hDlg,SCAN_Y_GAIN_TEN),status);
    
}

void repaint_scan_dir(HWND hDlg)
{
  if(SD->x_first)
  {
      CheckDlgButton(hDlg,X_FIRST,1);
      CheckDlgButton(hDlg,Y_FIRST,0);
  }
  else {
      CheckDlgButton(hDlg,X_FIRST,0);
      CheckDlgButton(hDlg,Y_FIRST,1);
  }
  switch (SD->scan_dir)
  {
    case FORWARD_DIR:
      CheckDlgButton(hDlg,READ_FORWARD,1);
      CheckDlgButton(hDlg,READ_1_IMAGE,0);
      CheckDlgButton(hDlg,READ_2_IMAGES,0);
      CheckDlgButton(hDlg,READ_2_POLAR_IMAGES,0);
      CheckDlgButton(hDlg,READ_BACKWARD,0);
        break;
    case BACKWARD_DIR:
      CheckDlgButton(hDlg,READ_FORWARD,0);
      CheckDlgButton(hDlg,READ_1_IMAGE,0);
      CheckDlgButton(hDlg,READ_2_POLAR_IMAGES,0);
      CheckDlgButton(hDlg,READ_2_IMAGES,0);
      CheckDlgButton(hDlg,READ_BACKWARD,1);
        break;
    case BOTH_DIR1:
      CheckDlgButton(hDlg,READ_FORWARD,0);
      CheckDlgButton(hDlg,READ_2_POLAR_IMAGES,0);
      CheckDlgButton(hDlg,READ_1_IMAGE,1);
      CheckDlgButton(hDlg,READ_2_IMAGES,0);
      CheckDlgButton(hDlg,READ_BACKWARD,0);
        break;
    case BOTH_DIR2:
      CheckDlgButton(hDlg,READ_FORWARD,0);
      CheckDlgButton(hDlg,READ_2_POLAR_IMAGES,0);
      CheckDlgButton(hDlg,READ_1_IMAGE,0);
      CheckDlgButton(hDlg,READ_2_IMAGES,1);
      CheckDlgButton(hDlg,READ_BACKWARD,0);
        break;
    case BOTH_DIR2_POLAR:
      CheckDlgButton(hDlg,READ_FORWARD,0);
      CheckDlgButton(hDlg,READ_2_POLAR_IMAGES,1);
      CheckDlgButton(hDlg,READ_1_IMAGE,0);
      CheckDlgButton(hDlg,READ_2_IMAGES,0);
      CheckDlgButton(hDlg,READ_BACKWARD,0);
        break;
  }
  repaint_scan_time(hDlg);
}

void repaint_scan_x(HWND hDlg)
{
  int mini,maxi;

  mini = scan_x_min + (int)floor(scan_step * scan_size / 2 * scan_x_gain / OFFSET_GAIN + 0.5);
  maxi = scan_x_max - (int)floor((scan_step * scan_size / 2 - 1) * scan_x_gain / OFFSET_GAIN + 0.5);
  scan_x = min(max(scan_x,mini),maxi);
/*
  sprintf(string,"min %d max %d",mini,maxi);
  scan_status(hDlg,string,-1);
*/
  if(dac_data[x_ch] != ZERO) enable_x_gain(hDlg,0);
  else enable_x_gain(hDlg,1);
  out_smart = 1;
  if(scan_fine)
  {
    SetDlgItemInt(hDlg,SCAN_X_BITS_EDIT,scan_fine_x,FALSE);
    SetScrollPos(GetDlgItem(hDlg,SCAN_X_SCROLL),SB_CTL,scan_fine_x,TRUE);
    SetDlgItemText(hDlg,SCAN_X_VOLTS_EDIT,gcvt(dtov(scan_fine_x,x_range),5,string));
    SetDlgItemText(hDlg,SCAN_X_ANGS_EDIT,gcvt(dtov(scan_fine_x,x_range) * A_PER_V,5,string));
  }
  else
  {
    SetDlgItemInt(hDlg,SCAN_X_BITS_EDIT,scan_x,FALSE);
    SetScrollPos(GetDlgItem(hDlg,SCAN_X_SCROLL),SB_CTL,scan_x,TRUE);
    SetDlgItemText(hDlg,SCAN_X_VOLTS_EDIT,gcvt(10 * dtov(scan_x,x_offset_range),5, string));
    SetDlgItemText(hDlg,SCAN_X_ANGS_EDIT,gcvt(10 * dtov(scan_x,x_offset_range) * A_PER_V,5,string));
  }
  out_smart = 0;
}

void repaint_scan_y(HWND hDlg)
{
  int mini,maxi;

  mini = scan_y_min + (int)floor(scan_step * scan_size / 2 * scan_y_gain / OFFSET_GAIN + 0.5);
  maxi = scan_y_max - (int)floor((scan_step * scan_size / 2 - 1) * scan_y_gain / OFFSET_GAIN + 0.5); 
  scan_y = min(max(scan_y,mini),maxi);
  if(dac_data[y_ch] != ZERO) enable_y_gain(hDlg,0);
  else enable_y_gain(hDlg,1);
  out_smart = 1;
  if(scan_fine)
  {
    SetDlgItemInt(hDlg,SCAN_Y_BITS_EDIT,scan_fine_y,FALSE);
    SetScrollPos(GetDlgItem(hDlg, SCAN_Y_SCROLL),SB_CTL,scan_y_max - scan_fine_y,TRUE); 
    SetDlgItemText(hDlg,SCAN_Y_VOLTS_EDIT,gcvt(dtov(scan_fine_y,y_range),5,string));
    SetDlgItemText(hDlg,SCAN_Y_ANGS_EDIT,gcvt(dtov(scan_fine_y,y_range) * A_PER_V,5,string));
  }
  else
  {
    SetDlgItemInt(hDlg,SCAN_Y_BITS_EDIT,scan_y,FALSE);
    SetScrollPos(GetDlgItem(hDlg,SCAN_Y_SCROLL),SB_CTL,scan_y_max - scan_y,TRUE);
    SetDlgItemText(hDlg,SCAN_Y_VOLTS_EDIT,gcvt(10 * dtov(scan_y,y_offset_range),5,string));
    SetDlgItemText(hDlg,SCAN_Y_ANGS_EDIT,gcvt(10 * dtov(scan_y,y_offset_range) * A_PER_V,5,string));
  }
  out_smart = 0;
}

void repaint_scan_step(HWND hDlg)
{
  int i;

  i = (int)((scan_x - scan_x_min) * OFFSET_GAIN / scan_x_gain);
  i = min(i,(int)((scan_x_max + 1 - scan_x) * OFFSET_GAIN / scan_x_gain));
  i = min(i,(int)((scan_y - scan_y_min) * OFFSET_GAIN / scan_y_gain));
  i = min(i,(int)((scan_y_max + 1 - scan_y) * OFFSET_GAIN / scan_y_gain));
  i = (i * 2 + 1) / scan_size;
  scan_step = min(max(scan_step,scan_step_min),scan_step_max);
  scan_step = min(scan_step,i);
  scan_step = min(scan_step,SCANSIZE / scan_size);
  SetDlgItemInt(hDlg,SCAN_STEP_BITS_EDIT,scan_step,FALSE);
  SetScrollPos(GetDlgItem(hDlg,SCAN_STEP_SCROLL),SB_CTL,scan_step,TRUE);
//    SetDlgItemText(hDlg,SCAN_STEP_VOLTS_EDIT,gcvt(scan_step * scan_x_gain * 20 / 4096,5,string));
  sprintf(string,"%f.5",scan_step * scan_x_gain * 20 / 4096);
  SetDlgItemText(hDlg,SCAN_STEP_VOLTS_EDIT,string);
  SetDlgItemText(hDlg,SCAN_STEP_ANGS_EDIT,gcvt(scan_step * scan_x_gain * 20 / 4096 * A_PER_V,5,string));
  repaint_scan_time(hDlg);
}

void repaint_scan_size(HWND hDlg)
{
  int i;

  i = (int)((scan_x - scan_x_min) * OFFSET_GAIN / scan_x_gain);
  i = min(i,(int)((scan_x_max + 1 - scan_x) * OFFSET_GAIN / scan_x_gain));
  i = min(i,(int)((scan_y - scan_y_min) * OFFSET_GAIN / scan_y_gain));
  i = min(i,(int)((scan_y_max + 1 - scan_y) * OFFSET_GAIN / scan_y_gain));
  i = (i * 2 + 1) / scan_step;
  scan_size = min(max(scan_size,scan_size_min),scan_size_max);
  scan_size = min(scan_size,i);
  scan_size = min(scan_size,SCANSIZE / scan_step);
  if(scan_size == 64)
        CheckDlgButton(hDlg,SCAN_SIZE_64,1);
  else
        CheckDlgButton(hDlg,SCAN_SIZE_64,0);
  if(scan_size == 128)
        CheckDlgButton(hDlg,SCAN_SIZE_128,1);
  else
        CheckDlgButton(hDlg,SCAN_SIZE_128,0);
  if(scan_size == 256)
        CheckDlgButton(hDlg,SCAN_SIZE_256,1);
  else
        CheckDlgButton(hDlg,SCAN_SIZE_256,0);
  if(scan_size == 512)
        CheckDlgButton(hDlg,SCAN_SIZE_512,1);
  else
        CheckDlgButton(hDlg,SCAN_SIZE_512,0);
  SetDlgItemInt(hDlg,SCAN_SIZE_STEPS_EDIT,scan_size,FALSE);
  SetScrollPos(GetDlgItem(hDlg,SCAN_SIZE_SCROLL),SB_CTL,scan_size,TRUE);
  SetDlgItemInt(hDlg,SCAN_SIZE_BITS_EDIT,scan_size * scan_step,FALSE);
  SetDlgItemText(hDlg,SCAN_SIZE_VOLTS_EDIT,gcvt(dtov_len(scan_size * scan_step,x_range) * scan_x_gain,5,string));
  SetDlgItemText(hDlg,SCAN_SIZE_ANGS_EDIT,gcvt(dtov_len(scan_size * scan_step,x_range) * scan_x_gain * A_PER_V,5,string));
  repaint_scan_time(hDlg);
}

void repaint_scan_time(HWND hDlg)
{
  sprintf(string,"%0.2f",calc_total_time());
  SetDlgItemText(hDlg,SCAN_TIME,string);
}    

void repaint_scan_bitmap(HWND hDlg)
{
  HDC hDC;
  HDC hMemDC;
  LOGBRUSH lplb;
  HBRUSH hBrush,hBrushOld;
  HPEN hPen,hPenOld;
  HRGN hRgn;
  float x,y,x2,y2;
  int lx,ly;
  int offscreen = 0;
  int dest_x,dest_y,dest_width,dest_height,src_x,src_y,src_width,src_height;
  int image_end_x,image_end_y;

  hDC = GetDC(hDlg);
  hRgn = CreateRectRgn(IMAGE_X_POS - 1,IMAGE_Y_POS - 1,IMAGE_X_POS + IMAGESIZE + 1,
					   IMAGE_Y_POS + IMAGESIZE + 1);
  SelectClipRgn(hDC,hRgn);
  hMemDC = CreateCompatibleDC(hDC);
  SelectPalette(hDC,hPal,0);
  RealizePalette(hDC);
  SelectPalette(hMemDC,hPal,0);
  RealizePalette(hMemDC);
 
  bitmapinfo->bmiHeader.biBitCount = 8;
  bitmapinfo->bmiHeader.biWidth = bitmap_size;
  bitmapinfo->bmiHeader.biHeight = bitmap_size;
    
  SetMapMode(hDC,MM_TEXT);

//    set_Plasma_palette();
  lplb.lbStyle = BS_SOLID;
  lplb.lbColor = RGB(0,0,0);
  hBrush = CreateBrushIndirect(&lplb);
  hBrushOld = SelectObject(hDC,hBrush);
  hPen = CreatePen(PS_SOLID,1,RGB(0,0,0));
  hPenOld = SelectObject(hDC,hPen);
  Rectangle(hDC,IMAGE_X_POS,IMAGE_Y_POS,IMAGESIZE + IMAGE_X_POS,IMAGESIZE + IMAGE_Y_POS);
  SelectObject(hDC,hBrushOld);
  SelectObject(hDC,hPenOld);
  DeleteObject(hBrush);
  DeleteObject(hPen);
  if(data->valid)
  {
    image_end_x = image_orig_x + (int)(data->size * data->step * data->x_gain * 10 * data->x_range);
    image_end_y = image_orig_y + (int)(data->size * data->step * data->y_gain * 10 * data->y_range);
    if(image_orig_x > screen_orig_x + screen_lines || image_end_x < screen_orig_x
        || image_orig_y > screen_orig_y || image_end_y < screen_orig_y - screen_lines)
    {
      offscreen = 1;
    }
    else 
    {
      if(image_orig_x >= screen_orig_x)
      {
        src_x = 0;
        dest_x = (int)floor((float)(image_orig_x - screen_orig_x) *
                 (float)IMAGESIZE / (float)screen_lines + 0.5);
        if(image_end_x < screen_orig_x + screen_lines)
        {
          src_width = data->size;
          dest_width = (int)floor((float)(data->size * data->step * data->x_gain *
                    10 * data->x_range * IMAGESIZE) / (float)screen_lines + 0.5);
        }
        else
        {
          dest_width = IMAGESIZE - dest_x;
          src_width = (int)floor((float)(dest_width * screen_lines)
                        /(float)IMAGESIZE / (float)data->x_range / (float)10 /
                        (float)data->x_gain / (float)data->step + 0.5);
        }
      }
      else
      {
        dest_x = 0;
        src_x = (int)floor((float)(screen_orig_x - image_orig_x) /
                (float)data->x_range / (float)10 / (float)data->x_gain /
                (float)data->step + 0.5);
        if(image_end_x < screen_orig_x + screen_lines)
        {
          src_width = data->size - src_x;
          dest_width = (int)floor((float)src_width * (float)data->step *
                       (float)data->x_gain * (float)10 * (float)data->x_range *
                       (float)IMAGESIZE / (float)screen_lines + 0.5);
        }
        else
        {
          dest_width = IMAGESIZE;
          src_width = (int) floor((float)(dest_width * screen_lines) /
                      (float) IMAGESIZE / (float)data->x_range /
                      (float)10 / (float)data->x_gain / (float)data->step + 0.5);
        }
      }
      if(image_orig_y > screen_orig_y - screen_lines)
      {
        src_y = 0;
        dest_y = (int)floor((float)((image_orig_y - screen_orig_y + screen_lines) * IMAGESIZE)
                 / (float)screen_lines + 0.5);
        if(image_end_y<=screen_orig_y)
        {
          src_height = data->size;                                                 
          dest_height = (int)floor((float)(data->size * data->step *
                        data->y_gain * 10 * data->y_range * IMAGESIZE) /
                        (float)screen_lines + 0.5);
        }
        else
        {
          dest_height = IMAGESIZE - dest_y;
          src_height = (int)floor((float)(dest_height * screen_lines) /
                       (float)IMAGESIZE / (float)data->y_range / (float)10 /
                       (float)data->y_gain / (float)data->step + 0.5);
        }
      }
      else
      {
        dest_y = 0;
        src_y = (int)floor((float)(screen_orig_y - screen_lines - image_orig_y) /
                (float)data->y_range / (float)10 / (float)data->y_gain /
                (float)data->step + 0.5);
        if(image_end_y <= screen_orig_y)
        {
          src_height = data->size - src_y;
          dest_height = (int)floor((float)(src_height * data->step *
                        data->y_gain * 10 * data->y_range * IMAGESIZE) /
                        (float)screen_lines + 0.5);
        }
        else
        {
          dest_height = IMAGESIZE;
          src_height = (int)floor((float)(dest_height * screen_lines) /
                       (float)IMAGESIZE / (float)data->y_range / (float)10 /
                       (float)data->y_gain / (float)data->step + 0.5);
        }
      }
      dest_y = IMAGESIZE - dest_y - dest_height;
    }
/*
    sprintf(string,"%d %d %d %d",dest_y,dest_height,src_y,src_height);
    MessageBox(hDlg,string,"Test",MB_OKCANCEL | MB_ICONEXCLAMATION);
*/
  }

  if(data->valid && !offscreen)
  {
    StretchDIBits(hDC,dest_x + IMAGE_X_POS,dest_y + IMAGE_Y_POS,dest_width,dest_height,
		src_x,src_y,src_width,src_height,bitmap_data,bitmapinfo,DIB_PAL_COLORS,SRCCOPY);
  }

  x = (float)(((float)scan_x - ZERO) * 100 * x_offset_range - screen_orig_x) * IMAGESIZE /
        (float)screen_lines;
  y = (float) -(((float)scan_y - ZERO) * 100 * y_offset_range - screen_orig_y) * IMAGESIZE /
        (float)screen_lines;
  lplb.lbStyle = BS_HOLLOW;
  hBrush = CreateBrushIndirect(&lplb);
  hBrushOld = SelectObject(hDC,hBrush);
  hPen = CreatePen(PS_DASH,1,RGB(0,255,0));
  hPenOld = SelectObject(hDC,hPen);

  lx = (int)(scan_size * 10 * scan_x_gain * scan_step * x_range);
  ly = (int)(scan_size * 10 * scan_y_gain * scan_step * y_range);

  x2 = x + lx * IMAGESIZE / (float)2 / (float)screen_lines;
  y2 = y + ly * IMAGESIZE / (float)2 / (float)screen_lines;
  x -= lx * IMAGESIZE / (float)2 / (float)screen_lines;
  y -= ly * IMAGESIZE / (float)2 / (float)screen_lines;
  Rectangle(hDC,(int)floor(x + IMAGE_X_POS + 0.5),(int)floor(0.5 + y + IMAGE_Y_POS), 
        (int)floor(0.5 + x2 + IMAGE_X_POS),(int)floor(0.5 + y2 + IMAGE_Y_POS));
  SelectObject(hDC,hBrushOld);
  SelectObject(hDC,hPenOld);
  DeleteObject(hBrush);
  DeleteObject(hPen);
  SelectClipRgn(hDC,NULL);
  DeleteObject(hRgn);
  DeleteDC(hMemDC);
  ReleaseDC(hDlg,hDC);
  repaint_scan_cursor(hDlg);
  if(TrackOn) repaint_track_limits(hDlg);
}

static void repaint_track_limits(HWND hDlg)
{
  HDC hDC;
  LOGBRUSH lplb;
  HBRUSH hBrush,hBrushOld;
  HPEN hPen,hPenOld;
  HRGN hRgn;
  float x,y,x2,y2;
  int OldROP;
        
  hDC = GetDC(hDlg);
  hRgn = CreateRectRgn(IMAGE_X_POS - 1,IMAGE_Y_POS - 1,IMAGE_X_POS + IMAGESIZE + 1,
	     IMAGE_Y_POS + IMAGESIZE + 1);
  SelectClipRgn(hDC, hRgn);
  SelectPalette(hDC, hPal, 0);
  RealizePalette(hDC);
  lplb.lbStyle = BS_HOLLOW;
  hBrush = CreateBrushIndirect(&lplb);
  hBrushOld = SelectObject(hDC,hBrush);
  hPen = CreatePen(PS_DASH,1,RGB(0,255,0));
  hPenOld = SelectObject(hDC,hPen);
  OldROP = SetROP2(hDC,R2_NOTXORPEN);
  x = (float) (((float)scan_x - ZERO) * 100 * x_offset_range 
        +(float) (((float)track_limit_x_min - ZERO) * 10 * x_range*scan_x_gain) 
        - screen_orig_x) * IMAGESIZE /(float)screen_lines;
  y = (float) -(((float)scan_y - ZERO) * 100 * y_offset_range 
        +(float) (((float)track_limit_y_min - ZERO) * 10 * y_range*scan_y_gain) 
        - screen_orig_y) * IMAGESIZE /(float) screen_lines;
  x2 = (float) (((float)scan_x - ZERO) * 100 * x_offset_range 
        +(float) (((float)track_limit_x_max - ZERO) * 10 * x_range*scan_x_gain) 
        - screen_orig_x) * IMAGESIZE /(float)screen_lines;
  y2 = (float) -(((float)scan_y - ZERO) * 100 * y_offset_range 
        +(float) (((float)track_limit_y_max - ZERO) * 10 * y_range*scan_y_gain) 
        - screen_orig_y) * IMAGESIZE /(float) screen_lines;

  Rectangle(hDC,(int)floor(x + IMAGE_X_POS+0.5),(int)floor(0.5 + y + IMAGE_Y_POS), 
        (int)floor(0.5 + x2 + IMAGE_X_POS),(int)floor(0.5 + y2 + IMAGE_Y_POS));
  SetROP2(hDC,OldROP);
  SelectObject(hDC,hBrushOld);
  SelectObject(hDC,hPenOld);
  DeleteObject(hBrush);
  DeleteObject(hPen);
  SelectClipRgn(hDC,NULL);
  DeleteObject(hRgn);
  ReleaseDC(hDlg, hDC);
} 

void repaint_scan_cursor(HWND hDlg)
{
  HDC hDC;
  HDC hMemDC;
  HRGN hRgn;
  float x,y;

  hDC = GetDC(hDlg);
  hRgn = CreateRectRgn(IMAGE_X_POS - 1,IMAGE_Y_POS-1,IMAGE_X_POS + IMAGESIZE + 1,
	     IMAGE_Y_POS + IMAGESIZE + 1);
  SelectClipRgn(hDC,hRgn);
  hMemDC = CreateCompatibleDC(hDC);
  SelectPalette(hDC,hPal,0);
  RealizePalette(hDC);
  SelectPalette(hMemDC,hPal,0);
  RealizePalette(hMemDC);
    
  SetMapMode(hDC,MM_TEXT);

  SelectObject(hMemDC,hCross);
  x = (float) (((float)scan_x - ZERO) * 100 * x_offset_range 
        - screen_orig_x) * IMAGESIZE /(float)screen_lines;
  y = (float) -(((float)scan_y - ZERO) * 100 * y_offset_range 
        - screen_orig_y) * IMAGESIZE /(float) screen_lines;

  if(x >= 0 && y >= 0 && x < IMAGESIZE && y < IMAGESIZE)
    BitBlt(hDC,(int)(x + 30),(int)(y + 30),15,15,hMemDC,0,0,SRCINVERT);
  if(scan_fine_x !=ZERO || scan_fine_y != ZERO)
  {        
    SelectObject(hMemDC, hXcross);
    x = (float) (((float)scan_x - ZERO) * 100 * x_offset_range 
            +(float) (((float)scan_fine_x - ZERO) * 10 * x_range*scan_x_gain) 
            - screen_orig_x) * IMAGESIZE /(float)screen_lines;
    y = (float) -(((float)scan_y - ZERO) * 100 * y_offset_range 
            +(float) (((float)scan_fine_y - ZERO) * 10 * y_range*scan_y_gain) 
            - screen_orig_y) * IMAGESIZE /(float) screen_lines;

    if (x >= 0 && y >= 0 && x < IMAGESIZE && y < IMAGESIZE)
      BitBlt(hDC,(int)(x + 30),(int)(y + 30),15,15,hMemDC,0,0,SRCINVERT);
  }
  x = (float) (((float)scan_x - ZERO) * 100 * x_offset_range - screen_orig_x) * IMAGESIZE /
        (float)screen_lines;
  y = (float) -(((float)scan_y - ZERO) * 100 * y_offset_range - screen_orig_y) * IMAGESIZE /
        (float) screen_lines;
  DeleteObject(hRgn);
  DeleteDC(hMemDC);
  ReleaseDC(hDlg, hDC);
}

void repaint_image_scrolls(HWND hDlg)
{
  HDC hDC;

  hDC = GetDC(hDlg);
  image_x_min = -ABS_SCALE / 2;
  image_x_max = ABS_SCALE / 2 - screen_lines;
  image_y_min = -image_x_max;
  image_y_max = -image_x_min;
/*
  image_y_min = -image_y_min;
  image_y_max = -image_y_max;
*/
/*
  SetScrollRange(GetDlgItem(hDlg,IMAGE_X_SCROLL),SB_CTL,image_x_min,image_x_max,FALSE);
  SetScrollRange(GetDlgItem(hDlg,IMAGE_Y_SCROLL),SB_CTL,image_y_min,image_y_max,FALSE);
*/
  out_smart = 1;
  SetScrollPos(GetDlgItem(hDlg,IMAGE_X_SCROLL),SB_CTL,DTOI(screen_orig_x,image_x_min,
	          image_x_max),TRUE);
  SetScrollPos(GetDlgItem(hDlg,IMAGE_Y_SCROLL),SB_CTL,DTOI(image_y_min + image_y_max -
			  screen_orig_y,image_y_min,image_y_max),TRUE);
  out_smart = 0;

  ReleaseDC(hDlg, hDC);
}

static void repaint_data(HWND hDlg)
{
  int xa,ya;
  init_bitmap_data();
  if(data->valid)
  {
    xa = (data->x - ZERO) * 100 * data->x_offset_range;
    ya = (data->y - ZERO) * 100 * data->y_offset_range;
    image_orig_x = (int)(-(float)data->size * (float)data->step * 5.0 *
                        data->x_gain * (float)data->x_range);
    image_orig_y = (int)(-(float)data->size * (float)data->step * 5.0 *
                        (float)data->y_gain * (float)data->y_range);
    image_orig_x += xa;
    image_orig_y += ya;
    repaint_image_scrolls(hDlg);
  }
  repaint_scan_bitmap(hDlg);
  scan_set_title(hDlg);
}    

static void repaint_scan_current_data(HWND hDlg)
{
  out_smart = 1;
  sprintf(string,"%d",scan_current_data);
  SetDlgItemText(hDlg,SCAN_CURRENT_DATA,string);
  SetScrollRange(GetDlgItem(hDlg,SCAN_CURRENT_DATA_SCROLL),SB_CTL,0,scan_max_data - 1,FALSE);
  SetScrollPos(GetDlgItem(hDlg,SCAN_CURRENT_DATA_SCROLL),SB_CTL,scan_current_data,TRUE);
  out_smart = 0;
}
    
//Triggering Shaowei

static void repaint_scantrigger_ch(HWND hDlg)
{
    out_smart = 1;
    sprintf(string,"%d",scan_trigger_ch);
    SetDlgItemText(hDlg,SCAN_TRIGGER_CH,string);
    out_smart = 0;
}

// ******************************************************************************************
//		O L D * O L D * O L D * O L D * O L D * O L D * O L D * O L D * O L D * O L D
// ******************************************************************************************


#ifdef OLD

#define STRETCH
//#define TIME_ERROR

int sign(float i)
{
  if(i < 0)
    return -1;
  else if(i > 0)
    return 1;
  else
    return 0;
}

void send_x_y_old()
{
// not really a function. cut and pasted from above

  // Offsets	
  i = 10 / scan_x_gain * SD->inter_step_delay * SCAN_OFFSET_FUDGE * (
                abs(scan_x - dac_data[x_offset_ch])) + 10 / scan_y_gain *
                SD->inter_step_delay * SCAN_OFFSET_FUDGE *
                (abs(scan_y-dac_data[y_offset_ch]));
  if(i > SCAN_WARN_TIME)
  {           
    sprintf(string,"It will take %d seconds to move!\nDo it anyway?",
                    i/1000000);
    i = (MessageBox(hDlg,string, 
                    "Warning",MB_YESNO | MB_ICONEXCLAMATION) == IDNO);
  }
  else i = 0;
  if(i) break;
                        
  dac_data[x_offset_ch] = move_to_protect(x_offset_ch,
                dac_data[x_offset_ch],
                scan_x,10 / scan_x_gain * SD->inter_step_delay * SCAN_OFFSET_FUDGE,
                SD->crash_protection,SD->z_limit_percent);
  if(dac_data[x_offset_ch] != scan_x)
  {
    scan_x = dac_data[x_offset_ch];
    repaint_scan_x(hDlg);
    repaint_scan_bitmap(hDlg);
    i = (MessageBox(hDlg,"Crash Protection activated while moving X.\nContinue with Y move?", 
                    "Warning",MB_YESNO | MB_ICONEXCLAMATION) == IDYES);
  }
  else
    i = 1;
  if(i)
  {
    dac_data[y_offset_ch] = move_to_protect(y_offset_ch,
                    dac_data[y_offset_ch],scan_y,
                    10 / scan_y_gain * SD->inter_step_delay * SCAN_OFFSET_FUDGE,
                    SD->crash_protection,SD->z_limit_percent);
    if(dac_data[y_offset_ch] != scan_y)
    {
                    scan_y=dac_data[y_offset_ch];
                    repaint_scan_y(hDlg);
                    repaint_scan_bitmap(hDlg);
                       MessageBox(hDlg, "Crash Protection activated while moving Y. No fine movement!", 
                        "Warning", MB_OKCANCEL | MB_ICONEXCLAMATION);
                    break;
                    
    }
  }

  // Fine coordinates
  i = SD->inter_step_delay * SCAN_OFFSET_FUDGE * (
                abs(scan_fine_x - dac_data[x_ch])) +
                SD->inter_step_delay * SCAN_OFFSET_FUDGE * 
                (abs(scan_fine_y - dac_data[y_ch]));
  if(i > SCAN_WARN_TIME)
  {
    sprintf(string,"It will take %d seconds to move!\nDo it anyway?",
                     i/1000000);
    i = (MessageBox(hDlg,string, 
                    "Warning",MB_YESNO | MB_ICONEXCLAMATION) == IDNO);
  }
  else i = 0;
  if(i) break;
                        
  dac_data[x_ch] = move_to_protect(x_ch,
                dac_data[x_ch],
                scan_fine_x,SD->inter_step_delay * SCAN_OFFSET_FUDGE,
                SD->crash_protection,SD->z_limit_percent);
  if(dac_data[x_ch] != scan_fine_x)
  {
    scan_fine_x = dac_data[x_ch];
    repaint_scan_x(hDlg);
    repaint_scan_bitmap(hDlg);
    i = (MessageBox(hDlg,"Crash Protection activated while moving Fine X.\nContinue with Y move?", 
                    "Warning", MB_YESNO | MB_ICONEXCLAMATION) == IDYES);
  }
  else
    i = 1;
  if(i)
  {
    dac_data[y_ch] = move_to_protect(y_ch,
                    dac_data[y_ch],scan_fine_y,
                    SD->inter_step_delay * SCAN_OFFSET_FUDGE,
                    SD->crash_protection,SD->z_limit_percent);
    if(dac_data[y_ch] != scan_fine_y)
    {
      scan_fine_y = dac_data[y_ch];
      repaint_scan_y(hDlg);
      repaint_scan_bitmap(hDlg);
      MessageBox(hDlg, "Crash Protection activated while moving Fine Y.", 
                        "Warning", MB_OKCANCEL | MB_ICONEXCLAMATION);
    }
  }
  repaint_scan_x(hDlg);
  repaint_scan_y(hDlg);

}

void sharpen_old(HWND hDlg)
{
 // I'm pretty certain that this code is missing an important '}' somewhere -mw

  HDC hDC;
  LOGBRUSH lplb;
  HBRUSH hBrush,hBrushOld;
  HPEN hPen,hPenOld;
  int delta_z_offset = 0;
  float delta_z_factor = 1;
  float last_line_min,last_line_max,this_line_min,this_line_max,
        temp_min,temp_max,temp_range;
  int i,j,k;
  unsigned int high_limit,low_limit,high_target,low_target;
  unsigned int offset;
  unsigned int n,moveto_steps,first_ch,second_ch;
  unsigned int *x,*y,*para_ptr,end_y;
  float z;
  int forward = 1;
  MSG Message;
  time_t scan_time;
  int max_x;
  int this_line;
  int this_bias;
  float stime,time_per_line;
  unsigned int I;
  unsigned int old_scan_z;
  float delta_z;
  int stop_now = 0;
  int Imax,Imin;
  int dest_x,dest_y,dest_width,dest_height,src_x, 
      src_y,src_width,src_height,dest_offset_x,dest_offset_y;
  int line_size;
  int z_offset_crashed;
  unsigned int x_gain,y_gain,z_gain,z2_gain;
    
#ifdef TIME_ERROR
  int time_error_line = 0;
  int time_error_step = 0;
  int time_error_inter = 0;
#endif    

  dio_start_clock(SD->inter_line_delay);
  delta_z = 0;
  src_x = src_y = 0;
    
  hDC = GetDC(hDlg);
  SetMapMode(hDC,MM_TEXT);

//  set_Plasma_palette();
  SelectPalette(hDC,hPal,0);
  RealizePalette(hDC);
  lplb.lbStyle = BS_SOLID;
  lplb.lbColor = RGB(0,0,0);
  hBrush = CreateBrushIndirect(&lplb);
  hBrushOld = SelectObject(hDC,hBrush);
  hPen = CreatePen(PS_SOLID,1,RGB(0,0,0));
  hPenOld = SelectObject(hDC,hPen);
  Rectangle(hDC,IMAGE_X_POS,IMAGE_Y_POS,IMAGESIZE + IMAGE_X_POS,IMAGESIZE + IMAGE_Y_POS);
  SelectObject(hDC,hBrushOld);
  SelectObject(hDC,hPenOld);
  DeleteObject(hBrush);
  DeleteObject(hPen);
    
  stime = calc_time(scan_size);
  time_per_line = stime / (float)scan_size;
  Imax = in_vtod(pow(10,SD->tip_spacing-dtov(i_setpoint,i_set_range) / 10.0));
  Imin = in_vtod(pow(10,-SD->tip_spacing-dtov(i_setpoint,i_set_range) / 10.0));
    
  high_limit = IN_MAX - (unsigned int)((float)IN_MAX * SD->z_limit_percent / 100);
  low_limit = (unsigned int)((float)IN_MAX * SD->z_limit_percent / 100);
  low_target = DEFAULT_Z_OFFSET_MOVE * low_limit;
  high_target = IN_MAX - (unsigned int)((float)IN_MAX * SD->z_limit_percent * 
							DEFAULT_Z_OFFSET_MOVE / 100);
/*
  sprintf(string,"min %d max %d",Imin,Imax);
  if(digital_feedback) MessageBox(hDlg,string,"Digital feedback settings",MB_ICONEXCLAMATION);
*/

  switch (scan_scale)
  {
    case 0:
      delta_z_factor = 100;
      break;
    case 1:
      delta_z_factor = 10;
      break;
    case 2:
      delta_z_factor = 1;
      break;
  }
  line_size = find_size(scan_size);
#ifdef STRETCH
  scan_display_factor = IMAGESIZE / scan_size;
#else
  scan_display_factor = 1;
#endif
  bitmap_size = line_size;
  if(bitmap_data == NULL)
    bitmap_data = (unsigned char *)malloc(bitmap_size * bitmap_size * sizeof(unsigned char));
  else
    bitmap_data = (unsigned char *)realloc((void *) bitmap_data, bitmap_size * bitmap_size * 
									sizeof(unsigned char));
  dest_offset_x = IMAGE_X_POS;
  dest_offset_x += (IMAGESIZE - scan_size * scan_display_factor) / 2;
  dest_offset_y = dest_offset_x;
  bitmapinfo->bmiHeader.biBitCount = 8;
  bitmapinfo->bmiHeader.biWidth = line_size;
  bitmapinfo->bmiHeader.biHeight = line_size;
    
  if(SD->x_first)
  {
    first_ch = x_ch;
    second_ch = y_ch;
/*
    bitmapinfo->bmiHeader.biWidth = line_size;
    bitmapinfo->bmiHeader.biHeight = 1;
*/
    src_width = scan_size;
    src_height = 1;
    dest_width = scan_size * scan_display_factor;
    dest_height = scan_display_factor;
    dest_x = 0;    
    dest_y = scan_size * scan_display_factor;
/*
    dest_y = IMAGESIZE - dest_offset_y - scan_display_factor + IMAGE_Y_POS;        
*/
  }
  else
  {
    first_ch = y_ch;
    second_ch = x_ch;
/*
    bitmapinfo->bmiHeader.biWidth = 1;
    bitmapinfo->bmiHeader.biHeight = line_size;
*/
    src_width = 1;
    src_height = scan_size;
    dest_height = scan_size * scan_display_factor;
    dest_width = scan_display_factor;
    dest_x = 0;    
    dest_y = 0;
  }
/*
  sprintf(string,"%d %d %d %d",dest_x,dest_y,dest_width,dest_height);
  MessageBox(hDlg,string,"Test",MB_ICONEXCLAMATION);
*/

  min_z = 7000000;
  max_z = -7000000;
  offset = ZERO - scan_step * scan_size / 2;
  x = y = para_ptr = stair_move_setup(scan_step,scan_size,offset);
  n = stair_size(scan_step);
//  _disable();
#ifdef OLD
  if(SD->scan_dir != BOTH_DIR1) max_x = scan_size;
  else max_x = scan_size / 2;   /* ## WARNING! need to do extra for odd sized scans ! */
#endif
  if(SD->scan_dir != BOTH_DIR1) max_x = sharp_cycles * sharp_lines;
  else max_x = sharp_cycles * sharp_lines / 2;
#ifdef TIME_ERROR
  if((inpw(stat) & 0x4000)) time_error_line++;
#endif
  dio_wait_clock();
  this_line = 1;
  ramp_bias((unsigned int)sharp_bias1,BIAS_RAMP_TIME,BIAS_RAMP_SKIP,1);
//  MessageBox(hDlg,"initial","Test",MB_ICONEXCLAMATION);
  this_bias = 0;
  for(j = 0;j < max_x && !stop_now;j++)
  {
    PeekMessage(&Message,hDlg,NULL,NULL,PM_REMOVE);
    if(LOWORD(Message.wParam) == VK_ESCAPE)
    {
      sprintf(string,"Scanning terminated.");
      MessageBox(hDlg,string,"Warning",MB_ICONEXCLAMATION);
      break;
    }

    for(k = 0;k < scan_size && !stop_now;k++)
    {
      if(SD->scan_dir == FORWARD_DIR || SD->scan_dir == BOTH_DIR1 || 
                SD->scan_dir == BOTH_DIR2)
      {    
        dio_start_clock(SD->step_delay);
#ifdef TIME_ERROR
		if((inpw(stat) & 0x4000)) time_error_step++;
#endif
        dio_wait_clock();
//        z = dio_read(scan_num) + delta_z_offset * delta_z_factor;
        z = dio_read(scan_num);
        if(z + delta_z > max_z)
          max_z = z + delta_z;
        if(z + delta_z < min_z)
          min_z = z+delta_z;
        if(SD->crash_protection == CRASH_MINMAX_Z)
        {
          if(k == 0)
          {
            if(j == 0)
            {
              this_line_min = this_line_max = last_line_min =
                                last_line_max = z;
            }
            else
            {
              last_line_max = this_line_max;
              last_line_min = this_line_min;
              this_line_min = this_line_max = z;
            }
          }
          else
          {
            if(z > this_line_max) this_line_max = z;
            if(z < this_line_min) this_line_min = z;
          }
          temp_min = min(this_line_min,last_line_min);
          temp_max = max(this_line_max,last_line_max);
          temp_range = temp_max-temp_min;
          if(temp_range > IN_MAX)
          {
            low_target = DEFAULT_Z_OFFSET_MOVE * low_limit;
            high_target = IN_MAX - (unsigned int)((float)IN_MAX * SD->z_limit_percent * 
											DEFAULT_Z_OFFSET_MOVE / 100);
          }
          else
          {
            low_target = (IN_MAX - temp_range) / 2;
            high_target = IN_MAX - (IN_MAX - temp_range) / 2;
          }
        }
        if(z > high_limit || z < low_limit)
        {
          if(SD->crash_protection == CRASH_STOP)
          {
            stop_now = 1;
            break;
          }
          else if(SD->crash_protection == CRASH_AUTO_Z ||
                             SD->crash_protection == CRASH_MINMAX_Z)
          {
            old_scan_z = z;
            if(z > high_limit)
            {
              auto_z_below(high_target);
            }
            else
            {
              auto_z_above(low_target);
            }
            dio_in_ch(read_ch);
            z = dio_read(scan_num);
            delta_z += old_scan_z-z;
//            delta_z_offset += scan_z - old_scan_z;
          }
        }
      }
      if(k < scan_size - 1)
      {
        for(i = 0;i < n - 1;i++)
        {
          y++;
          dio_out(first_ch,*y);
          dio_start_clock(SD->inter_step_delay);
#ifdef TIME_ERROR
		  if((inpw(stat) & 0x4000)) time_error_inter++;
#endif
          dio_wait_clock();
          if(SD->digital_feedback)
          {
            if(digital_abort) stop_now = !dio_digital_feedback(Imin,Imax);
            else dio_digital_feedback(Imin,Imax);
            dio_in_ch(read_ch);
          }
        }                
        y++;
        dio_out(first_ch,*y);
        if(SD->scan_dir == BACKWARD_DIR) 
        {
          dio_start_clock(SD->inter_step_delay);
#ifdef TIME_ERROR
          if((inpw(stat) & 0x4000)) time_error_inter++;
#endif
          dio_wait_clock();
          if(SD->digital_feedback)
          {
            if(digital_abort) stop_now = !dio_digital_feedback(Imin,Imax);
            else dio_digital_feedback(Imin,Imax);
            dio_in_ch(read_ch);
          }
        }
      }

      if(SD->scan_dir == FORWARD_DIR || SD->scan_dir == BOTH_DIR2)
      {
        if(SD->x_first)
          *(data->ptr + j * scan_size + k) = z + delta_z;
        else
          *(data->ptr + k * scan_size + j) = z + delta_z;
      }
      if(SD->scan_dir == BOTH_DIR1)
      {
        if(SD->x_first)
          *(data->ptr + j * 2 * scan_size + k) = z + delta_z;
        else
          *(data->ptr + k * scan_size + j * 2) = z + delta_z;
      }
                
    }  // for (k = 0...)

	if(stop_now) break;
    end_y = *y;
    if(SD->scan_dir != BACKWARD_DIR)
    {
      if(this_line == sharp_lines)
      {
        if(this_bias) ramp_bias((unsigned int)sharp_bias1,BIAS_RAMP_TIME,BIAS_RAMP_SKIP,1);
        else ramp_bias((unsigned int)sharp_bias2,BIAS_RAMP_TIME,BIAS_RAMP_SKIP,1);
//        MessageBox(hDlg,"1st","Test",MB_ICONEXCLAMATION);
        this_bias = !this_bias;
        this_line = 0;
      }
      this_line++;
      if(abs(sharp_bias1 - sharp_bias2) * BIAS_RAMP_TIME > SD->inter_line_delay)
      {
        dio_start_clock(5);
      }
      else 
        dio_start_clock(SD->inter_line_delay -
                    abs(sharp_bias1 - sharp_bias2) * BIAS_RAMP_TIME);
      } else dio_start_clock(SD->inter_line_delay);
      if(SD->scan_dir == BOTH_DIR1)
      {
        stime -= time_per_line;
        sprintf(string,"%0.2f",stime);
        SetDlgItemText(hDlg,SCAN_TIME,string);
      }
      if(SD->scan_dir != BACKWARD_DIR)
      {
        update_line(j,0);
/*
		sprintf(string,"%d %d %d %d",dest_x,dest_y,dest_width,dest_height);
		MessageBox(hDlg, string, "Test", MB_ICONEXCLAMATION);
*/
#ifdef STRETCH
        StretchDIBits(hDC,dest_x + dest_offset_x,dest_y + dest_offset_y, 
                dest_width,dest_height,src_x,src_y,src_width,src_height, 
                bitmap_data,bitmapinfo,DIB_PAL_COLORS,SRCCOPY);
#else
        SetDIBitsToDevice(hDC,dest_x + dest_offset_x,dest_y + dest_offset_y,
                src_width,src_height,src_x,src_y,0,src_height, 
                bitmap_data,bitmapinfo,DIB_PAL_COLORS);
#endif
        if (SD->x_first)
        {
          dest_y -= scan_display_factor;
        }
        else dest_x += scan_display_factor;
      }
#ifdef TIME_ERROR
      if((inpw(stat) & 0x4000)) time_error_line++;
#endif
      dio_wait_clock();
      if(SD->scan_dir == BOTH_DIR1)
      {
        for(i = 0;i < n;i++)
        {
          x++;
          dio_out(second_ch,*x);
          dio_start_clock(SD->inter_step_delay);
#ifdef TIME_ERROR
          if((inpw(stat) & 0x4000)) time_error_inter++;
#endif
          dio_wait_clock();
          if(SD->digital_feedback)
		  {
            if(digital_abort) stop_now = !dio_digital_feedback(Imin,Imax);
            else dio_digital_feedback(Imin,Imax);
            dio_in_ch(read_ch);
		  }
		}
            
      }
      for(k = 0;k < scan_size && !stop_now;k++) /* move y back */
      {
        if(SD->scan_dir == BACKWARD_DIR || SD->scan_dir == BOTH_DIR1 ||
                SD->scan_dir == BOTH_DIR2)
        {
          dio_start_clock(SD->step_delay);
#ifdef TIME_ERROR
          if((inpw(stat) & 0x4000)) time_error_step++;
#endif
          dio_wait_clock();
//          z = dio_read(scan_num) + delta_z_offset * delta_z_factor;
          z = dio_read(scan_num);
          if(z + delta_z > max_z)
            max_z = z + delta_z;
          if(z + delta_z < min_z)
            min_z = z + delta_z;
          if(SD->crash_protection == CRASH_MINMAX_Z)
          {
            if(k == 0)
            {
              if(j == 0)
              {
                this_line_min = this_line_max = last_line_min =
                                last_line_max = z;
              }
              else
              {
                last_line_max = this_line_max;
                last_line_min = this_line_min;
                this_line_min = this_line_max=z;
              }
            }
            else
            {
              if(z > this_line_max) this_line_max = z;
              if(z < this_line_min) this_line_min = z;
            }
            temp_min = min(this_line_min,last_line_min);
            temp_max = max(this_line_max,last_line_max);
            temp_range = temp_max - temp_min;
            if(temp_range > IN_MAX)
            {
              low_target = DEFAULT_Z_OFFSET_MOVE * low_limit;
              high_target = IN_MAX - (unsigned int)((float)IN_MAX * SD->z_limit_percent *
										DEFAULT_Z_OFFSET_MOVE / 100);
            }
            else
            {
              low_target = (IN_MAX - temp_range) / 2;
              high_target = IN_MAX - (IN_MAX - temp_range) / 2;
            }
          }
          if(z > high_limit || z < low_limit)
          {
            if(SD->crash_protection == CRASH_STOP)
            {
              stop_now = 1;
              break;
            }
            else if(SD->crash_protection == CRASH_AUTO_Z ||
                             SD->crash_protection == CRASH_MINMAX_Z)
            {
              old_scan_z = z;
              if(z > high_limit)
              {
                auto_z_below(high_target);
              }
              else
              {
                auto_z_above(low_target);
              }
              dio_in_ch(read_ch);
              z = dio_read(scan_num);
              delta_z += old_scan_z-z;
//              delta_z_offset+=scan_z-old_scan_z;
            }
          }
        }
        if(k < scan_size - 1)
        {
          for(i = 0;i < n - 1; i++)
          {
            y--;
            dio_out(first_ch,*y);
            dio_start_clock(SD->inter_step_delay);
#ifdef TIME_ERROR
            if((inpw(stat) & 0x4000)) time_error_inter++;
#endif
            dio_wait_clock();
            if(SD->digital_feedback)
            {
              if(digital_abort) stop_now = !dio_digital_feedback(Imin,Imax);
              else dio_digital_feedback(Imin,Imax);                        
              dio_in_ch(read_ch);
            }
          }
          y--;
          dio_out(first_ch,*y);
          if(SD->scan_dir == FORWARD_DIR)
          {
            dio_start_clock(SD->inter_step_delay);
#ifdef TIME_ERROR
            if((inpw(stat) & 0x4000)) time_error_inter++;
#endif
            dio_wait_clock();
            if(SD->digital_feedback)
            {
              if(digital_abort) stop_now = !dio_digital_feedback(Imin,Imax);
              else dio_digital_feedback(Imin,Imax);                        
              dio_in_ch(read_ch);
            }
          }
        }
        if(SD->scan_dir == BACKWARD_DIR)
	      if(SD->x_first)
            *(data->ptr + j * scan_size + scan_size - k - 1) = z + delta_z;
          else
            *(data->ptr + (scan_size - k - 1) * scan_size + j) = z + delta_z;
        else if(SD->scan_dir == BOTH_DIR1)
          if(SD->x_first)
            *(data->ptr + (2 * j + 1) * scan_size + scan_size - k - 1) = z + delta_z;
          else
            *(data->ptr + (scan_size - k - 1) * scan_size + 2 * j + 1) = z + delta_z;
        else if(SD->scan_dir == BOTH_DIR2)
          if(SD->x_first)
            *(data2->ptr + j * scan_size + scan_size - k - 1) = z + delta_z;
          else
            *(data2->ptr + (scan_size - k - 1) * scan_size + j) = z + delta_z;
      
      } // for(k = 0...)
      for(i = 0;i < n ;i++)
      {
        x++;
        dio_out(second_ch,*x);
        dio_start_clock(SD->inter_step_delay);
#ifdef TIME_ERROR
        if((inpw(stat) & 0x4000)) time_error_inter++;
#endif
        dio_wait_clock();
        if(SD->digital_feedback)
        {
          if(digital_abort) stop_now = !dio_digital_feedback(Imin,Imax);
          else dio_digital_feedback(Imin,Imax);                
          dio_in_ch(read_ch);
        }
      }
      if(stop_now) break;
      if(SD->scan_dir == BACKWARD_DIR || SD->scan_dir == BOTH_DIR1)
      {
        if(this_line == sharp_lines)
        {
          if(this_bias) ramp_bias((unsigned int)sharp_bias1,BIAS_RAMP_TIME,BIAS_RAMP_SKIP,1);
          else ramp_bias((unsigned int)sharp_bias2,BIAS_RAMP_TIME,BIAS_RAMP_SKIP,1);
//          MessageBox(hDlg,"2nd","Test",MB_ICONEXCLAMATION);
          this_bias = !this_bias;
          this_line = 0;
        }
        this_line++;
        if(abs(sharp_bias1 - sharp_bias2) * BIAS_RAMP_TIME > SD->inter_line_delay)
        {
          dio_start_clock(5);
        }
        else 
          dio_start_clock(SD->inter_line_delay -
                    abs(sharp_bias1 - sharp_bias2) * BIAS_RAMP_TIME);
      }
      else dio_start_clock(SD->inter_line_delay);
      stime -= time_per_line;
      sprintf(string,"%0.2f",stime);
      SetDlgItemText(hDlg,SCAN_TIME,string);
      if(SD->scan_dir == BACKWARD_DIR || SD->scan_dir == BOTH_DIR1)
      {
        update_line(j,1);
#ifdef STRETCH
        StretchDIBits(hDC,dest_x + dest_offset_x,dest_y + dest_offset_y,
                dest_width,dest_height,src_x,src_y,src_width,src_height,
                bitmap_data,bitmapinfo,DIB_PAL_COLORS,SRCCOPY);
#else
        SetDIBitsToDevice(hDC,dest_x + dest_offset_x,dest_y + dest_offset_y,
                src_width,src_height,src_x,src_y,0,src_height,
                bitmap_data,bitmapinfo,DIB_PAL_COLORS);
#endif
        if(SD->x_first)
        {
          dest_y -= scan_display_factor;
        }
        else dest_x += scan_display_factor;
      }   
#ifdef TIME_ERROR
      if((inpw(stat) & 0x4000)) time_error_line++;
#endif
      dio_wait_clock();
    }

    ramp_bias((unsigned int)sample_bias,BIAS_RAMP_TIME,BIAS_RAMP_SKIP,1);
    if(stop_now)
    {
      sprintf(string,"Crash Protection! Scanning terminated.");
      MessageBox(hDlg, string,"Warning",MB_ICONEXCLAMATION);
      z_offset_crashed = 0;
      switch(scan_scale)
      {
        case 0:
          scan_freq -= 2;
          if(scan_freq < 0) scan_freq = 0;
          if(auto_z_above(IN_ZERO))
          {
            scan_scale = 1;
            calc_gains(&x_gain,&y_gain,&z_gain,&z2_gain);
            set_gain(x_gain,y_gain,z_gain,z2_gain);
            z_offset_crashed = !auto_z_above(IN_ZERO);
          }
          else 
          {
            z_offset_crashed = 1;
          }
          break;
        case 1:
          scan_freq -= 1;
          if(scan_freq < 0) scan_freq = 0;
          z_offset_crashed = !auto_z_above(IN_ZERO);
          break;
      }
      if(z_offset_crashed)
      {
        MessageBox(hDlg,"Could not change Z-gain to 10!","Warning", 
                MB_ICONEXCLAMATION);
        /* move back with no digital feedback */
        move_to_speed(first_ch,*y, 
                dac_data[first_ch],SD->inter_step_delay,0,Imin,Imax,
                SD->digital_feedback_max,SD->digital_feedback_reread);
        move_to_speed(second_ch,*x, 
                dac_data[second_ch],SD->inter_step_delay,0,Imin,Imax,
                SD->digital_feedback_max,SD->digital_feedback_reread);
      }
      else
      {
        scan_scale = 2;
        calc_gains(&x_gain,&y_gain,&z_gain,&z2_gain);
        set_gain(x_gain,y_gain,z_gain,z2_gain);
        move_to_speed(first_ch,*y, 
                dac_data[first_ch],SD->inter_step_delay,SD->digital_feedback,Imin,Imax,
                SD->digital_feedback_max,SD->digital_feedback_reread);
        move_to_speed(second_ch,*x, 
                dac_data[second_ch],SD->inter_step_delay,SD->digital_feedback,Imin,Imax,
                SD->digital_feedback_max,SD->digital_feedback_reread);
      }
    }
    else
    {
      move_to_speed(first_ch,*y, 
            dac_data[first_ch],SD->inter_step_delay,SD->digital_feedback,Imin,Imax,
            SD->digital_feedback_max,SD->digital_feedback_reread);
        move_to_speed(second_ch,*x, 
            dac_data[second_ch],SD->inter_step_delay,SD->digital_feedback,Imin,Imax,
            SD->digital_feedback_max,SD->digital_feedback_reread);
	}
//  _enable();
/*
    MessageBox(hDlg,"Test 1","Test Title",MB_OKCANCEL|MB_ICONEXCLAMATION);
    _enable();
    MessageBox(hDlg,"Test 2","Test Title",MB_OKCANCEL|MB_ICONEXCLAMATION);
*/
#ifdef TIME_ERROR
    sprintf(string,"%d %d %d",time_error_line,time_error_step,time_error_inter);
    MessageBox(hDlg,string,"Errors",MB_OKCANCEL|MB_ICONEXCLAMATION);
    SetDlgItemInt(hDlg, SCAN_STEP_ANGS_EDIT, time_error_line, FALSE);
#endif
    stair_free(para_ptr);
    data->valid = 1;
    data->x = scan_x;
    data->y = scan_y;
    data->z = scan_z;
    data->x_range = x_range;
    data->y_range = y_range;
    data->min_z = min_z;
    data->max_z = max_z;
    data->clip_min = min_z;
    data->clip_max = max_z;
    data->step = scan_step;
    data->size = scan_size;
    data->x_gain = scan_x_gain;
    data->y_gain = scan_y_gain;
    data->z_gain=calc_z_gain(scan_scale);
    data->z_freq=scan_freq;
    scan_time = time(NULL);
    strcpy(string, ctime(&scan_time));
    string[strlen(string)-1] = '\0';
    strcpy(data->comment.ptr, string);
    data->comment.size = strlen(string);
/*
    strcpy(data->sample_type.ptr, sample_type);
    data->sample_type.size = strlen(sample_type);
*/
    if(SD->scan_dir == BOTH_DIR2)
    {
      data2->valid = 1;
      data2->x = scan_x;
      data2->y = scan_y;
      data2->z = scan_z;
      data2->x_range = x_range;
      data2->y_range = y_range;
      data2->min_z = min_z;
      data2->max_z = max_z;
      data2->clip_min = min_z;
      data2->clip_max = max_z;
      data2->step = scan_step;
      data2->size = scan_size;
      data2->x_gain = scan_x_gain;
      data2->y_gain = scan_y_gain;
      data2->z_freq = scan_freq;
      data2->z_gain = calc_z_gain(scan_scale);
      strcpy(data2->comment.ptr, data->comment.ptr);
      data2->comment.size = strlen(data2->comment.ptr);
      strcpy(data2->sample_type.ptr, data->sample_type.ptr);
      data2->sample_type.size = data->sample_type.size;
    }
    ReleaseDC(hDlg, hDC);
}

int pre_scan_old(HWND hDlg)
{
  int i;
  unsigned int x_gain,
               y_gain,
               z_gain,
               z2_gain;
  float point1 = (float)0.1;
  float time,first_gain,second_gain;
  int Imin,Imax,mini,overshoot_offset_fast,overshoot_offset_slow;
  unsigned int first_ch,second_ch,first_offset,second_offset;
  unsigned int moved_value;

  // initialization
  wait_cursor();
  save_init(SCAN_SETTINGS);
  calc_gains(&x_gain,&y_gain,&z_gain,&z2_gain);
  //set_gain(x_gain,y_gain,z_gain,z2_gain);
  set_gain_serial(x_gain,y_gain,z_gain,z2_gain);
  time = calc_total_time();
  if(SD->x_first)
  {
    first_ch = x_ch;
    first_offset = x_offset_ch;
    second_ch = y_ch;
    second_offset = y_offset_ch;
    first_gain = scan_x_gain;
    second_gain = scan_y_gain;
  } else
  {
    first_ch = y_ch;
    first_offset = y_offset_ch;
    second_ch = x_ch;
    second_offset = x_offset_ch;
    first_gain = scan_y_gain;
    second_gain = scan_x_gain;
  }
  if(num_data() > 1 || scan_max_data > 1)
  {
    if(num_data() > scan_max_data)
    {
      for(i = scan_max_data;i < num_data();i++)
      {
        alloc_data(&(all_data[i]),DATATYPE_3D,
                    DATA_MAX,GEN2D_NONE,GEN2D_NONE,SD->read_seq_num);
      }
      scan_max_data = num_data();
    }
    else
    {
      for(i = max(num_data(),1);i < scan_max_data;i++) free_data(&(all_data[i]));
      scan_max_data=max(num_data(),1);
    }
  }
  data = all_data[0];
  scan_current_data = 0;
  repaint_scan_current_data(hDlg);
            
  // paranoia - set some parameters
  //bias(sample_bias);
  bias_serial(sample_bias);
  //tip_current(i_setpoint);
  tip_current_serial(i_setpoint);
  //mode(fine_mode);
  mode_serial(fine_mode);
  //mode(translate_mode);
  mode_serial(translate_mode);

  Imax = in_vtod(pow(10,SD->tip_spacing - dtov(i_setpoint,i_set_range) / 10.0));
  Imin = in_vtod(pow(10,-SD->tip_spacing - dtov(i_setpoint,i_set_range) / 10.0));

  //dio_feedback(SD->scan_feedback);
  dio_feedback_serial(SD->scan_feedback);
  dither0_before=dio_dither_status(0);
  dither1_before=dio_dither_status(1);
  //dio_dither(0,SD->scan_dither0);
  dio_dither_serial(0,SD->scan_dither0);
  //dio_dither(1,SD->scan_dither1);
  dio_dither_serial(1,SD->scan_dither1);

#ifdef OLD
  if(scan_dither0 || scan_dither1) 
  {
    dio_start_clock(dither_wait);
    dio_wait_clock();
  }
#endif
        
  scan_status(hDlg,"Zeroing Fine X and Y",-1.0);

  dac_data[x_ch] = move_to_protect2(x_ch,dac_data[x_ch], 
        ZERO,SD->inter_step_delay,SD->crash_protection,
        SD->z_limit_percent,SD->digital_feedback,Imin,Imax,digital_abort,0,
        SD->digital_feedback_max,SD->digital_feedback_reread);

  if(dac_data[x_ch] != ZERO)
  {
    MessageBox(hDlg, "Could not zero X. Aborting.", "Warning", 
            MB_ICONEXCLAMATION);
    return(0);
  }
  scan_fine_x = ZERO;
  repaint_scan_x(hDlg);

  dac_data[y_ch] = move_to_protect2(y_ch,dac_data[y_ch], 
        ZERO,SD->inter_step_delay,SD->crash_protection,
        SD->z_limit_percent,SD->digital_feedback,Imin,Imax,digital_abort,0,
        SD->digital_feedback_max,SD->digital_feedback_reread);

  if(dac_data[y_ch] != ZERO)
  {
    MessageBox(hDlg,"Could not zero Y. Aborting.","Warning", 
            MB_ICONEXCLAMATION);
    return(0);
  }
  scan_fine_y = ZERO;
  repaint_scan_y(hDlg);
  scan_status(hDlg,"Synching offsets",-1.0);

  dac_data[x_offset_ch] = move_to_protect2(x_offset_ch,dac_data[x_offset_ch], 
        scan_x,10 / scan_x_gain * SD->inter_step_delay,SD->crash_protection,
        SD->z_limit_percent,SD->digital_feedback,Imin,Imax,digital_abort,0,
        SD->digital_feedback_max,SD->digital_feedback_reread);

  if(dac_data[x_offset_ch] != scan_x)
  {
    MessageBox(hDlg, "Could not sync X Offset. Aborting.","Warning", 
      MB_ICONEXCLAMATION);
    return(0);
  }

  dac_data[y_offset_ch] = move_to_protect2(y_offset_ch,dac_data[y_offset_ch], 
        scan_y,10 / scan_y_gain * SD->inter_step_delay,SD->crash_protection,
        SD->z_limit_percent,SD->digital_feedback,Imin,Imax,digital_abort,0,
        SD->digital_feedback_max,SD->digital_feedback_reread);

  if(dac_data[y_offset_ch] != scan_y)
  {
    MessageBox(hDlg, "Could not sync Y Offset. Aborting.","Warning", 
            MB_ICONEXCLAMATION);
    return(0);
  }

  move_to_timed(z_offset_ch,dac_data[z_offset_ch],scan_z,DEFAULT_Z_OFFSET_TIME);
  dac_data[z_offset_ch] = scan_z;
  scan_status(hDlg,"Moving in the slow direction.",-1.0);
    
  /* Move to starting position */
  if((moved_value = move_to_protect2(second_ch,dac_data[second_ch],
        ZERO - scan_step * scan_size / 2, SD->inter_step_delay,
        SD->crash_protection,SD->z_limit_percent,SD->digital_feedback,
        Imin,Imax,digital_abort,0,SD->digital_feedback_max,
        SD->digital_feedback_reread)) !=
        ZERO - scan_step * scan_size / 2)
  {
    MessageBox(hDlg,"Could not move in slow direction!\nForcing it back and aborting.", 
        "Warning",MB_ICONEXCLAMATION);
    move_to_protect2(second_ch,moved_value,dac_data[second_ch], 
            SD->inter_step_delay,
            SD->crash_protection,SD->z_limit_percent,SD->digital_feedback,Imin,
            Imax,digital_abort,1,SD->digital_feedback_max,SD->digital_feedback_reread);
    return(0);
  }
        
  time -= calc_initial_time() / 2;
  sprintf(string,"%0.2f",time);
  SetDlgItemText(hDlg,SCAN_TIME,string);
    
  if((SD->overshoot & OVERSHOOT_SLOW))
  {
    mini = floor(scan_step * scan_size / 2 * second_gain / OFFSET_GAIN + 0.5);
    overshoot_offset_slow = max(dac_data[second_offset] -
            scan_step * scan_size / 2 * 
            second_gain / OFFSET_GAIN * SD->overshoot_percent / 100,mini);
    if(overshoot_offset_slow < dac_data[second_offset])
    {
      scan_status(hDlg,"Overshooting by %0.2f%% in the slow dir",
                (float)(dac_data[second_offset] - overshoot_offset_slow) *
                100 / (float) mini);
      moved_value = move_to_protect2(second_offset,dac_data[second_offset], 
                overshoot_offset_slow,10 / second_gain * SD->inter_step_delay,
                SD->crash_protection,SD->z_limit_percent,SD->digital_feedback,Imin,Imax,
                digital_abort,0,SD->digital_feedback_max,SD->digital_feedback_reread);
      if(moved_value != overshoot_offset_slow)
      {
        MessageBox(hDlg,"Could not overshoot in slow direction!\nForcing it back and aborting.", 
                "Warning",MB_ICONEXCLAMATION);
        move_to_protect2(second_offset,moved_value,
                    dac_data[second_offset],10 / second_gain * SD->inter_step_delay,
                    SD->crash_protection,SD->z_limit_percent,
                    SD->digital_feedback,Imin,Imax,digital_abort,1,
                    SD->digital_feedback_max,SD->digital_feedback_reread);
        move_to_protect2(second_ch,ZERO - scan_step * scan_size / 2 ,
                    dac_data[second_ch], SD->inter_step_delay,SD->crash_protection,
                    SD->z_limit_percent,SD->digital_feedback,Imin,Imax,digital_abort,1,
                    SD->digital_feedback_max,SD->digital_feedback_reread);
        return(0);
      }
    }
    time -= calc_overshoot_slow_time();
    sprintf(string,"%0.2f",time);
    SetDlgItemText(hDlg,SCAN_TIME,string);
                        
    scan_status(hDlg,"Waiting %0.0f seconds at overshot location",
            (float)SD->overshoot_wait1);
    for(i = 0;i < SD->overshoot_wait1;i++)
    {
      dio_start_clock(1000000);
      time -= 1;
      sprintf(string,"%0.2f",time);
      dio_wait_clock();
      SetDlgItemText(hDlg,SCAN_TIME,string);
    }
    if(overshoot_offset_slow < dac_data[second_offset])
    {
      scan_status(hDlg,"Moving overshot slow direction back...",-1.0);
      moved_value = move_to_protect2(second_offset,overshoot_offset_slow, 
                dac_data[second_offset], 10 / second_gain * SD->inter_step_delay,
                SD->crash_protection,SD->z_limit_percent,SD->digital_feedback,Imin,Imax,digital_abort,0,
                SD->digital_feedback_max,SD->digital_feedback_reread);
      if(moved_value != dac_data[second_offset])
      {
        MessageBox(hDlg,"Could not return overshoot in slow direction!\nForcing it back and aborting.", 
                "Warning",MB_ICONEXCLAMATION);
        move_to_protect2(second_offset,moved_value,
                    dac_data[second_offset],10 / second_gain * SD->inter_step_delay,
                    SD->crash_protection,SD->z_limit_percent,
                    SD->digital_feedback,Imin,Imax,digital_abort,1,
                    SD->digital_feedback_max,SD->digital_feedback_reread);
        move_to_protect2(second_ch,ZERO - scan_step * scan_size / 2 ,
                    dac_data[second_ch], SD->inter_step_delay,SD->crash_protection,
                    SD->z_limit_percent,SD->digital_feedback,Imin,Imax,digital_abort,1,
                    SD->digital_feedback_max,SD->digital_feedback_reread);
        return(0);
      }
                
      time -= calc_overshoot_slow_time();
      sprintf(string,"%0.2f",time);
      SetDlgItemText(hDlg, SCAN_TIME, string);
    }
                        
    scan_status(hDlg,"Waiting %0.0f seconds at image edge",
            (float)SD->overshoot_wait2);
    for(i=0;i<SD->overshoot_wait2;i++)
    {
      dio_start_clock(1000000);
      time -= 1;
      sprintf(string,"%0.2f",time);
      dio_wait_clock();
      SetDlgItemText(hDlg,SCAN_TIME,string);
    }
  }
  scan_status(hDlg,"Moving in the fast direction.",-1.0);
  if((moved_value = move_to_protect2(first_ch,dac_data[first_ch], 
        ZERO - scan_step * scan_size / 2, SD->inter_step_delay,
        SD->crash_protection,SD->z_limit_percent,SD->digital_feedback,
        Imin,Imax,digital_abort,0,
        SD->digital_feedback_max,SD->digital_feedback_reread)) !=
        ZERO- scan_step*scan_size / 2)
  {
    MessageBox(hDlg, "Could not move in fast direction!\nForcing it back and aborting.", 
        "Warning", MB_ICONEXCLAMATION);
    move_to_protect2(first_ch,moved_value,dac_data[first_ch], 
            SD->inter_step_delay,
            SD->crash_protection,SD->z_limit_percent,SD->digital_feedback,Imin,Imax,
            digital_abort,1,SD->digital_feedback_max,SD->digital_feedback_reread);
    move_to_protect2(second_ch,ZERO - scan_step * scan_size / 2 ,
            dac_data[second_ch], SD->inter_step_delay,SD->crash_protection,
            SD->z_limit_percent,SD->digital_feedback,Imin,Imax,digital_abort,1,
            SD->digital_feedback_max,SD->digital_feedback_reread);
    return(0);
  }
  if((SD->overshoot & OVERSHOOT_FAST))
  {
    mini =floor(scan_step * scan_size / 2 * first_gain / OFFSET_GAIN + 0.5);
    overshoot_offset_fast = max(dac_data[first_offset] -
            scan_step * scan_size / 2 * 
            first_gain / OFFSET_GAIN * SD->overshoot_percent / 100,mini);
    if(overshoot_offset_fast < dac_data[first_offset])
    {
      scan_status(hDlg,"Overshooting by %0.2f%% in the fast dir",
                (float) (dac_data[first_offset]-overshoot_offset_fast) *
                    100 / (float) mini);
      moved_value = move_to_protect2(first_offset,dac_data[first_offset], 
                overshoot_offset_fast,10 / first_gain * SD->inter_step_delay,
                SD->crash_protection,SD->z_limit_percent,SD->digital_feedback,Imin,Imax,
                digital_abort,0,SD->digital_feedback_max,SD->digital_feedback_reread);
      if(moved_value != overshoot_offset_fast)
      {
        MessageBox(hDlg, "Could not overshoot in fast direction!\nForcing it back and aborting.", 
                "Warning", MB_ICONEXCLAMATION);
        move_to_protect2(first_offset, moved_value,
                    dac_data[first_offset],10 / first_gain * SD->inter_step_delay,
                    SD->crash_protection,SD->z_limit_percent,
                    SD->digital_feedback,Imin,Imax,digital_abort,1,
                    SD->digital_feedback_max,SD->digital_feedback_reread);
        move_to_protect2(first_ch,ZERO - scan_step * scan_size / 2 ,
                    dac_data[first_ch], SD->inter_step_delay,SD->crash_protection,
                    SD->z_limit_percent,SD->digital_feedback,Imin,Imax,digital_abort,1,
                    SD->digital_feedback_max,SD->digital_feedback_reread);
        move_to_protect2(second_ch,ZERO - scan_step * scan_size / 2 ,
                    dac_data[second_ch], SD->inter_step_delay,SD->crash_protection,
                    SD->z_limit_percent,SD->digital_feedback,Imin,Imax,digital_abort,1,
                    SD->digital_feedback_max,SD->digital_feedback_reread);
        return(0);
      }
    }
    time -= calc_overshoot_fast_time();
    sprintf(string,"%0.2f",time);
    SetDlgItemText(hDlg, SCAN_TIME, string);
                        
    scan_status(hDlg,"Waiting %0.0f seconds at overshot location",
            (float)SD->overshoot_wait1);
    for(i = 0;i < SD->overshoot_wait1;i++)
    {
      dio_start_clock(1000000);
      time -= 1;
      sprintf(string,"%0.2f",time);
      dio_wait_clock();
      SetDlgItemText(hDlg,SCAN_TIME,string);
    }
    if(overshoot_offset_fast < dac_data[first_offset])
    {
      scan_status(hDlg,"Moving overshot fast direction back...",-1.0);
      move_to_speed(first_offset,overshoot_offset_fast,dac_data[first_offset],
                10 / first_gain * SD->inter_step_delay,SD->digital_feedback,Imin,Imax,
                SD->digital_feedback_max,
                SD->digital_feedback_reread);
      moved_value = move_to_protect2(first_offset,overshoot_offset_fast,
                dac_data[first_offset],10 / first_gain * SD->inter_step_delay,
                SD->crash_protection,SD->z_limit_percent,SD->digital_feedback,Imin,Imax,
                digital_abort,0,SD->digital_feedback_max,SD->digital_feedback_reread);
      if(moved_value != dac_data[first_offset])
      {
        MessageBox(hDlg, "Could not return in overshot fast direction!\nForcing it back and aborting.", 
                "Warning", MB_ICONEXCLAMATION);
        move_to_protect2(first_offset,moved_value,
                    dac_data[first_offset],10 / first_gain * SD->inter_step_delay,
                    SD->crash_protection,SD->z_limit_percent,
                    SD->digital_feedback,Imin,Imax,digital_abort,1,
                    SD->digital_feedback_max,SD->digital_feedback_reread);
        move_to_protect2(first_ch,ZERO - scan_step * scan_size / 2 ,
                    dac_data[first_ch], SD->inter_step_delay,SD->crash_protection,
                    SD->z_limit_percent,SD->digital_feedback,Imin,Imax,digital_abort,1,
                    SD->digital_feedback_max,SD->digital_feedback_reread);
        move_to_protect2(second_ch,ZERO - scan_step * scan_size / 2 ,
                    dac_data[second_ch], SD->inter_step_delay,SD->crash_protection,
                    SD->z_limit_percent,SD->digital_feedback,Imin,Imax,digital_abort,1,
                    SD->digital_feedback_max,SD->digital_feedback_reread);
        return(0);
      }
      time -= calc_overshoot_fast_time();
      sprintf(string,"%0.2f",time);
      SetDlgItemText(hDlg, SCAN_TIME, string);
    }
    scan_status(hDlg,"Waiting %0.0f seconds at image edge",
            (float)SD->overshoot_wait2);
    for(i = 0;i < SD->overshoot_wait2;i++)
    {
      dio_start_clock(1000000);
      time -= 1;
      sprintf(string,"%0.2f",time);
      dio_wait_clock();
      SetDlgItemText(hDlg,SCAN_TIME,string);
    }
                        
  }
    
  return(1); /* success */
}

float calc_seq_time_old()
{
  int i;
  unsigned int pre_bias = sample_bias;
  float time = (float)SD->step_delay;
  int feedback,dither0,dither1;

  feedback = SD->scan_feedback;
  dither0 = SD->scan_dither0;
  dither1 = SD->scan_dither1;
        
  for(i = 0;i < SD->read_seq_num;i++)
  {
    time += SD->read_seq[i].wait1;
    time += SD->read_seq[i].wait2;
    time += SD->read_seq[i].wait3;
    time += SD->read_seq[i].wait4;
    if(feedback != SD->read_seq[i].feedback)
    {
      feedback = SD->read_seq[i].feedback;
      time += dio_out_time;
    }
    if(dither0 != SD->read_seq[i].dither0)
      {
        dither0 = SD->read_seq[i].dither0;
        time += dio_out_time;
      }
      if(dither1 != SD->read_seq[i].dither1)
      {
        dither1 = SD->read_seq[i].dither1;
        time += dio_out_time;
      }
	  if(SD->read_seq[i].do_ramp_bias)
	  {
	    time += abs(SD->read_seq[i].ramp_value - pre_bias) * dio_out_time;
	    pre_bias = SD->read_seq[i].ramp_value;
	  }
      if (SD->read_seq[i].record)
      {
            time += one_input_time * SD->read_seq[i].num_samples;
      }
    }
    if(feedback != SD->scan_feedback)
    {
      feedback = SD->scan_feedback;
      time += dio_out_time;
    }
    if(dither0 != SD->scan_dither0)
    {
      dither0 = SD->scan_dither0;
      time += dio_out_time;
    }
    if(dither1 != SD->scan_dither1)
    {
      dither1 = SD->scan_dither1;
      time += dio_out_time;
    }
    return(time);
}
    
#endif