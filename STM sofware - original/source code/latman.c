#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include "common.h"
#include "stm.h"
#include "file.h"
#include "dio.h"
#include "data.h"
#include "pre_scan.h"
#include "scan.h"
#include "image.h"
#include "clock.h"
#include "print.h"
#include "serial.h"
#include "latman.h"

datadef *manip_data;
int manip_size;
unsigned int lm_image_bias,lm_manip_bias;	// bias during imaging, manipulation phases
unsigned int lm_image_i,lm_manip_i;			// i setpoint during imaging, manipulation phases
double lm_image_i_set,lm_manip_i_set;
int lm_ramp_delay;
int lm_read_delay,lm_interstep_delay;		// delays during manipulation phase
unsigned int lm_measure_every;				// how often to measure z during manip. phase
int lm_initial_x,lm_initial_y;				// initial coordinates of manipulation
int lm_final_x,lm_final_y;					// final coordinates of manipulation
int lm_read_z;								// boolean: true if we read z during manip.
STM_COLOR lm_line_color;					// color of line
float lm_bits_per_pix;
int lm_read_n_times;						// number of times to read z at each data
											// point on path
int manipulating;                           // set to 1 when manip starts, to 0 if stop button
											// pressed
unsigned int lm_entry_bias;					// latman remembers entry bias and current...
unsigned int lm_entry_i;					// ... so that it can restore them later
int lm_mode;								// const I or const Z?
int lm_z_dir;								// 1 for inc z offset during const height, 0 for dec
float lm_z_slope;
int lm_calc_slope = 1;						// set to automatically calculate z off slope from data
int lm_fback_off = 0;						// option to keep fback off after const z manip

HPEN hpen_white,hpen_blue,hpen_red;
float lm_vert_max,lm_vert_min;				// max and min values of graph

extern unsigned int bit16;
extern unsigned int sample_bias;
extern unsigned int i_setpoint;
extern double i_set;
extern int sample_bias_range;
extern int i_set_range;
extern unsigned int tip_gain;
extern int scan_freq;
extern float scan_z_gain;
extern float scan_x_gain;
extern float scan_y_gain;
extern int x_range;
extern int y_range;
extern int x_offset_range;
extern int y_offset_range;
extern unsigned int scan_x,scan_y;
extern unsigned int scan_z;
extern int datapro;

extern char string[];
extern char *current_file_cut;
extern int file_cut_count;

extern datadef *data,**glob_data;
extern BITMAPINFO *bitmapinfo;
extern HANDLE hInst;
extern HPALETTE hPal;
extern unsigned char *bitmap_data;
extern unsigned int dac_data[];
extern unsigned int scan_fine_x,scan_fine_y;
extern unsigned int bit16;
extern struct commentdef *gcomment;

static int out_smart = 0;
static int first_time = 1;
static int captured = 0;					// are we drawing?

void lateral_manipulation(HWND);

static void repaint_lm_timing_params(HWND);
static void repaint_lm_path_params(HWND);
static void repaint_lm_arrow(HWND);
static void repaint_lm_bias_params(HWND);
static void repaint_lm_path_data(HWND);
static void repaint_lm_mode(HWND);
static void repaint_lm_status(HWND,char *);
static void set_manip_data();
static void get_manip_data();

extern HWND ScnDlg;
extern BOOL FAR PASCAL CommentDlg(HWND, unsigned, WPARAM, LPARAM);

#define PIX_TO_BITS_X(A) ZERO + (int)(((A) - (LM_DISPLAY_LEFT + LM_DISPLAY_WIDTH / 2)) \
                                                * lm_bits_per_pix)
#define PIX_TO_BITS_Y(A) ZERO + (int)(((LM_DISPLAY_TOP + LM_DISPLAY_HEIGHT / 2) - (A)) \
                                                * lm_bits_per_pix)

#define BITS_TO_PIX_X(A) (int)(((A) - ZERO) / lm_bits_per_pix) + \
                                                LM_DISPLAY_LEFT + LM_DISPLAY_WIDTH / 2
#define BITS_TO_PIX_Y(A) (int)((ZERO - (A)) / lm_bits_per_pix) + \
                                                LM_DISPLAY_TOP + LM_DISPLAY_HEIGHT / 2

#define BITS_TO_STEPS(A,B) (A - ZERO + B->size * B->step/2) / B->step
#define STEPS_TO_BITS(A,B) A * B->step + (ZERO - B->size*B->step/2)

BOOL FAR PASCAL LatManDlg(HWND,unsigned,WPARAM,LPARAM);
// Lateral manipulation dialog. For manipulating single atoms & molecules.
// Traces a straight path in the x-y plane at manipulating conditions (high I,
// low sample bias). Optionally records data during movement.

