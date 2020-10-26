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
#include "highres.h"
#include "summary.h"
#include "pre_scan.h"

extern HANDLE hInst;
extern char string[];
extern char *current_file_spc;
extern int file_spc_count;
extern struct commentdef *gcomment;
extern unsigned int dac_data[];
extern int inter_step_delay,step_delay;
extern int z_offset_range;
extern unsigned int scan_z;
extern datadef *data;
extern unsigned int input_ch,out1;
extern unsigned int tip_gain;
extern unsigned int i_setpoint;
extern int i_set_range;
extern int sample_bias_range;
extern int sp_feedback_at_min;
extern int sp_scan_min;
extern int sp_scan_max;
extern int sp_out_range; /* 1 is +-5 V, 2 is +-10V */
extern int sp_input_ch;
extern int sp_output_ch;
extern int sp_p1_vbits;
extern int sp_p2_vbits;
//extern int sp_background;
//extern float sp_p1_press;
//extern float sp_p2_press;
//extern float sp_p1_calib;
//extern float sp_p2_calib;
extern int sp_speed;
extern int sp_move_delay;
extern int sp_ramp_delay;
extern int sp_dither_wait;
extern int sp_feedback_wait;
//Note: extern pass delay constants
extern int sp_start_delay[];// Mod. 1 Shaowei
extern int sp_start_delay_on[];// Mod. 1 Shaowei
extern int sp_fb_delay[];// Mod. 3 Shaowei
extern int sp_fb_delay_on[];// Mod. 3 Shaowei
extern int sp_step_delay[];// Mod. 5 Shaowei
extern int sp_step_delay_on[];// Mod. 5 Shaowei
extern int sp_final_delay[];// Mod. 4 Shaowei
extern int sp_final_delay_on[];// Mod. 4 Shaowei
//extern float sp_tot_integral;
//extern float sp_part_integral;
//extern int sp_interval; /* interval between steps in microsec */
extern int sp_screen_min;
extern int sp_screen_max;
extern int sp_scanning;
extern int sp_first_time;
extern int sp_crash_protection;
//extern int sp_didv;
extern int sp_z_offset;
extern unsigned int sp_num_samples;
extern int sp_num_passes;
//Note: extern int for trigger
extern unsigned int sp_trigger_ch;//Mod. 2 Shaowei
extern int sp_steptrigger; //Mod. 5 Shaowei
extern int sp_trigger; //Mod. 2 Shaowei
extern int sp_trigger_low;//Mod. 2 Shaowei
extern int sp_trigger_high;//Mod. 2 Shaowei
extern int sp_trigger_time; //Mod. 2 Shaowei
//extern int sp_measure;
extern int sp_measure_i;
extern int sp_measure_z;
extern int sp_measure_2;
extern int sp_measure_3;
extern int sp_forward;
extern int sp_backward;
extern int sp_avg_both;
extern int sp_dither0;
extern int sp_dither1;
extern datadef *sp_data[];
extern int sp_current;
extern datadef **glob_data;
extern unsigned int sample_bias;
extern float sp_max,sp_min;
extern int sp_track_offset_x,sp_track_offset_y;
extern int sp_track_limit_x_min,sp_track_limit_x_max;
extern int sp_track_limit_y_min,sp_track_limit_y_max;
extern int sp_track_max_bits;
extern int sp_track_at_min;
extern int sp_offset_hold;
extern int sp_track_sample_every;
extern int sp_tracking;
extern int sp_tracking_mode;
extern int sp_track_auto_auto;
extern int sp_track_avg_data_pts;
extern int sp_track_iterations;
extern int sp_track_every;
extern float sp_track_plane_a;
extern float sp_track_plane_b;
extern int sp_feedback_every;
extern int sp_bits_per_step;
extern int sp_z_offset_bias;
extern int sp_use_z_offset_bias;
extern int sp_feedback;
extern int sp_current_default;
extern datadef *sp_defaults_data[];
extern int sp_mode;
extern int sp_x,sp_y;
extern int sp_track_step_delay;
extern int sp_rel_min,sp_rel_max;

extern unsigned int bit16;

static int out_smart = 0;

//read seq variables
int sp_current_step = 0;
char cur_rs_type[STEP_TYPE_STR_MAX];
SAMPLELIST rs_types_list;
extern int sp_use_readsq;

static void repaint_z_offset(HWND);
static void recalc_track_limits();
static void repaint_track_max_bits();
static void sp_scan_enable(HWND,int);
static void sp_enable_abs_rel(HWND);
void calc_scan();
static void repaint_scan_min(HWND);
static void repaint_scan_max(HWND);
static void repaint_scan_min2(HWND);
static void repaint_scan_max2(HWND);
static void repaint_sp_z_offset_bias(HWND);
static void repaint_num_samples(HWND);
static void repaint_trigger_ch(HWND);//Mod. 2 Shaowei
static void repaint_trigger_low(HWND);
static void repaint_trigger_high(HWND);//End Mod. 2
static void repaint_num_passes(HWND);
static void repaint_sp_v_speed(HWND);
static void repaint_all(HWND);
static void repaint_track_current(HWND);
static void repaint_feedback(HWND);
void sp_calc_scan();
static void repaint_sp_trigger(HWND); //Mod. Trigger Shaowei

//read sequence definitions
static void repaint_this_sequence(HWND);
static void add_sequence(HWND);
static void insert_sequence(HWND);
static void del_sequence();
static void repaint_sequence_num(HWND);
static void summary(HWND);
static void make_rs_types_list();
static void free_rs_types_list();


BOOL FAR PASCAL SpecOptDlg(HWND,unsigned,WPARAM,LPARAM);
// Spectroscopy Options dialog is used to set spectroscopic scan parameters

