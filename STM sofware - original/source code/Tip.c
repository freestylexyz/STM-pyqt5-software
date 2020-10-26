#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include <dos.h>
#include <string.h>
#include <direct.h>
#include "stm.h"
#include "common.h" 
#include "dio.h"
#include "serial.h"
//#include "coarse.h"
#include "tip.h"
#include "pre_scan.h"
#include "clock.h"
#include "file.h"

extern datadef *data;
extern double i_set;
extern int z_offset_ch;
extern unsigned int dac_data[];
extern unsigned int out1;
extern unsigned int sample_bias;
extern int x_range,z_range;
extern unsigned int bit16;

extern HANDLE hInst;
extern BOOL FAR PASCAL IOCtrlDlg(HWND,unsigned,WPARAM,LPARAM);
extern BOOL FAR PASCAL OscillDlg(HWND,unsigned,WPARAM,LPARAM);
extern char *current_dir_stm,*current_file_stm,*initial_dir_stm;
extern int file_stm_count;
extern char string[];

unsigned int tunneling_min = TUNNELING_MIN;	// input current (bits) that triggers end of approach
unsigned int tip_number; // # of steps for step up/down
unsigned int tip_accel;
unsigned int tip_delay;
unsigned int tip_x_step;
unsigned int tip_zo_step;
int num_giant_steps = 3;
int baby_step_size = BABY_STEP_SIZE;
/////////////////////////////////////02132012////////////////////////////////////////////
static int tip_translate_sample_direction = TIP_TRANSLATE_SW;
/////////////////////////////////////02132012////////////////////////////////////////////
int tip_status = TIP_IDLE;
SAMPLELIST tip_sample_list,tip_dosed_list;
static int first_time = 1;
char cur_sample[LISTBOX_STR_MAX];
char cur_dosed[LISTBOX_STR_MAX];
static int out_smart = 0;

static void repaint_bias_current(HWND);
void repaint_tunneling_min(HWND);
void repaint_giant(HWND);
void repaint_baby(HWND);
void tip_approach_disable(HWND);
void tip_approach_enable(HWND);
void repaint_tip_translate_sample(HWND);//Shaowei

BOOL FAR PASCAL TipDlg(HWND,unsigned,WPARAM,LPARAM);
// Tip Approach Dialog. Used to perform coarse approach as well as "blind"
// steps up and steps down. Other features are included that are useful at
// the outset of the experiment: button for io controls, sample select listbox,
// temperature, etc.

