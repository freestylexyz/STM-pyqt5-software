#include <math.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <dos.h>
#include "common.h"
#include "file.h"
#include "data.h"
#include "stm.h"
#include "dio.h"
#include "spec.h"
#include "oscill.h"
#include "clock.h"
#include "scan.h"
#include "track.h"
#include "vibmap.h"
#include "image.h"
#include "infospc.h"
#include "serial.h"
#include "highres.h"


// spec scan settings
int stop = 0;
int sp_scanning = 0;
datadef *sp_data[SP_DATASETS];
int sp_current_default = 0;
datadef *sp_defaults_data[SP_NUM_DEFAULTS];
int sp_current = 0;
int sp_scan_min;
int sp_scan_max;
int sp_out_range; /* 1 is +-5 V, 2 is +-10V */
int sp_input_ch = i_in_ch;
int sp_output_ch = sample_bias_ch;
int sp_speed = 10;
int sp_move_delay = 10;
int sp_ramp_delay = 10;
int sp_start_delay[4] = { 0,0,0,0 };//Mod. 1 Shaowei, default start delay time 0 ms
int sp_start_delay_on[4] = { 0,0,0,0 }; // Mod. 1 Shaowei, default start delay off
int sp_fb_delay[4] = { 0,0,0,0 };//Mod. 3 Shaowei, default feedback delay time 0 ms
int sp_fb_delay_on[4] = {0,0,0,0};//Mod. 3 Shaowei, default feedback delay between two passes off
int sp_step_delay[4] = { 0,0,0,0 };//Mod. 5 Shaowei, default step delay time 0 ms
int sp_step_delay_on[4] = { 0,0,0,0 }; // Mod. 5 Shaowei, default step delay off
int sp_final_delay[4] = { 0,0,0,0 };//Mod. 4 Shaowei, default final delay time 0 ms
int sp_final_delay_on [4] = { 0,0,0,0 }; // Mod. 4 Shaowei, default final delay off
int sp_bits_per_step = 1;
int sp_dither_wait = 100000;
int sp_feedback_wait = 1000;
int sp_feedback_at_min = 0;
int sp_z_offset = 0;
unsigned int sp_num_samples = 100;
int sp_num_passes = 100;
int sp_measure_i = 1;
int sp_measure_z = 0;
int sp_measure_2 = 0;
int sp_measure_3 = 0;
int sp_forward = 1;
int sp_backward = 1;
int sp_avg_both = 1;
int sp_dither0 = SP_DITHER0_OFF;
int sp_dither1 = SP_DITHER1_OFF;
int sp_feedback_every = 0;
int sp_z_offset_bias = ZERO;
int sp_use_z_offset_bias = 0;
int sp_feedback = 0;
int sp_mode = SP_MODE_ABSOLUTE;
int sp_rel_min = 0,sp_rel_max = 0;
unsigned int sp_output_initial;
int sp_crash_protection = SP_CRASH_IN_RANGE;
float sp_max,sp_min;
int sp_x = 0,sp_y = 0;

// tracking variables
int sp_track_offset_x = 0,sp_track_offset_y = 0;
int sp_track_limit_x_min,sp_track_limit_x_max;
int sp_track_limit_y_min,sp_track_limit_y_max;
int sp_track_max_bits = 200;
int sp_track_sample_every = 100;
int sp_track_at_min = 0;
int sp_tracking = 0;
int sp_tracking_mode = SP_TRACK_MAX;
int sp_track_auto_auto = 0;
int sp_track_avg_data_pts = 200;
int sp_track_iterations = 10;
int sp_track_every = 0;
float sp_track_plane_a = 0.0;
float sp_track_plane_b = 0.0;
int sp_track_step_delay = 1000;
int sp_offset_hold = 0;
unsigned int sp_takeoff_x,sp_takeoff_y;
unsigned int sp_landing_x,sp_landing_y;

// mapping variables
int sp_multiple_scans = 0;	// boolean for whether or not we're doing multiple scans
int sp_scan_count = 0;		// number of points (scans) in currently loaded map 
OFFSET_PT sp_offsets_map[SP_MAX_MAP_SIZE];

datadef *second_data[SP_DATASETS];
int current_second_data = 0;
int sp_cur_selection = SP_UPPER_IMAGE;
int sp_screen_min;
int sp_screen_max;
int sp_screen_min_2;
int sp_screen_max_2;
float sp_vert_max = SP_VERT_MAX;		// vert_min and vert_max set vertical voltage scale (in bits) for graph 
float sp_vert_min = SP_VERT_MIN;
float sp_vert_max_2 = SP_VERT_MAX;
float sp_vert_min_2 = SP_VERT_MIN;
float sp_vert_max_max = SP_VERT_MAX;
float sp_vert_min_min = SP_VERT_MIN;
int sp_first_time = 1;
int sp_p1_vbits = SP_MIN_MIN;           // p1 is blue pointer, p2 is red pointer
int sp_p2_vbits = SP_MAX_MAX;
int sp_p1_vbits_2 = SP_MIN_MIN;
int sp_p2_vbits_2 = SP_MAX_MAX;
// trigger setting
int sp_trigger=0;//Mod. 2 Shaowei Default passtrigger is off.
int sp_steptrigger=0;//Mod. 5 Shaowei Default steptrigger is off.
int sp_trigger_low = 2048;//Mod. 2 Shaowei default trigger low = 0V
int sp_trigger_high = 3072;//Mod. 2 Shaowei default trigger high = 5V
int sp_trigger_time = 50;//Mod. 2 Shaowei default trigger time 50ms
unsigned int sp_trigger_ch = 7;//Mod. 2 Shaowei, default trigger channel 7
unsigned int sp_trigger_now = 2048;//Mod. 2 Shaowei default trigger ch. at low
//read sequence variables
int sp_use_readsq=0;
//int sp_background = IN_ZERO;
//float sp_p1_press = 0;
//float sp_p2_press = 0;
//float sp_p1_calib = -1;
//float sp_p2_calib = -1;
//float sp_tot_integral=0;
//float sp_part_integral=0;
//int sp_interval = 0; /* interval between steps in microsec */
//int sp_didv = 0;
//int sp_measure = SP_MEASURE_I;
//datadef *zoom_data = NULL;
//int zoom_is_valid = 0;
//datadef *sp_data_iv,*sp_data_didv;
// float didv_weights[SP_WEIGHTS] = {-573/143,2166/143,-1244/143,-3776/143,-3084/143};

extern unsigned int dac_data[];
extern datadef *scan_defaults_data[];
extern datadef **glob_data;
extern int scan_current_default;
extern int z_offset_range;
extern unsigned int scan_z;
extern datadef *data;
extern unsigned int input_ch,out1;
extern int feedback;
extern unsigned int    tip_gain;
extern unsigned int i_setpoint;
extern int i_set_range;
extern unsigned int sample_bias;
extern int sample_bias_range;
extern int z_offset_ch;
extern unsigned int bit16;
extern float lockin_sen1;
extern float lockin_sen2;
extern float lockin_osc;
extern float lockin_w;
extern float lockin_tc;
extern float lockin_n1;
extern float lockin_pha1;
extern float lockin_pha2;
extern int lockin_in_use;
extern int datapro;

extern char     string[];
extern char *current_file_spc;
extern char *current_dir_spc;
extern int file_spc_count;
extern struct commentdef *gcomment;
extern HANDLE   hInst;
extern int infospc_caller;

static int out_smart = 0;
static int dither0_before,dither1_before;
static HPEN hpen_black,hpen_red,hpen_green,hpen_blue,
    hpen_dash,hpen_white,hpen_cyan;
static HRGN spec_area;
static PAINTSTRUCT ps;

static int init_spec(HWND,int,int,int);
static int sp_data_valid(int);
static void sp_scan_enable(HWND,int);
static void spec_set_title(HWND);
static void sp_status(HWND,int,char *);
static void sp_update(HWND,int);
static void spec_scan(HWND,int *,int);
static int init_sp_input_data(HWND,int *);
static void make_spec_readseq(datadef *);
static void repaint_graphics(HWND);
static void repaint_p2(HWND);
static void repaint_vert_scale(HWND);
static void repaint_vert_scale_min(HWND);
static void repaint_spec_scale(HWND);
static void repaint_current(HWND);
static void repaint_screen_scroll(HWND);
static void set_data(datadef *);
static void get_data(datadef *);
static void repaint_mapping(HWND);
static void repaint_vib_map_params(HWND, int);
static void repaint_track_displacement(HWND);
static void repaint_all(HWND);
static void enable_vib_map_params(HWND, int);
//static void repaint_track_limits();
extern void sp_calc_scan();

extern BOOL FAR PASCAL PrescanDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL SpecOptDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL LockinDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL CommentDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL VibMapDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL InfospcDlg(HWND,unsigned,WPARAM,LPARAM);

BOOL FAR PASCAL SpecDlg(HWND, unsigned, WPARAM, LPARAM);
// Scan dialog. For obtaining input voltages as a function of output voltages:
// i-v curves, STS, STM-IETS, and so on. Most of the settings for spectroscopy
// must be set with the Spectroscopy Options dialog (SpecOptDlg). 