BOOL FAR PASCAL SpecOptDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{
    int id;
    int i,mini,maxi,delt,ddelt;
    double d,minid,maxid,deltd,ddeltd;
    int old_output_ch;

	static FARPROC lpfnDlgProc;

    switch(Message) {
      case WM_INITDIALOG:
        {
            SetScrollRange(GetDlgItem(hDlg,SP_MIN_SCROLL),SB_CTL,0,32767,TRUE);
            SetScrollRange(GetDlgItem(hDlg,SP_MAX_SCROLL),SB_CTL,0,32767,TRUE);
            SetScrollRange(GetDlgItem(hDlg,SP_MIN_SCROLL2),SB_CTL,0,32767,TRUE);
            SetScrollRange(GetDlgItem(hDlg,SP_MAX_SCROLL2),SB_CTL,0,32767,TRUE);
            SetScrollRange(GetDlgItem(hDlg,SP_Z_OFFSET_BIAS_SCROLL),SB_CTL,0,32767,TRUE);
//            SetScrollRange(GetDlgItem(hDlg,SP_VOLTS_SCROLL),SB_CTL,0,32767,TRUE);
            SetScrollRange(GetDlgItem(hDlg,SP_NUM_SAMPLES_SCROLL),SB_CTL,0,32767,TRUE);
            SetScrollRange(GetDlgItem(hDlg,SP_NUM_PASSES_SCROLL),SB_CTL,0,32767,TRUE);
            SetScrollRange(GetDlgItem(hDlg,SP_DELTAZ_SCROLL),SB_CTL,0,32767,TRUE);
            SetScrollRange(GetDlgItem(hDlg,SP_TRACK_MAX_BITS_SCROLL),SB_CTL,0,MAX / 2,FALSE);
			SetScrollRange(GetDlgItem(hDlg,SP_TRIGGER_CH_SCROLL),SB_CTL,0,32767,TRUE);
			SetScrollRange(GetDlgItem(hDlg,SP_TRIGGER_LOW_SCROLL),SB_CTL,0,32767,TRUE);
			SetScrollRange(GetDlgItem(hDlg,SP_TRIGGER_HIGH_SCROLL),SB_CTL,0,32767,TRUE);
            
            repaint_all(hDlg);

			make_rs_types_list();
			init_listbox(hDlg,SP_RS_TYPE,&(rs_types_list.listbox));
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
                case SP_TRACK_MAX_BITS_SCROLL:
                    i = sp_track_max_bits;
                    mini = 0;
                    maxi = MAX / 2;
                    delt = TRACK_MAX_BITS_DELT;
                    ddelt = TRACK_MAX_BITS_DDELT;
                    break;
                case SP_NUM_SAMPLES_SCROLL:
                    d = (double)sp_num_samples;
                    minid = SP_NUM_SAMPLES_MIN;
                    maxid = SP_NUM_SAMPLES_MAX;
                    deltd = SP_NUM_SAMPLES_DELT;
                    ddeltd = SP_NUM_SAMPLES_DDELT;
                    break;
                case SP_NUM_PASSES_SCROLL:
                    d = (double)sp_num_passes;
                    minid = SP_NUM_PASSES_MIN;
                    maxid = SP_NUM_PASSES_MAX;
                    deltd = SP_NUM_PASSES_DELT;
                    ddeltd = SP_NUM_PASSES_DDELT;
                    break;
                case SP_MIN_SCROLL:
                    d = (double)sp_scan_min;
                    minid = SP_MIN_MIN;
                    maxid = sp_scan_max - 1;
                    deltd = SP_SCAN_DELT;
                    ddeltd = SP_SCAN_DDELT;
                    break;
                case SP_MAX_SCROLL:
                    d = (double)sp_scan_max;
                    minid = sp_scan_min + 1;
					if(bit16 && (sp_output_ch == 10 || sp_output_ch == 11))
                      maxid = SP_MAX_MAX * 16; // high resolution
					else
                      maxid = SP_MAX_MAX;
                    deltd = SP_SCAN_DELT;
                    ddeltd = SP_SCAN_DDELT; 
                    break;
                case SP_MIN_SCROLL2:
                    d = (double)sp_rel_min;
					if(bit16 && (sp_output_ch == 10 || sp_output_ch == 11))
                      minid = -MAX * 16;
				    else
                      minid = -MAX;
                    maxid = sp_rel_max - 1;
                    deltd = SP_SCAN_DELT;
                    ddeltd = SP_SCAN_DDELT;
                    break;
                case SP_MAX_SCROLL2:
                    d = (double)sp_rel_max;
                    minid = sp_rel_min+1;
					if(bit16 && (sp_output_ch == 10 || sp_output_ch == 11))
                      maxid = MAX * 16;
				    else
                      maxid = MAX;
                    deltd = SP_SCAN_DELT;
                    ddeltd = SP_SCAN_DDELT; 
                    break;
                case SP_Z_OFFSET_BIAS_SCROLL:
                    d = (double)sp_z_offset_bias;
                    minid = SP_Z_OFFSET_BIAS_MIN;
					if(bit16)
                      maxid = SP_Z_OFFSET_BIAS_MAX * 16;
				    else
                      maxid = SP_Z_OFFSET_BIAS_MAX;
                    deltd = SP_SCAN_DELT;
                    ddeltd = SP_SCAN_DDELT; 
                    break;
                case SP_DELTAZ_SCROLL:
                    d = (double)sp_z_offset;
                    minid = SP_Z_OFFSET_MIN;
                    maxid = SP_Z_OFFSET_MAX;
                    deltd = SP_Z_OFFSET_DELT;
                    ddeltd = SP_Z_OFFSET_DDELT;
                    break;
				case SP_TRIGGER_CH_SCROLL:
                    d = (double)sp_trigger_ch;
                    minid = SP_TRIGGER_CH_MIN;
                    maxid = SP_TRIGGER_CH_MAX;
                    deltd = SP_TRIGGER_CH_DELT;
                    ddeltd = SP_Z_OFFSET_DDELT;
                    break;
				// Mod. 2 Shaowei set trigger level scroll
                case SP_TRIGGER_LOW_SCROLL:
                    d = (double)sp_trigger_low;
                    minid = SP_MIN_MIN;
                    maxid = sp_trigger_high - 1;
                    deltd = SP_SCAN_DELT;
                    ddeltd = SP_SCAN_DDELT;
                    break;
                case SP_TRIGGER_HIGH_SCROLL:
                    d = (double)sp_trigger_high;
                    minid = sp_trigger_low + 1;
					if(bit16 && (sp_trigger_ch == 10 || sp_trigger_ch == 11))
                      maxid = SP_MAX_MAX * 16; // high resolution
					else
                      maxid = SP_MAX_MAX;
                    deltd = SP_SCAN_DELT;
                    ddeltd = SP_SCAN_DDELT; 
                    break;
				//End
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
//            d = floor(d + 0.5);
            d = min(max(d,minid),maxid);
            switch(id)
            {
                case SP_TRACK_MAX_BITS_SCROLL:
                    sp_track_max_bits = i;
                    repaint_track_max_bits(hDlg);
                    break;
                case SP_NUM_SAMPLES_SCROLL:
                    sp_num_samples = (int)floor(d + 0.5);
                    repaint_num_samples(hDlg);
                    break;
                case SP_NUM_PASSES_SCROLL:
                    sp_num_passes = (int)floor(d + 0.5);
                    repaint_num_passes(hDlg);
                    break;
                case SP_MIN_SCROLL:
                    sp_scan_min = (int)floor(d + 0.5);
                    repaint_scan_min(hDlg);
                    break;
                case SP_Z_OFFSET_BIAS_SCROLL:
                    sp_z_offset_bias = (int)floor(d + 0.5);
                    repaint_sp_z_offset_bias(hDlg);
                    break;
                case SP_MAX_SCROLL:
                    sp_scan_max = (int)floor(d + 0.5);
                    repaint_scan_max(hDlg);
                    break;
                case SP_MIN_SCROLL2:
                    sp_rel_min = (int)floor(d + 0.5);
                    repaint_scan_min(hDlg);
                    sp_calc_scan();
                    repaint_scan_min2(hDlg);
                    break;
                case SP_MAX_SCROLL2:
                    sp_rel_max = (int)floor(d + 0.5);
                    repaint_scan_max2(hDlg);
                    sp_calc_scan();
                    repaint_scan_max(hDlg);
                    break;
                case SP_DELTAZ_SCROLL:
                    sp_z_offset = (int)d;
                    repaint_z_offset(hDlg);
                    break;
				case SP_TRIGGER_CH_SCROLL://Mod. 2 Shaowei
                    sp_trigger_ch = (int)d;
                    repaint_trigger_ch(hDlg);
                    break;
				case SP_TRIGGER_LOW_SCROLL:
                    sp_trigger_low = (int)floor(d + 0.5);
                    repaint_trigger_low(hDlg);
                    break;
                case SP_TRIGGER_HIGH_SCROLL:
                    sp_trigger_high = (int)floor(d + 0.5);
                    repaint_trigger_high(hDlg);
                    break;//End Mod. 2
            }
            out_smart = 0;
        }
        break;

      case WM_COMMAND:
        switch(LOWORD(wParam))
        {
			//read sequence commands
		case SP_SUMMARY:
        //  check_crash_protection(hDlg);
          lpfnDlgProc = MakeProcInstance(SummaryDlg,hInst);
		  glob_data = &(sp_defaults_data[sp_current_default]);
		  DialogBox(hInst,"COMMENTDLG",hDlg,lpfnDlgProc);
		  FreeProcInstance(lpfnDlgProc);                  
			break;
		case SP_RS_VOLTS:
		  if(!out_smart)
		  {
            GetDlgItemText(hDlg,PRESCAN_RS_VOLTS,string,9);
            i = vtod(atof(string),get_range(sp_defaults_data[sp_current_default]->sequence[sp_current_step].out_ch));
            i = min(max(i,sample_bias_min),sample_bias_max);
            sp_defaults_data[sp_current_default]->sequence[sp_current_step].step_bias = (unsigned int)i;
		  }
          break;
        case SP_RS_BITS:
		  if(!out_smart)
		  {
            GetDlgItemText(hDlg,PRESCAN_RS_BITS,string,9);
			i = atoi(string);
            i = min(max(i,sample_bias_min),sample_bias_max);
            sp_defaults_data[sp_current_default]->sequence[sp_current_step].step_bias = (unsigned int)i;
		  }
          break;
		case SP_RS_NUM:
		  if(!out_smart)
		  {
            GetDlgItemText(hDlg,SCAN_NUM_EDIT,string,9);
            i = atoi(string);
            sp_defaults_data[sp_current_default]->sequence[sp_current_step].num_reads = min(max(i,scan_num_min),scan_num_max);
            //if(sp_defaults_data[sp_current_default]->sequence[sp_current_step].num_reads != i) repaint_num_samples(hDlg);
		  }
		  break;
        case SP_RS_OUTCH:
		  if(!out_smart)
		  {
            GetDlgItemText(hDlg,LOWORD(wParam),string,10);
            i = atoi(string);
            sp_defaults_data[sp_current_default]->sequence[sp_current_step].out_ch = min(max(i,0),DIO_OUTPUT_CHANNELS - 1);
            //if(sp_defaults_data[sp_current_default]->sequence[sp_current_step].out_ch != i) repaint_read_ch(hDlg);
		  }
	      break;
		case SP_RS_INCH:
		  if(!out_smart)
		  {
            GetDlgItemText(hDlg,LOWORD(wParam),string,10);
            i = atoi(string);
            sp_defaults_data[sp_current_default]->sequence[sp_current_step].in_ch = min(max(i,0),DIO_INPUT_CHANNELS - 1);
            //if(sp_defaults_data[sp_current_default]->sequence[sp_current_step].in_ch != i) repaint_read_ch(hDlg);
		  }
		  break;
		case SP_RS_WAIT:
		  if(!out_smart)
		  {
            GetDlgItemText(hDlg,LOWORD(wParam),string,15);
            i = atoi(string);
            sp_defaults_data[sp_current_default]->sequence[sp_current_step].wait = min(max(i,DITHER_WAIT_MIN),DITHER_WAIT_MAX);
            //if(i != sp_defaults_data[sp_current_default]->sequence[sp_current_step].wait) repaint_waits(hDlg);
		  }
          break;
		case SP_USE_READSQ:
			sp_use_readsq = IsDlgButtonChecked(hDlg,LOWORD(wParam));
			break;
		case SP_RS_ON:
		  if(!out_smart)
		  {
		    CheckDlgButton(hDlg,PRESCAN_RS_OFF,0);
		    CheckDlgButton(hDlg,PRESCAN_RS_ON,1);
		    sp_defaults_data[sp_current_default]->sequence[sp_current_step].state = 1;
		  }
	      break;
        case SP_RS_OFF:
		  if(!out_smart)
		  {
		    CheckDlgButton(hDlg,PRESCAN_RS_OFF,1);
		    CheckDlgButton(hDlg,PRESCAN_RS_ON,0);
		    sp_defaults_data[sp_current_default]->sequence[sp_current_step].state = 0;
		  }
		  break;
			case SP_RS_TYPE:
		  switch(getcombomsg())
          {
            case CBN_SELCHANGE:
              i = SendDlgItemMessage(hDlg,PRESCAN_RS_TYPE,CB_GETCURSEL,0,0);
              strcpy(cur_rs_type,rs_types_list.listbox.str[ 
                        rs_types_list.listbox.index[i]]);
			  strcpy(sp_defaults_data[sp_current_default]->sequence[sp_current_step].step_type_string,cur_rs_type);
///*
		      // update sequence step type
			  if(!strcmp(cur_rs_type,"Feedback"))
			  {
				sp_defaults_data[sp_current_default]->sequence[sp_current_step].type = FEEDBACK_TYPE;
				sp_defaults_data[sp_current_default]->sequence[sp_current_step].state = 1;
			  }
			  else if(!strcmp(cur_rs_type,"Dither Ch 0"))
			  {
				sp_defaults_data[sp_current_default]->sequence[sp_current_step].type = DITHER0_TYPE;
				sp_defaults_data[sp_current_default]->sequence[sp_current_step].state = 0;
			  }
			  else if(!strcmp(cur_rs_type,"Dither Ch 1"))
			  {
				sp_defaults_data[sp_current_default]->sequence[sp_current_step].type = DITHER1_TYPE;
				sp_defaults_data[sp_current_default]->sequence[sp_current_step].state = 0;
			  }
			  else if(!strcmp(cur_rs_type,"Wait"))
			  {
				sp_defaults_data[sp_current_default]->sequence[sp_current_step].type = WAIT_TYPE;
				sp_defaults_data[sp_current_default]->sequence[sp_current_step].wait = 100;
			  }
			  else if(!strcmp(cur_rs_type,"Read"))
			  {
				sp_defaults_data[sp_current_default]->sequence[sp_current_step].type = READ_TYPE;
				sp_defaults_data[sp_current_default]->sequence[sp_current_step].in_ch = zi_ch;
				sp_defaults_data[sp_current_default]->sequence[sp_current_step].num_reads = 10;
			  }
			  else if(!strcmp(cur_rs_type,"Step Output Ch"))
			  {
				sp_defaults_data[sp_current_default]->sequence[sp_current_step].type = STEP_OUTPUT_TYPE;
				sp_defaults_data[sp_current_default]->sequence[sp_current_step].out_ch = sample_bias_ch;
				sp_defaults_data[sp_current_default]->sequence[sp_current_step].step_bias = sample_bias;
			  }
//*/
              //check_crash_protection(hDlg);
			  //repaint_this_sequence(hDlg);
          }

		  break;
			//end read sequence commands
            case SP_MODE_RELATIVE:
            case SP_MODE_ABSOLUTE:
                sp_mode = LOWORD(wParam);
                sp_enable_abs_rel(hDlg);
                sp_calc_scan();
                repaint_scan_min(hDlg);
                repaint_scan_min2(hDlg);
                repaint_scan_max(hDlg);
                repaint_scan_max2(hDlg);
				repaint_trigger_low(hDlg);
				repaint_trigger_high(hDlg);
                break;
            case SP_FEEDBACK_OFF:
                sp_feedback = 0;
                break;
            case SP_FEEDBACK_ON:
                sp_feedback = 1;
                break;
            case SP_FEEDBACK_ON_EVERY:
                sp_feedback = 2;
                break;
            case SP_BITS_PER_STEP:
                if(!out_smart)
                {
                    out_smart = 1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,9);
                    sp_bits_per_step = atoi(string);
                    if(sp_bits_per_step < 1) 
                    {
                        sp_bits_per_step = 1;
                        SetDlgItemText(hDlg,LOWORD(wParam),"1");
                    }
                    out_smart = 0;
                    repaint_sp_v_speed(hDlg);
                }
                break;
            case SP_TRACK_MAX:
            case SP_TRACK_MIN:
                sp_tracking_mode = LOWORD(wParam);
                break;
            case SP_TRACK_OFFSET_X:
                if(!out_smart)
                {
                    out_smart = 1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,9);
                    sp_track_offset_x = atoi(string);
                    if(sp_track_offset_x < -MAX) 
                    {
                        sp_track_offset_x = -MAX;
                        sprintf(string,"%d",sp_track_offset_x);
                        SetDlgItemText(hDlg,LOWORD(wParam),string);
                    }
                    if (sp_track_offset_x > MAX) 
                    {
                        sp_track_offset_x = MAX;
                        sprintf(string,"%d",sp_track_offset_x);
                        SetDlgItemText(hDlg,LOWORD(wParam),string);
                    }
                    out_smart = 0;
                }
                break;
            case SP_TRACK_OFFSET_Y:
                if(!out_smart)
                {
                    out_smart = 1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,9);
                    sp_track_offset_y = atoi(string);
                    if(sp_track_offset_y < -MAX) 
                    {
                        sp_track_offset_y = -MAX;
                        sprintf(string,"%d",sp_track_offset_y);
                        SetDlgItemText(hDlg,LOWORD(wParam),string);
                    }
                    if (sp_track_offset_y > MAX) 
                    {
                        sp_track_offset_y = MAX;
                        sprintf(string,"%d",sp_track_offset_y);
                        SetDlgItemText(hDlg,LOWORD(wParam),string);
                    }
                    out_smart = 0;
                }
                break;
            case SP_TRACK_SAMPLE_EVERY:
                if(!out_smart)
                {
                    out_smart = 1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,9);
                    sp_track_sample_every = atoi(string);
                    if(sp_track_sample_every < 0) 
                    {
                        sp_track_sample_every = 0;
                        SetDlgItemText(hDlg,LOWORD(wParam),"0");
                    }
                    out_smart = 0;
                }
                break;
            case SP_TRACK_STEP_DELAY:
                if(!out_smart)
                {
                    out_smart = 1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,9);
                    sp_track_step_delay = atoi(string);
                    if(sp_track_step_delay < 0) 
                    {
                        sp_track_step_delay = 0;
                        SetDlgItemText(hDlg,LOWORD(wParam),"0");
                    }
                    out_smart = 0;
                }
                break;
            case SP_TRACK_AVG_DATA_PTS:
                if(!out_smart)
                {
                    out_smart = 1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,9);
                    sp_track_avg_data_pts = atoi(string);
                    if(sp_track_avg_data_pts < 1) 
                    {
                        sp_track_avg_data_pts = 1;
                        SetDlgItemText(hDlg,LOWORD(wParam),"1");
                    }
                    out_smart = 0;
                }
                break;
            case SP_TRACK_EVERY:
                if(!out_smart)
                {
                    out_smart = 1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,9);
                    sp_track_every = atoi(string);
                    if(sp_track_every < 0) 
                    {
                        sp_track_every = 0;
                        SetDlgItemText(hDlg,LOWORD(wParam),"0");
                    }
                    out_smart = 0;
                }
                break;
            case SP_FEEDBACK_EVERY:
                if(!out_smart)
                {
                    out_smart = 1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,9);
                    sp_feedback_every = atoi(string);
                    if(sp_feedback_every < 0) 
                    {
                        sp_feedback_every = 0;
                        SetDlgItemText(hDlg,LOWORD(wParam),"0");
                    }
                    out_smart = 0;
                }
                break;
            case SP_TRACK_ITERATIONS:
                if(!out_smart)
                {
                    out_smart = 1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,9);
                    sp_track_iterations = atoi(string);
                    if(sp_track_avg_data_pts < 0) 
                    {
                        sp_track_sample_every = 0;
                        SetDlgItemText(hDlg,LOWORD(wParam),"0");
                    }
                    out_smart = 0;
                }
                break;
            case SP_TRACK_MAX_BITS:
                if(!out_smart)
                {
                    out_smart = 1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,9);
                    sp_track_max_bits = atoi(string);
                    if(sp_track_max_bits < 0) 
                    {
                        sp_track_max_bits = 0;
                        SetDlgItemText(hDlg,LOWORD(wParam),"0");
                    }
                    if(sp_track_max_bits > MAX / 2)
                    {
                        sp_track_max_bits = MAX / 2;
                        sprintf(string,"%d",sp_track_max_bits);
                        SetDlgItemText(hDlg,LOWORD(wParam),string);
                    }
                    out_smart = 0;