BOOL FAR PASCAL TipDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{
  int id;
  int i,mini,maxi,delt,ddelt;
//  unsigned int bias_min,bias_max;
//  MSG Message2;
//  int bias_range;
  int done = 0;
  static FARPROC lpfnDlgProc;
  
  switch(Message) {
    case WM_INITDIALOG:
      pre_tip();
      out_smart = 1;
      EnableWindow(GetDlgItem(hDlg,TIP_STOP),FALSE);
      tip_update_status(hDlg,"Idle.",-1);
      SetScrollRange(GetDlgItem(hDlg,TIP_TUNNEL_SCROLL),SB_CTL,TUNNELING_MIN_MIN,TUNNELING_MIN_MAX,FALSE);
      SetScrollRange(GetDlgItem(hDlg,TIP_ACCEL_SCROLL),SB_CTL,tip_accel_min,tip_accel_max,FALSE);
      SetScrollRange(GetDlgItem(hDlg,TIP_NUMBER_SCROLL),SB_CTL,tip_number_min,tip_number_max,FALSE);
      SetScrollRange(GetDlgItem(hDlg,TIP_DELAY_SCROLL),SB_CTL,tip_delay_min,tip_delay_max,FALSE);
      SetScrollRange(GetDlgItem(hDlg,TIP_X_STEP_SCROLL),SB_CTL,tip_x_step_min,tip_x_step_max,FALSE);
      SetScrollRange(GetDlgItem(hDlg,TIP_ZO_STEP_SCROLL),SB_CTL,tip_zo_step_min,tip_zo_step_max,FALSE);
      SetScrollRange(GetDlgItem(hDlg,TIP_GIANT_SCROLL),SB_CTL,TIP_GIANT_MIN,TIP_GIANT_MAX,FALSE);
      SetScrollRange(GetDlgItem(hDlg,TIP_BABY_SCROLL),SB_CTL,TIP_BABY_MIN,TIP_BABY_MAX,FALSE);
      SetDlgItemText(hDlg,TIP_ACCEL_EDIT,gcvt((double)tip_accel / 10,5,string));
      SetDlgItemInt(hDlg,TIP_NUMBER_EDIT,tip_number,FALSE);
      SetDlgItemInt(hDlg,TIP_DELAY_EDIT,tip_delay,FALSE);
      SetDlgItemText(hDlg,TIP_X_STEP_VOLTS_EDIT,gcvt(dtov(ZERO + tip_x_step,x_range),5,string));
      SetDlgItemInt(hDlg,TIP_X_STEP_BITS_EDIT,tip_x_step,FALSE);
      SetDlgItemInt(hDlg,TIP_ZO_STEP_BITS_EDIT,tip_zo_step,FALSE);
      SetDlgItemText(hDlg,TIP_ZO_STEP_VOLTS_EDIT,gcvt(dtov(ZERO + tip_zo_step,z_range),5,string));
      SetDlgItemText(hDlg,TIP_TUNNEL_VOLTS,gcvt(in_dtov((float)tunneling_min + IN_ZERO),5,string));
      SetDlgItemInt(hDlg,TIP_TUNNEL_BITS,tunneling_min,FALSE);
      SetDlgItemText(hDlg,TIP_SAMPLE_TYPE_TEXT,data->sample_type.ptr);
      SetDlgItemText(hDlg,TIP_DOSED_WITH,data->dosed_type.ptr); 
/////////////////////////////////////02132012////////////////////////////////////////////
      CheckDlgButton(hDlg,TIP_TRANSLATE_SAMPLE_CHECK,0);
	  tip_translate_sample_checker_disable(hDlg);
      repaint_tip_translate_sample(hDlg);
     
/////////////////////////////////////02132012////////////////////////////////////////////
      read_sample_list(&tip_sample_list,SAMPLES_INI,"",PRESCAN_UNKNOWN_SAMPLE);
      read_sample_list(&tip_dosed_list,DOSED_INI,"",PRESCAN_UNKNOWN_DOSED);
      init_listbox(hDlg,TIP_SAMPLE_LIST,&(tip_sample_list.listbox));
      init_listbox(hDlg,TIP_DOSED_LIST,&(tip_dosed_list.listbox));

      if(first_time)
      {
		first_time = 0;
        SendDlgItemMessage(hDlg,TIP_SAMPLE_LIST,CB_SETCURSEL, 
                (WPARAM)SendDlgItemMessage(hDlg,TIP_SAMPLE_LIST, 
                CB_FINDSTRINGEXACT,0,(LPARAM)PRESCAN_UNKNOWN_SAMPLE),
                (LPARAM)0);
        SendDlgItemMessage(hDlg,TIP_DOSED_LIST,CB_SETCURSEL, 
                (WPARAM)SendDlgItemMessage(hDlg,TIP_DOSED_LIST, 
                CB_FINDSTRINGEXACT,0,(LPARAM)PRESCAN_UNKNOWN_DOSED),
                (LPARAM)0);
        strcpy(cur_sample,tip_sample_list.listbox.str[0]);
        strcpy(cur_dosed,tip_dosed_list.listbox.str[0]);
      }
      else
      {
        SendDlgItemMessage(hDlg,TIP_SAMPLE_LIST,CB_SETCURSEL, 
                (WPARAM)SendDlgItemMessage(hDlg,TIP_SAMPLE_LIST, 
                CB_FINDSTRINGEXACT,0,(LPARAM)cur_sample),
                (LPARAM)0);
        SendDlgItemMessage(hDlg,TIP_DOSED_LIST,CB_SETCURSEL, 
                (WPARAM)SendDlgItemMessage(hDlg,TIP_DOSED_LIST, 
                CB_FINDSTRINGEXACT,0,(LPARAM)cur_dosed),
                (LPARAM)0);
      }

      SetFocus(GetDlgItem(hDlg,TIP_EXIT));
      sprintf(string,"%0.2f",data->temperature);
      SetDlgItemText(hDlg,PRESCAN_TEMP,string);
      repaint_bias_current(hDlg);
      repaint_accel(hDlg,tip_accel);
      repaint_number(hDlg,tip_number);
      repaint_delay(hDlg,tip_delay);
      repaint_x_step(hDlg,tip_x_step);
      repaint_zo_step(hDlg,tip_zo_step);
      repaint_giant(hDlg);
      repaint_baby(hDlg);
      out_smart = 0;
      break;

    case WM_HSCROLL:
      id = getscrollid();
      switch(id) {
		case TIP_ACCEL_SCROLL:
			i = (int)tip_accel;
			mini = (int)tip_accel_min;
			maxi = (int)tip_accel_max;
			delt = (int)tip_accel_delt;
			ddelt = (int)tip_accel_ddelt;
			break;
		case TIP_NUMBER_SCROLL:
			i = (int)tip_number;
			mini = (int)tip_number_min;
			maxi = (int)tip_number_max;
			delt = (int)tip_number_delt;
			ddelt = (int)tip_number_ddelt;
			break;
		case TIP_DELAY_SCROLL:
			i = (int)tip_delay;
			mini = (int)tip_delay_min;
			maxi = (int)tip_delay_max;
			delt = (int)tip_delay_delt;
			ddelt = (int)tip_delay_ddelt;
			break;
		case TIP_TUNNEL_SCROLL:
			i = (int)tunneling_min;
			mini = (int)TUNNELING_MIN_MIN;
			maxi = (int)TUNNELING_MIN_MAX;
			delt = (int)TUNNELING_MIN_DELT;
			ddelt = (int)TUNNELING_MIN_DDELT;
			break;
		case TIP_X_STEP_SCROLL:
			i= (int)tip_x_step;
			mini = (int)tip_x_step_min;
			maxi = (int)tip_x_step_max;
			delt = (int)tip_x_step_delt;
			ddelt = (int)tip_x_step_ddelt;
			break;
		case TIP_ZO_STEP_SCROLL:
			i = (int)tip_zo_step;
			mini = (int)tip_zo_step_min;
			maxi = (int)tip_zo_step_max;
			delt = (int)tip_zo_step_delt;
			ddelt = (int)tip_zo_step_ddelt;
			break;
		case TIP_GIANT_SCROLL:
			i = (int)num_giant_steps;
			mini = (int)TIP_GIANT_MIN;
			maxi = (int)TIP_GIANT_MAX;
			delt = (int)TIP_GIANT_DELT;
			ddelt = (int)TIP_GIANT_DDELT;
			break;
		case TIP_BABY_SCROLL:
			i = (int)baby_step_size;
			mini = (int)TIP_BABY_MIN;
			maxi = (int)TIP_BABY_MAX;
			delt = (int)TIP_BABY_DELT;
			ddelt = (int)TIP_BABY_DDELT;
			break;
      }
      switch(getscrollcode()) {
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
      }
      i = min(max(i,mini),maxi);
      if(!out_smart) 
      {
        out_smart = 1;
        switch(id) {
            case TIP_ACCEL_SCROLL:
                tip_accel = (unsigned int)i;
                SetDlgItemText(hDlg,TIP_ACCEL_EDIT,gcvt((double)tip_accel / 10,5,string));
                repaint_accel(hDlg,tip_accel);
                break;
            case TIP_NUMBER_SCROLL:
                tip_number = (unsigned int)i;
                SetDlgItemInt(hDlg,TIP_NUMBER_EDIT,tip_number,FALSE);
                repaint_number(hDlg,tip_number);
                break;
            case TIP_DELAY_SCROLL:
                SetDlgItemInt(hDlg,TIP_DELAY_EDIT,tip_delay,FALSE);
                tip_delay = (unsigned int)i;
                repaint_delay(hDlg,tip_delay);
                break;
            case TIP_TUNNEL_SCROLL:
                tunneling_min = (unsigned int)i;
                SetDlgItemText(hDlg,TIP_TUNNEL_VOLTS,gcvt(in_dtov((float)tunneling_min + IN_ZERO),5,string));
                SetDlgItemInt(hDlg,TIP_TUNNEL_BITS,tunneling_min,FALSE);
                repaint_tunneling_min(hDlg);
                break;
            case TIP_X_STEP_SCROLL:
                tip_x_step = (unsigned int)i;
                SetDlgItemText(hDlg,TIP_X_STEP_VOLTS_EDIT,gcvt(dtov(ZERO + tip_x_step,x_range),5,string));
                SetDlgItemInt(hDlg,TIP_X_STEP_BITS_EDIT,tip_x_step,FALSE);
                repaint_x_step(hDlg,tip_x_step);
                break;
            case TIP_ZO_STEP_SCROLL:
                tip_zo_step = (unsigned int)i;
                SetDlgItemInt(hDlg,TIP_ZO_STEP_BITS_EDIT,tip_zo_step,FALSE);
                SetDlgItemText(hDlg,TIP_ZO_STEP_VOLTS_EDIT,gcvt(dtov(ZERO + tip_zo_step,z_range),5,string));
                repaint_zo_step(hDlg,tip_zo_step);
                break;
            case TIP_GIANT_SCROLL:
                num_giant_steps = (unsigned int)i;
                repaint_giant(hDlg);
                break;
            case TIP_BABY_SCROLL:
                baby_step_size = (unsigned int)i;
                repaint_baby(hDlg);
                break;
        }
        out_smart = 0;
      }
      break;

	case WM_CHAR:
	case WM_SYSCHAR:
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_DEADCHAR:
    case WM_SYSDEADCHAR:
        switch(LOWORD(wParam))
        {
            case VK_ESCAPE:
                if(tip_status != TIP_IDLE)
                {
                    tip_status = TIP_STOP_NOW;
                }
                break;    
        }        
        break;

	case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case PRESCAN_TEMP:
            GetDlgItemText(hDlg,PRESCAN_TEMP,string,15);
            data->temperature  = (float)atof(string);
            break;

		// sample list box
        case TIP_SAMPLE_LIST:
            switch(getcombomsg())
            {
                case CBN_SELCHANGE:
                    i = SendDlgItemMessage(hDlg,TIP_SAMPLE_LIST,CB_GETCURSEL,0,0);
                    strcpy(data->sample_type.ptr,tip_sample_list.name[ 
                        tip_sample_list.listbox.index[i]]);
                    strcpy(cur_sample,tip_sample_list.listbox.str[ 
                        tip_sample_list.listbox.index[i]]);

                    SetDlgItemText(hDlg,TIP_SAMPLE_TYPE_TEXT,data->sample_type.ptr);
                    data->sample_type.size=strlen(data->sample_type.ptr);

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
					// create directory for this sample, if necessary
                    if(!isdir(current_dir_stm))
                    {
                        sprintf(string,"%s doesn't exist!\nCreate it?",
                            current_dir_stm);
                        if (MessageBox(hDlg,string,
                            "Notice!",MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
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

		// dosed with list box
		case TIP_DOSED_LIST:
            switch(getcombomsg())
            {
                case CBN_SELCHANGE:
                    i = SendDlgItemMessage(hDlg,TIP_DOSED_LIST,CB_GETCURSEL,0,0);
                    strcpy(data->dosed_type.ptr,tip_dosed_list.name[ 
                        tip_dosed_list.listbox.index[i]]);

                    strcpy(cur_dosed,tip_dosed_list.listbox.str[ 
                        tip_dosed_list.listbox.index[i]]);
                    SetDlgItemText(hDlg,TIP_DOSED_WITH,data->dosed_type.ptr);
                    data->dosed_type.size = strlen(data->dosed_type.ptr);

                    strcpy(current_dir_stm,initial_dir_stm);
                    if (strcmp(cur_sample,PRESCAN_UNKNOWN_SAMPLE))
                    {
                        strcat(current_dir_stm,"\\");
                        strcat(current_dir_stm,cur_sample);
                        if (strcmp(cur_dosed,PRESCAN_UNKNOWN_DOSED))
                        {
                            strcat(current_dir_stm,"\\");
                            strcat(current_dir_stm,cur_dosed);
                        }
                    }
					// create directory for this sample, if necessary
                    if(!isdir(current_dir_stm))
                    {
                        sprintf(string,"%s doesn't exist!\nCreate it?",
                            current_dir_stm);
                        if(MessageBox(hDlg,string,
                            "Notice!",MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
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

/* 
		// Bias square wave available in IO Controls dialog.
		// Bias square wave is now handled via serial connection to DSP, not
		// parallel dio card output!
		case BIAS_SQUARE_WAVE:
			bias_range = get_range(sample_bias_ch);
			bias_min = vtod(BIAS_SQUARE_MIN,bias_range);
			bias_max = vtod(BIAS_SQUARE_MAX,bias_range);
			done = 0;
			while(!done)
			{
				bias(bias_min);
				dio_start_clock(BIAS_SQUARE_DELAY);
				PeekMessage(&Message2,hDlg,NULL,NULL,PM_REMOVE);
				if(LOWORD(Message2.wParam) == VK_ESCAPE)
				{
					sprintf(string,"Sqaure Wave terminated.");
					MessageBox(hDlg,string,"Warning",MB_ICONEXCLAMATION);
					done = 1;
				}
				dio_wait_clock();
				bias(bias_max);
				dio_start_clock(BIAS_SQUARE_DELAY);
				PeekMessage(&Message2,hDlg,NULL,NULL,PM_REMOVE);
				if(LOWORD(Message2.wParam) == VK_ESCAPE)
				{
					sprintf(string,"Sqaure Wave terminated.");
					MessageBox(hDlg,string,"Warning",MB_ICONEXCLAMATION);
					done = 1;
				}
				dio_wait_clock();
			}
			bias(sample_bias);
			break;
*/

		// Action!
		case TIP_RETRACT:
			//retract(TRUE);
			retract_serial(TRUE);
			break;
		case TIP_UNRETRACT:
			//retract(FALSE);
			retract_serial(FALSE);
			break;
		case TIP_APPROACH:
			//dio_init();
			dio_init_serial(); /* paranoia */
			tip_approach(hDlg,tip_accel);
			break;
		case TIP_STOP:
			if(tip_status != TIP_IDLE)
			{
				tip_status = TIP_STOP_NOW;
			}
			break;    
		case TIP_REINIT:
			//dio_init(); 
			dio_init_serial(); 
			break;
		case TIP_STEP_UP:
			//dio_init();
			dio_init_serial(); /* paranoia */
			tip_step(hDlg,UP);
			break;
		case TIP_STEP_DOWN:
			//dio_init();
			dio_init_serial(); /* paranoia */
			i = 1;
			if(tip_accel != TIP_UP_ACCEL_DEF || tip_x_step !=TIP_UP_X_STEP_DEF ||
				tip_zo_step != TIP_UP_Z_STEP_DEF)
			{
				if(MessageBox(hDlg,"The settings are not defaults! Continue with step down?",
                    "Warning!",MB_YESNO | MB_ICONEXCLAMATION) == IDNO) 
					i = 0;
			}
			if(!(dac_data[feedback_ch] & feedback_bit))
			{
				if(MessageBox(hDlg,"Feedback is off! Continue with step down?",
                    "Warning!",MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
					i = 0;
			}
			if(i) tip_step(hDlg,DOWN);
			break;
/////////////////////////////////////02132012////////////////////////////////////////////
        case TIP_TRANSLATE_SAMPLE_CHECK:
//			tip_translate_sample_check = IsDlgButtonChecked(hDlg,TIP_TRANSLATE_SAMPLE_CHECK);
			if(IsDlgButtonChecked(hDlg,TIP_TRANSLATE_SAMPLE_CHECK))
			{
				tip_translate_sample_checker_enable(hDlg);
			}
			else
			{
				tip_translate_sample_checker_disable(hDlg);
			}
			break;
		case TIP_TRANSLATE_NE:
			tip_translate_sample_direction = TIP_TRANSLATE_NE;
			repaint_tip_translate_sample(hDlg);
			break;
		case TIP_TRANSLATE_SW:
			tip_translate_sample_direction = TIP_TRANSLATE_SW;
			repaint_tip_translate_sample(hDlg);
			break;
		case TIP_TRANSLATE_SAMPLE:
			dio_init_serial(); /* paranoia */
			tip_translate_sample(hDlg,tip_translate_sample_direction);
			break;
/////////////////////////////////////02132012////////////////////////////////////////////
		// Tip Approach parameters
		case TIP_ACCEL_EDIT:
			if(!out_smart)
			{
				out_smart = 1;
				GetDlgItemText(hDlg,TIP_ACCEL_EDIT,string,6);
				i = (int)(atof(string) * 10);
				i = min(max(i,tip_accel_min),tip_accel_max);
				tip_accel = i;
				repaint_accel(hDlg,tip_accel);
				out_smart = 0;
			}
			break;
		case TIP_NUM_GIANT:
			if(!out_smart)
			{
				out_smart = 1;
				GetDlgItemText(hDlg,TIP_NUM_GIANT,string,6);
				i = (int)atoi(string);
				i = min(max(i,TIP_GIANT_MIN),TIP_GIANT_MAX);
				num_giant_steps = i;
				SetScrollPos(GetDlgItem(hDlg,TIP_GIANT_SCROLL),SB_CTL,num_giant_steps,TRUE);
				out_smart = 0;
			}
			break;
		case TIP_BABY_SIZE:
			if(!out_smart)
			{
				out_smart = 1;
				GetDlgItemText(hDlg,TIP_BABY_SIZE,string,6);
				i = (int) atoi(string);
				i = min(max(i,TIP_BABY_MIN),TIP_BABY_MAX);
				baby_step_size = i;
				SetScrollPos(GetDlgItem(hDlg,TIP_BABY_SCROLL),SB_CTL,baby_step_size,TRUE);
				out_smart = 0;
			}
			break;
		case TIP_DELAY_EDIT:
			if(!out_smart)
			{
				GetDlgItemText(hDlg,TIP_DELAY_EDIT,string,4);
				i = atoi(string);
				i = min(max(i,tip_delay_min),tip_delay_max);
				tip_delay = (unsigned int)i;
				repaint_delay(hDlg,tip_delay);
			}
			break;
		case TIP_TUNNEL_BITS:
			if(!out_smart)
			{
				out_smart = 1;
				GetDlgItemText(hDlg,TIP_TUNNEL_BITS,string,5);
				i = atoi(string);
				i = min(max(i,TUNNELING_MIN_MIN),TUNNELING_MIN_MAX);
				tunneling_min = (unsigned int)i;
				SetDlgItemText(hDlg,TIP_TUNNEL_VOLTS,gcvt(in_dtov((float)tunneling_min+IN_ZERO),5,string));
				repaint_tunneling_min(hDlg);
				out_smart = 0;
			}
			break;
		case TIP_TUNNEL_VOLTS:
			if(!out_smart)
			{
				out_smart = 1;
				GetDlgItemText(hDlg,TIP_TUNNEL_VOLTS,string,9);
				i = in_vtod(atof(string))-IN_ZERO;
				i = min(max(i,TUNNELING_MIN_MIN),TUNNELING_MIN_MAX);
				tunneling_min = (unsigned int)i;
				SetDlgItemInt(hDlg,TIP_TUNNEL_BITS,tunneling_min,FALSE);
				repaint_tunneling_min(hDlg);
				out_smart = 0;
			}
			break;
		case TIP_X_STEP_BITS_EDIT:
			if(!out_smart)
			{
				out_smart = 1;
				GetDlgItemText(hDlg,TIP_X_STEP_BITS_EDIT,string,5);
				i = atoi(string);
				i = min(max(i,tip_x_step_min),tip_x_step_max);
				tip_x_step = (unsigned int)i;
				SetDlgItemText(hDlg,TIP_X_STEP_VOLTS_EDIT,gcvt(dtov(ZERO + tip_x_step,x_range),5,string));
				repaint_x_step(hDlg,tip_x_step);
				out_smart = 0;
			}
			break;
		case TIP_X_STEP_VOLTS_EDIT:
			if(!out_smart)
			{
				out_smart = 1;
				GetDlgItemText(hDlg,TIP_X_STEP_VOLTS_EDIT,string,9);
				i = vtod(atof(string),x_range) - ZERO;
				i = min(max(i,tip_x_step_min),tip_x_step_max);
				tip_x_step = (unsigned int)i;
				SetDlgItemInt(hDlg,TIP_X_STEP_BITS_EDIT,tip_x_step,FALSE);
				repaint_x_step(hDlg,tip_x_step);
				out_smart = 0;
			}
			break;
		case TIP_ZO_STEP_BITS_EDIT:
			if(!out_smart)
			{
				out_smart = 1;
				GetDlgItemText(hDlg,TIP_ZO_STEP_BITS_EDIT,string,5);
				i = atoi(string);
				i = min(max(i,tip_zo_step_min),tip_zo_step_max);
				tip_zo_step = (unsigned int)i;
				SetDlgItemText(hDlg,TIP_ZO_STEP_VOLTS_EDIT,gcvt(dtov(ZERO + tip_zo_step,z_range),5,string));
				repaint_zo_step(hDlg,tip_zo_step);
				out_smart = 0;
			}
			break;
		case TIP_ZO_STEP_VOLTS_EDIT:
			if(!out_smart)
			{
				out_smart = 1;
				GetDlgItemText(hDlg,TIP_ZO_STEP_VOLTS_EDIT,string,9);
				i = vtod(atof(string),z_range) - ZERO;
				i = min(max(i,tip_zo_step_min),tip_zo_step_max);
				tip_zo_step = (unsigned int)i;
				SetDlgItemInt(hDlg,TIP_ZO_STEP_BITS_EDIT,tip_zo_step,FALSE);
				repaint_zo_step(hDlg,tip_zo_step);
				out_smart = 0;
			}
			break;

		// default Tip Approach parameters (x step,zout step,accleration)
		case TIP_UP_DEFAULTS:
			tip_accel = TIP_UP_ACCEL_DEF;
			tip_x_step = TIP_UP_X_STEP_DEF;
			tip_zo_step = TIP_UP_Z_STEP_DEF;
			repaint_accel(hDlg,tip_accel);
			SetDlgItemText(hDlg,TIP_ACCEL_EDIT,gcvt((double)tip_accel / 10,5,string));
			SetDlgItemText(hDlg,TIP_X_STEP_VOLTS_EDIT,gcvt(dtov(ZERO + tip_x_step,x_range),5,string));
			SetDlgItemText(hDlg,TIP_ZO_STEP_VOLTS_EDIT,gcvt(dtov(ZERO + tip_zo_step,z_range),5,string));
			SetDlgItemInt(hDlg,TIP_X_STEP_BITS_EDIT,tip_x_step,FALSE);
			SetDlgItemInt(hDlg,TIP_ZO_STEP_BITS_EDIT,tip_zo_step,FALSE);
			repaint_x_step(hDlg,tip_x_step);
			repaint_zo_step(hDlg,tip_zo_step);
			tip_number = 5;	// X. Cao, 2004-05-06
			SetDlgItemInt(hDlg,TIP_NUMBER_EDIT,tip_number,FALSE);	// X. Cao, 2004-05-06
			repaint_number(hDlg, tip_number);	// X. Cao, 2004-05-06
			break;
		case TIP_DOWN_DEFAULTS:
			tip_accel = TIP_DOWN_ACCEL_DEF;
			tip_x_step = TIP_DOWN_X_STEP_DEF;
			tip_zo_step = TIP_DOWN_Z_STEP_DEF;
			repaint_accel(hDlg,tip_accel);
			SetDlgItemText(hDlg,TIP_ACCEL_EDIT,gcvt((double)tip_accel / 10,5,string));
			SetDlgItemText(hDlg,TIP_X_STEP_VOLTS_EDIT,gcvt(dtov(ZERO + tip_x_step,x_range),5,string));
			SetDlgItemInt(hDlg,TIP_X_STEP_BITS_EDIT,tip_x_step,FALSE);
			repaint_x_step(hDlg,tip_x_step);
			SetDlgItemInt(hDlg,TIP_ZO_STEP_BITS_EDIT,tip_zo_step,FALSE);
			SetDlgItemText(hDlg,TIP_ZO_STEP_VOLTS_EDIT,gcvt(dtov(ZERO + tip_zo_step,z_range),5,string));
			repaint_zo_step(hDlg,tip_zo_step);
			tip_number = 5;		// X. Cao, 2004-05-06
			SetDlgItemInt(hDlg,TIP_NUMBER_EDIT,tip_number,FALSE);	// X. Cao, 2004-05-06
			repaint_number(hDlg, tip_number);	// X. Cao, 2004-05-06
			break;

		case TIP_NUMBER_EDIT:	// used during STEP UP/DOWN, not in Tip Approach
			if(!out_smart)
			{
				GetDlgItemText(hDlg,TIP_NUMBER_EDIT,string,6);
				i = atoi(string);
				i = min(tip_number_max,max(tip_number_min,i));
				tip_number = (unsigned int)i;
				repaint_number(hDlg,tip_number);
			}
			break;

		case TIP_CONTROLS:
			lpfnDlgProc = MakeProcInstance(IOCtrlDlg,hInst);
			DialogBox(hInst,"IODLG",hDlg,lpfnDlgProc);
			FreeProcInstance(lpfnDlgProc);
			repaint_bias_current(hDlg);
			break;

/*    
		case TIP_OSCILLOSCOPE:
			lpfnDlgProc = MakeProcInstance(OscillDlg, hInst);
			DialogBox(hInst, "OSCILLDLG", hDlg, lpfnDlgProc);
			FreeProcInstance(lpfnDlgProc);
			break;
*/

		case ENTER: 
			out_smart = 1;
			SetDlgItemText(hDlg,TIP_ACCEL_EDIT,gcvt((double)tip_accel / 10,5,string));
			SetDlgItemInt(hDlg,TIP_NUMBER_EDIT,tip_number,FALSE);
			SetDlgItemInt(hDlg,TIP_DELAY_EDIT,tip_delay,FALSE);
			SetDlgItemText(hDlg,TIP_X_STEP_VOLTS_EDIT,gcvt(dtov(ZERO + tip_x_step,x_range),5,string));
			SetDlgItemInt(hDlg,TIP_X_STEP_BITS_EDIT,tip_x_step,FALSE);
			SetDlgItemInt(hDlg,TIP_ZO_STEP_BITS_EDIT,tip_zo_step,FALSE);
			SetDlgItemText(hDlg,TIP_ZO_STEP_VOLTS_EDIT,gcvt(dtov(ZERO + tip_zo_step,z_range),5,string));
			SetDlgItemText(hDlg,TIP_TUNNEL_VOLTS,gcvt(in_dtov((float)tunneling_min + IN_ZERO),5,string));
			SetDlgItemInt(hDlg,TIP_TUNNEL_BITS,tunneling_min,FALSE);
      
			repaint_accel(hDlg,tip_accel);
			repaint_number(hDlg,tip_number);
			repaint_delay(hDlg,tip_delay);
			repaint_x_step(hDlg,tip_x_step);
			repaint_zo_step(hDlg,tip_zo_step);
			repaint_tip_translate_sample(hDlg);
			out_smart = 0;
			break;
		case TIP_EXIT:
			free_sample_list(&tip_sample_list);
			free_sample_list(&tip_dosed_list);
			post_tip();
			EndDialog(hDlg,TRUE);
			return(TRUE);
			break;
	  }
	  break;
  }
  return(FALSE);
}

void tip_approach(HWND hDlg,unsigned int accel)
{
  // performs "true" tip approach, alternating giant steps
  // with baby steps that check for minimum tunneling current
  unsigned int  i = 0;
  unsigned int  xn;		// number of outputs to execute x parabola
  unsigned int  zon;	// number of outputs to execute z outer parabola 
  unsigned int  zn;		// number of outputs to execute baby parabola
  int tunneling = 0;
  unsigned int pass_result = 0;

  // warnings
  if(tip_accel != TIP_DOWN_ACCEL_DEF || tip_x_step !=TIP_DOWN_X_STEP_DEF ||
    tip_zo_step != TIP_DOWN_Z_STEP_DEF)
  {
    if(MessageBox(hDlg,"The settings are not defaults! Continue with approach?",
                "Warning!",MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
        return;
  }
  if(!(dac_data[feedback_ch] & feedback_bit))
  {
    if(MessageBox(hDlg,"Feedback is off! Continue with approach?",
                "Warning!",MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
        return;
  }
        
  // miscellaneous inizialization
  tip_approach_disable(hDlg);
  tip_status = TIP_APPROACH;
  tip_update_status(hDlg,"Starting Approach",0);
  mode_serial(coarse_mode);
  mode_serial(rotate_mode);

  // calculate the x and z outer parabola parameters
	/* Compute size for xstep size x parabola:  t=sqrt(2*sqrt(2)*dist/accel) */
	/* where dist = v*10*(meters_per_output) */
  xn = (unsigned int)ceil(sqrt(2 * sqrt(2) * (dtov(ZERO + tip_x_step / 2,x_range) * METERS_PER_XY_VOLT)
				/ ((double)G * tip_accel / 1000)) / SEC_PER_OUTPUT);
	/* Compute size for zstep size z outer parabola:  t=sqrt(2*dist/accel) */
	/* where dist = v*10*(meters_per_output) */
  zon = (unsigned int)ceil(sqrt(2 * (dtov(ZERO + tip_zo_step / 2,z_range) * 10 * METERS_PER_Z_VOLT)
				/((double)G * tip_accel / 1000)) / SEC_PER_OUTPUT);
	/* Compute size for 50V z outer parabola */
  zn = (unsigned int)ceil(sqrt(2 * (50 * 10 * METERS_PER_Z_VOLT)
				/ ((double)G * accel / 1000)) / SEC_PER_OUTPUT);

  // send the parameters to the dsp
  tip_steps_setup((float)multiplier_fix*(tip_zo_step / 2 / (float)pow(zon,2)),(float)multiplier_fix*(tip_x_step / 2 / (float)pow(xn,2)),
				  (unsigned int)(0.586 * xn),(unsigned int)(tip_x_step),tip_zo_step,
				  tip_delay * 1000);

  // start the approach
  // 1023.5 is maximum baby step search size (2047 bits = 100 V) divided by 2
  approach_serial(num_giant_steps,baby_step_size,tunneling_min,(float)(1023.5 / pow(zon,2)) );

  tunneling = 0;
  while(!tunneling)
  {
	tip_update_status(hDlg,"Tip Approach",i);
	i++;

	// what happened on current pass?
	pass_result = tip_approach_update(&tunneling);
	
	if(tunneling)
	{
		// tunneled.
		Beep( 750, 300 );
		SetDlgItemText(hDlg,TIP_ZO_EDIT,gcvt(dtov(pass_result,2),9,string));
		tip_update_status(hDlg,"Tunneled! Retracting Tip...",-1);
		retract_serial(TRUE);                      /* Retract */
		move_to_serial(zo_ch,pass_result,ZERO);
		mode_serial(fine_mode);
		tip_update_status(hDlg,"Tunneled!   Unretracting Tip...",-1);
		retract_serial(FALSE);                     /* Unretract */
	    tip_update_status(hDlg,"Tunneled!   Done!",-1);
	}
	else
	{
	  // didn't tunnel.
	  if(pass_result)
	  {
		sprintf(string,"%d",pass_result);        
		SetDlgItemText(hDlg,TIP_ZO_EDIT,string);
	  }

	  CheckMsgQ();
      if(tip_status == TIP_STOP_NOW)
	  {
		// end the approach
		tip_steps_terminate();
		tunneling = 1;		// get out of loop
	  }
	}
  }
  
  tip_status = TIP_IDLE;
  tip_approach_enable(hDlg);
  tip_update_status(hDlg,"Idle.",-1);
}

void tip_step(HWND hDlg,int dir)
{
  // Performs "blind" steps up (dir = +1) or down (dir = -1).
  // Unlike tip_approach(...), does NOT check for tunnel current.
  unsigned int i = 0;
  unsigned int xn,zon;
//  unsigned int *tip_data;
  char str[100];
  char dummy;
  int dummy2;

  tip_approach_disable(hDlg);

  switch(dir)
  {
    case UP:
        strcpy(str,"Stepping Up.");
        tip_status = TIP_STEPING_UP;
        break;
    case DOWN:
        strcpy(str,"Stepping Down.");
        tip_status = TIP_STEPING_DOWN;
        break;
  }

  // retract tip
  tip_update_status(hDlg,"Retracting...",0);
  retract_serial(TRUE);

  // into rotate, coarse mode. just to be sure.
  mode_serial(coarse_mode);
  mode_serial(rotate_mode);

  // calculate the x and z outer parabola parameters
	/* Compute size for xstep size x parabola:  t=sqrt(2*sqrt(2)*dist/accel) */
	/* where dist = v*10*(meters_per_output) */
  xn = (unsigned int)ceil(sqrt(2 * sqrt(2) * (dtov(ZERO + tip_x_step/2,x_range) * METERS_PER_XY_VOLT)
				/ ((double)G * tip_accel / 1000)) / SEC_PER_OUTPUT);
	/* Compute size for zstep size z parabola:  t=sqrt(2*dist/accel) */
	/* where dist = v*10*(meters_per_output) */
  zon = (unsigned int)ceil(sqrt(2 * (dtov(ZERO + tip_zo_step / 2,z_range) * 10 * METERS_PER_Z_VOLT)
				/ ((double)G * tip_accel / 1000)) / SEC_PER_OUTPUT);

  // send the parameters to the dsp
  tip_steps_setup((float)multiplier_fix*(tip_zo_step / 2 / (float)pow(zon,2)),(float)multiplier_fix*(tip_x_step / 2 / (float)pow(xn,2)),
				  (unsigned int)(0.586 * xn),(unsigned int)(tip_x_step),tip_zo_step,
				  tip_delay * 1000);

  // start giant steps
  tip_steps(tip_number,dir);

  while(i < tip_number)
  {
	i += tip_steps_update(tip_number - i);
	tip_update_status(hDlg,str,i);
	CheckMsgQ();
    if(tip_status == TIP_STOP_NOW)
	{
		// end the giant steps
		tip_steps_terminate();
		i = tip_number;		// get out of loop
	}
  }

  // unretract
  tip_update_status(hDlg,"Unretracting...",-1);
  read_serial(&dummy,1,&dummy2); // bogus read serial - resets port? or just delays?
  retract_serial(FALSE);

  tip_approach_enable(hDlg);
  tip_update_status(hDlg,"Idle.",-1);
}



//////////////////////////////02132012////////////////////////////////////////////////////////////////
void tip_translate_sample(HWND hDlg,int dir)
{
  unsigned int i = 0;
  unsigned int xn,zon;
//  unsigned int *tip_data;
  char str[100];
  char dummy;
  int dummy2;

  tip_translate_sample_disable(hDlg);

  strcpy(str,"Move...");
  tip_status = TIP_STEPING_UP;//no change here, doesn't matter

//  switch(dir)
//  {
//    case UP:
//        strcpy(str,"Move...");
//        tip_status = TIP_STEPING_UP;//no change here, doesn't matter
//        break;
//    case DOWN:
//        strcpy(str,"Move...");
//        tip_status = TIP_STEPING_DOWN;//no change here, doesn't matter
//       break;
//  }

  // retract tip
  tip_update_status(hDlg,"Retracting...",0);
  retract_serial(TRUE);

  // into translate, coarse mode. just to be sure.
  mode_serial(coarse_mode);
  mode_serial(translate_mode);

  // calculate the x and z outer parabola parameters
	/* Compute size for xstep size x parabola:  t=sqrt(2*sqrt(2)*dist/accel) */
	/* where dist = v*10*(meters_per_output) */
  xn = (unsigned int)ceil(sqrt(2 * sqrt(2) * (dtov(ZERO + tip_x_step/2,x_range) * METERS_PER_XY_VOLT)
				/ ((double)G * tip_accel / 1000)) / SEC_PER_OUTPUT);
	/* Compute size for zstep size z parabola:  t=sqrt(2*dist/accel) */
	/* where dist = v*10*(meters_per_output) */
  zon = (unsigned int)ceil(sqrt(2 * (dtov(ZERO + tip_zo_step / 2,z_range) * 10 * METERS_PER_Z_VOLT)
				/ ((double)G * tip_accel / 1000)) / SEC_PER_OUTPUT);

  // send the parameters to the dsp
  tip_steps_setup((float)multiplier_fix*(tip_zo_step / 2 / (float)pow(zon,2)),(float)multiplier_fix*(tip_x_step / 2 / (float)pow(xn,2)),
				  (unsigned int)(0.586 * xn),(unsigned int)(tip_x_step),tip_zo_step,
				  tip_delay * 1000);

  // start giant steps
  switch(tip_translate_sample_direction)
  {
	    case TIP_TRANSLATE_NE:
			tip_steps(tip_number,UP); //here UP == NE
			break;
	    case TIP_TRANSLATE_SW:
			tip_steps(tip_number,DOWN); //here DOWN == SW
			break;
	    default:
			break;
  }
  
  while(i < tip_number)
  {
	  switch(tip_translate_sample_direction)
	  {
			case TIP_TRANSLATE_NE:
			case TIP_TRANSLATE_SW:
				i += tip_steps_update(tip_number - i);
				break;
	  		default:
				break;
	  }

	tip_update_status(hDlg,str,i);
	CheckMsgQ();










    if(tip_status == TIP_STOP_NOW)
	{
		// end the giant steps
		tip_steps_terminate();
		i = tip_number;		// get out of loop
	}
  }

  // unretract
  tip_update_status(hDlg,"Unretracting...",-1);
  read_serial(&dummy,1,&dummy2); // bogus read serial - resets port? or just delays?
  retract_serial(FALSE);

  tip_translate_sample_enable(hDlg);
  tip_update_status(hDlg,"Idle.",-1);
}
//////////////////////////////02132012////////////////////////////////////////////////////////////////////



void pre_tip()
{
// Set output signals to "safe" values when dialog is opened.
	
//  move_to(x_ch,dac_data[x_ch],ZERO);
  move_to_serial(x_ch,dac_data[x_ch],ZERO);
//  move_to(y_ch,dac_data[y_ch],ZERO);
  move_to_serial(y_ch,dac_data[y_ch],ZERO);
//  move_to(x_offset_ch,dac_data[x_offset_ch],ZERO);
  move_to_serial(x_offset_ch,dac_data[x_offset_ch],ZERO);
//  move_to(y_offset_ch,dac_data[y_offset_ch],ZERO);
  move_to_serial(y_offset_ch,dac_data[y_offset_ch],ZERO);
//  move_to(z_offset_ch,dac_data[z_offset_ch],ZERO);
  move_to_serial(z_offset_ch,dac_data[z_offset_ch],ZERO);
//  move_to(zo_ch,dac_data[zo_ch],ZERO);
  move_to_serial(zo_ch,dac_data[zo_ch],ZERO);
//  hold(FALSE);
  dio_feedback_serial(TRUE);
//  set_gain(X_TEN_GAIN,Y_TEN_GAIN,Z_TENTH_GAIN,Z2_TEN_GAIN);
  set_gain_serial(X_TEN_GAIN,Y_TEN_GAIN,Z_TENTH_GAIN,Z2_TEN_GAIN);
//  mode(coarse_mode);
//  mode(rotate_mode);
  mode_serial(coarse_mode);
  mode_serial(rotate_mode);
}

void post_tip()
{
// Set output signals to "safe" values when dialog is closed.

//  move_to(zo_ch,ZERO,dac_data[zo_ch]);
  move_to_serial(zo_ch,ZERO,dac_data[zo_ch]);
//  move_to(z_offset_ch,ZERO,dac_data[z_offset_ch]);
  move_to_serial(z_offset_ch,ZERO,dac_data[z_offset_ch]);
//  move_to(y_offset_ch,ZERO,dac_data[y_offset_ch]);
  move_to_serial(y_offset_ch,ZERO,dac_data[y_offset_ch]);
//  move_to(x_offset_ch,ZERO,dac_data[x_offset_ch]);
  move_to_serial(x_offset_ch,ZERO,dac_data[x_offset_ch]);
//  move_to(y_ch,ZERO,dac_data[y_ch]);
  move_to_serial(y_ch,ZERO,dac_data[y_ch]);
//  move_to(x_ch,ZERO,dac_data[x_ch]);
  move_to_serial(x_ch,ZERO,dac_data[x_ch]);

//  mode(fine_mode);
//  mode(translate_mode);
  mode_serial(fine_mode);
  mode_serial(translate_mode);
}

void repaint_accel(HWND hDlg,unsigned int tip_accel)
{
  out_smart = 1;
  SetScrollPos(GetDlgItem(hDlg,TIP_ACCEL_SCROLL),SB_CTL,tip_accel,TRUE);
  out_smart = 0;
}

// Shaowei Translate
void repaint_tip_translate_sample (HWND hDlg)
{
	switch (tip_translate_sample_direction)
	{
	case TIP_TRANSLATE_NE:					;
		CheckDlgButton(hDlg,TIP_TRANSLATE_NE,1);
		CheckDlgButton(hDlg,TIP_TRANSLATE_SW,0);
		break;
	case TIP_TRANSLATE_SW:
		CheckDlgButton(hDlg,TIP_TRANSLATE_NE,0);
		CheckDlgButton(hDlg,TIP_TRANSLATE_SW,1);
		break;

	}

}
//End Shaowei

void repaint_number(HWND hDlg,unsigned int tip_number)
{
  out_smart = 1;
  SetScrollPos(GetDlgItem(hDlg,TIP_NUMBER_SCROLL),SB_CTL,tip_number,TRUE);
  out_smart = 0;
}

void repaint_delay(HWND hDlg,unsigned int tip_delay)
{
  out_smart = 1;
  SetScrollPos(GetDlgItem(hDlg,TIP_DELAY_SCROLL),SB_CTL,tip_delay,TRUE);
  out_smart = 0;
} 

void repaint_tunneling_min(HWND hDlg)
{
    SetScrollPos(GetDlgItem(hDlg,TIP_TUNNEL_SCROLL),SB_CTL,tunneling_min,TRUE);
}

void repaint_x_step(HWND hDlg,unsigned int tip_x_step)
{
  out_smart = 1;
  SetScrollPos(GetDlgItem(hDlg,TIP_X_STEP_SCROLL),SB_CTL,tip_x_step,TRUE);
  out_smart = 0;
}

void repaint_zo_step(HWND hDlg,unsigned int tip_zo_step)
{
  out_smart = 1;
  SetScrollPos(GetDlgItem(hDlg,TIP_ZO_STEP_SCROLL),SB_CTL,tip_zo_step,TRUE);
  out_smart = 0;
}
 
void repaint_giant(HWND hDlg)
{
  out_smart = 1;
  SetDlgItemInt(hDlg,TIP_NUM_GIANT,num_giant_steps,FALSE);
  SetScrollPos(GetDlgItem(hDlg,TIP_GIANT_SCROLL),SB_CTL,num_giant_steps,TRUE);
  out_smart = 0;
}

void repaint_baby(HWND hDlg)
{
  out_smart = 1;
  SetDlgItemInt(hDlg,TIP_BABY_SIZE,baby_step_size,FALSE);
  SetScrollPos(GetDlgItem(hDlg,TIP_BABY_SCROLL),SB_CTL,baby_step_size,TRUE);
  out_smart = 0;
}

void tip_approach_disable(HWND hDlg)
{
    SetFocus(hDlg);
    EnableWindow(GetDlgItem(hDlg,TIP_DELAY_EDIT),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_NUMBER_EDIT),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_ACCEL_EDIT),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_X_STEP_VOLTS_EDIT),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_X_STEP_BITS_EDIT),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_ZO_STEP_VOLTS_EDIT),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_ZO_STEP_BITS_EDIT),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_TUNNEL_VOLTS),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_TUNNEL_BITS),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_NUM_GIANT),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_BABY_SIZE),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_RETRACT),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_UNRETRACT),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_STEP_UP),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_STEP_DOWN),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_STOP),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_CONTROLS),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_EXIT),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_APPROACH),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_REINIT),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_UP_DEFAULTS),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_DOWN_DEFAULTS),FALSE);