BOOL FAR PASCAL LatManDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{
  int value;
  float fvalue;
  int scroll_id;
  int mini,maxi,delt,ddelt,i,j;
  double d;
  double deltd,ddeltd,maxid,minid;
  float manip_min_z,manip_max_z;

  HDC hDC;
  PAINTSTRUCT ps;
  HPEN hPen,hPenOld;
  HBRUSH hBrush,hBrushOld;
  POINT mouse_pos;
  static FARPROC lpfnDlgProc;

  switch(Message)
  {
    case WM_INITDIALOG:

	  if(datapro)
	  {
	  EnableWindow(GetDlgItem(hDlg,LM_START),FALSE);
	  EnableWindow(GetDlgItem(hDlg,LM_STOP),FALSE);
	  }

      if(first_time)
	  {
        alloc_data(&manip_data,DATATYPE_GEN2D,MAX,GEN2D_NONE,GEN2D_FLOAT,1);
        set_manip_data();
        manip_data->valid = 0;
                
        lm_image_bias = sample_bias;
        lm_manip_bias = sample_bias;
        lm_read_delay = 1000;
        lm_ramp_delay = 1000;
        lm_interstep_delay = 1000;
        lm_measure_every = 1;
        lm_read_n_times = 10;
        lm_read_z = 1;
        lm_image_i = i_setpoint;
        lm_manip_i = i_setpoint;
        lm_image_i_set = i_set;
        lm_manip_i_set = i_set;
        lm_line_color.r = 0;
        lm_line_color.g = 0;
        lm_line_color.b = 0;
        lm_initial_x = lm_final_x = ZERO;
        lm_initial_y = lm_final_y = ZERO;
        manip_size = 0;
        lm_vert_min = (float)IN_MIN;
        lm_vert_max = (float)IN_MAX;
		lm_mode = LM_CONST_I;
		lm_z_slope = 0.0;
        first_time = 0;
	  }
	  else
	  {
		if(!bit16 && (lm_image_bias > 4095))
		{
	      sprintf(string,"%d",vtod(dtov16(lm_image_bias),get_range(sample_bias_ch)));
		  SetDlgItemText(hDlg,LM_IMAGE_BIAS_BITS,string);
		}
		if(!bit16 && (lm_manip_bias > 4095))
		{
	      sprintf(string,"%d",vtod(dtov16(lm_manip_bias),get_range(sample_bias_ch)));
		  SetDlgItemText(hDlg,LM_MANIP_BIAS_BITS,string);
		}
	  }
      if(data->valid)
	  {
        lm_bits_per_pix = (float)(data->size) * (float)(data->step) / (float)LM_DISPLAY_WIDTH;
	  }
      else
	  {
        lm_bits_per_pix = 1.0;
	  }
	  lm_entry_bias = sample_bias;
      lm_entry_i = i_setpoint;
      manipulating = 0;
      hpen_white = CreatePen(PS_SOLID,1,RGB(255,255,255));
      hpen_blue = CreatePen(PS_SOLID,1,RGB(0,0,255));
      hpen_red = CreatePen(PS_SOLID,1,RGB(255,0,0));
      SetScrollRange(GetDlgItem(hDlg,LM_IMAGE_BIAS_SCROLL),SB_CTL,sample_bias_min,sample_bias_max,TRUE);
      SetScrollRange(GetDlgItem(hDlg,LM_MANIP_BIAS_SCROLL),SB_CTL,sample_bias_min,sample_bias_max,TRUE);
      SetScrollRange(GetDlgItem(hDlg,LM_IMAGE_I_SCROLL),SB_CTL,tip_current_min,tip_current_max,TRUE);
      SetScrollRange(GetDlgItem(hDlg,LM_MANIP_I_SCROLL),SB_CTL,tip_current_min,tip_current_max,TRUE);
      SetScrollRange(GetDlgItem(hDlg,LM_V_SCALE_MAX_SCROLL),SB_CTL,0,32767,TRUE);
      SetScrollRange(GetDlgItem(hDlg,LM_V_SCALE_MIN_SCROLL),SB_CTL,0,32767,TRUE);
      SetFocus(GetDlgItem(hDlg,LM_EXIT));
      repaint_lm_arrow(hDlg); 
      repaint_lm_path_params(hDlg);
      repaint_lm_timing_params(hDlg);
      repaint_lm_path_data(hDlg);
      repaint_lm_bias_params(hDlg);
	  repaint_lm_mode(hDlg);
      repaint_lm_status(hDlg,"");
      if(manip_data->x_gain != scan_x_gain || manip_data->y_gain != scan_y_gain 
        || manip_data->x_range != x_range || manip_data->y_range != y_range
        || manip_data->x != scan_x || manip_data->y != scan_y)
      {
        MessageBox(hDlg,"Some settings may be different from scanned data!",
                   "Warning!",MB_OK);
      }      
      break;

    case WM_PAINT:
      hDC = BeginPaint(hDlg,&ps);

      // draw image rectangle, 256 x 256 pixels
      SetMapMode(hDC,MM_TEXT);
      hPen = GetStockObject(BLACK_PEN);
      hBrush = GetStockObject(WHITE_BRUSH);
      hBrushOld = SelectObject(hDC,hBrush);
      hPenOld = SelectObject( hDC,hPen);
      Rectangle(hDC,LM_DISPLAY_LEFT,LM_DISPLAY_TOP,LM_DISPLAY_RIGHT,LM_DISPLAY_BOTTOM);
      SelectObject(hDC,hBrushOld);
      SelectObject(hDC,hPenOld);

      // draw scanned image, if there is one
      if(data->valid)
	  {
        SelectPalette(hDC,hPal,0);
        RealizePalette(hDC);
        StretchDIBits(hDC,LM_DISPLAY_LEFT,LM_DISPLAY_TOP, 
          LM_DISPLAY_WIDTH,LM_DISPLAY_HEIGHT,0,0,data->size,data->size, 
          bitmap_data,bitmapinfo,DIB_PAL_COLORS,SRCCOPY);       
	  }
          
      // draw graph in lower box
      repaint_lm_arrow(hDlg);
      repaint_lm_path_data(hDlg);
 
      DeleteObject(hBrush);
      DeleteObject(hPen);
      EndPaint(hDlg,&ps);
      break;

    case WM_LBUTTONDOWN:
      // start drawing arrow
      mouse_pos.x = LOWORD(lParam);
      mouse_pos.y = HIWORD(lParam);

      if(pt_in_rect(LM_DISPLAY_LEFT,LM_DISPLAY_TOP,
                    LM_DISPLAY_LEFT + LM_DISPLAY_WIDTH,
                    LM_DISPLAY_TOP + LM_DISPLAY_HEIGHT,mouse_pos))
	  {

        SetCapture(hDlg);
        captured = 1;

        // convert mouse coords in pixels to bits
        // mouse coords: (0,0) in upper left of window
        // bits (2048,2048) is in center of image
        lm_initial_x = PIX_TO_BITS_X(mouse_pos.x);
        lm_initial_y = PIX_TO_BITS_Y(mouse_pos.y);
                
        // repaint
        repaint_lm_path_params(hDlg);
	  }
      break;

    case WM_LBUTTONUP:
      // let go of arrow
      mouse_pos.x = LOWORD(lParam);
      mouse_pos.y = HIWORD(lParam);
      mouse_pos.x = min(max(LM_DISPLAY_LEFT,mouse_pos.x),LM_DISPLAY_RIGHT);
      mouse_pos.y = min(max(LM_DISPLAY_TOP,mouse_pos.y),LM_DISPLAY_BOTTOM);

      if(captured)
	  {    
        ReleaseCapture();
        captured = 0;

        // convert mouse coords in pixels to bits
        // mouse coords: (0,0) in upper left of window
        // bits (2048,2048) is in center of image
        lm_final_x = PIX_TO_BITS_X(mouse_pos.x);
        lm_final_y = PIX_TO_BITS_Y(mouse_pos.y);
                
        // repaint
        repaint_lm_path_params(hDlg);
        repaint_lm_arrow(hDlg);
	  }
      break;

    case WM_MOUSEMOVE:
      // continue drawing arrow
      mouse_pos.x = LOWORD(lParam);
      mouse_pos.y = HIWORD(lParam);
      mouse_pos.x = min(max(LM_DISPLAY_LEFT,mouse_pos.x),LM_DISPLAY_RIGHT);
      mouse_pos.y = min(max(LM_DISPLAY_TOP,mouse_pos.y),LM_DISPLAY_BOTTOM);

      if(captured)
	  {    
        // convert mouse coords in pixels to bits
        // mouse coords: (0,0) in upper left of window
        // bits (2048,2048) is in center of image
        lm_final_x = PIX_TO_BITS_X(mouse_pos.x);
        lm_final_y = PIX_TO_BITS_Y(mouse_pos.y);
                
        // repaint
        repaint_lm_path_params(hDlg);
        repaint_lm_arrow(hDlg);
	  }
      break;

    case WM_HSCROLL:
    case WM_VSCROLL:
      if(!out_smart)
	  {
        out_smart = 1;
        scroll_id = getscrollid();
        switch(scroll_id)
		{
          case LM_IMAGE_BIAS_SCROLL:
            i = (int)lm_image_bias;
            mini = sample_bias_min;
            maxi = sample_bias_max;
            delt = sample_bias_delt;
            ddelt = sample_bias_ddelt;
            break;    
          case LM_MANIP_BIAS_SCROLL:
            i = (int)lm_manip_bias;
            mini = sample_bias_min;
            maxi = sample_bias_max;
            delt = sample_bias_delt;
            ddelt = sample_bias_ddelt;
            break;    
          case LM_IMAGE_I_SCROLL:
            i = (int)tip_current_max - lm_image_i;
            mini = tip_current_min;
            maxi = tip_current_max;
            delt = tip_current_delt;
            ddelt = tip_current_ddelt;
            break;
          case LM_MANIP_I_SCROLL:
            i = (int)tip_current_max - lm_manip_i;
            mini = tip_current_min;
            maxi = tip_current_max;
            delt = tip_current_delt;
            ddelt = tip_current_ddelt;
            break;
          case LM_V_SCALE_MIN_SCROLL:
            d = lm_vert_min;
            minid = IN_MIN;
            maxid = IN_MAX;
            deltd = IN_MAX / 100;
            ddeltd = IN_MAX / 10;
            break;
          case LM_V_SCALE_MAX_SCROLL:
            d = lm_vert_max;
            minid = IN_MIN;
            maxid = IN_MAX;
            deltd = IN_MAX / 100;
            ddeltd = IN_MAX / 10;
            break;
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
            break;
          case SB_THUMBTRACK:
            i = getscrollpos();
            d = ITOD(getscrollpos(),minid,maxid);
            break;
		}
        i = min(max(i,mini),maxi);
        d = min(max(d,minid),maxid);
        switch(scroll_id)
		{
          case LM_IMAGE_BIAS_SCROLL:
            lm_image_bias = i;
            break;
          case LM_MANIP_BIAS_SCROLL:
            lm_manip_bias = i;
            break;
          case LM_IMAGE_I_SCROLL:
            lm_image_i = tip_current_max - i;
            lm_image_i_set = calc_i_set(lm_image_i,i_set_range,tip_gain);
            break;
          case LM_MANIP_I_SCROLL:
            lm_manip_i = tip_current_max - i;
            lm_manip_i_set = calc_i_set(lm_manip_i,i_set_range,tip_gain);
            break;
          case LM_V_SCALE_MIN_SCROLL:
            lm_vert_min = (float)d;
            break;
          case LM_V_SCALE_MAX_SCROLL:
            lm_vert_max = (float)d;
            break;
		}
        repaint_lm_bias_params(hDlg);
        repaint_lm_path_data(hDlg);
        out_smart = 0; 
      }
      break;
    case WM_COMMAND:
      switch(LOWORD(wParam))
	  {

	    // Path coordinates
	    case LM_INITIAL_X:
          if(!out_smart)
		  {
            out_smart = 1;
            GetDlgItemText(hDlg,LM_INITIAL_X,string,9);
            value = atoi(string);
            // constrain to (1) 0-4095 bits (2) scan region
            lm_initial_x = min(max(value,MIN),MAX);
            lm_initial_x = min(max(lm_initial_x,PIX_TO_BITS_X(LM_DISPLAY_LEFT)),
                                                PIX_TO_BITS_X(LM_DISPLAY_RIGHT));
            out_smart = 0;
		  }
          break;
        case LM_INITIAL_Y:
          if(!out_smart)
		  {
            out_smart = 1;
            GetDlgItemText(hDlg,LM_INITIAL_Y,string,9);
            value = atoi(string);
            // constrain to (1) 0-4095 bits (2) scan region
            lm_initial_y = min(max(value,MIN),MAX);
            lm_initial_y = min(max(lm_initial_y,PIX_TO_BITS_X(LM_DISPLAY_TOP)),
                                                PIX_TO_BITS_X(LM_DISPLAY_BOTTOM));
            out_smart = 0;
		  }
          break;
        case LM_FINAL_X:
          if(!out_smart)
		  {
            out_smart = 1;
            GetDlgItemText(hDlg,LM_FINAL_X,string,9);
            value = atoi(string);
            // constrain to (1) 0-4095 bits (2) scan region
            lm_final_x = min(max(value,MIN),MAX);
            lm_final_x = min(max(lm_final_x,PIX_TO_BITS_X(LM_DISPLAY_LEFT)),
                                            PIX_TO_BITS_X(LM_DISPLAY_RIGHT));
            out_smart = 0;
		  }
          break;
        case LM_FINAL_Y:
          if(!out_smart)
		  {
            out_smart = 1;
            GetDlgItemText(hDlg,LM_FINAL_Y,string,9);
            value = atoi(string);
            // constrain to (1) 0-4095 bits (2) scan region
            lm_final_y = min(max(value,MIN),MAX);
            lm_final_y = min(max(lm_final_y,PIX_TO_BITS_X(LM_DISPLAY_TOP)),
                                            PIX_TO_BITS_X(LM_DISPLAY_BOTTOM));
            out_smart = 0;
		  }
          break;
        case LM_SWITCH:
          value = lm_initial_x;
          lm_initial_x = lm_final_x;
          lm_final_x = value;

          value = lm_initial_y;
          lm_initial_y = lm_final_y;
          lm_final_y = value;

          repaint_lm_path_params(hDlg);
          repaint_lm_arrow(hDlg);
          break;

		// Delays
        case LM_INTERSTEP_DELAY:
          if(!out_smart)
		  {
            out_smart = 1;
            GetDlgItemText(hDlg,LM_INTERSTEP_DELAY,string,9);
            value = atoi(string);
            lm_interstep_delay = min(max(value,INTER_STEP_MIN_DELAY),INTER_STEP_MAX_DELAY);
            out_smart = 0;
		  }
        case LM_RAMP_DELAY:
          if(!out_smart)
		  {
            out_smart = 1;
            GetDlgItemText(hDlg,LM_RAMP_DELAY,string,9);
            value = atoi(string);
            lm_ramp_delay = min(max(value,INTER_STEP_MIN_DELAY),INTER_STEP_MAX_DELAY);
            out_smart = 0;
		  }
          break;
        case LM_READ_DELAY:
          if(!out_smart)
		  {
            out_smart = 1;
            GetDlgItemText(hDlg,LM_READ_DELAY,string,9);        
            lm_read_delay = atoi(string);
            out_smart = 0;
		  }
          break;

		// Measurement parameters
        case LM_MEASURE_EVERY:
          if(!out_smart)
		  {
            out_smart = 1;
            GetDlgItemText(hDlg,LM_MEASURE_EVERY,string,9);     
            lm_measure_every = atoi(string);
            if(lm_measure_every == 0) lm_measure_every = 1; // prevent divide by 0
            out_smart = 0;
		  }
          break;
        case LM_DO_READ_Z:
          lm_read_z = IsDlgButtonChecked(hDlg,LM_DO_READ_Z);
          break;
        case LM_READ_N_TIMES:
          if(!out_smart)
		  {
            out_smart = 1;
            GetDlgItemText(hDlg,LM_READ_N_TIMES,string,9);      
            lm_read_n_times = atoi(string);
            out_smart = 0;
		  }
          break;

		// Line color
        case LM_COLOR_R:
          if(!out_smart)
		  {
            out_smart = 1;
            GetDlgItemText(hDlg,LM_COLOR_R,string,5);
            value = atoi(string);
            lm_line_color.r = min(max(value,0),255);
            out_smart = 0;
		  }
          break;
        case LM_COLOR_G:
          if(!out_smart)
		  {
            out_smart = 1;
            GetDlgItemText(hDlg,LM_COLOR_G,string,5);
            value = atoi(string);
            lm_line_color.g = min(max(value,0),255);
            out_smart = 0;
		  }
          break;
        case LM_COLOR_B:
          if(!out_smart)
		  {
            out_smart = 1;
            GetDlgItemText(hDlg,LM_COLOR_B,string,5);
            value = atoi(string);
            lm_line_color.b = min(max(value,0),255);
            out_smart = 0;
		  }
          break;

		// voltages and currents
        case LM_IMAGE_BIAS_BITS:
          if(!out_smart)
		  {
            out_smart = 1;
            GetDlgItemText(hDlg,LM_IMAGE_BIAS_BITS,string,9);
            value = atoi(string);
            lm_image_bias = min(max(value,sample_bias_min),sample_bias_max);
            out_smart = 0;
		  }
          break;
        case LM_IMAGE_BIAS_V:
          if(!out_smart)
		  {
            out_smart = 1;
            GetDlgItemText(hDlg,LM_IMAGE_BIAS_V,string,9);
            value = vtod_bias(atof(string),sample_bias_range);
			lm_image_bias = min(max(value,sample_bias_min),sample_bias_max);
            out_smart = 0;
          }
          break;
        case LM_MANIP_BIAS_BITS:
          if(!out_smart)
		  {
            out_smart = 1;
            GetDlgItemText(hDlg,LM_MANIP_BIAS_BITS,string,5);
            value = atoi(string);
            lm_manip_bias = min(max(value,sample_bias_min),sample_bias_max);
            out_smart = 0;
		  }
          break;
        case LM_MANIP_BIAS_V:
          if(!out_smart)
		  {
            out_smart = 1;
            GetDlgItemText(hDlg,LM_MANIP_BIAS_V,string,5);
            value = vtod_bias(atof(string),sample_bias_range);

            //value = vtod(atof(string),sample_bias_range);
            lm_manip_bias = min(max(value,sample_bias_min),sample_bias_max);
            out_smart = 0;
		  }
          break;
        case LM_IMAGE_I_BITS:
          if(!out_smart)
		  {
            out_smart =1;
            GetDlgItemText(hDlg,LM_IMAGE_I_BITS,string,5);
            value = atoi(string);
            value = min(max(value,tip_current_min),tip_current_max);
            lm_image_i = (unsigned int)value;
            lm_image_i_set = calc_i_set(lm_image_i,i_set_range,tip_gain);
            out_smart = 0;
		  }
          break;
        case LM_IMAGE_I_NA:
          if(!out_smart)
          {
            out_smart = 1;
            GetDlgItemText(hDlg,LM_IMAGE_I_NA,string,5);
            lm_image_i_set = (double)atof(string);
            lm_image_i = calc_i_setpoint(lm_image_i_set,i_set_range,tip_gain);
            lm_image_i_set = calc_i_set(lm_image_i,i_set_range,tip_gain);
            out_smart = 0;
          }
          break;
        case LM_MANIP_I_BITS:
          if(!out_smart)
		  {
            out_smart = 1;
            GetDlgItemText(hDlg,LM_MANIP_I_BITS,string,5);
            value = atoi(string);
            value = min(max(value,tip_current_min),tip_current_max);
            lm_manip_i = (unsigned int)value;
            lm_manip_i_set = calc_i_set(lm_manip_i,i_set_range,tip_gain);
            out_smart = 0;
		  }
          break;
        case LM_MANIP_I_NA:
          if(!out_smart)
          {
            out_smart = 1;
            GetDlgItemText(hDlg,LM_MANIP_I_NA,string,6);
             lm_manip_i_set = (double)atof(string);
             lm_manip_i = calc_i_setpoint(lm_manip_i_set,i_set_range,tip_gain);
             lm_manip_i_set = calc_i_set(lm_manip_i,i_set_range,tip_gain);
             out_smart = 0;
		  }
          break;
        case LM_IMPORT:
          //import imaging settings from current bias, current
          lm_image_bias = sample_bias;
          lm_image_i = i_setpoint;
          lm_image_i_set = calc_i_set(lm_image_i,i_set_range,tip_gain);
          repaint_lm_bias_params(hDlg);
          break;

		// graph parameters
        case LM_V_SCALE_MIN:
          if(!out_smart)
		  {
            out_smart = 1;
            GetDlgItemText(hDlg,LM_V_SCALE_MIN,string,9);
            fvalue = (float)(atof(string));
            lm_vert_min = min(max(fvalue,IN_MIN),IN_MAX);
            if(lm_vert_min > lm_vert_max) lm_vert_min = lm_vert_max - (float)1.0;
            out_smart = 0;
		  }
          break;
        case LM_V_SCALE_MAX:
          if(!out_smart)
		  {
            out_smart = 1;
            GetDlgItemText(hDlg,LM_V_SCALE_MAX,string,9);
            fvalue = (float)(atof(string));
            lm_vert_max = min(max(fvalue,IN_MIN),IN_MAX);
            if(lm_vert_max < lm_vert_min) lm_vert_max = lm_vert_min + (float)1.0;
            out_smart = 0;
		  }
          break;
        case LM_ZOOM_TO_MAX:
          if(manip_data->valid)
		  {
            find_min_max(manip_data,&manip_min_z,&manip_max_z);
            lm_vert_max = manip_max_z;
            repaint_lm_path_data(hDlg);  
		  }
          break;
        case LM_ZOOM_TO_MIN:
          if(manip_data->valid)
		  {
            find_min_max(manip_data,&manip_min_z,&manip_max_z);
            lm_vert_min = manip_min_z;
            repaint_lm_path_data(hDlg);  
		  }
          break;
        case LM_CENTER:
          if(manip_data->valid)
		  {
            find_min_max(manip_data,&manip_min_z,&manip_max_z);
            lm_vert_max = (float)IN_ZERO + max(
				(float)(fabs((double)manip_min_z - (double)IN_ZERO)),
				(float)(fabs((double)manip_max_z - (double)IN_ZERO)));
            lm_vert_min = IN_ZERO - (lm_vert_max - IN_ZERO);
            repaint_lm_path_data(hDlg);  
		  }
          break;

        case LM_CONST_I:
		case LM_CONST_Z:
          lm_mode = LOWORD(wParam);
          repaint_lm_mode(hDlg);
		  break;
        case LM_FBACK_OFF:
          lm_fback_off = IsDlgButtonChecked(hDlg,LM_FBACK_OFF);
          break;
        case LM_CALC_SLOPE:
          lm_calc_slope = IsDlgButtonChecked(hDlg,LM_CALC_SLOPE);
          break;
		case LM_Z_SLOPE:
          if(!out_smart)
		  {
            out_smart = 1;
            GetDlgItemText(hDlg,LM_Z_SLOPE,string,9);
            fvalue = (float)(atof(string));
            lm_z_slope = min(max(fvalue,MIN),MAX);
            out_smart = 0;
		  }
		  break;

        case LM_START:
          // disable controls?
          lateral_manipulation(hDlg);
		  repaint_lm_mode(hDlg);
          // enable controls?
		  if(manip_data->valid)
		  {
            find_min_max(manip_data,&manip_min_z,&manip_max_z);
            lm_vert_max = manip_max_z;
            lm_vert_min = manip_min_z;
            repaint_lm_path_data(hDlg);
		  }
          break;
        case LM_STOP:
          sprintf(string,"%f %u %u",lm_bits_per_pix,data->size,data->step);
          mprintf(string);
          manipulating = 0;
          break;

        // file handling
	    case LM_SAVE:
          if(data->valid && manip_data->valid)
          {
            glob_data = &manip_data;
            if(file_save_as(hDlg,hInst,FTYPE_CUT))
            {
              inc_file(current_file_cut,&file_cut_count);
              manip_data->saved = 1;
            }    
          }
          else MessageBox(hDlg,"Invalid Data!","Alert!",MB_OKCANCEL);
          break;
        case LM_LOAD:

          if(!manip_data->saved && manip_data->valid)
		  {
            if(MessageBox(hDlg,"Manip data not saved!","Warning",
			   MB_OKCANCEL | MB_ICONEXCLAMATION) == IDCANCEL) 
			{
              break;
			}
		  }
          glob_data = &manip_data;
          file_open(hDlg,hInst,FTYPE_CUT,0,current_file_cut);
          get_manip_data();
          repaint_lm_path_data(hDlg);
          repaint_lm_bias_params(hDlg);
          repaint_lm_path_params(hDlg);
          repaint_lm_arrow(hDlg);
          repaint_lm_timing_params(hDlg);
          break;
        case LM_INFO:  // make some test data for debugging
                       // info for cuts not implemented yet
          if(data->valid)
		  {
            for(j = 0;j < 100;j++)
			{
              *(manip_data->yf + j) = abs(50 - j) * (IN_MAX - IN_MIN) / (float)(200.0);
			}
            manip_data->cut_x1 = (float)(BITS_TO_STEPS(lm_initial_x,data));
            manip_data->cut_x2 = (float)(BITS_TO_STEPS(lm_initial_x+100,data));
            manip_data->cut_y1 = manip_data->cut_y2 = (float)(BITS_TO_STEPS(lm_final_y,data));
            manip_data->min_x = 0.0;
            manip_data->max_x = (float)(sqrt( (manip_data->cut_x2 - manip_data->cut_x1) *
                                  (manip_data->cut_x2 - manip_data->cut_x1) + 
                                  (manip_data->cut_y2 - manip_data->cut_y1) *
                                  (manip_data->cut_y2 - manip_data->cut_y1) ));
            manip_data->size = manip_size = 100;
            manip_data->valid = 1;
            manip_data->saved = 0;
            repaint_lm_path_data(hDlg);
		  }
          break;
        case LM_COMMENT:
          gcomment = &(manip_data->comment);
          lpfnDlgProc = MakeProcInstance(CommentDlg,hInst);
          DialogBox(hInst,"COMMENTDLG",hDlg,lpfnDlgProc);
          FreeProcInstance(lpfnDlgProc);                  
          break;
 
        case ENTER:
          SetFocus(GetDlgItem(hDlg,LM_EXIT));
          repaint_lm_path_params(hDlg);
          repaint_lm_bias_params(hDlg);
          repaint_lm_timing_params(hDlg);
          repaint_lm_arrow(hDlg);
          repaint_lm_path_data(hDlg);
	      repaint_lm_mode(hDlg);
          break;

        case LM_EXIT:
          scan_fine_x = dac_data[x_ch];
          scan_fine_y = dac_data[y_ch];
          SendMessage(ScnDlg,WM_COMMAND,SCAN_REPAINT_FINE_POS,0);
          EndDialog(hDlg, TRUE);

    }
    break;
  }
  return(FALSE);
}