//                    SendMessage(ScnDlg,WM_COMMAND,SCAN_REPAINT_SP_TRACK_LIMITS,0);
                    recalc_track_limits();
//                    SendMessage(ScnDlg,WM_COMMAND,SCAN_REPAINT_SP_TRACK_LIMITS,0);
//                    repaint_track_limits(hDlg);
                    
                }
                break;
            case SP_TRACK_PLANE_A:
                if(!out_smart)
                {
                    out_smart = 1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,9);
                    sp_track_plane_a = (float)atof(string);
                    out_smart = 0;
                }
                break;
            case SP_TRACK_PLANE_B:
                if(!out_smart)
                {
                    out_smart = 1;
                    GetDlgItemText(hDlg,LOWORD(wParam),string,9);
                    sp_track_plane_b = (float)atof(string);
                    out_smart = 0;
                }
                break;
            case SP_TRACK_AT_MIN:
            	sp_track_at_min = IsDlgButtonChecked(hDlg,LOWORD(wParam));
            	break;
            case SP_OFFSET_HOLD_ON:
            	sp_offset_hold = IsDlgButtonChecked(hDlg,LOWORD(wParam));
            	break;
			//End of Tracking bottoms

			// Shaowei Trigger Delay
			case SP_START_DELAY_ON://Mod. 1 Shaowei
            	sp_start_delay_on[sp_current_default] = IsDlgButtonChecked(hDlg,LOWORD(wParam));
            	break;//end
			case SP_FB_DELAY_ON://Mod. 3 Shaowei
            	sp_fb_delay_on[sp_current_default] = IsDlgButtonChecked(hDlg,LOWORD(wParam));
            	break;//end
			case SP_FINAL_DELAY_ON://Mod. 4 Shaowei
            	sp_final_delay_on[sp_current_default] = IsDlgButtonChecked(hDlg,LOWORD(wParam));
            	break;//end
			case SP_STEP_DELAY_ON://Mod. 5 Shaowei
            	sp_step_delay_on[sp_current_default] = IsDlgButtonChecked(hDlg,LOWORD(wParam));
            	break;//end			
			case SP_TRIGGER_ON://Mod. 2 Shaowei
            	sp_trigger = IsDlgButtonChecked(hDlg,LOWORD(wParam));
            	break;//end
            case SP_STEPTRIGGER_ON://Mod. 5 Shaowei
            	sp_steptrigger = IsDlgButtonChecked(hDlg,LOWORD(wParam));
            	break;//end
            case SP_DELTAZ_BITS:
                if(!out_smart)
                {
                    GetDlgItemText(hDlg,LOWORD(wParam),string,5);
                    i = atoi(string);
                    i = min(max(i,SP_Z_OFFSET_MIN),SP_Z_OFFSET_MAX);
                    sp_z_offset = i;
                }
				break;
			case SP_DITHER0_ON:
			case SP_DITHER0_OFF:
				sp_dither0 = LOWORD(wParam);
				break;
			case SP_DITHER1_ON:
			case SP_DITHER1_OFF:
				sp_dither1 = LOWORD(wParam);
				break;
			case SP_STOP_CRASHING:
			case SP_CRASH_IN_RANGE:
			case SP_CRASH_DO_NOTHING:
				sp_crash_protection = LOWORD(wParam);
				break;
#ifdef OLD
			case SP_MEASURE_I:
				sp_crash_protection = LOWORD(wParam);
				sp_data_iv->type = DATATYPE_SPEC_I;
				break;
			case SP_MEASURE_Z:
				sp_data_iv->type = DATATYPE_SPEC_Z;
				sp_crash_protection = LOWORD(wParam);
				break;
#endif
			case SP_FORWARD:
				sp_forward = IsDlgButtonChecked(hDlg,LOWORD(wParam));
				break;
			case SP_BACKWARD:
				sp_backward = IsDlgButtonChecked(hDlg,LOWORD(wParam));
				break;
			case SP_AVG_BOTH:
				sp_avg_both = IsDlgButtonChecked(hDlg,LOWORD(wParam));
				break;
			case SP_MEASURE_I:
				sp_measure_i = IsDlgButtonChecked(hDlg,LOWORD(wParam));
				break;
			case SP_MEASURE_Z:
				sp_measure_z = IsDlgButtonChecked(hDlg,LOWORD(wParam));
				break;
			case SP_MEASURE_2:
				sp_measure_2 = IsDlgButtonChecked(hDlg,LOWORD(wParam));
				break;
			case SP_MEASURE_3:
				sp_measure_3 = IsDlgButtonChecked(hDlg,LOWORD(wParam));
				break;
#ifdef OLD
			case SP_BACKGROUND:
				if(!out_smart)
				{
					out_smart = 1;
					GetDlgItemText(hDlg,SP_BACKGROUND,string,9);
					sp_background = in_vtod(atof(string));
                
					out_smart = 0;
				}
				break;