/////////////////////////////////////02132012////////////////////////////////////////////
    EnableWindow(GetDlgItem(hDlg,TIP_TRANSLATE_SAMPLE_CHECK),FALSE);
/////////////////////////////////////02132012////////////////////////////////////////////

}    

void tip_approach_enable(HWND hDlg)
{
    EnableWindow(GetDlgItem(hDlg,TIP_DELAY_EDIT),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_NUMBER_EDIT),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_ACCEL_EDIT),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_X_STEP_VOLTS_EDIT),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_X_STEP_BITS_EDIT),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_ZO_STEP_VOLTS_EDIT),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_ZO_STEP_BITS_EDIT),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_TUNNEL_VOLTS),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_TUNNEL_BITS),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_NUM_GIANT),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_BABY_SIZE),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_RETRACT),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_UNRETRACT),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_STEP_UP),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_CONTROLS),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_EXIT),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_REINIT),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_UP_DEFAULTS),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_DOWN_DEFAULTS),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_STOP),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_STEP_DOWN),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_APPROACH),TRUE);
/////////////////////////////////////02132012////////////////////////////////////////////
    EnableWindow(GetDlgItem(hDlg,TIP_TRANSLATE_SAMPLE_CHECK),TRUE);
/////////////////////////////////////02132012////////////////////////////////////////////

}    
/////////////////////////////////////02132012////////////////////////////////////////////
void tip_translate_sample_checker_enable(HWND hDlg)
{
	EnableWindow(GetDlgItem(hDlg,TIP_STEP_UP),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_STEP_DOWN),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_APPROACH),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_UP_DEFAULTS),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_DOWN_DEFAULTS),FALSE);

	EnableWindow(GetDlgItem(hDlg,TIP_TRANSLATE_NE),TRUE);
	EnableWindow(GetDlgItem(hDlg,TIP_TRANSLATE_SW),TRUE);
	EnableWindow(GetDlgItem(hDlg,TIP_TRANSLATE_SAMPLE),TRUE);
}
void tip_translate_sample_checker_disable(HWND hDlg)
{
	EnableWindow(GetDlgItem(hDlg,TIP_STEP_UP),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_STEP_DOWN),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_APPROACH),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_UP_DEFAULTS),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_DOWN_DEFAULTS),TRUE);

	EnableWindow(GetDlgItem(hDlg,TIP_TRANSLATE_NE),FALSE);
	EnableWindow(GetDlgItem(hDlg,TIP_TRANSLATE_SW),FALSE);
	EnableWindow(GetDlgItem(hDlg,TIP_TRANSLATE_SAMPLE),FALSE);
}
void tip_translate_sample_disable(HWND hDlg)
{
    SetFocus(hDlg);
    EnableWindow(GetDlgItem(hDlg,TIP_DELAY_EDIT),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_NUMBER_EDIT),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_ACCEL_EDIT),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_X_STEP_VOLTS_EDIT),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_X_STEP_BITS_EDIT),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_ZO_STEP_VOLTS_EDIT),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_ZO_STEP_BITS_EDIT),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_TUNNEL_VOLTS),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_TUNNEL_BITS),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_NUM_GIANT),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_BABY_SIZE),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_RETRACT),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_UNRETRACT),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_STOP),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_CONTROLS),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_EXIT),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_REINIT),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_TRANSLATE_SAMPLE_CHECK),FALSE);
	EnableWindow(GetDlgItem(hDlg,TIP_TRANSLATE_NE),FALSE);
	EnableWindow(GetDlgItem(hDlg,TIP_TRANSLATE_SW),FALSE);
	EnableWindow(GetDlgItem(hDlg,TIP_TRANSLATE_SAMPLE),FALSE);

}    