static void set_manip_data()
{
  strcpy(manip_data->filename,"Unsaved");
  strcpy(manip_data->full_filename,"Unsaved");
  manip_data->saved = 0;
  manip_data->version = CUT_DATA_VERSION;
  manip_data->type2d = TYPE2D_CUT;
  strcpy(string, LATMAN_COMMENT_STRING);
  strcpy(manip_data->comment.ptr,string);
  manip_data->comment.size = strlen(string);

  //meaning unique to lat man data
  manip_data->i_setpoint = lm_manip_i;  
  manip_data->step_delay = lm_read_delay;  
  manip_data->inter_step_delay = lm_interstep_delay;
  manip_data->bias = lm_manip_bias;  

  manip_data->amp_gain = tip_gain;  
  manip_data->bias_range = sample_bias_range;  
  manip_data->i_set_range = i_set_range;  
  manip_data->x_offset_range = x_offset_range;
  manip_data->y_offset_range = y_offset_range;

  manip_data->read_ch = zi_ch;  

  manip_data->step = lm_measure_every;
 
  if(data->valid)
  {
    //meaningful
    memcpy(manip_data->dosed_type.ptr,data->dosed_type.ptr,SAMPLETYPESIZE);
    memcpy(manip_data->sample_type.ptr,data->sample_type.ptr,SAMPLETYPESIZE);
    manip_data->sample_type.size = data->sample_type.size;
    manip_data->dosed_type.size = data->dosed_type.size;
    manip_data->dosed_langmuir = data->dosed_langmuir;  
    manip_data->temperature = data->temperature;
    manip_data->x_gain = data->x_gain; // global vars exist for gains, but
    manip_data->y_gain = data->y_gain; // for this dialog to make sense,
    manip_data->z_gain = data->z_gain; // they must equal data->?_gain's
    manip_data->z_freq = data->z_freq; // user is warned at WM_INITDIALOG!
    manip_data->x_range = data->x_range;
    manip_data->y_range = data->y_range;
    manip_data->x = data->x;
    manip_data->y = data->y;
 
    //just for compatibility with save_cut_fp
    manip_data->digital_feedback = data->digital_feedback;  
    manip_data->read_feedback = data->read_feedback;
    manip_data->scan_dither0 = data->scan_dither0;
    manip_data->scan_dither1 = data->scan_dither1;
    manip_data->dither_wait = data->dither_wait;
    manip_data->dither0 = data->dither0;
    manip_data->dither1 = data->dither1;
    manip_data->tip_spacing = data->tip_spacing;  
    manip_data->inter_line_delay = data->inter_line_delay;  
    manip_data->scan_num = data->scan_num;  
    manip_data->scan_feedback = data->scan_feedback;  
    manip_data->overshoot = data->overshoot;  
    manip_data->overshoot_percent = data->overshoot_percent;  
    manip_data->overshoot_wait1 = data->overshoot_wait1;  
    manip_data->overshoot_wait2 = data->overshoot_wait2;  
    manip_data->scan_dir = data->scan_dir;  
    manip_data->z = data->z; // meaningless
    manip_data->crash_protection = data->crash_protection;  
  }

}

