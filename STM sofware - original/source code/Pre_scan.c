#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <dos.h>
#include <sys\types.h>
#include <direct.h>
#include <time.h>	// added in Mod 11, Bias_RampTo
//#include "mw_dirent.h"
#include "common.h"
#include "stm.h"
#include "dio.h"
#include "data.h"
#include "scan.h"
#include "clock.h"
#include "file.h"
#include "pre_scan.h"
#include "summary.h"
#include "data.h"
#include "serial.h"

extern unsigned int scan_scale;
extern int scan_freq;
extern int feedback;
extern unsigned int scan_z;
extern unsigned int sample_bias;
extern unsigned int tip_gain;
extern unsigned int i_setpoint;
extern float scan_x_gain,
             scan_y_gain;
extern unsigned int dac_data[];
extern datadef *data,**glob_data;
extern datadef *scan_defaults_data[];
extern int scan_current_default;
extern unsigned int bit16;
extern int i_set_range,z_offset_range,sample_bias_range,scan_now;
extern int scan_end_fb; //Mod. Shaowei Constant Height mode
extern int scan_end_auto;
extern int scan_end_retract;

extern HANDLE   hInst;
extern BOOL FAR PASCAL IOCtrlDlg(HWND,unsigned,WPARAM,LPARAM);
extern HWND ScnDlg;
extern char string[];
extern char cur_sample[];
extern char cur_dosed[];
extern char *current_dir_stm,*current_file_stm,*initial_dir_stm;
extern int file_stm_count;

int bias_rampto_continue = 0;	  // Mod 11, Bias_RampTo
unsigned int bias_rampto_speed1 = 10; // Mod 11, Bias_RampTo
unsigned int bias_rampto_speed2 = 10; // Mod 11, Bias_RampTo
unsigned int bias_rampto_speed3 = 10; // Mod 11, Bias_RampTo
unsigned int bias_rampto_speed4 = 10; // Mod 11, Bias_RampTo
int current_rampto_continue = 0;	  // Mod 11, Current_RampTo
unsigned int current_rampto_speed1 = 10; // Mod 11, Current_RampTo
unsigned int current_rampto_speed2 = 10; // Mod 11, Current_RampTo
unsigned int current_rampto_speed3 = 10; // Mod 11, Current_RampTo
unsigned int current_rampto_speed4 = 10; // Mod 11, Current_RampTo
double bias_target;             // Mod 11, Bias_RampTo
double bias_target1;            // Mod 11, Bias_RampTo
double bias_target2;            // Mod 11, Bias_RampTo
double bias_target3;            // Mod 11, Bias_RampTo
double bias_target4;			// Mod 11, Bias_RampTo
unsigned int bias_target_int;	// Mod 11, Bias_RampTo
MSG stop_message;				// Mod 11, Bias_RampTo
double current_target ;         // Mod 11, Current_RampTo
double current_target1 ;        // Mod 11, Current_RampTo
double current_target2 ;        // Mod 11, Current_RampTo
double current_target3 ;        // Mod 11, Current_RampTo
double current_target4 ;		// Mod 11, Current_RampTo
unsigned int current_target_int;	  // Mod 11, Current_RampTo




int z_offset_crash_protect = 1;
int digital_abort = 1;
double a_per_v = 200;
double z_a_per_v = 30;
double          i_set;          /* this is the current value in nanoamps
                                 * entered by the user */

//HWND Prescan2Wnd = NULL;
//BOOL Prescan2On = FALSE;
static int out_smart = 0;
static int first_time = 1;
SAMPLELIST sample_list,dosed_list;
SAMPLELIST rs_types_list;
char cur_rs_type[STEP_TYPE_STR_MAX];
int current_step = 0;

static void check_digital_feedback(HWND);
static int check_crash_protection(HWND);
static void enable_crash_protection(HWND,BOOL);
static void enable_digital(HWND,BOOL);

static void repaint_this_sequence(HWND);
static void add_sequence(HWND);
static void insert_sequence(HWND);
static void del_sequence();
static void repaint_sequence_num(HWND);
static void summary(HWND);
static void make_rs_types_list();
static void free_rs_types_list();

void repaint_step_delay(HWND);
void repaint_inter_step(HWND);
void repaint_inter_line(HWND);
void repaint_tip_spacing(HWND);
void repaint_prescan_conseq(HWND);
void repaint_prescan_over(HWND);
void repaint_scan_scale(HWND);
static void repaint_gains(HWND);
void repaint_crash(HWND);
void repaint_digital_max(HWND);
static void repaint_waits(HWND);
static void repaint_read_ch(HWND);
static void repaint_num_samples(HWND);
static void repaint_ramp_bias(HWND);
static void repaint_rs_state(HWND);
static void repaint_angs_calibration(HWND);
static void repaint_prescan_temp(HWND);
static void repaint_bias_rampto(HWND);	// Mod 11, Bias_RampTo
static void repaint_current_rampto(HWND);
static void repaint_current_target(HWND);
static void repaint_bias_target(HWND);	// Mod 11, Bias_RampTo

void sleep_ms(unsigned);  // Mod 11, Bias_RampTo

BOOL FAR PASCAL PrescanDlg(HWND,unsigned,WPARAM,LPARAM);
BOOL FAR PASCAL Prescan2Dlg(HWND,unsigned,WPARAM,LPARAM);

