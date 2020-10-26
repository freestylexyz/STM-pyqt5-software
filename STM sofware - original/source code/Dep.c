#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>
#include <sys\timeb.h>
#include <string.h>
#include <dos.h>
#include <math.h>
#include <windows.h>
#include <time.h>
//#include <toolhelp.h>
#include "common.h"
#include "stm.h"
#include "dio.h"
#include "data.h"
#include "file.h"
#include "scan.h"
#include "image.h"
#include "pre_scan.h"
#include "pal.h"
#include "clock.h"
#include "sel.h"
#include "dep.h"
#include "infodep.h"
#include "serial.h"
#include "track.h"
#include "spec.h"
#include <intrin.h>


//tracking variables Shaowei
int dep_track_offset_x = 0,dep_track_offset_y = 0;
int dep_track_limit_x_min,dep_track_limit_x_max;
int dep_track_limit_y_min,dep_track_limit_y_max;
int dep_track_max_bits = 200;
int dep_track_sample_every = 100;
int dep_track_at_min = 0;
int dep_tracking = 0;
int dep_tracking_mode = DEP_TRACK_MAX;
int dep_track_auto_auto = 0;
int dep_track_avg_data_pts = 200;
int dep_track_iterations = 10;
int dep_track_every = 0;
float dep_track_plane_a = 0.0;
float dep_track_plane_b = 0.0;
int dep_track_step_delay = 1000;
int dep_offset_hold = 0;
unsigned int dep_takeoff_x,dep_takeoff_y;
unsigned int dep_landing_x,dep_landing_y;

  

extern struct commentdef *gcomment;
extern int infodep_caller;

extern BOOL FAR PASCAL CommentDlg(HWND, unsigned, WPARAM, LPARAM);
BOOL FAR PASCAL DepDlg(HWND,unsigned,WPARAM,LPARAM);
BOOL FAR PASCAL DepPulseDlg(HWND,unsigned,WPARAM,LPARAM);
extern BOOL FAR PASCAL ImageDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL InfodepDlg(HWND,unsigned,WPARAM,LPARAM);

extern HWND ImgDlg;
extern HBITMAP  hCross;
extern HBITMAP  hXcross;

extern unsigned char *dacbox;
extern unsigned char *scan_dacbox;
extern unsigned int input_ch,out1;
extern unsigned int dac_data[];
extern int z_offset_range;
extern unsigned int scan_z;
extern datadef *gendata[];
extern int current_image;
extern BITMAPINFO *bitmapinfo;
extern int bitmap_size;
extern unsigned char *bitmap_data;
extern LOGPALETTE              *pLogPal;
extern HPALETTE                hPal;
extern HANDLE hInst;
extern char     string[];
/*
extern int zoomed_in;
*/
extern int im_src_x[],im_src_y[],im_screenlines[];
extern unsigned char *im_bitmaps[];
extern int sel1_on,sel2_on;
extern int last_button;
extern unsigned int sample_bias;
extern int sample_bias_range;
extern int z_offset_ch;
extern double a_per_v, z_a_per_v;


#ifdef OLD
extern int inter_step_delay;
extern float tip_spacing;
extern int digital_feedback;
extern int inter_line_delay;
extern float z_limit_percent;
#endif

extern unsigned int i_setpoint;
extern int i_set_range;
extern unsigned int    scan_step;
extern unsigned int    scan_size;
extern unsigned int tip_gain;
extern int scan_freq;
extern unsigned int scan_scale;
extern datadef *data;
extern datadef *scan_defaults_data[];
extern int scan_current_default;

extern int image_caller;
extern char 
    *current_dir_dep;
extern char 
    *initial_dir_dep;
extern char 
    *current_file_dep;
extern int file_dep_count;
extern unsigned int bit16;


int dep_pulse_measure_i = 0;
int dep_pulse_measure_z = 0;
int dep_pulse_measure_2 = 0;
int dep_pulse_measure_3 = 0;
int dep_pulse_duration_unit=DEP_PULSE_DURATION_MILLI;

int dep_trigger_mode= DEP_TRIGGER_NONE;//Triggering Shaowei
int dep_trigger_up = 1;
int dep_trigger_down = 0;
int dep_trigger_start = 0;
int dep_trigger_end = 0;
int dep_trigger_low = 2048;//Mod. 2 Shaowei default trigger low = 0V
int dep_trigger_high = 3072;//Mod. 2 Shaowei default trigger high = 5V
int dep_trigger_time = 50;//Mod. 2 Shaowei default trigger time 50ms
unsigned int dep_trigger_ch = 7;//Mod. 2 Shaowei, default trigger channel 7
unsigned int dep_trigger_now = 2048;//Mod. 2 Shaowei default trigger ch. at low


int dep_pulse_number=1;
int dep_inter_pulse_delay=500;//Shaowei Autopoking
int move_bias, write_bias;
int move_time, write_time, max_write_time;
unsigned int write_i_setpoint,move_i_setpoint;
float move_i_set,write_i_set,lines_max_i_set;
float dep_pulse_bias_speed=1.0;
float dep_pulse_i_speed=1.0;
float dep_pulse_z_offset_speed=1.0;
static int dep_first_time=1;
static out_smart=0;
int dep_mode=DEP_PULSE_MODE;
int dep_pulse_duration_mode=DEP_PULSE_DURATION_FIXED;
int dep_pulse_feedback=DEP_PULSE_FEEDBACK_ON;
int dep_pulse_feedback_after=0;
int dep_num_lines=5;
int dep_pulse_measure_before=0;
int dep_pulse_bias_step=0;
int dep_pulse_duration=30000;
int dep_pulse_z_offset=0;
int dep_pulse_z_offset_crash_protect=0;
int dep_pulse_z_after=DEP_PULSE_Z_AFTER_NONE;
int dep_pulse_z_offset_overshoot=0;
float dep_pulse_z_offset_overshoot_percentage=0;
int dep_current_channel=i_in_ch;
unsigned int lines_max_bias;
unsigned int lines_max_i_setpoint;
int dep_pulse_feedback_wait=100;
int dep_pulse_measure_i_wait=100;
int dep_pulse_measure_i_once=0;
int dep_pulse_use_move_bias=0;
int dep_pulse_freq;
int dep_measure_extra=0;
int dep_current_default=0;
int dep_pulse_current_step=0; 
static zoomed_in=0;
int dep_num = 1;
// these variables set the vertical scale (voltage, in bits) of the graphs
float dep_vert_max[DIO_INPUT_CHANNELS];
float dep_vert_min[DIO_INPUT_CHANNELS];
float dep_vert_max_max=DEP_VERT_MAX;
float dep_vert_min_min=DEP_VERT_MIN;

datadef *dep_oscill_data[DIO_INPUT_CHANNELS]; // I vs t, z vs t, ch2 vs t, ch3 vs t
datadef *dep_defaults_data[DEP_NUM_DEFAULTS];
extern datadef **glob_data;
int dep_pulse_ignore_initial=0,dep_pulse_change_time=1000,dep_pulse_avg_stop_pts=10,
    dep_pulse_after_time=100;
int dep_pulse_avg_pts=1;
float dep_pulse_percent_change=10;

int dep_delay=2;	//2 second delay after deposition pulse
time_t start,finish;

static void dep_set_title(HWND);

static HPEN hpen_black,hpen_red,hpen_green,hpen_blue,hpen_old,hpen_dash,hpen_white;

static void enable_all (HWND,int);
static void repaint_pulse_measure_i_wait(HWND);
static void repaint_pulse_feedback_wait(HWND);
static void repaint_pulse_z_offset(HWND);
static void repaint_pulse_freq_text(HWND);
static void repaint_pulse_number(HWND);
static void repaint_pulse_bias_speed(HWND);
static void repaint_pulse_duration(HWND);
static void repaint_inter_pulse_delay(HWND);//Mod. Shaowei Autopoking

//Tracking Shaowei
static void repaint_deptrack_max_bits (HWND);
static void recalc_deptrack_limits();
static void repaint_deptrack(HWND);
static void repaint_deptrack_current(HWND);
static void repaint_dep_planes(HWND);

//Trigger Shaowei
static void repaint_dep_trigger(HWND);
static void repaint_steptrigger_mode(HWND);
static void enable_trigger_options(HWND,int);
static void repaint_deptrigger_ch(HWND);
static void repaint_deptrigger_low(HWND);
static void repaint_deptrigger_high(HWND);
static void repaint_deptrigger_time(HWND);

void repaint_write_time(HWND);
void repaint_max_write_time(HWND);
void repaint_max_write_time(HWND);
void repaint_move_time(HWND);
void repaint_move_bias(HWND);
void repaint_write_bias(HWND);
void repaint_move_current(HWND);
void repaint_lines_bias(HWND);
void repaint_lines_current(HWND);
void repaint_write_current(HWND);
void dep_pulse3(HWND);
void dep_pulse_dsp(HWND);
void set_data(datadef *);
void get_data(datadef *);
void repaint_oscill_data(HWND, int);
void dep_box(HWND);
void dep_lines(HWND);
void set_write_cond(HWND, unsigned int, unsigned int);
void set_move_cond(HWND);
void set_scan_cond(HWND);
void repaint_all(HWND);
void repaint_images(HWND);
static void repaint_pulse_avg_pts(HWND);
static void repaint_vert_scale(HWND);
static void repaint_vert_scale_min(HWND);