#endif
			case SP_NUM_PASSES:
				if(!out_smart)
				{
					out_smart = 1;
					GetDlgItemText(hDlg,SP_NUM_PASSES,string,9);
					sp_num_passes = (unsigned int)atoi(string);
					sp_num_passes = min(max(sp_num_passes,SP_NUM_PASSES_MIN),SP_NUM_PASSES_MAX);
					SetScrollPos(GetDlgItem(hDlg,SP_NUM_PASSES_SCROLL),SB_CTL, 
						DTOI(sp_num_passes,SP_NUM_PASSES_MIN,SP_NUM_PASSES_MAX),TRUE);
					out_smart = 0;
				}
				break;
			case SP_NUM_SAMPLES:
				if(!out_smart)
				{
					out_smart = 1;
					GetDlgItemText(hDlg,SP_NUM_SAMPLES,string,9);
					sp_num_samples = (unsigned int)atoi(string);
					sp_num_samples = min(max(sp_num_samples,SP_NUM_SAMPLES_MIN),SP_NUM_SAMPLES_MAX);
					SetScrollPos(GetDlgItem(hDlg,SP_NUM_SAMPLES_SCROLL),SB_CTL, 
						DTOI(sp_num_samples,SP_NUM_SAMPLES_MIN,SP_NUM_SAMPLES_MAX),TRUE);
					out_smart = 0;
					repaint_sp_v_speed(hDlg); 
				}
				break;


				//Mod. 2 Shaowei read trigger channel
			case SP_TRIGGER_CH:
				if(!out_smart)
				{
					out_smart = 1;
					GetDlgItemText(hDlg,SP_TRIGGER_CH,string,9);
					sp_trigger_ch = (unsigned int)atoi(string);
					sp_trigger_ch = min(max(sp_trigger_ch,SP_TRIGGER_CH_MIN),SP_TRIGGER_CH_MAX);
					SetScrollPos(GetDlgItem(hDlg,SP_TRIGGER_CH_SCROLL),SB_CTL, 
						DTOI(sp_trigger_ch,SP_TRIGGER_CH_MIN,SP_TRIGGER_CH_MAX),TRUE);
					out_smart = 0;
				}
				break;
				//
			case SP_MIN_RANGE:
				if(!out_smart)
				{
					out_smart = 1;
					GetDlgItemText(hDlg,SP_MIN_RANGE,string,9);
					if(bit16 && (sp_output_ch == 10 || sp_output_ch == 11))
					{
					  sp_scan_min = (unsigned int)vtod16(atof(string));
					  sp_scan_min = min(max(sp_scan_min,SP_MIN_MIN),sp_scan_max - 1);
					  SetScrollPos(GetDlgItem(hDlg,SP_MIN_SCROLL),SB_CTL,
						DTOI(sp_scan_min,SP_MIN_MIN,SP_MIN_MAX * 16),TRUE);
                    }
					else
					{
					  sp_scan_min = (unsigned int)vtod(atof(string),get_range(sp_output_ch));
					  sp_scan_min = min(max(sp_scan_min,SP_MIN_MIN),sp_scan_max - 1);
					  SetScrollPos(GetDlgItem(hDlg,SP_MIN_SCROLL),SB_CTL,
						DTOI(sp_scan_min,SP_MIN_MIN,SP_MIN_MAX),TRUE);
					}
					out_smart = 0;
				}
				break;
			case SP_MAX_RANGE:
				if(!out_smart)
				{
					out_smart = 1;
					GetDlgItemText(hDlg,SP_MAX_RANGE,string,9);
					if(bit16 && (sp_output_ch == 10 || sp_output_ch == 11))
					{
 					  sp_scan_max = (unsigned int)vtod16(atof(string));
					  sp_scan_max = min(max(sp_scan_max,sp_scan_min + 1),SP_MAX_MAX * 16);
					  SetScrollPos(GetDlgItem(hDlg,SP_MAX_SCROLL),SB_CTL,
						DTOI(sp_scan_max,SP_MAX_MIN,SP_MAX_MAX * 16),TRUE);
					}
					else
					{
 					  sp_scan_max = (unsigned int)vtod(atof(string),get_range(sp_output_ch));
					  sp_scan_max = min(max(sp_scan_max,sp_scan_min + 1),SP_MAX_MAX);
					  SetScrollPos(GetDlgItem(hDlg,SP_MAX_SCROLL),SB_CTL,
						DTOI(sp_scan_max,SP_MAX_MIN,SP_MAX_MAX),TRUE);
					}
					out_smart = 0;
				}
				break;
			case SP_MIN_RANGE2:
				if(!out_smart)
				{
					out_smart = 1;
					GetDlgItemText(hDlg,SP_MIN_RANGE2,string,9);
					sp_rel_min = atoi(string);
					if(bit16 && (sp_output_ch == 10 || sp_output_ch == 11))
					{
					  sp_rel_min = min(max(sp_rel_min,-MAX * 16),sp_rel_max - 1);
					  SetScrollPos(GetDlgItem(hDlg,SP_MIN_SCROLL2),SB_CTL,
						DTOI(sp_rel_min,(-MAX * 16),MAX * 16),TRUE);
					}
					else
					{
					  sp_rel_min = min(max(sp_rel_min,-MAX),sp_rel_max - 1);
					  SetScrollPos(GetDlgItem(hDlg,SP_MIN_SCROLL2),SB_CTL,
						DTOI(sp_rel_min,(-MAX),MAX),TRUE);
					}
					sp_calc_scan();
					repaint_scan_min(hDlg);
					out_smart = 0;
				}
				break;
			case SP_MAX_RANGE2:
				if(!out_smart)
				{
					out_smart = 1;
					GetDlgItemText(hDlg,SP_MAX_RANGE2,string,9);
					sp_rel_max = atoi(string);
					if(bit16 && (sp_output_ch == 10 || sp_output_ch == 11))
					{
					  sp_rel_max = min(max(sp_rel_max,sp_rel_min + 1),MAX * 16);
					  SetScrollPos(GetDlgItem(hDlg,SP_MAX_SCROLL2),SB_CTL,
						DTOI(sp_rel_max,(-MAX * 16),MAX * 16),TRUE);
                    }
					else
					{
					  sp_rel_max = min(max(sp_rel_max,sp_rel_min + 1),MAX);
					  SetScrollPos(GetDlgItem(hDlg,SP_MAX_SCROLL2),SB_CTL,
						DTOI(sp_rel_max,(-MAX),MAX),TRUE);
					}
					sp_calc_scan();
					repaint_scan_max(hDlg);
					out_smart = 0;
				}
				break;