BOOL FAR PASCAL PrescanDlg(HWND hDlg, unsigned Message, WPARAM wParam, LPARAM lParam)
{
  int i;
  int id;
  int delt,ddelt,mini,maxi;
  unsigned int x_gain,y_gain,z_gain,z2_gain,z;
//  double d;

  HWND handle;
  static FARPROC lpfnDlgProc;

  switch(Message) {
    case WM_INITDIALOG:
      scan_now = 0;

      switch(dac_data[gain_ch] & z_gain_bits)
      {
        case Z_TENTH_GAIN:
          scan_freq = 0;
          break;            
        case Z_ONE_GAIN:
          scan_freq = 1;
          break;            
        case Z_TEN_GAIN:
          scan_freq = 2;
          break;
      }
      switch(dac_data[gain_ch] & z2_gain_bits)
      {
        case Z2_TENTH_GAIN:
          scan_scale = 0;
          break;            
        case Z2_ONE_GAIN:
          scan_scale = 1;
          break;            
        case Z2_TEN_GAIN:
          scan_scale = 2;
          break;
      }

	  calc_gains(&x_gain,&y_gain,&z_gain,&z2_gain);
      handle = GetParent(hDlg);
      SetScrollRange(GetDlgItem(hDlg,SCAN_Z_SCROLL),SB_CTL,scan_z_min,scan_z_max,FALSE);
//      SetScrollRange(GetDlgItem(hDlg,SCAN_NUM_SCROLL),SB_CTL,scan_num_min,scan_num_max,FALSE);
      SetScrollRange(GetDlgItem(hDlg,TIP_CURRENT_SCROLL),SB_CTL,tip_current_min,tip_current_max,FALSE);
      SetScrollRange(GetDlgItem(hDlg,SAMPLE_BIAS_SCROLL),SB_CTL,sample_bias_min,sample_bias_max,FALSE);
      SetScrollRange(GetDlgItem(hDlg,PRESCAN_RS_SCROLL),SB_CTL,sample_bias_min,sample_bias_max,FALSE);
      SetScrollRange(GetDlgItem(hDlg,INTER_STEP_SCROLL),SB_CTL,INTER_STEP_MIN_DELAY,INTER_STEP_MAX_DELAY,FALSE);
      SetScrollRange(GetDlgItem(hDlg,INTER_LINE_SCROLL),SB_CTL,INTER_LINE_MIN_DELAY,INTER_LINE_MAX_DELAY,FALSE);
      SetScrollRange(GetDlgItem(hDlg,TIP_SPACING_SCROLL),SB_CTL,TIP_SPACING_MIN,TIP_SPACING_MAX,FALSE);
      SetScrollRange(GetDlgItem(hDlg,PRESCAN_CONSEQ_SCROLL),SB_CTL,
            DIGITAL_FEEDBACK_REREAD_MIN,DIGITAL_FEEDBACK_REREAD_MAX,FALSE);

      switch(SD->crash_protection)
      {
        case CRASH_DO_NOTHING:
          CheckDlgButton(hDlg,CRASH_DO_NOTHING,1);
          CheckDlgButton(hDlg,CRASH_STOP,0);
          CheckDlgButton(hDlg,CRASH_AUTO_Z,0);
          CheckDlgButton(hDlg,CRASH_MINMAX_Z,0);
          break;
        case CRASH_STOP:
          CheckDlgButton(hDlg,CRASH_DO_NOTHING,0);
          CheckDlgButton(hDlg,CRASH_STOP,1);
          CheckDlgButton(hDlg,CRASH_AUTO_Z,0);
          CheckDlgButton(hDlg,CRASH_MINMAX_Z,0);
          break;
        case CRASH_AUTO_Z:
          CheckDlgButton(hDlg,CRASH_DO_NOTHING,0);
          CheckDlgButton(hDlg,CRASH_STOP,0);
          CheckDlgButton(hDlg,CRASH_AUTO_Z,1);
          CheckDlgButton(hDlg,CRASH_MINMAX_Z,0);
          break;
        case CRASH_MINMAX_Z:
          CheckDlgButton(hDlg,CRASH_DO_NOTHING,0);
          CheckDlgButton(hDlg,CRASH_STOP,0);
          CheckDlgButton(hDlg,CRASH_AUTO_Z,0);
          CheckDlgButton(hDlg,CRASH_MINMAX_Z,1);
          break;
      }

	  // The following 3 lines added in Mod 11
	  CheckDlgButton( hDlg, PRESCAN_FEEDBACK_MOVE_ON, SD->scan_feedback );
      CheckDlgButton( hDlg, PRESCAN_DITHER0_MOVE_ON, SD->scan_dither0 );
      CheckDlgButton( hDlg, PRESCAN_DITHER1_MOVE_ON, SD->scan_dither1 );
      // Mod. Shaowei Post Scan
	  CheckDlgButton( hDlg, PRESCAN_FEEDBACK_END_ON, scan_end_fb );
	  CheckDlgButton(hDlg, PRESCAN_TIPAUTO_END_ON, scan_end_auto);
	  CheckDlgButton(hDlg, PRESCAN_RETRACT_END_ON, scan_end_retract);



#ifdef OLD
      //CheckDlgButton(hDlg,PRESCAN_FEEDBACK_MOVE_ON,SD->scan_feedback);
      //CheckDlgButton(hDlg,PRESCAN_DITHER0_MOVE_ON,SD->scan_dither0);
      //CheckDlgButton(hDlg,PRESCAN_DITHER1_MOVE_ON,SD->scan_dither1);

      switch(read_ch)
      {
        case 0:
          CheckDlgButton(hDlg,PRESCAN_CHANNEL_0,1);
          break;            
        case 1:
          CheckDlgButton(hDlg,PRESCAN_CHANNEL_1,1);
          break;            
        case 2:
          CheckDlgButton(hDlg,PRESCAN_CHANNEL_2,1);
          break;            
        case 3:
          CheckDlgButton(hDlg,PRESCAN_CHANNEL_3,1);
          break;
      }

	  if (SD->scan_feedback && read_ch == zi_ch)
      {
        CheckDlgButton(hDlg,FEEDBACK_ON,1);
        CheckDlgButton(hDlg,FEEDBACK_OFF,0);
        CheckDlgButton(hDlg,FEEDBACK_ON_CURRENT,0);
      } else if (SD->scan_feedback)    /* feedback on,reading current */
      {
        CheckDlgButton(hDlg,FEEDBACK_ON,0);
        CheckDlgButton(hDlg,FEEDBACK_OFF,0);
        CheckDlgButton(hDlg,FEEDBACK_ON_CURRENT,1);
      } else
      {                       /* feedback off,reading current */
        CheckDlgButton(hDlg,FEEDBACK_ON,0);
        CheckDlgButton(hDlg,FEEDBACK_OFF,1);
        CheckDlgButton(hDlg,FEEDBACK_ON_CURRENT,0);
        read_ch = i_in_ch;
      }
      if (read_ch == zi_ch)
      {
        enable_digital(hDlg,TRUE);
      } else
        enable_digital(hDlg,FALSE);
#endif

      check_digital_feedback(hDlg);
      check_crash_protection(hDlg);
      if(SD->digital_feedback)
            CheckDlgButton(hDlg,WAIT_FOR_CURRENT,1);

      switch(scan_freq)
      {
        case 0:
            CheckDlgButton(hDlg,SCAN_FREQ_100,1);
            break;
        case 1:
            CheckDlgButton(hDlg,SCAN_FREQ_1,1);
            break;
        case 2:
            CheckDlgButton(hDlg,SCAN_FREQ_10,1);
            break;
      }
      
	  switch (tip_gain)
	  {
	  case 8:
		  CheckDlgButton(hDlg, TIP_CURRENT_GAIN_8, 1);
		  CheckDlgButton(hDlg, TIP_CURRENT_GAIN_9, 0);
		  CheckDlgButton(hDlg, TIP_CURRENT_GAIN_10, 0);
		  CheckDlgButton(hDlg, TIP_CURRENT_GAIN_11, 0);
		  SetDlgItemText(hDlg, TIP_CURRENT_RANGE_TEXT, "1 to 100 nA");
		  break;
	  case 9:
		  CheckDlgButton(hDlg, TIP_CURRENT_GAIN_8, 0);
		  CheckDlgButton(hDlg, TIP_CURRENT_GAIN_9, 1);
		  CheckDlgButton(hDlg, TIP_CURRENT_GAIN_10, 0);
		  CheckDlgButton(hDlg, TIP_CURRENT_GAIN_11, 0);
		  SetDlgItemText(hDlg, TIP_CURRENT_RANGE_TEXT, "0.1 to 10 nA");
		  break;
	  case 10:
		  CheckDlgButton(hDlg, TIP_CURRENT_GAIN_8, 0);
		  CheckDlgButton(hDlg, TIP_CURRENT_GAIN_9, 0);
		  CheckDlgButton(hDlg, TIP_CURRENT_GAIN_10, 1);
		  CheckDlgButton(hDlg, TIP_CURRENT_GAIN_11, 0);
		  SetDlgItemText(hDlg, TIP_CURRENT_RANGE_TEXT, "0.01 to 1 nA");
		  break;
	  case 11:
		  CheckDlgButton(hDlg, TIP_CURRENT_GAIN_11, 1);
		  CheckDlgButton(hDlg, TIP_CURRENT_GAIN_8, 0);
		  CheckDlgButton(hDlg, TIP_CURRENT_GAIN_9, 0);
		  CheckDlgButton(hDlg, TIP_CURRENT_GAIN_10, 0);
		  SetDlgItemText(hDlg, TIP_CURRENT_RANGE_TEXT, "0.001 to 0.1 nA");
		  break;

	  }
	  
	  /* 2019-3-25 Shaowei Add gain 11
	  switch(tip_gain)
      {
        case 8:
            CheckDlgButton(hDlg,TIP_CURRENT_GAIN_8,1);
            SetDlgItemText(hDlg,TIP_CURRENT_RANGE_TEXT,"1 to 100 nA");
            break;
        case 9:
            CheckDlgButton(hDlg,TIP_CURRENT_GAIN_9,1);
            SetDlgItemText(hDlg,TIP_CURRENT_RANGE_TEXT,"0.1 to 10 nA");
            break;
        case 10:
            CheckDlgButton(hDlg,TIP_CURRENT_GAIN_10,1);
            SetDlgItemText(hDlg,TIP_CURRENT_RANGE_TEXT,"0.01 to 1 nA");
            break;
        case 11:
            CheckDlgButton(hDlg,TIP_CURRENT_GAIN_11,1);
            SetDlgItemText(hDlg,TIP_CURRENT_RANGE_TEXT,"0.001 to 0.1 nA");
            break;
      }
      */


	  switch(SD->step_delay_fixed)
      {
        case TRUE:
            CheckDlgButton(hDlg,DELAY_FIXED,1);
            break;
        case FALSE:
            CheckDlgButton(hDlg,DELAY_FREQ_DEP,1);
            break;
      }

	  CheckDlgButton(hDlg,Z_OFFSET_CRASH,z_offset_crash_protect);
      CheckDlgButton(hDlg,DIGITAL_ABORT,digital_abort);

	  make_rs_types_list();
      init_listbox(hDlg,PRESCAN_RS_TYPE,&(rs_types_list.listbox));
      if(first_time)
      {
        first_time = 0;
	    current_step = 0;
        strcpy(cur_rs_type,SD->sequence[0].step_type_string);
		SendDlgItemMessage(hDlg,PRESCAN_RS_TYPE,CB_SETCURSEL,
                (WPARAM) SendDlgItemMessage(hDlg,PRESCAN_RS_TYPE,
                CB_FINDSTRINGEXACT,0,(LPARAM)cur_rs_type),
                (LPARAM)0);
	  }
      else
	  {
		  SendDlgItemMessage(hDlg,PRESCAN_RS_TYPE,CB_SETCURSEL,
                (WPARAM) SendDlgItemMessage(hDlg,PRESCAN_RS_TYPE,
                CB_FINDSTRINGEXACT,0,(LPARAM)cur_rs_type),
                (LPARAM)0);
	  }

      repaint_digital_max(hDlg);
      repaint_crash(hDlg);
      repaint_scan_scale(hDlg);
      repaint_step_delay(hDlg);
      repaint_inter_step(hDlg);
      repaint_inter_line(hDlg);
      repaint_scan_z(hDlg);
      repaint_scan_freq(hDlg);
      repaint_sample_bias(hDlg);
      repaint_tip_current(hDlg);
      repaint_tip_spacing(hDlg);
      repaint_prescan_conseq(hDlg);
      repaint_this_sequence(hDlg);
	  repaint_bias_rampto( hDlg );	// Mod 11, Bias_RampTo
	  repaint_current_rampto( hDlg );
	  repaint_current_target( hDlg );
	  repaint_bias_target( hDlg );// Mod 11, Current_RampTo
	  repaint_prescan_over(hDlg);

      break;

	case WM_PAINT:
      break;
    case WM_LBUTTONDOWN:
      break;

    case WM_HSCROLL:
    case WM_VSCROLL:
	  id = getscrollid();
      switch(id)
      {
        case PRESCAN_CONSEQ_SCROLL:
          i = (int)(SD->digital_feedback_reread);
          mini = DIGITAL_FEEDBACK_REREAD_MIN;
          maxi = DIGITAL_FEEDBACK_REREAD_MAX;
          delt = PRESCAN_CONSEQ_DELT;
          ddelt = PRESCAN_CONSEQ_DDELT;
          break;
        case TIP_SPACING_SCROLL:
          i = (int)(SD->tip_spacing * (float)FTOD_SCALE);
          mini = TIP_SPACING_MIN;
          maxi = TIP_SPACING_MAX;
          delt = TIP_SPACING_DELT;
          ddelt = TIP_SPACING_DDELT;
          break;
        case INTER_STEP_SCROLL:
          i = (int)SD->inter_step_delay;
          mini = INTER_STEP_MIN_DELAY;
          maxi = INTER_STEP_MAX_DELAY;
          delt = INTER_STEP_DELT;
          ddelt = INTER_STEP_DDELT;
          break;
        case INTER_LINE_SCROLL:
          i = (int)SD->inter_line_delay / 1000;
          mini = INTER_LINE_MIN_DELAY;
          maxi = INTER_LINE_MAX_DELAY;
          delt = INTER_LINE_DELT;
          ddelt = INTER_LINE_DDELT;
          break;
        case SCAN_Z_SCROLL:
          i = (int)scan_z;
          mini = scan_z_min;
          maxi = scan_z_max;
          delt = scan_z_delt;
          ddelt = scan_z_ddelt;
          break;
#ifdef OLD
        case SCAN_NUM_SCROLL:
          i = (int)SDR.num_samples;
          mini = scan_num_min;
          maxi = scan_num_max;
          delt = scan_num_delt;
          ddelt = scan_num_ddelt;
          break;
#endif
        case SAMPLE_BIAS_SCROLL:
          i = (int)sample_bias;
          mini = sample_bias_min;
          maxi = sample_bias_max;
          delt = sample_bias_delt;
          ddelt = sample_bias_ddelt;
          break;
        case PRESCAN_RS_SCROLL:
          i = (int)SD->sequence[current_step].step_bias;
          mini = sample_bias_min;
          maxi = sample_bias_max;
          delt = sample_bias_delt;
          ddelt = sample_bias_ddelt;
          break;
        case TIP_CURRENT_SCROLL:
          i = (int)tip_current_max - i_setpoint;
          mini = tip_current_min;
          maxi = tip_current_max;
          delt = tip_current_delt;
          ddelt = tip_current_ddelt;
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
      switch(id)
      {
        case PRESCAN_CONSEQ_SCROLL:
          if(!out_smart)
          {
            SD->digital_feedback_reread = i;
            repaint_prescan_conseq(hDlg);
          }
          break;
        case TIP_SPACING_SCROLL:
          SD->tip_spacing = ((float)i) / (float)FTOD_SCALE;
          repaint_tip_spacing(hDlg);
          break;
        case INTER_STEP_SCROLL:
          SD->inter_step_delay = i;
          repaint_inter_step(hDlg);
          break;
        case INTER_LINE_SCROLL:
          SD->inter_line_delay = i * 1000;
          repaint_inter_line(hDlg);
          break;
        case SCAN_Z_SCROLL:
          if(!out_smart)
          {
//            if(i != scan_z) enable_z_offset_send(hDlg,1);
            scan_z = (unsigned int)i;
            repaint_scan_z(hDlg);
          }
          break;
#ifdef OLD
        case SCAN_NUM_SCROLL:
          SDR.num_samples = (unsigned int)i;
          repaint_scan_num(hDlg);
          break;
#endif
        case SAMPLE_BIAS_SCROLL:
          if(i == ZERO)
          {
            if(sample_bias > (unsigned int)i) sample_bias = ZERO - 1;
            else sample_bias = ZERO + 1;
          }
          else sample_bias = (unsigned int)i;
          //bias(sample_bias);
          bias_serial(sample_bias);
          repaint_sample_bias(hDlg);
          break;
        case PRESCAN_RS_SCROLL:
	      if(!out_smart)
		  {
			SD->sequence[current_step].step_bias = (unsigned int)i;
            repaint_this_sequence(hDlg);
          }
          break;
        case TIP_CURRENT_SCROLL:
          i_setpoint = (unsigned int)(tip_current_max - i);
          //tip_current(i_setpoint);
          tip_current_serial(i_setpoint);
          repaint_tip_current(hDlg);
          break;
	  }
      break;

    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
		// read sequence controls
	    case PRESCAN_NEXT:
          if(current_step < SD->total_steps - 1)
          {
            current_step++;
			strcpy(cur_rs_type,SD->sequence[current_step].step_type_string);
            repaint_this_sequence(hDlg);
          }
          break;
        case PRESCAN_PREVIOUS:
          if(current_step > 0)
          {
            current_step--;
			strcpy(cur_rs_type,SD->sequence[current_step].step_type_string);
            repaint_this_sequence(hDlg);
          }
          break;
        case PRESCAN_ADD:
          add_sequence(hDlg);
          repaint_this_sequence(hDlg);
          break;
		case PRESCAN_INSERT:
          insert_sequence(hDlg);
          repaint_this_sequence(hDlg);
          break;
        case PRESCAN_DEL:
          if(SD->total_steps > 1) 
          {
            del_sequence();
            repaint_this_sequence(hDlg);
          }
          break;
        case PRESCAN_SUMMARY:
          check_crash_protection(hDlg);
          summary(hDlg);
          break;
		  
		  //Shaowei 2019-3-25 add save sequence
		case PRESCAN_SAVE_SEQUENCE:
			glob_data = &scan_defaults_data[scan_current_default];
			file_save_as(hDlg, hInst, FTYPE_STP);
			break;
		case PRESCAN_LOAD_SEQUENCE:
			if (scan_current_default == 0)
			{
				sprintf(string, "You are overwriting sequence #1, continue?");
				if (MessageBox(hDlg, string,
					"Warning", MB_OKCANCEL | MB_ICONEXCLAMATION) == IDCANCEL)
					break;
			}
			glob_data = &scan_defaults_data[scan_current_default];
			file_open(hDlg, hInst, FTYPE_STP, 0, "*.stp");
			break;







        case PRESCAN_RS_VOLTS:
		  if(!out_smart)
		  {
            GetDlgItemText(hDlg,PRESCAN_RS_VOLTS,string,9);
            i = vtod(atof(string),get_range(SD->sequence[current_step].out_ch));
            i = min(max(i,sample_bias_min),sample_bias_max);
            SD->sequence[current_step].step_bias = (unsigned int)i;
		  }
          break;
        case PRESCAN_RS_BITS:
		  if(!out_smart)
		  {
            GetDlgItemText(hDlg,PRESCAN_RS_BITS,string,9);
			i = atoi(string);
            i = min(max(i,sample_bias_min),sample_bias_max);
            SD->sequence[current_step].step_bias = (unsigned int)i;
		  }
          break;
        case PRESCAN_RS_WAIT:
		  if(!out_smart)
		  {
            GetDlgItemText(hDlg,LOWORD(wParam),string,15);
            i = atoi(string);
            SD->sequence[current_step].wait = min(max(i,DITHER_WAIT_MIN),DITHER_WAIT_MAX);
            if(i != SD->sequence[current_step].wait) repaint_waits(hDlg);
		  }
          break;
        case PRESCAN_RS_INCH:
		  if(!out_smart)
		  {
            GetDlgItemText(hDlg,LOWORD(wParam),string,10);
            i = atoi(string);
            SD->sequence[current_step].in_ch = min(max(i,0),DIO_INPUT_CHANNELS - 1);
            if(SD->sequence[current_step].in_ch != i) repaint_read_ch(hDlg);
		  }
		  break;
        case PRESCAN_RS_NUM:
		  if(!out_smart)
		  {
            GetDlgItemText(hDlg,SCAN_NUM_EDIT,string,9);
            i = atoi(string);
            SD->sequence[current_step].num_reads = min(max(i,scan_num_min),scan_num_max);
            if(SD->sequence[current_step].num_reads != i) repaint_num_samples(hDlg);
		  }
		  break;
        case PRESCAN_RS_OUTCH:
		  if(!out_smart)
		  {
            GetDlgItemText(hDlg,LOWORD(wParam),string,10);
            i = atoi(string);
            SD->sequence[current_step].out_ch = min(max(i,0),DIO_OUTPUT_CHANNELS - 1);
            if(SD->sequence[current_step].out_ch != i) repaint_read_ch(hDlg);
		  }
	      break;
        case PRESCAN_RS_ON:
		  if(!out_smart)
		  {
		    CheckDlgButton(hDlg,PRESCAN_RS_OFF,0);
		    CheckDlgButton(hDlg,PRESCAN_RS_ON,1);
		    SD->sequence[current_step].state = 1;
		  }
	      break;
        case PRESCAN_RS_OFF:
		  if(!out_smart)
		  {
		    CheckDlgButton(hDlg,PRESCAN_RS_OFF,1);
		    CheckDlgButton(hDlg,PRESCAN_RS_ON,0);
		    SD->sequence[current_step].state = 0;
		  }
		  break;
		case PRESCAN_RS_TYPE:
		  switch(getcombomsg())
          {
            case CBN_SELCHANGE:
              i = SendDlgItemMessage(hDlg,PRESCAN_RS_TYPE,CB_GETCURSEL,0,0);
              strcpy(cur_rs_type,rs_types_list.listbox.str[ 
                        rs_types_list.listbox.index[i]]);
			  strcpy(SD->sequence[current_step].step_type_string,cur_rs_type);

		      // update sequence step type
			  if(!strcmp(cur_rs_type,"Feedback"))
			  {
				SD->sequence[current_step].type = FEEDBACK_TYPE;
				SD->sequence[current_step].state = 1;
			  }
			  else if(!strcmp(cur_rs_type,"Dither Ch 0"))
			  {
				SD->sequence[current_step].type = DITHER0_TYPE;
				SD->sequence[current_step].state = 0;
			  }
			  else if(!strcmp(cur_rs_type,"Dither Ch 1"))
			  {
				SD->sequence[current_step].type = DITHER1_TYPE;
				SD->sequence[current_step].state = 0;
			  }
			  else if(!strcmp(cur_rs_type,"Wait"))
			  {
				SD->sequence[current_step].type = WAIT_TYPE;
				SD->sequence[current_step].wait = 100;
			  }
			  else if(!strcmp(cur_rs_type,"Read"))
			  {
				SD->sequence[current_step].type = READ_TYPE;
				SD->sequence[current_step].in_ch = zi_ch;
				SD->sequence[current_step].num_reads = 10;
			  }
			  else if(!strcmp(cur_rs_type,"Step Output Ch"))
			  {
				SD->sequence[current_step].type = STEP_OUTPUT_TYPE;
				SD->sequence[current_step].out_ch = sample_bias_ch;
				SD->sequence[current_step].step_bias = sample_bias;
			  }

              check_crash_protection(hDlg);
			  repaint_this_sequence(hDlg);
          }

		  break;

		// movement params
        case PRESCAN_FEEDBACK_MOVE_ON:
          SD->scan_feedback = IsDlgButtonChecked(hDlg,PRESCAN_FEEDBACK_MOVE_ON);
          check_crash_protection(hDlg);
          check_digital_feedback(hDlg);
          break;

// constant height mode
		case PRESCAN_FEEDBACK_END_ON:
          scan_end_fb = IsDlgButtonChecked(hDlg,PRESCAN_FEEDBACK_END_ON);
          check_crash_protection(hDlg);
          check_digital_feedback(hDlg);
		  break;

// Post Scan Shaowei
		case PRESCAN_TIPAUTO_END_ON:
			scan_end_auto = IsDlgButtonChecked(hDlg, PRESCAN_TIPAUTO_END_ON);
			break;

		case PRESCAN_RETRACT_END_ON:
			scan_end_retract = IsDlgButtonChecked(hDlg, PRESCAN_RETRACT_END_ON);
			break;



        case PRESCAN_DITHER0_MOVE_ON:
          SD->scan_dither0 = IsDlgButtonChecked(hDlg,PRESCAN_DITHER0_MOVE_ON);
          break;
        case PRESCAN_DITHER1_MOVE_ON:
          SD->scan_dither1 = IsDlgButtonChecked(hDlg,PRESCAN_DITHER1_MOVE_ON);
          break;

#ifdef OLD_READ_SEQUENCE
        case PRESCAN_RECORD_CH:
          SDR.record = IsDlgButtonChecked(hDlg,LOWORD(wParam));
          check_crash_protection(hDlg);
          break;
        case PRESCAN_FEEDBACK_READ_ON:
          SDR.feedback = IsDlgButtonChecked(hDlg,PRESCAN_FEEDBACK_READ_ON);
          break;
	    case PRESCAN_READ_RAMPBIAS:
          SDR.do_ramp_bias = IsDlgButtonChecked(hDlg,PRESCAN_READ_RAMPBIAS);
          break;
	    case PRESCAN_READ_RAMPCH:
          GetDlgItemText(hDlg,LOWORD(wParam),string,10);
          i = atoi(string);
          SDR.ramp_ch = min(max(i,0),16 - 1);
          break;
        case PRESCAN_DITHER0_READ_ON:
          SDR.dither0 = IsDlgButtonChecked(hDlg,PRESCAN_DITHER0_READ_ON);
          break;
        case PRESCAN_DITHER1_READ_ON:
          SDR.dither1 = IsDlgButtonChecked(hDlg,PRESCAN_DITHER1_READ_ON);
          break;
        case RAMPBIAS_VOLTS_EDIT:
          GetDlgItemText(hDlg,RAMPBIAS_VOLTS_EDIT,string,9);
          i = vtod(atof(string),sample_bias_range);
          i = min(max(i,sample_bias_min),sample_bias_max);
          SDR.ramp_value = (unsigned int)i;
          break;
        case PRESCAN_READ_CH:
          GetDlgItemText(hDlg,LOWORD(wParam),string,10);
          i = atoi(string);
          SDR.read_ch = min(max(i,0),DIO_INPUT_CHANNELS - 1);
          if(SDR.read_ch != i) repaint_read_ch(hDlg);
          break;
        case SCAN_NUM_EDIT:
          GetDlgItemText(hDlg,SCAN_NUM_EDIT,string,9);
          i = atoi(string);
          SDR.num_samples = min(max(i,scan_num_min),scan_num_max);
          if(SDR.num_samples != i) repaint_num_samples(hDlg);
          break;
        case PRESCAN_WAIT_1:
          GetDlgItemText(hDlg,LOWORD(wParam),string,15);
          i = atoi(string);
          SDR.wait1 = min(max(i,DITHER_WAIT_MIN),DITHER_WAIT_MAX);
          if(i != SDR.wait1) repaint_waits(hDlg);
          break;
        case PRESCAN_WAIT_2:
          GetDlgItemText(hDlg,LOWORD(wParam),string,15);
          i = atoi(string);
          SDR.wait2 = min(max(i,DITHER_WAIT_MIN),DITHER_WAIT_MAX);
          if(i != SDR.wait2) repaint_waits(hDlg);
          break;
        case PRESCAN_WAIT_3:
          GetDlgItemText(hDlg,LOWORD(wParam),string,15);
          i = atoi(string);
          SDR.wait3 = min(max(i,DITHER_WAIT_MIN),DITHER_WAIT_MAX);
          if(i != SDR.wait3) repaint_waits(hDlg);
          break;
	    case PRESCAN_WAIT_4:
          GetDlgItemText(hDlg,LOWORD(wParam),string,15);
          i = atoi(string);
          SDR.wait4 = min(max(i,DITHER_WAIT_MIN),DITHER_WAIT_MAX);
          if(i != SDR.wait4) repaint_waits(hDlg);
          break;
#endif
		  
		// digital feedback controls
        case DIGITAL_ABORT:
          digital_abort = IsDlgButtonChecked(hDlg,DIGITAL_ABORT);
            break;
        case PRESCAN_CONSEQ:
          if(!out_smart)
          {
            GetDlgItemText(hDlg,PRESCAN_CONSEQ,string,15);
            SD->digital_feedback_reread  = atoi(string);
            SD->digital_feedback_reread = min(max(SD->digital_feedback_reread, 
                    DIGITAL_FEEDBACK_REREAD_MIN),DIGITAL_FEEDBACK_REREAD_MAX);
          }
          break;
        case DIGITAL_MAX_TRY:
          if(!out_smart)
          {
            GetDlgItemText(hDlg,DIGITAL_MAX_TRY,string,15);
            SD->digital_feedback_max = (int)atof(string);
            SD->digital_feedback_max = min(max(SD->digital_feedback_max, 
                    DIGITAL_FEEDBACK_MAX_MIN),DIGITAL_FEEDBACK_MAX_MAX);
          }
          break;
        case TIP_SPACING:
          GetDlgItemText(hDlg,TIP_SPACING,string,10);
          SD->tip_spacing = min(
						max((float)atof(string),(float)TIP_SPACING_MIN / (float)FTOD_SCALE),
                        (float)TIP_SPACING_MAX / (float)FTOD_SCALE);
          break;
        case WAIT_FOR_CURRENT:
          SD->digital_feedback = IsDlgButtonChecked(hDlg, WAIT_FOR_CURRENT);
          break;
            
		// crash protection controls
        case CRASH_DO_NOTHING:
        case CRASH_STOP:
        case CRASH_AUTO_Z:
        case CRASH_MINMAX_Z:
          SD->crash_protection = LOWORD(wParam);
          break;
        case CRASH_TOLERANCE:
          if(!out_smart)
          {
            GetDlgItemText(hDlg,CRASH_TOLERANCE,string,10);
            SD->z_limit_percent = (float)atof(string);
            SD->z_limit_percent = min(max(SD->z_limit_percent,CRASH_TOLERANCE_MIN), 
                    CRASH_TOLERANCE_MAX);
          }
          break;

		// changes in z offset
        case Z_OFFSET_CRASH:
          z_offset_crash_protect = IsDlgButtonChecked(hDlg,Z_OFFSET_CRASH);
          break;
        case Z_OFFSET_AUTO_BELOW:
          auto_z_below(IN_ZERO);
          repaint_scan_z(hDlg);
          repaint_scan_freq(hDlg);
          break;
        case Z_OFFSET_AUTO_ABOVE:
          auto_z_above(IN_ZERO);
          repaint_scan_z(hDlg);
          repaint_scan_freq(hDlg);
          break;
        case SEND_Z_OFFSET:
          if(z_offset_crash_protect)
          {
            i = scan_z;
            if(scan_z > dac_data[z_offset_ch])
            {
              for(scan_z = dac_data[z_offset_ch];scan_z < (unsigned int)i;scan_z++)
              {
				ramp_serial(z_offset_ch,scan_z,scan_z + 1,DEFAULT_Z_OFFSET_TIME,0);
                dio_in_serial(zi_ch,&z);
                if(z >= IN_MAX - 5 || z < 5) break;
              }
              dac_data[z_offset_ch] = scan_z;
            }
            else
            {
              for(scan_z = dac_data[z_offset_ch];scan_z > (unsigned int)i;scan_z--)
              {
				ramp_serial(z_offset_ch,scan_z,scan_z - 1,DEFAULT_Z_OFFSET_TIME,0);
                dio_in_serial(zi_ch,&z);
                if(z >= IN_MAX - 5 || z < 5) break;
              }
              dac_data[z_offset_ch] = scan_z;
            }
            repaint_scan_z(hDlg);
          }
          else
          {
		    ramp_serial(z_offset_ch,dac_data[z_offset_ch],scan_z,DEFAULT_Z_OFFSET_TIME,0);
            dac_data[z_offset_ch] = scan_z;
            repaint_scan_z(hDlg);
          }
          break;
        case SCAN_Z_BITS_EDIT:
          if(!out_smart)
          {
            GetDlgItemText(hDlg,SCAN_Z_BITS_EDIT,string,5);
            i = atoi(string);
            i = min(max(i,scan_z_min),scan_z_max);
//            if(i != scan_z) enable_z_offset_send(hDlg,1);
            scan_z = (unsigned int)i;
          }
          break;
        case SCAN_Z_VOLTS_EDIT:
          if(!out_smart)
          {
            GetDlgItemText(hDlg,SCAN_Z_VOLTS_EDIT,string,9);
            i = vtod(atof(string),z_offset_range);
            i = min(max(i,scan_z_min),scan_z_max);
//            if(i != scan_z) enable_z_offset_send(hDlg,1);
            scan_z = (unsigned int)i;
          }
          break;
        case SCAN_Z_ANGS_EDIT:
          if(!out_smart)
          {
            GetDlgItemText(hDlg,SCAN_Z_ANGS_EDIT,string,9);
            i = (int)(ZERO + vtod(atof(string) / (double)Z_A_PER_V,z_offset_range));
            i = min(max(i,scan_z_min),scan_z_max);
            scan_z = (unsigned int)i;
          }
          break;
        case SCAN_Z_ZERO:
          scan_z = ZERO;
          repaint_scan_z(hDlg);
          break;

        // changes to bias, current
        case PRESCAN_BIAS_FLIP:
          sample_bias = flipped_bias(sample_bias);
          //bias(sample_bias);
          bias_serial(sample_bias);
          repaint_sample_bias(hDlg);
          break;
        case SAMPLE_BIAS_BITS_EDIT:
          GetDlgItemText(hDlg,SAMPLE_BIAS_BITS_EDIT,string,9);
          i = atoi(string);
          i = min(max(i,sample_bias_min),sample_bias_max);
          sample_bias = (unsigned int)i;
          //bias(sample_bias);
          bias_serial(sample_bias);
          break;
        case SAMPLE_BIAS_VOLTS_EDIT:
          GetDlgItemText(hDlg,SAMPLE_BIAS_VOLTS_EDIT,string,9);
#ifdef DAC16
          i = vtod_bias(atof(string),sample_bias_range);
#else
		  i = vtod(atof(string),sample_bias_range);
#endif
          i = min(max(i,sample_bias_min),sample_bias_max);
          sample_bias = (unsigned int)i;
          //bias(sample_bias);
          bias_serial(sample_bias);
          break;

		//////////////////////////////////
		// Begin of Mod 11, Bias_RampTo //
		//////////////////////////////////
		case Bias_RampTo_Stop:
		  bias_rampto_continue = 0;
		  //MessageBox(hDlg, "Stop activated.", "Good try", MB_OK);
		  break;
        
		case Bias_RampToSpeed_Edit:
		  GetDlgItemText(hDlg, Bias_RampToSpeed_Edit, string, 9);
		  if( atoi( string ) < 1 || atoi( string ) >100 )
		  {
			MessageBox(hDlg, "Valid speed 1~100 ms.", "Try again", MB_OK);
			sprintf(string, "%u", bias_rampto_speed1);
			SetDlgItemText(hDlg, Bias_RampToSpeed_Edit, string);
		  }
		  else
		  {
		    bias_rampto_speed1 = (unsigned) atoi( string );
		  }
		  break;

		case Bias_RampToSpeed_Edit2:
		  GetDlgItemText(hDlg, Bias_RampToSpeed_Edit2, string, 9);
		  if( atoi( string ) < 1 || atoi( string ) >100 )
		  {
			MessageBox(hDlg, "Valid speed 1~100 ms.", "Try again", MB_OK);
			sprintf(string, "%u", bias_rampto_speed2);
			SetDlgItemText(hDlg, Bias_RampToSpeed_Edit2, string);
		  }
		  else
		  {
		    bias_rampto_speed2 = (unsigned) atoi( string );
		  }
		  break;
		case Bias_RampToSpeed_Edit3:
		  GetDlgItemText(hDlg, Bias_RampToSpeed_Edit3, string, 9);
		  if( atoi( string ) < 1 || atoi( string ) >100 )
		  {
			MessageBox(hDlg, "Valid speed 1~100 ms.", "Try again", MB_OK);
			sprintf(string, "%u", bias_rampto_speed3);
			SetDlgItemText(hDlg, Bias_RampToSpeed_Edit3, string);
		  }
		  else
		  {
		    bias_rampto_speed3 = (unsigned) atoi( string );
		  }
		  break;

		case Bias_RampToSpeed_Edit4:
		  GetDlgItemText(hDlg, Bias_RampToSpeed_Edit4, string, 9);
		  if( atoi( string ) < 1 || atoi( string ) >100 )
		  {
			MessageBox(hDlg, "Valid speed 1~100 ms.", "Try again", MB_OK);
			sprintf(string, "%u", bias_rampto_speed4);
			SetDlgItemText(hDlg, Bias_RampToSpeed_Edit4, string);
		  }
		  else
		  {
		    bias_rampto_speed4 = (unsigned) atoi( string );
		  }
		  break;

        case Bias_RampTo_Edit:
            GetDlgItemText(hDlg, Bias_RampTo_Edit, string, 9);
		    bias_target1 = atof( string );
		  break;
		
		case Bias_RampTo_Edit2:
            GetDlgItemText(hDlg, Bias_RampTo_Edit2, string, 9);
		    bias_target2 = atof( string );
		  break;

        case Bias_RampTo_Edit3:
            GetDlgItemText(hDlg, Bias_RampTo_Edit3, string, 9);
		    bias_target3 = atof( string );
		  break;

        case Bias_RampTo_Edit4:
            GetDlgItemText(hDlg, Bias_RampTo_Edit4, string, 9);
		    bias_target4 = atof( string );
		  break;
		
		case Bias_RampTo_Btn:
		  GetDlgItemText(hDlg, Bias_RampTo_Edit, string, 9);
		  bias_target = bias_target1 = atof( string );
		  repaint_bias_target(hDlg);
		  // Note that the next line assumes to be 12-bit bias resolution
		  if( (sample_bias_range==1) && ((bias_target<-5.0)||(bias_target>5.0))
			||(sample_bias_range==2) && ((bias_target<-10.0)||(bias_target>10.0)) )
		  {
			MessageBox(hDlg, "Bias ramp to value out of range. Abort.", "Warning!", MB_OK);
			break;
		  }
		  if( bias_target * dtov(sample_bias,sample_bias_range) <= 0.0 )
		  {
			MessageBox(hDlg, "Attempt to ramp bias across ZERO. Abort.", "Warning!", MB_OK);
			break;
		  }
#ifdef DAC16
          bias_target_int = vtod_bias(bias_target,sample_bias_range);
#else
		  bias_target_int = vtod(bias_target,sample_bias_range);
#endif
		  bias_target_int = min(max(bias_target_int,sample_bias_min),sample_bias_max);
		  bias_rampto_continue = 1;
		  if( sample_bias < bias_target_int )
		  {
			do
			{
			  sample_bias++;
			  bias_serial( sample_bias );

			  //repaint_sample_bias( hDlg );
			  sprintf( string, "%u", sample_bias );
			  SetDlgItemText( hDlg, SAMPLE_BIAS_BITS_EDIT, string );
			  UpdateWindow( GetDlgItem(hDlg, SAMPLE_BIAS_BITS_EDIT) );
			  //SetScrollPos(GetDlgItem(hDlg,SAMPLE_BIAS_SCROLL),SB_CTL,sample_bias,TRUE);
#ifdef DAC16
			  sprintf(string,"%.5f",dtov_bias(sample_bias,get_range(sample_bias_ch)));
#else
			  sprintf(string,"%.5f",dtov(sample_bias,get_range(sample_bias_ch)));
#endif
			  SetDlgItemText(hDlg,SAMPLE_BIAS_VOLTS_EDIT,string);
			  UpdateWindow( GetDlgItem(hDlg, SAMPLE_BIAS_VOLTS_EDIT) );

			  sleep_ms( bias_rampto_speed1 );
			  if( PeekMessage( &stop_message, GetDlgItem(hDlg, Bias_RampTo_Stop),
					WM_LBUTTONUP, WM_LBUTTONUP, PM_NOREMOVE ) )
				bias_rampto_continue = 0;
			}while( (sample_bias < bias_target_int) && bias_rampto_continue );
		  }
		  else if( sample_bias > bias_target_int )
		  {
			do
			{
			  sample_bias--;
			  bias_serial( sample_bias );

			  //repaint_sample_bias( hDlg );
			  sprintf( string, "%u", sample_bias );
			  SetDlgItemText( hDlg, SAMPLE_BIAS_BITS_EDIT, string );
			  UpdateWindow( GetDlgItem(hDlg, SAMPLE_BIAS_BITS_EDIT) );
			  //SetScrollPos(GetDlgItem(hDlg,SAMPLE_BIAS_SCROLL),SB_CTL,sample_bias,TRUE);
#ifdef DAC16
			  sprintf(string,"%.5f",dtov_bias(sample_bias,get_range(sample_bias_ch)));
#else
			  sprintf(string,"%.5f",dtov(sample_bias,get_range(sample_bias_ch)));
#endif
			  SetDlgItemText(hDlg,SAMPLE_BIAS_VOLTS_EDIT,string);
			  UpdateWindow( GetDlgItem(hDlg, SAMPLE_BIAS_VOLTS_EDIT) );

			  sleep_ms( bias_rampto_speed1 );
			  if( PeekMessage( &stop_message, GetDlgItem(hDlg, Bias_RampTo_Stop),
					WM_LBUTTONUP, WM_LBUTTONUP, PM_NOREMOVE ) )
				bias_rampto_continue = 0;
			}while( (sample_bias > bias_target_int) && bias_rampto_continue );
		  }
		  else;	// do nothing when equal
		  bias_rampto_continue = 0;
		  repaint_sample_bias( hDlg );
		  break;

//btn 2
		  case Bias_RampTo_Btn2:
		  GetDlgItemText(hDlg, Bias_RampTo_Edit2, string, 9);
		  bias_target = bias_target2 = atof( string );
		  repaint_bias_target(hDlg);
		  // Note that the next line assumes to be 12-bit bias resolution
		  if( (sample_bias_range==1) && ((bias_target<-5.0)||(bias_target>5.0))
			||(sample_bias_range==2) && ((bias_target<-10.0)||(bias_target>10.0)) )
		  {
			MessageBox(hDlg, "Bias ramp to value out of range. Abort.", "Warning!", MB_OK);
			break;
		  }
		  if( bias_target * dtov(sample_bias,sample_bias_range) <= 0.0 )
		  {
			MessageBox(hDlg, "Attempt to ramp bias across ZERO. Abort.", "Warning!", MB_OK);
			break;
		  }
#ifdef DAC16
          bias_target_int = vtod_bias(bias_target,sample_bias_range);
#else
		  bias_target_int = vtod(bias_target,sample_bias_range);
#endif
		  bias_target_int = min(max(bias_target_int,sample_bias_min),sample_bias_max);
		  bias_rampto_continue = 1;
		  if( sample_bias < bias_target_int )
		  {
			do
			{
			  sample_bias++;
			  bias_serial( sample_bias );

			  //repaint_sample_bias( hDlg );
			  sprintf( string, "%u", sample_bias );
			  SetDlgItemText( hDlg, SAMPLE_BIAS_BITS_EDIT, string );
			  UpdateWindow( GetDlgItem(hDlg, SAMPLE_BIAS_BITS_EDIT) );
			  //SetScrollPos(GetDlgItem(hDlg,SAMPLE_BIAS_SCROLL),SB_CTL,sample_bias,TRUE);
#ifdef DAC16
			  sprintf(string,"%.5f",dtov_bias(sample_bias,get_range(sample_bias_ch)));
#else
			  sprintf(string,"%.5f",dtov(sample_bias,get_range(sample_bias_ch)));
#endif
			  SetDlgItemText(hDlg,SAMPLE_BIAS_VOLTS_EDIT,string);
			  UpdateWindow( GetDlgItem(hDlg, SAMPLE_BIAS_VOLTS_EDIT) );

			  sleep_ms( bias_rampto_speed2 );
			  if( PeekMessage( &stop_message, GetDlgItem(hDlg, Bias_RampTo_Stop),
					WM_LBUTTONUP, WM_LBUTTONUP, PM_NOREMOVE ) )
				bias_rampto_continue = 0;
			}while( (sample_bias < bias_target_int) && bias_rampto_continue );
		  }
		  else if( sample_bias > bias_target_int )
		  {
			do
			{
			  sample_bias--;
			  bias_serial( sample_bias );

			  //repaint_sample_bias( hDlg );
			  sprintf( string, "%u", sample_bias );
			  SetDlgItemText( hDlg, SAMPLE_BIAS_BITS_EDIT, string );
			  UpdateWindow( GetDlgItem(hDlg, SAMPLE_BIAS_BITS_EDIT) );
			  //SetScrollPos(GetDlgItem(hDlg,SAMPLE_BIAS_SCROLL),SB_CTL,sample_bias,TRUE);
#ifdef DAC16
			  sprintf(string,"%.5f",dtov_bias(sample_bias,get_range(sample_bias_ch)));
#else
			  sprintf(string,"%.5f",dtov(sample_bias,get_range(sample_bias_ch)));
#endif
			  SetDlgItemText(hDlg,SAMPLE_BIAS_VOLTS_EDIT,string);
			  UpdateWindow( GetDlgItem(hDlg, SAMPLE_BIAS_VOLTS_EDIT) );

			  sleep_ms( bias_rampto_speed2 );
			  if( PeekMessage( &stop_message, GetDlgItem(hDlg, Bias_RampTo_Stop),
					WM_LBUTTONUP, WM_LBUTTONUP, PM_NOREMOVE ) )
				bias_rampto_continue = 0;
			}while( (sample_bias > bias_target_int) && bias_rampto_continue );
		  }
		  else;	// do nothing when equal
		  bias_rampto_continue = 0;
		  repaint_sample_bias( hDlg );
		  break;

//btn 3
		  case Bias_RampTo_Btn3:
		  GetDlgItemText(hDlg, Bias_RampTo_Edit3, string, 9);
		  bias_target = bias_target3 = atof( string );
		  repaint_bias_target(hDlg);
		  // Note that the next line assumes to be 12-bit bias resolution
		  if( (sample_bias_range==1) && ((bias_target<-5.0)||(bias_target>5.0))
			||(sample_bias_range==2) && ((bias_target<-10.0)||(bias_target>10.0)) )
		  {
			MessageBox(hDlg, "Bias ramp to value out of range. Abort.", "Warning!", MB_OK);
			break;
		  }
		  if( bias_target * dtov(sample_bias,sample_bias_range) <= 0.0 )
		  {
			MessageBox(hDlg, "Attempt to ramp bias across ZERO. Abort.", "Warning!", MB_OK);
			break;
		  }
#ifdef DAC16
          bias_target_int = vtod_bias(bias_target,sample_bias_range);
#else
		  bias_target_int = vtod(bias_target,sample_bias_range);
#endif
		  bias_target_int = min(max(bias_target_int,sample_bias_min),sample_bias_max);
		  bias_rampto_continue = 1;
		  if( sample_bias < bias_target_int )
		  {
			do
			{
			  sample_bias++;
			  bias_serial( sample_bias );

			  //repaint_sample_bias( hDlg );
			  sprintf( string, "%u", sample_bias );
			  SetDlgItemText( hDlg, SAMPLE_BIAS_BITS_EDIT, string );
			  UpdateWindow( GetDlgItem(hDlg, SAMPLE_BIAS_BITS_EDIT) );
			  //SetScrollPos(GetDlgItem(hDlg,SAMPLE_BIAS_SCROLL),SB_CTL,sample_bias,TRUE);
#ifdef DAC16
			  sprintf(string,"%.5f",dtov_bias(sample_bias,get_range(sample_bias_ch)));
#else
			  sprintf(string,"%.5f",dtov(sample_bias,get_range(sample_bias_ch)));
#endif
			  SetDlgItemText(hDlg,SAMPLE_BIAS_VOLTS_EDIT,string);
			  UpdateWindow( GetDlgItem(hDlg, SAMPLE_BIAS_VOLTS_EDIT) );

			  sleep_ms( bias_rampto_speed3 );
			  if( PeekMessage( &stop_message, GetDlgItem(hDlg, Bias_RampTo_Stop),
					WM_LBUTTONUP, WM_LBUTTONUP, PM_NOREMOVE ) )
				bias_rampto_continue = 0;
			}while( (sample_bias < bias_target_int) && bias_rampto_continue );
		  }
		  else if( sample_bias > bias_target_int )
		  {
			do
			{
			  sample_bias--;
			  bias_serial( sample_bias );

			  //repaint_sample_bias( hDlg );
			  sprintf( string, "%u", sample_bias );
			  SetDlgItemText( hDlg, SAMPLE_BIAS_BITS_EDIT, string );
			  UpdateWindow( GetDlgItem(hDlg, SAMPLE_BIAS_BITS_EDIT) );
			  //SetScrollPos(GetDlgItem(hDlg,SAMPLE_BIAS_SCROLL),SB_CTL,sample_bias,TRUE);
#ifdef DAC16
			  sprintf(string,"%.5f",dtov_bias(sample_bias,get_range(sample_bias_ch)));
#else
			  sprintf(string,"%.5f",dtov(sample_bias,get_range(sample_bias_ch)));
#endif
			  SetDlgItemText(hDlg,SAMPLE_BIAS_VOLTS_EDIT,string);
			  UpdateWindow( GetDlgItem(hDlg, SAMPLE_BIAS_VOLTS_EDIT) );

			  sleep_ms( bias_rampto_speed3 );
			  if( PeekMessage( &stop_message, GetDlgItem(hDlg, Bias_RampTo_Stop),
					WM_LBUTTONUP, WM_LBUTTONUP, PM_NOREMOVE ) )
				bias_rampto_continue = 0;
			}while( (sample_bias > bias_target_int) && bias_rampto_continue );
		  }
		  else;	// do nothing when equal
		  bias_rampto_continue = 0;
		  repaint_sample_bias( hDlg );
		  break;

//btn 4
		  case Bias_RampTo_Btn4:
		  GetDlgItemText(hDlg, Bias_RampTo_Edit4, string, 9);
		  bias_target = bias_target4 = atof( string );
		  repaint_bias_target(hDlg);
		  // Note that the next line assumes to be 12-bit bias resolution
		  if( (sample_bias_range==1) && ((bias_target<-5.0)||(bias_target>5.0))
			||(sample_bias_range==2) && ((bias_target<-10.0)||(bias_target>10.0)) )
		  {
			MessageBox(hDlg, "Bias ramp to value out of range. Abort.", "Warning!", MB_OK);
			break;
		  }
		  if( bias_target * dtov(sample_bias,sample_bias_range) <= 0.0 )
		  {
			MessageBox(hDlg, "Attempt to ramp bias across ZERO. Abort.", "Warning!", MB_OK);
			break;
		  }
#ifdef DAC16
          bias_target_int = vtod_bias(bias_target,sample_bias_range);
#else
		  bias_target_int = vtod(bias_target,sample_bias_range);
#endif
		  bias_target_int = min(max(bias_target_int,sample_bias_min),sample_bias_max);
		  bias_rampto_continue = 1;
		  if( sample_bias < bias_target_int )
		  {
			do
			{
			  sample_bias++;
			  bias_serial( sample_bias );

			  //repaint_sample_bias( hDlg );
			  sprintf( string, "%u", sample_bias );
			  SetDlgItemText( hDlg, SAMPLE_BIAS_BITS_EDIT, string );
			  UpdateWindow( GetDlgItem(hDlg, SAMPLE_BIAS_BITS_EDIT) );
			  //SetScrollPos(GetDlgItem(hDlg,SAMPLE_BIAS_SCROLL),SB_CTL,sample_bias,TRUE);
#ifdef DAC16
			  sprintf(string,"%.5f",dtov_bias(sample_bias,get_range(sample_bias_ch)));
#else
			  sprintf(string,"%.5f",dtov(sample_bias,get_range(sample_bias_ch)));
#endif
			  SetDlgItemText(hDlg,SAMPLE_BIAS_VOLTS_EDIT,string);
			  UpdateWindow( GetDlgItem(hDlg, SAMPLE_BIAS_VOLTS_EDIT) );

			  sleep_ms( bias_rampto_speed4 );
			  if( PeekMessage( &stop_message, GetDlgItem(hDlg, Bias_RampTo_Stop),
					WM_LBUTTONUP, WM_LBUTTONUP, PM_NOREMOVE ) )
				bias_rampto_continue = 0;
			}while( (sample_bias < bias_target_int) && bias_rampto_continue );
		  }
		  else if( sample_bias > bias_target_int )
		  {
			do
			{
			  sample_bias--;
			  bias_serial( sample_bias );

			  //repaint_sample_bias( hDlg );
			  sprintf( string, "%u", sample_bias );
			  SetDlgItemText( hDlg, SAMPLE_BIAS_BITS_EDIT, string );
			  UpdateWindow( GetDlgItem(hDlg, SAMPLE_BIAS_BITS_EDIT) );
			  //SetScrollPos(GetDlgItem(hDlg,SAMPLE_BIAS_SCROLL),SB_CTL,sample_bias,TRUE);
#ifdef DAC16
			  sprintf(string,"%.5f",dtov_bias(sample_bias,get_range(sample_bias_ch)));
#else
			  sprintf(string,"%.5f",dtov(sample_bias,get_range(sample_bias_ch)));
#endif
			  SetDlgItemText(hDlg,SAMPLE_BIAS_VOLTS_EDIT,string);
			  UpdateWindow( GetDlgItem(hDlg, SAMPLE_BIAS_VOLTS_EDIT) );

			  sleep_ms( bias_rampto_speed4 );
			  if( PeekMessage( &stop_message, GetDlgItem(hDlg, Bias_RampTo_Stop),
					WM_LBUTTONUP, WM_LBUTTONUP, PM_NOREMOVE ) )
				bias_rampto_continue = 0;
			}while( (sample_bias > bias_target_int) && bias_rampto_continue );
		  }
		  else;	// do nothing when equal
		  bias_rampto_continue = 0;
		  repaint_sample_bias( hDlg );
		  break;
		////////////////////////////////
		// End of Mod 11, Bias_RampTo //
		////////////////////////////////

		/////////////////////////////////////
		// Begin of Mod 11, Current_RampTo //
		/////////////////////////////////////
		case Current_RampTo_Stop:
		  current_rampto_continue = 0;
		  //MessageBox(hDlg, "Stop activated.", "Good try", MB_OK);
		  break;

		case Current_RampToSpeed_Edit:
		  GetDlgItemText(hDlg, Current_RampToSpeed_Edit, string, 9);
		  if( atoi( string ) < 1 || atoi( string ) >100 )
		  {
			MessageBox(hDlg, "Valid speed 1~100 ms.", "Try again", MB_OK);
			sprintf(string, "%u", current_rampto_speed1);
			SetDlgItemText(hDlg, Current_RampToSpeed_Edit, string);
		  }
		  else
		  {
		    current_rampto_speed1 = (unsigned) atoi( string );
		  }
		  break;


		case Current_RampToSpeed_Edit2:
		  GetDlgItemText(hDlg, Current_RampToSpeed_Edit2, string, 9);
		  if( atoi( string ) < 1 || atoi( string ) >100 )
		  {
			MessageBox(hDlg, "Valid speed 1~100 ms.", "Try again", MB_OK);
			sprintf(string, "%u", current_rampto_speed2);
			SetDlgItemText(hDlg, Current_RampToSpeed_Edit, string);
		  }
		  else
		  {
		    current_rampto_speed2 = (unsigned) atoi( string );
		  }
		  break;
		
		case Current_RampToSpeed_Edit3:
		  GetDlgItemText(hDlg, Current_RampToSpeed_Edit3, string, 9);
		  if( atoi( string ) < 1 || atoi( string ) >100 )
		  {
			MessageBox(hDlg, "Valid speed 1~100 ms.", "Try again", MB_OK);
			sprintf(string, "%u", current_rampto_speed3);
			SetDlgItemText(hDlg, Current_RampToSpeed_Edit3, string);
		  }
		  else
		  {
		    current_rampto_speed3 = (unsigned) atoi( string );
		  }
		  break;

		case Current_RampToSpeed_Edit4:
		  GetDlgItemText(hDlg, Current_RampToSpeed_Edit4, string, 9);
		  if( atoi( string ) < 1 || atoi( string ) >100 )
		  {
			MessageBox(hDlg, "Valid speed 1~100 ms.", "Try again", MB_OK);
			sprintf(string, "%u", current_rampto_speed4);
			SetDlgItemText(hDlg, Current_RampToSpeed_Edit4, string);
		  }
		  else
		  {
		    current_rampto_speed4 = (unsigned) atoi( string );
		  }
		  break;


        case Current_RampTo_Edit:
		  GetDlgItemText(hDlg, Current_RampTo_Edit, string, 9);
          current_target1 = atof( string );
		  break;

        case Current_RampTo_Edit2:
		  GetDlgItemText(hDlg, Current_RampTo_Edit2, string, 9);
          current_target2 = atof( string );
		  break;

        case Current_RampTo_Edit3:
		  GetDlgItemText(hDlg, Current_RampTo_Edit3, string, 9);
          current_target3 = atof( string );
		  break;
        
		case Current_RampTo_Edit4:
		  GetDlgItemText(hDlg, Current_RampTo_Edit4, string, 9);
          current_target4 = atof( string );
		  break;

		case Current_RampTo_Btn:
		  GetDlgItemText(hDlg, Current_RampTo_Edit, string, 9);
          current_target1 = atof( string );
		  current_target = current_target1;
		  repaint_current_target(hDlg);
		  if( ( (tip_gain==8) && ((current_target<1.0)||(current_target>100)) ) 
			||( (tip_gain==9) && ((current_target<0.1)||(current_target>10.0)) ) 
			||( (tip_gain==10) && ((current_target<0.01)||(current_target>1.0)) ) )
		  {
			MessageBox( hDlg, "Current ramp to value out of range. Abort.", 
			  "Warning", MB_OK );
			break;
		  }
		  current_target_int = calc_i_setpoint(current_target, i_set_range, tip_gain );
		  current_rampto_continue = 1;
		  
		  if( i_setpoint < current_target_int )	  // ramp up
		  {
			do
			{
			  i_setpoint++;
			  i_set = calc_i_set( i_setpoint, i_set_range, tip_gain );
			  tip_current_serial(i_setpoint);

			  sprintf( string, "%u", i_setpoint );
			  SetDlgItemText( hDlg, TIP_CURRENT_BITS_EDIT, string );
			  UpdateWindow( GetDlgItem(hDlg, TIP_CURRENT_BITS_EDIT) );

			  out_smart = 1;
			  sprintf( string, "%.5f", i_set );
			  SetDlgItemText( hDlg, TIP_CURRENT_AMPS_EDIT, string );
			  UpdateWindow( GetDlgItem(hDlg, TIP_CURRENT_AMPS_EDIT) );
			  out_smart = 0;

			  sleep_ms( current_rampto_speed1 );
			  if( PeekMessage( &stop_message, GetDlgItem(hDlg, Current_RampTo_Stop),
					WM_LBUTTONUP, WM_LBUTTONUP, PM_NOREMOVE ) )
				current_rampto_continue = 0;
			}while( (i_setpoint < current_target_int) && current_rampto_continue );
		  }
		  else if( i_setpoint > current_target_int )	  // ramp down
		  {
			do
			{
			  //sprintf( string, "i_set = %f, i_setpoint = %u while i_target_int = %u", i_set, i_setpoint, current_target_int );
			  //if( MessageBox( hDlg, string, "Status 0", MB_OKCANCEL ) == IDCANCEL )
				//current_rampto_continue = 0;

			  i_setpoint--;
			  i_set = calc_i_set( i_setpoint, i_set_range, tip_gain );
			  tip_current_serial( i_setpoint );

			  //sprintf( string, "i_set = %f, i_setpoint = %u while i_target_int = %u", i_set, i_setpoint, current_target_int );
			  //if( MessageBox( hDlg, string, "Status 1", MB_OKCANCEL ) == IDCANCEL )
				//current_rampto_continue = 0;

			  sprintf( string, "%u", i_setpoint );
			  SetDlgItemText( hDlg, TIP_CURRENT_BITS_EDIT, string );
			  UpdateWindow( GetDlgItem(hDlg, TIP_CURRENT_BITS_EDIT) );

			  out_smart = 1;
			  sprintf( string, "%.5f", i_set );
			  SetDlgItemText( hDlg, TIP_CURRENT_AMPS_EDIT, string );
			  UpdateWindow( GetDlgItem(hDlg, TIP_CURRENT_AMPS_EDIT) );
			  out_smart = 0;
			  
			  //sprintf( string, "i_set = %f, i_setpoint = %u while i_target_int = %u", i_set, i_setpoint, current_target_int );
			  //if( MessageBox( hDlg, string, "Status2", MB_OKCANCEL ) == IDCANCEL )
				//current_rampto_continue = 0;

			  sleep_ms( current_rampto_speed1 );

			  if( PeekMessage( &stop_message, GetDlgItem(hDlg, Current_RampTo_Stop),
					WM_LBUTTONUP, WM_LBUTTONUP, PM_NOREMOVE ) )
				current_rampto_continue = 0;

			  //sprintf( string, "i_set = %f, i_setpoint = %u while i_target_int = %u", i_set, i_setpoint, current_target_int );
			  //if( MessageBox( hDlg, string, "Status 3", MB_OKCANCEL ) == IDCANCEL )
				//current_rampto_continue = 0;
			}while( (i_setpoint > current_target_int) && current_rampto_continue );
		  }
		  else;	// do nothing when equal
		  current_rampto_continue = 0;
		  repaint_tip_current( hDlg );
		  break;

		case Current_RampTo_Btn2:
		  GetDlgItemText(hDlg, Current_RampTo_Edit2, string, 9);
          current_target2 = atof( string );
		  current_target = current_target2;
		  repaint_current_target(hDlg);
		  if( ( (tip_gain==8) && ((current_target<1.0)||(current_target>100)) ) 
			||( (tip_gain==9) && ((current_target<0.1)||(current_target>10.0)) ) 
			||( (tip_gain==10) && ((current_target<0.01)||(current_target>1.0)) ) )
		  {
			MessageBox( hDlg, "Current ramp to value out of range. Abort.", 
			  "Warning", MB_OK );
			break;
		  }
		  current_target_int = calc_i_setpoint(current_target, i_set_range, tip_gain );
		  current_rampto_continue = 1;
		  if( i_setpoint < current_target_int )	  // ramp up
		  {
			do
			{
			  i_setpoint++;
			  i_set = calc_i_set( i_setpoint, i_set_range, tip_gain );
			  tip_current_serial(i_setpoint);

			  sprintf( string, "%u", i_setpoint );
			  SetDlgItemText( hDlg, TIP_CURRENT_BITS_EDIT, string );
			  UpdateWindow( GetDlgItem(hDlg, TIP_CURRENT_BITS_EDIT) );

			  out_smart = 1;
			  sprintf( string, "%.5f", i_set );
			  SetDlgItemText( hDlg, TIP_CURRENT_AMPS_EDIT, string );
			  UpdateWindow( GetDlgItem(hDlg, TIP_CURRENT_AMPS_EDIT) );
			  out_smart = 0;

			  sleep_ms( current_rampto_speed2 );
			  if( PeekMessage( &stop_message, GetDlgItem(hDlg, Current_RampTo_Stop),
					WM_LBUTTONUP, WM_LBUTTONUP, PM_NOREMOVE ) )
				current_rampto_continue = 0;
			}while( (i_setpoint < current_target_int) && current_rampto_continue );
		  }
		  else if( i_setpoint > current_target_int )	  // ramp down
		  {
			do
			{
			  //sprintf( string, "i_set = %f, i_setpoint = %u while i_target_int = %u", i_set, i_setpoint, current_target_int );
			  //if( MessageBox( hDlg, string, "Status 0", MB_OKCANCEL ) == IDCANCEL )
				//current_rampto_continue = 0;

			  i_setpoint--;
			  i_set = calc_i_set( i_setpoint, i_set_range, tip_gain );
			  tip_current_serial( i_setpoint );

			  //sprintf( string, "i_set = %f, i_setpoint = %u while i_target_int = %u", i_set, i_setpoint, current_target_int );
			  //if( MessageBox( hDlg, string, "Status 1", MB_OKCANCEL ) == IDCANCEL )
				//current_rampto_continue = 0;

			  sprintf( string, "%u", i_setpoint );
			  SetDlgItemText( hDlg, TIP_CURRENT_BITS_EDIT, string );
			  UpdateWindow( GetDlgItem(hDlg, TIP_CURRENT_BITS_EDIT) );

			  out_smart = 1;
			  sprintf( string, "%.5f", i_set );
			  SetDlgItemText( hDlg, TIP_CURRENT_AMPS_EDIT, string );
			  UpdateWindow( GetDlgItem(hDlg, TIP_CURRENT_AMPS_EDIT) );
			  out_smart = 0;
			  
			  //sprintf( string, "i_set = %f, i_setpoint = %u while i_target_int = %u", i_set, i_setpoint, current_target_int );
			  //if( MessageBox( hDlg, string, "Status2", MB_OKCANCEL ) == IDCANCEL )
				//current_rampto_continue = 0;

			  sleep_ms( current_rampto_speed2 );

			  if( PeekMessage( &stop_message, GetDlgItem(hDlg, Current_RampTo_Stop),
					WM_LBUTTONUP, WM_LBUTTONUP, PM_NOREMOVE ) )
				current_rampto_continue = 0;

			  //sprintf( string, "i_set = %f, i_setpoint = %u while i_target_int = %u", i_set, i_setpoint, current_target_int );
			  //if( MessageBox( hDlg, string, "Status 3", MB_OKCANCEL ) == IDCANCEL )
				//current_rampto_continue = 0;
			}while( (i_setpoint > current_target_int) && current_rampto_continue );
		  }
		  else;	// do nothing when equal
		  current_rampto_continue = 0;
		  repaint_tip_current( hDlg );
		  break;

		  case Current_RampTo_Btn3:
		  GetDlgItemText(hDlg, Current_RampTo_Edit3, string, 9);
          current_target3 = atof( string );
		  current_target = current_target3;
		  repaint_current_target(hDlg);
		  if( ( (tip_gain==8) && ((current_target<1.0)||(current_target>100)) ) 
			||( (tip_gain==9) && ((current_target<0.1)||(current_target>10.0)) ) 
			||( (tip_gain==10) && ((current_target<0.01)||(current_target>1.0)) ) )
		  {
			MessageBox( hDlg, "Current ramp to value out of range. Abort.", 
			  "Warning", MB_OK );
			break;
		  }
		  current_target_int = calc_i_setpoint(current_target, i_set_range, tip_gain );
		  current_rampto_continue = 1;
		  if( i_setpoint < current_target_int )	  // ramp up
		  {
			do
			{
			  i_setpoint++;
			  i_set = calc_i_set( i_setpoint, i_set_range, tip_gain );
			  tip_current_serial(i_setpoint);

			  sprintf( string, "%u", i_setpoint );
			  SetDlgItemText( hDlg, TIP_CURRENT_BITS_EDIT, string );
			  UpdateWindow( GetDlgItem(hDlg, TIP_CURRENT_BITS_EDIT) );

			  out_smart = 1;
			  sprintf( string, "%.5f", i_set );
			  SetDlgItemText( hDlg, TIP_CURRENT_AMPS_EDIT, string );
			  UpdateWindow( GetDlgItem(hDlg, TIP_CURRENT_AMPS_EDIT) );
			  out_smart = 0;

			  sleep_ms( current_rampto_speed3 );
			  if( PeekMessage( &stop_message, GetDlgItem(hDlg, Current_RampTo_Stop),
					WM_LBUTTONUP, WM_LBUTTONUP, PM_NOREMOVE ) )
				current_rampto_continue = 0;
			}while( (i_setpoint < current_target_int) && current_rampto_continue );
		  }
		  else if( i_setpoint > current_target_int )	  // ramp down
		  {
			do
			{
			  //sprintf( string, "i_set = %f, i_setpoint = %u while i_target_int = %u", i_set, i_setpoint, current_target_int );
			  //if( MessageBox( hDlg, string, "Status 0", MB_OKCANCEL ) == IDCANCEL )
				//current_rampto_continue = 0;

			  i_setpoint--;
			  i_set = calc_i_set( i_setpoint, i_set_range, tip_gain );
			  tip_current_serial( i_setpoint );

			  //sprintf( string, "i_set = %f, i_setpoint = %u while i_target_int = %u", i_set, i_setpoint, current_target_int );
			  //if( MessageBox( hDlg, string, "Status 1", MB_OKCANCEL ) == IDCANCEL )
				//current_rampto_continue = 0;

			  sprintf( string, "%u", i_setpoint );
			  SetDlgItemText( hDlg, TIP_CURRENT_BITS_EDIT, string );
			  UpdateWindow( GetDlgItem(hDlg, TIP_CURRENT_BITS_EDIT) );

			  out_smart = 1;
			  sprintf( string, "%.5f", i_set );
			  SetDlgItemText( hDlg, TIP_CURRENT_AMPS_EDIT, string );
			  UpdateWindow( GetDlgItem(hDlg, TIP_CURRENT_AMPS_EDIT) );
			  out_smart = 0;
			  
			  //sprintf( string, "i_set = %f, i_setpoint = %u while i_target_int = %u", i_set, i_setpoint, current_target_int );
			  //if( MessageBox( hDlg, string, "Status2", MB_OKCANCEL ) == IDCANCEL )
				//current_rampto_continue = 0;

			  sleep_ms( current_rampto_speed3 );

			  if( PeekMessage( &stop_message, GetDlgItem(hDlg, Current_RampTo_Stop),
					WM_LBUTTONUP, WM_LBUTTONUP, PM_NOREMOVE ) )
				current_rampto_continue = 0;

			  //sprintf( string, "i_set = %f, i_setpoint = %u while i_target_int = %u", i_set, i_setpoint, current_target_int );
			  //if( MessageBox( hDlg, string, "Status 3", MB_OKCANCEL ) == IDCANCEL )
				//current_rampto_continue = 0;
			}while( (i_setpoint > current_target_int) && current_rampto_continue );
		  }
		  else;	// do nothing when equal
		  current_rampto_continue = 0;
		  repaint_tip_current( hDlg );
		  break;
           
		case Current_RampTo_Btn4:  //Mod 11 Bnt4
		  GetDlgItemText(hDlg, Current_RampTo_Edit4, string, 9);
          current_target4 = atof( string );
		  current_target = current_target4;
		  repaint_current_target(hDlg);
		  if( ( (tip_gain==8) && ((current_target<1.0)||(current_target>100)) ) 
			||( (tip_gain==9) && ((current_target<0.1)||(current_target>10.0)) ) 
			||( (tip_gain==10) && ((current_target<0.01)||(current_target>1.0)) ) )
		  {
			MessageBox( hDlg, "Current ramp to value out of range. Abort.", 
			  "Warning", MB_OK );
			break;
		  }
		  current_target_int = calc_i_setpoint(current_target, i_set_range, tip_gain );
		  current_rampto_continue = 1;
		  if( i_setpoint < current_target_int )	  // ramp up
		  {
			do
			{
			  i_setpoint++;
			  i_set = calc_i_set( i_setpoint, i_set_range, tip_gain );
			  tip_current_serial(i_setpoint);

			  sprintf( string, "%u", i_setpoint );
			  SetDlgItemText( hDlg, TIP_CURRENT_BITS_EDIT, string );
			  UpdateWindow( GetDlgItem(hDlg, TIP_CURRENT_BITS_EDIT) );

			  out_smart = 1;
			  sprintf( string, "%.5f", i_set );
			  SetDlgItemText( hDlg, TIP_CURRENT_AMPS_EDIT, string );
			  UpdateWindow( GetDlgItem(hDlg, TIP_CURRENT_AMPS_EDIT) );
			  out_smart = 0;

			  sleep_ms( current_rampto_speed4 );
			  if( PeekMessage( &stop_message, GetDlgItem(hDlg, Current_RampTo_Stop),
					WM_LBUTTONUP, WM_LBUTTONUP, PM_NOREMOVE ) )
				current_rampto_continue = 0;
			}while( (i_setpoint < current_target_int) && current_rampto_continue );
		  }
		  else if( i_setpoint > current_target_int )	  // ramp down
		  {
			do
			{
			  //sprintf( string, "i_set = %f, i_setpoint = %u while i_target_int = %u", i_set, i_setpoint, current_target_int );
			  //if( MessageBox( hDlg, string, "Status 0", MB_OKCANCEL ) == IDCANCEL )
				//current_rampto_continue = 0;

			  i_setpoint--;
			  i_set = calc_i_set( i_setpoint, i_set_range, tip_gain );
			  tip_current_serial( i_setpoint );

			  //sprintf( string, "i_set = %f, i_setpoint = %u while i_target_int = %u", i_set, i_setpoint, current_target_int );
			  //if( MessageBox( hDlg, string, "Status 1", MB_OKCANCEL ) == IDCANCEL )
				//current_rampto_continue = 0;

			  sprintf( string, "%u", i_setpoint );
			  SetDlgItemText( hDlg, TIP_CURRENT_BITS_EDIT, string );
			  UpdateWindow( GetDlgItem(hDlg, TIP_CURRENT_BITS_EDIT) );

			  out_smart = 1;
			  sprintf( string, "%.5f", i_set );
			  SetDlgItemText( hDlg, TIP_CURRENT_AMPS_EDIT, string );
			  UpdateWindow( GetDlgItem(hDlg, TIP_CURRENT_AMPS_EDIT) );
			  out_smart = 0;
			  
			  //sprintf( string, "i_set = %f, i_setpoint = %u while i_target_int = %u", i_set, i_setpoint, current_target_int );
			  //if( MessageBox( hDlg, string, "Status2", MB_OKCANCEL ) == IDCANCEL )
				//current_rampto_continue = 0;

			  sleep_ms( current_rampto_speed4 );

			  if( PeekMessage( &stop_message, GetDlgItem(hDlg, Current_RampTo_Stop),
					WM_LBUTTONUP, WM_LBUTTONUP, PM_NOREMOVE ) )
				current_rampto_continue = 0;

			  //sprintf( string, "i_set = %f, i_setpoint = %u while i_target_int = %u", i_set, i_setpoint, current_target_int );
			  //if( MessageBox( hDlg, string, "Status 3", MB_OKCANCEL ) == IDCANCEL )
				//current_rampto_continue = 0;
			}while( (i_setpoint > current_target_int) && current_rampto_continue );
		  }
		  else;	// do nothing when equal
		  current_rampto_continue = 0;
		  repaint_tip_current( hDlg );
		  break;
  		/////////////////////////////////////
		// End of Mod 11, Current_RampTo //
		/////////////////////////////////////


        case TIP_CURRENT_AMPS_EDIT:
          if(!out_smart)
          {
            GetDlgItemText(hDlg,TIP_CURRENT_AMPS_EDIT,string,5);
            i_set = (atof(string));
            i_setpoint = calc_i_setpoint(i_set,i_set_range,tip_gain);
            i_set = calc_i_set(i_setpoint,i_set_range,tip_gain);
            //tip_current(i_setpoint);
            tip_current_serial(i_setpoint);
          }
          break;
        case TIP_CURRENT_GAIN_8:
          if(IsDlgButtonChecked(hDlg,TIP_CURRENT_GAIN_8))
          {
            SetDlgItemText(hDlg,TIP_CURRENT_RANGE_TEXT,"1 to 100 nA");
            tip_gain = 8;
            if(SD->step_delay_fixed == FALSE && !SD->scan_feedback)
            {
              SD->step_delay = 100;
              repaint_step_delay(hDlg);
            }
            repaint_tip_current(hDlg);
          }
          break;
        case TIP_CURRENT_GAIN_9:
          if(IsDlgButtonChecked(hDlg,TIP_CURRENT_GAIN_9))
          {
            SetDlgItemText(hDlg,TIP_CURRENT_RANGE_TEXT,"0.1 to 10 nA");
            tip_gain = 9;
            if(SD->step_delay_fixed == FALSE && !SD->scan_feedback)
            {
              SD->step_delay = 200;
              repaint_step_delay(hDlg);
            }
            repaint_tip_current(hDlg);
          }
          break;
        case TIP_CURRENT_GAIN_10:
          if(IsDlgButtonChecked(hDlg,TIP_CURRENT_GAIN_10))
          {
            SetDlgItemText(hDlg,TIP_CURRENT_RANGE_TEXT,"0.01 to 1 nA");
            tip_gain = 10;
            if(SD->step_delay_fixed == FALSE && !SD->scan_feedback)
            {
              SD->step_delay = 1000;
              repaint_step_delay(hDlg);
            }
            repaint_tip_current(hDlg);
          }
          break;
        case TIP_CURRENT_GAIN_11:
          if(IsDlgButtonChecked(hDlg,TIP_CURRENT_GAIN_11))
          {
            SetDlgItemText(hDlg,TIP_CURRENT_RANGE_TEXT,"0.001 to 0.1 nA");
            tip_gain = 11;
            if(SD->step_delay_fixed == FALSE && !SD->scan_feedback)
            {
              SD->step_delay = 1000;
              repaint_step_delay(hDlg);
            }
            repaint_tip_current(hDlg);
          }
          break;
        case TIP_CURRENT_BITS_EDIT:
          GetDlgItemText(hDlg,TIP_CURRENT_BITS_EDIT,string,5);
          i = atoi(string);
          i = min(max(i,tip_current_min),tip_current_max);
          i_setpoint = (unsigned int)i;
          //tip_current(i_setpoint);
          tip_current_serial(i_setpoint);
          i_set = calc_i_set(i_setpoint,i_set_range,tip_gain);
          break;

        // delays
        case DELAY_FREQ_DEP:
          if(IsDlgButtonChecked(hDlg,DELAY_FREQ_DEP))
          {
            SD->step_delay_fixed = FALSE;
            calc_gains(&x_gain,&y_gain,&z_gain,&z2_gain);
            repaint_step_delay(hDlg);
            CheckDlgButton(hDlg,DELAY_FREQ_DEP,1);
            CheckDlgButton(hDlg,DELAY_FIXED,0);
          }
          break;
        case DELAY_FIXED:
          if(IsDlgButtonChecked(hDlg,DELAY_FIXED))
          {
            SD->step_delay_fixed = TRUE;
            CheckDlgButton(hDlg,DELAY_FREQ_DEP,0);
            CheckDlgButton(hDlg,DELAY_FIXED,1);
          }
        case DELAY_FIXED_VALUE:
          GetDlgItemText(hDlg,DELAY_FIXED_VALUE,string,9);
          SD->step_delay = atoi(string);
          break;
          break;
        case INTER_STEP_DELAY_VALUE:
            GetDlgItemText(hDlg,INTER_STEP_DELAY_VALUE,string,10);
            i = atoi(string);
            i = min(max(i,INTER_STEP_MIN_DELAY),INTER_STEP_MAX_DELAY);
            SD->inter_step_delay = (int)i;
            break;
        case INTER_LINE_DELAY_VALUE:
            GetDlgItemText(hDlg,INTER_LINE_DELAY_VALUE,string,10);
            i = atoi(string);
            i = min(max(i,INTER_LINE_MIN_DELAY),INTER_LINE_MAX_DELAY);
            SD->inter_line_delay = (int)i * 1000;
            break;

        case PRE_SCAN_CONTROLS:
          lpfnDlgProc = MakeProcInstance(IOCtrlDlg, hInst);
          DialogBox(hInst,"IODLG",hDlg,lpfnDlgProc);
		  FreeProcInstance(lpfnDlgProc);

		  // after i/o controls dialog, update prescan variables & controls
          switch(dac_data[gain_ch] & z_gain_bits)
          {
            case Z_TENTH_GAIN:
              scan_freq = 0;
              break;            
            case Z_ONE_GAIN:
              scan_freq = 1;
              break;            
            case Z_TEN_GAIN:
              scan_freq = 2;
              break;
          }
                
          switch(dac_data[gain_ch] & z2_gain_bits)
          {
            case Z2_TENTH_GAIN:
              scan_scale = 0;
              break;            
            case Z2_ONE_GAIN:
              scan_scale = 1;
              break;            
            case Z2_TEN_GAIN:
              scan_scale = 2;
              break;
          }
          calc_gains(&x_gain,&y_gain,&z_gain,&z2_gain);
          repaint_gains(hDlg);
          repaint_scan_scale(hDlg);
          repaint_scan_freq(hDlg);
          scan_z = dac_data[z_offset_ch];
          repaint_scan_z(hDlg);
          repaint_sample_bias(hDlg);
          repaint_tip_current(hDlg);
          switch (tip_gain)
          {
            case 8:
              CheckDlgButton(hDlg,TIP_CURRENT_GAIN_8,1);
              CheckDlgButton(hDlg,TIP_CURRENT_GAIN_9,0);
              CheckDlgButton(hDlg,TIP_CURRENT_GAIN_10,0);
			  CheckDlgButton(hDlg, TIP_CURRENT_GAIN_11,0);
              SetDlgItemText(hDlg,TIP_CURRENT_RANGE_TEXT,"1 to 100 nA");
              break;
            case 9:
              CheckDlgButton(hDlg,TIP_CURRENT_GAIN_8,0);
              CheckDlgButton(hDlg,TIP_CURRENT_GAIN_9,1);
              CheckDlgButton(hDlg,TIP_CURRENT_GAIN_10,0);
			  CheckDlgButton(hDlg,TIP_CURRENT_GAIN_11,0);
			  SetDlgItemText(hDlg,TIP_CURRENT_RANGE_TEXT,"0.1 to 10 nA");
              break;
            case 10:
              CheckDlgButton(hDlg,TIP_CURRENT_GAIN_8,0);
              CheckDlgButton(hDlg,TIP_CURRENT_GAIN_9,0);
              CheckDlgButton(hDlg,TIP_CURRENT_GAIN_10,1);
			  CheckDlgButton(hDlg,TIP_CURRENT_GAIN_11,0);
              SetDlgItemText(hDlg,TIP_CURRENT_RANGE_TEXT,"0.01 to 1 nA");
              break;
			case 11:
              CheckDlgButton(hDlg,TIP_CURRENT_GAIN_11,1);
			  CheckDlgButton(hDlg,TIP_CURRENT_GAIN_8,0);
			  CheckDlgButton(hDlg,TIP_CURRENT_GAIN_9,0);
			  CheckDlgButton(hDlg,TIP_CURRENT_GAIN_10,0);
              SetDlgItemText(hDlg,TIP_CURRENT_RANGE_TEXT,"0.001 to 0.1 nA");
              break;

          }
          break;
            
		// change z gain
        case SCAN_SCALE_001:
          if(IsDlgButtonChecked(hDlg,SCAN_SCALE_001))
          {
#ifdef OLD
            scan_scale = 0;
            if(scan_freq == 2)
            {
              scan_freq = 1;
              CheckDlgButton(hDlg,SCAN_FREQ_10,0);
              CheckDlgButton(hDlg,SCAN_FREQ_1,1);
            }
#endif
            switch(scan_scale)
            {
              case 1:
                scan_freq += 1;
                if(scan_freq > 2) scan_freq = 2;
                auto_z_below(IN_ZERO - Z_SAFETY / 10);
                break;
              case 2:
                scan_freq += 2;
                if(scan_freq > 2) scan_freq = 2;
                auto_z_below(IN_ZERO - Z_SAFETY / 10);
                scan_scale = 1;
                calc_gains(&x_gain,&y_gain,&z_gain,&z2_gain);
                //set_gain(x_gain,y_gain,z_gain,z2_gain);
                set_gain_serial(x_gain,y_gain,z_gain,z2_gain);
                auto_z_below(IN_ZERO);
                break;
            }
            if(scan_freq > 2) scan_freq = 2;
                
            scan_scale = 0;
                      
            calc_gains(&x_gain,&y_gain,&z_gain,&z2_gain);
            //set_gain(x_gain,y_gain,z_gain,z2_gain);
            set_gain_serial(x_gain,y_gain,z_gain,z2_gain);
            repaint_scan_z(hDlg);
            repaint_step_delay(hDlg);
            repaint_scan_freq(hDlg);
          }
          break;
        case SCAN_SCALE_01:
          if(IsDlgButtonChecked(hDlg, SCAN_SCALE_01))
          {
#ifdef OLD
            scan_scale = 1;
#endif
            switch(scan_scale)
            {
              case 0:
                scan_freq -= 1;
                if(scan_freq > 2) scan_freq = 2;
                if(scan_freq < 0) scan_freq = 0;
                auto_z_above(IN_ZERO + Z_SAFETY);
//                auto_z_below(IN_ZERO - Z_SAFETY);
                break;
              case 2:
                scan_freq += 1;
                if(scan_freq > 2) scan_freq = 2;
                if(scan_freq < 0) scan_freq = 0;
                auto_z_below(IN_ZERO - Z_SAFETY);
//                auto_z_above(IN_ZERO + Z_SAFETY);
                break;
            }

			scan_scale = 1;
            calc_gains(&x_gain,&y_gain,&z_gain,&z2_gain);
            //set_gain(x_gain,y_gain,z_gain,z2_gain);
            set_gain_serial(x_gain,y_gain,z_gain,z2_gain);
            repaint_scan_z(hDlg);
            repaint_step_delay(hDlg);
            repaint_scan_freq(hDlg);
          }
          break;

        case SCAN_SCALE_1:
          if (IsDlgButtonChecked(hDlg, SCAN_SCALE_1))
          {
#ifdef OLD
            scan_scale = 2;
            if(scan_freq == 0)
            {
              scan_freq = 1;
              CheckDlgButton(hDlg,SCAN_FREQ_100,0);
              CheckDlgButton(hDlg,SCAN_FREQ_1,1);
            }
#endif
            switch(scan_scale)
            {
              case 0:
                scan_freq -= 2;
                if(scan_freq < 0) scan_freq = 0;
                auto_z_above(IN_ZERO + Z_SAFETY * 10);
//                auto_z_below(IN_ZERO - Z_SAFETY * 10);
                scan_scale = 1;
                calc_gains(&x_gain,&y_gain,&z_gain,&z2_gain);
                //set_gain(x_gain,y_gain,z_gain,z2_gain);
                set_gain_serial(x_gain,y_gain,z_gain,z2_gain);
                auto_z_above(IN_ZERO);
                break;
             case 1:
               scan_freq -= 1;
               if(scan_freq < 0) scan_freq = 0;
               auto_z_above(IN_ZERO + Z_SAFETY * 10);
//               auto_z_below(IN_ZERO - Z_SAFETY * 10);
               break;
            }
            scan_scale = 2;
            calc_gains(&x_gain,&y_gain,&z_gain,&z2_gain);
            //set_gain(x_gain,y_gain,z_gain,z2_gain);
            set_gain_serial(x_gain,y_gain,z_gain,z2_gain);
                
			repaint_scan_z(hDlg);
            repaint_step_delay(hDlg);
            repaint_scan_freq(hDlg);
          }
          break;
		case SCAN_FREQ_100:
          if(IsDlgButtonChecked(hDlg,SCAN_FREQ_100))
          {
            scan_freq = 0;
            calc_gains(&x_gain,&y_gain,&z_gain,&z2_gain);
            //set_gain(x_gain,y_gain,z_gain,z2_gain);
            set_gain_serial(x_gain,y_gain,z_gain,z2_gain);
            repaint_step_delay(hDlg);
            repaint_scan_freq(hDlg);
          }
          break;
        case SCAN_FREQ_1:
          if(IsDlgButtonChecked(hDlg,SCAN_FREQ_1))
          {
            scan_freq = 1;
            calc_gains(&x_gain,&y_gain,&z_gain,&z2_gain);
            //set_gain(x_gain,y_gain,z_gain,z2_gain);
            set_gain_serial(x_gain,y_gain,z_gain,z2_gain);
            repaint_step_delay(hDlg);
            repaint_scan_freq(hDlg);
          }
          break;
        case SCAN_FREQ_10:
          if(IsDlgButtonChecked(hDlg,SCAN_FREQ_10))
          {
            scan_freq = 2;
            calc_gains(&x_gain,&y_gain,&z_gain,&z2_gain);
            //set_gain(x_gain,y_gain,z_gain,z2_gain);
            set_gain_serial(x_gain,y_gain,z_gain,z2_gain);
            repaint_step_delay(hDlg);
            repaint_scan_freq(hDlg);
          }
          break;

        case ENTER:
//        SetFocus(GetDlgItem(hDlg,PRESCAN_EXIT));
          repaint_inter_step(hDlg);
          repaint_inter_line(hDlg);
          repaint_crash(hDlg);
          repaint_scan_z(hDlg);
          repaint_sample_bias(hDlg);
          repaint_tip_current(hDlg);
          repaint_prescan_conseq(hDlg);
          repaint_prescan_over(hDlg);
		  repaint_this_sequence(hDlg);
		  repaint_current_target(hDlg);
          repaint_bias_target(hDlg);
          repaint_current_rampto(hDlg);
          repaint_bias_rampto(hDlg);
		  break;

        case PRESCAN_EXIT:
			free_rs_types_list();
            if(check_crash_protection(hDlg)) break;
            EndDialog(hDlg,TRUE);
            return(TRUE);
      }
      break;
  }
  return (FALSE);
}