BOOL FAR PASCAL DepDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{
    int i=0,j=0;
    int id=0;
    int delt=0,ddelt=0,mini=0,maxi=0;
    float max_z=0,min_z=0;
    float apex, nadir;
    double d=0,minid=0,maxid=0,deltd=0,ddeltd=0;
    static FARPROC  lpfnDlgProc;
    PAINTSTRUCT ps;
    char filename[256];
    POINT mouse_pos;
	int tracking_mode = TRACK_MAX;
    unsigned int cur_x, cur_y;

  switch(Message) {
    case WM_INITDIALOG:
        if (dep_first_time)
        {
            lines_max_bias=write_bias=move_bias=sample_bias;
            max_write_time=write_time = move_time = SD->inter_step_delay;
            dep_first_time=0;
            
            lines_max_i_setpoint=write_i_setpoint=move_i_setpoint=i_setpoint;
            for(i=0;i<DEP_NUM_DEFAULTS;i++) 
            {
                dep_defaults_data[i]=NULL;
                alloc_data(&(dep_defaults_data[i]),DATATYPE_2D,1,GEN2D_NONE,GEN2D_NONE,0);
                strcpy(filename,DEP_INI);
                sprintf(filename+strlen(filename),"%d.ini",i);
                glob_data=&dep_defaults_data[i];
                if (!load_dep(filename))
                {
                    set_data(dep_defaults_data[i]);
                    dep_defaults_data[i]->type2d=TYPE2D_PULSE_DEF;
                }
            }
            
            for(i=0;i<DIO_INPUT_CHANNELS;i++)
                dep_oscill_data[i]=NULL;
            switch(scan_freq)
            {
                case 0:
                    dep_pulse_freq=DEP_PULSE_FREQ_100;
                    break;
                case 1:
                    dep_pulse_freq=DEP_PULSE_FREQ_1;
                    break;
                case 2:
                    dep_pulse_freq=DEP_PULSE_FREQ_10;
                    break;
            }

	    // initially, all graphs go from -10 V to +10 V on vertical scale
	    for(i = 0; i < DIO_INPUT_CHANNELS; i++) dep_vert_min[i] = DEP_VERT_MIN;
	    for(i = 0; i < DIO_INPUT_CHANNELS; i++) dep_vert_max[i] = DEP_VERT_MAX;

        }
        get_data(dep_defaults_data[dep_current_default]);
        switch(dep_current_default)
        {
            case 1:
                CheckDlgButton(hDlg,DEP_PULSE_CRASH,1);
                break;
            case 2:
                CheckDlgButton(hDlg,DEP_PULSE_HIGH_V,1);
                break;
            case 3:
                CheckDlgButton(hDlg,DEP_PULSE_LOW_V,1);
                break;
            case 0:
                CheckDlgButton(hDlg,DEP_PULSE_OTHER,1);
                break;
        }   
        
        hpen_black=CreatePen(PS_SOLID,1,RGB(0,0,0));
        hpen_red=CreatePen(PS_SOLID,1,RGB(255,0,0));
        hpen_green=CreatePen(PS_SOLID,1,RGB(0,255,0));
        hpen_blue=CreatePen(PS_SOLID,1,RGB(0,0,255));
        hpen_black=CreatePen(PS_SOLID,1,RGB(0,0,0));
        hpen_white=CreatePen(PS_SOLID,1,RGB(255,255,255));
        hpen_dash=CreatePen(PS_DASH,1,RGB(0,0,0));
        lines_max_i_set=(float)calc_i_set(lines_max_i_setpoint,i_set_range,tip_gain);
        move_i_set=(float)calc_i_set(move_i_setpoint,i_set_range,tip_gain);
        write_i_set=(float)calc_i_set(write_i_setpoint,i_set_range,tip_gain);
        
        SetScrollRange(GetDlgItem(hDlg, DEP_MOV_TIME_SC), SB_CTL, DEP_TIME_MIN, 
            DEP_TIME_MAX, FALSE);
        
		SetScrollRange(GetDlgItem(hDlg, DEP_WRI_TIME_SC), SB_CTL, DEP_TIME_MIN, 
            DEP_TIME_MAX, FALSE);
        SetScrollRange(GetDlgItem(hDlg, DEP_MAX_WRI_TIME_SC), SB_CTL, 
            DEP_TIME_MIN, DEP_TIME_MAX, FALSE);
        SetScrollRange(GetDlgItem(hDlg, DEP_MOV_BIAS_SC), SB_CTL, 0, 
            MAX, FALSE);
        //SetScrollRange(GetDlgItem(hDlg, DEP_WRI_BIAS_SC), SB_CTL, 0, 
        //    MAX, FALSE);
		
		SetScrollRange(GetDlgItem(hDlg, DEP_WRI_BIAS_SC), SB_CTL, 0, 
            sample_bias_max, FALSE);
        SetScrollRange(GetDlgItem(hDlg, DEP_MOV_CURRENT_SCROLL), SB_CTL, 0, 
            MAX, FALSE);
        SetScrollRange(GetDlgItem(hDlg, DEP_WRI_CURRENT_SCROLL), SB_CTL, 0, 
            MAX, FALSE);
        SetScrollRange(GetDlgItem(hDlg, DEP_MAX_BIAS_SCROLL), SB_CTL, 0, 
            MAX, FALSE);
        SetScrollRange(GetDlgItem(hDlg, DEP_MAX_I_SCROLL), SB_CTL, 0, 
            MAX, FALSE);
        SetScrollRange(GetDlgItem(hDlg, DEP_V_SCALE_MAX_SCROLL), SB_CTL, 0, 
            32767, TRUE);
        SetScrollRange(GetDlgItem(hDlg, DEP_V_SCALE_MIN_SCROLL), SB_CTL, 0, 
            32767, TRUE);
		//Shaowei
        SetScrollRange(GetDlgItem(hDlg, DEP_PULSE_Z_SCROLL), SB_CTL, 0, 32767, TRUE);
		write_bias = min(write_bias,sample_bias_max);

        repaint_all(hDlg);        
/*
      sprintf(string,"%.2lf",gamma);
      SetDlgItemText(hDlg,GAMMA_VALUE,string);
      sprintf(string,"%d",anim_delay);
      SetDlgItemText(hDlg,ANIMATE_DELAY,string);
*/
      break;
    case WM_LBUTTONDOWN:
        mouse_pos.x = LOWORD(lParam);
        mouse_pos.y = HIWORD(lParam);
        mouse_pos.x-=SMALL_BORDER_SIZE;
        mouse_pos.y-=SMALL_BORDER_SIZE;
        if (!zoomed_in)
        {
            
            repaint_rect(hDlg,0,dep_current_channel,SMALL_BORDER_SIZE,SMALL_IMAGE_SIZE);
            if (mouse_pos.x<SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE && 
                mouse_pos.y<(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE*2)) 
            {
                dep_current_channel=0;
/*
                if (mouse_pos.y>SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE 
                    && mouse_pos.y<(SMALL_IMAGE_SIZE*2+SMALL_BORDER_SIZE*3)) 
                    dep_current_channel+=2;
*/
            }
            else if (mouse_pos.x<(SMALL_IMAGE_SIZE*2+SMALL_BORDER_SIZE*3) &&
                mouse_pos.y<(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE*2)) 
            {
                dep_current_channel=1;
/*
                if (mouse_pos.y>SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE 
                    && mouse_pos.y<(SMALL_IMAGE_SIZE*2+SMALL_BORDER_SIZE*3)) 
                    dep_current_channel+=2;
*/
            }
            repaint_rect(hDlg,1,dep_current_channel,SMALL_BORDER_SIZE,SMALL_IMAGE_SIZE);
	    repaint_vert_scale(hDlg);
	    repaint_vert_scale_min(hDlg);
            dep_set_title(hDlg);
        }
    case WM_PAINT:
        BeginPaint(hDlg,&ps);
/*
        if (!zoomed_in) 
        {
            repaint_rect(hDlg,1);
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
        repaint_pal(hDlg,PAL_X_POS2,PAL_Y_POS2);
*/
        repaint_images(hDlg);
/*
        repaint_oscill_data(hDlg,i_in_ch);
        repaint_oscill_data(hDlg,zi_ch);
*/
        EndPaint(hDlg,&ps);
      break;
    case WM_HSCROLL:
    case WM_VSCROLL:
      id = getscrollid();
      switch(id) {      
	case DEP_V_SCALE_MAX_SCROLL:
	    if( dep_current_channel == i_in_ch) {
                d = (double) dep_vert_max[i_in_ch];
                minid=dep_vert_min[i_in_ch]+abs((int)(dep_vert_max_max-dep_vert_min_min))/100;
                maxid=dep_vert_max_max;
                deltd=abs((int)(dep_vert_max_max-dep_vert_min_min))/100;
                ddeltd=abs((int)(dep_vert_max_max-dep_vert_min_min))/10;
	    }
	    else {
                d = (double) dep_vert_max[zi_ch];
                minid = dep_vert_min[zi_ch] + abs((int)(dep_vert_max_max - dep_vert_min_min)) / 100;
                maxid = dep_vert_max_max;
                deltd = abs((int)(dep_vert_max_max - dep_vert_min_min)) / 100;
                ddeltd = abs((int)(dep_vert_max_max - dep_vert_min_min)) / 10;
	    }
	    break;


//Shaowei

    case DEP_PULSE_Z_SCROLL:
                    d = (double)dep_pulse_z_offset;
                    minid = DEP_PULSE_Z_OFFSET_MIN;
                    maxid = DEP_PULSE_Z_OFFSET_MAX;
                    deltd = DEP_PULSE_Z_OFFSET_DELT;
                    ddeltd = DEP_PULSE_Z_OFFSET_DDELT;
       break;
//
	case DEP_V_SCALE_MIN_SCROLL:
	    if( dep_current_channel == i_in_ch) {
                d = (double) dep_vert_min[i_in_ch];
                minid=dep_vert_min_min;
                maxid=dep_vert_max[i_in_ch]-abs((int)(dep_vert_max_max-dep_vert_min_min))/100;
                deltd=abs((int)(dep_vert_max_max-dep_vert_min_min))/100;
                ddeltd=abs((int)(dep_vert_max_max-dep_vert_min_min))/10;
	    }
	    else {
                d = (double) dep_vert_min[zi_ch];
                minid = dep_vert_min_min;
                maxid = dep_vert_max[zi_ch] - abs((int)(dep_vert_max_max - dep_vert_min_min)) / 100;
                deltd = abs((int)(dep_vert_max_max - dep_vert_min_min)) / 100;
                ddeltd = abs((int)(dep_vert_max_max - dep_vert_min_min)) / 10;
	    }
	   break;
        case DEP_MOV_CURRENT_SCROLL:
            i = (int) tip_current_max-move_i_setpoint;
            mini = tip_current_min;
            maxi = tip_current_max;
            delt = tip_current_delt;
            ddelt = tip_current_ddelt;
            break;
        case DEP_MAX_I_SCROLL:
            i = (int) tip_current_max-lines_max_i_setpoint;
            mini = tip_current_min;
            maxi = tip_current_max;
            delt = tip_current_delt;
            ddelt = tip_current_ddelt;
            break;
        case DEP_WRI_CURRENT_SCROLL:
            i = (int) tip_current_max-write_i_setpoint;
            mini = tip_current_min;
            maxi = tip_current_max;
            delt = tip_current_delt;
            ddelt = tip_current_ddelt;
            break;
        case DEP_MOV_TIME_SC:
          i = (int) move_time;
		  mini = DEP_TIME_MIN;
          maxi = DEP_TIME_MAX;
     	  delt = DEP_TIME_DELT;
          ddelt = DEP_TIME_DDELT;
          break;
        case DEP_WRI_TIME_SC:
          i = (int) write_time;
          mini = DEP_TIME_MIN;
          maxi = DEP_TIME_MAX;
          delt = DEP_TIME_DELT;
          ddelt = DEP_TIME_DDELT;
          break;
        case DEP_MAX_WRI_TIME_SC:
          i = (int) max_write_time;
          mini = DEP_TIME_MIN;
          maxi = DEP_TIME_MAX;
          delt = DEP_TIME_DELT;
          ddelt = DEP_TIME_DDELT;
          break;
        case DEP_MOV_BIAS_SC:
          i = move_bias;
          mini = DEP_BIAS_MIN;
          maxi = DEP_BIAS_MAX;
          delt = DEP_BIAS_DELT;
          ddelt = DEP_BIAS_DDELT;
          break;
        case DEP_WRI_BIAS_SC:
          i = write_bias;
          mini = DEP_BIAS_MIN;
          //maxi = DEP_BIAS_MAX;
		  maxi = sample_bias_max; //takes into account the bit 16
          delt = DEP_BIAS_DELT;
          ddelt = DEP_BIAS_DDELT;
          break;
        case DEP_MAX_BIAS_SCROLL:
          i = lines_max_bias;
          mini = DEP_BIAS_MIN;
          maxi = DEP_BIAS_MAX;
          delt = DEP_BIAS_DELT;
          ddelt = DEP_BIAS_DDELT;
          break;
      }
        if (!out_smart) switch (getscrollcode())
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
        default:
            id = -1;
            break;
        }
        i = min(max(i, mini), maxi);
        d = min(max(d, minid), maxid);
      switch(id)  {
	case DEP_V_SCALE_MAX_SCROLL:
	    if( dep_current_channel == i_in_ch) {
                dep_vert_max[i_in_ch] =(float) d;
	    }
	    else {
                dep_vert_max[zi_ch] = (float)d;
	    }
            repaint_vert_scale(hDlg);
	    repaint_oscill_data(hDlg, dep_current_channel);
	    break;
	case DEP_V_SCALE_MIN_SCROLL:
	    if( dep_current_channel == i_in_ch) {
                dep_vert_min[i_in_ch] = (float)d;
	    }
	    else {
                dep_vert_min[zi_ch] = (float)d;
	    }
            repaint_vert_scale_min(hDlg);
	    repaint_oscill_data(hDlg, dep_current_channel);
	    break;
        case DEP_MAX_I_SCROLL:
            lines_max_i_setpoint = (unsigned int) (tip_current_max-i);
            lines_max_i_set=(float) calc_i_set(lines_max_i_setpoint,i_set_range,tip_gain);
            repaint_lines_current(hDlg);            
            break;
        case DEP_MOV_CURRENT_SCROLL:
            move_i_setpoint = (unsigned int) (tip_current_max-i);
            move_i_set=(float)calc_i_set(move_i_setpoint,i_set_range,tip_gain);
            repaint_move_current(hDlg);            
            break;
        case DEP_WRI_CURRENT_SCROLL:
            write_i_setpoint = (unsigned int) (tip_current_max-i);
            write_i_set=(float) calc_i_set(write_i_setpoint,i_set_range,tip_gain);
            repaint_write_current(hDlg);            
            break;
        case DEP_MOV_TIME_SC:
          move_time = (int) i;
          repaint_move_time(hDlg);
          break;
        case DEP_WRI_TIME_SC:
          write_time = (int) i;
          repaint_write_time(hDlg);
          break;
        case DEP_MAX_WRI_TIME_SC:
          max_write_time = (int) i;
          repaint_max_write_time(hDlg);
          break;
        case DEP_MOV_BIAS_SC:
          move_bias = (int) i;
          repaint_move_bias(hDlg);
          break;
        case DEP_MAX_BIAS_SCROLL:
          lines_max_bias = (int) i;
          repaint_lines_bias(hDlg);
          break;
        case DEP_WRI_BIAS_SC:
          write_bias = (int) i;
          repaint_write_bias(hDlg);
          break;
     //Shaowei
        case DEP_PULSE_Z_SCROLL:
           dep_pulse_z_offset = (int)d;
          repaint_pulse_z_offset(hDlg);
          break;
	// Shaowei
      }
      break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
            case DEP_INFO:
	      if(dep_oscill_data[dep_current_channel]!=NULL)
	      {
                if(dep_oscill_data[dep_current_channel]->type==DATATYPE_2D)
        	{
		    if( dep_oscill_data[dep_current_channel]->valid) {
		      infodep_caller = INFODEP_CALLER_DEP;
                      lpfnDlgProc = MakeProcInstance(InfodepDlg, hInst);
                      DialogBox(hInst, "INFODEPDLG", hDlg, lpfnDlgProc);
                      FreeProcInstance(lpfnDlgProc);
		    }
        	}
	      }

/* code to export data
                fp=fopen("c:\\dep.dat","wb");
                for(i=0;i<dep_oscill_data[0]->size;i++)
                {
                    fprintf(fp,"%f %f\n",*(dep_oscill_data[0]->time2d+i),
                    in_dtov(*(dep_oscill_data[0]->data2d+i)));
                }
                fclose(fp);
*/

              break; 
            case DEP_1_4_IMAGES:
                zoomed_in=!zoomed_in;
		clear_area(hDlg,3,3,12+LARGE_IMAGE_SIZE,12+LARGE_IMAGE_SIZE,RGB(255,255,255));
                repaint_images(hDlg);
                break;
            case DEP_PULSE_CRASH:
                dep_current_default=1;
                CheckDlgButton(hDlg,dep_mode,0);
                dep_mode=DEP_PULSE_MODE;
                get_data(dep_defaults_data[dep_current_default]);
                repaint_all(hDlg);
                break;
            case DEP_PULSE_HIGH_V:
                dep_current_default=2;
                CheckDlgButton(hDlg,dep_mode,0);
                dep_mode=DEP_PULSE_MODE;
                get_data(dep_defaults_data[dep_current_default]);
                repaint_all(hDlg);
                break;
            case DEP_PULSE_LOW_V:
                dep_current_default=3;
                CheckDlgButton(hDlg,dep_mode,0);
                dep_mode=DEP_PULSE_MODE;
                get_data(dep_defaults_data[dep_current_default]);
                repaint_all(hDlg);
                break;
            case DEP_PULSE_OTHER:
                dep_current_default=0;
                CheckDlgButton(hDlg,dep_mode,0);
                dep_mode=DEP_PULSE_MODE;
                get_data(dep_defaults_data[dep_current_default]);
                repaint_all(hDlg);
                break;
            case DEP_COMMENT:
				if(dep_oscill_data[dep_current_channel] != NULL)
				{
        			if( dep_oscill_data[dep_current_channel]->valid)
					{
						gcomment=&(dep_oscill_data[dep_current_channel]->comment);
						lpfnDlgProc = MakeProcInstance(CommentDlg, hInst);
						DialogBox(hInst, "COMMENTDLG", hDlg, lpfnDlgProc);
						FreeProcInstance(lpfnDlgProc);
					}
				}
				break;
            case DEP_LOAD:
                free_data(&(dep_oscill_data[dep_current_channel]));
                alloc_data(&(dep_oscill_data[dep_current_channel]),DATATYPE_2D,1,GEN2D_NONE,GEN2D_NONE,0);
                glob_data=&dep_oscill_data[dep_current_channel];
                file_open(hDlg,hInst,FTYPE_DEP,0,current_file_dep);
                find_min_max(dep_oscill_data[dep_current_channel],&min_z,&max_z);
		if( dep_current_channel == i_in_ch) {
		  dep_vert_min[i_in_ch] = DEP_VERT_MIN;
		  dep_vert_max[i_in_ch] = DEP_VERT_MAX;
		}
		else {
		  dep_vert_min[zi_ch] = DEP_VERT_MIN;
		  dep_vert_max[zi_ch] = DEP_VERT_MAX;
		}
		repaint_vert_scale(hDlg);
		repaint_vert_scale_min(hDlg);
                repaint_oscill_data(hDlg,dep_current_channel);
                dep_set_title(hDlg);
                break;      
            case DEP_SAVE:
                if (dep_oscill_data[dep_current_channel]!=NULL)
                {
                    if (dep_oscill_data[dep_current_channel]->valid)
                    {
                        glob_data=&dep_oscill_data[dep_current_channel];
                        strcpy(data->dep_filename,current_file_dep);
                        if (file_save_as(hDlg,hInst,FTYPE_DEP))
                        {
                            strcpy(data->dep_filename,current_file_dep);
//                            strcpy(data2->dep_filename,current_file_dep);
                    
                            inc_file(current_file_dep,&file_dep_count);
                        }
                    }
                }
                dep_set_title(hDlg);
                break;

	    case DEP_V_SCALE_MAX:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg, DEP_V_SCALE_MAX, string, 9);
                    if ( dep_current_channel == i_in_ch) {
                      dep_vert_max[i_in_ch] = (float) in_vtod(atof(string));
                      dep_vert_max[i_in_ch] = min(max(dep_vert_max[i_in_ch],
                         dep_vert_min[i_in_ch]+abs((int)(dep_vert_max_max-dep_vert_min_min))/100),
                         dep_vert_max_max);
                      SetScrollPos(GetDlgItem(hDlg, DEP_V_SCALE_MAX_SCROLL), 
                         SB_CTL,DTOI(dep_vert_max[i_in_ch],dep_vert_min_min,dep_vert_max_max) , 
                         TRUE);
		    }
		    else {
		      dep_vert_max[zi_ch] = (float) in_vtod(atof(string));
                      dep_vert_max[zi_ch] = min(max(dep_vert_max[zi_ch],
                         dep_vert_min[zi_ch] + abs((int)(dep_vert_max_max - dep_vert_min_min)) / 100),
                         dep_vert_max_max);
                      SetScrollPos(GetDlgItem(hDlg, DEP_V_SCALE_MAX_SCROLL), 
                         SB_CTL,DTOI(dep_vert_max[zi_ch],dep_vert_min_min,dep_vert_max_max) , 
                         TRUE);
		    }
		    repaint_oscill_data( hDlg, dep_current_channel);
                    out_smart=0;
                }
		break;
	    case DEP_V_SCALE_MIN:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg, DEP_V_SCALE_MIN, string, 9);
                    if (dep_current_channel == i_in_ch) {
		      dep_vert_min[i_in_ch] = (float)in_vtod(atof(string));
                      dep_vert_min[i_in_ch] = min(max(dep_vert_min[i_in_ch],dep_vert_min_min),
                         dep_vert_max[i_in_ch]-abs((int)(dep_vert_max_max-dep_vert_min_min))/100);
                      SetScrollPos(GetDlgItem(hDlg, DEP_V_SCALE_MIN_SCROLL), 
                         SB_CTL,DTOI(dep_vert_min[i_in_ch],dep_vert_min_min,dep_vert_max_max) , TRUE);
		    }
		    else {
		      dep_vert_min[zi_ch] =(float) in_vtod(atof(string));
                      dep_vert_min[zi_ch] = min(max(dep_vert_min[zi_ch],dep_vert_min_min),
                         dep_vert_max[zi_ch]-abs((int)(dep_vert_max_max-dep_vert_min_min))/100);
                      SetScrollPos(GetDlgItem(hDlg, DEP_V_SCALE_MIN_SCROLL), 
                         SB_CTL,DTOI(dep_vert_min[zi_ch],dep_vert_min_min,dep_vert_max_max) , TRUE);
		    }
		    repaint_oscill_data( hDlg, dep_current_channel);
                    out_smart=0;
                }
		break;
	    case DEP_ZOOM_MAX:
		if( dep_oscill_data[dep_current_channel]!=NULL) {

		  apex = (float)((dep_oscill_data[dep_current_channel])->data2d[0]);

		  for( j=1; j<dep_oscill_data[dep_current_channel]->size; j++) {
		    if( (dep_oscill_data[dep_current_channel])->data2d[j] > apex) {
			apex = (dep_oscill_data[dep_current_channel])->data2d[j];
		    }
		  }

		  if(dep_current_channel == i_in_ch) dep_vert_max[i_in_ch] = apex;
		  if(dep_current_channel == zi_ch) dep_vert_max[zi_ch] = apex;

		  repaint_vert_scale(hDlg);
		  repaint_oscill_data(hDlg, dep_current_channel);		
		}
		break;
	    case DEP_ZOOM_MIN:
		if( dep_oscill_data[dep_current_channel]!=NULL) {

		  nadir = (float)((dep_oscill_data[dep_current_channel])->data2d[0]);

		  for( j=1; j<dep_oscill_data[dep_current_channel]->size; j++) {
		    if( (dep_oscill_data[dep_current_channel])->data2d[j] < nadir) {
			nadir = (dep_oscill_data[dep_current_channel])->data2d[j];
		    }
		  }

		  if(dep_current_channel == i_in_ch) dep_vert_min[i_in_ch] = nadir;
		  if(dep_current_channel == zi_ch) dep_vert_min[zi_ch] = nadir;

		  repaint_vert_scale_min(hDlg);
		  repaint_oscill_data(hDlg, dep_current_channel);		
		}
		break;
	    case DEP_ZOOM_CENTER:
		if( dep_oscill_data[dep_current_channel]!=NULL) {

		  apex = (float)((dep_oscill_data[dep_current_channel])->data2d[0]);

		  for( j=1; j<dep_oscill_data[dep_current_channel]->size; j++) {
		    if( (dep_oscill_data[dep_current_channel])->data2d[j] > apex) {
			apex = (dep_oscill_data[dep_current_channel])->data2d[j];
		    }
		  }

		  nadir = (float)((dep_oscill_data[dep_current_channel])->data2d[0]);

		  for( j=1; j<dep_oscill_data[dep_current_channel]->size; j++) {
		    if( (dep_oscill_data[dep_current_channel])->data2d[j] < nadir) {
			nadir = (dep_oscill_data[dep_current_channel])->data2d[j];
		    }
		  }

		  if( dep_current_channel == i_in_ch) {
		    if( apex > IN_ZERO) {
			dep_vert_max[i_in_ch] = apex;
		    }
		    else {
			dep_vert_max[i_in_ch] = IN_ZERO;
		    }
		    if( nadir < IN_ZERO) {
			dep_vert_min[i_in_ch] = nadir;
		    }
		    else {
			dep_vert_min[i_in_ch] = IN_ZERO;
		    }
		    if( dep_vert_max[i_in_ch]-IN_ZERO > IN_ZERO-dep_vert_min[i_in_ch]) {
			dep_vert_min[i_in_ch] = IN_ZERO - (dep_vert_max[i_in_ch]-IN_ZERO);
		    }
		    else {
			dep_vert_max[i_in_ch] = IN_ZERO + (IN_ZERO-dep_vert_min[i_in_ch]);
		    }

		  }

		  if( dep_current_channel == zi_ch) {
		    if( apex > IN_ZERO) {
			dep_vert_max[zi_ch] = apex;
		    }
		    else {
			dep_vert_max[zi_ch] = IN_ZERO;
		    }
		    if( nadir < IN_ZERO) {
			dep_vert_min[zi_ch] = nadir;
		    }
		    else {
			dep_vert_min[zi_ch] = IN_ZERO;
		    }
		    if( dep_vert_max[zi_ch]-IN_ZERO > IN_ZERO-dep_vert_min[zi_ch]) {
			dep_vert_min[zi_ch] = IN_ZERO - (dep_vert_max[zi_ch]-IN_ZERO);
		    }
		    else {
			dep_vert_max[zi_ch] = IN_ZERO + (IN_ZERO-dep_vert_min[zi_ch]);
		    }

		  }

		  repaint_vert_scale(hDlg);
		  repaint_vert_scale_min(hDlg);
		  repaint_oscill_data(hDlg, dep_current_channel);		
		}
		break;

            case DEP_IMAGE_MANIPULATION:
                image_caller=IMAGE_CALLER_DEP;
                glob_data=&dep_oscill_data[dep_current_channel];
                lpfnDlgProc = MakeProcInstance(ImageDlg, hInst);
                DialogBox(hInst, "IMAGEDLG", hDlg, lpfnDlgProc);
                FreeProcInstance(lpfnDlgProc);
//                dacbox=scan_dacbox;
//                spindac(0,1,scan_dacbox);
                break;
            case DEP_PULSE_OPTIONS:
                lpfnDlgProc = MakeProcInstance(DepPulseDlg, hInst);
                DialogBox(hInst, "DEPPULSEDLG", hDlg, lpfnDlgProc);
                FreeProcInstance(lpfnDlgProc);
                break;
                
            case DEP_NUM_LINES:
                GetDlgItemText(hDlg,DEP_NUM_LINES,string,5);
                dep_num_lines=min(max(1,atoi(string)),scan_size);
                break;
            
            case DEP_BOX_MODE:
            case DEP_LINES_MODE:
            case DEP_PULSE_MODE:
                dep_mode=LOWORD(wParam);
                break;
			//Triggering Shaowei
			case DEP_TRIGGER_TOHIGH://Trigger to high
			if(dep_trigger_mode!=DEP_TRIGGER_NONE)
			{
				dep_trigger_now = ramp_serial(dep_trigger_ch,dep_trigger_now,
		                                 dep_trigger_high,10,0);
			}
				else
			{
				MessageBox(hDlg, "Deposition trigger is off", "Trigger Error", MB_OK);
			}
				break;
		
			case DEP_TRIGGER_TOLOW: //Trigger to low
				if(dep_trigger_mode!=DEP_TRIGGER_NONE)
			{
				 dep_trigger_now = ramp_serial(dep_trigger_ch,dep_trigger_now,
		                                 dep_trigger_low,10,0);
			}
				else
			{
				MessageBox(hDlg, "Deposition trigger is off", "Trigger Error", MB_OK);
			}
				break;
			case DEP_STOP:
				out_smart = 1;
				break;
            case DEP_DOIT:



			//.Shaowei Mod. Autopoking
            if(dep_pulse_number != 1)
			{
                sprintf(string,"You are doing repeated pulses, continue?");
                if(MessageBox(hDlg,string, 
                    "Warning",MB_OKCANCEL | MB_ICONEXCLAMATION) == IDCANCEL)
                break;

			}
				enable_all(hDlg,FALSE);
			//.Shaowei Mod. Triggering	

			switch(dep_trigger_mode)
			{
			case DEP_TRIGGER_STEP:
				if(dep_trigger_up)
				dep_trigger_now = ramp_serial(dep_trigger_ch,dep_trigger_now,
		                                 dep_trigger_high,10,0);
				else
				dep_trigger_now = ramp_serial(dep_trigger_ch,dep_trigger_now,
		                                 dep_trigger_low,10,0);
				break;
				
			case DEP_TRIGGER_PULSE:
				if(dep_trigger_start)
				{
				dep_trigger_now = ramp_serial(dep_trigger_ch,dep_trigger_now,
		                                 dep_trigger_high,10,0);
				Sleep(dep_trigger_time);
				dep_trigger_now = ramp_serial(dep_trigger_ch,dep_trigger_now,
		                                 dep_trigger_low,10,0);
				}
				break;

			}

			for(j=0;j<dep_pulse_number;j++)
			{   CheckMsgQ();
				if(out_smart)
				{
					j=dep_pulse_number;
					out_smart=0;
                   
				}
                
		

		
				//tracking Shaowei
            if(dep_track_every) 
            {
			  if((!(j  % dep_track_every) && j != dep_pulse_number )||j==0)
			  {
				if(!out_smart)
				{
					SetDlgItemText(hDlg,DEP_STATUS,"Tracking...");
		  
				// "undo" x, y offsets
					if(dep_track_offset_x) 
					{
						i = (int)dac_data[x_ch] - dep_track_offset_x;
						if(i < 0) i = 0;
						if(i > MAX) i = MAX;
						dac_data[x_ch] = ramp_serial(x_ch,dac_data[x_ch],i,SD->step_delay,0);
					}
					if(dep_track_offset_y) 
					{
						i = (int)dac_data[y_ch] - dep_track_offset_y;
						if(i < 0) i = 0;
						if(i > MAX) i = MAX;
						dac_data[y_ch] = ramp_serial(y_ch,dac_data[y_ch],i,SD->step_delay,0);
					}

			//	Do the tracking
					track_setup(dep_track_avg_data_pts,dep_track_step_delay,
					dep_track_step_delay,dep_track_plane_a,dep_track_plane_b);
					cur_x = dac_data[x_ch];
					cur_y = dac_data[y_ch];
					if(dep_tracking_mode == DEP_TRACK_MAX) tracking_mode = TRACK_MAX;
					else tracking_mode = TRACK_MIN;
					track_start(cur_x,cur_y,tracking_mode);
					for(i = 0;i < dep_track_iterations && dep_tracking;i++)
					{
						cur_x = dac_data[x_ch];
						cur_y = dac_data[y_ch];
	
						if(cur_x > dep_track_limit_x_max ||
						cur_x < dep_track_limit_x_min ||
						cur_y > dep_track_limit_y_max||
						cur_y < dep_track_limit_y_min)
						dep_tracking = 0;
	 					else track_again();
					}
					track_terminate();
					dep_landing_x = dac_data[x_ch];
					dep_landing_y = dac_data[y_ch];

		  // restore x,y offsets
					if(dep_track_offset_x) 
		    		{
						i = (int)dac_data[x_ch] + dep_track_offset_x;
						if(i < 0) i = 0;
						if(i > MAX) i = MAX;
						dac_data[x_ch] = ramp_serial(x_ch,dac_data[x_ch],i,SD->step_delay,0);
					}
					if(dep_track_offset_y)
					{
						i = (int)dac_data[y_ch] + dep_track_offset_y;
						if(i < 0) i = 0;
						if(i > MAX) i = MAX;
						dac_data[y_ch] = ramp_serial(y_ch,dac_data[y_ch],i,SD->step_delay,0);
					}
				}		// end of tracking business.
			  SetDlgItemText(hDlg,DEP_STATUS,"");
			  }
			}
				//end of tracking shaowei
	

			if(dep_pulse_number != 1 && j!=dep_pulse_number)
				{
				SetDlgItemText(hDlg,DEP_STATUS1,"Pulse #.");
                sprintf(string,"%d",j+1);
				SetDlgItemText(hDlg,DEP_STATUS2,string);
				SetDlgItemText(hDlg,DEP_STATUS3, "of");
				sprintf(string,"%d",dep_pulse_number);
				SetDlgItemText(hDlg,DEP_STATUS4,string);
				}
				
/*
				switch(dep_mode)
                {
*/
/*
					case DEP_BOX_MODE:
						dep_box(hDlg);
                        break;
                    case DEP_LINES_MODE:

                        if (dep_num_lines>1 && 
                            write_i_set/lines_max_i_set!=
                                max_write_time/write_time)
                            MessageBox(hDlg,"Can't do geometric progression!","Warning",MB_OK);

                        dep_lines(hDlg);
                        break;
                    case DEP_PULSE_MODE:
*/                        
						
			(dep_defaults_data[dep_current_default]);

						strcpy(filename,DEP_INI);
                        sprintf(filename+strlen(filename),"%d.ini",
                            dep_current_default);
                        glob_data=&dep_defaults_data[dep_current_default];
                        save_dep(filename);
						if( dep_pulse_measure_i) 
						{
						  dep_vert_min[i_in_ch] = DEP_VERT_MIN;
						  dep_vert_max[i_in_ch] = DEP_VERT_MAX;
						}
						if( dep_pulse_measure_z) 
						{
						  dep_vert_min[zi_ch] = DEP_VERT_MIN;
						  dep_vert_max[zi_ch] = DEP_VERT_MAX;
						}
						repaint_vert_scale( hDlg);
						repaint_vert_scale_min( hDlg);
                        //dep_pulse3(hDlg);
						dep_pulse_dsp(hDlg);
						if (dep_pulse_measure_i) repaint_oscill_data(hDlg,i_in_ch);
                        if (dep_pulse_measure_z) repaint_oscill_data(hDlg,zi_ch);

						dep_set_title(hDlg);
				        
						if(dep_pulse_z_after == DEP_PULSE_Z_AFTER_AUTO) 
						{  	
							time( &start );//Shaowei wait for DSP
							while ( finish - start < dep_delay )  time( &finish );	
							auto_z_below(IN_ZERO);
						}
											
						CheckMsgQ();
						if(dep_pulse_number != 1) Sleep(dep_inter_pulse_delay);
						CheckMsgQ();

						//autominus if the box is checked -Shaowei

					
				}

			    //Tip retract if the box is checked -Shaowei
                if(dep_pulse_z_after == DEP_PULSE_Z_AFTER_BACK)  
				{  	
					time( &start );//Shaowei wait for DSP
					while ( finish - start < dep_delay )  time( &finish );	
					retract_serial(TRUE);
				}
				
			    //Repeated pulse-Shaowei
				if(dep_pulse_number != 1)
				{
			    SetDlgItemText(hDlg,DEP_STATUS1,"");
                SetDlgItemText(hDlg,DEP_STATUS2,"");
				SetDlgItemText(hDlg,DEP_STATUS3, "");
				SetDlgItemText(hDlg,DEP_STATUS4,"");
			    }


				//Triggering Shaowei

			switch(dep_trigger_mode)
			{
			case DEP_TRIGGER_STEP:
				if(dep_trigger_down)
				dep_trigger_now = ramp_serial(dep_trigger_ch,dep_trigger_now,
		                                 dep_trigger_high,10,0);
				else
				dep_trigger_now = ramp_serial(dep_trigger_ch,dep_trigger_now,
		                                 dep_trigger_low,10,0);
				break;
				
			case DEP_TRIGGER_PULSE:
				if(dep_trigger_end)
				{
				dep_trigger_now = ramp_serial(dep_trigger_ch,dep_trigger_now,
		                                 dep_trigger_high,10,0);
				Sleep(dep_trigger_time);
				dep_trigger_now = ramp_serial(dep_trigger_ch,dep_trigger_now,
		                                 dep_trigger_low,10,0);
				}
				break;

			}
				enable_all(hDlg,TRUE);
                break;