//Mod. 2 tirgger range Shaowei
				
				case SP_TRIGGER_HIGH:
				if(!out_smart)
				{
					out_smart = 1;
					GetDlgItemText(hDlg,SP_TRIGGER_HIGH,string,9);
					if(bit16 && (sp_trigger_ch == 10 || sp_trigger_ch == 11))
					{
 					  sp_trigger_high = (unsigned int)vtod16(atof(string));
					  sp_trigger_high = min(max(sp_trigger_high,sp_trigger_low + 1),SP_MAX_MAX * 16);
					  SetScrollPos(GetDlgItem(hDlg,SP_TRIGGER_HIGH_SCROLL),SB_CTL,
						DTOI(sp_trigger_high,SP_MAX_MIN,SP_MAX_MAX * 16),TRUE);
					}
					else
					{
 					  sp_trigger_high = (unsigned int)vtod(atof(string),get_range(sp_trigger_ch));
					  sp_trigger_high = min(max(sp_trigger_high,sp_trigger_low + 1),SP_MAX_MAX);
					  SetScrollPos(GetDlgItem(hDlg,SP_TRIGGER_HIGH_SCROLL),SB_CTL,
						DTOI(sp_trigger_high,SP_MAX_MIN,SP_MAX_MAX),TRUE);
					}
					repaint_trigger_high(hDlg);
					out_smart = 0;
				}
				break;              
				
				case SP_TRIGGER_LOW:
				if(!out_smart)
				{
					out_smart = 1;
					GetDlgItemText(hDlg,SP_TRIGGER_LOW,string,9);
					if(bit16 && (sp_trigger_ch == 10 || sp_trigger_ch == 11))
					{
					  sp_trigger_low = (unsigned int)vtod16(atof(string));
					  
					  sp_trigger_low = min(max(sp_trigger_low,SP_MIN_MIN),sp_trigger_high - 1);
					  SetScrollPos(GetDlgItem(hDlg,SP_TRIGGER_LOW_SCROLL),SB_CTL,
						DTOI(sp_trigger_low,SP_MIN_MIN,SP_MIN_MAX * 16),TRUE);
                    }
					else
					{
					  sp_trigger_low = (unsigned int)vtod(atof(string),get_range(sp_trigger_ch));
					  sp_trigger_low = min(max(sp_trigger_low,SP_MIN_MIN),sp_trigger_high - 1);
					  SetScrollPos(GetDlgItem(hDlg,SP_TRIGGER_LOW_SCROLL),SB_CTL,
						DTOI(sp_trigger_low,SP_MIN_MIN,SP_MIN_MAX),TRUE);
					}
					out_smart = 0;
				    repaint_trigger_low(hDlg);
				}
				break;

				//



			case SP_USE_Z_OFFSET_BIAS:
				sp_use_z_offset_bias = IsDlgButtonChecked(hDlg,LOWORD(wParam));
				break;
			case SP_Z_OFFSET_BIAS:
				if(!out_smart)
				{
					out_smart = 1;
					GetDlgItemText(hDlg,LOWORD(wParam),string,9);
					if(bit16)
					{
					  sp_z_offset_bias = (unsigned int)vtod16(atof(string));
					  sp_z_offset_bias = min(max(sp_z_offset_bias,SP_Z_OFFSET_BIAS_MIN),SP_Z_OFFSET_BIAS_MAX * 16);
					  SetScrollPos(GetDlgItem(hDlg,SP_Z_OFFSET_BIAS_SCROLL),SB_CTL, 
						DTOI(sp_z_offset_bias,SP_Z_OFFSET_BIAS_MIN,SP_Z_OFFSET_BIAS_MAX * 16), TRUE);
                    }
					else
					{
					  sp_z_offset_bias = (unsigned int)vtod(atof(string),get_range(sp_output_ch));
					  sp_z_offset_bias = min(max(sp_z_offset_bias,SP_Z_OFFSET_BIAS_MIN),SP_Z_OFFSET_BIAS_MAX);
					  SetScrollPos(GetDlgItem(hDlg,SP_Z_OFFSET_BIAS_SCROLL),SB_CTL, 
						DTOI(sp_z_offset_bias,SP_Z_OFFSET_BIAS_MIN,SP_Z_OFFSET_BIAS_MAX), TRUE);
					}
					out_smart = 0;
				}
				break;
			case SP_SPEED:
				if(!out_smart)
				{
					out_smart = 1;
					GetDlgItemText(hDlg,SP_SPEED,string,9);
					sp_speed = atoi(string);
					sp_speed = min(max(sp_speed,SP_SPEED_MIN),SP_SPEED_MAX);
//						SetScrollPos(GetDlgItem(hDlg, SP_VOLTS_SCROLL),SB_CTL,DTOI(sp_speed,SP_SPEED_MIN,SP_SPEED_MAX),TRUE);
					out_smart = 0;
					repaint_sp_v_speed(hDlg);
				}
				break;
			case SP_DITHER_WAIT:
				if(!out_smart)
				{
					out_smart = 1;
					GetDlgItemText(hDlg,SP_DITHER_WAIT,string,9);
					sp_dither_wait = atoi(string);
					sp_dither_wait = min(max(sp_dither_wait,SP_SPEED_MIN),SP_SPEED_MAX);
					out_smart = 0;
				}
				break;
			case SP_FEEDBACK_WAIT:
				if(!out_smart)
				{
					out_smart = 1;
					GetDlgItemText(hDlg,SP_FEEDBACK_WAIT,string,9);
					sp_feedback_wait = atoi(string);
					sp_feedback_wait = min(max(sp_feedback_wait,SP_SPEED_MIN),SP_SPEED_MAX);
					out_smart = 0;
				}
				break;
			case SP_FEEDBACK_AT_MIN:
				sp_feedback_at_min = IsDlgButtonChecked(hDlg,LOWORD(wParam));
				break;
			case SP_MOVE_DELAY:
				if(!out_smart)
				{
					out_smart = 1;
					GetDlgItemText(hDlg,SP_MOVE_DELAY,string,9);
                sp_move_delay = atoi(string);
                sp_move_delay = min(max(sp_move_delay,SP_SPEED_MIN),SP_SPEED_MAX);
                out_smart = 0;
				}
				break;
			case SP_RAMP_DELAY:
				if(!out_smart)
				{
					out_smart = 1;
					GetDlgItemText(hDlg,SP_RAMP_DELAY,string,9);
                sp_ramp_delay = atoi(string);
                sp_ramp_delay = min(max(sp_ramp_delay,SP_SPEED_MIN),SP_SPEED_MAX);
                out_smart = 0;
				}
				break;

			case SP_START_DELAY://Mod. 1 Shaowei
				if(!out_smart)
				{
					out_smart = 1;
					GetDlgItemText(hDlg,SP_START_DELAY,string,9);
                sp_start_delay[sp_current_default] = atoi(string);
                sp_start_delay[sp_current_default] = min(max(sp_start_delay[sp_current_default],SP_MIN_MIN),SP_SPEED_MAX);//0 to 10000000
                out_smart = 0;
				}
				break;//end Shaowei 


			case SP_FB_DELAY://Mod. 3 Shaowei
				if(!out_smart)
				{
					out_smart = 1;
					GetDlgItemText(hDlg,SP_FB_DELAY,string,9);
                sp_fb_delay[sp_current_default] = atoi(string);
                sp_fb_delay[sp_current_default] = min(max(sp_fb_delay[sp_current_default],SP_MIN_MIN),SP_SPEED_MAX);//0 to 10000000
                out_smart = 0;
				}
				break;//end Shaowei 

			case SP_STEP_DELAY://Mod. 5 Shaowei
				if(!out_smart)
				{
					out_smart = 1;
					GetDlgItemText(hDlg,SP_STEP_DELAY,string,9);
                sp_step_delay[sp_current_default] = atoi(string);
                sp_step_delay[sp_current_default] = min(max(sp_step_delay[sp_current_default],SP_MIN_MIN),SP_SPEED_MAX);//0 to 10000000
                out_smart = 0;
				}
				break;//end Shaowei 

			case SP_FINAL_DELAY://Mod. 4 Shaowei
				if(!out_smart)
				{
					out_smart = 1;
					GetDlgItemText(hDlg,SP_FINAL_DELAY,string,9);
                sp_final_delay[sp_current_default] = atoi(string);
                sp_final_delay[sp_current_default] = min(max(sp_final_delay[sp_current_default],SP_MIN_MIN),SP_SPEED_MAX);//0 to 10000000
                out_smart = 0;
				}
				break;//end Shaowei 
			case SP_TRIGGER_TIME://Mod. 2 Shaowei
				if(!out_smart)
				{
					out_smart = 1;
					GetDlgItemText(hDlg,SP_TRIGGER_TIME,string,9);
                sp_trigger_time = atoi(string);
                sp_trigger_time = min(max(sp_trigger_time,SP_MIN_MIN),SP_SPEED_MAX);//0 to 10000000
                out_smart = 0;
				}
				break;//end Shaowei 
			case SP_OUTPUT_CH:
				if(!out_smart)
				{
					out_smart = 1;
					GetDlgItemText(hDlg,SP_OUTPUT_CH,string,9);
					old_output_ch = sp_output_ch;
					sp_output_ch = atoi(string);
					sp_output_ch = min(max(sp_output_ch,0),11);
					if(bit16)
					{
                      // handle switch from 16 bit to 12 bit
					  if(old_output_ch > 9 && sp_output_ch < 10)
					  {
				        sp_scan_min = vtod(dtov16(sp_scan_min),get_range(sp_output_ch));
				        sp_scan_max = vtod(dtov16(sp_scan_max),get_range(sp_output_ch));
						if(sp_scan_min >= sp_scan_max) sp_scan_min = sp_scan_max - 1;
					    sp_rel_min = min(max(sp_rel_min,-MAX),MAX);
					    sp_rel_max = min(max(sp_rel_max,-MAX),MAX);
						if(sp_rel_min >= sp_rel_max) sp_rel_min = sp_rel_max - 1;
					  }
                      // handle switch from 12 bit to 16 bit
					  if(old_output_ch < 10 && sp_output_ch > 9)
					  {
				        sp_scan_min = vtod16(dtov(sp_scan_min,get_range(old_output_ch)));
				        sp_scan_max = vtod16(dtov(sp_scan_max,get_range(old_output_ch)));
						if(sp_scan_min >= sp_scan_max) sp_scan_min = sp_scan_max - 1;
					    sp_rel_min = min(max(sp_rel_min,-MAX * 16),MAX * 16);
					    sp_rel_max = min(max(sp_rel_max,-MAX * 16),MAX * 16);
						if(sp_rel_min >= sp_rel_max) sp_rel_min = sp_rel_max - 1;
					  }
// Trigger bit change Mod.2 Shaowei 
                      if(old_output_ch > 9 && sp_trigger_ch < 10)
					  {
				        sp_trigger_low = vtod(dtov16(sp_trigger_low),get_range(sp_trigger_ch));
				        sp_trigger_high = vtod(dtov16(sp_trigger_high),get_range(sp_trigger_ch));
						if(sp_trigger_low >= sp_trigger_high) sp_trigger_low = sp_trigger_high - 1;
					  }
                      // handle switch from 12 bit to 16 bit
					  if(old_output_ch < 10 && sp_output_ch > 9)
					  {
				        sp_trigger_low = vtod16(dtov(sp_trigger_low,get_range(old_output_ch)));
				        sp_trigger_high = vtod16(dtov(sp_trigger_high,get_range(old_output_ch)));
						if(sp_scan_min >= sp_scan_max) sp_scan_min = sp_scan_max - 1;
					   }


					}
					sp_calc_scan();
					repaint_scan_min(hDlg);
					repaint_scan_max(hDlg);
					repaint_scan_min2(hDlg);
					repaint_scan_max2(hDlg);
					repaint_sp_v_speed(hDlg);
					repaint_trigger_high(hDlg);
					repaint_trigger_low(hDlg);
					out_smart = 0;
				}
				break;
			case ENTER:
				repaint_all(hDlg);
				break;
			case SPOPT_EXIT:
				EndDialog(hDlg,TRUE);
				return(TRUE);
				break;
        }
        break;
    }
    return(FALSE);
}                                                                  


static void repaint_channels(HWND hDlg)
{
#ifdef OLD
    sprintf(string,"%ld",sp_input_ch);
    SetDlgItemText(hDlg,SP_INPUT_CH,string);
#endif
    sprintf(string,"%ld",sp_output_ch);
    SetDlgItemText(hDlg,SP_OUTPUT_CH,string);
}

static void repaint_scan_min(HWND hDlg)
{
    out_smart = 1;
	if(bit16 && (sp_output_ch == 10 || sp_output_ch == 11))
	{
      SetScrollPos(GetDlgItem(hDlg, SP_MIN_SCROLL),SB_CTL,
		DTOI(sp_scan_min,SP_MIN_MIN,SP_MIN_MAX * 16),TRUE);
      sprintf(string,"%0.4f",dtov16(sp_scan_min));
    }
	else
	{
      SetScrollPos(GetDlgItem(hDlg, SP_MIN_SCROLL),SB_CTL,
		DTOI(sp_scan_min,SP_MIN_MIN,SP_MIN_MAX),TRUE);
      sprintf(string,"%0.3f",dtov(sp_scan_min,get_range(sp_output_ch)));
	}
    
	SetDlgItemText(hDlg, SP_MIN_RANGE, string);
    out_smart=0;
}


static void repaint_sp_z_offset_bias(HWND hDlg)
{
    out_smart = 1;
	if(bit16)
	{
      SetScrollPos(GetDlgItem(hDlg,SP_Z_OFFSET_BIAS_SCROLL),SB_CTL, 
        DTOI(sp_z_offset_bias,SP_Z_OFFSET_BIAS_MIN,SP_Z_OFFSET_BIAS_MAX * 16),TRUE);
      sprintf(string,"%0.4f",dtov16(sp_z_offset_bias));
	}
	else
	{ 
      SetScrollPos(GetDlgItem(hDlg,SP_Z_OFFSET_BIAS_SCROLL),SB_CTL, 
        DTOI(sp_z_offset_bias,SP_Z_OFFSET_BIAS_MIN,SP_Z_OFFSET_BIAS_MAX),TRUE);
      sprintf(string,"%0.3f",dtov(sp_z_offset_bias,get_range(sample_bias_range)));
	}
	SetDlgItemText(hDlg,SP_Z_OFFSET_BIAS,string);
    out_smart = 0;
}

static void repaint_scan_max(HWND hDlg)
{
    out_smart = 1;
	if(bit16 && (sp_output_ch == 10 || sp_output_ch == 11))
	{
      SetScrollPos(GetDlgItem(hDlg, SP_MAX_SCROLL),SB_CTL,
		DTOI(sp_scan_max,SP_MAX_MIN,SP_MAX_MAX * 16),TRUE);
      sprintf(string,"%0.4f",dtov16(sp_scan_max));
    }
	else
	{
      SetScrollPos(GetDlgItem(hDlg, SP_MAX_SCROLL),SB_CTL,
		DTOI(sp_scan_max,SP_MAX_MIN,SP_MAX_MAX),TRUE);
      sprintf(string,"%0.3f",dtov(sp_scan_max,get_range(sp_output_ch)));
	}
	SetDlgItemText(hDlg,SP_MAX_RANGE,string);
    out_smart = 0;
}

static void repaint_scan_min2(HWND hDlg)
{
    out_smart = 1;
	if(bit16 && (sp_output_ch == 10 || sp_output_ch == 11))
	{
      SetScrollPos(GetDlgItem(hDlg, SP_MIN_SCROLL2),SB_CTL,DTOI(sp_rel_min,(-MAX * 16),+MAX * 16),TRUE);
      sprintf(string,"%d",sp_rel_min);
	}
	else
	{
      SetScrollPos(GetDlgItem(hDlg, SP_MIN_SCROLL2),SB_CTL,DTOI(sp_rel_min,(-MAX),+MAX),TRUE);
      sprintf(string,"%d",sp_rel_min);
	}
    SetDlgItemText(hDlg,SP_MIN_RANGE2,string);
    sp_calc_scan();
    out_smart = 0;
}