static void get_manip_data()
{
  lm_manip_i = manip_data->i_setpoint;  
  lm_read_delay = manip_data->step_delay;  
  lm_interstep_delay = manip_data->inter_step_delay;
  lm_manip_bias = manip_data->bias;  
  if(manip_data->read_ch == zi_ch)
  {
    lm_read_z = 1;
  }
  else
  {
    lm_read_z = 0;
  }

  lm_measure_every = manip_data->step;

  if(data->valid)
  {
    lm_initial_x = (int)(STEPS_TO_BITS(manip_data->cut_x1,data));
    lm_final_x = (int)(STEPS_TO_BITS(manip_data->cut_x2,data));
    lm_initial_y = (int)(STEPS_TO_BITS(manip_data->cut_y1,data));
    lm_final_y = (int)(STEPS_TO_BITS(manip_data->cut_y2,data));
  }
  else
  { 
    lm_initial_x = (int)(manip_data->cut_x1);
    lm_final_x = (int)(manip_data->cut_x2);
    lm_initial_y = (int)(manip_data->cut_y1);
    lm_final_y = (int)(manip_data->cut_y2);
  }
  manip_size = manip_data->size;
}

static void repaint_lm_bias_params(HWND hDlg)
{
  out_smart = 1;

  sprintf(string,"%d",lm_image_bias);
  SetDlgItemText(hDlg,LM_IMAGE_BIAS_BITS,string);
  sprintf(string,"%.4f",dtov_bias(lm_image_bias,sample_bias_range));
  SetDlgItemText(hDlg,LM_IMAGE_BIAS_V,string);
  SetScrollPos(GetDlgItem(hDlg,LM_IMAGE_BIAS_SCROLL),SB_CTL,(int)lm_image_bias,TRUE);
  
  sprintf(string,"%d",lm_manip_bias);
  SetDlgItemText(hDlg,LM_MANIP_BIAS_BITS,string);
  sprintf(string,"%.4f",dtov_bias(lm_manip_bias,sample_bias_range));
  SetDlgItemText(hDlg,LM_MANIP_BIAS_V,string);
  SetScrollPos(GetDlgItem(hDlg,LM_MANIP_BIAS_SCROLL),SB_CTL,(int)lm_manip_bias,TRUE);

  sprintf(string,"%d",lm_image_i);
  SetDlgItemText(hDlg,LM_IMAGE_I_BITS,string);
  SetScrollPos(GetDlgItem(hDlg,LM_IMAGE_I_SCROLL),SB_CTL,(int)tip_current_max - lm_image_i,TRUE);
  gcvt(lm_image_i_set,5,string);
  SetDlgItemText(hDlg,LM_IMAGE_I_NA,string);

  sprintf(string,"%d",lm_manip_i);
  SetDlgItemText(hDlg,LM_MANIP_I_BITS,string);
  SetScrollPos(GetDlgItem(hDlg,LM_MANIP_I_SCROLL),SB_CTL,(int)tip_current_max - lm_manip_i,TRUE);
  gcvt(lm_manip_i_set,5,string);
  SetDlgItemText(hDlg,LM_MANIP_I_NA,string);
   
  out_smart = 0;
}