/*                
            case DEP_BOX:
                dep_box(hDlg);
                                    
                break;                                           
*/
            case DEP_MAX_I:
                if (!out_smart)
                {
                    GetDlgItemText(hDlg, DEP_MAX_I, string, 9);
                    lines_max_i_set=(float)atof(string);
                    lines_max_i_setpoint=calc_i_setpoint(lines_max_i_set,i_set_range,tip_gain);
                    lines_max_i_set=(float)calc_i_set(lines_max_i_setpoint,i_set_range,tip_gain);

                    out_smart=1;
                    SetScrollPos(GetDlgItem(hDlg, DEP_MAX_I_SCROLL), SB_CTL, 
                        (int) tip_current_max-lines_max_i_setpoint, TRUE);
                    out_smart=0;
                }
                break;
            case DEP_MOV_CURRENT:
                if (!out_smart)
                {
                    GetDlgItemText(hDlg, DEP_MOV_CURRENT, string, 9);
                    move_i_set=(float)atof(string);
                    move_i_setpoint=calc_i_setpoint(move_i_set,i_set_range,tip_gain);
                    move_i_set=(float)calc_i_set(move_i_setpoint,i_set_range,tip_gain);

                    out_smart=1;
                    SetScrollPos(GetDlgItem(hDlg, DEP_MOV_CURRENT_SCROLL), SB_CTL, 
                        (int) tip_current_max-move_i_setpoint, TRUE);
                    out_smart=0;
                }
                break;
            case DEP_WRI_CURRENT:
                if (!out_smart)
                {
                    GetDlgItemText(hDlg, DEP_WRI_CURRENT, string, 9);
                    write_i_set=(float)atof(string);
                    write_i_setpoint=calc_i_setpoint(write_i_set,i_set_range,tip_gain);
                    write_i_set=(float)calc_i_set(write_i_setpoint,i_set_range,tip_gain);

                    out_smart=1;
                    SetScrollPos(GetDlgItem(hDlg, DEP_WRI_CURRENT_SCROLL), SB_CTL, 
                        (int) tip_current_max-write_i_setpoint, TRUE);
                    out_smart=0;
                }
                break;
                    
            case DEP_MOV_TIME:
                if (!out_smart)
                {
                    GetDlgItemText(hDlg, DEP_MOV_TIME, string, 9);
                    move_time=min(max(DEP_TIME_MIN,atoi(string)),DEP_TIME_MAX);
                    out_smart=1;
                    SetScrollPos(GetDlgItem(hDlg,DEP_MOV_TIME_SC),SB_CTL,move_time,TRUE);
                    out_smart=0;
                    
                }
                break;
            case DEP_WRI_TIME:
                if (!out_smart)
                {
                    GetDlgItemText(hDlg, DEP_WRI_TIME, string, 9);
                    write_time=min(max(DEP_TIME_MIN,atoi(string)),DEP_TIME_MAX);
                    out_smart=1;
                    SetScrollPos(GetDlgItem(hDlg,DEP_WRI_TIME_SC),SB_CTL,write_time,TRUE);
                    out_smart=0;
                    
                }
                break;
            case DEP_MAX_WRI_TIME:
                if (!out_smart)
                {
                    GetDlgItemText(hDlg, DEP_MAX_WRI_TIME, string, 9);
                    max_write_time=min(max(DEP_TIME_MIN,atoi(string)),
                        DEP_TIME_MAX);
                    out_smart=1;
                    SetScrollPos(GetDlgItem(hDlg,DEP_MAX_WRI_TIME_SC),
                        SB_CTL,max_write_time,TRUE);
                    out_smart=0;
                    
                }
                break;
            case DEP_MOV_BIAS:
                if (!out_smart)
                {
                    GetDlgItemText(hDlg, DEP_MOV_BIAS, string, 9);
                    i = vtod(atof(string),sample_bias_range);
                    i = min(max(i, DEP_BIAS_MIN), DEP_BIAS_MAX);
                    move_bias=i;
                    out_smart=1;
                    SetScrollPos(GetDlgItem(hDlg,DEP_MOV_BIAS_SC),SB_CTL,move_bias,TRUE);
                    out_smart=0;
                    
                }
                break;
            case DEP_MAX_BIAS:
                if (!out_smart)
                {
                    GetDlgItemText(hDlg, DEP_MAX_BIAS, string, 9);
                    i = vtod(atof(string),sample_bias_range);
                    i = min(max(i, DEP_BIAS_MIN), DEP_BIAS_MAX);
                    lines_max_bias=i;
                    out_smart=1;
                    SetScrollPos(GetDlgItem(hDlg,DEP_MAX_BIAS_SCROLL),SB_CTL,lines_max_bias,TRUE);
                    out_smart=0;
                    
                }
                break;
            case DEP_WRI_BIAS:
                if (!out_smart)
                {
                    GetDlgItemText(hDlg, DEP_WRI_BIAS, string, 9);
                    i = vtod_bias(atof(string),sample_bias_range);
                    i = min(max(i, DEP_BIAS_MIN), DEP_BIAS_MAX);
                    write_bias=i;
                    out_smart=1;
                    SetScrollPos(GetDlgItem(hDlg,DEP_WRI_BIAS_SC),SB_CTL,write_bias,TRUE);
                    out_smart=0;
                    
                }
                break;
//Mod. Shaowei delta z offset/ auto-
			case DEP_PULSE_Z_BITS:
                if(!out_smart) {
                    GetDlgItemText(hDlg, DEP_PULSE_Z_BITS, string, 5);
                    i = atoi(string);
                    i = min(max(i, DEP_PULSE_Z_OFFSET_MIN), DEP_PULSE_Z_OFFSET_MAX);
                    dep_pulse_z_offset = i;
                }
                break;

		
//Mod. Auto-
            case DEP_OFFSET_AUTO_BELOW:
                 auto_z_below(IN_ZERO);
		    break;
// end auto-

 // Mod. Shaowei
            case ENTER:

                repaint_move_time(hDlg);
                repaint_write_time(hDlg);
                repaint_move_bias(hDlg);
                repaint_max_write_time(hDlg);
                repaint_write_bias(hDlg);
                repaint_write_current(hDlg);
                repaint_move_current(hDlg);
                repaint_lines_bias(hDlg);
                repaint_lines_current(hDlg);
				repaint_pulse_z_offset(hDlg);
				repaint_vert_scale(hDlg);
		        repaint_vert_scale_min(hDlg);
                sprintf(string,"%d",dep_num_lines);
                wprint(DEP_NUM_LINES);
                break;
                
            case DEP_EXIT:
                set_data(dep_defaults_data[dep_current_default]);
                strcpy(filename,DEP_INI);
                sprintf(filename+strlen(filename),"%d.ini",
                    dep_current_default);
                glob_data=&dep_defaults_data[dep_current_default];
                save_dep(filename);
                DeleteObject(hpen_black);
                DeleteObject(hpen_white);
                DeleteObject(hpen_red);
                DeleteObject(hpen_green);
                DeleteObject(hpen_blue);
                DeleteObject(hpen_dash);
                EndDialog(hDlg,TRUE);
                return(TRUE);
                break;
    }
    break;
  }
  return(FALSE);
}