static void repaint_scan_max2(HWND hDlg)
{
    out_smart = 1;
	if(bit16 && (sp_output_ch == 10 || sp_output_ch == 11))
	{
      SetScrollPos(GetDlgItem(hDlg, SP_MAX_SCROLL2),SB_CTL, DTOI(sp_rel_max,(-MAX * 16),MAX * 16),TRUE);
      sprintf(string,"%d",sp_rel_max);
	}
	else
	{
      SetScrollPos(GetDlgItem(hDlg, SP_MAX_SCROLL2),SB_CTL, DTOI(sp_rel_max,(-MAX),MAX),TRUE);
      sprintf(string,"%d",sp_rel_max);
	}
    SetDlgItemText(hDlg,SP_MAX_RANGE2,string);
    sp_calc_scan();
    out_smart = 0;
}
//Mod. 2 Shaowei Repaint trigger level
static void repaint_trigger_low(HWND hDlg)
{
    out_smart = 1;
	if(bit16 && (sp_trigger_ch == 10 || sp_trigger_ch == 11))
	{
      SetScrollPos(GetDlgItem(hDlg, SP_TRIGGER_LOW_SCROLL),SB_CTL,
		DTOI(sp_trigger_low,SP_MIN_MIN,SP_MIN_MAX * 16),TRUE);
      sprintf(string,"%0.4f",dtov16(sp_trigger_low));
    }
	else
	{
      SetScrollPos(GetDlgItem(hDlg, SP_TRIGGER_LOW_SCROLL),SB_CTL,
		DTOI(sp_trigger_low,SP_MIN_MIN,SP_MIN_MAX),TRUE);
      sprintf(string,"%0.3f",dtov(sp_trigger_low,get_range(sp_trigger_ch)));
	}
    
	SetDlgItemText(hDlg, SP_TRIGGER_LOW, string);
    out_smart=0;
}


static void repaint_trigger_high(HWND hDlg)//Mod. 2 Shaowei repaint trigger high level
{
    out_smart = 1;
	if(bit16 && (sp_trigger_ch == 10 || sp_trigger_ch == 11))
	{
      SetScrollPos(GetDlgItem(hDlg, SP_TRIGGER_HIGH_SCROLL),SB_CTL,
		DTOI(sp_trigger_high,SP_MAX_MIN,SP_MAX_MAX * 16),TRUE);
      sprintf(string,"%0.4f",dtov16(sp_trigger_high));
    }
	else
	{
      SetScrollPos(GetDlgItem(hDlg, SP_TRIGGER_HIGH_SCROLL),SB_CTL,
		DTOI(sp_trigger_high,SP_MAX_MIN,SP_MAX_MAX),TRUE);
      sprintf(string,"%0.3f",dtov(sp_trigger_high,get_range(sp_trigger_ch)));
	}
	SetDlgItemText(hDlg,SP_TRIGGER_HIGH,string);
    out_smart = 0;
}//End


static void repaint_speed(HWND hDlg)
{
    out_smart = 1;
/*    
	SetScrollPos(GetDlgItem(hDlg,SP_VOLTS_SCROLL),
		SB_CTL,DTOI(sp_speed,SP_SPEED_MIN,SP_SPEED_MAX),TRUE);
*/
    sprintf(string,"%d",sp_speed);
    SetDlgItemText(hDlg,SP_SPEED,string);
    out_smart = 0;
}

static void repaint_dither_wait(HWND hDlg)
{
    out_smart = 1;
    sprintf(string,"%d",sp_dither_wait);
    SetDlgItemText(hDlg,SP_DITHER_WAIT,string);
    out_smart = 0;
}

static void repaint_move_delay(HWND hDlg)
{
    out_smart = 1;
    sprintf(string,"%d",sp_move_delay);
    SetDlgItemText(hDlg,SP_MOVE_DELAY,string);
    out_smart = 0;
}

static void repaint_ramp_delay(HWND hDlg)
{
    out_smart = 1;
    sprintf(string,"%d",sp_ramp_delay);
    SetDlgItemText(hDlg,SP_RAMP_DELAY,string);
    out_smart = 0;
}

static void repaint_start_delay(HWND hDlg)//Mod. 1 Shaowei
{
    out_smart = 1;
    sprintf(string,"%d",sp_start_delay[sp_current_default]);
    SetDlgItemText(hDlg,SP_START_DELAY,string);
    out_smart = 0;
}

static void repaint_fb_delay(HWND hDlg)//Mod. 3 Shaowei
{
    out_smart = 1;
    sprintf(string,"%d",sp_fb_delay[sp_current_default]);
    SetDlgItemText(hDlg,SP_FB_DELAY,string);
    out_smart = 0;
}

static void repaint_final_delay(HWND hDlg)//Mod. 4 Shaowei
{
    out_smart = 1;
    sprintf(string,"%d",sp_final_delay[sp_current_default]);
    SetDlgItemText(hDlg,SP_FINAL_DELAY,string);
    out_smart = 0;
}

static void repaint_step_delay(HWND hDlg)//Mod. 5 Shaowei
{
    out_smart = 1;
    sprintf(string,"%d",sp_step_delay[sp_current_default]);
    SetDlgItemText(hDlg,SP_STEP_DELAY,string);
    out_smart = 0;
}

static void repaint_trigger_time(HWND hDlg)//Mod. 2 Shaowei
{
    out_smart = 1;
    sprintf(string,"%d",sp_trigger_time);
    SetDlgItemText(hDlg,SP_TRIGGER_TIME,string);
    out_smart = 0;
}

#ifdef OLD
static void repaint_background(HWND hDlg)
{
    out_smart = 1;
    sprintf(string,"%0.3lf",in_dtov((float)sp_background));
    SetDlgItemText(hDlg,SP_BACKGROUND,string);
    out_smart = 0;
}
#endif
        
static void repaint_num_samples(HWND hDlg)
{
    out_smart = 1;
    sprintf(string,"%d",sp_num_samples);
    SetDlgItemText(hDlg,SP_NUM_SAMPLES,string);
    SetScrollPos(GetDlgItem(hDlg,SP_NUM_SAMPLES_SCROLL), 
        SB_CTL,DTOI(sp_num_samples,SP_NUM_SAMPLES_MIN,SP_NUM_SAMPLES_MAX),TRUE);
    out_smart = 0;
    repaint_sp_v_speed(hDlg);
}

// Mod. 2 Shaowei repaint trigger ch
static void repaint_trigger_ch(HWND hDlg)
{
    out_smart = 1;
    sprintf(string,"%d",sp_trigger_ch);
    SetDlgItemText(hDlg,SP_TRIGGER_CH,string);
    SetScrollPos(GetDlgItem(hDlg,SP_TRIGGER_CH_SCROLL), 
        SB_CTL,DTOI(sp_trigger_ch,SP_TRIGGER_CH_MIN,SP_TRIGGER_CH_MAX),TRUE);
    out_smart = 0;
}
//end

static void repaint_num_passes(HWND hDlg)
{
    out_smart = 1;
    sprintf(string,"%d",sp_num_passes);
    SetDlgItemText(hDlg,SP_NUM_PASSES,string);
    SetScrollPos(GetDlgItem(hDlg,SP_NUM_PASSES_SCROLL), 
        SB_CTL,DTOI(sp_num_passes,SP_NUM_PASSES_MIN,SP_NUM_PASSES_MAX),TRUE);
    out_smart = 0;
}

static void sp_enable_abs_rel(HWND hDlg)
{
    int status = (sp_mode == SP_MODE_ABSOLUTE);
    
    EnableWindow(GetDlgItem(hDlg,SP_MIN_RANGE),status);
    EnableWindow(GetDlgItem(hDlg,SP_MAX_RANGE),status);
    EnableWindow(GetDlgItem(hDlg,SP_MIN_SCROLL),status);
    EnableWindow(GetDlgItem(hDlg,SP_MAX_SCROLL),status);
    EnableWindow(GetDlgItem(hDlg,SP_MIN_RANGE2),!status);
    EnableWindow(GetDlgItem(hDlg,SP_MAX_RANGE2),!status);
    EnableWindow(GetDlgItem(hDlg,SP_MIN_SCROLL2),!status);
    EnableWindow(GetDlgItem(hDlg,SP_MAX_SCROLL2),!status);

}

#ifdef OLD
static void sp_scan_enable(HWND hDlg,int status)
{
    EnableWindow(GetDlgItem(hDlg,SP_SPEED),status);
//    EnableWindow(GetDlgItem(hDlg,SP_VOLTS_SCROLL),status);
//    EnableWindow(GetDlgItem(hDlg,SP_INPUT_CH),status);
//    EnableWindow(GetDlgItem(hDlg,SP_OUTPUT_CH),status);
    EnableWindow(GetDlgItem(hDlg,SP_COMMENT),status);
//    EnableWindow(GetDlgItem(hDlg,SP_PRINT),status);
    EnableWindow(GetDlgItem(hDlg,SP_SAVE),status);
    EnableWindow(GetDlgItem(hDlg,SP_EXIT),status);
    EnableWindow(GetDlgItem(hDlg,SP_LOAD),status);
    EnableWindow(GetDlgItem(hDlg,SP_STOP_CRASHING),status);
    EnableWindow(GetDlgItem(hDlg,SP_BITS_PER_STEP),status);
    EnableWindow(GetDlgItem(hDlg,SP_DELTAZ_VOLTS),status);
    EnableWindow(GetDlgItem(hDlg,SP_DELTAZ_BITS),status);
    EnableWindow(GetDlgItem(hDlg,SP_DELTAZ_SCROLL),status);
    EnableWindow(GetDlgItem(hDlg,SP_Z_OFFSET_BIAS),status);
    EnableWindow(GetDlgItem(hDlg,SP_Z_OFFSET_BIAS_SCROLL),status);
    EnableWindow(GetDlgItem(hDlg,SP_TRACK_MAX_BITS),status);
    EnableWindow(GetDlgItem(hDlg,SP_TRACK_MAX_BITS_SCROLL),status);
    EnableWindow(GetDlgItem(hDlg,SP_TRACK_AVG_DATA_PTS),status);
    EnableWindow(GetDlgItem(hDlg,SP_TRACK_SAMPLE_EVERY),status);
    EnableWindow(GetDlgItem(hDlg,SP_TRACK_EVERY),status);
    EnableWindow(GetDlgItem(hDlg,SP_FEEDBACK_EVERY),status);
    EnableWindow(GetDlgItem(hDlg,SP_TRACK_OFFSET_X),status);
    EnableWindow(GetDlgItem(hDlg,SP_TRACK_OFFSET_Y),status);
    EnableWindow(GetDlgItem(hDlg,SP_MEASURE_I),status);
    EnableWindow(GetDlgItem(hDlg,SP_MEASURE_Z),status);
    EnableWindow(GetDlgItem(hDlg,SP_MEASURE_2),status);
    EnableWindow(GetDlgItem(hDlg,SP_MEASURE_3),status);
    EnableWindow(GetDlgItem(hDlg,SP_FORWARD),status);
    EnableWindow(GetDlgItem(hDlg,SP_BACKWARD),status);
    EnableWindow(GetDlgItem(hDlg,SP_AVG_BOTH),status);
    EnableWindow(GetDlgItem(hDlg,SP_NUM_SAMPLES),status);
    EnableWindow(GetDlgItem(hDlg,SP_NUM_SAMPLES_SCROLL),status);
    EnableWindow(GetDlgItem(hDlg,SP_NUM_PASSES),status);
    EnableWindow(GetDlgItem(hDlg,SP_NUM_PASSES_SCROLL),status);
    EnableWindow(GetDlgItem(hDlg,SP_DITHER0_ON),status);
    EnableWindow(GetDlgItem(hDlg,SP_DITHER0_OFF),status);
    EnableWindow(GetDlgItem(hDlg,SP_DITHER1_ON),status);
    EnableWindow(GetDlgItem(hDlg,SP_DITHER1_OFF),status);
    EnableWindow(GetDlgItem(hDlg,SP_DITHER_WAIT),status);
    EnableWindow(GetDlgItem(hDlg,SP_FEEDBACK_WAIT),status);
    EnableWindow(GetDlgItem(hDlg,SP_FEEDBACK_ON),status);
    EnableWindow(GetDlgItem(hDlg,SP_FEEDBACK_OFF),status);
    EnableWindow(GetDlgItem(hDlg,SP_CLEAR),status);
    EnableWindow(GetDlgItem(hDlg,SP_CLEAR_ALL),status);
    EnableWindow(GetDlgItem(hDlg,SP_MOVE_DELAY),status);

}
#endif