static void repaint_lm_timing_params(HWND hDlg)
{
  sprintf(string,"%d",lm_ramp_delay);
  SetDlgItemText(hDlg,LM_RAMP_DELAY,string);
  sprintf(string,"%d",lm_interstep_delay);
  SetDlgItemText(hDlg,LM_INTERSTEP_DELAY,string);
  sprintf(string,"%d",lm_read_delay);
  SetDlgItemText(hDlg,LM_READ_DELAY,string);
  sprintf(string,"%d",lm_measure_every);
  SetDlgItemText(hDlg,LM_MEASURE_EVERY,string);
  sprintf(string,"%d",lm_read_n_times);
  SetDlgItemText(hDlg,LM_READ_N_TIMES,string);
  CheckDlgButton(hDlg,LM_DO_READ_Z,lm_read_z);
}

static void repaint_lm_arrow(HWND hDlg)
{
  HDC arrow_DC;
  int OldROP;
  HPEN hPen,hPenOld;
  float theta,omega,delta;
  int base_x,base_y;
  POINT head[3];
  LOGBRUSH lplb;
  HBRUSH hBrush,hBrushOld;

  if(lm_initial_x == lm_final_x && lm_initial_y == lm_final_y) return;
  
  arrow_DC = GetDC(hDlg);
  SetMapMode(arrow_DC,MM_TEXT);

  // repaint image, effectively erasing old arrow
  if(data->valid)
  {
    SelectPalette(arrow_DC,hPal,0);
    RealizePalette(arrow_DC);
    StretchDIBits(arrow_DC,LM_DISPLAY_LEFT,LM_DISPLAY_TOP, 
      LM_DISPLAY_WIDTH,LM_DISPLAY_HEIGHT,0,0,data->size,data->size, 
      bitmap_data,bitmapinfo,DIB_PAL_COLORS,SRCCOPY);   
  }

  theta = (float)(atan2(lm_final_y - lm_initial_y,lm_final_x - lm_initial_x));

  // paint line
  OldROP = SetROP2(arrow_DC,R2_NOTXORPEN);
  hPen = CreatePen(PS_SOLID,/*thickness*/2,RGB(lm_line_color.r,
                lm_line_color.g,lm_line_color.b));
  hPenOld = SelectObject(arrow_DC,hPen);

  MoveToEx(arrow_DC,BITS_TO_PIX_X(lm_initial_x),BITS_TO_PIX_Y(lm_initial_y),NULL);
  base_x = (int)(lm_final_x - (LM_ARROW_HEIGHT * lm_bits_per_pix) * 
	             cos(RADIAN(LM_ARROW_HEAD_ANGLE) / 2) * cos(theta));
  base_y = (int)(lm_final_y - (LM_ARROW_HEIGHT * lm_bits_per_pix) *
	             cos(RADIAN(LM_ARROW_HEAD_ANGLE) / 2) * sin(theta));
  LineTo(arrow_DC,BITS_TO_PIX_X(base_x),BITS_TO_PIX_Y(base_y));

  // paint arrow head
  lplb.lbStyle = BS_SOLID;
  lplb.lbColor = RGB(lm_line_color.r,lm_line_color.g,lm_line_color.b);
  hBrush = CreateBrushIndirect(&lplb);
  hBrushOld = SelectObject(arrow_DC,hBrush);
 
  head[0].x = BITS_TO_PIX_X(lm_final_x);
  head[0].y = BITS_TO_PIX_Y(lm_final_y);
  omega = (float)(theta - RADIAN(LM_ARROW_HEAD_ANGLE) / 2);
  head[1].x = (long)(BITS_TO_PIX_X(lm_final_x - (LM_ARROW_HEIGHT * lm_bits_per_pix) * cos(omega)));
  head[1].y = (long)(BITS_TO_PIX_Y(lm_final_y - (LM_ARROW_HEIGHT * lm_bits_per_pix) * sin(omega)));
  
  delta = (float)(PI / 2 - RADIAN(LM_ARROW_HEAD_ANGLE) / 2 - theta);
  head[2].x = (long)(BITS_TO_PIX_X(lm_final_x - (LM_ARROW_HEIGHT * lm_bits_per_pix) * sin(delta)));
  head[2].y = (long)(BITS_TO_PIX_Y(lm_final_y - (LM_ARROW_HEIGHT * lm_bits_per_pix) * cos(delta)));
  Polygon(arrow_DC,head,3);

  SelectObject(arrow_DC,hBrushOld);
  DeleteObject(hBrush);
  SelectObject(arrow_DC,hPenOld);
  DeleteObject(hPen);
  SetROP2(arrow_DC,OldROP);
  ReleaseDC(hDlg,arrow_DC);
}
 