BOOL FAR PASCAL DepPulseDlg(HWND hDlg, unsigned Message, WPARAM wParam, LPARAM lParam)
{
  // pulse options dialog

  int i=0;
  int id=0;
  int delt=0, ddelt=0, mini=0, maxi=0;
  double d=0, minid=0, maxid=0, deltd=0, ddeltd=0;
    
  switch(Message) {
    case WM_INITDIALOG:
        if (dep_pulse_duration < 8000) dep_pulse_duration_unit = DEP_PULSE_DURATION_MICRO;
        CheckDlgButton(hDlg, DEP_PULSE_MEASURE_I, dep_pulse_measure_i);
        CheckDlgButton(hDlg, DEP_PULSE_MEASURE_Z, dep_pulse_measure_z);
        CheckDlgButton(hDlg, DEP_PULSE_MEASURE_2, dep_pulse_measure_2);
        CheckDlgButton(hDlg, DEP_PULSE_MEASURE_3, dep_pulse_measure_3);
        CheckDlgButton(hDlg, DEP_PULSE_BIAS_STEP, dep_pulse_bias_step);
        CheckDlgButton(hDlg, dep_pulse_duration_mode, 1);
        CheckDlgButton(hDlg, dep_pulse_duration_unit, 1);
        CheckDlgButton(hDlg, dep_pulse_feedback, 1);
        CheckDlgButton(hDlg, DEP_PULSE_FEEDBACK_AFTER, dep_pulse_feedback_after);
        CheckDlgButton(hDlg, DEP_PULSE_Z_AFTER_NONE, 0);
        CheckDlgButton(hDlg, DEP_PULSE_Z_AFTER_AUTO, 0);
        CheckDlgButton(hDlg, DEP_PULSE_Z_AFTER_BACK, 0);
	    CheckDlgButton(hDlg, dep_pulse_z_after, 1);
		CheckDlgButton(hDlg, dep_trigger_mode, 1);
        CheckDlgButton(hDlg, dep_pulse_freq, 1);
        CheckDlgButton(hDlg, DEP_PULSE_MEASURE_I_ONCE, dep_pulse_measure_i_once);
        CheckDlgButton(hDlg, DEP_PULSE_USE_MOVE_BIAS, dep_pulse_use_move_bias);
        SetScrollRange(GetDlgItem(hDlg, DEP_PULSE_BIAS_SPEED_SCROLL), SB_CTL, 0, 32767, TRUE);
        SetScrollRange(GetDlgItem(hDlg, DEP_PULSE_Z_SCROLL), SB_CTL, 0, 32767, TRUE);
		SetScrollRange(GetDlgItem(hDlg,DEP_TRACK_MAX_BITS_SCROLL),SB_CTL,0,MAX / 2,FALSE);
	    SetScrollRange(GetDlgItem(hDlg,DEP_TRIGGER_CH_SCROLL),SB_CTL,0,32767,TRUE);
		SetScrollRange(GetDlgItem(hDlg,DEP_TRIGGER_LOW_SCROLL),SB_CTL,0,32767,TRUE);
		SetScrollRange(GetDlgItem(hDlg,DEP_TRIGGER_HIGH_SCROLL),SB_CTL,0,32767,TRUE);
        repaint_pulse_avg_pts(hDlg);
        repaint_pulse_freq_text(hDlg);        
        repaint_pulse_z_offset(hDlg);
        repaint_pulse_bias_speed(hDlg);
        //repaint_pulse_i_speed(hDlg);
        repaint_pulse_number(hDlg);
        repaint_inter_pulse_delay(hDlg);//autopoking
		repaint_pulse_duration(hDlg);
        repaint_pulse_measure_i_wait(hDlg);
        repaint_pulse_feedback_wait(hDlg);
		repaint_deptrack(hDlg);
		repaint_dep_trigger(hDlg);
        break;
    case WM_HSCROLL:
    case WM_VSCROLL:
        if(!out_smart) {
            out_smart = 1;
            id = getscrollid();
            switch(id) {      
                case DEP_PULSE_BIAS_SPEED_SCROLL:
                    d = (double)dep_pulse_bias_speed;
                    minid = DEP_PULSE_BIAS_SPEED_MIN;
                    maxid = DEP_PULSE_BIAS_SPEED_MAX;
                    deltd = DEP_PULSE_BIAS_SPEED_DELT;
                    ddeltd = DEP_PULSE_BIAS_SPEED_DDELT;
                    break;
                /*case DEP_PULSE_Z_SCROLL:
                    d = (double)dep_pulse_z_offset;
                    minid = DEP_PULSE_Z_OFFSET_MIN;
                    maxid = DEP_PULSE_Z_OFFSET_MAX;
                    deltd = DEP_PULSE_Z_OFFSET_DELT;
                    ddeltd = DEP_PULSE_Z_OFFSET_DDELT;
                    break;
				*/
				case DEP_TRACK_MAX_BITS_SCROLL: //tracking shaowei
                    i = dep_track_max_bits;
                    mini = 0;
                    maxi = MAX / 2;
                    delt = TRACK_MAX_BITS_DELT;
                    ddelt = TRACK_MAX_BITS_DDELT;
                    break;

				//triggering shaowei
				case DEP_TRIGGER_CH_SCROLL:
                    d = (double)dep_trigger_ch;
                    minid = SP_TRIGGER_CH_MIN;
                    maxid = SP_TRIGGER_CH_MAX;
                    deltd = SP_TRIGGER_CH_DELT;
                    ddeltd = SP_Z_OFFSET_DDELT;
                    break;
			    case DEP_TRIGGER_LOW_SCROLL:
                    d = (double)dep_trigger_low;
                    minid = SP_MIN_MIN;
                    maxid = dep_trigger_high - 1;
                    deltd = SP_SCAN_DELT;
                    ddeltd = SP_SCAN_DDELT;
                    break;
                case DEP_TRIGGER_HIGH_SCROLL:
                    d = (double)dep_trigger_high;
                    minid = dep_trigger_low + 1;
					if(bit16 && (dep_trigger_ch == 10 || dep_trigger_ch == 11))
                      maxid = SP_MAX_MAX * 16; // high resolution
					else
                      maxid = SP_MAX_MAX;
                    deltd = SP_SCAN_DELT;
                    ddeltd = SP_SCAN_DDELT; 
                    break;
				//End Triggering Shaowei

            }
            switch(getscrollcode()) {
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
                    d = ITOD(getscrollpos(), minid, maxid);
                    break;
                case SB_THUMBTRACK:
                    i = getscrollpos();
                    d = ITOD(getscrollpos(), minid, maxid);
                    break;
                default:
                    id = -1;
                    break;
            }
            i = min(max(i, mini), maxi);
            d = min(max(d, minid), maxid);
            switch(id) {
                case DEP_PULSE_BIAS_SPEED_SCROLL:
                    dep_pulse_bias_speed = (float)d;
                    repaint_pulse_bias_speed(hDlg);
                    break;
                /*case DEP_PULSE_Z_SCROLL:
                    dep_pulse_z_offset = (int)d;
                    repaint_pulse_z_offset(hDlg);
                    break;
					*/
				case DEP_TRACK_MAX_BITS_SCROLL: //tracking shaowei
                    dep_track_max_bits = i;
                    repaint_deptrack_max_bits(hDlg);
                    break;
				//triggering shaowei
				case DEP_TRIGGER_CH_SCROLL:
                    dep_trigger_ch = (int)d;
                    repaint_deptrigger_ch(hDlg);
                    break;
				case DEP_TRIGGER_LOW_SCROLL:
                    dep_trigger_low = (int)floor(d + 0.5);
                    repaint_deptrigger_low(hDlg);
                    break;
                case DEP_TRIGGER_HIGH_SCROLL:
                    dep_trigger_high = (int)floor(d + 0.5);
                    repaint_deptrigger_high(hDlg);
                    break;//End Triggering Shaowei
            }
            out_smart = 0;
        }
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam)) {
            case DEP_PULSE_IGNORE_INITIAL:
                if(!out_smart) {
                    GetDlgItemText(hDlg, DEP_PULSE_IGNORE_INITIAL, string, 10);
                    dep_pulse_ignore_initial = min(max(DEP_TIME_PER_SAMPLE, atoi(string)),
                        dep_pulse_duration - DEP_PULSE_I_SAMPLES * DEP_TIME_PER_SAMPLE);
                }
                break;
            case DEP_PULSE_CHANGE_TIME:
                if(!out_smart) {
                    GetDlgItemText(hDlg, DEP_PULSE_CHANGE_TIME, string, 10);
                    dep_pulse_change_time = min(max(0, atoi(string)),
                        dep_pulse_duration - DEP_PULSE_I_SAMPLES * DEP_TIME_PER_SAMPLE);
                }
                break;
            case DEP_PULSE_AVG_STOP_PTS:
                if(!out_smart) {
                    GetDlgItemText(hDlg, DEP_PULSE_AVG_STOP_PTS, string, 10);
                    dep_pulse_avg_stop_pts = min(max(1, atoi(string)), 1000);
                }
                break;
            case DEP_PULSE_AFTER_TIME:
                if(!out_smart) {
                    GetDlgItemText(hDlg, DEP_PULSE_AFTER_TIME, string, 10);
                    dep_pulse_after_time = min(max(0, atoi(string)),
                        dep_pulse_duration - DEP_PULSE_I_SAMPLES * DEP_TIME_PER_SAMPLE -
			dep_pulse_ignore_initial - dep_pulse_change_time);
                }
                break;
            case DEP_PULSE_AVG_PTS:
                if(!out_smart) {
                    GetDlgItemText(hDlg, DEP_PULSE_AVG_PTS, string, 10);
                    //dep_pulse_avg_pts = min(max(1, atoi(string)), 1000);
					dep_pulse_avg_pts = min(max(1, atoi(string)), 256); //modified by SW
                }
                break;
            case DEP_PULSE_PERCENT_CHANGE:
                if(!out_smart)
                {
                    GetDlgItemText(hDlg, DEP_PULSE_PERCENT_CHANGE, string, 10);
                    dep_pulse_percent_change = (float)atof(string); 
/*
                    dep_pulse_percent_change = min(max(DEP_PULSE_PERCENT_CHANGE_MIN,
                        atof(string)), DEP_PULSE_PERCENT_CHANGE_MAX);
*/
                }
                break;
            case DEP_PULSE_MEASURE_I_WAIT:
                if(!out_smart) {
                    GetDlgItemText(hDlg, DEP_PULSE_MEASURE_I_WAIT, string, 10);
                    dep_pulse_measure_i_wait = min(max(DEP_PULSE_MEASURE_I_WAIT_MIN, atoi(string)),
                        dep_pulse_duration - DEP_PULSE_I_SAMPLES * DEP_TIME_PER_SAMPLE);
                }
                break;
            case DEP_PULSE_FEEDBACK_WAIT:
                if(!out_smart) {
                    GetDlgItemText(hDlg, DEP_PULSE_FEEDBACK_WAIT, string, 10);
                    dep_pulse_feedback_wait = min(max(DEP_PULSE_FEEDBACK_WAIT_MIN,
                        atoi(string)), dep_pulse_duration);
                }
                break;
            case DEP_PULSE_Z_BITS:
                if(!out_smart) {
                    GetDlgItemText(hDlg, DEP_PULSE_Z_BITS, string, 5);
                    i = atoi(string);
                    i = min(max(i, DEP_PULSE_Z_OFFSET_MIN), DEP_PULSE_Z_OFFSET_MAX);
                    dep_pulse_z_offset = i;
                }
                break;
            case DEP_PULSE_BIAS_SPEED:
                if(!out_smart) {
                    GetDlgItemText(hDlg, DEP_PULSE_BIAS_SPEED, string, 6);
                    dep_pulse_bias_speed = (float)min(max(DEP_PULSE_BIAS_SPEED_MIN,
                        atof(string)), DEP_PULSE_BIAS_SPEED_MAX);
                    out_smart = 1;
                    SetScrollPos(GetDlgItem(hDlg, DEP_PULSE_BIAS_SPEED_SCROLL), SB_CTL, 
                        DTOI(dep_pulse_bias_speed, DEP_PULSE_BIAS_SPEED_MIN,
                        DEP_PULSE_BIAS_SPEED_MAX), TRUE);
                    out_smart = 0;
                }
                break;
            case DEP_PULSE_DURATION:
                if(!out_smart) {
                    GetDlgItemText(hDlg, DEP_PULSE_DURATION, string, 10);
                    dep_pulse_duration = min(max(DEP_PULSE_DURATION_MIN, atoi(string)), DEP_PULSE_DURATION_MAX);
                    if (dep_pulse_duration_unit == DEP_PULSE_DURATION_MILLI)
                        dep_pulse_duration *= 1000;
                    out_smart = 1;
//                    SetScrollPos(GetDlgItem(hDlg,DEP_MOV_TIME_SC),SB_CTL,move_time,TRUE);
                    out_smart = 0;
                }
                break;
            case DEP_PULSE_BIAS_STEP:
                dep_pulse_bias_step = IsDlgButtonChecked(hDlg, DEP_PULSE_BIAS_STEP);
                break;
            case DEP_PULSE_MEASURE_I:
                dep_pulse_measure_i = IsDlgButtonChecked(hDlg, DEP_PULSE_MEASURE_I);
                break;
            case DEP_PULSE_MEASURE_Z:
                dep_pulse_measure_z = IsDlgButtonChecked(hDlg, DEP_PULSE_MEASURE_Z);
                break;
            case DEP_PULSE_MEASURE_2:
                dep_pulse_measure_z = IsDlgButtonChecked(hDlg, DEP_PULSE_MEASURE_2);
                break;
            case DEP_PULSE_MEASURE_3:
                dep_pulse_measure_z = IsDlgButtonChecked(hDlg, DEP_PULSE_MEASURE_3);
                break;

            case DEP_PULSE_MEASURE_I_ONCE:
                dep_pulse_measure_i_once=IsDlgButtonChecked(hDlg,DEP_PULSE_MEASURE_I_ONCE);
                break;
            case DEP_PULSE_USE_MOVE_BIAS:
                dep_pulse_use_move_bias=IsDlgButtonChecked(hDlg,DEP_PULSE_USE_MOVE_BIAS);
                break;
            case DEP_PULSE_FEEDBACK_ON:
            case DEP_PULSE_FEEDBACK_OFF:
                dep_pulse_feedback=LOWORD(wParam);
                break;
            case DEP_PULSE_Z_AFTER_NONE:
            case DEP_PULSE_Z_AFTER_AUTO:
            case DEP_PULSE_Z_AFTER_BACK:
                dep_pulse_z_after=LOWORD(wParam);
                break;
            case DEP_PULSE_FEEDBACK_AFTER:
                dep_pulse_feedback_after=IsDlgButtonChecked(hDlg,DEP_PULSE_FEEDBACK_AFTER);
                break;
            case DEP_PULSE_FREQ_100:
            case DEP_PULSE_FREQ_1:
            case DEP_PULSE_FREQ_10:
                dep_pulse_freq=LOWORD(wParam);
                repaint_pulse_freq_text(hDlg);
                break;
            case DEP_PULSE_DURATION_FIXED:
                dep_pulse_duration_mode=LOWORD(wParam);
                break;    
            case DEP_PULSE_DURATION_Z:
                dep_pulse_duration_mode=LOWORD(wParam);
                CheckDlgButton(hDlg,DEP_PULSE_MEASURE_Z,1);
                dep_pulse_measure_z=1;
                break;    
            case DEP_PULSE_DURATION_I:
                dep_pulse_duration_mode=LOWORD(wParam);
                CheckDlgButton(hDlg,DEP_PULSE_MEASURE_I,1);
                dep_pulse_measure_i=1;
                break;    
            case DEP_PULSE_DURATION_MICRO:
            case DEP_PULSE_DURATION_MILLI:
                dep_pulse_duration_unit=LOWORD(wParam);
                repaint_pulse_duration(hDlg);
                break;
			//Triggering Shaowei
			case DEP_TRIGGER_NONE:
            case DEP_TRIGGER_STEP:
            case DEP_TRIGGER_PULSE:
                dep_trigger_mode=LOWORD(wParam);
				enable_trigger_options(hDlg, dep_trigger_mode);
                break;
			
			case DEP_TRIGGER_UP://Mod. 2 Shaowei
            	dep_trigger_up = IsDlgButtonChecked(hDlg,LOWORD(wParam));
				dep_trigger_down = !dep_trigger_up;
				repaint_steptrigger_mode(hDlg);
            	break;

			case DEP_TRIGGER_DOWN://Mod. 2 Shaowei
            	dep_trigger_down = IsDlgButtonChecked(hDlg,LOWORD(wParam));
				dep_trigger_up = !dep_trigger_down;
				repaint_steptrigger_mode(hDlg);
            	break;

			case DEP_TRIGGER_START://Mod. 2 Shaowei
            	dep_trigger_start = IsDlgButtonChecked(hDlg,LOWORD(wParam));
				break;
			
			case DEP_TRIGGER_END://Mod. 2 Shaowei
            	dep_trigger_end = IsDlgButtonChecked(hDlg,LOWORD(wParam));
				break;
			case DEP_TRIGGER_CH:
				if(!out_smart)
				{
					out_smart = 1;
					GetDlgItemText(hDlg,DEP_TRIGGER_CH,string,9);
					dep_trigger_ch = (unsigned int)atoi(string);
					dep_trigger_ch = min(max(dep_trigger_ch,SP_TRIGGER_CH_MIN),SP_TRIGGER_CH_MAX);
					SetScrollPos(GetDlgItem(hDlg,DEP_TRIGGER_CH_SCROLL),SB_CTL, 
						DTOI(dep_trigger_ch,SP_TRIGGER_CH_MIN,SP_TRIGGER_CH_MAX),TRUE);
					out_smart = 0;
				}
				break;
				
			case DEP_TRIGGER_HIGH:
				if(!out_smart)
				{
					out_smart = 1;
					GetDlgItemText(hDlg,DEP_TRIGGER_HIGH,string,9);
					if(bit16 && (dep_trigger_ch == 10 || dep_trigger_ch == 11))
					{
 					  dep_trigger_high = (unsigned int)vtod16(atof(string));
					  dep_trigger_high = min(max(dep_trigger_high,dep_trigger_low + 1),SP_MAX_MAX * 16);
					  SetScrollPos(GetDlgItem(hDlg,SP_TRIGGER_HIGH_SCROLL),SB_CTL,
						DTOI(dep_trigger_high,SP_MAX_MIN,SP_MAX_MAX * 16),TRUE);
					}
					else
					{
 					  dep_trigger_high = (unsigned int)vtod(atof(string),get_range(dep_trigger_ch));
					  dep_trigger_high = min(max(dep_trigger_high,dep_trigger_low + 1),SP_MAX_MAX);
					  SetScrollPos(GetDlgItem(hDlg,DEP_TRIGGER_HIGH_SCROLL),SB_CTL,
						DTOI(dep_trigger_high,SP_MAX_MIN,SP_MAX_MAX),TRUE);
					}
					
					out_smart = 0;
				}
				break;              
				
				case DEP_TRIGGER_LOW:
				if(!out_smart)
				{
					out_smart = 1;
					GetDlgItemText(hDlg,DEP_TRIGGER_LOW,string,9);
					if(bit16 && (dep_trigger_ch == 10 || dep_trigger_ch == 11))
					{
					  dep_trigger_low = (unsigned int)vtod16(atof(string));
					  
					  dep_trigger_low = min(max(dep_trigger_low,SP_MIN_MIN),dep_trigger_high - 1);
					  SetScrollPos(GetDlgItem(hDlg,DEP_TRIGGER_LOW_SCROLL),SB_CTL,
						DTOI(dep_trigger_low,SP_MIN_MIN,SP_MIN_MAX * 16),TRUE);
                    }
					else
					{
					  dep_trigger_low = (unsigned int)vtod(atof(string),get_range(dep_trigger_ch));
					  dep_trigger_low = min(max(dep_trigger_low,SP_MIN_MIN),dep_trigger_high - 1);
					  SetScrollPos(GetDlgItem(hDlg,SP_TRIGGER_LOW_SCROLL),SB_CTL,
						DTOI(dep_trigger_low,SP_MIN_MIN,SP_MIN_MAX),TRUE);
					}
					out_smart = 0;
				    
				}
				break;
				
			case DEP_TRIGGER_TIME://Mod. 2 Shaowei
				if(!out_smart)
				{
					out_smart = 1;
					GetDlgItemText(hDlg,DEP_TRIGGER_TIME,string,9);
                dep_trigger_time = atoi(string);
                dep_trigger_time = min(max(dep_trigger_time,SP_MIN_MIN),SP_SPEED_MAX);//0 to 10000000
                out_smart = 0;
				}
				break;//end Shaowei
				//
			//End Triggering Shaowei


			//Shaowei Autopoking
			case DEP_PULSE_NUMBER:
			    if(!out_smart) {
                    GetDlgItemText(hDlg, DEP_PULSE_NUMBER, string, 5);
                    dep_pulse_number = max (atoi(string),1);}
				break;

			case DEP_INTER_PULSE_DELAY:
			    if(!out_smart) {
                    GetDlgItemText(hDlg, DEP_INTER_PULSE_DELAY, string, 9);
                    dep_inter_pulse_delay = max (atoi(string),0);}
				break;
            
            //tracking shaowei
			case DEP_TRACK_MAX_BITS:
                if(!out_smart)
                {
                    out_smart = 1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,9);
                    dep_track_max_bits = atoi(string);
                    if(dep_track_max_bits < 0) 
                    {
                        dep_track_max_bits = 0;
                        SetDlgItemText(hDlg,LOWORD(wParam),"0");
                    }
                    if(dep_track_max_bits > MAX / 2)
                    {
                        dep_track_max_bits = MAX / 2;
                        sprintf(string,"%d",dep_track_max_bits);
                        SetDlgItemText(hDlg,LOWORD(wParam),string);
                    }
                    out_smart = 0;
//                    SendMessage(ScnDlg,WM_COMMAND,SCAN_REPAINT_SP_TRACK_LIMITS,0);
                    recalc_deptrack_limits();
//                    SendMessage(ScnDlg,WM_COMMAND,SCAN_REPAINT_SP_TRACK_LIMITS,0);
//                    repaint_track_limits(hDlg);
                    
                }
                break;
			
			case DEP_TRACK_SAMPLE_EVERY:
                if(!out_smart)
                {
                    out_smart = 1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,9);
                    dep_track_sample_every = atoi(string);
                    if(dep_track_sample_every < 0) 
                    {
                        dep_track_sample_every = 0;
                        SetDlgItemText(hDlg,LOWORD(wParam),"0");
                    }
                    out_smart = 0;
                }
                break;
            
            case DEP_TRACK_AVG_DATA_PTS:
                if(!out_smart)
                {
                    out_smart = 1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,9);
                    dep_track_avg_data_pts = atoi(string);
                    if(dep_track_avg_data_pts < 1) 
                    {
                        dep_track_avg_data_pts = 1;
                        SetDlgItemText(hDlg,LOWORD(wParam),"1");
                    }
                    out_smart = 0;
                }
                break;
			
			case DEP_TRACK_MAX:
            case DEP_TRACK_MIN:
                dep_tracking_mode = LOWORD(wParam);
                break;
			
			case DEP_TRACK_ITERATIONS:
                if(!out_smart)
                {
                    out_smart = 1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,9);
                    dep_track_iterations = atoi(string);
                    if(dep_track_avg_data_pts < 0) 
                    {
                        dep_track_sample_every = 0;
                        SetDlgItemText(hDlg,LOWORD(wParam),"0");
                    }
                    out_smart = 0;
                }
                break;
			
			case DEP_TRACK_EVERY:
                if(!out_smart)
                {
                    out_smart = 1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,9);
                    dep_track_every = atoi(string);
                    if(dep_track_every < 0) 
                    {
                        dep_track_every = 0;
                        SetDlgItemText(hDlg,LOWORD(wParam),"0");
                    }
                    out_smart = 0;
                }
                break;
			
			case DEP_TRACK_OFFSET_X:
                if(!out_smart)
                {
                    out_smart = 1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,9);
                    dep_track_offset_x = atoi(string);
                    if(dep_track_offset_x < -MAX) 
                    {
                        dep_track_offset_x = -MAX;
                        sprintf(string,"%d",dep_track_offset_x);
                        SetDlgItemText(hDlg,LOWORD(wParam),string);
                    }
                    if (dep_track_offset_x > MAX) 
                    {
                        dep_track_offset_x = MAX;
                        sprintf(string,"%d",dep_track_offset_x);
                        SetDlgItemText(hDlg,LOWORD(wParam),string);
                    }
                    out_smart = 0;
                }
                break;
            case DEP_TRACK_OFFSET_Y:
                if(!out_smart)
                {
                    out_smart = 1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,9);
                    dep_track_offset_y = atoi(string);
                    if(dep_track_offset_y < -MAX) 
                    {
                        dep_track_offset_y = -MAX;
                        sprintf(string,"%d",dep_track_offset_y);
                        SetDlgItemText(hDlg,LOWORD(wParam),string);
                    }
                    if (dep_track_offset_y > MAX) 
                    {
                        dep_track_offset_y = MAX;
                        sprintf(string,"%d",dep_track_offset_y);
                        SetDlgItemText(hDlg,LOWORD(wParam),string);
                    }
                    out_smart = 0;
                }
                break;
		   
			case DEP_TRACK_STEP_DELAY:
                if(!out_smart)
                {
                    out_smart = 1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,9);
                    dep_track_step_delay = atoi(string);
                    if(dep_track_step_delay < 0) 
                    {
                        dep_track_step_delay = 0;
                        SetDlgItemText(hDlg,LOWORD(wParam),"0");
                    }
                    out_smart = 0;
                }
                break;

			 case DEP_TRACK_PLANE_A:
                if(!out_smart)
                {
                    out_smart = 1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,9);
                    dep_track_plane_a = (float)atof(string);
                    out_smart = 0;
                }
                break;
            case DEP_TRACK_PLANE_B:
                if(!out_smart)
                {
                    out_smart = 1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,9);
                    dep_track_plane_b = (float)atof(string);
                    out_smart = 0;
                }
                break;
            case DEP_TRACK_AT_MIN:
            	dep_track_at_min = IsDlgButtonChecked(hDlg,LOWORD(wParam));
            	break;
			case DEP_OFFSET_HOLD_ON:
            	dep_offset_hold = IsDlgButtonChecked(hDlg,LOWORD(wParam));
            	break;

            case ENTER:
                repaint_pulse_measure_i_wait(hDlg);
                repaint_pulse_feedback_wait(hDlg);
                repaint_pulse_z_offset(hDlg);
                repaint_pulse_bias_speed(hDlg);
                repaint_pulse_duration(hDlg);
				repaint_pulse_number(hDlg);
				repaint_inter_pulse_delay(hDlg);//autopoking
				repaint_deptrack(hDlg);
				repaint_dep_trigger(hDlg);
                break;
                
            case DEP_PULSE_EXIT:
/*
                PostMessage(ImgDlg,WM_COMMAND,IM_SEL_EXIT,0);
*/
          
                EndDialog(hDlg,TRUE);
				return(TRUE);
                break;
    }
    break;
  }

  return(FALSE);
}

static void enable_all(HWND hDlg,int status)//Shaowei Autopoking
{

    EnableWindow(GetDlgItem(hDlg, DEP_STOP), !status);
    EnableWindow(GetDlgItem(hDlg, DEP_OFFSET_AUTO_BELOW), status);
    EnableWindow(GetDlgItem(hDlg, DEP_DOIT), status);
    EnableWindow(GetDlgItem(hDlg, DEP_COMMENT), status);
    EnableWindow(GetDlgItem(hDlg, DEP_LOAD), status);
    EnableWindow(GetDlgItem(hDlg, DEP_INFO), status);
    EnableWindow(GetDlgItem(hDlg, DEP_SAVE), status);
	EnableWindow(GetDlgItem(hDlg, DEP_EXIT), status);

}

//tracking Shaowei
static void repaint_deptrack(HWND hDlg)
{
    out_smart = 1;
    CheckDlgButton(hDlg,dep_tracking_mode,1);
    CheckDlgButton(hDlg,DEP_TRACK_AT_MIN,dep_track_at_min);
    CheckDlgButton(hDlg,DEP_OFFSET_HOLD_ON,dep_offset_hold);
	sprintf(string,"%d",dep_track_offset_x);
    SetDlgItemText(hDlg,DEP_TRACK_OFFSET_X,string);
    sprintf(string,"%d",dep_track_offset_y);
    SetDlgItemText(hDlg,DEP_TRACK_OFFSET_Y,string);
    sprintf(string,"%d",dep_track_step_delay);
    SetDlgItemText(hDlg,DEP_TRACK_STEP_DELAY,string);
    sprintf(string,"%d",dep_track_sample_every);
    SetDlgItemText(hDlg,DEP_TRACK_SAMPLE_EVERY,string);
    sprintf(string,"%d",dep_track_avg_data_pts);
    SetDlgItemText(hDlg,DEP_TRACK_AVG_DATA_PTS,string);
    sprintf(string,"%d",dep_track_iterations);
    SetDlgItemText(hDlg,DEP_TRACK_ITERATIONS,string);
    sprintf(string,"%d",dep_track_every);
    SetDlgItemText(hDlg,DEP_TRACK_EVERY,string);
    SetScrollPos(GetDlgItem(hDlg,DEP_TRACK_MAX_BITS_SCROLL),SB_CTL,dep_track_max_bits,TRUE);
    repaint_dep_planes(hDlg);
    out_smart = 0;
    
//    repaint_current_pos(hDlg);
    repaint_deptrack_max_bits(hDlg);
    repaint_deptrack_current(hDlg);
}    

static void repaint_dep_planes(HWND hDlg)
{
    sprintf(string,"%.4f",dep_track_plane_a);
    SetDlgItemText(hDlg,DEP_TRACK_PLANE_A,string);
    sprintf(string,"%.4f",dep_track_plane_b);
    SetDlgItemText(hDlg,DEP_TRACK_PLANE_B,string);
}

static void repaint_deptrack_current(HWND hDlg)
{
    sprintf(string,"%d",dac_data[x_ch]);
    SetDlgItemText(hDlg,DEP_TRACK_CURRENT_X,string);
    sprintf(string,"%d",dac_data[y_ch]);
    SetDlgItemText(hDlg,DEP_TRACK_CURRENT_Y,string);
}   
static void repaint_deptrack_max_bits(HWND hDlg) 
{
    out_smart = 1;    
    SetScrollPos(GetDlgItem(hDlg,DEP_TRACK_MAX_BITS_SCROLL),SB_CTL,dep_track_max_bits,TRUE);
    recalc_deptrack_limits();
    sprintf(string,"%d",dep_track_max_bits);
    SetDlgItemText(hDlg,DEP_TRACK_MAX_BITS,string);
//    repaint_track_limits(hDlg);
    out_smart = 0;
}

static void recalc_deptrack_limits()
{
  int current_x = dac_data[x_ch];
  int current_y = dac_data[y_ch];
    
  dep_track_limit_x_max = current_x + dep_track_max_bits;
  if(dep_track_limit_x_max > MAX) dep_track_limit_x_max = MAX;
  dep_track_limit_x_min = ((int)current_x) - dep_track_max_bits;
  if(dep_track_limit_x_min < 0) dep_track_limit_x_min = 0;
  dep_track_limit_y_max = current_y + dep_track_max_bits;
  if(dep_track_limit_y_max > MAX) dep_track_limit_y_max = MAX;
  dep_track_limit_y_min = ((int)current_y) - dep_track_max_bits;
  if(dep_track_limit_y_min < 0) dep_track_limit_y_min = 0;
}

       

static void repaint_pulse_measure_i_wait(HWND hDlg)
{
    out_smart=1;
    sprintf(string,"%d",dep_pulse_measure_i_wait);
    SetDlgItemText(hDlg,DEP_PULSE_MEASURE_I_WAIT,string);
    out_smart=0;
}

static void repaint_pulse_feedback_wait(HWND hDlg)
{
    out_smart=1;
    sprintf(string,"%d",dep_pulse_feedback_wait);
    SetDlgItemText(hDlg,DEP_PULSE_FEEDBACK_WAIT,string);
    out_smart=0;
}

static void repaint_pulse_z_offset(HWND hDlg)
{
    out_smart=1;
    sprintf(string,"%d",dep_pulse_z_offset);
    SetDlgItemText(hDlg,DEP_PULSE_Z_BITS,string);
    sprintf(string,"%0.3f",dtov_len(dep_pulse_z_offset,z_offset_range));
    SetDlgItemText(hDlg, DEP_PULSE_Z_VOLTS,string);
    sprintf(string,"%0.3f",dtov_len(dep_pulse_z_offset,z_offset_range)*
        (double) Z_A_PER_V);
    SetDlgItemText(hDlg, DEP_PULSE_Z_ANGS,string);
    SetScrollPos(GetDlgItem(hDlg, DEP_PULSE_Z_SCROLL), SB_CTL,
        DTOI(dep_pulse_z_offset,DEP_PULSE_Z_OFFSET_MIN, DEP_PULSE_Z_OFFSET_MAX), TRUE);
    
    out_smart=0;
}

static void repaint_pulse_freq_text(HWND hDlg)
{
    SetDlgItemText(hDlg, DEP_PULSE_FREQ_LOW, scan_freq_str(string,scan_scale,0));
    SetDlgItemText(hDlg, DEP_PULSE_FREQ_MED, scan_freq_str(string,scan_scale,1));
    SetDlgItemText(hDlg, DEP_PULSE_FREQ_HIGH, scan_freq_str(string,scan_scale,2));
}
//Triggering Shaowei
static void repaint_dep_trigger(HWND hDlg)
{
enable_trigger_options(hDlg, dep_trigger_mode);
repaint_steptrigger_mode(hDlg);
repaint_deptrigger_ch(hDlg);
repaint_deptrigger_low(hDlg);
repaint_deptrigger_high(hDlg);
repaint_deptrigger_time(hDlg);
}