static void repaint_z_offset(HWND hDlg)
{
    out_smart = 1;
    sprintf(string,"%d",sp_z_offset);
    SetDlgItemText(hDlg,SP_DELTAZ_BITS,string);
    sprintf(string,"%0.3f",dtov_len(sp_z_offset,z_offset_range));
    SetDlgItemText(hDlg,SP_DELTAZ_VOLTS,string);
/*
    sprintf(string,"%0.3f",dtov_len(sp_z_offset,z_offset_range)*
        (double) Z_A_PER_V);
    SetDlgItemText(hDlg, SP_DELTAZ_ANGS,string);
*/
    SetScrollPos(GetDlgItem(hDlg,SP_DELTAZ_SCROLL),SB_CTL, 
        DTOI(sp_z_offset,SP_Z_OFFSET_MIN,SP_Z_OFFSET_MAX),TRUE);
    
    out_smart = 0;
}

static void repaint_sp_planes(HWND hDlg)
{
    sprintf(string,"%.4f",sp_track_plane_a);
    SetDlgItemText(hDlg,SP_TRACK_PLANE_A,string);
    sprintf(string,"%.4f",sp_track_plane_b);
    SetDlgItemText(hDlg,SP_TRACK_PLANE_B,string);
}
 
static void repaint_track(HWND hDlg)
{
    out_smart = 1;
    CheckDlgButton(hDlg,sp_tracking_mode,1);
    CheckDlgButton(hDlg,SP_TRACK_AT_MIN,sp_track_at_min);
    CheckDlgButton(hDlg,SP_OFFSET_HOLD_ON,sp_offset_hold);
    sprintf(string,"%d",sp_track_offset_x);
    SetDlgItemText(hDlg,SP_TRACK_OFFSET_X,string);
    sprintf(string,"%d",sp_track_offset_y);
    SetDlgItemText(hDlg,SP_TRACK_OFFSET_Y,string);
    sprintf(string,"%d",sp_track_step_delay);
    SetDlgItemText(hDlg,SP_TRACK_STEP_DELAY,string);
    sprintf(string,"%d",sp_track_sample_every);
    SetDlgItemText(hDlg,SP_TRACK_SAMPLE_EVERY,string);
    sprintf(string,"%d",sp_track_avg_data_pts);
    SetDlgItemText(hDlg,SP_TRACK_AVG_DATA_PTS,string);
    sprintf(string,"%d",sp_track_iterations);
    SetDlgItemText(hDlg,SP_TRACK_ITERATIONS,string);
    sprintf(string,"%d",sp_track_every);
    SetDlgItemText(hDlg,SP_TRACK_EVERY,string);
    SetScrollPos(GetDlgItem(hDlg,SP_TRACK_MAX_BITS_SCROLL),SB_CTL,sp_track_max_bits,TRUE);
    repaint_sp_planes(hDlg);
    out_smart = 0;
    
//    repaint_current_pos(hDlg);
    repaint_track_max_bits(hDlg);
    repaint_track_current(hDlg);
}    
static void repaint_sp_trigger(HWND hDlg)
{
	CheckDlgButton(hDlg,SP_START_DELAY_ON,sp_start_delay_on[sp_current_default]);//Mod. 1 Shaowei
	CheckDlgButton(hDlg,SP_FINAL_DELAY_ON,sp_final_delay_on[sp_current_default]);//Mod. 4 Shaowei
	CheckDlgButton(hDlg,SP_STEP_DELAY_ON,sp_step_delay_on[sp_current_default]);//Mod. 5 Shaowei
	CheckDlgButton(hDlg,SP_FB_DELAY_ON,sp_fb_delay_on[sp_current_default]);//Mod. 3 Shaowei
	CheckDlgButton(hDlg,SP_TRIGGER_ON,sp_trigger);//Mod. 2 Shaowei
	CheckDlgButton(hDlg,SP_STEPTRIGGER_ON,sp_steptrigger);//Mod. 5 Shaowei
    repaint_trigger_ch(hDlg); //Mod. 2 Shaowei
	repaint_start_delay(hDlg);//Mod. 1 Shaowei
	repaint_fb_delay(hDlg);//Mod. 3 Shaowei
	repaint_final_delay(hDlg);//Mod. 4 Shaowei
	repaint_step_delay(hDlg);//Mod. 5 Shaowei
	repaint_trigger_time(hDlg);//Mod. 2 Shaowei
	repaint_trigger_low(hDlg);//Mod. 2 Shaowei
	repaint_trigger_high(hDlg);//Mod. 2 Shaowei
}
static void repaint_track_max_bits(HWND hDlg)
{
    out_smart = 1;    
    SetScrollPos(GetDlgItem(hDlg,SP_TRACK_MAX_BITS_SCROLL),SB_CTL,sp_track_max_bits,TRUE);
    recalc_track_limits();
    sprintf(string,"%d",sp_track_max_bits);
    SetDlgItemText(hDlg,SP_TRACK_MAX_BITS,string);
//    repaint_track_limits(hDlg);
    out_smart = 0;
}        

#ifdef OLD
static void repaint_track_limits(HWND hDlg)
{
    sprintf(string,"%d",sp_track_limit_x_min);
    SetDlgItemText(hDlg,SP_TRACK_CUR_LIMIT_X_MIN,string);
    sprintf(string,"%d",sp_track_limit_x_max);
    SetDlgItemText(hDlg,SP_TRACK_CUR_LIMIT_X_MAX,string);
    sprintf(string,"%d",sp_track_limit_y_min);
    SetDlgItemText(hDlg,SP_TRACK_CUR_LIMIT_Y_MIN,string);
    sprintf(string,"%d",sp_track_limit_y_max);
    SetDlgItemText(hDlg,SP_TRACK_CUR_LIMIT_Y_MAX,string);
}    
#endif

static void recalc_track_limits()
{
  int current_x = dac_data[x_ch];
  int current_y = dac_data[y_ch];
    
  sp_track_limit_x_max = current_x + sp_track_max_bits;
  if(sp_track_limit_x_max > MAX) sp_track_limit_x_max = MAX;
  sp_track_limit_x_min = ((int)current_x) - sp_track_max_bits;
  if(sp_track_limit_x_min < 0) sp_track_limit_x_min = 0;
  sp_track_limit_y_max = current_y + sp_track_max_bits;
  if(sp_track_limit_y_max > MAX) sp_track_limit_y_max = MAX;
  sp_track_limit_y_min = ((int)current_y) - sp_track_max_bits;
  if(sp_track_limit_y_min < 0) sp_track_limit_y_min = 0;
}

static void repaint_sp_v_speed(HWND hDlg)
{
  if(bit16 && (sp_output_ch == 10 || sp_output_ch == 11))
  {
	sprintf(string,"%0.3f",dtov_len16(sp_bits_per_step) / 
        (sp_speed + one_input_time * sp_num_samples) * (double)1000000);
  }
  else
  {
	sprintf(string,"%0.3f",dtov_len(sp_bits_per_step,get_range(sp_output_ch)) / 
        (sp_speed + one_input_time * sp_num_samples) * (double)1000000);
  }
  SetDlgItemText(hDlg,SP_V_SPEED,string);
}

static void repaint_sp_bits_per_step(HWND hDlg)
{
  sprintf(string,"%d",sp_bits_per_step);
  SetDlgItemText(hDlg,SP_BITS_PER_STEP,string);
}