BOOL FAR PASCAL Prescan2Dlg(HWND hDlg, unsigned Message, WPARAM wParam, LPARAM lParam)
{
  int i;
  double f;
//  int id;
//  int delt,ddelt,mini,maxi;

  static FARPROC lpfnDlgProc;

  switch(Message) {
    case WM_INITDIALOG:
      scan_now = 0;
      SetDlgItemText(hDlg,SAMPLE_TYPE_TEXT,data->sample_type.ptr);
      SetDlgItemText(hDlg,PRESCAN_DOSED_WITH,data->dosed_type.ptr); 
      sprintf(string,"%0.4f",data->dosed_langmuir);
      SetDlgItemText(hDlg,PRESCAN_DOSED_LANG,string);
      sprintf(string,"%0.2f",data->temperature);
      SetDlgItemText(hDlg,PRESCAN_TEMP,string);
      CheckDlgButton(hDlg,PRESCAN_OVER_FAST,(SD->overshoot & OVERSHOOT_FAST));
      CheckDlgButton(hDlg,PRESCAN_OVER_SLOW,(SD->overshoot & OVERSHOOT_SLOW));
      read_sample_list(&sample_list,SAMPLES_INI,"",PRESCAN_UNKNOWN_SAMPLE);
      read_sample_list(&dosed_list,DOSED_INI,"",PRESCAN_UNKNOWN_DOSED);
      init_listbox(hDlg,PRESCAN_SAMPLE_LIST,&(sample_list.listbox));
      init_listbox(hDlg,PRESCAN_DOSED_LIST,&(dosed_list.listbox));
	  repaint_angs_calibration(hDlg);

/*
      if(first_time)
      {
        first_time = 0;

	    SendDlgItemMessage(hDlg,PRESCAN_SAMPLE_LIST,CB_SETCURSEL,
                (WPARAM) SendDlgItemMessage(hDlg,PRESCAN_SAMPLE_LIST,
                CB_FINDSTRINGEXACT,0,(LPARAM)PRESCAN_UNKNOWN_SAMPLE),
                (LPARAM)0);
            SendDlgItemMessage(hDlg,PRESCAN_DOSED_LIST,CB_SETCURSEL,
                (WPARAM) SendDlgItemMessage(hDlg,PRESCAN_DOSED_LIST,
                CB_FINDSTRINGEXACT,0,(LPARAM)PRESCAN_UNKNOWN_DOSED),
                (LPARAM)0);
            strcpy(cur_sample,sample_list.listbox.str[0]);
            strcpy(cur_dosed,dosed_list.listbox.str[0]);
      }
      else
*/
      {
        SendDlgItemMessage(hDlg,PRESCAN_SAMPLE_LIST,CB_SETCURSEL,
                (WPARAM) SendDlgItemMessage(hDlg,PRESCAN_SAMPLE_LIST,
                CB_FINDSTRINGEXACT,0,(LPARAM)cur_sample),
                (LPARAM)0);
        SendDlgItemMessage(hDlg,PRESCAN_DOSED_LIST,CB_SETCURSEL,
                (WPARAM) SendDlgItemMessage(hDlg,PRESCAN_DOSED_LIST,
                CB_FINDSTRINGEXACT,0,(LPARAM)cur_dosed),
                (LPARAM)0);
      }
      repaint_prescan_over(hDlg);
      repaint_gains(hDlg);
        
      break;

    case WM_PAINT:
      break;
    case WM_LBUTTONDOWN:
      break;

    case WM_HSCROLL:
    case WM_VSCROLL:
/*
	  id = getscrollid();
      switch(id)
      {
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
      switch(id)
      {
      }
*/
      break;

	case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case PRESCAN_TEMP:
          GetDlgItemText(hDlg,PRESCAN_TEMP,string,15);
          data->temperature  = (float)atof(string);
//          data2->temperature  = atof(string);
          break;

		// listboxes
        case PRESCAN_SAMPLE_LIST:
          switch(getcombomsg())
          {
            case CBN_SELCHANGE:
              i = SendDlgItemMessage(hDlg,PRESCAN_SAMPLE_LIST,CB_GETCURSEL,0,0);
              strcpy(data->sample_type.ptr,sample_list.name[sample_list.listbox.index[i]]);
//              strcpy(data2->sample_type.ptr,sample_list.name[sample_list.listbox.index[i]]);
              strcpy(cur_sample,sample_list.listbox.str[sample_list.listbox.index[i]]);
              SetDlgItemText(hDlg,SAMPLE_TYPE_TEXT,data->sample_type.ptr);
              data->sample_type.size = strlen(data->sample_type.ptr);
//              data2->sample_type.size = strlen(data->sample_type.ptr);
              strcpy(current_dir_stm,initial_dir_stm);
              if(strcmp(cur_sample,PRESCAN_UNKNOWN_SAMPLE))
              {
                strcat(current_dir_stm,"\\");
                strcat(current_dir_stm,cur_sample);
                if(strcmp(cur_dosed,PRESCAN_UNKNOWN_DOSED))
                {
                  strcat(current_dir_stm,"\\");
                  strcat(current_dir_stm,cur_dosed);
                }
              }
              if(!isdir(current_dir_stm))
              {
                sprintf(string,"%s doesn't exist!\nCreate it?",current_dir_stm);
                if(MessageBox(hDlg,string,"Notice!",MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
                {
                  if(mkdir(current_dir_stm))
                  {
                                MessageBox(hDlg,"Mkdir Failed!","Warning!",
                                    MB_OK | MB_ICONEXCLAMATION);
                  }
                }
                else
                {
                  strcpy(current_dir_stm,initial_dir_stm);
                }
              }
              init_file(current_dir_stm,current_file_stm,"stm",&file_stm_count);

              break;
          }
          break;
        case PRESCAN_DOSED_LIST:
          switch(getcombomsg())
          {
            case CBN_SELCHANGE:
              i = SendDlgItemMessage(hDlg,PRESCAN_DOSED_LIST,CB_GETCURSEL,0,0);
              strcpy(data->dosed_type.ptr,dosed_list.name[dosed_list.listbox.index[i]]);
//              strcpy(data2->dosed_type.ptr,dosed_list.name[dosed_list.listbox.index[i]]);
              strcpy(cur_dosed,dosed_list.listbox.str[dosed_list.listbox.index[i]]);
              SetDlgItemText(hDlg,PRESCAN_DOSED_WITH,data->dosed_type.ptr);
              data->dosed_type.size = strlen(data->dosed_type.ptr);
//              data2->dosed_type.size = strlen(data2->dosed_type.ptr);
              strcpy(current_dir_stm,initial_dir_stm);
              if(strcmp(cur_sample,PRESCAN_UNKNOWN_SAMPLE))
              {
                strcat(current_dir_stm,"\\");
                strcat(current_dir_stm,cur_sample);
                if(strcmp(cur_dosed,PRESCAN_UNKNOWN_DOSED))
                {
                  strcat(current_dir_stm,"\\");
                  strcat(current_dir_stm,cur_dosed);
                }
              }
              if(!isdir(current_dir_stm))
              {
                sprintf(string,"%s doesn't exist!\nCreate it?",current_dir_stm);
                if(MessageBox(hDlg,string,"Notice!",MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
                {
                  if(mkdir(current_dir_stm))
                  {
                    MessageBox(hDlg,"Mkdir Failed!","Warning!",MB_OK | MB_ICONEXCLAMATION);
                  }
				}
                else
				{
                  strcpy(current_dir_stm,initial_dir_stm);
				}
              }
              init_file(current_dir_stm,current_file_stm,"stm",&file_stm_count);
                            
              break;
          }
          break;
        case SAMPLE_TYPE_TEXT:  // messy
          GetDlgItemText(hDlg,SAMPLE_TYPE_TEXT,data->sample_type.ptr,SAMPLETYPESIZE);
          data->sample_type.size = strlen(data->sample_type.ptr);
//          GetDlgItemText(hDlg,SAMPLE_TYPE_TEXT,data2->sample_type.ptr,SAMPLETYPESIZE);
//          data2->sample_type.size = strlen(data2->sample_type.ptr);
            break;
        case PRESCAN_DOSED_WITH:  // messy
            GetDlgItemText(hDlg,PRESCAN_DOSED_WITH,data->dosed_type.ptr,SAMPLETYPESIZE);
            data->dosed_type.size = strlen(data->dosed_type.ptr);
//            GetDlgItemText(hDlg,PRESCAN_DOSED_WITH,data2->dosed_type.ptr,SAMPLETYPESIZE);
//            data2->dosed_type.size = strlen(data2->dosed_type.ptr);
            break;
        case PRESCAN_DOSED_LANG:
          if(!out_smart)
          {
            GetDlgItemText(hDlg, PRESCAN_DOSED_LANG, string, 15);
            data->dosed_langmuir  = (float)atof(string);
//            data2->dosed_langmuir  = atof(string);
          }
          break;

		// overshoot parameters
		case PRESCAN_OVER_FAST:
          if (IsDlgButtonChecked(hDlg,PRESCAN_OVER_FAST))
          {
            SD->overshoot |= OVERSHOOT_FAST;
          }
          else
          {
            SD->overshoot &= (~OVERSHOOT_FAST);
          }
          break;
        case PRESCAN_OVER_SLOW:
          if(IsDlgButtonChecked(hDlg,PRESCAN_OVER_SLOW))
          {
            SD->overshoot |= OVERSHOOT_SLOW;
          }
          else
          {
            SD->overshoot &= (~OVERSHOOT_SLOW);
          }
          break;
        case PRESCAN_OVER_PERCENT:
          if(!out_smart)
          {
            GetDlgItemText(hDlg,PRESCAN_OVER_PERCENT,string,15);
            SD->overshoot_percent = (float)atof(string);
            SD->overshoot_percent = min(max(SD->overshoot_percent, 
								PRESCAN_OVER_PERCENT_MIN),PRESCAN_OVER_PERCENT_MAX);
          }
          break;
        case PRESCAN_OVER_WAIT1:
          if(!out_smart)
          {
            GetDlgItemText(hDlg,PRESCAN_OVER_WAIT1,string,15);
            SD->overshoot_wait1 = atoi(string);
            SD->overshoot_wait1 = min(max(SD->overshoot_wait1, 
								PRESCAN_OVER_WAIT1_MIN),PRESCAN_OVER_WAIT1_MAX);
          }
          break;
        case PRESCAN_OVER_WAIT2:
          if(!out_smart)
          {
            GetDlgItemText(hDlg,PRESCAN_OVER_WAIT2,string,15);
            SD->overshoot_wait2  = atoi(string);
            SD->overshoot_wait2 = min(max( SD->overshoot_wait2, 
								PRESCAN_OVER_WAIT2_MIN),PRESCAN_OVER_WAIT2_MAX);
          }
          break;

		// response calibration
        case PRESCAN_A_PER_V:
          if(!out_smart)
		  {
		    out_smart = 1;
            GetDlgItemText(hDlg,PRESCAN_A_PER_V,string,15);
		    f = atof(string);
		    if(f <= 0.0)
			{
		      f = 200.0; // default value
		      a_per_v = f;
		      //repaint_angs_calibration(hDlg);
			}
		    a_per_v = f;
            SendMessage(ScnDlg,WM_COMMAND,SCAN_REPAINT_SIZES,1);
		    out_smart = 0;
		  }
	      break;
	    case PRESCAN_Z_A_PER_V:
          if(!out_smart)
          {
		    out_smart = 1;
            GetDlgItemText(hDlg,PRESCAN_Z_A_PER_V,string,15);
		    f = atof(string);
		    if(f <= 0.0)
			{
		      f = 30.0; // default value
		      z_a_per_v = f;
		      repaint_angs_calibration(hDlg);
			}
		    z_a_per_v = f;
            SendMessage(ScnDlg, WM_COMMAND, SCAN_REPAINT_SIZES, 1);
		    out_smart = 0;
		  }
	      break;
            
		// ranges
		case X_RANGE_5:
            //set_range(x_ch,1);
            set_range_serial(x_ch,1);
            break;
        case X_RANGE_10:
            //set_range(x_ch,2);
            set_range_serial(x_ch,2);
            break;
        case X_OFFSET_RANGE_5:
            //set_range(x_offset_ch,1);
            set_range_serial(x_offset_ch,1);
            break;
        case X_OFFSET_RANGE_10:
            //set_range(x_offset_ch,2);
            set_range_serial(x_offset_ch,2);
            break;
        case Y_RANGE_5:
            //set_range(y_ch,1);
            set_range_serial(y_ch,1);
            break;
        case Y_RANGE_10:
            //set_range(y_ch,2);
            set_range_serial(y_ch,2);
            break;
        case Y_OFFSET_RANGE_5:
            //set_range(y_offset_ch,1);
            set_range_serial(y_offset_ch,1);
            break;
        case Y_OFFSET_RANGE_10:
            //set_range(y_offset_ch,2);
            set_range_serial(y_offset_ch,2);
            break;
        case Z_RANGE_5:
            //set_range(zo_ch,1);
            set_range_serial(zo_ch,1);
            break;
        case Z_RANGE_10:
            //set_range(zo_ch,2);
            set_range_serial(zo_ch,2);
            break;
        case Z_OFFSET_RANGE_5:
            //set_range(z_offset_ch,1);
            set_range_serial(z_offset_ch,1);
            break;
        case Z_OFFSET_RANGE_10:
            //set_range(z_offset_ch,2);
            set_range_serial(z_offset_ch,2);
            break;

        case ENTER:
            repaint_prescan_over(hDlg);
			repaint_angs_calibration(hDlg);
			repaint_prescan_temp(hDlg);
            break;
        
        case PRESCAN2_EXIT:
            free_sample_list(&sample_list);
            free_sample_list(&dosed_list);
            EndDialog(hDlg,TRUE);
            return(TRUE);
//            DestroyWindow(hDlg);
//            Prescan2Wnd = NULL;
//            Prescan2On = FALSE;
            break;
      }
      break;
  }
  return (FALSE);
}

int auto_z_below(int target)
{    
    if(feedback)
    {
	  auto_serial(AUTOMINUS);
	  scan_z = dac_data[z_offset_ch];
    }
    else
    {
      mprintf("Feedback is OFF!!");
      return 0;
    }
    return(scan_z != MAX -1  && scan_z != 0); /*success if not railed*/
}     

int auto_z_above(int target)
{
    if(feedback)
    {            
	  auto_serial(AUTOPLUS);
	  scan_z = dac_data[z_offset_ch];
    }
    else
    {
      mprintf("Feedback is OFF!!");
      return 0;
    }
        
    return(scan_z != MAX - 1 && scan_z != 0); /*success if not railed*/
}    

unsigned int calc_i_setpoint(double i_set,int i_set_range,unsigned int tip_gain)
{
/*
  The following segment of code will calculate I(setpoint) to give to the feedback loop.
  The user enters the desired current in NanoAmps, which is then multiplied by
  the PreAmp gain as set on an exterior control. This number (I(set) * Gain) is V(in), and
  must be further divided by 1 V, then logged (base 10), then multiplied by 10 to be
  suitable for comparison with the value of V(in) returned by the analog feedback loop.
  Note that there are two V(in)s, an original value given by the user in terms of I(set) and
  the V(in) returned by the feedback loop.
        i_set is the current value given by the user in NanoAmps
        i_setpoint is the voltage value to be compared with V(in) from feedback loop
*/
    double v_in;
    double temp;
    v_in = (((i_set) * 1E-9) * (pow(10,tip_gain)));
    temp = -10 * log10(v_in);
    if(temp < -5 * i_set_range)
        return(MIN);
    if(temp >= 5 * i_set_range) return(MAX);
    return(max(min(vtod(temp,i_set_range),MAX),MIN));//Shaowei 2/6/2018 fix rounding issue at gain 9
}

void read_sample_list(SAMPLELIST *list,char *fname,char *first_name,
    char *first_dir)
{
	// get list of sample (or dosed) names from file "fname" and put in *list
    FILE *fp;
    int count;
    char buf[SAMPLE_BUF_MAX];
    
    count = 0;
    list->name[count] = (char *)malloc(sizeof(char) * (strlen(first_name) + 1));
    strcpy(list->name[count],first_name);
    
    list->listbox.str[count] = (char *)malloc(sizeof(char) * (strlen(first_dir) + 1));
    strcpy(list->listbox.str[count],first_dir);
    count++;
    fp = fopen(fname,"r");
    if(fp == NULL)
    {
        MessageBox(GetFocus(),fname,"File not found!",MB_ICONHAND);    
        return;
    }
        
    fgets(buf,SAMPLE_BUF_MAX - 1,fp);
    buf[strlen(buf) - 1] = '\0';
    list->name[count] = (char *)malloc(sizeof(char) * (strlen(buf) + 1));
    strcpy(list->name[count],buf);
    
    fgets(buf,SAMPLE_BUF_MAX - 1,fp);
    buf[strlen(buf) - 1] = '\0';
    list->listbox.str[count] = (char *)malloc(sizeof(char) * (strlen(buf) + 1));
    strcpy(list->listbox.str[count],buf);
//    MessageBox(GetFocus(),list->listbox.str[count],list->name[count],MB_ICONHAND);    
    count++;
    while(1)
    {
        if(fgets(buf,SAMPLE_BUF_MAX - 1,fp) == NULL) break;
        buf[strlen(buf) - 1] = '\0';
        list->name[count] = (char *)malloc(sizeof(char)*(strlen(buf) + 1));
        strcpy(list->name[count],buf);
        if(fgets(buf,SAMPLE_BUF_MAX - 1,fp) == NULL) break;
        buf[strlen(buf) - 1] = '\0';
        list->listbox.str[count] = (char *)malloc(sizeof(char) * (strlen(buf) + 1));
        strcpy(list->listbox.str[count],buf);
//        MessageBox(GetFocus(),list->listbox.str[count],list->name[count],MB_ICONHAND);    
        count++;
    }
    list->listbox.num = count;
    fclose(fp);
}
        
void free_sample_list(SAMPLELIST *list)
{
	// free up memory malloc'ed by read_sample_list
    int i;
    
    for(i = 0;i < list->listbox.num;i++)
    {
        free(list->listbox.str[i]);
        free(list->name[i]);
    }
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

int isdir(char *name)
{
	// returns 1 if directory "name" exists, otherwise 0

#define ISDIR_STR_LENGTH 255

	char old_file[ISDIR_STR_LENGTH];

	GetCurrentDirectory(ISDIR_STR_LENGTH,old_file);
	
    if( SetCurrentDirectory(name)) {
		SetCurrentDirectory(old_file);
        return 1;
    } 
    else
    {
		SetCurrentDirectory(old_file);
        return 0;
    }
}

static int z_read()
{
    int i;
    
    for(i = 0;i < SD->total_steps;i++)
    {
        if(SD->sequence[i].type == READ_TYPE && SD->sequence[i].in_ch == zi_ch)
            return 1;
    }
    return 0;
}

static int check_crash_protection(HWND hDlg)
{
    int old_crash;
    
    old_crash = SD->crash_protection;
    
    if(SD->scan_feedback && z_read())
    {
        enable_crash_protection(hDlg,TRUE);
    }
    else
    {
        CheckDlgButton(hDlg,CRASH_DO_NOTHING,1);
        CheckDlgButton(hDlg,CRASH_STOP,0);
        CheckDlgButton(hDlg,CRASH_AUTO_Z,0);
        CheckDlgButton(hDlg,CRASH_MINMAX_Z,0);
        SD->crash_protection = CRASH_DO_NOTHING;
        enable_crash_protection(hDlg,FALSE);
        
    }
    if (old_crash != SD->crash_protection) return 1;
    else return 0;
}

static void enable_crash_protection(HWND hDlg,BOOL status)
{
    EnableWindow(GetDlgItem(hDlg,CRASH_STOP),status);
    EnableWindow(GetDlgItem(hDlg,CRASH_DO_NOTHING),status);
    EnableWindow(GetDlgItem(hDlg,CRASH_AUTO_Z),status);
    EnableWindow(GetDlgItem(hDlg,CRASH_MINMAX_Z),status);
    EnableWindow(GetDlgItem(hDlg,CRASH_TOLERANCE),status);
}

static void check_digital_feedback(HWND hDlg)
{
    if(SD->scan_feedback)
    {
        enable_digital(hDlg,TRUE);
    }
    else
    {
        SD->digital_feedback = 0;
        CheckDlgButton(hDlg,WAIT_FOR_CURRENT,0);
        enable_digital(hDlg,FALSE);
    }
}

static void enable_digital(HWND hDlg,BOOL status)
{
/*
    EnableWindow(GetDlgItem(hDlg,WAIT_FOR_CURRENT),status);
    EnableWindow(GetDlgItem(hDlg,TIP_SPACING),status);
    EnableWindow(GetDlgItem(hDlg,TIP_SPACING_SCROLL),status);
    EnableWindow(GetDlgItem(hDlg,DIGITAL_ABORT),status);
    EnableWindow(GetDlgItem(hDlg,DIGITAL_MAX_TRY),status);
*/
	// not yet implemented with DSP... may never need it
    EnableWindow(GetDlgItem(hDlg,WAIT_FOR_CURRENT),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_SPACING),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_SPACING_SCROLL),FALSE);
    EnableWindow(GetDlgItem(hDlg,DIGITAL_ABORT),FALSE);
    EnableWindow(GetDlgItem(hDlg,DIGITAL_MAX_TRY),FALSE);
}

static void copy_seq(READ_SEQ *dest,READ_SEQ *src)
{
    memcpy(dest,src,sizeof(READ_SEQ));
}

static void set_dumb_seq(READ_SEQ *this_seq)
{
    this_seq->wait1 = this_seq->wait2 = this_seq->wait3 = this_seq->wait4 = 0;
    this_seq->feedback = 1;
    this_seq->dither0 = this_seq->dither1 = 0;
    this_seq->record = 0;
    this_seq->read_ch = 1;
    this_seq->num_samples = 5;
    this_seq->do_ramp_bias = 0;
    this_seq->ramp_value = 0;
}












static void add_sequence(HWND hDlg)
{
	// add a new read step at the end of the read sequence
  if(SD->total_steps < MAX_READ_STEPS)
  {
    SD->total_steps++;
	current_step = SD->total_steps - 1;
	SD->sequence[current_step].type = WAIT_TYPE;
	SD->sequence[current_step].wait = 100; // 100 microsec wait is default
	strcpy(cur_rs_type,"Wait");
	strcpy(SD->sequence[current_step].step_type_string,cur_rs_type);
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

  if(SD->total_steps < MAX_READ_STEPS)
  {
    for(i = SD->total_steps;i > current_step;i--)
	{
      copy_step(&(SD->sequence[i]),&(SD->sequence[i - 1]));
	}
    SD->total_steps++;
	SD->sequence[current_step].type = WAIT_TYPE;
	SD->sequence[current_step].wait = 100; // 100 microsec wait is default
	strcpy(cur_rs_type,"Wait");
	strcpy(SD->sequence[current_step].step_type_string,cur_rs_type);
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

  if(SD->total_steps > 1)
  {
	for(i = current_step;i < SD->total_steps - 1;i++)
	{
      copy_step(&(SD->sequence[i]),&(SD->sequence[i + 1]));
	}
    SD->total_steps--;
	if(current_step > SD->total_steps - 1) current_step = SD->total_steps - 1;
	strcpy(cur_rs_type,SD->sequence[current_step].step_type_string);
  }
  else
  {
    sprintf(string,"Read sequence can't go below 1 step.",MAX_READ_STEPS);
	MessageBox(hDlg,string,"Warning",MB_OK);
  }    
  check_crash_protection(hDlg);
}


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

static void summary(HWND hDlg)
{
    static FARPROC  lpfnDlgProc;
    
    lpfnDlgProc = MakeProcInstance(SummaryDlg,hInst);
    glob_data = &(SD);
    DialogBox(hInst,"COMMENTDLG",hDlg,lpfnDlgProc);
    FreeProcInstance(lpfnDlgProc);                  
}

void repaint_step_delay(HWND hDlg)
{
    sprintf(string,"%d",SD->step_delay);
    SetDlgItemText(hDlg,DELAY_FIXED_VALUE,string);
}

void repaint_inter_step(HWND hDlg)
{
    sprintf(string,"%d",SD->inter_step_delay);
    SetDlgItemText(hDlg,INTER_STEP_DELAY_VALUE,string);
    SetScrollPos(GetDlgItem(hDlg,INTER_STEP_SCROLL),SB_CTL,SD->inter_step_delay,TRUE);
	int xy_range;
		if (SD->x_first == 1) xy_range = get_range(x_offset_ch);
		else xy_range = get_range(y_offset_ch);

	sprintf(string,"%0.2f", (xy_range)*10.0 / 4096.0 * scan_x_gain * A_PER_V / SD->inter_step_delay * 1000000);
    /* Speed depends on X Y range shaowei */
    SetDlgItemText(hDlg,STEP_VELOCITY,string);
}

void repaint_inter_line(HWND hDlg)
{
    sprintf(string,"%d",SD->inter_line_delay/1000);
    SetDlgItemText(hDlg,INTER_LINE_DELAY_VALUE,string);
    SetScrollPos(GetDlgItem(hDlg,INTER_LINE_SCROLL),SB_CTL,SD->inter_line_delay/1000,TRUE);
}

void repaint_tip_spacing(HWND hDlg)
{
    sprintf(string,"%0.2f",SD->tip_spacing);
    SetDlgItemText(hDlg,TIP_SPACING,string);
    SetScrollPos(GetDlgItem(hDlg,TIP_SPACING_SCROLL),SB_CTL,(int)(SD->tip_spacing * 
				(float)FTOD_SCALE),TRUE);
}

void repaint_prescan_conseq(HWND hDlg)
{
    out_smart = 1;
    sprintf(string,"%d",SD->digital_feedback_reread);
    SetDlgItemText(hDlg,PRESCAN_CONSEQ,string);
    SetScrollPos(GetDlgItem(hDlg,PRESCAN_CONSEQ_SCROLL),SB_CTL,
        SD->digital_feedback_reread,TRUE);
    out_smart = 0;
}

void repaint_prescan_over(HWND hDlg)
{
    out_smart = 1;
    sprintf(string,"%0.2f",SD->overshoot_percent);
    SetDlgItemText(hDlg,PRESCAN_OVER_PERCENT,string);
    sprintf(string,"%d",SD->overshoot_wait1);
    SetDlgItemText(hDlg,PRESCAN_OVER_WAIT1,string);
    sprintf(string,"%d",SD->overshoot_wait2);
    SetDlgItemText(hDlg,PRESCAN_OVER_WAIT2,string);
    out_smart = 0;
}

void repaint_tip_current(HWND hDlg)
{
    SetDlgItemInt(hDlg,TIP_CURRENT_BITS_EDIT,i_setpoint,FALSE);
    SetScrollPos(GetDlgItem(hDlg,TIP_CURRENT_SCROLL),SB_CTL,
				(int)tip_current_max - i_setpoint,TRUE);
    out_smart = 1;
    SetDlgItemText(hDlg,TIP_CURRENT_AMPS_EDIT,gcvt(i_set,5,string));
    out_smart = 0;

	switch (tip_gain)
	{
	case 8:
		CheckDlgButton(hDlg, TIP_CURRENT_GAIN_8, 1);
		CheckDlgButton(hDlg, TIP_CURRENT_GAIN_9, 0);
		CheckDlgButton(hDlg, TIP_CURRENT_GAIN_10, 0);
		CheckDlgButton(hDlg, TIP_CURRENT_GAIN_11, 0);
		SetDlgItemText(hDlg, TIP_CURRENT_RANGE_TEXT, "1 to 100 nA");
		break;
	case 9:
		CheckDlgButton(hDlg, TIP_CURRENT_GAIN_8, 0);
		CheckDlgButton(hDlg, TIP_CURRENT_GAIN_9, 1);
		CheckDlgButton(hDlg, TIP_CURRENT_GAIN_10, 0);
		CheckDlgButton(hDlg, TIP_CURRENT_GAIN_11, 0);
		SetDlgItemText(hDlg, TIP_CURRENT_RANGE_TEXT, "0.1 to 10 nA");
		break;
	case 10:
		CheckDlgButton(hDlg, TIP_CURRENT_GAIN_8, 0);
		CheckDlgButton(hDlg, TIP_CURRENT_GAIN_9, 0);
		CheckDlgButton(hDlg, TIP_CURRENT_GAIN_10, 1);
		CheckDlgButton(hDlg, TIP_CURRENT_GAIN_11, 0);
		SetDlgItemText(hDlg, TIP_CURRENT_RANGE_TEXT, "0.01 to 1 nA");
		break;
	case 11:
		CheckDlgButton(hDlg, TIP_CURRENT_GAIN_11, 1);
		CheckDlgButton(hDlg, TIP_CURRENT_GAIN_8, 0);
		CheckDlgButton(hDlg, TIP_CURRENT_GAIN_9, 0);
		CheckDlgButton(hDlg, TIP_CURRENT_GAIN_10, 0);
		SetDlgItemText(hDlg, TIP_CURRENT_RANGE_TEXT, "0.001 to 0.1 nA");
		break;

	}  //Shaowei 2019-3-25 Add gain 11
}

void repaint_scan_z(HWND hDlg)
{
    out_smart = 1;
    scan_z = min(max(scan_z,scan_z_min),scan_z_max);
    SetDlgItemInt(hDlg,SCAN_Z_BITS_EDIT,scan_z,FALSE);
    SetScrollPos(GetDlgItem(hDlg,SCAN_Z_SCROLL),SB_CTL,scan_z,TRUE);
    SetDlgItemText(hDlg,SCAN_Z_VOLTS_EDIT,gcvt(dtov(scan_z,z_offset_range),5,string));
    SetDlgItemText(hDlg,SCAN_Z_ANGS_EDIT,gcvt(dtov(scan_z,z_offset_range) * (double)Z_A_PER_V,
					5,string));
    out_smart = 0;
}

void repaint_scan_freq(HWND hDlg)
{
    CheckDlgButton(hDlg,SCAN_FREQ_100,0);
    CheckDlgButton(hDlg,SCAN_FREQ_1,0);
    CheckDlgButton(hDlg,SCAN_FREQ_10,0);
    switch(scan_freq)
    {
        case 0:
            CheckDlgButton(hDlg,SCAN_FREQ_100,1);
            break;
        case 1:
            CheckDlgButton(hDlg,SCAN_FREQ_1,1);
            break;
        case 2:
            CheckDlgButton(hDlg,SCAN_FREQ_10,1);
            break;
    }
    SetDlgItemText(hDlg,SCAN_FREQ_LOW,scan_freq_str(string,scan_scale,0));
    SetDlgItemText(hDlg,SCAN_FREQ_MED,scan_freq_str(string,scan_scale,1));
    SetDlgItemText(hDlg,SCAN_FREQ_HIGH,scan_freq_str(string,scan_scale,2));
}

void repaint_scan_scale(HWND hDlg)
{
    CheckDlgButton(hDlg,SCAN_SCALE_001,0);
    CheckDlgButton(hDlg,SCAN_SCALE_01,0);
    CheckDlgButton(hDlg,SCAN_SCALE_1,0);
    switch(scan_scale)
    {
        case 0:
            CheckDlgButton(hDlg,SCAN_SCALE_001,1);
            break;
        case 1:
            CheckDlgButton(hDlg,SCAN_SCALE_01,1);
            break;
        case 2:
            CheckDlgButton(hDlg,SCAN_SCALE_1,1);
            break;
    }
}    

static void repaint_gains(HWND hDlg)
{
    switch(get_range(y_ch))
    {
        case 1:
            CheckDlgButton(hDlg,Y_RANGE_5,1);
            CheckDlgButton(hDlg,Y_RANGE_10,0);
            break;
        case 2:
            CheckDlgButton(hDlg,Y_RANGE_5,0);
            CheckDlgButton(hDlg,Y_RANGE_10,1);
            break;
    }
    switch(get_range(zo_ch))
    {
        case 1:
            CheckDlgButton(hDlg,Z_RANGE_5,1);
            CheckDlgButton(hDlg,Z_RANGE_10,0);
            break;
        case 2:
            CheckDlgButton(hDlg,Z_RANGE_5,0);
            CheckDlgButton(hDlg,Z_RANGE_10,1);
            break;
    }
    switch(get_range(x_ch))
    {
        case 1:
            CheckDlgButton(hDlg,X_RANGE_5,1);
            CheckDlgButton(hDlg,X_RANGE_10,0);
            break;
        case 2:
            CheckDlgButton(hDlg,X_RANGE_5,0);
            CheckDlgButton(hDlg,X_RANGE_10,1);
            break;
    }
    switch(get_range(x_offset_ch))
    {
        case 1:
            CheckDlgButton(hDlg,X_OFFSET_RANGE_5,1);
            CheckDlgButton(hDlg,X_OFFSET_RANGE_10,0);
            break;
        case 2:
            CheckDlgButton(hDlg,X_OFFSET_RANGE_5,0);
            CheckDlgButton(hDlg,X_OFFSET_RANGE_10,1);
            break;
    }
    switch(get_range(y_offset_ch))
    {
        case 1:
            CheckDlgButton(hDlg,Y_OFFSET_RANGE_5,1);
            CheckDlgButton(hDlg,Y_OFFSET_RANGE_10,0);
            break;
        case 2:
            CheckDlgButton(hDlg,Y_OFFSET_RANGE_5,0);
            CheckDlgButton(hDlg,Y_OFFSET_RANGE_10,1);
            break;
    }
    switch(get_range(z_offset_ch))
    {
        case 1:
            CheckDlgButton(hDlg,Z_OFFSET_RANGE_5,1);
            CheckDlgButton(hDlg,Z_OFFSET_RANGE_10,0);
            break;
        case 2:
            CheckDlgButton(hDlg,Z_OFFSET_RANGE_5,0);
            CheckDlgButton(hDlg,Z_OFFSET_RANGE_10,1);
            break;
    }
}
        
void repaint_crash(HWND hDlg)
{
    out_smart = 1;
    sprintf(string,"%0.2f",SD->z_limit_percent);
    SetDlgItemText(hDlg,CRASH_TOLERANCE,string);
    out_smart = 0;
}

void repaint_digital_max(HWND hDlg)
{
    out_smart = 1;
    sprintf(string,"%d",SD->digital_feedback_max);
    SetDlgItemText(hDlg,DIGITAL_MAX_TRY,string);
    out_smart = 0;
}

void repaint_sample_bias(HWND hDlg)
{
	out_smart = 1;
	sample_bias = min(max(sample_bias,sample_bias_min),sample_bias_max);
//    SetDlgItemInt(hDlg,SAMPLE_BIAS_BITS_EDIT,sample_bias,FALSE);
	sprintf(string,"%u",sample_bias);
    SetDlgItemText(hDlg,SAMPLE_BIAS_BITS_EDIT,string);
    SetScrollPos(GetDlgItem(hDlg,SAMPLE_BIAS_SCROLL),SB_CTL,sample_bias,TRUE);
#ifdef DAC16
	sprintf(string,"%.5f",dtov_bias(sample_bias,get_range(sample_bias_ch)));
#else
	sprintf(string,"%.5f",dtov(sample_bias,get_range(sample_bias_ch)));
#endif
    SetDlgItemText(hDlg,SAMPLE_BIAS_VOLTS_EDIT,string);
	out_smart = 0;
}

// Begin of Mod 11, Bias_RampTo
void repaint_bias_rampto(HWND hDlg)
{
  sprintf(string, "%d", bias_rampto_speed1);
  SetDlgItemText(hDlg, Bias_RampToSpeed_Edit, string);
  sprintf(string, "%d", bias_rampto_speed2);
  SetDlgItemText(hDlg, Bias_RampToSpeed_Edit2, string);
  sprintf(string, "%d", bias_rampto_speed3);
  SetDlgItemText(hDlg, Bias_RampToSpeed_Edit3, string);
  sprintf(string, "%d", bias_rampto_speed4);
  SetDlgItemText(hDlg, Bias_RampToSpeed_Edit4, string);
}

void repaint_bias_target(HWND hDlg)
{
  sprintf(string, "%.5f", bias_target1);
  SetDlgItemText(hDlg, Bias_RampTo_Edit, string);
  sprintf(string, "%.5f", bias_target2);
  SetDlgItemText(hDlg, Bias_RampTo_Edit2, string);
  sprintf(string, "%.5f", bias_target3);
  SetDlgItemText(hDlg, Bias_RampTo_Edit3, string);
  sprintf(string, "%.5f", bias_target4);
  SetDlgItemText(hDlg, Bias_RampTo_Edit4, string);

}
// End of Mod 11, Bias_RampTo

// Begin of Mod 11, Current_RampTo
void repaint_current_rampto(HWND hDlg)
{
  sprintf(string, "%d", current_rampto_speed1);
  SetDlgItemText(hDlg, Current_RampToSpeed_Edit, string);
  sprintf(string, "%d", current_rampto_speed2);
  SetDlgItemText(hDlg, Current_RampToSpeed_Edit2, string);
  sprintf(string, "%d", current_rampto_speed3);
  SetDlgItemText(hDlg, Current_RampToSpeed_Edit3, string);
  sprintf(string, "%d", current_rampto_speed4);
  SetDlgItemText(hDlg, Current_RampToSpeed_Edit4, string);
}

void repaint_current_target(HWND hDlg)
{
  sprintf(string, "%.5f", current_target1);
  SetDlgItemText(hDlg, Current_RampTo_Edit, string);
  sprintf(string, "%.5f", current_target2);
  SetDlgItemText(hDlg, Current_RampTo_Edit2, string);
  sprintf(string, "%.5f", current_target3);
  SetDlgItemText(hDlg, Current_RampTo_Edit3, string);
  sprintf(string, "%.5f", current_target4);
  SetDlgItemText(hDlg, Current_RampTo_Edit4, string);
}
// End of Mod 11, Current_RampTo

static void repaint_read_ch(HWND hDlg)
{
  if(SD->sequence[current_step].type == READ_TYPE)
  {
	EnableWindow(GetDlgItem(hDlg,PRESCAN_RS_INCH),TRUE);
	sprintf(string,"%d",SD->sequence[current_step].in_ch);
    SetDlgItemText(hDlg,PRESCAN_RS_INCH,string);
  }
  else
  {
	EnableWindow(GetDlgItem(hDlg,PRESCAN_RS_INCH),FALSE);
    SetDlgItemText(hDlg,PRESCAN_RS_INCH,"n/a");
  }
}
    
static void repaint_sequence_num(HWND hDlg)
{
    sprintf(string,"%d of %d",current_step + 1,SD->total_steps);
    SetDlgItemText(hDlg,PRESCAN_SEQUENCE_NUMBER,string);

}

static void repaint_waits(HWND hDlg)
{
  if(SD->sequence[current_step].type == WAIT_TYPE)
  {
	EnableWindow(GetDlgItem(hDlg,PRESCAN_RS_WAIT),TRUE);
	sprintf(string,"%d",SD->sequence[current_step].wait);
    SetDlgItemText(hDlg,PRESCAN_RS_WAIT,string);
  }
  else
  {
	EnableWindow(GetDlgItem(hDlg,PRESCAN_RS_WAIT),FALSE);
    SetDlgItemText(hDlg,PRESCAN_RS_WAIT,"n/a");
  }

/* OLD READ SEQUENCE
    sprintf(string,"%d",SDR.wait1);
    SetDlgItemText(hDlg,PRESCAN_WAIT_1,string);
    sprintf(string,"%d",SDR.wait2);
    SetDlgItemText(hDlg,PRESCAN_WAIT_2,string);
    sprintf(string,"%d",SDR.wait3);
    SetDlgItemText(hDlg,PRESCAN_WAIT_3,string);
    sprintf(string,"%d",SDR.wait4);
    SetDlgItemText(hDlg,PRESCAN_WAIT_4,string);
*/
}    

static void repaint_num_samples(HWND hDlg)
{
  if(SD->sequence[current_step].type == READ_TYPE)
  {
	EnableWindow(GetDlgItem(hDlg,PRESCAN_RS_NUM),TRUE);
	sprintf(string,"%d",SD->sequence[current_step].num_reads);
    SetDlgItemText(hDlg,PRESCAN_RS_NUM,string);
  }
  else
  {
	EnableWindow(GetDlgItem(hDlg,PRESCAN_RS_NUM),FALSE);
    SetDlgItemText(hDlg,PRESCAN_RS_NUM,"n/a");
  }
}

static void repaint_ramp_bias(HWND hDlg)
{
  if(SD->sequence[current_step].type == STEP_OUTPUT_TYPE)
  {
	EnableWindow(GetDlgItem(hDlg,PRESCAN_RS_OUTCH),TRUE);
	EnableWindow(GetDlgItem(hDlg,PRESCAN_RS_SCROLL),TRUE);
	EnableWindow(GetDlgItem(hDlg,PRESCAN_RS_VOLTS),TRUE);
	EnableWindow(GetDlgItem(hDlg,PRESCAN_RS_BITS),TRUE);
	sprintf(string,"%d",SD->sequence[current_step].out_ch);
    SetDlgItemText(hDlg,PRESCAN_RS_OUTCH,string);
    SetScrollPos(GetDlgItem(hDlg,PRESCAN_RS_SCROLL),SB_CTL,
		         SD->sequence[current_step].step_bias,TRUE);
	sprintf(string,"%f",dtov(SD->sequence[current_step].step_bias,
			get_range(SD->sequence[current_step].out_ch)));
    SetDlgItemText(hDlg,PRESCAN_RS_VOLTS,string);
	sprintf(string,"%d",SD->sequence[current_step].step_bias);
    SetDlgItemText(hDlg,PRESCAN_RS_BITS,string);
  }
  else
  {
	EnableWindow(GetDlgItem(hDlg,PRESCAN_RS_OUTCH),FALSE);
	EnableWindow(GetDlgItem(hDlg,PRESCAN_RS_SCROLL),FALSE);
	EnableWindow(GetDlgItem(hDlg,PRESCAN_RS_VOLTS),FALSE);
	EnableWindow(GetDlgItem(hDlg,PRESCAN_RS_BITS),FALSE);
    SetDlgItemText(hDlg,PRESCAN_RS_OUTCH,"n/a");
    SetDlgItemText(hDlg,PRESCAN_RS_VOLTS,"n/a");
    SetDlgItemText(hDlg,PRESCAN_RS_BITS,"n/a");
  }
}

static void repaint_rs_state(HWND hDlg)
{
  if((SD->sequence[current_step].type == FEEDBACK_TYPE) ||
	 (SD->sequence[current_step].type == DITHER0_TYPE) ||
	 (SD->sequence[current_step].type == DITHER1_TYPE) )
  {
	EnableWindow(GetDlgItem(hDlg,PRESCAN_RS_ON),TRUE);
	EnableWindow(GetDlgItem(hDlg,PRESCAN_RS_OFF),TRUE);
    if(SD->sequence[current_step].state == 1)
	{
	  CheckDlgButton(hDlg,PRESCAN_RS_OFF,0);
      CheckDlgButton(hDlg,PRESCAN_RS_ON,1);
	}
	else
	{
	  CheckDlgButton(hDlg,PRESCAN_RS_OFF,1);
      CheckDlgButton(hDlg,PRESCAN_RS_ON,0);
	}
  }
  else
  {
	EnableWindow(GetDlgItem(hDlg,PRESCAN_RS_ON),FALSE);
	EnableWindow(GetDlgItem(hDlg,PRESCAN_RS_OFF),FALSE);
  }
}

static void repaint_angs_calibration(HWND hDlg)
{
    sprintf(string,"%.3f",a_per_v);
    SetDlgItemText(hDlg,PRESCAN_A_PER_V,string);
    sprintf(string,"%.3f",z_a_per_v);
    SetDlgItemText(hDlg,PRESCAN_Z_A_PER_V,string);
}

static void repaint_prescan_temp(HWND hDlg)
{
  sprintf(string,"%0.2f",data->temperature);
  SetDlgItemText(hDlg,PRESCAN_TEMP,string);
}

// Begin of Mod 11, Bias_RampTo
void sleep_ms(unsigned wait_ms )
{
   clock_t goal = clock();
   goal += (clock_t) wait_ms * CLOCKS_PER_SEC / 1000;
   while( goal > clock() )
      ;
}
// End of Mod 11, Bias_RampTo

// ******************************************************************************************
//		O L D * O L D * O L D * O L D * O L D * O L D * O L D * O L D * O L D * O L D
// ******************************************************************************************


#ifdef OLD

int isdir_old(char *name)
{
    DIR *dirp;
            
    dirp = opendir(name);
    if(dirp == NULL) {
        return 0;
    } 
    else
    {
        closedir(dirp);
        return 1;
    }
}

int auto_z_below_old(int target)
{
    unsigned int z,old_z;
    int cross_over;
    
    if(feedback)
    {
      scan_z = dac_data[z_offset_ch];
      dio_in_ch(zi_ch);
      dio_in_data(&z);
      if(z >= target)
        cross_over = 1;
      else
        cross_over = 0;
      while(cross_over < 2 && (z >= target || scan_z < (MAX - 1))
             && (z < target || scan_z > 0))
      {
        old_z = z;
        dio_in_data(&z);
        if(z < target && old_z >= target)
          cross_over++;
        if(z >= target && old_z < target)
          cross_over++;
        if(cross_over < 2)
        {
          if(z < target)
            scan_z++;
          else
            scan_z--;
          move_to_timed(z_offset_ch,dac_data[z_offset_ch],scan_z,10);
          switch(scan_scale*10+scan_freq)
          {
            case 0:
            case 1:
            case 2:
              dio_start_clock(10000);
              dio_wait_clock();
              break;
            case 10:
            case 11:
            case 12:
              dio_start_clock(10000);
              dio_wait_clock();
              break;
            case 20:
            case 21:
            case 22:
              dio_start_clock(10000);
              dio_wait_clock();
              break;
          }
          dac_data[z_offset_ch] = scan_z;
		}
      }
    }
    else
    {
      mprintf("Feedback is OFF!!");
      return 0;
    }
/*
    sprintf(string,"wanted %d, got to %d",target,z);
    MessageBox(GetFocus(),string,"Info",MB_OKCANCEL);
*/
    return(scan_z != MAX - 1 && scan_z != 0); /*success if not railed*/
}     

int auto_z_above_old(int target)
{
    unsigned int z,old_z;
    int cross_over;

    if(feedback)
    {            
      scan_z=dac_data[z_offset_ch];
      dio_in_ch(zi_ch);
      dio_in_data(&z);
      if(z <= target)
        cross_over = 1;
      else
        cross_over = 0;
      while(cross_over < 2 && (z >= target || scan_z < (MAX - 1))
             && (z < target || scan_z > 0))
      {
        old_z = z;
        dio_in_data(&z);
        if(z < target && old_z >= target)
          cross_over++;
        if(z >= target && old_z < target)
          cross_over++;
        if(cross_over < 2)
        {
          if(z < target)
            scan_z++;
          else
            scan_z--;
          move_to_timed(z_offset_ch,dac_data[z_offset_ch],scan_z,10);
          switch(scan_scale*10+scan_freq)
          {
            case 0:
            case 1:
            case 2:
              dio_start_clock(10000);
              dio_wait_clock();
              break;
            case 10:
            case 11:
            case 12:
              dio_start_clock(10000);
              dio_wait_clock();
              break;
            case 20:
              dio_start_clock(10000);
              dio_wait_clock();
              break;
            case 21:
            case 22:
              dio_start_clock(10000);
              dio_wait_clock();
              break;
          }
          dac_data[z_offset_ch] = scan_z;
        }
      }
    }
    else
    {
        mprintf("Feedback is OFF!!");
        return 0;
    }
        
/*
    sprintf(string,"wanted %d, got to %d",target,z);
    MessageBox(GetFocus(),string,"Info",MB_OKCANCEL);
*/
    return(scan_z != MAX - 1 && scan_z!=0); /*success if not railed*/
}    

static void add_sequence_old()
{
    READ_SEQ *temp;
    int i;
    
    temp = (READ_SEQ *)malloc(sizeof(READ_SEQ) * (SD->read_seq_num + 1));
    for(i = 0;i < SD->read_seq_num;i++)
    {
        copy_seq(temp + i,SD->read_seq + i);
    }
    set_dumb_seq(temp + SD->read_seq_num);
    free(SD->read_seq);
    SD->read_seq = temp;
    SD->read_seq_num++;
}

static void insert_sequence_old()
{
    READ_SEQ *temp;
    int i;
    
    temp = (READ_SEQ *)malloc(sizeof(READ_SEQ) * (SD->read_seq_num + 1));
    for(i = 0;i < SD->current_read_seq;i++)
    {
        copy_seq(temp + i,SD->read_seq + i);
    }
    set_dumb_seq(temp + SD->current_read_seq);
    for(i = SD->current_read_seq;i < SD->read_seq_num;i++)
    {
        copy_seq(temp + i + 1,SD->read_seq + i);
    }
    
    free(SD->read_seq);
    SD->read_seq = temp;
    SD->read_seq_num++;
}

static void del_sequence_old()
{
    READ_SEQ *temp;
    int i;
    
    temp = (READ_SEQ *)malloc(sizeof(READ_SEQ) * (SD->read_seq_num - 1));
    for(i = 0;i < SD->current_read_seq;i++)
    {
        copy_seq(temp + i,SD->read_seq + i);
    }
    for(i = SD->current_read_seq + 1;i < SD->read_seq_num;i++)
    {
        copy_seq(temp + i - 1,SD->read_seq + i);
    }
    
    free(SD->read_seq);
    SD->read_seq = temp;
    SD->read_seq_num--;
    if (SD->current_read_seq >= SD->read_seq_num) 
        SD->current_read_seq = SD->read_seq_num - 1;
    
}

void repaint_scan_num(HWND hDlg)
{
//    scan_num = min(max(scan_num,scan_num_min),scan_num_max);
    SetDlgItemInt(hDlg,PRESCAN_RS_NUM,SDR.num_samples,FALSE);
//    SetScrollPos(GetDlgItem(hDlg,SCAN_NUM_SCROLL),SB_CTL,SDR.num_samples,TRUE);
}


#endif