static void repaint_deptrigger_time(HWND hDlg)
{
    out_smart = 1;
    sprintf(string,"%d",dep_trigger_time);
    SetDlgItemText(hDlg,DEP_TRIGGER_TIME,string);
    out_smart = 0;
}

static void repaint_deptrigger_low(HWND hDlg)
{
    out_smart = 1;
	if(bit16 && (dep_trigger_ch == 10 || dep_trigger_ch == 11))
	{
      SetScrollPos(GetDlgItem(hDlg, DEP_TRIGGER_LOW_SCROLL),SB_CTL,
		DTOI(dep_trigger_low,SP_MIN_MIN,SP_MIN_MAX * 16),TRUE);
      sprintf(string,"%0.4f",dtov16(dep_trigger_low));
    }
	else
	{
      SetScrollPos(GetDlgItem(hDlg, DEP_TRIGGER_LOW_SCROLL),SB_CTL,
		DTOI(dep_trigger_low,SP_MIN_MIN,SP_MIN_MAX),TRUE);
      sprintf(string,"%0.3f",dtov(dep_trigger_low,get_range(dep_trigger_ch)));
	}
    
	SetDlgItemText(hDlg, DEP_TRIGGER_LOW, string);
    out_smart=0;
}


static void repaint_deptrigger_high(HWND hDlg)//Mod. 2 Shaowei repaint trigger high level
{
    out_smart = 1;
	if(bit16 && (dep_trigger_ch == 10 || dep_trigger_ch == 11))
	{
      SetScrollPos(GetDlgItem(hDlg, DEP_TRIGGER_HIGH_SCROLL),SB_CTL,
		DTOI(dep_trigger_high,SP_MAX_MIN,SP_MAX_MAX * 16),TRUE);
      sprintf(string,"%0.4f",dtov16(dep_trigger_high));
    }
	else
	{
      SetScrollPos(GetDlgItem(hDlg, DEP_TRIGGER_HIGH_SCROLL),SB_CTL,
		DTOI(dep_trigger_high,SP_MAX_MIN,SP_MAX_MAX),TRUE);
      sprintf(string,"%0.3f",dtov(dep_trigger_high,get_range(dep_trigger_ch)));
	}
	SetDlgItemText(hDlg,DEP_TRIGGER_HIGH,string);
    out_smart = 0;
}//End

static void repaint_steptrigger_mode(HWND hDlg)
{
CheckDlgButton(hDlg, DEP_TRIGGER_UP, dep_trigger_up);
CheckDlgButton(hDlg, DEP_TRIGGER_DOWN, dep_trigger_down);
CheckDlgButton(hDlg, DEP_TRIGGER_START, dep_trigger_start);
CheckDlgButton(hDlg, DEP_TRIGGER_END, dep_trigger_end);
}

static void repaint_deptrigger_ch(HWND hDlg)
{
    out_smart = 1;
    sprintf(string,"%d",dep_trigger_ch);
    SetDlgItemText(hDlg,DEP_TRIGGER_CH,string);
    SetScrollPos(GetDlgItem(hDlg,DEP_TRIGGER_CH_SCROLL), 
        SB_CTL,DTOI(dep_trigger_ch,SP_TRIGGER_CH_MIN,SP_TRIGGER_CH_MAX),TRUE);
    out_smart = 0;
}

static void enable_trigger_options(HWND hDlg,int status)
{
	switch(status)
	{
	case DEP_TRIGGER_NONE:
		EnableWindow(GetDlgItem(hDlg, DEP_TRIGGER_UP), FALSE);
		EnableWindow(GetDlgItem(hDlg, DEP_TRIGGER_DOWN), FALSE);
		EnableWindow(GetDlgItem(hDlg, DEP_TRIGGER_START), FALSE);
		EnableWindow(GetDlgItem(hDlg, DEP_TRIGGER_END), FALSE);
		CheckDlgButton(hDlg, DEP_TRIGGER_STEP, 0);
        CheckDlgButton(hDlg, DEP_TRIGGER_PULSE, 0);
		break;
	case DEP_TRIGGER_STEP:
		EnableWindow(GetDlgItem(hDlg, DEP_TRIGGER_UP), TRUE);
		EnableWindow(GetDlgItem(hDlg, DEP_TRIGGER_DOWN), TRUE);
		EnableWindow(GetDlgItem(hDlg, DEP_TRIGGER_START), FALSE);
		EnableWindow(GetDlgItem(hDlg, DEP_TRIGGER_END), FALSE);
		CheckDlgButton(hDlg, DEP_TRIGGER_NONE, 0);
        CheckDlgButton(hDlg, DEP_TRIGGER_PULSE, 0);
		break;
	case DEP_TRIGGER_PULSE:
		EnableWindow(GetDlgItem(hDlg, DEP_TRIGGER_UP), FALSE);
		EnableWindow(GetDlgItem(hDlg, DEP_TRIGGER_DOWN), FALSE);
		EnableWindow(GetDlgItem(hDlg, DEP_TRIGGER_START), TRUE);
		EnableWindow(GetDlgItem(hDlg, DEP_TRIGGER_END), TRUE);
		CheckDlgButton(hDlg, DEP_TRIGGER_NONE, 0);
        CheckDlgButton(hDlg, DEP_TRIGGER_STEP, 0);
		break;

	}
}
//shaowei Mod. Autopoking
    
static void repaint_pulse_number(HWND hDlg)
{
        out_smart=1;
        sprintf(string,"%d",dep_pulse_number);
        SetDlgItemText(hDlg,DEP_PULSE_NUMBER,string);
        out_smart=0;
}
static void repaint_inter_pulse_delay(HWND hDlg)
{
        out_smart=1;
        sprintf(string,"%d",dep_inter_pulse_delay);
        SetDlgItemText(hDlg,DEP_INTER_PULSE_DELAY,string);
        out_smart=0;
}
    

static void repaint_pulse_bias_speed(HWND hDlg)
{
        out_smart=1;
        sprintf(string,"%0.3f",dep_pulse_bias_speed);
        SetDlgItemText(hDlg,DEP_PULSE_BIAS_SPEED,string);
        SetScrollPos(GetDlgItem(hDlg, DEP_PULSE_BIAS_SPEED_SCROLL), SB_CTL, 
            DTOI(dep_pulse_bias_speed,DEP_PULSE_BIAS_SPEED_MIN,
                DEP_PULSE_BIAS_SPEED_MAX), TRUE);
        out_smart=0;
}

/*
static void repaint_pulse_i_speed(HWND hDlg)
{
        out_smart=1;
        sprintf(string,"%0.3f",dep_pulse_bias_speed);
        SetDlgItemText(hDlg,DEP_PULSE_BIAS_SPEED,string);
//        SetScrollPos(GetDlgItem(hDlg,DEP_WRI_TIME_SC),SB_CTL,write_time,TRUE);
        out_smart=0;
}
*/

static void repaint_pulse_duration(HWND hDlg)
{
        out_smart=1;
        if (dep_pulse_duration_unit==DEP_PULSE_DURATION_MICRO)
            sprintf(string,"%d",dep_pulse_duration);
        else sprintf(string,"%d",dep_pulse_duration/1000);
        SetDlgItemText(hDlg,DEP_PULSE_DURATION,string);
        sprintf(string,"%d",dep_pulse_ignore_initial);
        SetDlgItemText(hDlg,DEP_PULSE_IGNORE_INITIAL,string);
        sprintf(string,"%d",dep_pulse_change_time);
        SetDlgItemText(hDlg,DEP_PULSE_CHANGE_TIME,string);
        sprintf(string,"%d",dep_pulse_avg_stop_pts);
        SetDlgItemText(hDlg,DEP_PULSE_AVG_STOP_PTS,string);
        sprintf(string,"%d",dep_pulse_after_time);
        SetDlgItemText(hDlg,DEP_PULSE_AFTER_TIME,string);
        sprintf(string,"%0.2f",dep_pulse_percent_change);
        SetDlgItemText(hDlg,DEP_PULSE_PERCENT_CHANGE,string);
        out_smart=0;
}

void repaint_write_time(HWND hDlg)
{
        out_smart=1;
        sprintf(string,"%d",write_time);
        SetDlgItemText(hDlg,DEP_WRI_TIME,string);
        SetScrollPos(GetDlgItem(hDlg,DEP_WRI_TIME_SC),SB_CTL,write_time,TRUE);
        out_smart=0;
}

void repaint_max_write_time(HWND hDlg)
{
        out_smart=1;
        sprintf(string,"%d",max_write_time);
        SetDlgItemText(hDlg,DEP_MAX_WRI_TIME,string);
        SetScrollPos(GetDlgItem(hDlg,DEP_MAX_WRI_TIME_SC),SB_CTL,
            max_write_time,TRUE);
        out_smart=0;
}

void repaint_move_time(HWND hDlg)
{
        out_smart=1;
        sprintf(string,"%d",move_time);
        SetDlgItemText(hDlg,DEP_MOV_TIME,string);
        SetScrollPos(GetDlgItem(hDlg,DEP_MOV_TIME_SC),SB_CTL,move_time,TRUE);
        out_smart=0;
}

void repaint_move_bias(HWND hDlg)
{
        out_smart=1;
        SetDlgItemText(hDlg, DEP_MOV_BIAS, gcvt(dtov(move_bias,sample_bias_range), 5, string));
        SetScrollPos(GetDlgItem(hDlg,DEP_MOV_BIAS_SC),SB_CTL,move_bias,TRUE);
        out_smart=0;
        
}

void repaint_write_bias(HWND hDlg)
{
        out_smart=1;
		sprintf(string,"%0.5f",dtov_bias(write_bias,sample_bias_range));
    SetDlgItemText(hDlg, DEP_WRI_BIAS, string);
    SetScrollPos(GetDlgItem(hDlg,DEP_WRI_BIAS_SC),SB_CTL,write_bias,TRUE);
        out_smart=0;
        
}


void repaint_move_current(HWND hDlg)
{
    out_smart=1;
    SetScrollPos(GetDlgItem(hDlg, DEP_MOV_CURRENT_SCROLL), SB_CTL, 
        (int) tip_current_max-move_i_setpoint, TRUE);
    SetDlgItemText(hDlg, DEP_MOV_CURRENT, 
        gcvt(move_i_set, 5, string));
    out_smart=0;
}



void repaint_lines_bias(HWND hDlg)
{
    out_smart=1;
    SetDlgItemText(hDlg, DEP_MAX_BIAS, gcvt(dtov(lines_max_bias,sample_bias_range), 5, string));
    SetScrollPos(GetDlgItem(hDlg,DEP_MAX_BIAS_SCROLL),SB_CTL,lines_max_bias,TRUE);
    out_smart=0;
        
}

void repaint_lines_current(HWND hDlg)
{
    out_smart=1;
    SetScrollPos(GetDlgItem(hDlg, DEP_MAX_I_SCROLL), SB_CTL, 
        (int) tip_current_max-lines_max_i_setpoint, TRUE);
    SetDlgItemText(hDlg, DEP_MAX_I, 
        gcvt(lines_max_i_set, 5, string));
    out_smart=0;
}

void repaint_write_current(HWND hDlg)
{
    out_smart=1;
    SetScrollPos(GetDlgItem(hDlg, DEP_WRI_CURRENT_SCROLL), SB_CTL, 
        (int) tip_current_max-write_i_setpoint, TRUE);
    SetDlgItemText(hDlg, DEP_WRI_CURRENT, 
        gcvt(write_i_set, 5, string));
    out_smart=0;
}

#define READ_DEP_CLOCK()       {\
        outp(cntrcmd,0x84); /*latch cntr3 */ \
        lobyte=inp(cntr3); \
        hibyte=inp(cntr3)*256+lobyte; \
        if (cntr2value<hibyte) \
        { \
            dep_oscill_time_offset+=65535.0/2; \
        } \
        cntr2value=hibyte; \
        dep_oscill_time=(float)(((double)(65535-hibyte))*0.5+ \
            dep_oscill_time_offset); \
        }

#ifdef OLD
void dep_pulse2(HWND hDlg)
{
    int i;
    int bias_time;    
    int bias_bits=1;
    int dep_data_max;
    int event_fb_on=0;
    int event_read_i=0;
    int n[DIO_INPUT_CHANNELS];
    float min_z,max_z;
    
    unsigned int dep_oscill_read_ch;
    float dep_once_measured_i;
    unsigned int z_gain_before,z_gain;
    int *data_pos;
    int total_selected,ch_selected[DIO_INPUT_CHANNELS];
    float dep_oscill_time_offset,dep_oscill_time,start_time;
    unsigned int lobyte,hibyte,cntr2value;
    float avg_data[DIO_INPUT_CHANNELS];
    int clock_read=0;

    data_pos=(int *) malloc(sizeof(int)*4);
    dep_data_max=(dep_measure_extra*2+dep_pulse_duration)/
        ADC_DELAY/dep_pulse_avg_pts+1;
    if (dep_pulse_measure_i) 
    {
        free_data(&(dep_oscill_data[i_in_ch]));
        alloc_data(&(dep_oscill_data[i_in_ch]),DATATYPE_2D,dep_data_max,GEN2D_NONE,GEN2D_NONE,0);
    }
    if (dep_pulse_measure_z)
    {
        free_data(&(dep_oscill_data[zi_ch]));
        alloc_data(&(dep_oscill_data[zi_ch]),DATATYPE_2D,dep_data_max,GEN2D_NONE,GEN2D_NONE,0);
    }
    
    bias_time=sample_bias_range*10/4.096/dep_pulse_bias_speed;
    while (bias_time<3*CLOCK_DEAD_TIME)
    {
        bias_bits++;
        bias_time=bias_bits*sample_bias_range*10/4.096/dep_pulse_bias_speed;
    }


/*
    sprintf(string,"time %d",bias_time);
    MessageBox(GetFocus(),string,"Info",MB_OKCANCEL);
*/
    
    if (dep_pulse_feedback==DEP_PULSE_FEEDBACK_OFF )
/*
        || dep_pulse_feedback==DEP_PULSE_FEEDBACK_AFTER) 
*/
    {
        dio_feedback(0);
        if (dep_pulse_z_offset)
        {
            if (dep_pulse_use_move_bias)
            {
                ramp_bias(move_bias,bias_time,0,bias_bits);
            }
            move_to_speed(z_offset_ch,dac_data[z_offset_ch],
                dac_data[z_offset_ch]+dep_pulse_z_offset,
                DEFAULT_Z_OFFSET_TIME, 0, 0,0,DIGITAL_FEEDBACK_MAX,
                DIGITAL_FEEDBACK_REREAD);
            dio_start_clock(1000000);
            dio_wait_clock(); /* creep */
            dac_data[z_offset_ch]+=dep_pulse_z_offset;
            scan_z=dac_data[z_offset_ch];
        }
    }
    switch (scan_freq)
    {
        case 0:
            z_gain_before = Z_TENTH_GAIN;
            break;
        case 1:
            z_gain_before = Z_ONE_GAIN;
            break;
        case 2:
            z_gain_before = Z_TEN_GAIN;
            break;
    }
    switch (dep_pulse_freq)
    {
        case DEP_PULSE_FREQ_100:
            z_gain = Z_TENTH_GAIN;
            break;
        case DEP_PULSE_FREQ_1:
            z_gain = Z_ONE_GAIN;
            break;
        case DEP_PULSE_FREQ_10:
            z_gain = Z_TEN_GAIN;
            break;
    }
    set_z_gain(z_gain);
    for(i=0;i<4;i++) 
    {
        ch_selected[i]=0;
        n[i]=0;
        avg_data[i]=0;
    }
    total_selected=0;
    if (dep_pulse_measure_i) 
    {
        ch_selected[i_in_ch]=1;
        total_selected++;
    }
    if (dep_pulse_measure_z) 
    {
        ch_selected[zi_ch]=1;
        total_selected++;
    }
    for(i=0;i<4;i++) 
    {
        data_pos[i]=0;
/*
            (dep_oscill_data[i]+dep_data_max-1)->time2d=0;
*/
    }
    if (dep_pulse_bias_step)
    {
        dac_data[sample_bias_ch]=write_bias;
        dio_out(sample_bias_ch,write_bias);
    }
    else ramp_bias(write_bias,bias_time,0,bias_bits);

/*
    if (dep_pulse_feedback==DEP_PULSE_FEEDBACK_AFTER) dio_feedback(1);
    if (dep_pulse_measure_i || dep_pulse_measure_z)
*/
    {    
        /* start the clock **now** since we've changed the bias! */
        outpw(cfg3,0x2400);
        outp(cntrcmd,0xB4); /* cntr3 16 bit rate generation */
        outp(cntr3,0xFF);
        outp(cntr3,0xFF);
        cntr2value=65535;
        dep_oscill_read_ch=0;
        dep_oscill_time_offset=0;
        if (total_selected) 
        {
            while (!ch_selected[dep_oscill_read_ch])
            {
                dep_oscill_read_ch++;
                if (dep_oscill_read_ch>3) dep_oscill_read_ch=0;
            } 
            outpw(cfg1,dep_oscill_read_ch&1);         /* Set out1 = bit 0 of input channel number */
            outpw(cfg2,dep_oscill_read_ch>>1);            /* Set out2 = bit 1 of input channel number */
            input_ch = 1<<(dep_oscill_read_ch*2+1);       /* Set A/D start convert bit */
            out1 = dep_oscill_read_ch&1;              /* Update current out1 */
        }
        dep_oscill_time=0;
        READ_DEP_CLOCK();
        start_time=dep_oscill_time;
/* here we start to take data */        
        while(dep_oscill_time<dep_pulse_duration)
        {
            clock_read=0;
            if (dep_pulse_feedback==DEP_PULSE_FEEDBACK_AFTER && !event_fb_on
                && dep_oscill_time-start_time>=dep_pulse_feedback_wait)
            {
                dio_feedback(1);
                event_fb_on=1;
                READ_DEP_CLOCK();
                clock_read=1;
            }
            if (dep_pulse_measure_i_once && !event_read_i &&
                dep_oscill_time-start_time>=dep_pulse_measure_i_wait)
            {
                if (!dep_pulse_measure_i) mprintf("Weird shit man!");                
                event_read_i=1;
                outpw(cfg1,i_in_ch&1);         /* Set out1 = bit 0 of input channel number */
                outpw(cfg2,i_in_ch>>1);            /* Set out2 = bit 1 of input channel number */
                input_ch = 1<<(i_in_ch*2+1);       /* Set A/D start convert bit */
                out1 = i_in_ch&1;              /* Update current out1 */
                dio_out(AD_ch,input_ch|dac_data[AD_ch]);      /* Start A/D conversion (bit => 1) */
                dio_out(AD_ch,AD_strobe|dac_data[AD_ch]);     /* Pulse (bit => 0) */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
                READ_DEP_CLOCK();
                clock_read=1;
                dep_once_measured_i=inpw(portc);
            }
            if (total_selected && dep_oscill_time<dep_pulse_duration &&
                data_pos[0]<dep_data_max)   
            {
                if (total_selected>4) mprintf("Weird shit, man!");                
                if (total_selected>1 || !ch_selected[dep_oscill_read_ch])
                {
                    do
                    {
                        dep_oscill_read_ch++;
                        if (dep_oscill_read_ch>3) dep_oscill_read_ch=0;
                    } while (!ch_selected[dep_oscill_read_ch]);
                    outpw(cfg1,dep_oscill_read_ch&1);         /* Set out1 = bit 0 of input channel number */
                    outpw(cfg2,dep_oscill_read_ch>>1);            /* Set out2 = bit 1 of input channel number */
                    input_ch = 1<<(dep_oscill_read_ch*2+1);       /* Set A/D start convert bit */
                    out1 = dep_oscill_read_ch&1;              /* Update current out1 */
                }           
/*
                READ_DEP_CLOCK();
        sprintf(string,"1:%0.1f",dep_oscill_time);
        SetDlgItemText(hDlg,DEP_PULSE_AVG_I,string);
*/
                dio_out(AD_ch,input_ch|dac_data[AD_ch]);      /* Start A/D conversion (bit => 1) */
                dio_out(AD_ch,AD_strobe|dac_data[AD_ch]);     /* Pulse (bit => 0) */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
                READ_DEP_CLOCK();
                clock_read=1;
                n[dep_oscill_read_ch]++;
                if (n[dep_oscill_read_ch]==dep_pulse_avg_pts)
                {
                    n[dep_oscill_read_ch]=0;
                    avg_data[dep_oscill_read_ch]+=inpw(portc);
                    *(dep_oscill_data[dep_oscill_read_ch]->time2d+data_pos[dep_oscill_read_ch])=
                        dep_oscill_time;  
                    *(dep_oscill_data[dep_oscill_read_ch]->data2d+data_pos[dep_oscill_read_ch])=
                        (unsigned short)(avg_data[dep_oscill_read_ch]/(float)dep_pulse_avg_pts+0.5);
                    data_pos[dep_oscill_read_ch]++;
                    avg_data[dep_oscill_read_ch]=0;
                }
                else avg_data[dep_oscill_read_ch]+=inpw(portc);
            } /* got the data */
            if (!clock_read)
            {
                READ_DEP_CLOCK();
            }
        }
        
    }
    outpw(cfg3,0x2400);
    if (dep_pulse_z_offset>0) /* preventing current overload */
    {
        if (dep_pulse_use_move_bias)
        {
            ramp_bias(move_bias,bias_time,0,bias_bits);
        }
        move_to_speed(z_offset_ch,dac_data[z_offset_ch],
            dac_data[z_offset_ch]-dep_pulse_z_offset,
            DEFAULT_Z_OFFSET_TIME, 0, 0,0,DIGITAL_FEEDBACK_MAX,
            DIGITAL_FEEDBACK_REREAD);
        dac_data[z_offset_ch]-=dep_pulse_z_offset;
        scan_z=dac_data[z_offset_ch];
    }
    if (z_gain!=z_gain_before) 
    {
//        mprintf("funny z");
        set_z_gain(z_gain_before);
    }
    if (dep_pulse_bias_step)
    {
        dio_out(sample_bias_ch,sample_bias);
        dac_data[sample_bias_ch]=sample_bias;
    }        
    else ramp_bias(sample_bias,bias_time,0,bias_bits);
    if (dep_pulse_feedback==DEP_PULSE_FEEDBACK_OFF || 
        dep_pulse_feedback==DEP_PULSE_FEEDBACK_AFTER) 
    {
        dio_feedback(1);
        if (dep_pulse_z_offset) 
        {
            dio_start_clock(100000);
            dio_wait_clock();
            auto_z_below(IN_ZERO);
        }
    }
    for(i=0;i<4;i++)
    {
        if (n[i])
        {
            *(dep_oscill_data[i]->time2d+data_pos[i])=
                        dep_oscill_time;            
            *(dep_oscill_data[i]->data2d+data_pos[i])=
                        (unsigned short)(avg_data[i]/(float)n[i]+0.5);
            data_pos[i]++;
        }
    }

    if (dep_pulse_measure_i)
    {
        dep_oscill_data[i_in_ch]->size=data_pos[i_in_ch];
        dep_oscill_data[i_in_ch]->type2d=TYPE2D_PULSE_I;
        find_min_max(dep_oscill_data[i_in_ch],&min_z,&max_z);
        set_data(dep_oscill_data[i_in_ch]);
        dep_oscill_data[i_in_ch]->type2d=TYPE2D_PULSE_I;
    }
    if (dep_pulse_measure_z)
    {
        dep_oscill_data[zi_ch]->size=data_pos[zi_ch];
        dep_oscill_data[zi_ch]->type2d=TYPE2D_PULSE_Z;
        find_min_max(dep_oscill_data[zi_ch],&min_z,&max_z);
            
        set_data(dep_oscill_data[zi_ch]);
        dep_oscill_data[zi_ch]->type2d=TYPE2D_PULSE_Z;
    }
    if (dep_pulse_measure_i_once)
    {
        sprintf(string,"%0.3f",in_dtov(dep_once_measured_i));
        SetDlgItemText(hDlg,DEP_PULSE_AVG_I,string);
    }
    free(data_pos);
}
#endif