static void repaint_lm_path_params(HWND hDlg)
{
  sprintf(string,"%d",lm_initial_x);
  SetDlgItemText(hDlg,LM_INITIAL_X,string);
  sprintf(string,"%d",lm_initial_y);
  SetDlgItemText(hDlg,LM_INITIAL_Y,string);

  sprintf(string,"%d",lm_final_x);
  SetDlgItemText(hDlg,LM_FINAL_X,string);
  sprintf(string,"%d",lm_final_y);
  SetDlgItemText(hDlg,LM_FINAL_Y,string);

  sprintf(string,"%d",lm_line_color.r);
  SetDlgItemText(hDlg,LM_COLOR_R,string);
  sprintf(string,"%d",lm_line_color.g);
  SetDlgItemText(hDlg,LM_COLOR_G,string);
  sprintf(string,"%d",lm_line_color.b);
  SetDlgItemText(hDlg,LM_COLOR_B,string);
}

static void repaint_lm_path_data(HWND hDlg)
{
  // repaint data graph
  HDC   lm_data_hdc;
  HPEN  hpen_old;
  int   x1,y1,x2,y2; // boundary coordinates of painting area, in pixels
  float y_min = (float)IN_MIN; // range of vertical axis, in bits
  float y_max = (float)IN_MAX;
  int xs,ys; // coordinates of current data point, in pixels
  float yfs;
  int j;

  // update relevant editable text fields
  sprintf(string,"%.4f",lm_vert_min);
  SetDlgItemText(hDlg,LM_V_SCALE_MIN,string);
  sprintf(string,"%.4f",lm_vert_max);
  SetDlgItemText(hDlg,LM_V_SCALE_MAX,string);
  SetScrollPos(GetDlgItem(hDlg,LM_V_SCALE_MIN_SCROLL), 
               SB_CTL,DTOI(lm_vert_min,IN_MIN,IN_MAX),TRUE);
  SetScrollPos(GetDlgItem(hDlg,LM_V_SCALE_MAX_SCROLL), 
               SB_CTL,DTOI(lm_vert_max,IN_MIN,IN_MAX),TRUE);
 
  x1 = SMALL_BORDER_SIZE;
  y1 = SMALL_BORDER_SIZE + SMALL_IMAGE_SIZE + SMALL_BORDER_SIZE;
  x2 = x1 + SMALL_IMAGE_SIZE;
  y2 = y1 + SMALL_IMAGE_SIZE;
 
  // draw background
  lm_data_hdc = GetDC(hDlg);
  hpen_old = SelectObject(lm_data_hdc,CreatePen(PS_SOLID,1,RGB(0,0,0)));
  Rectangle(lm_data_hdc,x1,y1,x2,y2);
  repaint_grid(hDlg,x1,x2,y1,y2);

  // now, draw graph
  if(manip_size != 0)
  {
    if(lm_mode == LM_CONST_I)
	  SelectObject(lm_data_hdc,hpen_blue);
    else
	  SelectObject(lm_data_hdc,hpen_red);
    y_min = lm_vert_min;
    y_max = lm_vert_max;

    // get to starting position w/o making line
    xs = x1;
    yfs = y1 - (*(manip_data->yf) - y_max) * (y2 - y1 + 1) / (y_max - y_min + 1);
    ys = (int)yfs;
    xs = max(min(xs,x2),x1);
    ys = max(min(ys,y2 - 1),y1 + 1);
    MoveToEx(lm_data_hdc,xs,ys,NULL);

    // draw line to each subsequent position
    for(j = 1;j<manip_size;j++)
	{
      xs = x1 + j * (x2 - x1 + 1) / manip_size;
      yfs = y1 - (*(manip_data->yf + j) - y_max) * (y2 - y1 + 1) / (y_max - y_min + 1);
      ys = (int)yfs;
      xs = max(min(xs,x2),x1);
      ys = max(min(ys,y2),y1);
      LineTo(lm_data_hdc,xs,ys);
	}
  }

  SelectObject(lm_data_hdc,hpen_old);
  ReleaseDC(hDlg,lm_data_hdc);
}

static void repaint_lm_status(HWND hDlg,char *status)
{
  SetDlgItemText(hDlg,LM_STATUS,status);
}

static void repaint_lm_mode(HWND hDlg)
{
  // disable const z controls if we're in const I mode
  if(lm_mode == LM_CONST_I)
  {
    EnableWindow(GetDlgItem(hDlg,LM_Z_SLOPE),FALSE);
    EnableWindow(GetDlgItem(hDlg,LM_FBACK_OFF),FALSE);
    EnableWindow(GetDlgItem(hDlg,LM_CALC_SLOPE),FALSE);
  }
  else
  {
    EnableWindow(GetDlgItem(hDlg,LM_Z_SLOPE),TRUE);
    EnableWindow(GetDlgItem(hDlg,LM_FBACK_OFF),TRUE);
    EnableWindow(GetDlgItem(hDlg,LM_CALC_SLOPE),TRUE);
  }

  sprintf(string,"%.4f",lm_z_slope);
  SetDlgItemText(hDlg,LM_Z_SLOPE,string);
  if(lm_mode == LM_CONST_I)
  {
    CheckDlgButton(hDlg,LM_CONST_I,1);
    CheckDlgButton(hDlg,LM_CONST_Z,0);
  }
  if(lm_mode == LM_CONST_Z)
  {
    CheckDlgButton(hDlg,LM_CONST_I,0);
    CheckDlgButton(hDlg,LM_CONST_Z,1);
  }
  CheckDlgButton(hDlg,LM_FBACK_OFF,lm_fback_off);
  CheckDlgButton(hDlg,LM_CALC_SLOPE,lm_calc_slope);


}