void tip_translate_sample_enable(HWND hDlg)
{
    EnableWindow(GetDlgItem(hDlg,TIP_DELAY_EDIT),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_NUMBER_EDIT),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_ACCEL_EDIT),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_X_STEP_VOLTS_EDIT),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_X_STEP_BITS_EDIT),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_ZO_STEP_VOLTS_EDIT),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_ZO_STEP_BITS_EDIT),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_TUNNEL_VOLTS),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_TUNNEL_BITS),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_NUM_GIANT),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_BABY_SIZE),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_RETRACT),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_UNRETRACT),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_CONTROLS),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_EXIT),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_REINIT),TRUE);
    EnableWindow(GetDlgItem(hDlg,TIP_STOP),FALSE);
    EnableWindow(GetDlgItem(hDlg,TIP_TRANSLATE_SAMPLE_CHECK),TRUE);
	EnableWindow(GetDlgItem(hDlg,TIP_TRANSLATE_NE),TRUE);
	EnableWindow(GetDlgItem(hDlg,TIP_TRANSLATE_SW),TRUE);
	EnableWindow(GetDlgItem(hDlg,TIP_TRANSLATE_SAMPLE),TRUE);

}    
/////////////////////////////////////02132012////////////////////////////////////////////
void tip_update_status(HWND hDlg,char *msg,int pass)
{
    SetDlgItemText(hDlg,TIP_STATUS,msg);
    if(pass >= 0)
        SetDlgItemInt(hDlg,TIP_PASS_NUM,pass,FALSE);
//    else SetDlgItemText(hDlg,TIP_PASS_NUM,"");
}     