//pushkin
void dep_pulse_dsp(HWND hDlg)
{
	int i;
	int pts_number;
	int	flags = 0;
	int channel = 0;
	float min_z,max_z;

	//the following is added by SW.
	int all_read=0;
	int read_result=0;
	int bytes_unread=0;
	int data_size=0;
	int l=0,j=0;
	int temp1=0;
	unsigned int temp2;
	unsigned char size_buff[2]={0,0};
	unsigned char serial_buff[131072];		//DSP module will send out up to 131072 bytes.

	if((dep_pulse_measure_i) | (dep_pulse_measure_z)) 
	{
		flags+=0x01; //measure none
		if(dep_pulse_measure_i) 
		{
			flags+=0x02;
			dep_pulse_measure_z=0;
		}
		else
			dep_pulse_measure_z=1;
	
	}
	if(dep_pulse_feedback == DEP_PULSE_FEEDBACK_ON) flags+=0x04;
	if(dep_pulse_bias_step) flags+=0x08;
	if(dep_pulse_duration_mode!=DEP_PULSE_DURATION_FIXED) flags+=0x10;
	if(bit16) flags += 0x20;
	if(dep_pulse_use_move_bias) flags+=0x40;
	if(dep_pulse_percent_change>100) flags+=0x80;
	
	//pts_number=(int)(dep_pulse_duration/(ADC_CONV_TIME*dep_pulse_avg_pts));
	//the DSP code takes Dep_Num_Pts*[Dep_Delay+Dep_Aver_Every*(2276)+22] clock cycles. SW
	pts_number=(int)(dep_pulse_duration/(move_time+DSP_CLOCK_PERIOD*(dep_pulse_avg_pts*2.276+0.022)));

	if (dep_pulse_measure_i) channel = i_in_ch;
	else if(dep_pulse_measure_z) channel = zi_ch;

	free_data(&(dep_oscill_data[channel]));

	dep_setup(move_bias,write_bias,move_i_setpoint,write_i_setpoint,dac_data[z_offset_ch]+dep_pulse_z_offset,
		dep_pulse_measure_i_wait,pts_number,move_time,dep_pulse_avg_stop_pts,(int)(dep_pulse_ignore_initial/(ADC_CONV_TIME*dep_pulse_avg_pts)),
		(int)(dep_pulse_change_time/(ADC_CONV_TIME*dep_pulse_avg_pts)),(int)(dep_pulse_after_time/(ADC_CONV_TIME*dep_pulse_avg_pts)),dep_pulse_percent_change/100,dep_pulse_avg_pts,flags);
	
	dep_start();
	
	//do not recieve data back from the DSP if we didn't read anything
	if((dep_pulse_measure_i) | (dep_pulse_measure_z)){
	  
	  pts_number = read_2_serial();

	  //read data from DSP
	  data_size=pts_number*2;
	  read_result=read_serial(&(serial_buff[l]),data_size,&bytes_unread);
	  l +=(data_size-bytes_unread);
      while((!read_result || (bytes_unread !=0))&& (j<10))//temp Shaowei
	  {
		  data_size=bytes_unread;
		  read_result=read_serial(&(serial_buff[l]),data_size,&bytes_unread);
		  l+=(data_size-bytes_unread);
		  j++;
	  }

	  if(bytes_unread==0) 
	  {
		  all_read=1;
	  	  alloc_data(&(dep_oscill_data[channel]),DATATYPE_2D,pts_number,GEN2D_NONE,GEN2D_NONE,0);
		  l=l/2;
		  temp2=(unsigned int)(serial_buff[0] + (serial_buff[1] << 8));
		  for(i=0; i<l; i++)
		  {
			//*(dep_oscill_data[channel]->time2d + i)= i*ADC_CONV_TIME;            
			*(dep_oscill_data[channel]->time2d + i)= (float)(i*(move_time+DSP_CLOCK_PERIOD*(2.276*dep_pulse_avg_pts+0.022)));            
			*(dep_oscill_data[channel]->data2d + i)= (unsigned int)(serial_buff[i*2] + (serial_buff[i*2+1] << 8));
		  
			if (temp2!=(unsigned int)(serial_buff[i*2] + (serial_buff[i*2+1] << 8)))
			{
				temp1=1;
			}

		  }

	  }
	  
	  if(temp1==0)
		  MessageBox(GetFocus(),"Dep Error in serial communication?","Serial Error",MB_OK);

	  if(j==10)
	  {
		  all_read=1;
		  // Shaowei Aug 29 2013 Tip retract when error
		  retract_serial(TRUE);
		  MessageBox(GetFocus(),"Comm Error during deposition! Possible underflow.","Comm Error", MB_OK);
	  }
	  
	}	

	if (dep_pulse_measure_i)
    {
        dep_oscill_data[i_in_ch]->size=pts_number;
        dep_oscill_data[i_in_ch]->type2d=TYPE2D_PULSE_I;
        find_min_max(dep_oscill_data[i_in_ch],&min_z,&max_z);
        set_data(dep_oscill_data[i_in_ch]);
        dep_oscill_data[i_in_ch]->type2d=TYPE2D_PULSE_I;
    }
    if (dep_pulse_measure_z)
    {
        dep_oscill_data[zi_ch]->size=pts_number;
        dep_oscill_data[zi_ch]->type2d=TYPE2D_PULSE_Z;
        find_min_max(dep_oscill_data[zi_ch],&min_z,&max_z);
        set_data(dep_oscill_data[zi_ch]);
        dep_oscill_data[zi_ch]->type2d=TYPE2D_PULSE_Z;
    }
	
}

void dep_pulse3(HWND hDlg)
{
    int i;
    int bias_time;    
    int bias_bits=1;
    int dep_data_max;
    int event_fb_on=0;
    int event_read_i=0;
    int n[DIO_INPUT_CHANNELS];
    float min_z,max_z;
    
    unsigned int dep_oscill_read_ch;
    float dep_once_measured_i;
    unsigned int z_gain_before,z_gain;
    int data_pos[DIO_INPUT_CHANNELS];
    int total_selected,ch_selected[DIO_INPUT_CHANNELS];
    float dep_oscill_time_offset,dep_oscill_time,start_time;
    unsigned int lobyte,hibyte,cntr2value;
    float avg_data[DIO_INPUT_CHANNELS];
    int clock_read=0;
    int *stop_data;
    int stop_data_max,stop_data_pos_1,stop_data_pos_2,stop_data_pos_3,
        stop_data_pos_4,stop_data_pts;
    float stop_avg_1,stop_avg_2,dep_pulse_this_duration;
    unsigned short this_data;
    int stop_now=0;
    int stop_data_first_time=1;

    
    dep_pulse_this_duration=(float)dep_pulse_duration;
    dep_data_max=(dep_measure_extra*2+dep_pulse_duration)/
        ADC_DELAY/dep_pulse_avg_pts+1;
    if (dep_pulse_duration_mode!=DEP_PULSE_DURATION_FIXED)
    {
        stop_data_max=2*dep_pulse_avg_stop_pts+dep_pulse_change_time/ADC_DELAY;
        stop_data=(int *)malloc(sizeof(int)*stop_data_max);
        stop_avg_1=0;
        stop_avg_2=0;
        stop_data_pos_3=0; 
    }
    if (dep_pulse_measure_i) 
    {
        free_data(&(dep_oscill_data[i_in_ch]));
        alloc_data(&(dep_oscill_data[i_in_ch]),DATATYPE_2D,dep_data_max,GEN2D_NONE,GEN2D_NONE,0);
    }
    if (dep_pulse_measure_z)
    {
        free_data(&(dep_oscill_data[zi_ch]));
        alloc_data(&(dep_oscill_data[zi_ch]),DATATYPE_2D,dep_data_max,GEN2D_NONE,GEN2D_NONE,0);
    }
    
    bias_time=(int)(sample_bias_range*10/4.096/dep_pulse_bias_speed);
    while (bias_time<3*CLOCK_DEAD_TIME)
    {
        bias_bits++;
        bias_time=(int)(bias_bits*sample_bias_range*10/4.096/dep_pulse_bias_speed);
    }


/*
    sprintf(string,"time %d",bias_time);
    MessageBox(GetFocus(),string,"Info",MB_OKCANCEL);
*/
    for(i=0;i<4;i++) 
    {
        ch_selected[i]=0;
        n[i]=0;
        avg_data[i]=0;
    }
    total_selected=0;
    if (dep_pulse_measure_i) 
    {
        ch_selected[i_in_ch]=1;
        total_selected++;
    }
    if (dep_pulse_measure_z) 
    {
        ch_selected[zi_ch]=1;
        total_selected++;
    }
    for(i=0;i<4;i++) 
    {
        data_pos[i]=0;
    }

	__MACHINE(void __cdecl _disable());
    if (dep_pulse_feedback==DEP_PULSE_FEEDBACK_OFF)
    {
        dio_feedback(0);
        if (dep_pulse_z_offset)
        {
            if (dep_pulse_use_move_bias)
            {
                ramp_bias(move_bias,bias_time,0,bias_bits);
                if (dac_data[i_setpoint_ch]!=move_i_setpoint)
                {
                    move_to_timed(i_setpoint_ch,dac_data[i_setpoint_ch],
                        move_i_setpoint,DEP_CURRENT_BIT_TIME);
                    dac_data[i_setpoint_ch]=move_i_setpoint;
                }
            }
            move_to_speed(z_offset_ch,dac_data[z_offset_ch],
                dac_data[z_offset_ch]+dep_pulse_z_offset,
                DEFAULT_Z_OFFSET_TIME, 0, 0,0,DIGITAL_FEEDBACK_MAX,
                DIGITAL_FEEDBACK_REREAD);
            dio_start_clock(1000000);
            dio_wait_clock(); /* creep */
            dac_data[z_offset_ch]+=dep_pulse_z_offset;
            scan_z=dac_data[z_offset_ch];
        }
    }
    switch (scan_freq)
    {
        case 0:
            z_gain_before = Z_TENTH_GAIN;
            break;
        case 1:
            z_gain_before = Z_ONE_GAIN;
            break;
        case 2:
            z_gain_before = Z_TEN_GAIN;
            break;
    }
    switch (dep_pulse_freq)
    {
        case DEP_PULSE_FREQ_100:
            z_gain = Z_TENTH_GAIN;
            break;
        case DEP_PULSE_FREQ_1:
            z_gain = Z_ONE_GAIN;
            break;
        case DEP_PULSE_FREQ_10:
            z_gain = Z_TEN_GAIN;
            break;
    }
    set_z_gain(z_gain);
    if (dep_pulse_bias_step)
    {
        dio_out(sample_bias_ch,write_bias);
        dac_data[sample_bias_ch]=write_bias;
    }
    else ramp_bias(write_bias,bias_time,0,bias_bits);
    if (dac_data[i_setpoint_ch]!=write_i_setpoint)
    {
        if (dep_pulse_feedback==DEP_PULSE_FEEDBACK_OFF || dep_pulse_current_step)
        {
            dio_out(i_setpoint_ch,write_i_setpoint);
        }
        else
        {
            move_to_timed(i_setpoint_ch,dac_data[i_setpoint_ch],write_i_setpoint,
                DEP_CURRENT_BIT_TIME);
        }
        dac_data[i_setpoint_ch]=write_i_setpoint;
    }

/*
    if (dep_pulse_feedback==DEP_PULSE_FEEDBACK_AFTER) dio_feedback(1);
    if (dep_pulse_measure_i || dep_pulse_measure_z)
*/
    {    
        /* start the clock **now** since we've changed the bias! */
        outpw(cfg3,0x2400);
        outp(cntrcmd,0xB4); /* cntr3 16 bit rate generation */
        outp(cntr3,0xFF);
        outp(cntr3,0xFF);
        cntr2value=65535;
        dep_oscill_read_ch=0;
        dep_oscill_time_offset=0;
        if (total_selected) 
        {
            while (!ch_selected[dep_oscill_read_ch])
            {
                dep_oscill_read_ch++;
                if (dep_oscill_read_ch>3) dep_oscill_read_ch=0;
            } 

	    /* kafka */
	    if( dep_oscill_read_ch != zi_ch) {
              outpw(cfg1,dep_oscill_read_ch&1);         /* Set out1 = bit 0 of input channel number */
              outpw(cfg2,dep_oscill_read_ch>>1);            /* Set out2 = bit 1 of input channel number */
              input_ch = 1<<(dep_oscill_read_ch*2+1);       /* Set A/D start convert bit */
              out1 = dep_oscill_read_ch&1;              /* Update current out1 */
	    }
	    else {
	      unsigned int ch_3 = 3;
              outpw(cfg1,ch_3&1);         /* Set out1 = bit 0 of input channel number */
              outpw(cfg2,ch_3>>1);            /* Set out2 = bit 1 of input channel number */
              input_ch = 1<<(ch_3*2+1);       /* Set A/D start convert bit */
              out1 = ch_3&1;              /* Update current out1 */
	    }
	    /* kafka */

        }
        dep_oscill_time=0;
        READ_DEP_CLOCK();
        start_time=dep_oscill_time;
/* here we start to take data */        
        while(dep_oscill_time<dep_pulse_this_duration)
        {
            clock_read=0;
            if (dep_pulse_feedback_after && (!event_fb_on)
                && ((dep_oscill_time-start_time)>=dep_pulse_feedback_wait))
            {
                if (dep_pulse_feedback==DEP_PULSE_FEEDBACK_OFF)
                    dio_feedback(1);
                else dio_feedback(0);
//mprintf("toggled feedback");                
                event_fb_on=1;
                READ_DEP_CLOCK();
                clock_read=1;
            }
            if (dep_pulse_measure_i_once && !event_read_i &&
                dep_oscill_time-start_time>=dep_pulse_measure_i_wait)
            {
                event_read_i=1;
                outpw(cfg1,i_in_ch&1);         /* Set out1 = bit 0 of input channel number */
                outpw(cfg2,i_in_ch>>1);            /* Set out2 = bit 1 of input channel number */
                input_ch = 1<<(i_in_ch*2+1);       /* Set A/D start convert bit */
                out1 = i_in_ch&1;              /* Update current out1 */
                dio_out(AD_ch,input_ch|dac_data[AD_ch]);      /* Start A/D conversion (bit => 1) */
                dio_out(AD_ch,AD_strobe|dac_data[AD_ch]);     /* Pulse (bit => 0) */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
                READ_DEP_CLOCK();
                clock_read=1;
                dep_once_measured_i=inpw(portc);
            }
            if (total_selected && dep_oscill_time<dep_pulse_this_duration &&
                data_pos[0]<dep_data_max)
            {

                if (total_selected>1 || !ch_selected[dep_oscill_read_ch])
                {
                    do
                    {
                        dep_oscill_read_ch++;
                        if (dep_oscill_read_ch>3) dep_oscill_read_ch=0;
                    } while (!ch_selected[dep_oscill_read_ch]);
		    
		/* kafka */
		  if( dep_oscill_read_ch != zi_ch) {
                    outpw(cfg1,dep_oscill_read_ch&1);         /* Set out1 = bit 0 of input channel number */
                    outpw(cfg2,dep_oscill_read_ch>>1);            /* Set out2 = bit 1 of input channel number */
                    input_ch = 1<<(dep_oscill_read_ch*2+1);       /* Set A/D start convert bit */
                    out1 = dep_oscill_read_ch&1;              /* Update current out1 */
		  }
		  else {
		    unsigned int ch_3 = 3;
                    outpw(cfg1,ch_3&1);         /* Set out1 = bit 0 of input channel number */
                    outpw(cfg2,ch_3>>1);            /* Set out2 = bit 1 of input channel number */
                    input_ch = 1<<(ch_3*2+1);       /* Set A/D start convert bit */
                    out1 = ch_3&1;              /* Update current out1 */
		  }
                }           
		/* kafka */

                dio_out(AD_ch,input_ch|dac_data[AD_ch]);      /* Start A/D conversion (bit => 1) */
                dio_out(AD_ch,AD_strobe|dac_data[AD_ch]);     /* Pulse (bit => 0) */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
                READ_DEP_CLOCK();
                clock_read=1;
                n[dep_oscill_read_ch]++;
                if (n[dep_oscill_read_ch]==dep_pulse_avg_pts)
                {
                    n[dep_oscill_read_ch]=0;
                    this_data=inpw(portc); 
                    avg_data[dep_oscill_read_ch]+=this_data;
                    *(dep_oscill_data[dep_oscill_read_ch]->time2d+data_pos[dep_oscill_read_ch])=
                        dep_oscill_time;  
                    *(dep_oscill_data[dep_oscill_read_ch]->data2d+data_pos[dep_oscill_read_ch])=
                        (unsigned short)(avg_data[dep_oscill_read_ch]/(float)dep_pulse_avg_pts+0.5);
                    data_pos[dep_oscill_read_ch]++;
                    avg_data[dep_oscill_read_ch]=0;
                    if ((dep_pulse_duration_mode==DEP_PULSE_DURATION_I &&
                        dep_oscill_read_ch==i_in_ch) || 
                        (dep_pulse_duration_mode==DEP_PULSE_DURATION_Z &&
                        dep_oscill_read_ch==zi_ch) && !stop_now)
                    {
                        if (dep_oscill_time-start_time>dep_pulse_change_time)
                        {
                            if (stop_data_first_time)
                            {
                                stop_data_first_time=0;
                                stop_data_pts=stop_data_pos_3;
                                stop_data_pos_3=0;
                                stop_data_pos_4=dep_pulse_avg_stop_pts-1;
                                stop_data_pos_2=stop_data_pts-1;
                                stop_data_pos_1=stop_data_pts-dep_pulse_avg_stop_pts;
                                for(i=stop_data_pos_3;i<=stop_data_pos_4;i++)
                                    stop_avg_1+=stop_data[i];
                                for(i=stop_data_pos_1;i<=stop_data_pos_2;i++)
                                    stop_avg_2+=stop_data[i];
                            }
                                    
                            
                            stop_avg_1-=stop_data[stop_data_pos_3];
                            stop_avg_2-=stop_data[stop_data_pos_1];
                            stop_data[stop_data_pos_3]=((int)this_data)-IN_ZERO;
                            stop_data_pos_1++;
                            stop_data_pos_2++;
                            stop_data_pos_3++;
                            stop_data_pos_4++;
                            if (stop_data_pos_1==stop_data_pts) stop_data_pos_1=0;
                            if (stop_data_pos_2==stop_data_pts) stop_data_pos_2=0;
                            if (stop_data_pos_3==stop_data_pts) stop_data_pos_3=0;
                            if (stop_data_pos_4==stop_data_pts) stop_data_pos_4=0;
                            stop_avg_1+=stop_data[stop_data_pos_4];
                            stop_avg_2+=stop_data[stop_data_pos_2];
                            if (dep_oscill_time>dep_pulse_ignore_initial)
                            {
                                if (stop_avg_2>stop_avg_1 && 
                                    dep_pulse_percent_change>0)
                                {
                                    if ((stop_avg_2-stop_avg_1)>=
                                        dep_pulse_percent_change*stop_avg_1/100)
                                    {
                                        dep_pulse_this_duration=dep_oscill_time+dep_pulse_after_time;
                                        stop_now=1;
                                    }
                                }
                                else if (stop_avg_2<stop_avg_1 && 
                                    dep_pulse_percent_change<0)
                                {
                                    if ((stop_avg_1-stop_avg_2)>=
                                        -dep_pulse_percent_change*stop_avg_1/100)
                                    {
                                        dep_pulse_this_duration=dep_oscill_time+dep_pulse_after_time;
                                        stop_now=1;
                                    }
                                }
                            }
                        }
                        else
                        {
                            stop_data[stop_data_pos_3]=((int)this_data)-IN_ZERO;
                            stop_data_pos_3++;
                        }
                    }
                            
                            
                }
                else 
                {
                    this_data=inpw(portc);
                    avg_data[dep_oscill_read_ch]+=this_data;
                    if ((dep_pulse_duration_mode==DEP_PULSE_DURATION_I &&
                        dep_oscill_read_ch==i_in_ch) || 
                        (dep_pulse_duration_mode==DEP_PULSE_DURATION_Z &&
                        dep_oscill_read_ch==zi_ch) && !stop_now)
                    {
                        if (dep_oscill_time-start_time>dep_pulse_change_time)
                        {
                            if (stop_data_first_time)
                            {
                                stop_data_first_time=0;
                                stop_data_pts=stop_data_pos_3;
                                stop_data_pos_3=0;
                                stop_data_pos_4=dep_pulse_avg_stop_pts-1;
                                stop_data_pos_2=stop_data_pts-1;
                                stop_data_pos_1=stop_data_pts-dep_pulse_avg_stop_pts;
                                for(i=stop_data_pos_3;i<=stop_data_pos_4;i++)
                                    stop_avg_1+=stop_data[i];
                                for(i=stop_data_pos_1;i<=stop_data_pos_2;i++)
                                    stop_avg_2+=stop_data[i];
                            }
                                    
                            
                            stop_avg_1-=stop_data[stop_data_pos_3];
                            stop_avg_2-=stop_data[stop_data_pos_1];
                            stop_data[stop_data_pos_3]=((int)this_data)-IN_ZERO;
                            stop_data_pos_1++;
                            stop_data_pos_2++;
                            stop_data_pos_3++;
                            stop_data_pos_4++;
                            if (stop_data_pos_1==stop_data_pts) stop_data_pos_1=0;
                            if (stop_data_pos_2==stop_data_pts) stop_data_pos_2=0;
                            if (stop_data_pos_3==stop_data_pts) stop_data_pos_3=0;
                            if (stop_data_pos_4==stop_data_pts) stop_data_pos_4=0;
                            stop_avg_1+=stop_data[stop_data_pos_4];
                            stop_avg_2+=stop_data[stop_data_pos_2];
                            if (dep_oscill_time>dep_pulse_ignore_initial)
                            {
                                if (stop_avg_2>stop_avg_1 && 
                                    dep_pulse_percent_change>0)
                                {
                                    if ((stop_avg_2-stop_avg_1)>=
                                        dep_pulse_percent_change*stop_avg_1/100)
                                    {
                                        dep_pulse_this_duration=dep_oscill_time+dep_pulse_after_time;
                                        stop_now=1;
                                    }
                                }
                                else if (stop_avg_2<stop_avg_1 && 
                                    dep_pulse_percent_change<0)
                                {
                                    if ((stop_avg_1-stop_avg_2)>=
                                        -dep_pulse_percent_change*stop_avg_1/100)
                                    {
                                        dep_pulse_this_duration=dep_oscill_time+dep_pulse_after_time;
                                        stop_now=1;
                                    }
                                }
                            }
                        }
                        else
                        {
                            stop_data[stop_data_pos_3]=((int)this_data)-IN_ZERO;
                            stop_data_pos_3++;
                        }
                    }
                }
            } /* got the data */
            if (!clock_read)
            {
                READ_DEP_CLOCK();
            }
        }
        outpw(cfg3,0x2400);
        
    }
    
    if (dep_pulse_z_offset>0) /* preventing current overload */
    {
        if (dep_pulse_use_move_bias)
        {
            ramp_bias(move_bias,bias_time,0,bias_bits);
            if (dac_data[i_setpoint_ch]!=move_i_setpoint)
            {
                if (dep_pulse_current_step)
                {
                    dio_out(i_setpoint_ch,move_i_setpoint);
                }
                else
                {
                    move_to_timed(i_setpoint_ch,dac_data[i_setpoint_ch],
                        move_i_setpoint,DEP_CURRENT_BIT_TIME);
                }
                dac_data[i_setpoint_ch]=move_i_setpoint;
            }
        }
        move_to_speed(z_offset_ch,dac_data[z_offset_ch],
            dac_data[z_offset_ch]-dep_pulse_z_offset,
            DEFAULT_Z_OFFSET_TIME, 0, 0,0,DIGITAL_FEEDBACK_MAX,
            DIGITAL_FEEDBACK_REREAD);
        dac_data[z_offset_ch]-=dep_pulse_z_offset;
        scan_z=dac_data[z_offset_ch];
    }
    if (z_gain!=z_gain_before) set_z_gain(z_gain_before);
    if (dep_pulse_bias_step)
    {
        dio_out(sample_bias_ch,sample_bias);
        dac_data[sample_bias_ch]=sample_bias;
    }        
    else ramp_bias(sample_bias,bias_time,0,bias_bits);
    if (dac_data[i_setpoint_ch]!=i_setpoint)
    {    
        if (dep_pulse_feedback==DEP_PULSE_FEEDBACK_OFF || 
            (dep_pulse_feedback==DEP_PULSE_FEEDBACK_ON && dep_pulse_feedback_after)
            || dep_pulse_current_step) 
        {
            dio_out(i_setpoint_ch,i_setpoint);
        }
        else
        {
            move_to_timed(i_setpoint_ch,dac_data[i_setpoint_ch],i_setpoint,
                DEP_CURRENT_BIT_TIME);
        }
        dac_data[i_setpoint_ch]=i_setpoint;
    }
    if (dep_pulse_feedback==DEP_PULSE_FEEDBACK_OFF || 
        (dep_pulse_feedback==DEP_PULSE_FEEDBACK_ON && dep_pulse_feedback_after)) 
    {
        dio_feedback(1);
        if (dep_pulse_z_offset) 
        {
            dio_start_clock(100000);
            dio_wait_clock();
            auto_z_below(IN_ZERO);
        }
    }
	__MACHINE(void __cdecl _enable());
    for(i=0;i<4;i++)
    {
        if (n[i])
        {
            *(dep_oscill_data[i]->time2d+data_pos[i])=
                        dep_oscill_time;            
            *(dep_oscill_data[i]->data2d+data_pos[i])=
                        (unsigned short)(avg_data[i]/(float)n[i]+0.5);
            data_pos[i]++;
        }
    }

    if (dep_pulse_measure_i)
    {
        dep_oscill_data[i_in_ch]->size=data_pos[i_in_ch];
        dep_oscill_data[i_in_ch]->type2d=TYPE2D_PULSE_I;
        find_min_max(dep_oscill_data[i_in_ch],&min_z,&max_z);
        set_data(dep_oscill_data[i_in_ch]);
        dep_oscill_data[i_in_ch]->type2d=TYPE2D_PULSE_I;
    }
    if (dep_pulse_measure_z)
    {
        dep_oscill_data[zi_ch]->size=data_pos[zi_ch];
        dep_oscill_data[zi_ch]->type2d=TYPE2D_PULSE_Z;
        find_min_max(dep_oscill_data[zi_ch],&min_z,&max_z);
            
        set_data(dep_oscill_data[zi_ch]);
        dep_oscill_data[zi_ch]->type2d=TYPE2D_PULSE_Z;
    }
    if (dep_pulse_measure_i_once)
    {
        sprintf(string,"%0.3f",in_dtov(dep_once_measured_i));
        SetDlgItemText(hDlg,DEP_PULSE_AVG_I,string);
    }
    if (dep_pulse_duration_mode!=DEP_PULSE_DURATION_FIXED)
    {
        free(stop_data);
    }
}