static void calc_const_height()
{
  int delta_long;
  float delta_zoff;
  float initial_z,final_z;
  float lm_init_x_steps,lm_final_x_steps,lm_init_y_steps,lm_final_y_steps;
  double mult = 1.0;

  if(lm_calc_slope)
  {
    if(abs(lm_final_x - lm_initial_x) >= abs(lm_final_y - lm_initial_y))
	{
      delta_long = abs(lm_final_x - lm_initial_x);
	}
    else
	{
      delta_long = abs(lm_final_y - lm_initial_y);
	}
	  
    // calc initial_z, final_z from image data + other vars
    if(data->valid)
	{
      lm_init_x_steps = ((float)data->size / 2) + ((float)lm_initial_x - 2048) 
					/ (float)data->step;
      lm_final_x_steps = ((float)data->size / 2) + ((float)lm_final_x - 2048)
					/ (float)data->step;
      lm_init_y_steps = ((float)data->size / 2) + ((float)lm_initial_y - 2048)
					/ (float)data->step;
      lm_final_y_steps = ((float)data->size / 2) + ((float)lm_final_y - 2048)
					/ (float)data->step;

	  initial_z = interp_z(data,lm_init_x_steps,lm_init_y_steps);
	  final_z = interp_z(data,lm_final_x_steps,lm_final_y_steps);
	  if(initial_z > final_z)
	  {
	    lm_z_dir = 1;
	  }
	  else
	  {
	    lm_z_dir = 0;
	  }

	  // mult based on feedback z gain comp. to z offset gain
      // recall that z offset is always on gain 10
	  switch(dac_data[gain_ch] & z2_gain_bits)
	  {
        case Z2_TENTH_GAIN:
          mult = 0.01;
          break;            
        case Z2_ONE_GAIN:
          mult = 0.1;
          break;            
        case Z2_TEN_GAIN:
          mult = 1.0;
         break;
	  } 

      if((dac_data[range_ch] & 0x0002) == 0x0002) // true if z off range is +- 5V
	  {
        mult *= 2;
	  }

      // divide by 16 for 16-to-12 bit conversion
	  delta_zoff = (float)(fabs(initial_z - final_z) * mult / 16);
	}
    else
	{
      delta_zoff = 0.0;
	}

    if(delta_zoff != 0.0)
	{
      lm_z_slope = (float)delta_long / delta_zoff;
	}
    else
	{  
      lm_z_slope = (float)delta_long + (float)1.0; // safety
	} 
    if(lm_z_slope > 4096.0) lm_z_slope = 4096.0;
  } 
}

void lateral_manipulation(HWND hDlg)
{
  // path variables. think of path as hypotenuse of right triangle.
  // "long" ("short") variables refer to the longer (shorter) of the two sides that are
  // adjacent to the right angle 
  int delta_long,delta_short;
  int long_ch,short_ch;
  float ratio;
  int initial_long,final_long;        // path - long side
  int initial_short,final_short;      // path - short side
  int current_long;
  int current_short;
  int long_dir; // = -1 for decreasing, +1 for increasing
  int short_dir; // same
  int do_const_height = (lm_mode == LM_CONST_Z); // bool
  int z_inc = 1; // bool
//  unsigned int bump;
//  double lm_data;
//  MSG manip_Message;

  // saved or unsaved?
  if(!manip_data->saved && manip_data->valid && lm_read_z)
  {
    if(MessageBox(hDlg,"Manip data not saved!","Warning",
	   MB_OKCANCEL | MB_ICONEXCLAMATION) == IDCANCEL)
	{
      return;
	}
  }

  if(lm_read_z)
  {
    free_data(&manip_data);
    alloc_data(&manip_data,DATATYPE_GEN2D,MAX,GEN2D_NONE,GEN2D_FLOAT,1);
    set_manip_data();
  }

  // future... check if feedback is on

  // calculate path variables
  if(abs(lm_final_x - lm_initial_x) >= abs(lm_final_y - lm_initial_y))
  {
    long_ch = x_ch;
    short_ch = y_ch;
    initial_long = lm_initial_x;
    final_long = lm_final_x;
    initial_short = lm_initial_y;
    final_short = lm_final_y;
  }
  else
  {
    long_ch = y_ch;
    short_ch = x_ch;
    initial_long = lm_initial_y;
    final_long = lm_final_y;
    initial_short = lm_initial_x;
    final_short = lm_final_x;
  }

  if(final_long > initial_long) long_dir = 1;
  else long_dir = 0;
  if(final_short > initial_short) short_dir = 1;
  else short_dir = 0;
        
  current_long = (int)(dac_data[long_ch]);
  current_short = (int)(dac_data[short_ch]);

  delta_long = abs(final_long - initial_long);
  delta_short = abs(final_short - initial_short);
  if(delta_short != 0)
  {
    ratio = (float)delta_long / (float)delta_short;
  }
  else
  { 
    ratio = (float)delta_long + (float)1.0; // safety
  }
  if(ratio > 4096.0) ratio = 4096.0;

  // calculate slope z vs long ch, direction of z increment
  if(lm_mode == LM_CONST_Z)
  {
    calc_const_height();
  }
  else
  { // dummy's for const current
    lm_z_slope = (float)delta_long + (float)1.0;
	lm_z_dir = 1;
  }

  if(lm_read_z)
  {
    manip_data->cut_x1 = (float)(BITS_TO_STEPS(lm_initial_x,data));
    manip_data->cut_x2 = (float)(BITS_TO_STEPS(lm_final_x,data));
    manip_data->cut_y1 = (float)(BITS_TO_STEPS(lm_initial_y,data));
    manip_data->cut_y2 = (float)(BITS_TO_STEPS(lm_final_y,data));
    manip_data->min_x = 0.0;
    manip_data->max_x = (float)(sqrt( (manip_data->cut_x2 - manip_data->cut_x1) *
                            (manip_data->cut_x2 - manip_data->cut_x1) + 
                            (manip_data->cut_y2 - manip_data->cut_y1) *
                            (manip_data->cut_y2 - manip_data->cut_y1) ));
  }

  // ramp to image current, bias (order may be important)
  // ramp through zero??
  repaint_lm_status(hDlg,"Ramp to Image conditions");
  dac_data[i_setpoint_ch] = ramp_serial(i_setpoint_ch,dac_data[i_setpoint_ch],
	                                    lm_image_i,lm_ramp_delay,0);
  if(bit16)
  {
    sample_bias = ramp_serial(highres_bias_ch,sample_bias,
	                                    lm_image_bias,lm_ramp_delay,0);
  }
  else
  {
    sample_bias = ramp_serial(sample_bias_ch,sample_bias,
	                                    lm_image_bias,lm_ramp_delay,0);
  }
  
  repaint_lm_status(hDlg,"Move to start location");
  // move to the initial point
  dac_data[long_ch]  = move_to_protect_serial(long_ch,current_long,initial_long,
                                   lm_interstep_delay,CRASH_STOP,LM_LIMIT_PERCENT);
  dac_data[short_ch] = move_to_protect_serial(short_ch,current_short,initial_short,
                                   lm_interstep_delay,CRASH_STOP,LM_LIMIT_PERCENT);
  if(dac_data[long_ch] != (unsigned int)initial_long || dac_data[short_ch] != (unsigned int)initial_short)
  {
    MessageBox(hDlg,"Aborting","Unable to move to start location",MB_ICONHAND);
    return;
  }
  else
  {
    current_long = initial_long;
    current_short = initial_short;
  }
        
  manipulating = 1;
  manip_size = 0;

  // ramp to manip bias, current (order may be important)
  repaint_lm_status(hDlg,"Ramp to Manip conditions");
  if(bit16)
  {
    sample_bias = ramp_serial(highres_bias_ch,sample_bias,
	                                    lm_manip_bias,lm_ramp_delay,0);
  }
  else
  {
    sample_bias = ramp_serial(sample_bias_ch,sample_bias,
	                                    lm_manip_bias,lm_ramp_delay,0);
  }
  dac_data[i_setpoint_ch] = ramp_serial(i_setpoint_ch,dac_data[i_setpoint_ch],
	                                    lm_manip_i,lm_ramp_delay,0);
  

  // feedback off in const height mode
  if(lm_mode == LM_CONST_Z)
    dio_feedback_serial(0);

  // trace the path
  repaint_lm_status(hDlg,"Manipulating...");
  manip_data->sequence[0].type = READ_TYPE;
  if(lm_mode == LM_CONST_Z)
  {
    manip_data->sequence[0].in_ch = 0;
  }
  else
  {
    manip_data->sequence[0].in_ch = 1;
  }
  manip_data->sequence[0].num_reads = lm_read_n_times;
  strcpy(manip_data->sequence[0].step_type_string,"Read");
  manip_data->total_steps = 1;
  readseq_setup(manip_data);

  latman_setup(initial_long,final_long,initial_short,scan_z,
	           lm_measure_every,ratio,lm_z_slope,
			   lm_interstep_delay,lm_read_delay);
  latman(long_ch,short_ch,lm_read_z,long_dir,short_dir,
	     &current_long,&current_short,&manip_size,manip_data->yf,
		 do_const_height,lm_z_dir);

  if(manip_size > 0)
  {
    manip_data->size = manip_size;
    manip_data->valid = 1;
  }
 
  // update dac_data
  dac_data[long_ch] = (unsigned int)current_long;
  dac_data[short_ch] = (unsigned int)current_short;
        
  // feedback back on in const height mode
  if(lm_mode == LM_CONST_Z && !lm_fback_off)
    dio_feedback_serial(1);

  // ramp to image current, bias (order may be important)
  // really ramp to "entry" values (same as before latman dialog opened)
  repaint_lm_status(hDlg,"Ramp to Image conditions");
  dac_data[i_setpoint_ch] = ramp_serial(i_setpoint_ch,dac_data[i_setpoint_ch],
	                                    lm_entry_i,lm_ramp_delay,0);
  if(bit16)
  {
    sample_bias = ramp_serial(highres_bias_ch,sample_bias,
	                                    lm_entry_bias,lm_ramp_delay,0);
  }
  else
  {
    sample_bias = ramp_serial(sample_bias_ch,sample_bias,
	                                    lm_entry_bias,lm_ramp_delay,0);
  }
  repaint_lm_status(hDlg,"Idle.");
}