static void repaint_bias_current(HWND hDlg)
{
    SetDlgItemText(hDlg,TIP_BIAS,gcvt(dtov(sample_bias,get_range(sample_bias_ch)),5,string));
#ifdef DAC16
	if (bit16) SetDlgItemText(hDlg,TIP_BIAS,gcvt(dtov16(sample_bias),5,string));
#endif

    SetDlgItemText(hDlg,TIP_CURRENT,gcvt(i_set,5,string));
}    


// ******************************************************************************************
//		O L D * O L D * O L D * O L D * O L D * O L D * O L D * O L D * O L D * O L D
// ******************************************************************************************


#ifdef OLD

void giant_steps_old(unsigned int xn,unsigned int zon,unsigned int *data)
{
  int i;
  for(i = 0;i < num_giant_steps;i++) {
    dio_blk_out(2 * xn + 9,data);                       /* Output waveform */
    dio_start_clock(1000 * tip_delay);
    dio_wait_clock();
    dio_blk_out(2 * zon + 1,data + 2 * xn + 9);                       /* Output waveform */
  }
}

unsigned int baby_steps_old(HWND hDlg,unsigned int n,unsigned int *data)
{
  int  zo = ZERO;
  unsigned int  I = IN_ZERO;

  dio_start_clock(TIP_BABY_WAIT_1);
  dio_wait_clock();
  
  dio_in_data(&I);
  if(abs((int)I - IN_ZERO) >= tunneling_min)
  {
      dio_start_clock(TIP_BABY_WAIT_2);
      sprintf(string,"%d",I - IN_ZERO);        
      SetDlgItemText(hDlg,TIP_ZO_EDIT,string);
      dio_wait_clock(); 
      dio_in_data(&I);
  }
  while(abs((int)I - IN_ZERO) < tunneling_min && zo <= (MAX - baby_step_size)) {
    zo += baby_step_size;
    dio_out(zo_ch,zo);
    adc_delay();
    dio_in_data(&I);
    if(abs((int)I - IN_ZERO) >= tunneling_min)
    {
        dio_start_clock(TIP_BABY_WAIT_2);
        sprintf(string,"%d",I - IN_ZERO);        
        SetDlgItemText(hDlg,TIP_ZO_EDIT,string);
        dio_wait_clock();
        dio_in_data(&I);
    }
  }
  if(abs((int)I - IN_ZERO) >= tunneling_min) {
    Beep( 750, 300 );
    tip_update_status(hDlg,"Tunneled!   Retracting Tip...",-1);
    retract(TRUE);                      /* Retract */
    move_to(zo_ch,zo,ZERO);
    mode(fine_mode);
    SetDlgItemText(hDlg,TIP_ZO_EDIT,gcvt(dtov(zo,z_range),9,string));
    tip_update_status(hDlg,"Tunneled!   Unretracting Tip...",-1);
    
    retract(FALSE);                     /* Unretract */
    tip_update_status(hDlg,"Tunneled!   Done!",-1);
    
    return(1);                          /* Tunneling */
  }
  else {
    dio_blk_out(n,data);
    return(0);
  }                 
}