static void repaint_all(HWND hDlg)
{
  CheckDlgButton(hDlg,SP_DITHER0_ON,0);
  CheckDlgButton(hDlg,SP_DITHER0_OFF,0);
  CheckDlgButton(hDlg,SP_DITHER1_ON,0);
  CheckDlgButton(hDlg,SP_DITHER1_OFF,0);
  CheckDlgButton(hDlg,SP_TRACK_MAX,0);
  CheckDlgButton(hDlg,SP_TRACK_MIN,0);
  CheckDlgButton(hDlg,SP_STOP_CRASHING,0);
  CheckDlgButton(hDlg,SP_CRASH_IN_RANGE,0);
  CheckDlgButton(hDlg,SP_CRASH_DO_NOTHING,0);
  CheckDlgButton(hDlg,SP_MODE_ABSOLUTE,0);
  CheckDlgButton(hDlg,SP_MODE_RELATIVE,0);
  CheckDlgButton(hDlg,sp_mode,1);
  CheckDlgButton(hDlg,sp_crash_protection,1);
  CheckDlgButton(hDlg,sp_dither0,1);
  CheckDlgButton(hDlg,sp_dither1,1);
  CheckDlgButton(hDlg,SP_MEASURE_I,sp_measure_i);
  CheckDlgButton(hDlg,SP_MEASURE_Z,sp_measure_z);
  CheckDlgButton(hDlg,SP_MEASURE_2,sp_measure_2);
  CheckDlgButton(hDlg,SP_MEASURE_3,sp_measure_3);
  CheckDlgButton(hDlg,SP_FORWARD,sp_forward);
  CheckDlgButton(hDlg,SP_BACKWARD,sp_backward);
  CheckDlgButton(hDlg,SP_AVG_BOTH,sp_avg_both);
  CheckDlgButton(hDlg,SP_TRACK_AT_MIN,sp_track_at_min);
  CheckDlgButton(hDlg,SP_OFFSET_HOLD_ON,sp_offset_hold);
  CheckDlgButton(hDlg,SP_TRIGGER_ON,sp_trigger);//Mod. 2 Shaowei
  CheckDlgButton(hDlg,SP_STEPTRIGGER_ON,sp_steptrigger);//Mod. 5 Shaowei
  CheckDlgButton(hDlg,SP_START_DELAY_ON,sp_start_delay_on[sp_current_default]);//Mod. 1 Shaowei
  CheckDlgButton(hDlg,SP_FINAL_DELAY_ON,sp_final_delay_on[sp_current_default]);//Mod. 4 Shaowei
  CheckDlgButton(hDlg,SP_STEP_DELAY_ON,sp_step_delay_on[sp_current_default]);//Mod. 5 Shaowei
  CheckDlgButton(hDlg,SP_FB_DELAY_ON,sp_fb_delay_on[sp_current_default]);//Mod. 3 Shaowei
  CheckDlgButton(hDlg,SP_FEEDBACK_AT_MIN,sp_feedback_at_min);
  CheckDlgButton(hDlg,SP_USE_Z_OFFSET_BIAS,sp_use_z_offset_bias);
  sp_calc_scan();
  repaint_sp_z_offset_bias(hDlg);
  repaint_sp_bits_per_step(hDlg);
  repaint_sp_v_speed(hDlg);
  repaint_num_samples(hDlg);
  repaint_num_passes(hDlg);

  repaint_scan_min(hDlg);
  repaint_scan_max(hDlg);
  repaint_scan_min2(hDlg);
  repaint_scan_max2(hDlg);
  repaint_speed(hDlg);
  repaint_channels(hDlg);
  repaint_z_offset(hDlg);
  repaint_track(hDlg);
  repaint_dither_wait(hDlg);
  repaint_feedback(hDlg);
  repaint_move_delay(hDlg);
  repaint_ramp_delay(hDlg);
  //Triggering Shaowei
  repaint_sp_trigger(hDlg);
  sp_enable_abs_rel(hDlg);
  sp_calc_scan();
}

static void repaint_track_current(HWND hDlg)
{
    sprintf(string,"%d",dac_data[x_ch]);
    SetDlgItemText(hDlg,SP_TRACK_CURRENT_X,string);
    sprintf(string,"%d",dac_data[y_ch]);
    SetDlgItemText(hDlg,SP_TRACK_CURRENT_Y,string);
}    

static void repaint_feedback(HWND hDlg)
{
    out_smart = 1;
    CheckDlgButton(hDlg,SP_FEEDBACK_ON,0);
    CheckDlgButton(hDlg,SP_FEEDBACK_OFF,0);
    CheckDlgButton(hDlg,SP_FEEDBACK_ON_EVERY,0);
    switch(sp_feedback)
    {
        case 0:
            CheckDlgButton(hDlg,SP_FEEDBACK_OFF,1);
            break;
        case 1:
            CheckDlgButton(hDlg,SP_FEEDBACK_ON,1);
            break;
        case 2:
            CheckDlgButton(hDlg,SP_FEEDBACK_ON_EVERY,1);
            break;
    }
    sprintf(string,"%d",sp_feedback_wait);
    SetDlgItemText(hDlg,SP_FEEDBACK_WAIT,string);
    sprintf(string,"%d",sp_feedback_every);
    SetDlgItemText(hDlg,SP_FEEDBACK_EVERY,string);
    out_smart = 0;
}

void sp_calc_scan()
{
    unsigned int reference;

    switch(sp_mode)
    {
        case SP_MODE_RELATIVE:
            if(bit16 && (sp_output_ch == 10 || sp_output_ch == 11))
			{
              reference = dac_data[sp_output_ch];
			  reference += (dac_data[extra_ch] & 0xF00 << 4);
              sp_scan_min = max(min((int)reference + sp_rel_min,MAX * 16),0);
              sp_scan_max = max(min((int)reference + sp_rel_max,MAX * 16),0);
			}
			else
			{
              sp_scan_min = max(min((int)dac_data[sp_output_ch] + sp_rel_min,MAX),0);
              sp_scan_max = max(min((int)dac_data[sp_output_ch] + sp_rel_max,MAX),0);
			}
            break;
    }
}

/*
static void repaint_this_sequence(HWND hDlg)
{
	out_smart = 1;
    SendDlgItemMessage(hDlg,PRESCAN_RS_TYPE,CB_SETCURSEL, 
      (WPARAM)SendDlgItemMessage(hDlg,PRESCAN_RS_TYPE, 
      CB_FINDSTRINGEXACT,0,(LPARAM)cur_rs_type),
      (LPARAM)0);
	repaint_sequence_num(hDlg);
    repaint_waits(hDlg);
    repaint_num_samples(hDlg);
    repaint_read_ch(hDlg);
    repaint_ramp_bias(hDlg);
	repaint_rs_state(hDlg);
    out_smart = 0;
}
*/

static void add_sequence(HWND hDlg)

{
	// add a new read step at the end of the read sequence
  if(sp_defaults_data[sp_current_default]->total_steps < MAX_READ_STEPS)
  {
    sp_defaults_data[sp_current_default]->total_steps++;
	sp_current_step = sp_defaults_data[sp_current_default]->total_steps - 1;
	sp_defaults_data[sp_current_default]->sequence[sp_current_step].type = WAIT_TYPE;
	sp_defaults_data[sp_current_default]->sequence[sp_current_step].wait = 100; // 100 microsec wait is default
	strcpy(cur_rs_type,"Wait");
	strcpy(sp_defaults_data[sp_current_default]->sequence[sp_current_step].step_type_string,cur_rs_type);
  }
  else
  {
    sprintf(string,"Read sequence can't exceed %d steps.",MAX_READ_STEPS);
	MessageBox(hDlg,string,"Warning",MB_OK);
  }
}

static void insert_sequence(HWND hDlg)
{
  // insert a read sequence step just before the current step
  int i;

  if(sp_defaults_data[sp_current_default]->total_steps < MAX_READ_STEPS)
  {
    for(i = sp_defaults_data[sp_current_default]->total_steps;i > sp_current_step;i--)
	{
      copy_step(&(sp_defaults_data[sp_current_default]->sequence[i]),&(sp_defaults_data[sp_current_default]->sequence[i - 1]));
	}
    sp_defaults_data[sp_current_default]->total_steps++;
	sp_defaults_data[sp_current_default]->sequence[sp_current_step].type = WAIT_TYPE;
	sp_defaults_data[sp_current_default]->sequence[sp_current_step].wait = 100; // 100 microsec wait is default
	strcpy(cur_rs_type,"Wait");
	strcpy(sp_defaults_data[sp_current_default]->sequence[sp_current_step].step_type_string,cur_rs_type);
  }
  else
  {
    sprintf(string,"Read sequence can't exceed %d steps.",MAX_READ_STEPS);
	MessageBox(hDlg,string,"Warning",MB_OK);
  }
}

static void del_sequence(HWND hDlg)
{
  // deletes the current step of the read sequence
  int i;

  if(sp_defaults_data[sp_current_default]->total_steps > 1)
  {
	for(i = sp_current_step;i < sp_defaults_data[sp_current_default]->total_steps - 1;i++)
	{
      copy_step(&(sp_defaults_data[sp_current_default]->sequence[i]),&(sp_defaults_data[sp_current_default]->sequence[i + 1]));
	}
    sp_defaults_data[sp_current_default]->total_steps--;
	if(sp_current_step > sp_defaults_data[sp_current_default]->total_steps - 1) sp_current_step = sp_defaults_data[sp_current_default]->total_steps - 1;
	strcpy(cur_rs_type,sp_defaults_data[sp_current_default]->sequence[sp_current_step].step_type_string);
  }
  else
  {
    sprintf(string,"Read sequence can't go below 1 step.",MAX_READ_STEPS);
	MessageBox(hDlg,string,"Warning",MB_OK);
  }    
  check_crash_protection(hDlg);
}

/*
static void repaint_this_sequence(HWND hDlg)
{
	out_smart = 1;
    SendDlgItemMessage(hDlg,PRESCAN_RS_TYPE,CB_SETCURSEL, 
      (WPARAM)SendDlgItemMessage(hDlg,PRESCAN_RS_TYPE, 
      CB_FINDSTRINGEXACT,0,(LPARAM)cur_rs_type),
      (LPARAM)0);
	repaint_sequence_num(hDlg);
    repaint_waits(hDlg);
    repaint_num_samples(hDlg);
    repaint_read_ch(hDlg);
    repaint_ramp_bias(hDlg);
	repaint_rs_state(hDlg);
    out_smart = 0;
}
*/


static void summary(HWND hDlg)
{
    static FARPROC  lpfnDlgProc;
    
    lpfnDlgProc = MakeProcInstance(SummaryDlg,hInst);
    glob_data = &(sp_defaults_data[sp_current_default]);
    DialogBox(hInst,"COMMENTDLG",hDlg,lpfnDlgProc);
    FreeProcInstance(lpfnDlgProc);                  
}

static void make_rs_types_list()
{
    rs_types_list.listbox.str[0] = (char *)malloc(sizeof(char) * RS_TYPES_STR_MAX);
    strcpy(rs_types_list.listbox.str[0],"Read");

    rs_types_list.listbox.str[1] = (char *)malloc(sizeof(char) * RS_TYPES_STR_MAX);
    strcpy(rs_types_list.listbox.str[1],"Wait");

    rs_types_list.listbox.str[2] = (char *)malloc(sizeof(char) * RS_TYPES_STR_MAX);
    strcpy(rs_types_list.listbox.str[2],"Feedback");

    rs_types_list.listbox.str[3] = (char *)malloc(sizeof(char) * RS_TYPES_STR_MAX);
    strcpy(rs_types_list.listbox.str[3],"Dither Ch 0");

    rs_types_list.listbox.str[4] = (char *)malloc(sizeof(char) * RS_TYPES_STR_MAX);
    strcpy(rs_types_list.listbox.str[4],"Dither Ch 1");

    rs_types_list.listbox.str[5] = (char *)malloc(sizeof(char) * RS_TYPES_STR_MAX);
    strcpy(rs_types_list.listbox.str[5],"Step Output Ch");

    rs_types_list.listbox.num = 6;
}

static void free_rs_types_list()
{
    int i;
    
    for(i = 0;i < rs_types_list.listbox.num;i++)
    {
        free(rs_types_list.listbox.str[i]);
        free(rs_types_list.name[i]);
    }
}