// ******************************************************************************************
//		O L D * O L D * O L D * O L D * O L D * O L D * O L D * O L D * O L D * O L D
// ******************************************************************************************


#ifdef OLD
void lateral_manipulation(HWND hDlg)
{
  // path variables. think of path as hypotenuse of right triangle.
  // "long" ("short") variables refer to the longer (shorter) of the two sides that are
  // adjacent to the right angle 
  int delta_long,delta_short;
  int long_ch,short_ch;
  float ratio;
  int initial_long,final_long;        // path - long side
  int initial_short,final_short;      // path - short side
  int current_long;
  int current_short;
  int long_dir; // = -1 for decreasing, +1 for increasing
  int short_dir; // same
  unsigned int bump;
  double lm_data;

  MSG manip_Message;

  // saved or unsaved?
  if(!manip_data->saved && manip_data->valid)
  {
    if(MessageBox(hDlg,"Manip data not saved!","Warning",
		MB_OKCANCEL | MB_ICONEXCLAMATION) == IDCANCEL)
	{
      return;
	}
  }

  free_data(&manip_data);
  alloc_data(&manip_data,DATATYPE_GEN2D,MAX,GEN2D_NONE,GEN2D_FLOAT,1);
  set_manip_data();

  // future... check if feedback is on

  // calculate path variables
  if(abs(lm_final_x - lm_initial_x) >= abs(lm_final_y - lm_initial_y))
  {
    long_ch = x_ch;
    short_ch = y_ch;
    initial_long = lm_initial_x;
    final_long = lm_final_x;
    initial_short = lm_initial_y;
    final_short = lm_final_y;
  }
  else
  {
    long_ch = y_ch;
    short_ch = x_ch;
    initial_long = lm_initial_y;
    final_long = lm_final_y;
    initial_short = lm_initial_x;
    final_short = lm_final_x;
  }

  if(final_long > initial_long) long_dir = 1;
  else long_dir = -1;
  if(final_short > initial_short) short_dir = 1;
  else short_dir = -1;
        
  current_long = (int)(dac_data[long_ch]);
  current_short = (int)(dac_data[short_ch]);
  delta_long = abs(final_long - initial_long);
  delta_short = abs(final_short - initial_short);
  if(delta_short != 0)
  {
    ratio = (float)delta_long / (float)delta_short;
  }
  else
  {
    ratio = (float)delta_long + 1.0; // safety
  }

  manip_data->cut_x1 = BITS_TO_STEPS(lm_initial_x,data);
  manip_data->cut_x2 = BITS_TO_STEPS(lm_final_x,data);
  manip_data->cut_y1 = BITS_TO_STEPS(lm_initial_y,data);
  manip_data->cut_y2 = BITS_TO_STEPS(lm_final_y,data);
  manip_data->min_x = 0.0;
  manip_data->max_x = sqrt( (manip_data->cut_x2 - manip_data->cut_x1) *
                                  (manip_data->cut_x2 - manip_data->cut_x1) + 
                                  (manip_data->cut_y2 - manip_data->cut_y1) *
                                  (manip_data->cut_y2 - manip_data->cut_y1) );

  repaint_lm_status(hDlg,"Ramp to Image conditions");
  ramp_ch(i_setpoint_ch,lm_image_i,lm_ramp_delay,0,1); // ramp to image current (order may be important)
  ramp_bias(lm_image_bias,lm_ramp_delay,0,1);                  // ramp through zero??
  
  repaint_lm_status(hDlg,"Move to start location");
  // move to the initial point
  dac_data[long_ch]  = move_to_protect(long_ch,current_long,initial_long,
                                   lm_interstep_delay,CRASH_STOP,LM_LIMIT_PERCENT);
  dac_data[short_ch] = move_to_protect(short_ch,current_short,initial_short,
                                   lm_interstep_delay,CRASH_STOP,LM_LIMIT_PERCENT);
  if(dac_data[long_ch] != (unsigned int)initial_long || dac_data[short_ch] != (unsigned int)initial_short)
  {
    MessageBox(hDlg,"Aborting","Unable to move to start location",MB_ICONHAND);
    return;
  }
  else
  {
    current_long = initial_long;
    current_short = initial_short;
  }
        
  manipulating = 1;
  manip_size = 0;

  repaint_lm_status(hDlg,"Ramp to Manip conditions");
  ramp_bias(lm_manip_bias,lm_ramp_delay,0,1);
  ramp_ch(i_setpoint_ch,lm_manip_i,lm_ramp_delay,0,1); // ramp to manip current (order may be important)

  // trace the path
  for(current_long = initial_long;current_long != final_long;current_long+=long_dir)
  {
    ramp_ch(long_ch,(unsigned int)current_long,lm_interstep_delay,0,1 /*bit per step*/);
    bump = (unsigned int)((abs(current_short - initial_short) + 1) * ratio);
    if(abs(current_long - initial_long) == (double)bump)
	{
      ramp_ch(short_ch,(unsigned int)(current_short+=short_dir),lm_interstep_delay,0,1 /*bit per step*/);
	}
    if(lm_read_z  && (abs(current_long - initial_long) % lm_measure_every == 0))
	{
      dio_start_clock(lm_read_delay);
      dio_wait_clock();
      lm_data = dio_read(lm_read_n_times);
      *(manip_data->yf+manip_size) = lm_data;
      manip_size++;
	}

    sprintf(string,"Manipulating %d//%d",current_long,final_long);
    repaint_lm_status(hDlg,string);
    PeekMessage(&manip_Message,hDlg,NULL,NULL,PM_REMOVE);
    if((LOWORD(manip_Message.wParam) == VK_ESCAPE) || (LOWORD(manip_Message.wParam) == LM_STOP))
	{
      manipulating = 0;
	}
    if(!manipulating) break;
  }

  if(manip_size > 0)
  {
    manip_data->size = manip_size;
    manip_data->valid = 1;
  }

  // update dac_data just to be sure
  dac_data[long_ch] = (unsigned int)current_long;
  dac_data[short_ch] = (unsigned int)current_short;
        
  repaint_lm_status(hDlg,"Ramp to Image conditions");
  ramp_ch(i_setpoint_ch,lm_image_i,lm_ramp_delay,0,1); // ramp to image current (order may be important)
  ramp_bias(lm_image_bias,lm_ramp_delay,0,1);
  repaint_lm_status(hDlg,"Idle.");
}

#endif