void tip_step_old(HWND hDlg,int dir)
{
  unsigned int  i = 0;
  unsigned int  xn,zon;
  unsigned int  *tip_data;
  char str[100];

#ifdef OLD
  EnableWindow(GetDlgItem(hDlg,TIP_REMOVE),FALSE);
#endif
  tip_approach_disable(hDlg);
  tip_update_status(hDlg,"Retracting...",0);
  switch(dir)
  {
    case UP:
        strcpy(str,"Stepping Up.");
        tip_status = TIP_STEPING_UP;
        break;
    case DOWN:
        strcpy(str,"Stepping Down.");
        tip_status = TIP_STEPING_DOWN;
        break;
  }
  retract(TRUE);                      /* Retract */
  mode(coarse_mode);
  mode(rotate_mode);
	/* Compute size for xstep size x parabola:  t=sqrt(2*sqrt(2)*dist/accel) */
	/* where dist = v*10*(meters_per_output) */
  xn = (unsigned int)ceil(sqrt(2 * sqrt(2) * (dtov(ZERO + tip_x_step / 2,x_range) * METERS_PER_XY_VOLT)
				/ ((double)G * tip_accel / 1000)) / SEC_PER_OUTPUT);
	/* Compute size for zstep size z parabola:  t=sqrt(2*dist/accel) */
	/* where dist = v*10*(meters_per_output) */
  zon = (unsigned int)ceil(sqrt(2 * (dtov(ZERO + tip_zo_step / 2,z_range) * 10 * METERS_PER_Z_VOLT)
				/ ((double)G * tip_accel / 1000)) / SEC_PER_OUTPUT);
  tip_data = (unsigned int *)tip_setup(xn,zon,tip_x_step,tip_zo_step,dir);
  for(i = 0;i < tip_number;i++) {
    CheckMsgQ();
    if(tip_status == TIP_STOP_NOW) break;
    tip_update_status(hDlg,str,i + 1);
    dio_blk_out(2 * xn + 9,tip_data);                       /* Output waveform */
//  dio_start_clock(tip_delay * 1000);
//  dio_wait_clock();
    dio_start_clock(1000 * tip_delay);
    dio_wait_clock();
    dio_blk_out(2 * zon + 1,tip_data + 2 * xn + 9);                       /* Output waveform */
    tip_update_status(hDlg,"Windows in control...",i);
#ifdef OLD
    PeekMessage(&Message,hDlg,NULL,NULL,PM_REMOVE);     /* ESC = abort */
    if(Message.wParam==VK_ESCAPE) {
      sprintf(string,"Retract terminated\nafter %d steps.",i);
      MessageBox(hDlg,string,"Warning",MB_ICONEXCLAMATION);
      break;
    }
#endif
  }
  tip_free(tip_data);
  tip_update_status(hDlg,"Unretracting...",-1);
  retract(FALSE);                      /* Unretract */
  tip_approach_enable(hDlg);
  tip_update_status(hDlg,"Idle.",-1);
#ifdef OLD
  EnableWindow(GetDlgItem(hDlg,TIP_REMOVE),TRUE);
#endif
}