BOOL FAR PASCAL SpecDlg(HWND hDlg, unsigned Message, WPARAM wParam, LPARAM lParam)
{
  int id,result;
  POINT mouse_pos;
  int i,mini,maxi,delt,ddelt;
  char filename[256];
  int start,end;
  double d,minid,maxid,deltd,ddeltd;
  float data_max,data_min;
  static FARPROC lpfnDlgProc;
  int pass_count;
  int strobe_before;


    
  switch(Message)
  {
    case WM_INITDIALOG:

	  if(datapro)//datapro mode Mod. Shaowei
	  {
	  EnableWindow(GetDlgItem(hDlg,SP_OPTIONS),FALSE);
      EnableWindow(GetDlgItem(hDlg,SP_PRESCAN),FALSE);
	  EnableWindow(GetDlgItem(hDlg,SP_SCAN),FALSE);
	  EnableWindow(GetDlgItem(hDlg,SPE_OFFSET_AUTO_BELOW),FALSE);
	  EnableWindow(GetDlgItem(hDlg,SP_TRIGGER_LOW),FALSE);
	  EnableWindow(GetDlgItem(hDlg,SP_TRIGGER_HIGH),FALSE);
	  }


      if(clock_used)  // probably not needed w/DSP. -mw
      {
        MessageBox(hDlg,"Clock already in use!","Oops",MB_ICONEXCLAMATION);
        EndDialog(hDlg,TRUE);
        return(TRUE);
      }
      else
      {
        sp_out_range = get_range(sp_output_ch);
        if(sp_first_time)
        {
          sp_first_time = 0;
          for(i = 0;i < SP_DATASETS;i++)
		  {
            sp_data[i] = NULL;
		  }
          for(i = 0;i < SP_DATASETS;i++)
		  {
            second_data[i] = NULL;
		  }
                
          sp_scan_max = vtod(2,sp_out_range);
          sp_scan_min = vtod(1,sp_out_range);
          for(i = 0;i < SP_NUM_DEFAULTS;i++) 
		  {
            sp_defaults_data[i] = NULL;
            alloc_data(&(sp_defaults_data[i]),DATATYPE_GEN2D,MAX,GEN2D_NONE,GEN2D_NONE,0);
            strcpy(filename,SP_INI);
            sprintf(filename + strlen(filename),"%d.ini",i);
            glob_data = &sp_defaults_data[i];
            if(!load_mas(filename))
			{
              set_data(sp_defaults_data[i]);
              sp_defaults_data[i]->type2d = TYPE2D_SPEC_I;
			}
		  }
                
		}
        get_data(sp_defaults_data[sp_current_default]);
        sp_num_passes = sp_defaults_data[sp_current_default]->num_passes;
		sp_scan_max = sp_defaults_data[sp_current_default]->end;
        SetScrollRange(GetDlgItem(hDlg,SP_P1_SCROLL),SB_CTL,0,32767,TRUE);
        SetScrollRange(GetDlgItem(hDlg,SP_P2_SCROLL),SB_CTL,0,32767,TRUE);
        SetScrollRange(GetDlgItem(hDlg,SP_SCREEN_SCROLL),SB_CTL,0,32767,TRUE);
        SetScrollRange(GetDlgItem(hDlg,SP_V_SCALE_SCROLL),SB_CTL,0,32767,TRUE);
        SetScrollRange(GetDlgItem(hDlg,SP_V_SCALE_MIN_SCROLL),SB_CTL,0,32767,TRUE);
        SetScrollRange(GetDlgItem(hDlg,SP_CURRENT),SB_CTL,0,32767,TRUE);
        switch(sp_current_default)
		{
          case 0:
            CheckDlgButton(hDlg,SP_SETTINGS_1,1);
            break;
          case 1:
            CheckDlgButton(hDlg,SP_SETTINGS_2,1);
            break;
          case 2:
            CheckDlgButton(hDlg,SP_SETTINGS_3,1);
            break;
          case 3:
            CheckDlgButton(hDlg,SP_SETTINGS_4,1);
            break;
		}

		hpen_black = CreatePen(PS_SOLID,1,RGB(0,0,0));
        hpen_red = CreatePen(PS_SOLID,1,RGB(255,0,0));
        hpen_green = CreatePen(PS_SOLID,1,RGB(0,200,0));
        hpen_blue = CreatePen(PS_SOLID,1,RGB(0,0,255));
        hpen_cyan = CreatePen(PS_SOLID,1,RGB(255,0,255));
        hpen_white = CreatePen(PS_SOLID,1,RGB(255,255,255));
        hpen_dash = CreatePen(PS_DASH,1,RGB(0,0,0));
//        hpen_old = SelectObject(oscill_hdc,hpen_black);
        spec_area = CreateRectRgn(SP_X_MIN,SP_Y_MIN,SP_X_MAX,SP_Y_MAX);
        sp_calc_scan();
        repaint_all(hDlg);
		
	  }    
      break;

    case WM_PAINT:
      BeginPaint(hDlg,&ps);
      if(RectInRegion(spec_area,&(ps.rcPaint)))  
	  {
        repaint_graphics(hDlg);
	  }
      EndPaint(hDlg,&ps);
      break;

    case WM_LBUTTONDOWN:
      mouse_pos.x = LOWORD(lParam);
      mouse_pos.y = HIWORD(lParam);
      if(mouse_pos.x >= SP_X_MIN && mouse_pos.x <= SP_X_MAX &&
         mouse_pos.y >= SP_Y_MIN && mouse_pos.y <= SP_Y_MAX)
	  {
	    if(sp_cur_selection == SP_UPPER_IMAGE)
		{
          sp_p1_vbits = (mouse_pos.x - SP_X_MIN) * (sp_screen_max - sp_screen_min + 1) /
                        (SP_X_MAX - SP_X_MIN + 1) + sp_screen_min;
          repaint_graphics(hDlg);
          //repaint_integral(hDlg);
	    }
	    else
		{
		  sp_cur_selection = SP_UPPER_IMAGE;
          repaint_graphics(hDlg);
		  repaint_current(hDlg);
	    }                 
	  } 
	  else if(mouse_pos.x >= SP_X_MIN && mouse_pos.x <= SP_X_MAX &&
              mouse_pos.y >= SP_Y_MIN_2  && mouse_pos.y <= SP_Y_MAX_2)
	  {
	    if(sp_cur_selection == SP_LOWER_IMAGE)
		{
          sp_p1_vbits_2 = (mouse_pos.x - SP_X_MIN) * (sp_screen_max_2 - sp_screen_min_2 + 1) /
                          (SP_X_MAX - SP_X_MIN + 1) + sp_screen_min_2;
          repaint_graphics(hDlg);
		}
	    else
		{
		  sp_cur_selection = SP_LOWER_IMAGE;
          repaint_graphics(hDlg);
		  repaint_current(hDlg);
	    }
	  }
	  repaint_screen_scroll(hDlg);
	  repaint_vert_scale(hDlg);
	  repaint_vert_scale_min(hDlg);
	  spec_set_title(hDlg);
      break;
    case WM_RBUTTONDOWN:
      mouse_pos.x  = LOWORD(lParam);
      mouse_pos.y  = HIWORD(lParam);
      if(mouse_pos.x >= SP_X_MIN && mouse_pos.x <= SP_X_MAX &&
         mouse_pos.y >= SP_Y_MIN && mouse_pos.y <= SP_Y_MAX &&
	     sp_cur_selection == SP_UPPER_IMAGE)
	  { 
        sp_p2_vbits = (mouse_pos.x - SP_X_MIN) * (sp_screen_max - sp_screen_min + 1) /
                      (SP_X_MAX - SP_X_MIN + 1) + sp_screen_min;
        repaint_graphics(hDlg);
        //repaint_integral(hDlg);
	  }
      if(mouse_pos.x >= SP_X_MIN && mouse_pos.x <= SP_X_MAX &&
         mouse_pos.y >= SP_Y_MIN_2 && mouse_pos.y <= SP_Y_MAX_2 &&
	     sp_cur_selection == SP_LOWER_IMAGE)
	  {
         sp_p2_vbits_2 = (mouse_pos.x - SP_X_MIN) * (sp_screen_max_2 - sp_screen_min_2 + 1) /
                         (SP_X_MAX - SP_X_MIN + 1) + sp_screen_min_2;
         repaint_graphics(hDlg);
         //repaint_integral(hDlg);
	  }
      break;

    case WM_VSCROLL:
    case WM_HSCROLL:
      if(!out_smart)
      {
        out_smart = 1;
        id = getscrollid();
        switch(id)
        {
          case SP_CURRENT_SCROLL:
            if(sp_cur_selection == SP_UPPER_IMAGE)
			  d = (double)sp_current;
			else
			  d = (double)current_second_data;
            minid = 0;
            maxid = SP_DATASETS - 1;
            deltd = 1;
            ddeltd = 4;
            break;
          case SP_V_SCALE_SCROLL:
		    if(sp_cur_selection == SP_UPPER_IMAGE)
			{
              d = (double)sp_vert_max;
              minid = sp_vert_min + abs((int)(sp_vert_max_max - sp_vert_min_min)) / 100;
              maxid = sp_vert_max_max;
              deltd = abs((int)(sp_vert_max_max - sp_vert_min_min)) / 100;
              ddeltd = abs((int)(sp_vert_max_max - sp_vert_min_min)) / 10;
		    }
		    else
			{
              d = (double)sp_vert_max_2;
              minid = sp_vert_min_2 + abs((int)(sp_vert_max_max - sp_vert_min_min)) / 100;
              maxid = sp_vert_max_max;
              deltd = abs((int)(sp_vert_max_max - sp_vert_min_min)) / 100;
              ddeltd = abs((int)(sp_vert_max_max - sp_vert_min_min)) / 10;
		    }
            break;
          case SP_V_SCALE_MIN_SCROLL:
		    if(sp_cur_selection == SP_UPPER_IMAGE)
			{
              d = (double)sp_vert_min;
              minid = sp_vert_min_min;
              maxid = sp_vert_max - abs((int)(sp_vert_max_max - sp_vert_min_min)) / 100;
              deltd = abs((int)(sp_vert_max_max - sp_vert_min_min)) / 100;
              ddeltd = abs((int)(sp_vert_max_max - sp_vert_min_min)) / 10;
		    }
		    else
			{
              d = (double)sp_vert_min_2;
              minid = sp_vert_min_min;
              maxid = sp_vert_max_2 - abs((int)(sp_vert_max_max - sp_vert_min_min)) / 100;
              deltd = abs((int)(sp_vert_max_max - sp_vert_min_min)) / 100;
              ddeltd = abs((int)(sp_vert_max_max - sp_vert_min_min)) / 10;
		    }
            break;
          case SP_P1_SCROLL:
		    if(sp_cur_selection == SP_UPPER_IMAGE)
			{
              d = (double)sp_p1_vbits;
              ddeltd = (double)(sp_screen_max - sp_screen_min) / 10;
              minid = 0;
			  if(sp_data[sp_current])
			  {
                if(sp_data[sp_current]->bit16 &&
			      (sp_data[sp_current]->output_ch == 10 || sp_data[sp_current]->output_ch == 11))
				{
                  maxid = SP_NUM_CH * 16 - 1;
				}
			    else maxid = SP_NUM_CH - 1;
			  }
			  else maxid = SP_NUM_CH - 1;
              deltd = 1;
			}
		    else
			{
              d = (double)sp_p1_vbits_2;
              ddeltd = (double)(sp_screen_max_2 - sp_screen_min_2) / 10;
              minid = 0;
			  if(second_data[current_second_data])
			  {
                if(second_data[current_second_data]->bit16 &&
			      (second_data[current_second_data]->output_ch == 10 || second_data[current_second_data]->output_ch == 11))
				{
                  maxid = SP_NUM_CH * 16 - 1;
				}
			    else maxid = SP_NUM_CH - 1;
			  } 
			  else maxid = SP_NUM_CH - 1;
              deltd = 1;
			}
            break;
          case SP_P2_SCROLL:
		    if(sp_cur_selection == SP_UPPER_IMAGE)
			{
              d = (double)sp_p2_vbits;
              ddeltd = (double)(sp_screen_max - sp_screen_min) / 10;
              minid = 0;
			  if(sp_data[sp_current])
			  {
                if(sp_data[sp_current]->bit16 &&
			      (sp_data[sp_current]->output_ch == 10 || sp_data[sp_current]->output_ch == 11))
				{
                  maxid = SP_NUM_CH * 16 - 1;
				}
			    else maxid = SP_NUM_CH - 1;
			  }
			  else maxid = SP_NUM_CH - 1;
              deltd = 1;
			}
		    else
			{
              d = (double)sp_p2_vbits_2;
              ddeltd = (double)(sp_screen_max_2 - sp_screen_min_2) / 10;
              minid = 0;
			  if(second_data[current_second_data])
			  {
                if(second_data[current_second_data]->bit16 &&
			      (second_data[current_second_data]->output_ch == 10 || second_data[current_second_data]->output_ch == 11))
				{
                  maxid = SP_NUM_CH * 16 - 1;
				}
			    else maxid = SP_NUM_CH - 1;
			  }
			  else maxid = SP_NUM_CH - 1;
              deltd = 1;
			}
            break;
          case SP_SCREEN_SCROLL:
		    if(sp_cur_selection == SP_UPPER_IMAGE)
			{
              d = (double)sp_screen_min;
              if(sp_data_valid(sp_current))
              {
                minid = sp_data[sp_current]->min_x;
                maxid = sp_data[sp_current]->max_x;
              }
              else
              {
                minid = sp_screen_min;
                maxid = sp_screen_max;
              }
              deltd = 1;
              ddeltd = (double)(sp_screen_max - sp_screen_min) / 10;
			}
		    else
			{
              d = (double)sp_screen_min_2;
              if(second_data[current_second_data] != NULL)
              {
		        if(second_data[current_second_data]->valid)
				{
                  minid = second_data[current_second_data]->min_x;
                  maxid = second_data[current_second_data]->max_x;
				}
			  }
              else
			  {
                minid = sp_screen_min_2;
                maxid = sp_screen_max_2;
			  }
              deltd = 1;
              ddeltd = (double)(sp_screen_max_2 - sp_screen_min_2) / 10;
			}
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
        switch(id)
		{
          case SP_CURRENT_SCROLL:
            if(sp_cur_selection == SP_UPPER_IMAGE)
            {
			  sp_current = (int)floor(d + 0.5);
              if(sp_data_valid(sp_current))
			  {
                get_data(sp_data[sp_current]);
                repaint_all(hDlg);
			  }
              else repaint_current(hDlg);
			}
			else
			{
			  current_second_data = (int)floor(d + 0.5);
              if(second_data[current_second_data] != NULL)
              {
				if(second_data[current_second_data]->valid)
				{
                  repaint_all(hDlg);
				}
			  }
              else repaint_current(hDlg);
			}
			break;
          case SP_V_SCALE_SCROLL:
		    if(sp_cur_selection == SP_UPPER_IMAGE)
			{
              sp_vert_max = (float)d;
		    }
		    else
			{
              sp_vert_max_2 = (float)d;
		    }
            repaint_vert_scale(hDlg);
            repaint_graphics(hDlg);
            break;
          case SP_V_SCALE_MIN_SCROLL:
		    if(sp_cur_selection == SP_UPPER_IMAGE)
			{
              sp_vert_min = (float)d;
		    }
		    else
			{
              sp_vert_min_2 = (float)d;
		    }
            repaint_vert_scale_min(hDlg);
            repaint_graphics(hDlg);
            break;
		  case SP_P1_SCROLL:
            if(sp_cur_selection == SP_UPPER_IMAGE)
			{
              sp_p1_vbits = (unsigned int)d;
			  if(sp_data[sp_current]->bit16 &&
			    (sp_data[sp_current]->output_ch == 10 || sp_data[sp_current]->output_ch == 11))
			  {
			    SetScrollPos(GetDlgItem(hDlg,SP_P1_SCROLL),SB_CTL,
				         DTOI(sp_p1_vbits,SP_MIN_MIN,SP_MAX_MAX * 16),TRUE);
                sprintf(string,"%0.4f",dtov16(sp_p1_vbits));
			  }
			  else
			  { 
			    SetScrollPos(GetDlgItem(hDlg,SP_P1_SCROLL),SB_CTL,
				         DTOI(sp_p1_vbits,SP_MIN_MIN,SP_MAX_MAX),TRUE);
                sprintf(string,"%0.3f",dtov(sp_p1_vbits,get_range(sp_data[sp_current]->output_ch)));
			  }
			}
		    else
			{ 
		      int range = 2;
              sp_p1_vbits_2 = (unsigned int)d;
			  if(second_data[current_second_data]->bit16 &&
		        (second_data[current_second_data]->output_ch == 10 || second_data[current_second_data]->output_ch == 11))
			  {
                SetScrollPos(GetDlgItem(hDlg,SP_P1_SCROLL),SB_CTL,
				         DTOI(sp_p1_vbits_2,SP_MIN_MIN,SP_MAX_MAX * 16),TRUE);
                sprintf(string,"%0.4f",dtov16(sp_p1_vbits_2));
			  }
			  else
			  {
                SetScrollPos(GetDlgItem(hDlg,SP_P1_SCROLL),SB_CTL,
				         DTOI(sp_p1_vbits_2,SP_MIN_MIN,SP_MAX_MAX),TRUE);
			    if(second_data[current_second_data] != NULL )
				{
		          if(second_data[current_second_data]->valid)
					range = second_data[current_second_data]->v_range;
				}
                sprintf(string,"%0.3f",dtov(sp_p1_vbits_2,range));
			  }
			}
            SetDlgItemText(hDlg, SP_P1_VOLTS, string);
            //repaint_integral(hDlg);
            repaint_graphics(hDlg);
            break;
          case SP_P2_SCROLL:
		    if(sp_cur_selection == SP_UPPER_IMAGE)
			{
              sp_p2_vbits = (unsigned int)d;
			  if(sp_data[sp_current]->bit16 &&
			    (sp_data[sp_current]->output_ch == 10 || sp_data[sp_current]->output_ch == 11))
			  {
                SetScrollPos(GetDlgItem(hDlg,SP_P2_SCROLL),SB_CTL,
			 	         DTOI(sp_p2_vbits,SP_MIN_MIN,SP_MAX_MAX * 16),TRUE);
                sprintf(string,"%0.4f",dtov16(sp_p2_vbits));
			  }
			  else
			  {
                SetScrollPos(GetDlgItem(hDlg,SP_P2_SCROLL),SB_CTL,
				         DTOI(sp_p2_vbits,SP_MIN_MIN,SP_MAX_MAX),TRUE);
                sprintf(string,"%0.3f", dtov(sp_p2_vbits,get_range(sp_data[sp_current]->output_ch)));
			  }
			}
		    else
			{
		      int range = 2;
              sp_p2_vbits_2 = (unsigned int)d;
			  if(second_data[current_second_data]->bit16 &&
		        (second_data[current_second_data]->output_ch == 10 || second_data[current_second_data]->output_ch == 11))
			  {
		        SetScrollPos(GetDlgItem(hDlg,SP_P2_SCROLL),SB_CTL,
				           DTOI(sp_p2_vbits_2,SP_MIN_MIN,SP_MAX_MAX * 16),TRUE);
                sprintf(string,"%0.4f",dtov16(sp_p2_vbits_2));
			  }
			  else
			  {
		        SetScrollPos(GetDlgItem(hDlg,SP_P2_SCROLL),SB_CTL,
				           DTOI(sp_p2_vbits_2,SP_MIN_MIN,SP_MAX_MAX),TRUE);
		        if(second_data[current_second_data] != NULL)
				{
		          if(second_data[current_second_data]->valid) range = second_data[current_second_data]->v_range;
				}
                sprintf(string,"%0.3f",dtov(sp_p2_vbits_2,range));
			  }
			}
            SetDlgItemText(hDlg,SP_P2_VOLTS,string);
            //repaint_integral(hDlg);
            repaint_graphics(hDlg);
            break;
		  case SP_SCREEN_SCROLL:
		    if(sp_cur_selection == SP_UPPER_IMAGE)
			{
              sp_screen_max += (unsigned int)d - sp_screen_min;
              sp_screen_min = (unsigned int)d;
              repaint_screen_scroll(hDlg);
              repaint_graphics(hDlg);
			}
		    else
			{
              sp_screen_max_2 += (unsigned int)d - sp_screen_min_2;
              sp_screen_min_2 = (unsigned int) d;
              repaint_screen_scroll(hDlg);
              repaint_graphics(hDlg);
			}
            break;
        }
        out_smart = 0;
      }
      break;

    case WM_COMMAND:
      switch (LOWORD(wParam))
	  {
		// default spectroscopy settings
        case SP_SETTINGS_1:
          sp_current_default = 0;
          get_data(sp_defaults_data[sp_current_default]);
          sp_num_passes = sp_defaults_data[sp_current_default]->num_passes;
//          sp_scan_max = sp_defaults_data[sp_current_default]->end;
          repaint_all(hDlg);
          break;
        case SP_SETTINGS_2:
          sp_current_default = 1;
          get_data(sp_defaults_data[sp_current_default]);
          sp_num_passes = sp_defaults_data[sp_current_default]->num_passes;
//          sp_scan_max = sp_defaults_data[sp_current_default]->end;
          repaint_all(hDlg);
          break;
        case SP_SETTINGS_3:
          sp_current_default = 2;
          get_data(sp_defaults_data[sp_current_default]);
          sp_num_passes = sp_defaults_data[sp_current_default]->num_passes;
//          sp_scan_max = sp_defaults_data[sp_current_default]->end;
          repaint_all(hDlg);
          break;
        case SP_SETTINGS_4:
          sp_current_default = 3;
          get_data(sp_defaults_data[sp_current_default]);
          sp_num_passes = sp_defaults_data[sp_current_default]->num_passes;
//          sp_scan_max = sp_defaults_data[sp_current_default]->end;
          repaint_all(hDlg);
          break;

        // opening other dialogs
        case SP_PRESCAN:
          lpfnDlgProc = MakeProcInstance(PrescanDlg,hInst);
          DialogBox(hInst,"PRESCANDLG",hDlg,lpfnDlgProc);
          FreeProcInstance(lpfnDlgProc);
          sp_calc_scan();
          repaint_spec_scale(hDlg);
          break;
        case SP_CHOOSE_MAP:
          lpfnDlgProc = MakeProcInstance(VibMapDlg,hInst);
          DialogBox(hInst,"VIBMAPDLG",hDlg,lpfnDlgProc);
          FreeProcInstance(lpfnDlgProc);
	      repaint_all(hDlg);
	      break;
	    case SP_VIB_MAP:
	      sp_multiple_scans = IsDlgButtonChecked(hDlg,LOWORD(wParam));
	      if(sp_multiple_scans && sp_scan_count == 0)
		  {
		    // in case no map has been loaded or created yet
		    sp_scan_count = 1;
		    sp_offsets_map[0].x = sp_offsets_map[0].y = 0;
		  }
	      repaint_mapping( hDlg);
	      break;
        case SP_OPTIONS:

		  // Mod. #5, X. Cao
		{
		  int stop5 = 0;
		  char string51[128];		  

		  for(i=0; i<SP_DATASETS; i++)
		  {
		    if(sp_data_valid(i) && !sp_data[i]->saved)
			{
			  sprintf(string51,"Channel %d not saved!\nPlease save or clear first", i);
			  MessageBox(hDlg, string51, "Warning", MB_OK | MB_ICONEXCLAMATION);
			  stop5 = 1;
			  break;
			}
			else;
		  }
		  if(stop5)	break;
		}
		  // end of Mod. #5, X. Cao

          lpfnDlgProc = MakeProcInstance(SpecOptDlg,hInst);
          DialogBox(hInst,"SPECOPTDLG",hDlg,lpfnDlgProc);
          FreeProcInstance(lpfnDlgProc);
          sp_calc_scan();
          repaint_all(hDlg);
          break;
        case SP_LOCKIN:
          lpfnDlgProc = MakeProcInstance(LockinDlg,hInst);
          DialogBox(hInst,"LOCKINDLG",hDlg,lpfnDlgProc);
          FreeProcInstance(lpfnDlgProc);
          repaint_all(hDlg);
          break;
        case SP_COMMENT:
          if(sp_cur_selection == SP_UPPER_IMAGE)
		  {
            if(sp_data_valid(sp_current))
            {
              gcomment = &(sp_data[sp_current]->comment);
              lpfnDlgProc = MakeProcInstance(CommentDlg,hInst);
              DialogBox(hInst,"COMMENTDLG",hDlg,lpfnDlgProc);
              FreeProcInstance(lpfnDlgProc);
			}
		  }
	      else
		  {
	        if(second_data[current_second_data] != NULL)
			{
              if(second_data[current_second_data]->valid)
              {
                gcomment = &(second_data[current_second_data]->comment);
                lpfnDlgProc = MakeProcInstance(CommentDlg,hInst);
                DialogBox(hInst,"COMMENTDLG",hDlg,lpfnDlgProc);
                FreeProcInstance(lpfnDlgProc);
			  }
			} 
		  }
	      break;
	    case SP_INFO:
	      if(sp_cur_selection == SP_LOWER_IMAGE && second_data[current_second_data] != NULL)
		  {
	        if(second_data[current_second_data]->valid)
			{
	          infospc_caller = INFOSPC_CALLER_SPC;
              lpfnDlgProc = MakeProcInstance(InfospcDlg,hInst);
              DialogBox(hInst,"INFOSPCDLG", hDlg,lpfnDlgProc);
              FreeProcInstance(lpfnDlgProc);
			} 
		  }
          break;

        // adjust or clear current sp data
        case SP_CURRENT:
          if(!out_smart)
          {
            out_smart = 1;
            GetDlgItemText(hDlg,SP_CURRENT,string,9);
            if(sp_cur_selection == SP_UPPER_IMAGE)
			{
              sp_current = (unsigned int)atoi(string);
              sp_current = min(max(sp_current,0),SP_DATASETS);
              SetScrollPos(GetDlgItem(hDlg,SP_CURRENT_SCROLL),SB_CTL, 
                       DTOI(sp_current,0,SP_DATASETS-1),TRUE);
              if(sp_data_valid(sp_current))
			  {
                get_data(sp_data[sp_current]);
                repaint_all(hDlg);
			  }
              spec_set_title(hDlg);
            }
            else
			{
              current_second_data = (unsigned int)atoi(string);
              current_second_data = min(max(sp_current,0),SP_DATASETS);
              SetScrollPos(GetDlgItem(hDlg,SP_CURRENT_SCROLL),SB_CTL, 
                       DTOI(current_second_data,0,SP_DATASETS-1),TRUE);
              if(second_data[current_second_data] != NULL)
			  {
			    if(second_data[current_second_data]->valid)
				{
                  repaint_all(hDlg);
				}
			  }
              spec_set_title(hDlg);
            }

            out_smart = 0;
		  }
          break;
		case SP_CLEAR_ALL:
		if (MessageBox(hDlg,"CLEAR ALL DATA?","Warning",MB_OKCANCEL) == IDOK){ //Joon 11/03/2003
		  if(sp_cur_selection == SP_UPPER_IMAGE)
		  {
            for(i = 0;i < SP_DATASETS;i++)
			{
              if(sp_data[i] != NULL && sp_data[i]->valid ) 
				free_data(&(sp_data[i]));
			}
		  }
		  else
          {
            for(i = 0;i < SP_DATASETS;i++)
			{
              if(second_data[i] != NULL && second_data[i]->valid ) 
				free_data(&(second_data[i]));
			}
		  }
		  // sp_data[i]->valid & second_data[i]->valid for double check to avoid
		  // CLEAR_ALL crash
		  // X. Cao
          repaint_graphics(hDlg);
        } 
          break;

        case SP_CLEAR:
        if (MessageBox(hDlg,"CLEAR DATA?","Warning",MB_OKCANCEL) == IDOK){ //Joon 11/03/2003 
		  if(sp_cur_selection == SP_UPPER_IMAGE)
		  {
            if(sp_data_valid(sp_current))
			{
              free_data(&(sp_data[sp_current]));
              repaint_graphics(hDlg);
			}
		  }
		  else
		  {
            if(second_data[current_second_data] != NULL)
			{
			  if(second_data[current_second_data]->valid)
			  {
                free_data(&(second_data[current_second_data]));
                repaint_graphics(hDlg);
			  }
			}
          }
		}
          break;

        // vertical scale
		case SP_V_SCALE:
          if(!out_smart)
          {
            out_smart = 1;
            GetDlgItemText(hDlg, SP_V_SCALE, string, 9);
            if(sp_cur_selection == SP_UPPER_IMAGE)
			{
              sp_vert_max = (float)in_vtod(atof(string));
              sp_vert_max = min(max(sp_vert_max,
                                sp_vert_min + abs((int)(sp_vert_max_max - sp_vert_min_min)) / 100),
                                sp_vert_max_max);
              SetScrollPos(GetDlgItem(hDlg,SP_V_SCALE_SCROLL),SB_CTL,
					DTOI(sp_vert_max,sp_vert_min_min,sp_vert_max_max),TRUE);
			}
		    else
			{
		      sp_vert_max_2 = (float)in_vtod(atof(string));
              sp_vert_max_2 = min(max(sp_vert_max_2,
                                  sp_vert_min_2 + abs((int)(sp_vert_max_max - sp_vert_min_min)) / 100),
                                  sp_vert_max_max);
              SetScrollPos(GetDlgItem(hDlg,SP_V_SCALE_SCROLL),SB_CTL,
					  DTOI(sp_vert_max_2,sp_vert_min_min,sp_vert_max_max),TRUE);
			}
            repaint_graphics(hDlg);
            out_smart = 0;
		  }
          break;
        case SP_V_SCALE_MIN:
          if(!out_smart)
		  {
            out_smart = 1;
            GetDlgItemText(hDlg,SP_V_SCALE_MIN,string,9);
            if(sp_cur_selection == SP_UPPER_IMAGE)
			{
		      sp_vert_min = (float)in_vtod(atof(string));
              sp_vert_min = min(max(sp_vert_min,sp_vert_min_min),
                                sp_vert_max - abs((int)(sp_vert_max_max - sp_vert_min_min)) / 100);
              SetScrollPos(GetDlgItem(hDlg,SP_V_SCALE_MIN_SCROLL),SB_CTL,
					DTOI(sp_vert_min,sp_vert_min_min,sp_vert_max_max),TRUE);
			} 
		    else
			{
		      sp_vert_min_2 = (float)in_vtod(atof(string));
              sp_vert_min_2 = min(max(sp_vert_min_2,sp_vert_min_min),
                                  sp_vert_max_2 - abs((int)(sp_vert_max_max - sp_vert_min_min)) / 100);
              SetScrollPos(GetDlgItem(hDlg,SP_V_SCALE_MIN_SCROLL),SB_CTL,
					DTOI(sp_vert_min_2,sp_vert_min_min,sp_vert_max_max),TRUE);
			}
            repaint_graphics(hDlg);
            out_smart = 0;
		  }
          break;
        case SP_ZOOM_PEAK:
	      if(sp_cur_selection == SP_UPPER_IMAGE)
		  {
            if(sp_data_valid(sp_current))
			{
              if(sp_screen_min > sp_data[sp_current]->min_x) 
			  {
                start = (int)((sp_screen_min - sp_data[sp_current]->min_x) /
                         sp_data[sp_current]->step);
			  }
              else
			  {
                start = 0;
			  }
              if(sp_screen_max < sp_data[sp_current]->max_x) 
			  {
                end = (int)((sp_screen_max - sp_data[sp_current]->min_x) /
                        sp_data[sp_current]->step);
			  }
              else 
			  {
                end = sp_data[sp_current]->size;
			  }
              data_max = sp_data[sp_current]->yf[start];
              for(i = start;i < end;i++)
			  {
                if(sp_data[sp_current]->yf[i] > data_max) data_max = sp_data[sp_current]->yf[i];
			  }
              sp_vert_max = data_max;
			}
		  }
	      else
		  {
            if(second_data[current_second_data] != NULL)
			{
		      if(second_data[current_second_data]->valid)
			  {
                if(sp_screen_min_2 > second_data[current_second_data]->min_x) 
				{
                  start = (int)((sp_screen_min_2 - second_data[current_second_data]->min_x) /
                           second_data[current_second_data]->step);
				}
                else
				{
                  start = 0;
				}
                if(sp_screen_max_2 < second_data[current_second_data]->max_x) 
				{
                  end = (int)((sp_screen_max_2 - second_data[current_second_data]->min_x) /
                         second_data[current_second_data]->step);
				}
                else 
				{
                  end = second_data[current_second_data]->size;
				}
                data_max = second_data[current_second_data]->yf[start];
                for(i = start;i < end;i++)
				{
                  if(second_data[current_second_data]->yf[i] > data_max) data_max = second_data[current_second_data]->yf[i];
				}
                sp_vert_max_2 = data_max;
			  } 
			}
		  }
          repaint_vert_scale(hDlg);
          repaint_graphics(hDlg);
          break;
        case SP_MAX_CENTER:
	      if(sp_cur_selection == SP_UPPER_IMAGE)
		  {
            if(sp_data_valid(sp_current))
			{
              if(sp_screen_min > sp_data[sp_current]->min_x) 
			  {
                start = (int)((sp_screen_min - sp_data[sp_current]->min_x) /
                         sp_data[sp_current]->step);
			  }
              else
			  {
                start = 0;
			  }
              if(sp_screen_max<sp_data[sp_current]->max_x) 
			  {
                end = (int)((sp_screen_max - sp_data[sp_current]->min_x) /
                       sp_data[sp_current]->step);
			  }
              else 
			  {
                end =sp_data[sp_current]->size;
			  }
              data_max = sp_data[sp_current]->yf[start];
              data_min = sp_data[sp_current]->yf[start];
              for(i = start;i < end;i++)
			  {
                if(sp_data[sp_current]->yf[i] < data_min) data_min = sp_data[sp_current]->yf[i];
                if(sp_data[sp_current]->yf[i] > data_max) data_max = sp_data[sp_current]->yf[i];
			  }
              if(data_min >= IN_ZERO) data_min = IN_ZERO - (data_max - IN_ZERO);
              if(data_max <= IN_ZERO) data_max = IN_ZERO + (IN_ZERO - data_min);
              if(IN_ZERO - data_min > data_max - IN_ZERO)
			  {
                data_max = IN_ZERO + (IN_ZERO - data_min);
			  }
              else
			  {
                data_min = IN_ZERO - (data_max - IN_ZERO);
			  }
              sp_vert_min = data_min;
              sp_vert_max = data_max;
			}
		  }
	      else
		  {
	        if(second_data[current_second_data] != NULL)
			{
              if(second_data[current_second_data]->valid)
			  {
                if(sp_screen_min_2 > second_data[current_second_data]->min_x) 
				{
                  start = (int)((sp_screen_min_2 - second_data[current_second_data]->min_x) /
                           second_data[current_second_data]->step);
				}
                else
				{
                  start = 0;
				}
                if(sp_screen_max_2 < second_data[current_second_data]->max_x) 
				{
                  end = (int)((sp_screen_max_2 - second_data[current_second_data]->min_x) /
                        second_data[current_second_data]->step);
				}
                else 
				{
                  end = second_data[current_second_data]->size;
				}
                data_max = second_data[current_second_data]->yf[start];
                data_min = second_data[current_second_data]->yf[start];
                for(i = start;i < end;i++)
				{
                  if(second_data[current_second_data]->yf[i] < data_min) data_min = second_data[current_second_data]->yf[i];
                  if(second_data[current_second_data]->yf[i] > data_max) data_max = second_data[current_second_data]->yf[i];
				}
                if(data_min >= IN_ZERO) data_min = IN_ZERO - (data_max - IN_ZERO);
                if(data_max <= IN_ZERO) data_max = IN_ZERO + (IN_ZERO - data_min);
                if(IN_ZERO - data_min > data_max - IN_ZERO)
				{
                  data_max = IN_ZERO + (IN_ZERO - data_min);
				}
                else
                {
                  data_min = IN_ZERO - (data_max - IN_ZERO);
                }
                sp_vert_min_2 = data_min;
                sp_vert_max_2 = data_max;
			  }
			}
		  }
          repaint_vert_scale(hDlg);
          repaint_vert_scale_min(hDlg);
          repaint_graphics(hDlg);
          break;
        case SP_ZOOM_TO_MIN:
	      if(sp_cur_selection == SP_UPPER_IMAGE)
		  {
            if(sp_data_valid(sp_current))
			{
              if(sp_screen_min > sp_data[sp_current]->min_x) 
			  {
                start = (int)((sp_screen_min - sp_data[sp_current]->min_x) /
                         sp_data[sp_current]->step);
			  }
              else
			  {
                start = 0;
			  }
              if(sp_screen_max < sp_data[sp_current]->max_x) 
			  { 
                end = (int)((sp_screen_max - sp_data[sp_current]->start) /
                        sp_data[sp_current]->step);
			  } 
              else 
			  {
                end = sp_data[sp_current]->size;
			  }
              data_min = sp_data[sp_current]->yf[start];
              for(i = start;i < end;i++)
			  {
                if(sp_data[sp_current]->yf[i] < data_min) data_min = sp_data[sp_current]->yf[i];
			  }
              sp_vert_min = data_min;
			}
		  }
	      else
		  {
	        if(second_data[current_second_data] != NULL)
			{
              if(second_data[current_second_data]->valid)
			  {
                if(sp_screen_min_2 > second_data[current_second_data]->min_x) 
				{
                  start = (int)((sp_screen_min_2 - second_data[current_second_data]->min_x) /
                           second_data[current_second_data]->step);
				}
                else
				{
                  start = 0;
				}
                if(sp_screen_max_2 < second_data[current_second_data]->max_x) 
				{
                  end = (int)((sp_screen_max_2 - second_data[current_second_data]->start) /
                         second_data[current_second_data]->step);
				} 
                else 
				{
                  end = second_data[current_second_data]->size;
				}
                data_min = second_data[current_second_data]->yf[start];
                for(i = start;i < end;i++)
				{
                  if(second_data[current_second_data]->yf[i] < data_min) data_min = second_data[current_second_data]->yf[i];
				}
                sp_vert_min_2 = data_min;
			  } 
			} 
		  } 
          repaint_vert_scale_min(hDlg);
          repaint_graphics(hDlg);
          break;

        // pointers (p1 == blue, p2 == red) 
        case SP_P1_VOLTS:
          if(!out_smart)
          {
            out_smart = 1;
            GetDlgItemText(hDlg,SP_P1_VOLTS,string,9);
		    if(sp_cur_selection == SP_UPPER_IMAGE)
			{
			  if(sp_data[sp_current]->bit16 &&
				(sp_data[sp_current]->output_ch == 10 || sp_data[sp_current]->output_ch == 11))
			  {
                sp_p1_vbits = (unsigned int)vtod16(atof(string));
                sp_p1_vbits = min(max(sp_p1_vbits,SP_MIN_MIN),SP_MAX_MAX * 16);
                SetScrollPos(GetDlgItem(hDlg,SP_P1_SCROLL),SB_CTL,
					       DTOI(sp_p1_vbits,SP_MIN_MIN,SP_MAX_MAX * 16),TRUE);
			  }
			  else
			  {
                sp_p1_vbits = (unsigned int)vtod(atof(string),sp_data[sp_current]->output_ch);
                sp_p1_vbits = min(max(sp_p1_vbits,SP_MIN_MIN),SP_MAX_MAX);
                SetScrollPos(GetDlgItem(hDlg,SP_P1_SCROLL),SB_CTL,
					       DTOI(sp_p1_vbits,SP_MIN_MIN,SP_MAX_MAX),TRUE);
			  }
			}
		    else
			{
		      int range = 2;
		      if(second_data[current_second_data] != NULL )
			  {
		        if(second_data[current_second_data]->valid) 
				{
		          range = second_data[current_second_data]->v_range;
				}
			  }
	  	      else range = 2;
			  if(second_data[current_second_data]->bit16 &&
				(second_data[current_second_data]->output_ch == 10 || second_data[current_second_data]->output_ch == 11))
			  {
                sp_p1_vbits_2 = (unsigned int)vtod16(atof(string));
                sp_p1_vbits_2 = min(max(sp_p1_vbits_2,SP_MIN_MIN),SP_MAX_MAX * 16);
                SetScrollPos(GetDlgItem(hDlg,SP_P1_SCROLL),SB_CTL,
					       DTOI(sp_p1_vbits_2,SP_MIN_MIN,SP_MAX_MAX * 16),TRUE);
			  }
			  else
			  {
                sp_p1_vbits_2 = (unsigned int)vtod(atof(string),range);
                sp_p1_vbits_2 = min(max(sp_p1_vbits_2,SP_MIN_MIN),SP_MAX_MAX);
                SetScrollPos(GetDlgItem(hDlg,SP_P1_SCROLL),SB_CTL,
					       DTOI(sp_p1_vbits_2,SP_MIN_MIN,SP_MAX_MAX),TRUE);
			  }
			}
            //repaint_integral(hDlg);
            repaint_graphics(hDlg);
            out_smart = 0;
		  }
          break;
        case SP_P2_VOLTS:
          if(!out_smart)
          {
            out_smart = 1;
            GetDlgItemText(hDlg,SP_P2_VOLTS,string,9);
		    if(sp_cur_selection == SP_UPPER_IMAGE)
			{
			  if(sp_data[sp_current]->bit16 &&
				(sp_data[sp_current]->output_ch == 10 || sp_data[sp_current]->output_ch == 11))
              {
                sp_p2_vbits = (unsigned int)vtod16(atof(string));
                sp_p2_vbits = min(max(sp_p2_vbits,SP_MIN_MIN),SP_MAX_MAX * 16);
                SetScrollPos(GetDlgItem(hDlg,SP_P2_SCROLL),SB_CTL,
				         DTOI(sp_p2_vbits,SP_MIN_MIN,SP_MAX_MAX * 16),TRUE);
			  }
			  else
			  {
                sp_p2_vbits = (unsigned int)vtod(atof(string),get_range(sp_data[sp_current]->output_ch));
                sp_p2_vbits = min(max(sp_p2_vbits,SP_MIN_MIN),SP_MAX_MAX);
                SetScrollPos(GetDlgItem(hDlg,SP_P2_SCROLL),SB_CTL,
				         DTOI(sp_p2_vbits,SP_MIN_MIN,SP_MAX_MAX),TRUE);
			  }
			}
		    else
			{
		      int range = 2;
		      if(second_data[current_second_data] != NULL)
			  {
		        if(second_data[current_second_data]->valid)
				{
		          range = second_data[current_second_data]->v_range;
				}
			  }
	  	      else range = 2;
			  if(second_data[current_second_data]->bit16 &&
				(second_data[current_second_data]->output_ch == 10 || second_data[current_second_data]->output_ch == 11))
			  {
                sp_p2_vbits_2 = (unsigned int)vtod16(atof(string));
                sp_p2_vbits_2 = min(max(sp_p2_vbits_2,SP_MIN_MIN),SP_MAX_MAX * 16);
                SetScrollPos(GetDlgItem(hDlg,SP_P2_SCROLL),SB_CTL,
				         DTOI(sp_p2_vbits_2,SP_MIN_MIN,SP_MAX_MAX * 16),TRUE);
			  }
			  else
			  {
                sp_p2_vbits_2 = (unsigned int)vtod(atof(string),range);
                sp_p2_vbits_2 = min(max(sp_p2_vbits_2,SP_MIN_MIN),SP_MAX_MAX);
                SetScrollPos(GetDlgItem(hDlg,SP_P2_SCROLL),SB_CTL,
				         DTOI(sp_p2_vbits_2,SP_MIN_MIN,SP_MAX_MAX),TRUE);
			  }
			}
            //repaint_integral(hDlg);
            repaint_graphics(hDlg);
            out_smart = 0;
		  }
          break;

        // loading, saving files
        case SP_LOAD:
	      if(sp_cur_selection == SP_UPPER_IMAGE)
		  {
            free_data(&(sp_data[sp_current]));
            alloc_data(&(sp_data[sp_current]),DATATYPE_GEN2D,1,GEN2D_NONE,
                       GEN2D_NONE,0);
            glob_data = &(sp_data[sp_current]);
            result = file_open(hDlg,hInst,FTYPE_SPC,0,current_file_spc);
            if(result)
            {
              sp_data[sp_current]->valid = 1;
              get_data(sp_data[sp_current]);
              sp_out_range = sp_data[sp_current]->v_range;
              if(sp_data[sp_current]->version > 2)
			  {
                sp_speed = (int)(sp_data[sp_current]->speed);
			  }
              else sp_speed = (int)((float)sp_out_range * 10000000.0 / 4096.0 / 
				               sp_data[sp_current]->speed);
//              sp_scale= sp_data[sp_current]->scale;
              sp_screen_min = sp_scan_min;
              sp_screen_max = sp_scan_max;
              sp_vert_max = SP_VERT_MAX;
              sp_vert_min = SP_VERT_MIN;
	          if(bit16 && (sp_output_ch == 10 || sp_output_ch == 11))
			  {
	            sp_p1_vbits = min(max(sp_p1_vbits,SP_MIN_MIN),SP_MAX_MAX * 16);
	            sp_p2_vbits = min(max(sp_p2_vbits,SP_MIN_MIN),SP_MAX_MAX * 16); 
			  }
	          else
			  {
	            sp_p1_vbits = min(max(sp_p1_vbits,SP_MIN_MIN),SP_MAX_MAX);
	            sp_p2_vbits = min(max(sp_p2_vbits,SP_MIN_MIN),SP_MAX_MAX); 
			  } 
              repaint_all(hDlg);
			}
		  }
	      else
		  {
            free_data(&(second_data[current_second_data]));
            alloc_data(&(second_data[current_second_data]),DATATYPE_GEN2D,1,GEN2D_NONE,GEN2D_NONE,0);
            glob_data = &(second_data[current_second_data]);
            result = file_open(hDlg,hInst,FTYPE_SPC,0,current_file_spc);

            if(result)
            {
              second_data[current_second_data]->valid = 1;
              sp_out_range = second_data[current_second_data]->v_range; // what is this var used for ???
              sp_screen_min_2 = second_data[current_second_data]->start;
              sp_screen_max_2 = second_data[current_second_data]->end;
              sp_vert_max_2 = SP_VERT_MAX;
              sp_vert_min_2 = SP_VERT_MIN;
	          if(second_data[current_second_data]->bit16 &&
				(second_data[current_second_data]->output_ch == 10 || second_data[current_second_data]->output_ch == 11))
			  {
	            sp_p1_vbits_2 = min(max(sp_p1_vbits_2,SP_MIN_MIN),SP_MAX_MAX * 16);
	            sp_p2_vbits_2 = min(max(sp_p2_vbits_2,SP_MIN_MIN),SP_MAX_MAX * 16); 
			  }
	          else
			  {
	            sp_p1_vbits_2 = min(max(sp_p1_vbits_2,SP_MIN_MIN),SP_MAX_MAX);
	            sp_p2_vbits_2 = min(max(sp_p2_vbits_2,SP_MIN_MIN),SP_MAX_MAX); 
			  } 
              repaint_all(hDlg);
			}
		  } 
          spec_set_title(hDlg);
          break;


        case SP_SAVE:
	      if(sp_cur_selection == SP_UPPER_IMAGE)
		  {
            if(sp_data_valid(sp_current))
            {
              glob_data = &(sp_data[sp_current]);
              if(file_save_as(hDlg,hInst,FTYPE_SPC))
			  {
                inc_file(current_file_spc,&file_spc_count);
			  }
              sp_data[sp_current]->saved = 1;
			}
            for(i = 0;i < SP_DATASETS;i++)
            { 
              if(sp_data_valid(i)) 
              if(!sp_data[i]->saved)
              {
                sprintf(string,"Save Channel %d too?",i);
                if(MessageBox(hDlg,string, 
                        "Notice",MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK)
				{
                  glob_data = &(sp_data[i]);
                  if(file_save_as(hDlg,hInst,FTYPE_SPC))
				  {
                    inc_file(current_file_spc,&file_spc_count);
				  }
				}
                sp_data[i]->saved = 1;
			  }
			}
		  }
	      else
		  {
	        if(second_data[current_second_data] != NULL)
			{
	          if(second_data[current_second_data]->valid)
			  {
                glob_data = &(second_data[current_second_data]);
                file_save_as(hDlg,hInst,FTYPE_SPC);
                second_data[current_second_data]->saved = 1;
			  }
			}
		  } 
          spec_set_title(hDlg);
          break;    

		// begin of Mod. #4 (Save All), X. Cao
        case SP_SAVE_ALL:

		  if(file_spc_count==1)
		  {
			MessageBox(hDlg,"First file.\nPlease use Save to set directory.","Save All",MB_OK);
			break;
		  }
		  else;

	      if(sp_cur_selection == SP_UPPER_IMAGE)
		  {
			char saveall_filename[500];
		    char first_saved[500];
			char last_saved[500];
		    char string2[100];
			char string3[100];
			int data_saved;
//			char path[CURRENT_DIR_MAX];
			HANDLE overwrite_danger = NULL;	// prevent overwrite
			WIN32_FIND_DATA overwrite_file;	// prevent overwrite

			strcpy(string3,"Channel ");
			
			strcpy(first_saved, current_file_spc);
			strcpy(last_saved, current_file_spc);

			data_saved = 0;

			//MessageBox(hDlg,string3,"Save All",MB_OK);
            
			for(i = 0;i < SP_DATASETS;i++)
            {
              if(sp_data_valid(i) && !sp_data[i]->saved)
              {
				strcpy(saveall_filename,current_dir_spc);
				strcat(saveall_filename,"\\");
				strcat(saveall_filename,current_file_spc);

				//check if there is already a file of the current name
				// to avoid over-writing
				overwrite_danger = FindFirstFile(saveall_filename, &overwrite_file);
				FindClose(overwrite_danger);
				//INVALID_HANDLE_VALUE is defined as 0xffffffff in ...\VC98\Include\WinBase.h
				if(overwrite_danger != 0xffffffff)	
				{
				  break;
				}

				//prepare data for save
				//Begin of Mod. #7
				//set_data(sp_data[i]);	This line is redundant and causes some problem in counting passes.
				//End of Mod. #7
		        glob_data = &(sp_data[i]);
			    
				//update the string
				strcpy(last_saved, current_file_spc);

				inc_file(current_file_spc,&file_spc_count);

				save_mas(saveall_filename,1);

				//MessageBox(hDlg,string3,"Save All",MB_OK);

				sprintf(string2, "%d, ", i);
			    strcat(string3, string2);
				
				sp_data[i]->saved = 1;
				data_saved = 1;

			  }
			  else;
			}

			//INVALID_HANDLE_VALUE is defined as 0xffffffff in ...\VC98\Include\WinBase.h
			if(overwrite_danger != 0xffffffff)
			{
			  MessageBox(hDlg,"Might overwrite.\nPlease use Save instead.","Save All",MB_OK);
			  break;
			}

			if(data_saved)
			{
			  sprintf(string2, "saved as file %s through %s", first_saved, last_saved);
			  strcat(string3, string2);
			  MessageBox(hDlg,string3,"Save All",MB_OK);

  			  sprintf(string3, "%s ~ %s", first_saved, last_saved);
			  SetDlgItemText(hDlg,SP_STATUS,string3);
			}
			else
			{
			  MessageBox(hDlg,"No unsaved data", "Save All", MB_OK);
			}

		  }
	      else
		  {
			MessageBox(hDlg, "Please select the upper image", "Save All Error", MB_OK);
		  } 
          spec_set_title(hDlg);
          break;    
		// end of Mod. #4 (Save All), X. Cao

		// zoom in, zoom out (horizontal scale)
        case SP_ZOOM_IN:
	      if(sp_cur_selection == SP_UPPER_IMAGE)
		  {
            if(sp_p1_vbits < sp_p2_vbits)
			{
              sp_screen_min = sp_p1_vbits;
              sp_screen_max = sp_p2_vbits;
            }
            else if(sp_p2_vbits < sp_p1_vbits)
            {
              sp_screen_min = sp_p2_vbits;
              sp_screen_max = sp_p1_vbits;
			}
		  }
	      else
		  {
            if(sp_p1_vbits_2 < sp_p2_vbits_2)
            { 
              sp_screen_min_2 = sp_p1_vbits_2;
              sp_screen_max_2 = sp_p2_vbits_2;
            }
            else if(sp_p2_vbits_2 < sp_p1_vbits_2)
            {
              sp_screen_min_2 = sp_p2_vbits_2;
              sp_screen_max_2 = sp_p1_vbits_2;
			}
		  }
          repaint_screen_scroll(hDlg);
          repaint_graphics(hDlg);
          break;
        case SP_ZOOM_OUT:
	      if(sp_cur_selection == SP_UPPER_IMAGE)
		  {
            if(sp_data_valid(sp_current))
            { 
              sp_screen_min = (int)(sp_data[sp_current]->min_x);
              if(!sp_scanning)
			  {
                sp_screen_max = (int)(sp_data[sp_current]->max_x);
			  }
              else
			  {
                sp_screen_max = sp_scan_max;
			  }
              repaint_screen_scroll(hDlg);
              repaint_graphics(hDlg);
			}
		  }
	      else
		  {
            if(second_data[current_second_data] != NULL)
			{
	          if(second_data[current_second_data]->valid)
			  {
                sp_screen_min_2 = (int)(second_data[current_second_data]->min_x);
                sp_screen_max_2 = (int)(second_data[current_second_data]->max_x);
                repaint_screen_scroll(hDlg);
                repaint_graphics(hDlg);
			  }
			}
		  }
          break;




		//Mod. 2 Shaowei Set Trigger Channel
		case SP_TRIGGER_HIGH://Trigger to high
		  if(sp_trigger||sp_steptrigger)
		  {
		   sp_trigger_now = ramp_serial(sp_trigger_ch,sp_trigger_now,
		                                 sp_trigger_high,10,0);
		  }
		  else
		  {
		  MessageBox(hDlg, "Spectroscopy trigger is off", "Trigger Error", MB_OK);
		  }
		break;
		
		case SP_TRIGGER_LOW: //Trigger to low
		  if(sp_trigger||sp_steptrigger)
		  {
		  sp_trigger_now = ramp_serial(sp_trigger_ch,sp_trigger_now,
		                                 sp_trigger_low,10,0);
		  }
		  else
		  {
		  MessageBox(hDlg, "Spectroscopy trigger is off", "Trigger Error", MB_OK);
		  }
		break;
        //End of Mod. 2 Shaowei 
		// do a spec scan
        case SP_SCAN:
          // begin of Mod. #3, X. Cao
		  // check if all dataset saved or not. This is to prevent the 4/2 crash

		{
		  char string31[128];
		  stop = 0;
          if(!sp_scanning)	// if not scanning, do the following
		    for(i=0; i<SP_DATASETS; i++)
			{
			  if(sp_data_valid(i) && !sp_data[i]->saved)
			  {
			    sprintf(string31,"Channel %d not saved!\nPlease save or clear first", i);
			    MessageBox(hDlg, string31, "Warning", MB_OK | MB_ICONEXCLAMATION);
			    stop = 1;
			    break;
			  }
			  else;
			}
		  else	// if scanning, ask whether or not to stop
		  {
			sprintf(string31, "This stops the scan.");
			MessageBox(hDlg, string31, "Message", MB_OK);
		    sp_scanning = 0;
			stop = 1;
		  }

		  if(stop)	break;
		}
		  // end of Mod. #3, X. Cao

          if(!sp_scanning)
          {
		    // initialization
	 	    sp_cur_selection = SP_UPPER_IMAGE;
			if(bit16 && (sp_output_ch == 10 || sp_output_ch == 11))
			{
              if(sp_output_ch == 10)
			    sp_output_initial = (dac_data[extra_ch] & 0xF00) << 4;
              else // (sp_output_ch == 11)
			    sp_output_initial = (dac_data[extra_ch] & 0x0F0) << 4;
		      sp_output_initial += dac_data[sp_output_ch];
			}
			else sp_output_initial = dac_data[sp_output_ch];
            sp_calc_scan();
            strobe_before = 0;

		    // save the current spectroscopy options, i.e. Sp#.ini
            set_data(sp_defaults_data[sp_current_default]);
            strcpy(filename,SP_INI);
            sprintf(filename + strlen(filename),"%d.ini",sp_current_default);
            glob_data = &sp_defaults_data[sp_current_default];
            save_mas(filename,0);

	        if(!init_sp_input_data(hDlg,&strobe_before)) break;

            sp_scanning = 1;

            repaint_current(hDlg);
            sp_screen_min = sp_scan_min;
            sp_screen_max = sp_scan_max;
            sp_vert_max = SP_VERT_MAX;
            sp_vert_min = SP_VERT_MIN;
            repaint_screen_scroll(hDlg);  
            repaint_vert_scale(hDlg);
            repaint_vert_scale_min(hDlg);
            sp_scan_enable(hDlg,0);

		    // remember initial dithering status
            dither0_before = dio_dither_status(0);
            dither1_before = dio_dither_status(1);

            // initialize deltaX, deltaY variables
			if(sp_track_every)
			{
              sp_takeoff_x = dac_data[x_ch];
              sp_takeoff_y = dac_data[y_ch];
              sp_landing_x = dac_data[x_ch];
              sp_landing_y = dac_data[y_ch];
			}

		    // case of vibrational mapping
			if(sp_multiple_scans && sp_scan_count != 0)
			{
		      int j,k;
    		  char this_filename[500];
    		  char first_spc_filename[500]; // first saved spc file
    		  char last_spc_filename[500];  // last saved spc file

		      strcpy(first_spc_filename,current_file_spc);
		      strcpy(last_spc_filename,current_file_spc);

		      for(j = 0;j < sp_scan_count && sp_scanning;j++)
			  {
                CheckMsgQ();

		        repaint_vib_map_params(hDlg,j);

		        // update tracking offsets
		        sp_track_offset_x = sp_offsets_map[j].x;
		        sp_track_offset_y = sp_offsets_map[j].y;

		        // invalidate all of the previous scans
                for(k = 0;k < SP_DATASETS;k++) 
					// if(sp_data[k]) sp_data[k]->valid = 0;
					if(sp_data[k] && sp_data[k]->valid) sp_data[k]->valid = 0;
					// modified to remove the 4/2 data set crash
					// Mod. #2, X. Cao

		        // do the spectroscopic scan
	            spec_scan(hDlg,&pass_count,strobe_before);

		        repaint_graphics(hDlg);

		        // save the spectroscopic scan datasets
                for(k = 0;k < SP_DATASETS;k++)
				{
                  if(sp_data_valid(k))
				  {
			        //set_data(sp_data[k]);		Mod. #8, X. Cao, 2004-06-14
					//	The above line is redundant since set_data() was already called
					//	in init_spec(). What's more, if the mapping is stopped, the number
					//  of passes of the last point is set to maximum value specified in
					//  the options dialog. However, usually it should be less than that.
					//  This is exactly the same as the one found in SP_SAVE_All by Joonhee.
                    glob_data = &(sp_data[k]);
			        strcpy(this_filename,current_dir_spc);
			        strcat(this_filename,"\\");
			        strcat(this_filename,current_file_spc);
		            strcpy(last_spc_filename,current_file_spc);
			        inc_file(current_file_spc,&file_spc_count);
			        save_mas(this_filename,1);
			        sp_data[k]->saved = 1;
				  }
				}

                // ramp x and y back to tracking point
                i = (int)dac_data[x_ch] - sp_track_offset_x;
                if(i < 0) i = 0;
                if(i > MAX) i = MAX;
	            dac_data[x_ch] = ramp_serial(x_ch,dac_data[x_ch],i,SD->step_delay,0);
                i = (int)dac_data[y_ch] - sp_track_offset_y;
                if(i < 0) i = 0;
                if(i > MAX) i = MAX;
	            dac_data[y_ch] = ramp_serial(y_ch,dac_data[y_ch],i,SD->step_delay,0);

			  }
		      sprintf(string,"%d datasets save as files %s through %s",j,
				      first_spc_filename,last_spc_filename);
		      MessageBox(hDlg,string,"Vibrational Mapping Complete",MB_OK);
			  
			  // begin of Mod. #8, X. Cao, 2004-06-15, More status
			  sprintf(string, "%s ~ %s", first_spc_filename, last_spc_filename);
			  SetDlgItemText(hDlg,SP_STATUS,string);
			  // end of Mod. #8, X. Cao, 2004-06-15, More status
			} 
			// case of standard scan (no mapping)
			else
			{
              CheckMsgQ();
	          spec_scan( hDlg, &pass_count, strobe_before);
			}
            sp_scan_enable(hDlg,1);

          }  // end of if(!sp_scanning)

          sp_scanning = 0;
          spec_set_title(hDlg);
          break;
//Mod. Shaowei Auto-
        case SPE_OFFSET_AUTO_BELOW:
        auto_z_below(IN_ZERO);
		break;
//End Auto-

        case ENTER:
		  repaint_all(hDlg);
          break;
        case SP_EXIT:
          set_data(sp_defaults_data[sp_current_default]);
          strcpy(filename,SP_INI);
          sprintf(filename+strlen(filename),"%d.ini",sp_current_default);
          glob_data = &sp_defaults_data[sp_current_default];
          save_mas(filename,0);
          DeleteObject(hpen_black);
          DeleteObject(hpen_cyan);
          DeleteObject(hpen_white);
          DeleteObject(hpen_red);
          DeleteObject(hpen_green);
          DeleteObject(hpen_blue);
          DeleteObject(hpen_dash);
          DeleteObject(spec_area);
          EndDialog(hDlg,TRUE);
          return(TRUE);
          break;
      }
      break;
  }
  return (FALSE);
}                                                                  

static void spec_scan( HWND hDlg, int *pass_count, int strobe_before)
{
  // spec_scan performs one spectroscopic scan (possibly consisting of many passes)
  // Spec_scan can be broken down into the following steps:
    // I. Initial tracking and x,y offsets
	// II. Wait and then turn feedback on / off
	// III. Delta z-offset
    // IV. Ramp output channel to minimum scanning voltage
    // V. Dithering
    // VI. Passes
	  // VI.a Set up DSP and Start Spectroscopy
	  // VI.b Get updates from DSP as we ramp through the values
	  // VI.c Get data from DSP and average it in. Redraw.
	  // VI.d Trackig between passes, if applicable
	  // VI.e Feedback on between passes, if applicable


  int i,j;
  //unsigned int ui;
  unsigned int cur_x, cur_y;
  int sp_tracking = 1;
  int spec_steps = 0;
  int spec_steps_fwd = 0;
  int spec_steps_bkd = 0;
  unsigned int current_step_bits;
  int one_pass_index = 0;
  float *one_pass_buffer;
  int data_per_pass;
  int take_data_fwd,take_data_bkd;
  int buffer_index;
  char result;
  int finish_needed = 1;
  unsigned int sp_output_now = sp_output_initial;
  int tracking_mode = TRACK_MAX;
  int entry_feedback = feedback;

  // I. Initial tracking and x,y offsets
  

  if(sp_track_every && sp_track_at_min)
  {
    // track at the minimum scanning voltage
    dio_feedback_serial(sp_feedback == 1);
	sp_output_now = ramp_serial(sp_output_ch,sp_output_initial,
		                                 sp_scan_min,sp_ramp_delay,0);
	if(sp_output_now != sp_scan_min)
	{
      sprintf(string,"Unable to ramp to %d on ch %d. Aborting scan.",sp_scan_min,sp_output_ch);
	  mprintf(string);
	  return;
	}
  }
  if (sp_track_every) 
  {
    dio_feedback_serial(1);
    dio_dither_serial(0,0);
    dio_dither_serial(1,0);
    sp_status(hDlg,0,"tracking...");	
    
	track_setup(sp_track_avg_data_pts,sp_track_step_delay,
		        sp_track_step_delay,sp_track_plane_a,sp_track_plane_b);
	cur_x = dac_data[x_ch];
    cur_y = dac_data[y_ch];
	if(sp_tracking_mode == SP_TRACK_MAX) tracking_mode = TRACK_MAX;
	else tracking_mode = TRACK_MIN;
	track_start(cur_x,cur_y,tracking_mode);
    for(i = 0;i < sp_track_iterations && sp_scanning && sp_tracking;i++)
    {
	  cur_x = dac_data[x_ch];
	  cur_y = dac_data[y_ch];

      if(cur_x > sp_track_limit_x_max ||
		 cur_x < sp_track_limit_x_min ||
		 cur_y > sp_track_limit_y_max||
		 cur_y < sp_track_limit_y_min)
		  sp_tracking = 0;
      else track_again();

	}
	track_terminate();
	sp_landing_x = dac_data[x_ch];
	sp_landing_y = dac_data[y_ch];
    if(sp_track_offset_x) 
    {
      if(sp_offset_hold && !sp_feedback)
        dio_feedback_serial(sp_feedback == 1);
      i = (int)dac_data[x_ch] + sp_track_offset_x;
      if(i < 0) i = 0;
      if(i > MAX) i = MAX;
	  dac_data[x_ch] = ramp_serial(x_ch,dac_data[x_ch],i,SD->step_delay,0);
    }
    if(sp_track_offset_y) 
    {
      if(sp_offset_hold && !sp_feedback)
        dio_feedback_serial(sp_feedback == 1);
      i = (int)dac_data[y_ch] + sp_track_offset_y;
      if(i < 0) i = 0;
      if(i > MAX) i = MAX;
	  dac_data[y_ch] = ramp_serial(y_ch,dac_data[y_ch],i,SD->step_delay,0);
	}
  }	
  repaint_track_displacement(hDlg);

  // II. Wait and then turn feedback on / off
  if(sp_feedback != 1) 
  {
    if(sp_feedback_wait > SP_UPDATE_TIME) sp_update(hDlg,sp_output_initial);
    serial_soft_delay(sp_feedback_wait);
  }
  dio_feedback_serial(sp_feedback == 1);
  CheckMsgQ();	// Temp, X. Cao
  // III. Delta z-offset
  if(sp_z_offset)
  {
    sp_status(hDlg,0,"z-offset...");
	if(sp_use_z_offset_bias)
	{
	  if(sp_output_ch == sample_bias_ch)
	  {
        ramp_serial(sample_bias_ch,sp_output_now,
		        sp_z_offset_bias,sp_ramp_delay,0);
        sp_output_now = sp_z_offset_bias;
	  }
	  else
	  {
        ramp_serial(sample_bias_ch,dac_data[sample_bias_ch],
		        sp_z_offset_bias,sp_ramp_delay,0);
	  }
	}
    ramp_serial(z_offset_ch,dac_data[z_offset_ch],
		        dac_data[z_offset_ch] + sp_z_offset,DEFAULT_Z_OFFSET_TIME,0);
    serial_soft_delay(SP_CREEP_TIME);
    dac_data[z_offset_ch] += sp_z_offset;
    scan_z = dac_data[z_offset_ch];
  }
  CheckMsgQ();	// Temp, X. Cao
  // IV. Ramp output channel to minimum scanning voltage
  sp_status(hDlg,0,"ramping to start...");
  sp_output_now = ramp_serial(sp_output_ch,sp_output_now,sp_scan_min,sp_ramp_delay,0);

  // V. Dithering
  dio_dither_serial(0,sp_dither0 == SP_DITHER0_ON);
  dio_dither_serial(1,sp_dither1 == SP_DITHER1_ON);
  if(sp_dither0 == SP_DITHER0_ON || sp_dither1 == SP_DITHER1_ON)
  {
    if(sp_dither_wait > SP_UPDATE_TIME) sp_update(hDlg,sp_scan_min);
	serial_soft_delay(sp_dither_wait);
  }

  CheckMsgQ();	// Temp, X. Cao
   // pre-VI. Get ready for passes.
  take_data_fwd = sp_forward || sp_avg_both;
  take_data_bkd = sp_backward || sp_avg_both;
  spec_steps = (sp_scan_max - sp_scan_min) / sp_bits_per_step + 1;
  spec_steps_fwd = spec_steps;
  spec_steps_bkd = spec_steps;
  data_per_pass = 0;
  if(sp_measure_i) data_per_pass++;
  if(sp_measure_z) data_per_pass++;
  if(sp_measure_2) data_per_pass++;
  if(sp_measure_3) data_per_pass++;
  if(take_data_fwd && take_data_bkd) data_per_pass *= 2;
  if(!take_data_fwd && !take_data_bkd) data_per_pass = 0;
  one_pass_buffer = (float *)malloc(spec_steps * data_per_pass * sizeof(float));
  for(i = 0;i < spec_steps;i++)
    for(j = 0;j < 8;j++)
	  if(sp_data[j] && sp_data[j]->valid) sp_data[j]->yf[i] = 0.0; // reset
	  // if(sp_data[j]) sp_data[j]->yf[i] = 0.0; // reset
	  // sp_data[j]->valid added for more security
	  // X. Cao

  // VI. Passes
  for((*pass_count) = 0;(*pass_count) < sp_num_passes && sp_scanning;(*pass_count)++)
  {
    if(sp_trigger && !stop)// pre-VI.a Triggering Mod. 2 Shaowei 12/12/12 triggeing if needed
	{
	sp_status(hDlg, (*pass_count),"triggering...");// fix info update 2/9/2018
	sp_trigger_now = ramp_serial(sp_trigger_ch,sp_trigger_now,
		                                 sp_trigger_high,10,0);
	Sleep(sp_trigger_time);
    sp_trigger_now = ramp_serial(sp_trigger_ch,sp_trigger_now,
		                                 sp_trigger_low,10,0);
	}//end Mod. 2
	// pre-VI.b Start delay Mod. 1 Shaowei 12/12/12 start delay is added before taking data to allow enough time for communicating with other programs.
    if(sp_start_delay_on[sp_current_default] && !stop)
	{sp_status(hDlg,(*pass_count),"Waiting at start...");
    Sleep(sp_start_delay[sp_current_default]);
	}
	// end of Mod. 1
	
	
    sp_status(hDlg,(*pass_count),"");
	one_pass_index = 0;
//**If step trigger is enabled, skip DSP.  Start Mod. 5 Step Trigger by Shaowei 6/18/13
	CheckMsgQ();	// Temp, X. Cao
    if(!stop)//passstop
	{
	if(sp_steptrigger)
	{
		//shaowei 2/8/2018 fix spectroscopy info update during triggering
		current_step_bits = sp_scan_min;
		for (i=0; i<spec_steps; i++)
		{
		sp_update(hDlg, current_step_bits);
		current_step_bits += sp_bits_per_step;
		sp_status(hDlg, (*pass_count),"triggering...");
		sp_trigger_now = ramp_serial(sp_trigger_ch,sp_trigger_now,sp_trigger_high,10,0);
		Sleep(sp_trigger_time);
		sp_trigger_now = ramp_serial(sp_trigger_ch,sp_trigger_now,sp_trigger_low,10,0);
		CheckMsgQ();	// Temp, X. Cao
		if (stop) i=spec_steps;
		if(sp_step_delay_on[sp_current_default] && !stop)
		{
		    sp_status(hDlg,(*pass_count),"Waiting between steps");
			Sleep(sp_step_delay[sp_current_default]);
			//shaowei end of 2/8/2018
		}
		sp_status(hDlg,(*pass_count),"ramping bias");
		sp_output_now = ramp_serial(sp_output_ch,sp_output_now,sp_output_now + sp_bits_per_step,sp_ramp_delay,0);
		CheckMsgQ();	// Temp, X. Cao
		}
	}
	else //start of Else Mod. 5 Shaowei
	{
	// VI.a Set up DSP and Start Spectroscopy
    // Create ReadSeq based on current spec parameters. Setup.
	if(stop) break;
	make_spec_readseq(sp_defaults_data[sp_current_default]);
	readseq_setup(sp_defaults_data[sp_current_default]);
    spec_setup(sp_scan_min,sp_scan_max,sp_bits_per_step,sp_move_delay,sp_speed);
		// StartSpec
    take_data_fwd = sp_forward || sp_avg_both;
    take_data_bkd = sp_backward || sp_avg_both;
	spec_start(take_data_fwd,take_data_bkd,sp_output_ch);

	// VI.b Get updates from DSP as we ramp through the values
    current_step_bits = sp_scan_min;

    for(i = 0;i < spec_steps_fwd;i++)
	{		
      result = spec_continue();
      sp_status(hDlg,(*pass_count),"");
	  if(result == DSPCMD_UPDATE)
	  {
	    sp_update(hDlg,current_step_bits);
		current_step_bits += sp_bits_per_step;
	  }
	  else if(result == DSPCMD_RAMPREAD && take_data_fwd)
	  {
        sp_status(hDlg,(*pass_count),"transferring data");
        spec_finish(&(one_pass_buffer[one_pass_index]),1);
        sp_status(hDlg,(*pass_count),"");
		one_pass_index += DSP_DATA_SIZE;
	    if(spec_continue() == DSPCMD_UPDATE)
		{
	      sp_update(hDlg,current_step_bits);
		  current_step_bits += sp_bits_per_step;
		}
	    else
		{
	      mprintf("Comm Error during spec scan!");
		  return;
		}
	  }
	  else
	  {
	    mprintf("Comm Error during spec scan!");
		return;
	  }

	}

	// special case
	if(take_data_fwd && !take_data_bkd && 
	   ((spec_steps_fwd * data_per_pass) % DSP_DATA_SIZE == 0) )
    {
      sp_status(hDlg,(*pass_count),"transferring data");
	  spec_finish(&(one_pass_buffer[one_pass_index]),0);
      sp_status(hDlg,(*pass_count),"");
	  buffer_index += DSP_DATA_SIZE;
      finish_needed = 0;
	}

    current_step_bits = sp_scan_min + (spec_steps - 1) * sp_bits_per_step;

    for(i = 0;i < spec_steps_bkd;i++)
	{
      result = spec_continue();
      sp_status(hDlg,(*pass_count),"");
	  if(result == DSPCMD_UPDATE)
	  {
	    sp_update(hDlg,current_step_bits);
		current_step_bits -= sp_bits_per_step;
	  }
	  else if(result == DSPCMD_RAMPREAD && take_data_bkd)
	  {
        sp_status(hDlg,(*pass_count),"transferring data");
        spec_finish(&(one_pass_buffer[one_pass_index]),1);
        sp_status(hDlg,(*pass_count),"");
		one_pass_index += DSP_DATA_SIZE;
	    if(spec_continue() == DSPCMD_UPDATE)
		{
	      sp_update(hDlg,current_step_bits);
		  current_step_bits -= sp_bits_per_step;
		}
	    else
		{
	      mprintf("Comm Error during spec scan!");
		  return;
		}
	  }
	  else
	  {
	    mprintf("Comm Error during spec scan!");
		return;
	  }

	}

	// VI.c Get data from DSP and average it in.
	if(data_per_pass && finish_needed)
	{
      sp_status(hDlg,(*pass_count),"transferring data");
	  spec_finish(&(one_pass_buffer[one_pass_index]),0);
      sp_status(hDlg,(*pass_count),"");
	}

    // stuff sp_data and update sp_data fields
	buffer_index = 0;
	if(take_data_fwd)
	{
      for(i = 0;i < spec_steps_fwd;i++)
	  {
        if(sp_measure_i)
		{
		  if(sp_avg_both)
		  {
            sp_data[0]->yf[i] = (float)(*pass_count * 2) / (*pass_count * 2 + 1) * sp_data[0]->yf[i]
			                + one_pass_buffer[buffer_index] / (*pass_count * 2 + 1);
		  }
		  else
		  {
            sp_data[0]->yf[i] = (float)(*pass_count) / (*pass_count + 1) * sp_data[0]->yf[i]
			                + one_pass_buffer[buffer_index] / (*pass_count + 1);
		  }
		  buffer_index++;
		}
        if(sp_measure_z)
		{
		  if(sp_avg_both)
		  {
            sp_data[1]->yf[i] = (float)(*pass_count * 2) / (*pass_count * 2 + 1) * sp_data[1]->yf[i]
			                + one_pass_buffer[buffer_index] / (*pass_count * 2 + 1);
		  }
		  else
		  {
            sp_data[1]->yf[i] = (float)(*pass_count) / (*pass_count + 1) * sp_data[1]->yf[i]
			                + one_pass_buffer[buffer_index] / (*pass_count + 1);
		  }
		  buffer_index++;
		}
        if(sp_measure_2)
		{
		  if(sp_avg_both)
		  {
            sp_data[2]->yf[i] = (float)(*pass_count * 2) / (*pass_count * 2 + 1) * sp_data[2]->yf[i]
			                + one_pass_buffer[buffer_index] / (*pass_count * 2 + 1);
		  }
		  else
		  {
            sp_data[2]->yf[i] = (float)(*pass_count) / (*pass_count + 1) * sp_data[2]->yf[i]
			                + one_pass_buffer[buffer_index] / (*pass_count + 1);
		  }
		  buffer_index++;
		}
        if(sp_measure_3)
		{
		  if(sp_avg_both)
		  {
            sp_data[3]->yf[i] = (float)(*pass_count * 2) / (*pass_count * 2 + 1) * sp_data[3]->yf[i]
			                + one_pass_buffer[buffer_index] / (*pass_count * 2 + 1);
		  }
		  else
		  {
            sp_data[3]->yf[i] = (float)(*pass_count) / (*pass_count + 1) * sp_data[3]->yf[i]
			                + one_pass_buffer[buffer_index] / (*pass_count + 1);
		  }
		  buffer_index++;
		}
	  }
	}
	if(take_data_bkd)
	{
      for(i = 0;i < spec_steps_bkd;i++)
	  {
        if(sp_measure_i)
		{
		  if(sp_avg_both)
		  {
            sp_data[0]->yf[spec_steps_bkd - i - 1] = (float)(*pass_count * 2) / (*pass_count * 2 + 1) * sp_data[0]->yf[spec_steps_bkd - i - 1]
			                + one_pass_buffer[buffer_index] / (*pass_count * 2 + 1);
		  } 
		  else
		  {
            sp_data[4]->yf[spec_steps_bkd - i - 1] = (float)(*pass_count) / (*pass_count + 1) * sp_data[4]->yf[spec_steps_bkd - i - 1]
			                + one_pass_buffer[buffer_index] / (*pass_count + 1);
		  }
		  buffer_index++;
		}
        if(sp_measure_z)
		{
		  if(sp_avg_both)
		  {
            sp_data[1]->yf[spec_steps_bkd - i - 1] = (float)(*pass_count * 2) / (*pass_count * 2 + 1) * sp_data[1]->yf[spec_steps_bkd - i - 1]
			                + one_pass_buffer[buffer_index] / (*pass_count * 2 + 1);
		  }
		  else
		  {
            sp_data[5]->yf[spec_steps_bkd - i - 1] = (float)(*pass_count) / (*pass_count + 1) * sp_data[5]->yf[spec_steps_bkd - i - 1]
			                + one_pass_buffer[buffer_index] / (*pass_count + 1);
		  }
		  buffer_index++;
		}
        if(sp_measure_2)
		{
		  if(sp_avg_both)
		  {
            sp_data[2]->yf[spec_steps_bkd - i - 1] = (float)(*pass_count * 2) / (*pass_count * 2 + 1) * sp_data[2]->yf[spec_steps_bkd - i - 1]
			                + one_pass_buffer[buffer_index] / (*pass_count * 2 + 1);
		  }
		  else
		  {
            sp_data[6]->yf[spec_steps_bkd - i - 1] = (float)(*pass_count) / (*pass_count + 1) * sp_data[6]->yf[spec_steps_bkd - i - 1]
			                + one_pass_buffer[buffer_index] / (*pass_count + 1);
		  }
		  buffer_index++;
		}
        if(sp_measure_3)
		{
		  if(sp_avg_both)
		  {
            sp_data[3]->yf[spec_steps_bkd - i - 1] = (float)(*pass_count * 2) / (*pass_count * 2 + 1) * sp_data[3]->yf[spec_steps_bkd - i - 1]
			                + one_pass_buffer[buffer_index] / (*pass_count * 2 + 1);
		  }
		  else
		  {
            sp_data[7]->yf[spec_steps_bkd - i - 1] = (float)(*pass_count) / (*pass_count + 1) * sp_data[7]->yf[spec_steps_bkd - i - 1]
			                + one_pass_buffer[buffer_index] / (*pass_count + 1);
		  }
		  buffer_index++;
		}
	  }
    }

	// VI.c Validate data. Update pass count.
    if(sp_forward || sp_avg_both)
	{
      if(sp_measure_i) 
	  {
        sp_data[0]->valid = 1;
        sp_data[0]->num_passes = (*pass_count) + 1;
        sp_data[0]->min_x = (float)sp_scan_min;
        sp_data[0]->max_x = (float)sp_scan_max;
	  }
      if(sp_measure_z) 
	  {
        sp_data[1]->valid = 1;
        sp_data[1]->num_passes = (*pass_count) + 1;
        sp_data[1]->min_x = (float)sp_scan_min;
        sp_data[1]->max_x = (float)sp_scan_max;
	  }
      if(sp_measure_2) 
	  {
        sp_data[2]->valid = 1;
        sp_data[2]->num_passes = (*pass_count) + 1;
        sp_data[2]->min_x = (float)sp_scan_min;
        sp_data[2]->max_x = (float)sp_scan_max;
	  }
      if(sp_measure_3) 
	  {
        sp_data[3]->valid = 1;
        sp_data[3]->num_passes = (*pass_count) + 1;
        sp_data[3]->min_x = (float)sp_scan_min;
        sp_data[3]->max_x = (float)sp_scan_max;
	  }
	}
    if(sp_backward && !sp_avg_both)
	{
      if(sp_measure_i) 
	  {
        sp_data[4]->valid = 1;
        sp_data[4]->num_passes = (*pass_count) + 1;
	  }
      if(sp_measure_z) 
	  {
        sp_data[5]->valid = 1;
        sp_data[5]->num_passes = (*pass_count) + 1;
	  }
      if(sp_measure_2) 
	  {
        sp_data[6]->valid = 1;
        sp_data[6]->num_passes = (*pass_count) + 1;
	  }
      if(sp_measure_3) 
	  {
        sp_data[7]->valid = 1;
       sp_data[7]->num_passes = (*pass_count) + 1;
	  }
	}

	// VI.c Redraw.
	if(bit16 && (sp_output_ch == 10 || sp_output_ch == 11))
	{
	  sp_p1_vbits = min(max(sp_p1_vbits,SP_MIN_MIN),SP_MAX_MAX * 16);
	  sp_p2_vbits = min(max(sp_p2_vbits,SP_MIN_MIN),SP_MAX_MAX * 16); 
	}
	else
	{
	  sp_p1_vbits = min(max(sp_p1_vbits,SP_MIN_MIN),SP_MAX_MAX);
	  sp_p2_vbits = min(max(sp_p2_vbits,SP_MIN_MIN),SP_MAX_MAX); 
	}
    repaint_graphics(hDlg);
	}//End of else Mod. 5 Shaowei
    }//End of passstop
    CheckMsgQ();	// Temp, X. Cao
    //sur V. Final Delay Mod. 4 Shaowei 
        if(sp_final_delay_on[sp_current_default] && !stop)
	{sp_status(hDlg,(*pass_count),"Waiting at final...");
		Sleep(sp_final_delay[sp_current_default]);}
    CheckMsgQ();
	// VI.d Tracking between passes, if applicable
    if(sp_track_every && sp_scanning) 
    {
      if(!(((*pass_count) + 1) % sp_track_every) && (*pass_count) != sp_num_passes - 1)
      {
		// suspend dithering
        dio_dither_serial(0,0);
        dio_dither_serial(1,0);

		// ramp to initial value for tracking
        if(sp_scan_min != sp_output_initial && !sp_track_at_min)
        {
		  if((sp_output_ch == sample_bias_ch) && sp_use_z_offset_bias && sp_z_offset)
		  {
			sp_status(hDlg,(*pass_count),"ramping to initial...");	
					// added by X. Cao, Mod. #8, 2004-06-01. More status
            sp_output_now = ramp_serial(sp_output_ch,sp_output_now,
											sp_z_offset_bias,sp_ramp_delay,0);
		  }
          else
		  {
			sp_status(hDlg,(*pass_count),"ramping to initial...");	
					// added by X. Cao, Mod. #8, 2004-06-01. More status
            sp_output_now = ramp_serial(sp_output_ch,sp_output_now,
											sp_output_initial,sp_ramp_delay,0);
		  }
        }

		// "undo" delta z-offset
        if(sp_z_offset)
        {
          sp_status(hDlg,(*pass_count),"z-offset...");
          ramp_serial(z_offset_ch,dac_data[z_offset_ch],
				      dac_data[z_offset_ch] - sp_z_offset,DEFAULT_Z_OFFSET_TIME,0);
          serial_soft_delay(1000000); // creep
          dac_data[z_offset_ch] -= sp_z_offset;
          scan_z = dac_data[z_offset_ch];
	      if(sp_use_z_offset_bias)
		  {
			if(sp_output_ch == sample_bias_ch)
			{
                ramp_serial(sample_bias_ch,sp_output_now,
		          sp_output_initial,sp_ramp_delay,0);
				sp_output_now = sp_output_initial;

			}
			else
			{
              ramp_serial(sample_bias_ch,sp_z_offset_bias,
		        dac_data[sample_bias_ch],sp_ramp_delay,0);
			}
		  }
        }

		// feedback on
        dio_feedback_serial(1);
		
		CheckMsgQ();	// added by X. Cao, Mod. #8, 2004-06-01. More stops

        // begin of X. Cao, Mod. #8, 2004-06-01. More stops
		// the following if(sp_scanning) test, enclosing all related to tracking,
		// is added to enable more stops during spectroscopy
		if(sp_scanning)
				// end of X. Cao, Mod. #8, 2004-06-01. More stops
		{
		  sp_status(hDlg,(*pass_count),"tracking...");
		  
		  // "undo" x, y offsets
          if(sp_track_offset_x) 
		  {
            i = (int)dac_data[x_ch] - sp_track_offset_x;
            if(i < 0) i = 0;
            if(i > MAX) i = MAX;
            dac_data[x_ch] = ramp_serial(x_ch,dac_data[x_ch],i,SD->step_delay,0);
		  }
          if(sp_track_offset_y) 
		  {
            i = (int)dac_data[y_ch] - sp_track_offset_y;
            if(i < 0) i = 0;
            if(i > MAX) i = MAX;
            dac_data[y_ch] = ramp_serial(y_ch,dac_data[y_ch],i,SD->step_delay,0);
		  }

          // Do the tracking
	      track_setup(sp_track_avg_data_pts,sp_track_step_delay,
		            sp_track_step_delay,sp_track_plane_a,sp_track_plane_b);
	      cur_x = dac_data[x_ch];
          cur_y = dac_data[y_ch];
	      if(sp_tracking_mode == SP_TRACK_MAX) tracking_mode = TRACK_MAX;
	      else tracking_mode = TRACK_MIN;
	      track_start(cur_x,cur_y,tracking_mode);
          for(i = 0;i < sp_track_iterations && sp_scanning && sp_tracking;i++)
		  {
	        cur_x = dac_data[x_ch];
		    cur_y = dac_data[y_ch];

            if(cur_x > sp_track_limit_x_max ||
		       cur_x < sp_track_limit_x_min ||
		       cur_y > sp_track_limit_y_max||
		       cur_y < sp_track_limit_y_min)
		       sp_tracking = 0;
	 	    else track_again();

		  }
	      track_terminate();
		  sp_landing_x = dac_data[x_ch];
		  sp_landing_y = dac_data[y_ch];

		  // restore x,y offsets
          if(sp_track_offset_x) 
		  {
		    if(sp_offset_hold && !sp_feedback)
			  dio_feedback_serial(sp_feedback == 1);
            i = (int)dac_data[x_ch] + sp_track_offset_x;
            if(i < 0) i = 0;
            if(i > MAX) i = MAX;
	        dac_data[x_ch] = ramp_serial(x_ch,dac_data[x_ch],i,SD->step_delay,0);
		  }
          if(sp_track_offset_y)
		  {
		    if(sp_offset_hold && !sp_feedback)
			  dio_feedback_serial(sp_feedback == 1);
            i = (int)dac_data[y_ch] + sp_track_offset_y;
            if(i < 0) i = 0;
            if(i > MAX) i = MAX;
	        dac_data[y_ch] = ramp_serial(y_ch,dac_data[y_ch],i,SD->step_delay,0);
		  }
		}		// end of tracking business.

		CheckMsgQ();	// added by X. Cao, Mod. #8, 2004-06-01. More stops

		// restore feedback
		// begin of X. Cao, Mod. #8, 2004-06-01. More stops
		// the following if(sp_scanning) test, enclosing all related to restoring
		// feedback, is added to enable more stops during spectroscopy
		if(sp_scanning)
				// end of X. Cao, Mod. #8, 2004-06-01. More stops
		{
          if(sp_feedback != 1) 
		  {
            if(sp_feedback_wait > SP_UPDATE_TIME) sp_update(hDlg,sp_output_initial);
            serial_soft_delay(sp_feedback_wait);
		  }
          dio_feedback_serial(sp_feedback == 1);
          if(sp_feedback == 1) 
		  {
            if(sp_feedback_wait > SP_UPDATE_TIME) sp_update(hDlg,sp_output_initial);
            serial_soft_delay(sp_feedback_wait);
		  }
		}

		// restore z offset
		// begin of X. Cao, Mod. #8, 2004-06-01. More stops
        // if(sp_z_offset)
		// This if-test is changed to enable more stops during spectroscopy
		if(sp_z_offset && sp_scanning)	
				// end of X. Cao, Mod. #8, 2004-06-01. More stops
        {
          sp_status(hDlg,(*pass_count),"z-offset...");
	      if(sp_use_z_offset_bias)
		  {
	        if(sp_output_ch == sample_bias_ch)
			{
              ramp_serial(sample_bias_ch,sp_output_now,
		                  sp_z_offset_bias,sp_ramp_delay,0);
              sp_output_now = sp_z_offset_bias;
			}
	        else
			{
              ramp_serial(sample_bias_ch,dac_data[sample_bias_ch],
		                  sp_z_offset_bias,sp_ramp_delay,0);
			} 
		  } 
          ramp_serial(z_offset_ch,dac_data[z_offset_ch],dac_data[z_offset_ch] + sp_z_offset,
				      DEFAULT_Z_OFFSET_TIME,0);
          serial_soft_delay(1000000);  // creep
          dac_data[z_offset_ch] += sp_z_offset;
          scan_z = dac_data[z_offset_ch];
        }

		// make sure we're at beginning of ramp
        // begin of X. Cao, Mod. #8, 2004-06-01. More stops
		// if(sp_scan_min != sp_output_initial && !sp_feedback_at_min)
		// This if-test is changed to enable more stops during spectroscopy
		if(sp_scan_min != sp_output_initial && !sp_feedback_at_min && sp_scanning)
				// end of X. Cao, Mod. #8, 2004-06-01. More stops
        {
		  sp_status(hDlg,(*pass_count),"ramping to start..."); 
				// The above line added by X. Cao, Mod. #8, 2004-06-01. More status.
          sp_output_now = ramp_serial(sp_output_ch,sp_output_now,sp_scan_min,sp_ramp_delay,0);
        }

        // restore dithering
		// begin of X. Cao, Mod. #8, 2004-06-01. More stops
		// The following if(sp_scanning) test, enclosing all related to restoring
		// dithering, is added to enable more stops during spectroscopy
		if(sp_scanning)
				// end of X. Cao, Mod. #8, 2004-06-01. More stops
		{
          dio_dither_serial(0,sp_dither0 == SP_DITHER0_ON);
          dio_dither_serial(1,sp_dither1 == SP_DITHER1_ON);
          if(sp_dither0 == SP_DITHER0_ON || sp_dither1 == SP_DITHER1_ON)
		  {
            if(sp_dither_wait > SP_UPDATE_TIME) sp_update(hDlg,sp_scan_min);
            serial_soft_delay(sp_dither_wait);
		  }
		}

      }
    }
		  
	// VI.e Feedback on between passes, if applicable
    if(sp_scanning && sp_feedback_every && sp_feedback == 2 && !sp_track_every) 
    {
      if(!(((*pass_count) + 1) % sp_feedback_every) && (*pass_count) != sp_num_passes - 1)
      {
		// suspend dithering
        dio_dither_serial(0,0);
        dio_dither_serial(1,0);

		// ramp to initial value for turning on feedback
        if(sp_scan_min != sp_output_initial && !sp_feedback_at_min)
        {
		  if((sp_output_ch == sample_bias_ch) && sp_use_z_offset_bias && sp_z_offset)
		  {
			sp_status(hDlg,(*pass_count),"ramping to initial...");	
					// added by X. Cao, Mod. #8, 2004-06-01. More status
            sp_output_now = ramp_serial(sp_output_ch,sp_output_now,
											sp_z_offset_bias,sp_ramp_delay,0);
		  }
          else
		  {
			sp_status(hDlg,(*pass_count),"ramping to initial...");    
					// added by X. Cao, Mod. #8, 2004-06-01. More status
            sp_output_now = ramp_serial(sp_output_ch,sp_output_now,
											sp_output_initial,sp_ramp_delay,0);
		  }
        }

        // "undo" delta z offset
		if(sp_z_offset)
        {
          sp_status(hDlg,(*pass_count),"z-offset...");
 		  ramp_serial(z_offset_ch,dac_data[z_offset_ch],dac_data[z_offset_ch] - sp_z_offset,
				      DEFAULT_Z_OFFSET_TIME,0);
          serial_soft_delay(1000000);            
          dac_data[z_offset_ch] -= sp_z_offset;
          scan_z = dac_data[z_offset_ch];
	      if(sp_use_z_offset_bias)
		  {
			if(sp_output_ch == sample_bias_ch)
			{
                ramp_serial(sample_bias_ch,sp_output_now,
		          sp_output_initial,sp_ramp_delay,0);
				sp_output_now = sp_output_initial;

			}
			else
			{
              ramp_serial(sample_bias_ch,sp_z_offset_bias,
		        dac_data[sample_bias_ch],sp_ramp_delay,0);
			}
		  }
		}

		// toggle feedback
        dio_feedback_serial(1);
        serial_soft_delay(sp_feedback_wait);
        sp_status(hDlg,(*pass_count),"feedback on...");

        if(sp_fb_delay_on[sp_current_default]) // Mod. 3 Shaowei, Feedback Delay between passes
		{
			sp_status(hDlg,(*pass_count),"Waiting when feedback is on...");
			CheckMsgQ();	
			Sleep(sp_fb_delay[sp_current_default]);
		}
		sp_update(hDlg,sp_output_initial);
        dio_feedback_serial(sp_feedback==1);

		CheckMsgQ();	// added by X. Cao, Mod. #8, 2004-06-01. More stops

        // restore z offset
		// begin of X. Cao, Mod. #8, 2004-06-01. More stops
        // if(sp_z_offset)
		// This if-test is changed to enable more stops during spectroscopy
		if(sp_z_offset && sp_scanning)	
				// end of X. Cao, Mod. #8, 2004-06-01. More stops
        {
          sp_status(hDlg,(*pass_count),"z-offset...");
  	      if(sp_use_z_offset_bias)
		  {
	        if(sp_output_ch == sample_bias_ch)
			{
              ramp_serial(sample_bias_ch,sp_output_now,
		                  sp_z_offset_bias,sp_ramp_delay,0);
              sp_output_now = sp_z_offset_bias;
			}
	        else
			{
              ramp_serial(sample_bias_ch,dac_data[sample_bias_ch],
		                  sp_z_offset_bias,sp_ramp_delay,0);
			}
		  }
		  ramp_serial(z_offset_ch,dac_data[z_offset_ch],dac_data[z_offset_ch] + sp_z_offset,
				      DEFAULT_Z_OFFSET_TIME,0);
          serial_soft_delay(1000000);            
          dac_data[z_offset_ch] += sp_z_offset;
          scan_z = dac_data[z_offset_ch];
        }

        // begin of X. Cao, Mod. #8, 2004-06-01. More stops
		// if(sp_scan_min != sp_output_initial && !sp_feedback_at_min)
		// This if-test is changed to enable more stops during spectroscopy
		if(sp_scan_min != sp_output_initial && !sp_feedback_at_min && sp_scanning)
				// end of X. Cao, Mod. #8, 2004-06-01. More stops
        {
		  sp_status(hDlg,(*pass_count),"ramping to start..."); 
				// The above line added by X. Cao, Mod. 8, 2004-06-01. More status.
          sp_output_now = ramp_serial(sp_output_ch,sp_output_now,sp_scan_min,sp_ramp_delay,0);
        }
		// restore dithering
		// begin of X. Cao, Mod. #8, 2004-06-01, More stops
		// The following if(sp_scanning) test, enclosing all related to restoring
		// dithering, is added to enable more stops during spectroscopy
		if(sp_scanning)
				// end of X. Cao, Mod. #8, 2004-06-01 More stops
		{
          dio_dither_serial(0,sp_dither0 == SP_DITHER0_ON);
          dio_dither_serial(1,sp_dither1 == SP_DITHER1_ON);
          if(sp_dither0 == SP_DITHER0_ON || sp_dither1 == SP_DITHER1_ON)
		  {
            serial_soft_delay(sp_dither_wait);
            if(sp_dither_wait > SP_UPDATE_TIME) sp_update(hDlg,sp_scan_min);
		  }
        }
	  }
	}

    repaint_track_displacement(hDlg);
    CheckMsgQ();	// Temp, X. Cao
		
  }  // end of for loop for passes

  // resume pre-scanning dithering status
  dio_dither_serial(0,dither0_before);
  dio_dither_serial(1,dither1_before);

  // back to initial output voltage
  sp_status(hDlg,*pass_count,"ramping to initial...");
  if((sp_output_ch == sample_bias_ch) && sp_use_z_offset_bias && sp_z_offset)
    sp_output_now = ramp_serial(sp_output_ch,sp_output_now,sp_z_offset_bias,sp_ramp_delay,0);
  else
    sp_output_now = ramp_serial(sp_output_ch,sp_output_now,sp_output_initial,sp_ramp_delay,0);

  if(sp_z_offset) /* preventing current overload */
  {
/*    ramp_serial(z_offset_ch,dac_data[z_offset_ch],dac_data[z_offset_ch] - sp_z_offset,
		        DEFAULT_Z_OFFSET_TIME,0);
    dac_data[z_offset_ch] -= sp_z_offset;
    scan_z = dac_data[z_offset_ch];
*/
    ramp_serial(z_offset_ch,dac_data[z_offset_ch],dac_data[z_offset_ch] - sp_z_offset,
				      DEFAULT_Z_OFFSET_TIME,0);
    serial_soft_delay(1000000);            
    dac_data[z_offset_ch] -= sp_z_offset;
    scan_z = dac_data[z_offset_ch];
    if(sp_use_z_offset_bias)
	{
	  if(sp_output_ch == sample_bias_ch)
	  {
        ramp_serial(sample_bias_ch,sp_output_now,
		          sp_output_initial,sp_ramp_delay,0);
		sp_output_now = sp_output_initial;

	  } 
	  else
	  {
        ramp_serial(sample_bias_ch,sp_z_offset_bias,
		        dac_data[sample_bias_ch],sp_ramp_delay,0);
	  }
	} 
  }

  // feedback to entry state
  dio_feedback_serial(entry_feedback);

  // auto-
  if(sp_z_offset) 
  {
    serial_soft_delay(100000);
    auto_serial(AUTOMINUS);
  }
      
  sp_update(hDlg,sp_output_initial);

  free(one_pass_buffer);
}		

static void make_spec_readseq(datadef *spec_settings)
{
  // The DSP assembly code uses the read sequence functions to perform
  // the various data reads at each point in the spec scan.
  // This function generates the read sequence information for a spec
  // scan and stuffs it into spec_settings. At present, the read sequence
  // generated by this function is NOT saved when the spec scan is saved
  // as a file. It must be called after each spc file Load or after any
  // relevant change in the spec settings.

  int index = 0;

  // start with a sequence for updating via a serial letter
  spec_settings->sequence[index].type = SERIAL_OUT_TYPE;
  spec_settings->sequence[index].serial_out_char = DSPCMD_UPDATE;
  strcpy(spec_settings->sequence[index].step_type_string,"Serial Out");
  index++;

  if(sp_measure_i)
  {
    // add a sequence to read i
	spec_settings->sequence[index].type = READ_TYPE;
	spec_settings->sequence[index].in_ch = 0;
	spec_settings->sequence[index].num_reads = sp_num_samples;
    strcpy(spec_settings->sequence[index].step_type_string,"Read");
	index++;
  }
  if(sp_measure_z)
  {
    // add a sequence to read z
	spec_settings->sequence[index].type = READ_TYPE;
	spec_settings->sequence[index].in_ch = 1;
	spec_settings->sequence[index].num_reads = sp_num_samples;
    strcpy(spec_settings->sequence[index].step_type_string,"Read");
	index++;
  }
  if(sp_measure_2)
  {
    // add a sequence to read ch 2
	spec_settings->sequence[index].type = READ_TYPE;
	spec_settings->sequence[index].in_ch = 2;
	spec_settings->sequence[index].num_reads = sp_num_samples;
    strcpy(spec_settings->sequence[index].step_type_string,"Read");
	index++;
  }
  if(sp_measure_3)
  {
    // add a sequence to read ch 3
	spec_settings->sequence[index].type = READ_TYPE;
	spec_settings->sequence[index].in_ch = 3;
	spec_settings->sequence[index].num_reads = sp_num_samples;
    strcpy(spec_settings->sequence[index].step_type_string,"Read");
	index++;
  }

  spec_settings->total_steps = index;
}

static int init_spec(HWND hDlg,int ch,int size,int type)
{
  int this_ch = ch;
  int i;
  
  if(sp_forward || sp_avg_both)
  {
    if(sp_data_valid(this_ch))
    {
      if(!sp_data[this_ch]->saved)
      {
        sprintf(string,"Channel %d not saved!\nScan will overwrite data...",this_ch);
        if(MessageBox(hDlg,string, 
                      "Warning", MB_OKCANCEL | MB_ICONEXCLAMATION) != IDOK)
        {
          return 0;
        }
      }
    }
    free_data(&(sp_data[this_ch]));
    alloc_data(&(sp_data[this_ch]),DATATYPE_GEN2D,size,GEN2D_NONE,
               GEN2D_FLOAT,0);
    sp_data[this_ch]->type2d = type;
    set_data(sp_data[this_ch]);
    for(i = 0;i < size;i++) sp_data[this_ch]->yf[i] = 0;
  }
            
  if(sp_backward && !sp_avg_both) 
  {
    this_ch += 4;
    if(sp_data_valid(this_ch))
    {
      if(!sp_data[this_ch]->saved)
      {
        sprintf(string,"Channel %d not saved!\nScan will overwrite data...",this_ch);
        if(MessageBox(hDlg,string, 
                      "Warning", MB_OKCANCEL | MB_ICONEXCLAMATION) != IDOK)
        {
          return 0;
        }
      }
    }
    free_data(&(sp_data[this_ch]));
    alloc_data(&(sp_data[this_ch]),DATATYPE_GEN2D,size,GEN2D_NONE,
               GEN2D_FLOAT,0);
    sp_data[this_ch]->type2d = type;
    set_data(sp_data[this_ch]);
    sp_data[this_ch]->scan_dir |= SP_BACKWARD_DATA;
    for(i = 0;i < size;i++) sp_data[this_ch]->yf[i] = 0;
  }
  return 1;
}

static int init_sp_input_data( HWND hDlg, int *strobe)
{
  // allocates memory and initializes data for up to 8 sets of spec data
  // 8 sets = 4 channels * 2 directions

  int i;

  i = (sp_scan_max - sp_scan_min) / sp_bits_per_step + 1;

  // allocate memory for the data
  if(sp_measure_3) 
  {
    if(!init_spec(hDlg,3,i,TYPE2D_SPEC_3)) return 0;
    *strobe |= 1<<7;
    sp_current = 3;
  }
  if(sp_measure_2) 
  {
    if (!init_spec(hDlg,2,i,TYPE2D_SPEC_2)) return 0;
    *strobe |= 1<<5;
    sp_current = 2;
  }
  if(sp_measure_z) 
  {
    if(!init_spec(hDlg,1,i,TYPE2D_SPEC_Z)) return 0;
    *strobe |= 1<<3;
    sp_current = 1;
  }
  if(sp_measure_i) 
  {
    if(!init_spec(hDlg,0,i,TYPE2D_SPEC_I)) return 0;
    *strobe = 2;
    sp_current = 0;
  }

  return 1;
}

static void set_data(datadef *this_data)
{
  // stuffs all of the current settings into this_data 

  time_t scan_time;
  int i;
 
  scan_time = time(NULL);
  strcpy(string,ctime(&scan_time));
  string[strlen(string) - 1] = '\0';
  strcpy(this_data->comment.ptr,string);
  this_data->comment.size = strlen(string);
  strcpy(this_data->filename,"Unsaved");
  strcpy(this_data->full_filename,"Unsaved");
  this_data->version = SP_DATA_VERSION;
  this_data->valid = 1;
  switch(sp_mode)
  {
    case SP_MODE_ABSOLUTE:
      this_data->start = sp_scan_min;
      this_data->end = sp_scan_max;
      break;
    case SP_MODE_RELATIVE:
      this_data->start = sp_rel_min;
      this_data->end = sp_rel_max;
      break;
  }
        
  this_data->bias = sample_bias;
  this_data->bias_range = sample_bias_range;
  this_data->i_setpoint = i_setpoint;
  this_data->i_set_range = i_set_range;
  this_data->amp_gain = tip_gain;
  memcpy(this_data->dosed_type.ptr,data->dosed_type.ptr,SAMPLETYPESIZE);
  memcpy(this_data->sample_type.ptr,data->sample_type.ptr,SAMPLETYPESIZE);
  this_data->sample_type.size = data->sample_type.size;
  this_data->dosed_type.size = data->dosed_type.size;
  this_data->dosed_langmuir = data->dosed_langmuir;
  this_data->temperature = data->temperature;
  this_data->v_range = get_range(sp_output_ch);
  this_data->delta_z_offset = sp_z_offset;
  this_data->speed = (float)sp_speed;
  this_data->dither_wait = sp_dither_wait;
  this_data->feedback_wait = sp_feedback_wait;
  this_data->move_time = sp_move_delay;
  this_data->ramp_time = sp_ramp_delay;
  this_data->measure_i = sp_measure_i;
  this_data->measure_z = sp_measure_z;
  this_data->measure_2 = sp_measure_2;
  this_data->measure_3 = sp_measure_3;
  this_data->track_avg_data_pts = sp_track_avg_data_pts;
  this_data->track_offset_x = sp_track_offset_x;
  this_data->track_offset_y = sp_track_offset_y;
  this_data->track_max_bits = sp_track_max_bits;
  this_data->track_sample_every = sp_track_sample_every;
  this_data->tracking_mode = sp_tracking_mode;
  this_data->track_auto_auto = sp_track_auto_auto;           
  this_data->track_iterations = sp_track_iterations;
  this_data->track_every = sp_track_every;
  this_data->z_offset_bias = sp_z_offset_bias;
  this_data->num_passes = sp_num_passes;
  this_data->scan_dir = (sp_forward << 2) + (sp_backward << 1) + (sp_avg_both);
  this_data->scan_num = sp_num_samples;
  this_data->crash_protection = sp_crash_protection;
  this_data->dither0 = sp_dither0;
  this_data->dither1 = sp_dither1;
  this_data->step = sp_bits_per_step;
  this_data->scan_feedback = sp_feedback;
  this_data->feedback_wait = sp_feedback_wait;
  this_data->feedback_every = sp_feedback_every;

  this_data->sp_mode = sp_mode;
  for(i = 0;i < 16;i++) this_data->pre_dac_data[i] = dac_data[i];
  this_data->x = sp_x;
  this_data->y = sp_y;
  strcpy(this_data->dep_filename,data->filename);
  this_data->track_step_delay = sp_track_step_delay;
  this_data->output_ch = sp_output_ch;
  this_data->min_x = (float)sp_scan_min;
  this_data->max_x = (float)sp_scan_max;
    
  this_data->lockin_sen1 = lockin_sen1;
  this_data->lockin_sen2 = lockin_sen2;
  this_data->lockin_osc = lockin_osc; 
  this_data->lockin_w = lockin_w;
  this_data->lockin_tc = lockin_tc;
  this_data->lockin_n1 = lockin_n1;
  this_data->lockin_pha1 = lockin_pha1;
  this_data->lockin_pha2 = lockin_pha2;
  this_data->lockin_in_use = lockin_in_use;

  this_data->track_at_min = sp_track_at_min;
  this_data->feedback_at_min = sp_feedback_at_min;

  this_data->track_plane_fit_a = sp_track_plane_a;
  this_data->track_plane_fit_b = sp_track_plane_b;

  this_data->offset_hold = sp_offset_hold;
  this_data->bit16 = bit16;

  //Shaowei Mod.13 
  /*
  this_data->spc_trigger_on = sp_trigger;
  this_data->spc_trigger_ch = sp_trigger_ch;
  this_data->spc_trigger_hi = sp_trigger_high;
  this_data->spc_trigger_lo = sp_trigger_low;
  */
  //End Shaowei
}

static void get_data(datadef *this_data)
{
  sp_out_range = this_data->v_range; 
  sp_z_offset = this_data->delta_z_offset;
  sp_speed = (int)(this_data->speed);
  sp_dither_wait = this_data->dither_wait;
  sp_feedback_wait = this_data->feedback_wait;
  sp_move_delay = this_data->move_time;
  sp_ramp_delay = this_data->ramp_time;
  switch(this_data->sp_mode)
  {
    case SP_MODE_ABSOLUTE:
      sp_scan_min = this_data->start;
      sp_scan_max = this_data->end;
      break;
    case SP_MODE_RELATIVE:
      sp_rel_min = this_data->start;
      sp_rel_max = this_data->end;
      break;
  }

  sp_measure_i = this_data->measure_i;
  sp_measure_z = this_data->measure_z;
  sp_measure_2 = this_data->measure_2;
  sp_measure_3 = this_data->measure_3;
  sp_track_offset_x = this_data->track_offset_x;
  sp_track_offset_y = this_data->track_offset_y;
  sp_track_max_bits = this_data->track_max_bits;
  sp_track_sample_every = this_data->track_sample_every;
  sp_tracking_mode = this_data->tracking_mode;
  sp_track_auto_auto = this_data->track_auto_auto;
  sp_track_iterations = this_data->track_iterations;
  sp_track_every = this_data->track_every;
  sp_z_offset_bias = this_data->z_offset_bias;
//  sp_num_passes = this_data->num_passes;
  sp_forward = (this_data->scan_dir & (1 << 2)) > 0;
  sp_backward = (this_data->scan_dir & (1 << 1)) > 0;
  sp_avg_both = (this_data->scan_dir & 1) > 0;
  sp_num_samples = this_data->scan_num;
  sp_num_passes = this_data->num_passes;
  sp_crash_protection = this_data->crash_protection;
  sp_dither0 = this_data->dither0;
  sp_dither1 = this_data->dither1;
  sp_bits_per_step = this_data->step;
  sp_feedback = this_data->scan_feedback;
  sp_feedback_wait = this_data->feedback_wait;
  sp_feedback_every = this_data->feedback_every;
    
  sp_mode = this_data->sp_mode;
  sp_x = this_data->x;
  sp_y = this_data->y;
  sp_track_step_delay = this_data->track_step_delay;
  sp_output_ch = this_data->output_ch;

  lockin_sen1 = this_data->lockin_sen1;
  lockin_sen2 = this_data->lockin_sen2;
  lockin_osc = this_data->lockin_osc; 
  lockin_w = this_data->lockin_w;
  lockin_tc = this_data->lockin_tc;
  lockin_n1 = this_data->lockin_n1;
  lockin_pha1 = this_data->lockin_pha1;
  lockin_pha2 = this_data->lockin_pha2;
  lockin_in_use = this_data->lockin_in_use;

  sp_track_at_min = this_data->track_at_min;
  sp_feedback_at_min = this_data->feedback_at_min;

  sp_track_plane_a = this_data->track_plane_fit_a;
  sp_track_plane_b = this_data->track_plane_fit_b;
    
  sp_offset_hold = this_data->offset_hold;
  
  sp_calc_scan();

//Shaowei Mod.13
  /*
  sp_trigger = this_data->spc_trigger_on;
  sp_trigger_ch = this_data->spc_trigger_ch;
  sp_trigger_high = this_data->spc_trigger_hi;
  sp_trigger_low = this_data->spc_trigger_lo;
  */
  // End Shaowei Mod.13
}

int sp_data_valid(int ch)
{
  int result = 0;
  if(sp_data[ch])
  { 
    result = sp_data[ch]->valid; 
  }
  return result;
}

static void sp_scan_enable(HWND hDlg,int status)
{
  EnableWindow(GetDlgItem(hDlg,SP_OPTIONS),status);
//  EnableWindow(GetDlgItem(hDlg,SP_VOLTS_SCROLL),status);
//  EnableWindow(GetDlgItem(hDlg,SP_INPUT_CH),status);
//  EnableWindow(GetDlgItem(hDlg,SP_OUTPUT_CH),status);
  EnableWindow(GetDlgItem(hDlg,SP_COMMENT),status);
//  EnableWindow(GetDlgItem(hDlg,SP_PRINT),status);
  EnableWindow(GetDlgItem(hDlg,SP_SAVE),status);
  EnableWindow(GetDlgItem(hDlg,SP_SAVE_ALL),status);	// part of Mod. #4, X. Cao
  EnableWindow(GetDlgItem(hDlg,SP_EXIT),status);
  EnableWindow(GetDlgItem(hDlg,SP_LOAD),status);
  EnableWindow(GetDlgItem(hDlg,SP_INFO),status);
  EnableWindow(GetDlgItem(hDlg,SP_CLEAR),status);
  EnableWindow(GetDlgItem(hDlg,SP_CLEAR_ALL),status);
  EnableWindow(GetDlgItem(hDlg,SPE_OFFSET_AUTO_BELOW),status);
}

static void enable_vib_map_params( HWND hDlg, int status)
{
  EnableWindow(GetDlgItem(hDlg,SPC_VM_POINT),status);
  EnableWindow(GetDlgItem(hDlg,SPC_VM_POINT_LABEL),status);
  EnableWindow(GetDlgItem(hDlg,SPC_VM_XOFF),status);
  EnableWindow(GetDlgItem(hDlg,SPC_VM_XOFF_LABEL),status);
  EnableWindow(GetDlgItem(hDlg,SPC_VM_YOFF),status);
  EnableWindow(GetDlgItem(hDlg,SPC_VM_YOFF_LABEL),status);
}

static void spec_set_title(HWND hDlg)
{
  if(sp_cur_selection == SP_UPPER_IMAGE)
  {
    if(sp_data_valid(sp_current))
	{
      sprintf(string,"Spectroscopy: %s",sp_data[sp_current]->filename);
      SetWindowText(hDlg,string);
	}
    else SetWindowText(hDlg,"Spectroscopy. No Data.");
  }
  else
  {
    if(second_data[current_second_data] != NULL)
	{
	  if(second_data[current_second_data]->valid)
	  {
        sprintf(string,"Spectroscopy: %s",second_data[current_second_data]->filename);
        SetWindowText(hDlg,string);
	  }
	  else
	  {
	    SetWindowText(hDlg,"Spectroscopy. No Data.");
	  }
	}
  }
}

static void sp_update(HWND hDlg,int value)
{
  unsigned int reference;

  switch(sp_mode)
  {
    case SP_MODE_ABSOLUTE:
	  if(bit16 && (sp_output_ch == 10 || sp_output_ch == 11))
	  {
        sprintf(string,"%0.5f V",dtov16(value));
	  }
	  else
	  {
        sprintf(string,"%0.4f V",dtov(value,get_range(sp_output_ch)));
	  }
      break;
    case SP_MODE_RELATIVE:
	  if(bit16 && (sp_output_ch == 10 || sp_output_ch == 11))
	  {
	    if(sp_output_ch == 10)
          reference = (dac_data[extra_ch] & 0xF00) << 4;
	    else //(sp_output_ch == 11)
          reference = (dac_data[extra_ch] & 0x0F0) << 4;
        reference += dac_data[sp_output_ch];
				
	    sprintf(string,"%0.4f V, %d delta bits",dtov16(value),value - reference);
	  }
	  else
	  {
	    sprintf(string,"%0.4f V, %d delta bits",dtov(value,get_range(sp_output_ch)),
              value - dac_data[sp_output_ch]);
	  }
      break;
  }
  SetDlgItemText(hDlg,SP_BIAS,string);
}

static void sp_status(HWND hDlg,int pass,char *status)
{
  char string[500];
  int pass_display;

  if(pass == sp_num_passes)
  {
	pass_display = pass;
  }
  else
  {
	pass_display = pass + 1;
  }

  sprintf(string,"Pass %d of %d %s",pass_display,sp_num_passes,status);
  SetDlgItemText(hDlg,SP_STATUS,string);
}

static void repaint_data(HWND hDlg)
{
  int j;
  HDC spec_hdc;
  HPEN *hpen;
    
  spec_hdc = GetDC(hDlg);

  // paint datasets in upper image
  for(j = 0;j < SP_DATASETS;j++)
  {
    if(sp_data_valid(j))
	{
      switch(j % 4)
	  {
        case 0:
          hpen = &hpen_red;
          break;
        case 1:
          hpen = &hpen_green;
          break;
        case 2:
          hpen = &hpen_blue;
          break;
        case 3:
          hpen = &hpen_cyan;
          break;
	  } 
      repaint_spec_graphics(spec_hdc,*hpen,sp_data[j],
            sp_screen_min,sp_screen_max,
            SP_X_MIN,SP_Y_MIN,SP_X_MAX,SP_Y_MAX,(int)sp_vert_max,(int)sp_vert_min);
	}
  }
	
  // paint datasets in lower image
  for(j = 0;j < SP_DATASETS;j++)
  {
    if(second_data[j] != NULL)
	{
      if(second_data[j]->valid)
	  {
        switch(j % 4)
		{
          case 0:
            hpen = &hpen_red;
            break;
          case 1:
            hpen = &hpen_green;
            break;
          case 2:
            hpen = &hpen_blue;
            break;
          case 3:
            hpen = &hpen_cyan;
            break;
		}
        repaint_spec_graphics(spec_hdc,*hpen,second_data[j],
            sp_screen_min_2,sp_screen_max_2,
            SP_X_MIN,SP_Y_MIN_2,SP_X_MAX,SP_Y_MAX_2,(int)sp_vert_max_2,(int)sp_vert_min_2);
	  } 
	}
  }
  ReleaseDC(hDlg,spec_hdc);
}

static void repaint_p1(HWND hDlg)
{
  HDC spec_hdc;
  HPEN hpen_old;
  int xs;
  int range = 2;
    
  // update the scroll and text field
  if(sp_cur_selection == SP_UPPER_IMAGE)
  {
    if(!out_smart)
    {
      out_smart = 1;
	  if(sp_data[sp_current])
      {
		if(sp_data[sp_current]->bit16 &&
		  (sp_data[sp_current]->output_ch == 10 || sp_data[sp_current]->output_ch == 11))
		{
          SetScrollPos(GetDlgItem(hDlg,SP_P1_SCROLL),SB_CTL,
			       DTOI(sp_p1_vbits,SP_MIN_MIN,SP_MAX_MAX * 16),TRUE);
          sprintf(string,"%0.4f",dtov16(sp_p1_vbits));
		}
	    else
		{ 
          SetScrollPos(GetDlgItem(hDlg,SP_P1_SCROLL),SB_CTL,
			       DTOI(sp_p1_vbits,SP_MIN_MIN,SP_MAX_MAX),TRUE);
          sprintf(string,"%0.3f",dtov(sp_p1_vbits,get_range(sp_data[sp_current]->output_ch)));
		} 
	  }
	  else
      {
        SetScrollPos(GetDlgItem(hDlg,SP_P1_SCROLL),SB_CTL,
			       DTOI(sp_p1_vbits,SP_MIN_MIN,SP_MAX_MAX),TRUE);
        sprintf(string,"%0.3f",dtov(sp_p1_vbits,get_range(sp_output_ch)));
	  }
      SetDlgItemText(hDlg,SP_P1_VOLTS,string);
      out_smart = 0;
    }
    if(sp_data_valid(sp_current))
    {
      if(sp_p1_vbits >= sp_data[sp_current]->min_x && 
         sp_p1_vbits <= sp_data[sp_current]->max_x)
      {
        sprintf(string,"%0.3lf",in_dtov(sp_data[sp_current]->yf[(int)(sp_p1_vbits -
            sp_data[sp_current]->min_x) / sp_data[sp_current]->step]));
        SetDlgItemText(hDlg,SP_P1_CURR,string);
      }
      else SetDlgItemText(hDlg,SP_P1_CURR,"N/A");
	}
  }
  else
  {
    if(!out_smart)
    {
      out_smart = 1;
	  if(second_data[current_second_data])
	  {
	    if(second_data[current_second_data]->bit16 &&
	      (second_data[current_second_data]->output_ch == 10 || second_data[current_second_data]->output_ch == 11))
		{
          SetScrollPos(GetDlgItem(hDlg,SP_P1_SCROLL),SB_CTL,
			       DTOI(sp_p1_vbits_2,SP_MIN_MIN,SP_MAX_MAX * 16),TRUE);
          sprintf(string,"%0.4f",dtov16(sp_p1_vbits_2));
		}
	    else
		{
          SetScrollPos(GetDlgItem(hDlg,SP_P1_SCROLL),SB_CTL,
			       DTOI(sp_p1_vbits_2,SP_MIN_MIN,SP_MAX_MAX),TRUE);
	      if(second_data[current_second_data] != NULL)
		  {
	        if(second_data[current_second_data]->valid)
			{
	          range = second_data[current_second_data]->v_range;
			}
	        else range = 2;
		  }
          sprintf(string,"%0.3f",dtov(sp_p1_vbits_2,range));
		}
	  }
	  else
	  {
        SetScrollPos(GetDlgItem(hDlg,SP_P1_SCROLL),SB_CTL,
			       DTOI(sp_p1_vbits_2,SP_MIN_MIN,SP_MAX_MAX),TRUE);
	    if(second_data[current_second_data] != NULL)
		{
	      if(second_data[current_second_data]->valid)
		  {
	        range = second_data[current_second_data]->v_range;
		  }
	      else range = 2;
		}
        sprintf(string,"%0.3f",dtov(sp_p1_vbits_2,range));
	  }
      SetDlgItemText(hDlg,SP_P1_VOLTS,string);
      out_smart = 0;
	}
    if(second_data[current_second_data] != NULL)
    {
      if(second_data[current_second_data]->valid)
	  {
        if(sp_p1_vbits_2 >= second_data[current_second_data]->min_x && 
           sp_p1_vbits_2 <= second_data[current_second_data]->max_x)
        {
          sprintf(string,"%0.3lf",in_dtov(second_data[current_second_data]->yf[(int)(sp_p1_vbits_2 -
            				second_data[current_second_data]->min_x) / second_data[current_second_data]->step]));
          SetDlgItemText(hDlg,SP_P1_CURR,string);
        }
        else SetDlgItemText(hDlg,SP_P1_CURR,"N/A");
      }
    }
  }

  // draw the blue p1 line in the upper image, if appropriate
  if(sp_p1_vbits >= sp_screen_min && sp_p1_vbits <= sp_screen_max)
  {
    spec_hdc = GetDC(hDlg);
    hpen_old = SelectObject(spec_hdc,hpen_blue);

    xs = (sp_p1_vbits - sp_screen_min) * (SP_X_MAX - SP_X_MIN + 1) /
         (sp_screen_max - sp_screen_min + 1) + SP_X_MIN;
    xs = max(min(xs,SP_X_MAX),SP_X_MIN);
//    MoveTo(spec_hdc,xs,SP_Y_MIN);
    MoveToEx(spec_hdc,xs,SP_Y_MIN,NULL);
    LineTo(spec_hdc,xs,SP_Y_MAX);
        
    SelectObject(spec_hdc,hpen_old);
    ReleaseDC(hDlg,spec_hdc);
  }

  // draw the blue p1 line in the lower image, if appropriate
  if(sp_p1_vbits_2 >= sp_screen_min_2 && sp_p1_vbits_2 <= sp_screen_max_2)
  {
    spec_hdc = GetDC(hDlg);
    hpen_old = SelectObject(spec_hdc,hpen_blue);

    xs = (sp_p1_vbits_2 - sp_screen_min_2) * (SP_X_MAX - SP_X_MIN + 1) /
         (sp_screen_max_2 - sp_screen_min_2 + 1) + SP_X_MIN;
    xs = max(min(xs,SP_X_MAX),SP_X_MIN);
//    MoveTo(spec_hdc,xs,SP_Y_MIN_2);
    MoveToEx(spec_hdc,xs,SP_Y_MIN_2,NULL);
    LineTo(spec_hdc,xs,SP_Y_MAX_2);
        
    SelectObject(spec_hdc,hpen_old);
    ReleaseDC(hDlg,spec_hdc);
  }
}

static void repaint_p2(HWND hDlg)
{
  HDC spec_hdc;
  HPEN hpen_old;
  int xs;    
  int range = 2;
    
  // update the scroll and text field
  if(sp_cur_selection == SP_UPPER_IMAGE)
  {    
    if(!out_smart)
    {
      out_smart = 1;
	  if(sp_data[sp_current])
      {
	    if(sp_data[sp_current]->bit16 &&
		  (sp_data[sp_current]->output_ch == 10 || sp_data[sp_current]->output_ch == 11))
		{
          SetScrollPos(GetDlgItem(hDlg,SP_P2_SCROLL),SB_CTL,
			       DTOI(sp_p2_vbits,SP_MIN_MIN,SP_MAX_MAX * 16),TRUE);
          sprintf(string,"%0.4f",dtov16(sp_p2_vbits));
		}
	    else
		{
          SetScrollPos(GetDlgItem(hDlg,SP_P2_SCROLL),SB_CTL,
			       DTOI(sp_p2_vbits,SP_MIN_MIN,SP_MAX_MAX),TRUE);
          sprintf(string,"%0.3f",dtov(sp_p2_vbits,get_range(sp_data[sp_current]->output_ch)));
		}
	  }
	  else
	  {
        SetScrollPos(GetDlgItem(hDlg,SP_P2_SCROLL),SB_CTL,
			       DTOI(sp_p2_vbits,SP_MIN_MIN,SP_MAX_MAX),TRUE);
        sprintf(string,"%0.3f",dtov(sp_p2_vbits,get_range(sp_output_ch)));
	  }
      SetDlgItemText(hDlg,SP_P2_VOLTS,string);
      out_smart = 0;
	}
    if(sp_data_valid(sp_current))
    {
      if(sp_p2_vbits >= sp_data[sp_current]->min_x && 
         sp_p2_vbits <= sp_data[sp_current]->max_x)
      {
        sprintf(string,"%0.3lf",in_dtov(sp_data[sp_current]->yf[(int)(sp_p2_vbits -
            sp_data[sp_current]->min_x) / sp_data[sp_current]->step]));
        SetDlgItemText(hDlg,SP_P2_CURR,string);
	  }
      else SetDlgItemText(hDlg,SP_P2_CURR,"N/A");
	}
  }
  else
  {
    if(!out_smart)
    {
      out_smart = 1;
	  if(second_data[current_second_data])
	  {
	    if(second_data[current_second_data]->bit16 &&
		  (second_data[current_second_data]->output_ch == 10 || second_data[current_second_data]->output_ch == 11))
		{
          SetScrollPos(GetDlgItem(hDlg,SP_P2_SCROLL),SB_CTL,
		           DTOI(sp_p2_vbits_2,SP_MIN_MIN,SP_MAX_MAX * 16),TRUE);
          sprintf(string,"%0.4f",dtov16(sp_p2_vbits_2));
		}
	    else
		{
          SetScrollPos(GetDlgItem(hDlg,SP_P2_SCROLL),SB_CTL,
		           DTOI(sp_p2_vbits_2,SP_MIN_MIN,SP_MAX_MAX),TRUE);
	      if(second_data[current_second_data] != NULL)
		  {
	        if(second_data[current_second_data]->valid)
			{
	          range = second_data[current_second_data]->v_range;
			}
	        else range = 2;
		  }
          sprintf(string,"%0.3f",dtov(sp_p2_vbits_2,range));
		}
	  }
	  else
	  {
        SetScrollPos(GetDlgItem(hDlg,SP_P2_SCROLL),SB_CTL,
		           DTOI(sp_p2_vbits_2,SP_MIN_MIN,SP_MAX_MAX),TRUE);
	    if(second_data[current_second_data] != NULL)
		{
	      if(second_data[current_second_data]->valid)
		  {
	        range = second_data[current_second_data]->v_range;
		  }
	      else range = 2;
		}
        sprintf(string,"%0.3f",dtov(sp_p2_vbits_2,range));
	  }
      SetDlgItemText(hDlg,SP_P2_VOLTS,string);
      out_smart = 0;
    }
    if(second_data[current_second_data] != NULL)
    {
      if(second_data[current_second_data]->valid)
	  {
        if(sp_p2_vbits_2 >= second_data[current_second_data]->min_x && 
           sp_p2_vbits_2 <= second_data[current_second_data]->max_x)
        {
          sprintf(string,"%0.3lf", in_dtov(second_data[current_second_data]->yf[(int)(sp_p2_vbits_2-
          second_data[current_second_data]->min_x) / second_data[current_second_data]->step]));
          SetDlgItemText(hDlg,SP_P2_CURR,string);
		}
        else SetDlgItemText(hDlg,SP_P2_CURR,"N/A");
	  }
	}
  }

  // draw the red p2 line in the upper image, if appropriate
  if(sp_p2_vbits >= sp_screen_min && sp_p2_vbits <= sp_screen_max)
  {
    spec_hdc = GetDC(hDlg);
    hpen_old = SelectObject(spec_hdc,hpen_red);

    xs = (sp_p2_vbits - sp_screen_min) * (SP_X_MAX - SP_X_MIN + 1) /
         (sp_screen_max - sp_screen_min + 1) + SP_X_MIN;
    xs = max(min(xs,SP_X_MAX),SP_X_MIN);
//    MoveTo(spec_hdc,xs,SP_Y_MIN);
    MoveToEx(spec_hdc,xs,SP_Y_MIN,NULL);
    LineTo(spec_hdc,xs,SP_Y_MAX);

    SelectObject(spec_hdc,hpen_old);
    ReleaseDC(hDlg,spec_hdc);
  }
    
  // draw the red p2 line in the lower image, if appropriate
  if(sp_p2_vbits_2 >= sp_screen_min_2 && sp_p2_vbits_2 <= sp_screen_max_2)
  {
    spec_hdc = GetDC(hDlg);
    hpen_old = SelectObject(spec_hdc,hpen_red);

    xs = (sp_p2_vbits_2 - sp_screen_min_2) * (SP_X_MAX - SP_X_MIN + 1) /
         (sp_screen_max_2 - sp_screen_min_2 + 1) + SP_X_MIN;
    xs = max(min(xs,SP_X_MAX),SP_X_MIN);
//    MoveTo(spec_hdc,xs,SP_Y_MIN_2);
    MoveToEx(spec_hdc,xs,SP_Y_MIN_2,NULL);
    LineTo(spec_hdc,xs,SP_Y_MAX_2);
        
    SelectObject(spec_hdc,hpen_old);
    ReleaseDC(hDlg,spec_hdc);
  }   
}

static void repaint_graphics(HWND hDlg)
{
  clear_area(hDlg,SP_X_MIN,SP_Y_MIN,SP_X_MAX,SP_Y_MAX,RGB(255,255,255));
  clear_area(hDlg,SP_X_MIN,SP_Y_MIN_2,SP_X_MAX,SP_Y_MAX_2,RGB(255,255,255));
  repaint_grid(hDlg,SP_X_MIN,SP_X_MAX,SP_Y_MIN,SP_Y_MAX);
  repaint_grid(hDlg,SP_X_MIN,SP_X_MAX,SP_Y_MIN_2,SP_Y_MAX_2);
  repaint_p1(hDlg);
  repaint_p2(hDlg);
  repaint_data(hDlg);

  // draw box borders border
  if(sp_cur_selection == SP_UPPER_IMAGE)
  {
	// 0 to erase old red border, 2 to draw new black border, 1 to draw new red border
	repaint_rect(hDlg,0,SP_LOWER_IMAGE,SMALL_BORDER_SIZE,SMALL_IMAGE_SIZE);
	repaint_rect(hDlg,2,SP_LOWER_IMAGE,SMALL_BORDER_SIZE,SMALL_IMAGE_SIZE);
	repaint_rect(hDlg,1,SP_UPPER_IMAGE,SMALL_BORDER_SIZE,SMALL_IMAGE_SIZE);
  }
  else // sp_cur_selection == SP_LOWER_IMAGE
  {
	// 0 to erase old red border,2 to draw new black border,1 to draw new red border
	repaint_rect(hDlg,0,SP_UPPER_IMAGE,SMALL_BORDER_SIZE,SMALL_IMAGE_SIZE);
	repaint_rect(hDlg,2,SP_UPPER_IMAGE,SMALL_BORDER_SIZE,SMALL_IMAGE_SIZE);
	repaint_rect(hDlg,1,SP_LOWER_IMAGE,SMALL_BORDER_SIZE,SMALL_IMAGE_SIZE);
  }
}

#define YFS(Y) (((float)vert_max - (float)Y) * \
                 (float)(y_max - y_min + 1) / \
                 (float)(vert_max - vert_min)) + y_min

void repaint_spec_graphics(HDC spec_hdc,HPEN hpen,datadef *this_data,
    int screen_min,int screen_max,
    int x_min,int y_min,int x_max,int y_max, int vert_max, int vert_min)
{    
  int start,end;
  HPEN hpen_old;
  int xs,ys,i;
  float yfs;
        
  hpen_old = SelectObject(spec_hdc,hpen);
        
  if(screen_min > this_data->min_x) 
  {
    start = (int)((screen_min - this_data->min_x) /
                    this_data->step);
  }
  else
  {
    start = 0;
  }
  if(screen_max < this_data->max_x) 
  { 
    end = (int)((screen_max - this_data->min_x) /
                this_data->step + 1);
    if(end > this_data->size) end = this_data->size;
  }
  else 
  {
    end = this_data->size;
  }
         
  xs = (index2d(this_data,start) - screen_min) * (x_max - x_min + 1) /
       (screen_max - screen_min + 1) + x_min;
  yfs = YFS(this_data->yf[start]);
  xs = max(min(xs,x_max),x_min);
  yfs = max(min(yfs,y_max),y_min);
  ys = (int)yfs;
//  MoveTo(spec_hdc,xs,ys);
  MoveToEx(spec_hdc,xs,ys,NULL);
  for(i = start + 1;i < end;i++)
  {
    xs = (index2d(this_data,i) - screen_min) * (x_max - x_min + 1) /
         (screen_max - screen_min + 1) + x_min;
    yfs = YFS(this_data->yf[i]);
    xs = max(min(xs,x_max),x_min);
    yfs = max(min(yfs,y_max),y_min);
    ys = (int)yfs;
    LineTo(spec_hdc,xs,ys);
  }
            
  SelectObject(spec_hdc,hpen_old);
}

static void repaint_track_displacement(HWND hDlg)
{
  if(sp_track_every)
  {
    sprintf(string,"%d",(int)(sp_landing_x) - (int)sp_takeoff_x);
    SetDlgItemText(hDlg,SP_DELTA_X,string);
    sprintf(string,"%d",(int)(sp_landing_y) - (int)sp_takeoff_y);
    SetDlgItemText(hDlg,SP_DELTA_Y,string);
  }
  else
  {
    SetDlgItemText(hDlg,SP_DELTA_X,"n/a");
    SetDlgItemText(hDlg,SP_DELTA_Y,"n/a");
  }
}

static void repaint_all(HWND hDlg)
{
  unsigned int current_output;

  repaint_current(hDlg);
  repaint_spec_scale(hDlg);
//  repaint_background(hDlg);
  repaint_p1(hDlg);
  repaint_p2(hDlg);
//  repaint_integral(hDlg);
  repaint_vert_scale(hDlg);
  repaint_vert_scale_min(hDlg);
  repaint_screen_scroll(hDlg);
  repaint_graphics(hDlg);
  repaint_mapping(hDlg);
  spec_set_title(hDlg);
            
  switch(sp_output_ch)
  {
    case sample_bias_ch:
	  if(!bit16)
	  {
        strcpy(string,"Bias:");
	  }
	  else
	  {
	    sprintf(string,"Ch. %d",sp_output_ch);
	  }
      break;
    case highres_bias_ch:
	  if(bit16)
	  {
        strcpy(string,"Bias:");
	  }
	  else
	  {
	    sprintf(string,"Ch. %d",sp_output_ch);
	  }
      break;
    case x_ch:
      strcpy(string,"X:");
      break;
    case x_offset_ch:
      strcpy(string,"X Off.:");
      break;
    case y_ch:
      strcpy(string,"Y:");
      break;
    case y_offset_ch:
      strcpy(string,"Y Off.:");
      break;
    case zo_ch:
      strcpy(string,"Z Outer:");
      break;
    case i_setpoint_ch:
      strcpy(string,"I Set:");
      break;
    case 1:
	  if(z_offset_ch == 1)
	  {
        strcpy(string,"Z Off.:");
	  }
	  else
	  {
	    sprintf(string,"Ch. %d",sp_output_ch);
	  }
      break;
    case 7:
	  if(z_offset_ch == 7)
	  {
        strcpy(string,"Z Off.:");
	  }
	  else
	  {
	    sprintf( string,"Ch. %d",sp_output_ch);
	  }
      break;
	default:
	  sprintf(string,"Ch. %d",sp_output_ch);
	  break;
  }
  SetDlgItemText(hDlg,SP_OUTPUT_CH_NAME,string);

  if(bit16 && (sp_output_ch == 10 || sp_output_ch == 11))
  {
	if(sp_output_ch == 10)
      current_output = (dac_data[extra_ch] & 0xF00) << 4;
	else //(sp_output_ch == 11)
      current_output = (dac_data[extra_ch] & 0x0F0) << 4;
    current_output += dac_data[sp_output_ch];
  }
  else current_output = dac_data[sp_output_ch];
    
  sp_update(hDlg,current_output);
}

static void repaint_vert_scale(HWND hDlg)
{
  out_smart = 1;
  if(sp_cur_selection == SP_UPPER_IMAGE)
  {
	sprintf(string,"%0.3f",in_dtov(sp_vert_max));
    SetDlgItemText(hDlg,SP_V_SCALE,string);
    SetScrollPos(GetDlgItem(hDlg,SP_V_SCALE_SCROLL),
            SB_CTL,DTOI(sp_vert_max,sp_vert_min_min,sp_vert_max_max) ,TRUE);
  }
  else 
  {
	sprintf(string,"%0.3f",in_dtov(sp_vert_max_2));
    SetDlgItemText(hDlg,SP_V_SCALE,string);
    SetScrollPos(GetDlgItem(hDlg,SP_V_SCALE_SCROLL),
          SB_CTL,DTOI(sp_vert_max_2,sp_vert_min_min,sp_vert_max_max) ,TRUE);
  }
  out_smart = 0;
}

static void repaint_vert_scale_min(HWND hDlg)
{
  out_smart = 1;
  if(sp_cur_selection == SP_UPPER_IMAGE)
  {
	sprintf(string,"%0.3f",in_dtov(sp_vert_min));
    SetDlgItemText(hDlg,SP_V_SCALE_MIN,string);
    SetScrollPos(GetDlgItem(hDlg,SP_V_SCALE_MIN_SCROLL),
           SB_CTL,DTOI(sp_vert_min,sp_vert_min_min,sp_vert_max_max) ,TRUE);
  }
  else
  {
	sprintf(string,"%0.3f",in_dtov(sp_vert_min_2));
    SetDlgItemText(hDlg,SP_V_SCALE_MIN,string);
    SetScrollPos(GetDlgItem(hDlg,SP_V_SCALE_MIN_SCROLL),
           SB_CTL,DTOI(sp_vert_min_2,sp_vert_min_min,sp_vert_max_max) ,TRUE);
  }
  out_smart = 0;
}

static void repaint_spec_scale(HWND hDlg)
{
  out_smart = 1;
  sprintf(string,"%d",tip_gain);
  SetDlgItemText(hDlg,SP_PREAMP_SCALE,string);
  out_smart = 0;
}

static void repaint_current(HWND hDlg)
{
  out_smart = 1;
  if(sp_cur_selection == SP_UPPER_IMAGE)
  {
    sprintf(string,"%d",sp_current);
    SetDlgItemText(hDlg,SP_CURRENT,string);
    SetScrollPos(GetDlgItem(hDlg,SP_CURRENT_SCROLL), 
        SB_CTL,DTOI(sp_current,0,SP_DATASETS - 1),TRUE);
  }
  else
  {
    sprintf(string,"%d",current_second_data);
    SetDlgItemText(hDlg,SP_CURRENT,string);
    SetScrollPos(GetDlgItem(hDlg,SP_CURRENT_SCROLL), 
        SB_CTL,DTOI(current_second_data,0,SP_DATASETS - 1),TRUE);
  }
  out_smart = 0;
}

static void repaint_screen_scroll(HWND hDlg)
{
  out_smart = 1;
  if(sp_cur_selection == SP_UPPER_IMAGE)
  {
    if(sp_data_valid(sp_current)) 
    SetScrollPos(GetDlgItem(hDlg,SP_SCREEN_SCROLL),SB_CTL,
            DTOI(sp_screen_min,sp_data[sp_current]->min_x,
            sp_data[sp_current]->max_x - sp_screen_max + sp_screen_min),TRUE);
  } 
  else
  {
    if(second_data[current_second_data] != NULL) 
	if(second_data[current_second_data]->valid)
	{
      SetScrollPos(GetDlgItem(hDlg,SP_SCREEN_SCROLL),SB_CTL,
            DTOI(sp_screen_min_2,second_data[current_second_data]->min_x,
            second_data[current_second_data]->max_x - sp_screen_max_2 + sp_screen_min_2),TRUE);
	}
  }
  out_smart = 0;
}
    
static void repaint_mapping(HWND hDlg)
{

  CheckDlgButton(hDlg,SP_VIB_MAP,sp_multiple_scans);
  if(sp_multiple_scans)
  {
	enable_vib_map_params(hDlg,1);
	repaint_vib_map_params(hDlg,0);
  }
  else
  {
	enable_vib_map_params(hDlg,0);
  }

}

static void repaint_vib_map_params(HWND hDlg,int point_num /* zero based */)
{
  sprintf(string,"%d of %d",point_num+1,sp_scan_count);
  SetDlgItemText(hDlg,SPC_VM_POINT,string);
  sprintf(string,"%d",sp_offsets_map[point_num].x);
  SetDlgItemText(hDlg,SPC_VM_XOFF,string);
  sprintf(string,"%d",sp_offsets_map[point_num].y);
  SetDlgItemText(hDlg,SPC_VM_YOFF,string);
}



// ******************************************************************************************
//		O L D * O L D * O L D * O L D * O L D * O L D * O L D * O L D * O L D * O L D
// ******************************************************************************************


#ifdef OLD

static void spec_scan_old(HWND hDlg,int *pass_count,int strobe_before)
{
// performs one spectroscopic scan (possibly consisting of many passes)

  int i,data_pos,sample_count,data_per_pass;
  float data_i,data_z,data_2,data_3;
  unsigned int this_data;
  int ch_offset,extra_data;
  int strobe_after;

  // initial tracking, offsets
  if(sp_track_every && sp_track_at_min)
  {
    // track at the minimum scanning voltage
	dio_feedback(sp_feedback == 1);		// assume this state appropriate for ramp
	sp_status(hDlg,0,"ramping to start...");
	ramp_ch(sp_output_ch,sp_scan_min,sp_move_delay,0,sp_bits_per_step);
  }
  if(sp_track_every) 
  {
    dio_feedback(1);
    dio_dither(0,0);
    dio_dither(1,0);
    sp_status(hDlg,0,"tracking...");
    for(i = 0;i < sp_track_iterations && sp_scanning;i++)
	{
      track(0,0,0,sp_track_avg_data_pts,
            sp_tracking_mode == SP_TRACK_MAX,sp_track_step_delay,
            sp_track_step_delay,sp_track_limit_x_min,
            sp_track_limit_x_max,sp_track_limit_y_min,
            sp_track_limit_y_max,sp_track_plane_a,
		    sp_track_plane_b);
	  // repaint_track_current(hDlg);
	}
    if(sp_track_offset_x) 
	{
	  if(sp_offset_hold && !sp_feedback)
	  dio_feedback(sp_feedback == 1);
      i = (int)dac_data[x_ch] + sp_track_offset_x;
      if(i < 0) i = 0;
      if(i > MAX) i = MAX;
      move_to_timed(x_ch,dac_data[x_ch],i,SD->step_delay);
      dac_data[x_ch] = i;
	}
    if(sp_track_offset_y)
	{
	  if(sp_offset_hold && !sp_feedback)
	  dio_feedback(sp_feedback == 1);
      i = (int)dac_data[y_ch] + sp_track_offset_y;
      if(i < 0) i = 0;
      if(i > MAX) i = MAX;
      move_to_timed(y_ch,dac_data[y_ch],i,SD->step_delay);
      dac_data[y_ch] = i;
	}
//    repaint_track_current(hDlg);
            
  }

  // wait and then turn feedback on / off
  if(sp_feedback != 1) 
  {
    dio_start_clock(sp_feedback_wait);
    if(sp_feedback_wait > SP_UPDATE_TIME) sp_update(hDlg,sp_output_initial);
    dio_wait_clock();
  }
  dio_feedback(sp_feedback == 1);

  // delta z-offset: ramp the sample bias, adjust z offset, wait
  if(sp_z_offset)
  {
    sp_status(hDlg,0,"z-offset...");
    ramp_bias(sp_z_offset_bias,sp_move_delay,0,sp_bits_per_step);
    move_to_speed(z_offset_ch,dac_data[z_offset_ch],
                dac_data[z_offset_ch] + sp_z_offset,
                DEFAULT_Z_OFFSET_TIME,0,0,0,DIGITAL_FEEDBACK_MAX,
                DIGITAL_FEEDBACK_REREAD);
    dio_start_clock(SP_CREEP_TIME);
    dac_data[z_offset_ch] += sp_z_offset;
    scan_z = dac_data[z_offset_ch];
    dio_wait_clock(); /* creep */
  }

  // ramp the output channel from initial value to minimum scanning voltage
  sp_status(hDlg,0,"ramping to start...");
  ramp_ch(sp_output_ch,sp_scan_min,sp_move_delay,0,sp_bits_per_step);

  // dithering
  dio_dither(0,sp_dither0 == SP_DITHER0_ON);
  dio_dither(1,sp_dither1 == SP_DITHER1_ON);
  if(sp_dither0 == SP_DITHER0_ON || sp_dither1 == SP_DITHER1_ON)
  {
    dio_start_clock(sp_dither_wait);
    if(sp_dither_wait > SP_UPDATE_TIME) sp_update(hDlg,sp_scan_min);
    dio_wait_clock();
  }

  strobe_before |= dac_data[AD_ch];
  strobe_after = dac_data[AD_ch];
  CheckMsgQ();
  if(sp_avg_both) data_per_pass = 2;
  else data_per_pass = 1;
//  sprintf(string,"strobe before %d",strobe_before);
//  mprintf(string);

  // begin passes
  for((*pass_count) = 0;(*pass_count) < sp_num_passes && sp_scanning;(*pass_count)++)
  {
    sp_status(hDlg,(*pass_count),"");
    data_pos = 0;

	// scan in forward direction
    for(i = sp_scan_min;i <= sp_scan_max;i += sp_bits_per_step,data_pos++)
	{
      dio_out(sp_output_ch,i);
      if(sp_forward || sp_avg_both)
	  {
        dio_start_clock(sp_speed);
        if(sp_speed > SP_UPDATE_TIME) sp_update(hDlg,i);
        dio_wait_clock();
        data_i = data_z = data_2 = data_3 = 0;
        for(sample_count = 0;sample_count < sp_num_samples;sample_count++)
		{

		  // take measurements
          if(sp_measure_i)
		  {
            dio_in_ch(0);
            dio_in_data(&this_data);
            data_i += (float)this_data;
		  }
          if(sp_measure_z)
		  {
            dio_in_ch(1);
            dio_in_data(&this_data);
            data_z += (float)this_data;
		  }
          if(sp_measure_2)
		  {
            dio_in_ch(2);
            dio_in_data(&this_data);
            data_2 += (float)this_data;
		  }
          if(sp_measure_3)
		  {
            dio_in_ch(3);
            dio_in_data(&this_data);
            data_3 += (float)this_data;
		  }
#ifdef OLD
          dio_out(AD_ch,strobe_before);
          dio_out(AD_ch,strobe_after);
          adc_delay();
#ifdef OLD
#define MULTI_READ_DELAY    outp(cntrcmd,0x74);\
                    outp(cntrcmd,0x74);\
                    outp(cntrcmd,0x74);\
                    outp(cntrcmd,0x74);\
                    outp(cntrcmd,0x74);
#endif
#define MULTI_READ_DELAY    adc_delay();
                        
          if(sp_measure_i)
		  {
            outpw(cfg1,0);
            outpw(cfg2,0);
            out1 = 0;
            MULTI_READ_DELAY;
            data_i += (float)inpw(portc);
		  }
          if(sp_measure_z)
		  {
            outpw(cfg1,1);
            outpw(cfg2,0);
            out1 = 1;
            MULTI_READ_DELAY;
            data_z += (float)inpw(portc);
		  }
          if(sp_measure_2)
		  {
            outpw(cfg1,0);
            outpw(cfg2,1);
            out1 = 0;
            MULTI_READ_DELAY;
            data_2 += (float)inpw(portc);
		  }
          if(sp_measure_3)
		  {
            outpw(cfg1,1);
            outpw(cfg2,1);
            out1 = 1;
            MULTI_READ_DELAY;
            data_3 += (float)inpw(portc);
		  }
#endif
		}

		// average the measurements
        if(sp_measure_i)
		{
          data_i /= sp_num_samples;
          sp_data[0]->yf[data_pos] =
                      (sp_data[0]->yf[data_pos] * data_per_pass *
                      (*pass_count) + data_i)
                      / (data_per_pass * (*pass_count) + 1);
		}
        if (sp_measure_z)
		{
          data_z /= sp_num_samples;
          sp_data[1]->yf[data_pos] =
                      (sp_data[1]->yf[data_pos] * data_per_pass *
                      (*pass_count) + data_z) /
                      (data_per_pass * (*pass_count) + 1);
		}
        if(sp_measure_2)
		{
          data_2 /= sp_num_samples;
          sp_data[2]->yf[data_pos] =
                      (sp_data[2]->yf[data_pos] * data_per_pass *
                      (*pass_count) + data_2) /
                      (data_per_pass * (*pass_count) + 1);
		}
        if(sp_measure_3)
		{
          data_3 /= sp_num_samples;
          sp_data[3]->yf[data_pos] =
                      (sp_data[3]->yf[data_pos] * data_per_pass *
                      (*pass_count) + data_3) /
                      (data_per_pass * (*pass_count) + 1);
		}
      }
      
	  // kafka
	  else
                {
                    dio_start_clock(sp_move_delay);
                    if (sp_move_delay>SP_UPDATE_TIME) sp_update(hDlg,i);
                    dio_wait_clock();
                }
            }
                
            if (sp_forward || sp_avg_both)
            {
                if (sp_measure_i) sp_data[0]->size=data_pos;
                if (sp_measure_z) sp_data[1]->size=data_pos;
                if (sp_measure_2) sp_data[2]->size=data_pos;
                if (sp_measure_3) sp_data[3]->size=data_pos;
            }
            if (sp_backward && !sp_avg_both)
            {
                if (sp_measure_i) sp_data[4]->size=data_pos;
                if (sp_measure_z) sp_data[5]->size=data_pos;
                if (sp_measure_2) sp_data[6]->size=data_pos;
                if (sp_measure_3) sp_data[7]->size=data_pos;
            }
            
	    // backward direction
            for(i-=sp_bits_per_step,data_pos--;i>=sp_scan_min;i-=sp_bits_per_step,data_pos--)
            {
                dio_out(sp_output_ch,i);
                if (sp_backward || sp_avg_both)
                {
                    dio_start_clock(sp_speed);
                    if (sp_speed>SP_UPDATE_TIME) sp_update(hDlg,i);
                    dio_wait_clock();
                    data_i=data_z=data_2=data_3=0;
                    for(sample_count=0;sample_count<sp_num_samples;sample_count++)
                    {
			// take measurements
                        if (sp_measure_i)
                        {
                            dio_in_ch(0);
                            dio_in_data(&this_data);
                            data_i+=(float) this_data;
                        }
                        if (sp_measure_z)
                        {
                            dio_in_ch(1);
                            dio_in_data(&this_data);
                            data_z+=(float) this_data;
                        }
                        if (sp_measure_2)
                        {
                            dio_in_ch(2);
                            dio_in_data(&this_data);
                            data_2+=(float) this_data;
                        }
                        if (sp_measure_3)
                        {
                            dio_in_ch(3);
                            dio_in_data(&this_data);
                            data_3+=(float) this_data;
                        }
#ifdef OLD
                        dio_out(AD_ch,strobe_before);
                        dio_out(AD_ch,strobe_after);
                        adc_delay();
                        if (sp_measure_i)
                        {
                            outpw(cfg1,0);
                            outpw(cfg2,0);
                            out1 = 0;
                            MULTI_READ_DELAY;
                            data_i+=(float) inpw(portc);
                        }
                        if (sp_measure_z)
                        {
                            outpw(cfg1,1);
                            outpw(cfg2,0);
                            out1 = 1;
                            MULTI_READ_DELAY;
                            data_z+=(float) inpw(portc);
                        }
                        if (sp_measure_2)
                        {
                            outpw(cfg1,0);
                            outpw(cfg2,1);
                            out1 = 0;
                            MULTI_READ_DELAY;
                            data_2+=(float) inpw(portc);
                        }
                        if (sp_measure_3)
                        {
                            outpw(cfg1,1);
                            outpw(cfg2,1);
                            out1 = 1;
                            MULTI_READ_DELAY;
                            data_3+=(float) inpw(portc);
                        }
#endif
                    }
                    if (sp_avg_both) 
                    {
                        ch_offset=0;
                        extra_data=1;
                    }
                    else
                    {
                        ch_offset=4;
                        extra_data=0;
                    }
                        
                    if (sp_measure_i)
                    {
                        data_i/=sp_num_samples;
                        sp_data[0+ch_offset]->yf[data_pos]=
                            (sp_data[0+ch_offset]->yf[data_pos]*
                            (data_per_pass*(*pass_count)+extra_data)+
                            data_i)/(data_per_pass*(*pass_count)+1+extra_data);
                    }
                    if (sp_measure_z)
                    {
                        data_z/=sp_num_samples;
                        sp_data[1+ch_offset]->yf[data_pos]=
                            (sp_data[1+ch_offset]->yf[data_pos]*
                            (data_per_pass*(*pass_count)+extra_data)+
                            data_z)/(data_per_pass*(*pass_count)+1+extra_data);
                    }
                    if (sp_measure_2)
                    {
                        data_2/=sp_num_samples;
                        sp_data[2+ch_offset]->yf[data_pos]=
                            (sp_data[2+ch_offset]->yf[data_pos]*
                            (data_per_pass*(*pass_count)+extra_data)+
                            data_2)/(data_per_pass*(*pass_count)+1+extra_data);
                    }
                    if (sp_measure_3)
                    {
                        data_3/=sp_num_samples;
                        sp_data[3+ch_offset]->yf[data_pos]=
                            (sp_data[3+ch_offset]->yf[data_pos]*
                            (data_per_pass*(*pass_count)+extra_data)+
                            data_3)/(data_per_pass*(*pass_count)+1+extra_data);
                    }
                
                }
                else
                {
                    dio_start_clock(sp_move_delay);
                    if (sp_move_delay>SP_UPDATE_TIME) sp_update(hDlg,i);
                    dio_wait_clock();
                }
            }
            if (sp_forward || sp_avg_both)
            {
                if (sp_measure_i) 
                {
                    sp_data[0]->valid=1;
                    sp_data[0]->num_passes=(*pass_count)+1;
                    sp_data[0]->min_x=sp_scan_min;
                    sp_data[0]->max_x=sp_scan_max;
                }
                if (sp_measure_z) 
                {
                    sp_data[1]->valid=1;
                    sp_data[1]->num_passes=(*pass_count)+1;
                    sp_data[1]->min_x=sp_scan_min;
                    sp_data[1]->max_x=sp_scan_max;
                }
                if (sp_measure_2) 
                {
                    sp_data[2]->valid=1;
                    sp_data[2]->num_passes=(*pass_count)+1;
                    sp_data[2]->min_x=sp_scan_min;
                    sp_data[2]->max_x=sp_scan_max;
                }
                if (sp_measure_3) 
                {
                    sp_data[3]->valid=1;
                    sp_data[3]->num_passes=(*pass_count)+1;
                    sp_data[3]->min_x=sp_scan_min;
                    sp_data[3]->max_x=sp_scan_max;
                }
            }
            if (sp_backward && !sp_avg_both)
            {
                if (sp_measure_i) 
                {
                    sp_data[4]->valid=1;
                    sp_data[4]->num_passes=(*pass_count)+1;
                }
                if (sp_measure_z) 
                {
                    sp_data[5]->valid=1;
                    sp_data[5]->num_passes=(*pass_count)+1;
                }
                if (sp_measure_2) 
                {
                    sp_data[6]->valid=1;
                    sp_data[6]->num_passes=(*pass_count)+1;
                }
                if (sp_measure_3) 
                {
                    sp_data[7]->valid=1;
                    sp_data[7]->num_passes=(*pass_count)+1;
                }
            }
                
            repaint_graphics(hDlg);
            CheckMsgQ();

	    // tracking
            if (sp_track_every && sp_scanning) 
            {
                if (!(((*pass_count)+1)%sp_track_every) && (*pass_count)!=sp_num_passes-1)
                {
		    // suspend dithering
                    dio_dither(0,0);
                    dio_dither(1,0);

		    // ramp to initial value for tracking
                    if (sp_scan_min!=sp_output_initial && !sp_track_at_min)
                    {
                        ramp_ch(sp_output_ch,sp_output_initial,sp_move_delay,0,sp_bits_per_step);
                    }
                    
		    // feedback on
                    dio_feedback(1);

		    // "undo" delta z-offset and x, y offsets
                    if (sp_z_offset)
                    {
                        sp_status(hDlg,0,"z-offset...");
                        if (sp_z_offset_bias!=ZERO) ramp_bias(sp_z_offset_bias,sp_move_delay,0,sp_bits_per_step);
                        move_to_speed(z_offset_ch,dac_data[z_offset_ch],
                            dac_data[z_offset_ch]-sp_z_offset,
                            DEFAULT_Z_OFFSET_TIME, 0, 0,0,DIGITAL_FEEDBACK_MAX,
                            DIGITAL_FEEDBACK_REREAD);
                        dio_start_clock(1000000);
                        dac_data[z_offset_ch]+=sp_z_offset;
                        scan_z=dac_data[z_offset_ch];
                        dio_wait_clock(); /* creep */
                    }
                    sp_status(hDlg,(*pass_count),"tracking...");
                    if (sp_track_offset_x) 
                    {
                        i=(int)dac_data[x_ch]-sp_track_offset_x;
                        if (i<0) i=0;
                        if (i>MAX) i=MAX;
                        move_to_timed(x_ch,dac_data[x_ch],
                            i,SD->step_delay);
                        dac_data[x_ch]=i;
                    }
                    if (sp_track_offset_y)
                    {
                        i=(int)dac_data[y_ch]-sp_track_offset_y;
                        if (i<0) i=0;
                        if (i>MAX) i=MAX;
                        move_to_timed(y_ch,dac_data[y_ch],
                        i,SD->step_delay);
                        dac_data[y_ch]=i;
                    }
                    
		    // actual tracking
                    for(i=0;i<sp_track_iterations && sp_scanning;i++)
                    {
#ifdef NEED_UPDATE
                        track(0,0,0,sp_track_avg_data_pts,
                            sp_tracking_mode==SP_TRACK_MAX,sp_track_step_delay,
                            sp_track_step_delay,sp_track_limit_x_min,
                            sp_track_limit_x_max,sp_track_limit_y_min,
                            sp_track_limit_y_max,sp_track_plane_a,
			    sp_track_plane_b);
//                                repaint_track_current(hDlg);
#endif
                    }

		    // return to scanning state (offsets, feedback, dithering back to scanning values)
                    if (sp_track_offset_x) 
                    {
				        if(sp_offset_hold && !sp_feedback)
					        dio_feedback(sp_feedback == 1);
                        i=(int)dac_data[x_ch]+sp_track_offset_x;
                        if (i<0) i=0;
                        if (i>MAX) i=MAX;
                        move_to_timed(x_ch,dac_data[x_ch],
                            i,SD->step_delay);
                        dac_data[x_ch]=i;
                    }
                    if (sp_track_offset_y)
                    {
				        if(sp_offset_hold && !sp_feedback)
					        dio_feedback(sp_feedback == 1);
                        i=(int)dac_data[y_ch]+sp_track_offset_y;
                        if (i<0) i=0;
                        if (i>MAX) i=MAX;
                        move_to_timed(y_ch,dac_data[y_ch],
                        i,SD->step_delay);
                        dac_data[y_ch]=i;
                    }
//                            repaint_track_current(hDlg);
                    
                    if (sp_feedback!=1) 
                    {
                        dio_start_clock(sp_feedback_wait);
                        if (sp_feedback_wait>SP_UPDATE_TIME) sp_update(hDlg,sp_output_initial);
                        dio_wait_clock();
                    }
                    dio_feedback(sp_feedback==1);
                    if (sp_feedback==1) 
                    {
                        dio_start_clock(sp_feedback_wait);
                        if (sp_feedback_wait>SP_UPDATE_TIME) sp_update(hDlg,sp_output_initial);
                        dio_wait_clock();
                    }
                    if (sp_z_offset)
                    {
                        sp_status(hDlg,0,"z-offset...");
                        ramp_bias(sp_z_offset_bias,sp_move_delay,0,sp_bits_per_step);
                        move_to_speed(z_offset_ch,dac_data[z_offset_ch],
                            dac_data[z_offset_ch]+sp_z_offset,
                            DEFAULT_Z_OFFSET_TIME, 0, 0,0,DIGITAL_FEEDBACK_MAX,
                            DIGITAL_FEEDBACK_REREAD);
                        dio_start_clock(1000000);
                        dac_data[z_offset_ch]+=sp_z_offset;
                        scan_z=dac_data[z_offset_ch];
                        dio_wait_clock(); /* creep */
                    }
                    if (sp_scan_min!=sp_output_initial  && !sp_track_at_min)
                    {
                        ramp_ch(sp_output_ch,sp_scan_min,sp_move_delay,0,sp_bits_per_step);
                    }

		    // restore dithering
                    dio_dither(0,sp_dither0==SP_DITHER0_ON);
                    dio_dither(1,sp_dither1==SP_DITHER1_ON);
                    if (sp_dither0==SP_DITHER0_ON || sp_dither1==SP_DITHER1_ON)
                    {
                        dio_start_clock(sp_dither_wait);
                        if (sp_dither_wait>SP_UPDATE_TIME) sp_update(hDlg,sp_scan_min);
                        dio_wait_clock();
                    }
                }
            }

	    // feedback toggling (only if we're not tracking during the scan)
            if (sp_scanning && sp_feedback_every && sp_feedback==2 && !sp_track_every) 
            {
                if (!(((*pass_count)+1)%sp_feedback_every) && (*pass_count)!=sp_num_passes-1)
                {
		    // suspend dithering
                    dio_dither(0,0);
                    dio_dither(1,0);

                    if (sp_scan_min!=sp_output_initial && !sp_feedback_at_min)
                    {
                        ramp_ch(sp_output_ch,sp_output_initial,sp_move_delay,0,sp_bits_per_step);
                    }
                    
                    dio_feedback(1);
                    if (sp_z_offset)
                    {
                        sp_status(hDlg,0,"z-offset...");
                        if (sp_z_offset_bias!=ZERO) ramp_bias(sp_z_offset_bias,sp_move_delay,0,sp_bits_per_step);
                        move_to_speed(z_offset_ch,dac_data[z_offset_ch],
                            dac_data[z_offset_ch]-sp_z_offset,
                            DEFAULT_Z_OFFSET_TIME, 0, 0,0,DIGITAL_FEEDBACK_MAX,
                            DIGITAL_FEEDBACK_REREAD);
                        dio_start_clock(1000000);
                        dac_data[z_offset_ch]+=sp_z_offset;
                        scan_z=dac_data[z_offset_ch];
                        dio_wait_clock(); /* creep */
                    }
                    dio_start_clock(sp_feedback_wait);
                    sp_status(hDlg,(*pass_count),"feedback on...");
                    sp_update(hDlg,sp_output_initial);
                    dio_wait_clock();
                    dio_feedback(sp_feedback==1);
                    
                    if (sp_scan_min!=sp_output_initial && !sp_feedback_at_min)
                    {
                        ramp_ch(sp_output_ch,sp_scan_min,sp_move_delay,0,sp_bits_per_step);
                    }
                    if (sp_z_offset)
                    {
                        sp_status(hDlg,0,"z-offset...");
                        ramp_bias(sp_z_offset_bias,sp_move_delay,0,sp_bits_per_step);
                        move_to_speed(z_offset_ch,dac_data[z_offset_ch],
                            dac_data[z_offset_ch]+sp_z_offset,
                            DEFAULT_Z_OFFSET_TIME, 0, 0,0,DIGITAL_FEEDBACK_MAX,
                            DIGITAL_FEEDBACK_REREAD);
                        dio_start_clock(SP_CREEP_TIME);
                        dac_data[z_offset_ch]+=sp_z_offset;
                        scan_z=dac_data[z_offset_ch];
                        dio_wait_clock(); /* creep */
                    }

		    // restore dithering
                    dio_dither(0,sp_dither0==SP_DITHER0_ON);
                    dio_dither(1,sp_dither1==SP_DITHER1_ON);
                    if (sp_dither0==SP_DITHER0_ON || sp_dither1==SP_DITHER1_ON)
                    {
                        dio_start_clock(sp_dither_wait);
                        if (sp_dither_wait>SP_UPDATE_TIME) sp_update(hDlg,sp_scan_min);
                        dio_wait_clock();
                    }
                }
            }
            CheckMsgQ();
        }

        if (sp_z_offset>0) /* preventing current overload */
        {
            ramp_bias(sp_z_offset_bias,sp_move_delay,0,sp_bits_per_step);
            move_to_speed(z_offset_ch,dac_data[z_offset_ch],
                dac_data[z_offset_ch]-sp_z_offset,
                DEFAULT_Z_OFFSET_TIME, 0, 0,0,DIGITAL_FEEDBACK_MAX,
                DIGITAL_FEEDBACK_REREAD);
            dac_data[z_offset_ch]-=sp_z_offset;
            scan_z=dac_data[z_offset_ch];
        }

	// resume pre-scanning dithering status
        dio_dither(0,dither0_before);
        dio_dither(1,dither1_before);

	// back to initial output voltage
        sp_status(hDlg,*pass_count,"ramping to initial...");
        ramp_ch(sp_output_ch,sp_output_initial,sp_move_delay,0,sp_bits_per_step);

	// feedback on
        dio_feedback(1);

	// auto-
        if (sp_z_offset) 
        {
            dio_start_clock(100000);
            dio_wait_clock();
            auto_z_below(IN_ZERO);
        }

        sp_update(hDlg,sp_output_initial);
}

static void calc_integral()
{
  int i;

  sp_tot_integral = 0;
#ifdef OLD
  if(sp_data->valid) 
  {
    for(i = sp_data->start;i <= sp_data->end;i++)
    {
      sp_tot_integral += sp_data->ptr[i] - sp_background;
	}
//    sp_tot_integral -= sp_background * (sp_data->end - sp_data->start + 1);
  }
  else sp_tot_integral = 1;
#endif
  sp_tot_integral = 1;
}

static void calc_part_integral()
{
  sp_part_integral = 0;

#ifdef OLD
  int i,start,end;

  if(sp_data->valid)
  {
    if(sp_p1_vbits < sp_p2_vbits)
    {
      if(sp_p1_vbits > sp_data->start) start = sp_p1_vbits;
      else start = sp_data->start;
      if(sp_p2_vbits < sp_data->end) end = sp_p2_vbits;
      else end = sp_data->end;
	}
    else
	{
      if(sp_p2_vbits > sp_data->start) start = sp_p2_vbits;
      else start = sp_data->start;
      if(sp_p1_vbits < sp_data->end) end = sp_p1_vbits;
      else end = sp_data->end;
    }
    for(i = start;i <= end;i++)
    {
      sp_part_integral += sp_data->ptr[i] - sp_background;
    }
//    sp_part_integral -= sp_background * (end - start + 1);
  }
#endif
}                                                 

void calc_didv()
{
  int i,j;
  float didv;

  sp_data_didv->version = sp_data_iv->version;
  sp_data_didv->start = sp_data_iv->start + SP_WEIGHTS;
  sp_data_didv->end = sp_data_iv->end - SP_WEIGHTS;
  sp_data_didv->skip = sp_data_iv->skip;
  sp_data_didv->v_range = sp_data_iv->v_range;
  sp_data_didv->speed = sp_data_iv->speed;
  sp_data_didv->scale = sp_data_iv->scale;
  for(i = sp_data_iv->start + SP_WEIGHTS;i <= sp_data_iv->end - SP_WEIGHTS;i++)
  {
    if(i != ZERO && in_dtov(sp_data_iv->ptr[i]) != 0)
	{
      didv = 0;
      for(j = 1;j <= SP_WEIGHTS;j++)
	  {
        didv += (-sp_data_iv->ptr[i - j] +
                 sp_data_iv->ptr[i + j]) * j;
	  }
      sp_data_didv->ptr[i] = didv *
                             dtov(i,sp_data_iv->v_range) /
                             in_dtov(sp_data_iv->ptr[i]);
	}
    else sp_data_didv->ptr[i] = 0;
/*
    sp_data_didv->ptr[i] = (sp_data_iv->ptr[i + 1] - sp_data_iv->ptr[i - 1]) /
                2 / dtov(ZERO + 1,sp_data_iv->v_range);
*/
    if(i == sp_data_iv->start + 1) sp_min = sp_max = sp_data_didv->ptr[i];
    else
	{
      sp_min = min(sp_min,sp_data_didv->ptr[i]);
      sp_max = max(sp_max,sp_data_didv->ptr[i]);
	}
  }
}

static void find_background()
{
#ifdef OLD
  int i,start,end;
    
  if(sp_p1_vbits < sp_p2_vbits)
  {
    start = sp_p1_vbits;
    end = sp_p2_vbits;
  }
  else
  {
    start = sp_p2_vbits;
    end = sp_p1_vbits;
  }
  sp_background = sp_data->ptr[start];

  for(i = start + 1;i <= end;i++)
  {
    sp_background += sp_data->ptr[i];
  }
  sp_background = (int)floor((float)sp_background / (float)(end - start + 1) + 0.5);
#endif
}

static void repaint_integral(HWND hDlg)
{
  calc_part_integral();
  if(sp_tot_integral)
    sprintf(string,"%0.3f",(float)sp_part_integral / (float)sp_tot_integral * 100);
  else strcpy(string,"N/A");

  SetDlgItemText(hDlg,SP_INTEGRAL,string);
}

#endif