void set_data(datadef *this_data)
{
    // set fields of datadef from current dep settings

    time_t scan_time;
    
    scan_time = time(NULL);
    strcpy(string, ctime(&scan_time));
    string[strlen(string) - 1] = '\0';
    strcpy(this_data->comment.ptr, string);
    this_data->comment.size = strlen(string);
    strcpy(this_data->filename, "Unsaved");
    strcpy(this_data->full_filename, "Unsaved");
    this_data->version = 5;
    this_data->valid = 1;
    this_data->bias = sample_bias;
    this_data->bias_range = sample_bias_range;
    this_data->i_setpoint = i_setpoint;
    this_data->i_set_range = i_set_range;
    this_data->amp_gain = tip_gain;
    memcpy(this_data->dosed_type.ptr, data->dosed_type.ptr, SAMPLETYPESIZE);
    memcpy(this_data->sample_type.ptr, data->sample_type.ptr, SAMPLETYPESIZE);
    this_data->sample_type.size = data->sample_type.size;
    this_data->dosed_type.size = data->dosed_type.size;
    this_data->dosed_langmuir = data->dosed_langmuir;
    this_data->temperature = data->temperature;
    this_data->move_time = move_time;
    this_data->write_time = write_time;
    this_data->move_bias = move_bias;
    this_data->write_bias = write_bias;
    this_data->move_i_setpoint = move_i_setpoint;
    this_data->write_i_setpoint = write_i_setpoint;
    this_data->pulse_duration = dep_pulse_duration;
    this_data->bias_speed_step = dep_pulse_bias_step;
/*
    if(dep_pulse_bias_step) {
        this_data->bias_speed = 0;
    }
    else 
*/
    this_data->bias_speed = dep_pulse_bias_speed;
    this_data->current_speed = dep_pulse_i_speed;
    this_data->z_offset_speed = dep_pulse_z_offset_speed;
    this_data->measure_before = dep_pulse_measure_before;
    this_data->scan_feedback = dep_pulse_feedback;
    switch(dep_pulse_freq) {
        case DEP_PULSE_FREQ_100:
            this_data->z_gain = Z_TENTH_GAIN;
            break;
        case DEP_PULSE_FREQ_1:
            this_data->z_gain = Z_ONE_GAIN;
            break;
        case DEP_PULSE_FREQ_10:
            this_data->z_gain = Z_TEN_GAIN;
            break;
    }
    this_data->delta_z_offset = dep_pulse_z_offset;
    this_data->z_offset_move = dep_pulse_use_move_bias;
    this_data->z_offset_crash_protect = dep_pulse_z_offset_crash_protect;
    this_data->measure_i_once = dep_pulse_measure_i_once;
    this_data->measure_i_wait = dep_pulse_measure_i_wait;
    this_data->feedback_wait = dep_pulse_feedback_wait;
    this_data->measure_i = dep_pulse_measure_i;
    this_data->measure_z = dep_pulse_measure_z;
    this_data->measure_2 = dep_pulse_measure_2;
    this_data->measure_3 = dep_pulse_measure_3;

/* version 4 */
    this_data->scan_num = dep_pulse_avg_pts;
    this_data->dep_pulse_ignore_initial = dep_pulse_ignore_initial;
    this_data->dep_pulse_change_time = dep_pulse_change_time;
    this_data->dep_pulse_avg_stop_pts = dep_pulse_avg_stop_pts;
    this_data->dep_pulse_after_time = dep_pulse_after_time;
    this_data->dep_pulse_percent_change = dep_pulse_percent_change;
    this_data->dep_pulse_duration_mode = dep_pulse_duration_mode;

/* version 5 */    
    this_data->dep_pulse_z_after = dep_pulse_z_after;
    this_data->dep_pulse_z_offset_overshoot = dep_pulse_z_offset_overshoot;
    this_data->dep_pulse_z_offset_overshoot_percentage = 
        (int)dep_pulse_z_offset_overshoot_percentage;
    this_data->dep_pulse_feedback_after = dep_pulse_feedback_after;
}

void get_data(datadef *this_data)
{
    // get dep settings from a datadef

    int different = 0;
    
    string[0] = '\0';
    if(this_data->bias_range != sample_bias_range) {
        sprintf(string, "Bias range, ");
        different = 1;
    }
    if(this_data->bias != sample_bias) {
        sprintf(string + strlen(string), "Bias, ");
        different = 1;
    }
    if(this_data->i_set_range != i_set_range) {
        sprintf(string + strlen(string), "Current range, ");
        different = 1;
    }
    if(this_data->i_setpoint != i_setpoint) {
        sprintf(string + strlen(string), "Current, ");
        different = 1;
    }
    if (this_data->amp_gain != tip_gain) {
        sprintf(string + strlen(string), "Amp gain, ");
        different = 1;
    }
    
    // warn user if datadef settings differ from "scan" settings
    if(different) {
        strcat(string," is/are different!");
        MessageBox(GetFocus(), string, "Warning", MB_OK);
    }
    
    move_time = this_data->move_time;
    write_time = this_data->write_time;
    move_bias = this_data->move_bias;
    write_bias = this_data->write_bias;
    move_i_setpoint = this_data->move_i_setpoint;
    write_i_setpoint = this_data->write_i_setpoint;
    dep_pulse_duration = this_data->pulse_duration;
    dep_pulse_bias_step = this_data->bias_speed_step;
    dep_pulse_measure_i = this_data->measure_i;
    dep_pulse_measure_z = this_data->measure_z;
    dep_pulse_measure_2 = this_data->measure_2;
    dep_pulse_measure_3 = this_data->measure_3;
    if (this_data->bias_speed == 0) {
        dep_pulse_bias_step = 1;
    }
    else dep_pulse_bias_speed = this_data->bias_speed;
    dep_pulse_i_speed = this_data->current_speed;
    dep_pulse_z_offset_speed = this_data->z_offset_speed;
    dep_pulse_measure_before = this_data->measure_before;
    dep_pulse_feedback = this_data->scan_feedback;

    if(this_data->z_gain == Z_TENTH_GAIN) {
            dep_pulse_freq = DEP_PULSE_FREQ_100;
    }
    else if(this_data->z_gain == Z_ONE_GAIN) {
            dep_pulse_freq = DEP_PULSE_FREQ_1;
    }
    else dep_pulse_freq = DEP_PULSE_FREQ_10;
/*
    switch(this_data->z_gain) {
	case Z_TENTH_GAIN:
            dep_pulse_freq = DEP_PULSE_FREQ_100;
	    break;
	case Z_ONE_GAIN:
            dep_pulse_freq = DEP_PULSE_FREQ_1;
	    break;
	case Z_TEN_GAIN:
            dep_pulse_freq = DEP_PULSE_FREQ_10;
	    break;
    }
*/
    dep_pulse_z_offset = this_data->delta_z_offset;
    dep_pulse_use_move_bias = this_data->z_offset_move;
    dep_pulse_z_offset_crash_protect = this_data->z_offset_crash_protect;
    dep_pulse_measure_i_once = this_data->measure_i_once;
    dep_pulse_measure_i_wait = this_data->measure_i_wait;
    dep_pulse_feedback_wait = this_data->feedback_wait;
    dep_pulse_avg_pts = this_data->scan_num;
    dep_pulse_ignore_initial = this_data->dep_pulse_ignore_initial;
    dep_pulse_change_time = this_data->dep_pulse_change_time;
    dep_pulse_avg_stop_pts = this_data->dep_pulse_avg_stop_pts;
    dep_pulse_after_time = this_data->dep_pulse_after_time;
    dep_pulse_percent_change = this_data->dep_pulse_percent_change;
    dep_pulse_duration_mode = this_data->dep_pulse_duration_mode;
    dep_pulse_z_after = this_data->dep_pulse_z_after;
    dep_pulse_z_offset_overshoot = this_data->dep_pulse_z_offset_overshoot;
    dep_pulse_z_offset_overshoot_percentage = (float)
        this_data->dep_pulse_z_offset_overshoot_percentage;
    dep_pulse_feedback_after = this_data->dep_pulse_feedback_after;
}

void repaint_oscill_data(HWND hDlg, int ch)
{
    // repaint the graph corresponding to channel ch

    HDC dep_oscill_hdc;
    int j;
    int x1, y1, x2, y2; // boundary coordinates of painting area, in pixels
    float y_min = (float)IN_MIN; // range of vertical axis, in bits
    float y_max = (float)IN_MAX;
    float start_time, total_time;
    int xs, ys; // coordinates of current data point, in pixels
    float yfs;
        
    if(zoomed_in && dep_current_channel != ch) return;

    if(!zoomed_in) {
        x1 = SMALL_BORDER_SIZE + (ch % 2) * (SMALL_IMAGE_SIZE + SMALL_BORDER_SIZE);
        y1 = SMALL_BORDER_SIZE + (ch / 2) * (SMALL_IMAGE_SIZE + SMALL_BORDER_SIZE);
        x2 = x1 + SMALL_IMAGE_SIZE;
        y2 = y1 + SMALL_IMAGE_SIZE;
    }
    else {
        x1 = LARGE_BORDER_SIZE;
        y1 = LARGE_BORDER_SIZE;
        x2 = x1 + LARGE_IMAGE_SIZE;
        y2 = y1 + LARGE_IMAGE_SIZE;
    }

    dep_oscill_hdc = GetDC(hDlg);
    hpen_old = SelectObject(dep_oscill_hdc, hpen_white);
    Rectangle(dep_oscill_hdc, x1, y1, x2, y2);
    repaint_grid(hDlg, x1, x2, y1, y2);

    y_min = dep_vert_min[ch];
    y_max = dep_vert_max[ch];

    if(dep_oscill_data[ch] != NULL) 
    {
        switch(ch)
        {
            case 0:
                SelectObject(dep_oscill_hdc, hpen_red);
                break;
            case 1:
                SelectObject(dep_oscill_hdc, hpen_green);
                break;
            case 2:
                SelectObject(dep_oscill_hdc, hpen_blue);
                break;
            case 3:
                SelectObject(dep_oscill_hdc, hpen_black);
                break;
        }       

        start_time = *(dep_oscill_data[ch]->time2d);
        total_time = *(dep_oscill_data[ch]->time2d +
                dep_oscill_data[ch]->size-1) - start_time;

	// move to starting position
        xs = (int)(x1 + (*((dep_oscill_data[ch])->time2d) - start_time) *
		(x2 - x1 + 1) / total_time);
	yfs = y1 - (*((dep_oscill_data[ch])->data2d) - y_max) *
		(y2 - y1 + 1) / (y_max - y_min + 1);
	ys = (int)yfs;
/*
            ys=-(*((dep_oscill_data[ch])->data2d+j-1)
                -IN_MAX)*(y2-y1+1)/(IN_MAX-
                IN_MIN+1)+y1;
*/
        xs = max(min(xs, x2), x1);
        ys = max(min(ys, y2), y1);
//        MoveTo(dep_oscill_hdc, xs, ys);
        MoveToEx(dep_oscill_hdc, xs, ys, NULL);

        for(j=1; j < dep_oscill_data[ch]->size; j++)
        {                        
	    // draw a line to the next data position
            xs = (int)(x1 + (*((dep_oscill_data[ch])->time2d + j) - start_time) *
		    (x2 - x1 + 1) / total_time);
	    yfs = y1 - (*((dep_oscill_data[ch])->data2d + j) - y_max) *
		    (y2 - y1 + 1) / (y_max - y_min + 1);
	    ys = (int)yfs;
/*
            ys=-(*((dep_oscill_data[ch])->data2d+j)
                -IN_MAX)*(y2-y1+1)/(IN_MAX-
                IN_MIN+1)+y1;
*/
            xs = max(min(xs, x2), x1);
            ys = max(min(ys, y2), y1);
            LineTo(dep_oscill_hdc, xs, ys);
        }

    }
    SelectObject(dep_oscill_hdc, hpen_old);
    ReleaseDC(hDlg, dep_oscill_hdc);
}