void tip_approach_old(HWND hDlg,unsigned int accel)
{
  unsigned int  i = 0;
  unsigned int  *tip_data;
  unsigned int  *z_data;
  unsigned int  tunneling = 0;
  unsigned int  xn;		// number of outputs required to execute x parabola
  unsigned int  zon;
  unsigned int  zn;		// number of outputs required to execute z parabola

  // warnings
  if (tip_accel != TIP_DOWN_ACCEL_DEF || tip_x_step !=TIP_DOWN_X_STEP_DEF ||
    tip_zo_step != TIP_DOWN_Z_STEP_DEF)
  {
    if (MessageBox(hDlg,"The settings are not defaults! Continue with approach?",
                "Warning!",MB_YESNO | MB_ICONEXCLAMATION)==IDNO)
        return;
  }
  if (!(dac_data[feedback_ch] & feedback_bit))
  {
    if (MessageBox(hDlg,"Feedback is off! Continue with approach?",
                "Warning!",MB_YESNO | MB_ICONEXCLAMATION)==IDNO)
        return;
  }
        
  // miscellaneous inizialization
  tip_approach_disable(hDlg);
  tip_status = TIP_APPROACH;
  tip_update_status(hDlg,"Starting Approach",0);
  mode(coarse_mode);
  mode(rotate_mode);

  // calculate and create the x and z parabolas
	/* Compute size for xstep size x parabola:  t=sqrt(2*sqrt(2)*dist/accel) */
	/* where dist = v*10*(meters_per_output) */
  xn = (unsigned int)ceil(sqrt(2 * sqrt(2) * (dtov(ZERO + tip_x_step / 2,x_range) * METERS_PER_XY_VOLT)
				/ ((double)G * tip_accel / 1000)) / SEC_PER_OUTPUT);
	/* Compute size for zstep size z parabola:  t=sqrt(2*dist/accel) */
	/* where dist = v*10*(meters_per_output) */
  zon = (unsigned int)ceil(sqrt(2 * (dtov(ZERO + tip_zo_step / 2,z_range) * 10 * METERS_PER_Z_VOLT)
				/ ((double)G * tip_accel / 1000)) / SEC_PER_OUTPUT);
  tip_data = (unsigned int *)tip_setup(xn,zon,tip_x_step,tip_zo_step,-1);
	/* Compute size for 50V z parabola */
  zn = (unsigned int)ceil(sqrt(2 * (50 * 10 * METERS_PER_Z_VOLT)
				/ ((double)G * accel / 1000)) / SEC_PER_OUTPUT);
  z_data = tip_zo_setup(zn);            /* Set up z parabola */
  
  // alternate baby steps and giant steps until we've tunnelled
  while(!tunneling) {
    i++;
    tip_update_status(hDlg,"Baby Steps",i);
    tunneling = baby_steps(hDlg,2 * zn,z_data);
    if(!tunneling) 
    {
        tip_update_status(hDlg,"Giant Steps",i);
        giant_steps(xn,zon,tip_data);
        tip_update_status(hDlg,"Windows in control...",i);
    }

//    coarse_z += 2 * 100 * METERS_PER_XY_VOLT * 1e10;                /* Change # */
    CheckMsgQ();
    if (tip_status == TIP_STOP_NOW) break;

  }
  tip_free(tip_data);
  tip_free(z_data);
  repaint_z(/*hDlg,coarse_z*/);
  tip_status = TIP_IDLE;
  tip_approach_enable(hDlg);
  tip_update_status(hDlg,"Idle.",-1);
}

#endif