void dep_box(HWND hDlg)
{
    int Imin = 0, Imax = 0;
    unsigned int moved_to;
    
    set_move_cond(hDlg);
    Imax=in_vtod(pow(10,SD->tip_spacing-dtov(i_setpoint,i_set_range)/10.0));
    Imin=in_vtod(pow(10,-SD->tip_spacing-dtov(i_setpoint,i_set_range)/10.0));
    SetDlgItemText(hDlg,DEP_STATUS,"Moving to edge.");
/*
    move_to_speed2(x_ch,dac_data[x_ch], ZERO - scan_step*scan_size/2,
        y_ch,dac_data[y_ch],ZERO-scan_step*scan_size/2,
        move_time, 0, Imin,Imax);
*/
    if ((moved_to=move_to_protect2(x_ch,dac_data[x_ch], ZERO-scan_step*scan_size/2,
            move_time-ONE_INPUT_TIME-INPUT_SWITCH_TIME,CRASH_STOP,SD->z_limit_percent,0,
            Imin,Imax,0,0,DIGITAL_FEEDBACK_MAX,DIGITAL_FEEDBACK_REREAD))!=
            ZERO-scan_step*scan_size/2)
    {
        set_move_cond(hDlg);
        SetDlgItemText(hDlg,DEP_STATUS,"Crash protection in x move. Returned to origin!");
        move_to_protect2(x_ch,moved_to,dac_data[x_ch],move_time-ONE_INPUT_TIME-INPUT_SWITCH_TIME,
            CRASH_AUTO_Z, SD->z_limit_percent,0,Imin,Imax,0,1,DIGITAL_FEEDBACK_MAX,DIGITAL_FEEDBACK_REREAD);
        goto DEP_BOX_END;
    }
    if ((moved_to=move_to_protect2(y_ch,dac_data[y_ch], ZERO-scan_step*scan_size/2,
            move_time-ONE_INPUT_TIME-INPUT_SWITCH_TIME,CRASH_STOP,
            SD->z_limit_percent,0,Imin,Imax,0,0,DIGITAL_FEEDBACK_MAX,DIGITAL_FEEDBACK_REREAD))!=
            ZERO-scan_step*scan_size/2)
    {
        set_move_cond(hDlg);
        SetDlgItemText(hDlg,DEP_STATUS,"Crash protection in y move. Returned to origin!");
        move_to_protect2(x_ch,ZERO-scan_step*scan_size/2,dac_data[x_ch],move_time-ONE_INPUT_TIME-INPUT_SWITCH_TIME,
            CRASH_AUTO_Z, SD->z_limit_percent,0,Imin,Imax,0,1,DIGITAL_FEEDBACK_MAX,DIGITAL_FEEDBACK_REREAD);
        
        move_to_protect2(y_ch,moved_to,dac_data[y_ch],move_time-ONE_INPUT_TIME-INPUT_SWITCH_TIME,
            CRASH_AUTO_Z, SD->z_limit_percent,0,Imin,Imax,0,1,DIGITAL_FEEDBACK_MAX,DIGITAL_FEEDBACK_REREAD);
        goto DEP_BOX_END;
    }
    set_write_cond(hDlg,write_bias,write_i_setpoint);
    SetDlgItemText(hDlg,DEP_STATUS,"Writing first side...");
    if ((moved_to=move_to_protect2(y_ch,ZERO-scan_step*scan_size/2,
        ZERO+scan_step*scan_size/2-1,
        write_time-ONE_INPUT_TIME-INPUT_SWITCH_TIME, CRASH_STOP,
        SD->z_limit_percent,0, Imin,Imax,0,0,DIGITAL_FEEDBACK_MAX,DIGITAL_FEEDBACK_REREAD))!=
            ZERO+scan_step*scan_size/2-1)
    {
        set_move_cond(hDlg);
        SetDlgItemText(hDlg,DEP_STATUS,"Crash protection in first side. Returned to origin!");
        move_to_protect2(x_ch,ZERO-scan_step*scan_size/2,dac_data[x_ch],move_time-ONE_INPUT_TIME-INPUT_SWITCH_TIME,
            CRASH_AUTO_Z, SD->z_limit_percent,0,Imin,Imax,0,1,DIGITAL_FEEDBACK_MAX,DIGITAL_FEEDBACK_REREAD);
        
        move_to_protect2(y_ch,moved_to,dac_data[y_ch],move_time-ONE_INPUT_TIME-INPUT_SWITCH_TIME,
            CRASH_AUTO_Z, SD->z_limit_percent,0,Imin,Imax,0,1,DIGITAL_FEEDBACK_MAX,DIGITAL_FEEDBACK_REREAD);
        goto DEP_BOX_END;
    }
        
    
    SetDlgItemText(hDlg,DEP_STATUS,"Writing second side...");
    
    if ((moved_to=move_to_protect2(x_ch,ZERO-scan_step*scan_size/2,
        ZERO+scan_step*scan_size/2-1,
        write_time-ONE_INPUT_TIME-INPUT_SWITCH_TIME, CRASH_STOP,SD->z_limit_percent,0, Imin,Imax,0,0,DIGITAL_FEEDBACK_MAX,DIGITAL_FEEDBACK_REREAD))!=
            ZERO+scan_step*scan_size/2-1)
    {
        set_move_cond(hDlg);
        SetDlgItemText(hDlg,DEP_STATUS,"Crash protection in second side. Returned to origin!");
        move_to_protect2(y_ch,ZERO+scan_step*scan_size/2-1,dac_data[y_ch],move_time-ONE_INPUT_TIME-INPUT_SWITCH_TIME,
            CRASH_AUTO_Z, SD->z_limit_percent,0,Imin,Imax,0,1,DIGITAL_FEEDBACK_MAX,DIGITAL_FEEDBACK_REREAD);
        
        move_to_protect2(x_ch,moved_to,dac_data[x_ch],move_time-ONE_INPUT_TIME-INPUT_SWITCH_TIME,
            CRASH_AUTO_Z, SD->z_limit_percent,0,Imin,Imax,0,1,DIGITAL_FEEDBACK_MAX,DIGITAL_FEEDBACK_REREAD);
        goto DEP_BOX_END;
    }
    SetDlgItemText(hDlg,DEP_STATUS,"Writing third side...");
    
    if ((moved_to=move_to_protect2(y_ch,ZERO+scan_step*scan_size/2-1,
        ZERO-scan_step*scan_size/2,
        write_time-ONE_INPUT_TIME-INPUT_SWITCH_TIME, CRASH_STOP,SD->z_limit_percent,0, Imin,Imax,0,0,DIGITAL_FEEDBACK_MAX,DIGITAL_FEEDBACK_REREAD))!=
            ZERO-scan_step*scan_size/2)
    {
        set_move_cond(hDlg);
        SetDlgItemText(hDlg,DEP_STATUS,"Crash protection in third side. Returned to origin!");
        move_to_protect2(x_ch,ZERO+scan_step*scan_size/2-1,dac_data[x_ch],move_time-ONE_INPUT_TIME-INPUT_SWITCH_TIME,
            CRASH_AUTO_Z, SD->z_limit_percent,0,Imin,Imax,0,1,DIGITAL_FEEDBACK_MAX,DIGITAL_FEEDBACK_REREAD);
        
        move_to_protect2(y_ch,moved_to,dac_data[y_ch],move_time-ONE_INPUT_TIME-INPUT_SWITCH_TIME,
            CRASH_AUTO_Z, SD->z_limit_percent,0,Imin,Imax,0,1,DIGITAL_FEEDBACK_MAX,DIGITAL_FEEDBACK_REREAD);
        goto DEP_BOX_END;
    }
    SetDlgItemText(hDlg,DEP_STATUS,"Writing fourth side...");
    
    if ((moved_to=move_to_protect2(x_ch,ZERO+scan_step*scan_size/2-1,
        ZERO-scan_step*scan_size/2,
        write_time-ONE_INPUT_TIME-INPUT_SWITCH_TIME, CRASH_STOP,SD->z_limit_percent,0, Imin,Imax,0,0,DIGITAL_FEEDBACK_MAX,DIGITAL_FEEDBACK_REREAD))!=
            ZERO-scan_step*scan_size/2)
    {
        set_move_cond(hDlg);
        SetDlgItemText(hDlg,DEP_STATUS,"Crash protection in fourth side. Returned to origin!");
        move_to_protect2(y_ch,ZERO-scan_step*scan_size/2,dac_data[y_ch],move_time-ONE_INPUT_TIME-INPUT_SWITCH_TIME,
            CRASH_AUTO_Z, SD->z_limit_percent,0,Imin,Imax,0,1,DIGITAL_FEEDBACK_MAX,DIGITAL_FEEDBACK_REREAD);
        
        move_to_protect2(x_ch,moved_to,dac_data[x_ch],move_time-ONE_INPUT_TIME-INPUT_SWITCH_TIME,
            CRASH_AUTO_Z, SD->z_limit_percent,0,Imin,Imax,0,1,DIGITAL_FEEDBACK_MAX,DIGITAL_FEEDBACK_REREAD);
        goto DEP_BOX_END;
    }
    set_move_cond(hDlg);
    SetDlgItemText(hDlg,DEP_STATUS,"Moving back to origin.");
    move_to_protect2(x_ch,ZERO-scan_step*scan_size/2,dac_data[x_ch],move_time-ONE_INPUT_TIME-INPUT_SWITCH_TIME,
        CRASH_AUTO_Z, SD->z_limit_percent,0,Imin,Imax,0,1,DIGITAL_FEEDBACK_MAX,DIGITAL_FEEDBACK_REREAD);
    move_to_protect2(y_ch,ZERO-scan_step*scan_size/2,dac_data[y_ch],move_time-ONE_INPUT_TIME-INPUT_SWITCH_TIME,
        CRASH_AUTO_Z, SD->z_limit_percent,0,Imin,Imax,0,1,DIGITAL_FEEDBACK_MAX,DIGITAL_FEEDBACK_REREAD);
DEP_BOX_END:    
    set_scan_cond(hDlg);
}

void dep_lines(HWND hDlg)
{
    int Imin = 0,Imax = 0;
    unsigned int current_bias, current_i_setpoint;
    unsigned int current_x,next_x,edge,y1,y2;
    int total_bits,i;
    float current_i_set;
    unsigned int moved_to;
    int current_write_time;
    
    y1=ZERO-scan_size*scan_step/2;
    y2=ZERO+scan_size*scan_step/2-1;
    set_move_cond(hDlg);
    SetDlgItemText(hDlg,DEP_STATUS,"Moving to bottom");
/*
    move_to_timed(y_ch,dac_data[y_ch],y1,
        move_time);
*/
    if ((moved_to=move_to_protect2(y_ch,dac_data[y_ch],y1,move_time-ONE_INPUT_TIME-INPUT_SWITCH_TIME,
        CRASH_STOP,SD->z_limit_percent,0,Imin,Imax,0,0,DIGITAL_FEEDBACK_MAX,DIGITAL_FEEDBACK_REREAD))!=y1)
    {
        SetDlgItemText(hDlg,DEP_STATUS,"Crash protection moving to bottom. Returned to origin!");
        move_to_protect2(y_ch,moved_to,dac_data[y_ch],move_time-ONE_INPUT_TIME-INPUT_SWITCH_TIME,
            CRASH_AUTO_Z,SD->z_limit_percent,0,Imin,Imax,0,1,DIGITAL_FEEDBACK_MAX,DIGITAL_FEEDBACK_REREAD);
        goto DEP_LINES_END2;
        
    }
    total_bits=scan_step*scan_size; 
    current_x=dac_data[x_ch];
    edge=current_x-total_bits/2;
    for(i=0;i<dep_num_lines;i++)
    {
        next_x=(i+1)*total_bits/(dep_num_lines+1)+edge;
        if (next_x > MAX) next_x=MAX;
        if (next_x <= MIN) next_x=MIN; // probably unnecessary/useless, since next_x is unsigned int and MIN==0
        sprintf(string,"Moving to line %d",(i+1));
        wprint(DEP_STATUS);
        if ((moved_to=move_to_protect2(y_ch,y1,y2,move_time-ONE_INPUT_TIME-INPUT_SWITCH_TIME,
            CRASH_STOP,SD->z_limit_percent,0,Imin,Imax,0,0,DIGITAL_FEEDBACK_MAX,DIGITAL_FEEDBACK_REREAD))!=y2)
        {
            set_move_cond(hDlg);
            SetDlgItemText(hDlg,DEP_STATUS,"Crash protection y! Returned to origin!");
            move_to_protect2(y_ch,moved_to,y1,move_time-ONE_INPUT_TIME-INPUT_SWITCH_TIME,
                CRASH_AUTO_Z,SD->z_limit_percent,0,Imin,Imax,0,1,DIGITAL_FEEDBACK_MAX,DIGITAL_FEEDBACK_REREAD);
            goto DEP_LINES_END;
        
        }
        if ((current_x=move_to_protect2(x_ch,current_x,next_x,move_time-ONE_INPUT_TIME-INPUT_SWITCH_TIME,
            CRASH_STOP,SD->z_limit_percent,0,Imin,Imax,0,0,DIGITAL_FEEDBACK_MAX,DIGITAL_FEEDBACK_REREAD))!=next_x)
        {
            set_move_cond(hDlg);
            SetDlgItemText(hDlg,DEP_STATUS,"Crash protection x! Returned to origin!");
            goto DEP_LINES_END;
        
        }
/*
        move_to_timed(y_ch,y1,y2,move_time);                 
        move_to_timed(x_ch,current_x,next_x,move_time);
        current_x=next_x;
*/
        dio_start_clock(SD->inter_line_delay);
        if (dep_num_lines>1)
        {
            if (lines_max_bias>write_bias)
            {
                current_bias=write_bias+(lines_max_bias-write_bias)
                    *i/(dep_num_lines-1);
            }
            else
            {
                current_bias=write_bias-(write_bias-lines_max_bias)
                    *i/(dep_num_lines-1);
            }
            current_i_set=(float)(  write_i_set*pow(lines_max_i_set/write_i_set,
                ((double)i)/((double)(dep_num_lines-1)))  );
            current_write_time=(int)( write_time*pow((double)max_write_time/(double)write_time,
                ((double)i)/((double)(dep_num_lines-1))) );
/*
            if (lines_max_i_set>write_i_set)
            {
                current_i_set=write_i_set+(lines_max_i_set-write_i_set)*
                    i/(dep_num_lines-1);
            }
            else
            {
                current_i_set=write_i_set-(write_i_set-lines_max_i_set)*
                    i/(dep_num_lines-1);
            }
            if (max_write_time>write_time)
            {
                current_write_time=write_time+(max_write_time-write_time)*
                    i/(dep_num_lines-1);
            }
            else
            {
                current_write_time=write_time-(write_time-max_write_time)*
                    i/(dep_num_lines-1);
            }
*/            
        }
        else 
        {
            current_bias=write_bias;
            current_i_set=write_i_set;
            current_write_time=write_time;
        }
        if (current_bias>MAX) current_bias=MAX;
        current_i_setpoint=calc_i_setpoint(current_i_set,i_set_range,tip_gain);
        dio_wait_clock();
        set_write_cond(hDlg,current_bias,current_i_setpoint);
        sprintf(string,"Writing line %d",(i+1));
        wprint(DEP_STATUS);
/*
        move_to_timed(y_ch,y2,y1,write_time);
*/
        if ((moved_to=move_to_protect2(y_ch,y2,y1,current_write_time-ONE_INPUT_TIME-INPUT_SWITCH_TIME,
            CRASH_STOP,SD->z_limit_percent,0,Imin,Imax,0,0,DIGITAL_FEEDBACK_MAX,DIGITAL_FEEDBACK_REREAD))!=y1)
        {
            set_move_cond(hDlg);
            SetDlgItemText(hDlg,DEP_STATUS,"Crash protection y! Returned to origin!");
            move_to_protect2(y_ch,moved_to,y1,move_time-ONE_INPUT_TIME-INPUT_SWITCH_TIME,
                CRASH_AUTO_Z,SD->z_limit_percent,0,Imin,Imax,0,1,DIGITAL_FEEDBACK_MAX,DIGITAL_FEEDBACK_REREAD);
            goto DEP_LINES_END;
        
        }
        set_move_cond(hDlg);
    }
    SetDlgItemText(hDlg,DEP_STATUS,"Moving to origin.");
DEP_LINES_END:
/*
    move_to_timed(x_ch,current_x,dac_data[x_ch],move_time);
    move_to_timed(y_ch,y1,dac_data[y_ch],move_time);
*/
    move_to_protect2(x_ch,current_x,dac_data[x_ch],move_time-ONE_INPUT_TIME-INPUT_SWITCH_TIME,
        CRASH_AUTO_Z,SD->z_limit_percent,0,Imin,Imax,0,1,DIGITAL_FEEDBACK_MAX,DIGITAL_FEEDBACK_REREAD);
    move_to_protect2(y_ch,y1,dac_data[y_ch],move_time-ONE_INPUT_TIME-INPUT_SWITCH_TIME,
        CRASH_AUTO_Z,SD->z_limit_percent,0,Imin,Imax,0,1,DIGITAL_FEEDBACK_MAX,DIGITAL_FEEDBACK_REREAD);
DEP_LINES_END2:     
    set_scan_cond(hDlg);
}

void set_write_cond(HWND hDlg, unsigned int writing_bias, unsigned int writing_i_setpoint)
{
    // sets the sample bias and tunneling current to writing values
    int bias_time;    
    int bias_bits = 1;
    
    bias_time = (int)(sample_bias_range * 10 / 4.096 / dep_pulse_bias_speed);
    while(bias_time < 3 * CLOCK_DEAD_TIME)
    {
        bias_bits++;
        bias_time = (int)(bias_bits * sample_bias_range * 10 / 4.096 / dep_pulse_bias_speed);
    }
    SetDlgItemText(hDlg, DEP_STATUS, "Bias is changed for writing.");
    ramp_bias(writing_bias, bias_time, BIAS_RAMP_SKIP, bias_bits);
    SetDlgItemText(hDlg, DEP_STATUS, "Current is changed for writing.");
    move_to_timed(i_setpoint_ch, dac_data[i_setpoint_ch], writing_i_setpoint, DEP_CURRENT_BIT_TIME);
    dac_data[i_setpoint_ch] = writing_i_setpoint;
}

void set_move_cond(HWND hDlg)
{
    // sets the sample bias and tunneling current to movement values
    int bias_time;
    int bias_bits = 1;

    bias_time = (int)(sample_bias_range * 10 / 4.096 / dep_pulse_bias_speed);
    while(bias_time < 3 * CLOCK_DEAD_TIME)
    {
        bias_bits++;
        bias_time = (int)(bias_bits * sample_bias_range * 10 / 4.096 / dep_pulse_bias_speed);
    }
    SetDlgItemText(hDlg, DEP_STATUS, "Bias is changed for moving.");
    ramp_bias(move_bias, bias_time, BIAS_RAMP_SKIP, bias_bits);
    SetDlgItemText(hDlg, DEP_STATUS, "Current is changed for moving.");
    move_to_timed(i_setpoint_ch, dac_data[i_setpoint_ch], move_i_setpoint, DEP_CURRENT_BIT_TIME);
    dac_data[i_setpoint_ch] = move_i_setpoint;
}

void set_scan_cond(HWND hDlg)
{
    // restores the sample bias and tunneling current to scanning (pre-dep) values
    int bias_time;
    int bias_bits = 1;

    bias_time =(int) (sample_bias_range * 10 / 4.096 / dep_pulse_bias_speed);
    while(bias_time < 3 * CLOCK_DEAD_TIME)
    {
        bias_bits++;
        bias_time = (int)(bias_bits * sample_bias_range * 10 / 4.096 / dep_pulse_bias_speed);
    }
    SetDlgItemText(hDlg, DEP_STATUS, "Done. Bias set back for scanning.");
    ramp_bias(sample_bias, bias_time, BIAS_RAMP_SKIP, bias_bits);
    SetDlgItemText(hDlg, DEP_STATUS, "Done. Current is changed for scanning.");
    move_to_timed(i_setpoint_ch, dac_data[i_setpoint_ch], i_setpoint, DEP_CURRENT_BIT_TIME);
    dac_data[i_setpoint_ch] = i_setpoint;
}

void repaint_all(HWND hDlg)
{
    CheckDlgButton(hDlg, dep_mode, 1);
    sprintf(string, "%d", dep_num_lines);
    wprint(DEP_NUM_LINES);
    repaint_move_time(hDlg);
    repaint_write_time(hDlg);
    repaint_max_write_time(hDlg);
    repaint_move_bias(hDlg);
    repaint_write_bias(hDlg);
    repaint_write_current(hDlg);
    repaint_move_current(hDlg);
    repaint_lines_current(hDlg);
    repaint_lines_bias(hDlg);
    repaint_rect(hDlg, 1, dep_current_channel, SMALL_BORDER_SIZE, SMALL_IMAGE_SIZE);
    repaint_vert_scale(hDlg);
    repaint_vert_scale_min(hDlg);
    dep_set_title(hDlg);
	repaint_pulse_z_offset(hDlg);
	repaint_dep_trigger(hDlg);
	
}    

void repaint_images(HWND hDlg)
{
    int channel;

    if(!zoomed_in) repaint_rect(hDlg, 1, dep_current_channel, SMALL_BORDER_SIZE, SMALL_IMAGE_SIZE);
    for(channel = 0; channel < DIO_INPUT_CHANNELS; channel++) repaint_oscill_data(hDlg, channel);
}

static void repaint_pulse_avg_pts(HWND hDlg)
{
    sprintf(string, "%d", dep_pulse_avg_pts);
    SetDlgItemText(hDlg, DEP_PULSE_AVG_PTS, string);
}

static void dep_set_title(HWND hDlg)
{
      if(dep_oscill_data[dep_current_channel]!=NULL)
      if(dep_oscill_data[dep_current_channel]->valid)
      {
        sprintf(string,"Deposition: %s", dep_oscill_data[dep_current_channel]->filename);
        SetWindowText(hDlg,string);
      }
      else SetWindowText(hDlg, "Deposition");
}

static void repaint_vert_scale(HWND hDlg)
{
    out_smart=1;
    sprintf(string, "%0.3f", in_dtov(dep_vert_max[dep_current_channel]));
    SetDlgItemText(hDlg, DEP_V_SCALE_MAX, string);
    SetScrollPos(GetDlgItem(hDlg, DEP_V_SCALE_MAX_SCROLL), SB_CTL,
            DTOI(dep_vert_max[dep_current_channel], dep_vert_min_min, dep_vert_max_max), TRUE);
    out_smart=0;
}

static void repaint_vert_scale_min(HWND hDlg)
{
    out_smart=1;
    sprintf(string, "%0.3f", in_dtov(dep_vert_min[dep_current_channel]));
    SetDlgItemText(hDlg, DEP_V_SCALE_MIN, string);
    SetScrollPos(GetDlgItem(hDlg, DEP_V_SCALE_MIN_SCROLL), SB_CTL,
            DTOI(dep_vert_min[dep_current_channel], dep_vert_min_min, dep_vert_max_max), TRUE);
    out_smart=0;
}

#ifdef OLD
void repaint_scan_data(HWND hDlg,int channel)
{
    HDC hDC;
    int xpos,ypos;
    int screen_size;

    if (!zoomed_in)
    {
        xpos=SMALL_BORDER_SIZE+(channel % 2)*(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE);
        ypos=SMALL_BORDER_SIZE+(channel/2)*(SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE);
        screen_size=SMALL_IMAGE_SIZE;
    }
    else
    {
        xpos=LARGE_BORDER_SIZE;
        ypos=LARGE_BORDER_SIZE;
        screen_size=LARGE_IMAGE_SIZE;
    }
    if (zoomed_in && dep_current_channel!=channel) return;
    hDC=GetDC(hDlg);
    SelectPalette(hDC,hPal,0);
    RealizePalette(hDC);
    SetMapMode(hDC,MM_TEXT);
    if (data->valid)
    {
        bitmapinfo->bmiHeader.biBitCount = 8;
        bitmapinfo->bmiHeader.biWidth=bitmap_size;
        bitmapinfo->bmiHeader.biHeight=bitmap_size;
        StretchDIBits(hDC, xpos, ypos, screen_size, screen_size, 
            0, 0, data->size,data->size,
            bitmap_data, bitmapinfo, DIB_PAL_COLORS,SRCCOPY);
/*
        if (SelOn) 
        {
            if (sel_r1.image==number) repaint_sel(hDlg,&sel_r1);
            if (sel_r2.image==number) repaint_sel(hDlg,&sel_r2);
        }
*/
                    
    }
    else
    {
        clear_area(hDlg,xpos,ypos,xpos+screen_size,ypos+screen_size,
            RGB(0,0,0));
    }
    ReleaseDC(hDlg,hDC);
}
#endif

