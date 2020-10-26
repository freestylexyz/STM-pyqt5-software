#include <math.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <time.h>
#include "common.h"
#include "file.h"
#include "data.h"
#include "stm.h"
#include "dio.h"
#include "scan.h"
#include "tip.h"
#include "io_ctrl.h"
#include "clock.h"
#include "serial.h"
#include "pre_scan.h"

extern unsigned int dac_data[];
extern unsigned int out1;
extern unsigned int bit16;
extern int	feedback,
            x_range,
            y_range,
            z_range,
            x_offset_range,
            y_offset_range,
            z_offset_range,
            i_set_range,
            sample_bias_range;
extern unsigned int sample_bias;
extern unsigned int tip_gain;
extern unsigned int i_setpoint;
extern double i_set;
extern int z_offset_ch;
extern float scan_x_gain,scan_y_gain;

extern char string[];
extern double a_per_v, z_a_per_v;

int current_pos = x_ch,current_offset = x_offset_ch;
unsigned int unsent_pos,unsent_offset;
int bias_delay = BIAS_SQUARE_DELAY;

static int out_smart = 0;

void repaint_pos(HWND);
void repaint_offset(HWND);
void repaint_gains(HWND);
static void repaint_bias_delay(HWND);
static void repaint_zoff_ch(HWND);
static void switch_zoff_ranges_biases();
static void repaint_bias_ch(HWND);

BOOL FAR PASCAL IOCtrlDlg(HWND,unsigned,WPARAM,LPARAM);
// The I/O Controls dialog provides access to all of the signals that control
// the STM (bias,i_setpoint,x_offset,y_offset,etc) and most of the digitally
// controlled electronics settings (feedback,gains,piezo mode,etc)

BOOL FAR PASCAL IOCtrlDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{
    int id;
    int i,mini,maxi,delt,ddelt;
    BOOL repaint_ch = FALSE;
    BOOL repaint_data = FALSE;
    int done = 0;
    float volts;
    unsigned int bias_min,bias_max,old_bias;
    MSG Message2;
    float point1 = (float)0.1;

    switch(Message)
    {
      case WM_INITDIALOG:
        unsent_pos = dac_data[current_pos];
        unsent_offset = dac_data[current_offset];
        SetScrollRange(GetDlgItem(hDlg,IO_POS_SCROLL),SB_CTL,IO_POS_MIN,IO_POS_MAX,FALSE);
        SetScrollRange(GetDlgItem(hDlg,IO_OFFSET_SCROLL),SB_CTL,IO_OFFSET_MIN,IO_OFFSET_MAX,FALSE);
        SetScrollRange(GetDlgItem(hDlg,TIP_CURRENT_SCROLL),SB_CTL,tip_current_min,tip_current_max,FALSE);
        SetScrollRange(GetDlgItem(hDlg,SAMPLE_BIAS_SCROLL),SB_CTL,sample_bias_min,sample_bias_max,FALSE);
        SetScrollRange(GetDlgItem(hDlg,TIP_CURRENT_SCROLL),SB_CTL,tip_current_min,tip_current_max,FALSE);
        if(get_range(sample_bias_ch) == 1)
        {
            CheckDlgButton(hDlg,IO_BIAS_RANGE_5,1);
            CheckDlgButton(hDlg,IO_BIAS_RANGE_10,0);
        }
		else
        {
            CheckDlgButton(hDlg,IO_BIAS_RANGE_5,0);
            CheckDlgButton(hDlg,IO_BIAS_RANGE_10,1);
        }
#ifdef	DAC16
		if(bit16)
		{
			CheckDlgButton(hDlg,IO_BIAS_RANGE_5,1);
            CheckDlgButton(hDlg,IO_BIAS_RANGE_10,0);
			EnableWindow(GetDlgItem(hDlg,IO_BIAS_RANGE_10),FALSE);
			//need to disable the +/-10 range button
		}
#endif

        if (dio_dither_status(0))
        {
            CheckDlgButton(hDlg,IO_DITHER0_ON,1);
            CheckDlgButton(hDlg,IO_DITHER0_OFF,0);
        }
		else
        {
            CheckDlgButton(hDlg,IO_DITHER0_ON,0);
            CheckDlgButton(hDlg,IO_DITHER0_OFF,1);
        }            
        if (dio_dither_status(1))
        {
            CheckDlgButton(hDlg,IO_DITHER1_ON,1);
            CheckDlgButton(hDlg,IO_DITHER1_OFF,0);
        }
		else
        {
            CheckDlgButton(hDlg,IO_DITHER1_ON,0);
            CheckDlgButton(hDlg,IO_DITHER1_OFF,1);
        }            
        if(dac_data[feedback_ch] & feedback_bit) 
        {
            CheckDlgButton(hDlg,IO_FEEDBACK_ON,1);
            CheckDlgButton(hDlg,IO_FEEDBACK_OFF,0);
        }
		else
        {
            CheckDlgButton(hDlg,IO_FEEDBACK_ON,0);
            CheckDlgButton(hDlg,IO_FEEDBACK_OFF,1);
        }            
        if(dac_data[retract_ch] & retract_bit) 
        {
            CheckDlgButton(hDlg,IO_RETRACT_ON,1);
            CheckDlgButton(hDlg,IO_RETRACT_OFF,0);
        }
		else
        {
            CheckDlgButton(hDlg,IO_RETRACT_ON,0);
            CheckDlgButton(hDlg,IO_RETRACT_OFF,1);
        }            
        if(dac_data[mode_ch] & coarse_bit) 
        {
            CheckDlgButton(hDlg,IO_COARSE,1);
            CheckDlgButton(hDlg,IO_FINE,0);
        }
		else
        {
            CheckDlgButton(hDlg,IO_COARSE,0);
            CheckDlgButton(hDlg,IO_FINE,1);
        }            
        if(dac_data[mode_ch] & translate_bit) 
        {
            CheckDlgButton(hDlg,IO_TRANSLATE,1);
            CheckDlgButton(hDlg,IO_ROTATE,0);
        }
		else
        {
            CheckDlgButton(hDlg,IO_TRANSLATE,0);
            CheckDlgButton(hDlg,IO_ROTATE,1);
        }            
        switch(tip_gain)
        {
          case 8:
            CheckDlgButton(hDlg, TIP_CURRENT_GAIN_8, 1);
            SetDlgItemText(hDlg, TIP_CURRENT_RANGE_TEXT, "1 to 100 nA");
            break;
          case 9:
            CheckDlgButton(hDlg, TIP_CURRENT_GAIN_9, 1);
            SetDlgItemText(hDlg, TIP_CURRENT_RANGE_TEXT, "0.1 to 10 nA");
            break;
          case 10:
            CheckDlgButton(hDlg, TIP_CURRENT_GAIN_10, 1);
            SetDlgItemText(hDlg, TIP_CURRENT_RANGE_TEXT, "0.01 to 1 nA");
            break;
/*		  Gain of 11 *does* exist on preamp.*/
          case 11:
            CheckDlgButton(hDlg, TIP_CURRENT_GAIN_11, 1);
            SetDlgItemText(hDlg, TIP_CURRENT_RANGE_TEXT, "0.001 to 0.1 nA");
            break;

		}
        
        repaint_pos(hDlg);
        repaint_offset(hDlg);
        repaint_gains(hDlg);
        repaint_sample_bias(hDlg);
        repaint_tip_current(hDlg);
        repaint_bias_delay(hDlg);
		repaint_zoff_ch( hDlg);
		repaint_bias_ch(hDlg);

        break;

	  case WM_VSCROLL:
      case WM_HSCROLL:
        if(!out_smart)
        {
            id = getscrollid();
            switch(id)
            {
                case IO_POS_SCROLL:
                    i = unsent_pos;
                    mini = IO_POS_MIN;
                    maxi = IO_POS_MAX;
                    delt = IO_POS_DELT;
                    ddelt = IO_POS_DDELT;
                    break;
                case IO_OFFSET_SCROLL:
                    i = unsent_offset;
                    mini = IO_OFFSET_MIN;
                    maxi = IO_OFFSET_MAX;
                    delt = IO_OFFSET_DELT;
                    ddelt = IO_OFFSET_DDELT;
                    break;
                case SAMPLE_BIAS_SCROLL:
                    i = (int)sample_bias;
                    mini = sample_bias_min;
                    maxi = sample_bias_max;
                    delt = sample_bias_delt;
                    ddelt = sample_bias_ddelt;
                    break;
                case TIP_CURRENT_SCROLL:
                    i = (int)tip_current_max-i_setpoint;
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
            }
            i = min(max(i,mini),maxi);
            switch(id)
            {
                case IO_POS_SCROLL:
                    unsent_pos = i;
                    repaint_pos(hDlg);
                    break;
                case IO_OFFSET_SCROLL:
                    unsent_offset = i;
                    repaint_offset(hDlg);
                    break;
                case SAMPLE_BIAS_SCROLL:
                    sample_bias = (unsigned int)i;
                    //bias(sample_bias);
                    bias_serial(sample_bias);
                    repaint_sample_bias(hDlg);
                    break;
                case TIP_CURRENT_SCROLL:
                    i_setpoint = (unsigned int)(tip_current_max - i);
                    //tip_current(i_setpoint);
                    tip_current_serial(i_setpoint);
                    repaint_tip_current(hDlg);
                    break;
            }
        }
        break;
      case WM_COMMAND:
        switch(LOWORD(wParam))
        {
            case IO_EMERGENCY_EXIT:
                if(MessageBox(hDlg,"Are you sure you want to exit the program?",
                    "Warning!",MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
                {
                    save_init(SCAN_SETTINGS);
                    remove(SCAN_IMAGE);
                    exit(0);
                }
                break;
                
			// Dithering
            case IO_DITHER0_ON:
                //dio_dither(0,1);
                dio_dither_serial(0,1);
                break;
            case IO_DITHER0_OFF:
                //dio_dither(0,0);
                dio_dither_serial(0,0);
                break;
            case IO_DITHER1_ON:
                //dio_dither(1,1);
                dio_dither_serial(1,1);
                break;
            case IO_DITHER1_OFF:
                //dio_dither(1,0);
                dio_dither_serial(1,0);
                break;

			// Square wave
            case BIAS_SQUARE_WAVE:
				// bias_square_wave_old();
				bias_min = vtod(BIAS_SQUARE_MIN,get_range(sample_bias_ch));
                bias_max = vtod(BIAS_SQUARE_MAX,get_range(sample_bias_ch));
				old_bias = dac_data[sample_bias_ch];
				start_bias_square(bias_max,bias_min,bias_delay);

                done = 0;
                while(!done)
                {
                   PeekMessage(&Message2,hDlg,NULL,NULL,PM_REMOVE);
                   if(LOWORD(Message2.wParam) == VK_ESCAPE)
                   {
						stop_bias_square();
						sprintf(string,"Sqaure Wave terminated.");
                        MessageBox(hDlg,string,"Warning",MB_ICONEXCLAMATION);
                        done = 1;
                        
                   }
                }

				// restore old bias
				bias_serial(old_bias);

                break;
			case IO_SQUARE_DELAY:
				if(!out_smart)
				{
					out_smart = 1;
					GetDlgItemText(hDlg,IO_SQUARE_DELAY,string,8);
					i = atoi(string);
					i = max(min(i,MAX_SERIAL_DELAY),0);
					bias_delay = (int)i;
					out_smart = 0;
				}
				break;

			// Positioning
			case IO_X:
				current_pos = x_ch;
				unsent_pos = dac_data[current_pos];
				repaint_pos(hDlg);
				break;
			case IO_Y:
				current_pos = y_ch;
				unsent_pos = dac_data[current_pos];
				repaint_pos(hDlg);
				break;
			case IO_Z:
				current_pos = zo_ch;
				unsent_pos = dac_data[current_pos];
				repaint_pos(hDlg);
				break;
			case IO_POS_SEND:
				//dio_out(current_pos,unsent_pos);
				dio_out_serial(current_pos,unsent_pos);
				dac_data[current_pos] = unsent_pos;
				repaint_pos(hDlg);
				break;
			case IO_POS_RANGE_5:
				//set_range(current_pos,1);
				set_range_serial(current_pos,1);
				repaint_pos(hDlg);
				break;
			case IO_POS_RANGE_10:
				//set_range(current_pos,2);
				set_range_serial(current_pos,2);
				repaint_pos(hDlg);
				break;
			case IO_POS_VOLTS:
				if(!out_smart)
				{
					GetDlgItemText(hDlg,IO_POS_VOLTS,string,10);
					volts = (float)atof(string);
					i = (unsigned int)vtod(volts,get_range(current_pos));
					unsent_pos= min(max(i,IO_POS_MIN),IO_POS_MAX);
				}
				break;
			case IO_POS_BITS:
				if(!out_smart)
				{
					GetDlgItemText(hDlg,IO_POS_BITS,string,7);
					i = atoi(string);
					i = min(max(i,IO_POS_MIN),IO_POS_MAX);
					unsent_pos = (unsigned int)i;
				}
				break;

			// Offsets
			case IO_X_OFFSET:
				current_offset = x_offset_ch;
				unsent_offset = dac_data[current_offset];
				repaint_offset(hDlg);
				break;
			case IO_Y_OFFSET:
				current_offset = y_offset_ch;
				unsent_offset = dac_data[current_offset];
				repaint_offset(hDlg);
				break;
			case IO_Z_OFFSET:
				current_offset = z_offset_ch;
				unsent_offset = dac_data[current_offset];
				repaint_offset(hDlg);
				break;
			case IO_OFFSET_SEND:
				//dio_out(current_offset,unsent_offset);
				dio_out_serial(current_offset,unsent_offset);
				dac_data[current_offset] = unsent_offset;
				repaint_offset(hDlg);
				break;
			case IO_OFFSET_RANGE_5:
				//set_range(current_offset,1);
				set_range_serial(current_offset,1);
				repaint_offset(hDlg);
				break;
			case IO_OFFSET_RANGE_10:
				//set_range(current_offset,2);
				set_range_serial(current_offset,2);
				repaint_offset(hDlg);
				break;
			case IO_OFFSET_VOLTS:
				if(!out_smart)
				{
					GetDlgItemText(hDlg,IO_OFFSET_VOLTS,string,10);
					volts = (float)atof(string);
					i = (unsigned int)vtod(volts,get_range(current_offset));
					i = min(max(i,IO_OFFSET_MIN),IO_OFFSET_MAX);
					unsent_offset = (unsigned int)i;
				}
				break;
			case IO_OFFSET_BITS:
				if(!out_smart)
				{
					GetDlgItemText(hDlg,IO_OFFSET_BITS,string,7);
					i = atoi(string);
					i = min(max(i,IO_OFFSET_MIN),IO_OFFSET_MAX);
					unsent_offset = (unsigned int)i;
				}
				break;

			// Gains
			case IO_X_01:
				//set_x_gain(X_TENTH_GAIN);
				set_x_gain_serial(X_TENTH_GAIN);
				scan_x_gain = point1;
				break;
			case IO_X_1:
				//set_x_gain(X_ONE_GAIN);
				set_x_gain_serial(X_ONE_GAIN);
				scan_x_gain = 1;
				break;
			case IO_X_10:
				//set_x_gain(X_TEN_GAIN);
				set_x_gain_serial(X_TEN_GAIN);
				scan_x_gain = 10;
				break;
			case IO_Y_01:
				//set_y_gain(Y_TENTH_GAIN);
				set_y_gain_serial(Y_TENTH_GAIN);
				scan_y_gain = point1;
				break;
			case IO_Y_1:
				//set_y_gain(Y_ONE_GAIN);
				set_y_gain_serial(Y_ONE_GAIN);
				scan_y_gain = 1;
				break;
			case IO_Y_10:
				//set_y_gain(Y_TEN_GAIN);
				set_y_gain_serial(Y_TEN_GAIN);
				scan_y_gain = 10;
				break;
			case IO_Z_01:
				//set_z_gain(Z_TENTH_GAIN);
				set_z_gain_serial(Z_TENTH_GAIN);
				break;
			case IO_Z_1:
				//set_z_gain(Z_ONE_GAIN);
				set_z_gain_serial(Z_ONE_GAIN);
				break;
			case IO_Z_10:
				//set_z_gain(Z_TEN_GAIN);
				set_z_gain_serial(Z_TEN_GAIN);
				break;
			case IO_Z2_01:
				//set_z2_gain(Z2_TENTH_GAIN);
				set_z2_gain_serial(Z2_TENTH_GAIN);
				break;
			case IO_Z2_1:
				//set_z2_gain(Z2_ONE_GAIN);
				set_z2_gain_serial(Z2_ONE_GAIN);
				break;
			case IO_Z2_10:
				//set_z2_gain(Z2_TEN_GAIN);
				set_z2_gain_serial(Z2_TEN_GAIN);
				break;

			// feedback & retract
			case IO_FEEDBACK_ON:
				//dio_feedback(1);
				dio_feedback_serial(1);
				break;
			case IO_FEEDBACK_OFF:
				//dio_feedback(0);
				dio_feedback_serial(0);
				break;
			case IO_RETRACT_ON:
				//retract(1);
				retract_serial(1);
				break;
			case IO_RETRACT_OFF:
				//retract(0);
				retract_serial(0);
				break;

			// mode
			case IO_TRANSLATE:
				//mode(translate_mode);
				mode_serial(translate_mode);
				break;
			case IO_ROTATE:
				//mode(rotate_mode);
				mode_serial(rotate_mode);
				break;
			case IO_FINE:
				//mode(fine_mode);
				mode_serial(fine_mode);
				if(current_pos == zo_ch)
				{
					// kluge to account for zo being zeroed by mode_serial
					unsent_pos = dac_data[zo_ch];
					repaint_pos(hDlg);
				}
				break;
			case IO_COARSE:
				//mode(coarse_mode);
				mode_serial(coarse_mode);
				if(current_pos == zo_ch)
				{
					// kluge to account for zo being zeroed by mode_serial
					unsent_pos = dac_data[zo_ch];
					repaint_pos(hDlg);
				}
				break;
				break;
			
			case IO_BIAS_CH0:
#ifdef DAC16
				bit16 = 0;
				repaint_bias_ch( hDlg);
				SetScrollRange(GetDlgItem(hDlg,SAMPLE_BIAS_SCROLL),SB_CTL,sample_bias_min,sample_bias_max,FALSE);
				EnableWindow(GetDlgItem(hDlg,IO_BIAS_RANGE_10),TRUE);
				if (get_range(sample_bias_ch)==2) 
				{
					CheckDlgButton(hDlg,IO_BIAS_RANGE_5,0);
					CheckDlgButton(hDlg,IO_BIAS_RANGE_10,1);
				}
				sample_bias=vtod(dtov16(sample_bias),get_range(sample_bias_ch));
				bias_serial(sample_bias);
				repaint_sample_bias(hDlg);
#endif
				break;
			case IO_BIAS_CH10:
#ifdef DAC16
				bit16 = 1;
				repaint_bias_ch( hDlg);
				SetScrollRange(GetDlgItem(hDlg,SAMPLE_BIAS_SCROLL),SB_CTL,sample_bias_min,sample_bias_max,FALSE);
				CheckDlgButton(hDlg,IO_BIAS_RANGE_5,1);
				CheckDlgButton(hDlg,IO_BIAS_RANGE_10,0);
				EnableWindow(GetDlgItem(hDlg,IO_BIAS_RANGE_10),FALSE);
				sample_bias=vtod16(dtov(sample_bias,get_range(sample_bias_ch)));
				bias_serial(sample_bias);
				repaint_sample_bias(hDlg);
#endif
				break;

			case IO_ZOFFSET_7:
				if(current_offset == z_offset_ch) current_offset = 7;	
				z_offset_ch = 7;
				switch_zoff_ranges_biases();
				repaint_zoff_ch( hDlg);
				break;
			case IO_ZOFFSET_1:
				if(current_offset == z_offset_ch) current_offset = 1;	
				z_offset_ch = 1;
				switch_zoff_ranges_biases();
				repaint_zoff_ch( hDlg);
				break;

			// sample bias
			case SAMPLE_BIAS_BITS_EDIT:
				if(!out_smart)
				{
				  out_smart = 1;
				  GetDlgItemText(hDlg,SAMPLE_BIAS_BITS_EDIT,string,9);
				  i = atoi(string);
				  i = min(max(i,sample_bias_min),sample_bias_max);
				  sample_bias = (unsigned int)i;
				  //bias(sample_bias);
				  bias_serial(sample_bias);
				  out_smart = 0;
				}
				break;
			case SAMPLE_BIAS_VOLTS_EDIT:
				if(!out_smart)
				{
				  out_smart = 1;
				  GetDlgItemText(hDlg,SAMPLE_BIAS_VOLTS_EDIT,string,9);
#ifdef DAC16
				  i = vtod_bias(atof(string),get_range(sample_bias_ch));
#else
				  i = vtod(atof(string),get_range(sample_bias_ch));
#endif
				  i = min(max(i,sample_bias_min),sample_bias_max);
				  sample_bias = (unsigned int)i;
				  //bias(sample_bias);
				  bias_serial(sample_bias);
				  out_smart = 0;
				}
				break;
			case IO_BIAS_RANGE_5:
				//set_range(sample_bias_ch,1);
				set_range_serial(sample_bias_ch,1);
				repaint_sample_bias(hDlg);
				break;
			case IO_BIAS_RANGE_10:
				//set_range(sample_bias_ch,2);
				if (!bit16)
				{
				set_range_serial(sample_bias_ch,2);
				repaint_sample_bias(hDlg);
				}
				else
				{
				CheckDlgButton(hDlg,IO_BIAS_RANGE_5,1);
				CheckDlgButton(hDlg,IO_BIAS_RANGE_10,0);
				//remove this if disabled is done
				}
				break;

			// tip current
			case TIP_CURRENT_GAIN_8:
              if(IsDlgButtonChecked(hDlg,TIP_CURRENT_GAIN_8))
			  {
                SetDlgItemText(hDlg,TIP_CURRENT_RANGE_TEXT,"1 to 100 nA");
                tip_gain = 8;
                repaint_tip_current(hDlg);
			  }
             break;
			case TIP_CURRENT_GAIN_9:
              if(IsDlgButtonChecked(hDlg,TIP_CURRENT_GAIN_9))
			  {
                SetDlgItemText(hDlg,TIP_CURRENT_RANGE_TEXT,"0.1 to 10 nA");
                tip_gain = 9;
                repaint_tip_current(hDlg);
			  }
             break;
			case TIP_CURRENT_GAIN_10:
              if(IsDlgButtonChecked(hDlg,TIP_CURRENT_GAIN_10))
			  {
                SetDlgItemText(hDlg,TIP_CURRENT_RANGE_TEXT,"0.01 to 1 nA");
                tip_gain = 10;
                repaint_tip_current(hDlg);
			  }
             break;

			case TIP_CURRENT_GAIN_11:
              if(IsDlgButtonChecked(hDlg,TIP_CURRENT_GAIN_11))
              {
                SetDlgItemText(hDlg,TIP_CURRENT_RANGE_TEXT,"0.001 to 0.1 nA");
                tip_gain = 11;
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
            
			case ENTER:
				repaint_pos(hDlg);
				repaint_offset(hDlg);
				repaint_sample_bias(hDlg);
				repaint_tip_current(hDlg);
				repaint_bias_delay(hDlg);
				break;
			case IO_REINIT:
//				dio_init();
				dio_init_serial();
				break;

			case IO_EXIT:
				feedback = ((dac_data[feedback_ch] & feedback_bit) && 1);
				x_range = get_range(x_ch);
				y_range = get_range(y_ch);
				z_range = get_range(zo_ch);
				x_offset_range = get_range(x_offset_ch);
				y_offset_range = get_range(y_offset_ch);
				z_offset_range = get_range(z_offset_ch);
				sample_bias_range = get_range(sample_bias_ch);
				i_set_range = get_range(i_setpoint_ch);
				EndDialog(hDlg, TRUE);
				return (TRUE);
				break;
        }
        break;
    }
    return (FALSE);
}

void repaint_pos(HWND hDlg)
{
    int range;
    
    out_smart = 1;
    range = get_range(current_pos);
    if(range == 1)
    {
        CheckDlgButton(hDlg,IO_POS_RANGE_10,0);
        CheckDlgButton(hDlg,IO_POS_RANGE_5,1);
    }
    else 
    {
        CheckDlgButton(hDlg,IO_POS_RANGE_10,1);
        CheckDlgButton(hDlg,IO_POS_RANGE_5,0);
    }
        
    SetScrollPos(GetDlgItem(hDlg,IO_POS_SCROLL),SB_CTL,(int)unsent_pos,TRUE);
    SetDlgItemText(hDlg,IO_POS_CH,gcvt(current_pos,5,string));
    sprintf(string,"%d",unsent_pos);
    SetDlgItemText(hDlg,IO_POS_BITS,string);
    sprintf(string,"%0.2f",dtov(unsent_pos,get_range(current_pos)));
    SetDlgItemText(hDlg,IO_POS_VOLTS,string);
    switch(current_pos)
    {
        case x_ch:
            CheckDlgButton(hDlg,IO_X,1);
            CheckDlgButton(hDlg,IO_Y,0);
            CheckDlgButton(hDlg,IO_Z,0);
            sprintf(string,"%0.1f",dtov(unsent_pos,get_range(current_pos)) * A_PER_V);
            SetDlgItemText(hDlg,IO_POS_ANGS,string);
            break;
        case y_ch:
            CheckDlgButton(hDlg,IO_X,0);
            CheckDlgButton(hDlg,IO_Y,1);
            CheckDlgButton(hDlg,IO_Z,0);
            sprintf(string,"%0.1f",dtov(unsent_pos,get_range(current_pos)) * A_PER_V);
            SetDlgItemText(hDlg,IO_POS_ANGS,string);
            break;
        case zo_ch:
            CheckDlgButton(hDlg,IO_X,0);
            CheckDlgButton(hDlg,IO_Y,0);
            CheckDlgButton(hDlg,IO_Z,1);
            sprintf(string,"%0.1f",dtov(unsent_pos,get_range(current_pos)) * Z_A_PER_V);
            SetDlgItemText(hDlg,IO_POS_ANGS,string);
            break;
    }
    if(unsent_pos != dac_data[current_pos])
        EnableWindow(GetDlgItem(hDlg,IO_POS_SEND),TRUE);
    else EnableWindow(GetDlgItem(hDlg,IO_POS_SEND),FALSE);

    out_smart = 0;
}

void repaint_offset(HWND hDlg)
{
    int range;
    
    out_smart = 1;
    range = get_range(current_offset);
    if(range == 1)
    {
        CheckDlgButton(hDlg,IO_OFFSET_RANGE_10,0);
        CheckDlgButton(hDlg,IO_OFFSET_RANGE_5,1);
    }
    else 
    {
        CheckDlgButton(hDlg,IO_OFFSET_RANGE_10,1);
        CheckDlgButton(hDlg,IO_OFFSET_RANGE_5,0);
    }
        
    SetScrollPos(GetDlgItem(hDlg,IO_OFFSET_SCROLL),SB_CTL,(int)unsent_offset,TRUE);
    SetDlgItemText(hDlg,IO_OFFSET_CH,gcvt(current_offset,5,string));
    sprintf(string,"%d",unsent_offset);
    SetDlgItemText(hDlg,IO_OFFSET_BITS,string);
    sprintf(string,"%0.2f",dtov(unsent_offset,get_range(current_offset)));
    SetDlgItemText(hDlg,IO_OFFSET_VOLTS,string);
    switch(current_offset)
    {
        case x_offset_ch:
            CheckDlgButton(hDlg,IO_X_OFFSET,1);
            CheckDlgButton(hDlg,IO_Y_OFFSET,0);
            CheckDlgButton(hDlg,IO_Z_OFFSET,0);
            sprintf(string,"%0.1f",dtov(unsent_offset,get_range(current_offset)) * A_PER_V * 10.0);
            SetDlgItemText(hDlg,IO_OFFSET_ANGS,string);
            break;
        case y_offset_ch:
            CheckDlgButton(hDlg,IO_X_OFFSET,0);
            CheckDlgButton(hDlg,IO_Y_OFFSET,1);
            CheckDlgButton(hDlg,IO_Z_OFFSET,0);
            sprintf(string,"%0.1f",dtov(unsent_offset,get_range(current_offset)) * A_PER_V * 10.0);
            SetDlgItemText(hDlg,IO_OFFSET_ANGS,string);
            break;
        //case z_offset_ch: 
		default: // since z_offset_ch is now a variable, handle z offset (crudely) w/ default case
            CheckDlgButton(hDlg,IO_X_OFFSET,0);
            CheckDlgButton(hDlg,IO_Y_OFFSET,0);
            CheckDlgButton(hDlg,IO_Z_OFFSET,1);
            sprintf(string,"%0.1f",dtov(unsent_offset,get_range(current_offset)) * Z_A_PER_V * 10.0);
            SetDlgItemText(hDlg,IO_OFFSET_ANGS,string);
            break;
    }
    if(unsent_offset != dac_data[current_offset])
        EnableWindow(GetDlgItem(hDlg,IO_OFFSET_SEND),TRUE);
    else EnableWindow(GetDlgItem(hDlg,IO_OFFSET_SEND),FALSE);
    out_smart = 0;
}
                        
void repaint_gains(HWND hDlg)
{
    switch(dac_data[gain_ch] & x_gain_bits)
    {
        case X_TENTH_GAIN:
            CheckDlgButton(hDlg,IO_X_01,1);
            break;            
        case X_ONE_GAIN:
            CheckDlgButton(hDlg,IO_X_1,1);
            break;            
        case X_TEN_GAIN:
            CheckDlgButton(hDlg,IO_X_10,1);
            break;
    }
    
    switch(dac_data[gain_ch] & y_gain_bits)
    {
        case Y_TENTH_GAIN:
            CheckDlgButton(hDlg,IO_Y_01,1);
            break;            
        case Y_ONE_GAIN:
            CheckDlgButton(hDlg,IO_Y_1,1);
            break;            
        case Y_TEN_GAIN:
            CheckDlgButton(hDlg,IO_Y_10,1);
            break;
    }
    
    switch(dac_data[gain_ch] & z_gain_bits)
    {
        case Z_TENTH_GAIN:
            CheckDlgButton(hDlg,IO_Z_01,1);
            break;            
        case Z_ONE_GAIN:
            CheckDlgButton(hDlg,IO_Z_1,1);
            break;            
        case Z_TEN_GAIN:
            CheckDlgButton(hDlg,IO_Z_10,1);
            break;
    }
    
    switch(dac_data[gain_ch] & z2_gain_bits)
    {
        case Z2_TENTH_GAIN:
            CheckDlgButton(hDlg,IO_Z2_01,1);
            break;            
        case Z2_ONE_GAIN:
            CheckDlgButton(hDlg,IO_Z2_1,1);
            break;            
        case Z2_TEN_GAIN:
            CheckDlgButton(hDlg,IO_Z2_10,1);
            break;
    }
    
}

static void repaint_tip_current(HWND hDlg)
{
    SetDlgItemInt(hDlg,TIP_CURRENT_BITS_EDIT,i_setpoint,FALSE);
    SetScrollPos(GetDlgItem(hDlg, TIP_CURRENT_SCROLL),SB_CTL,(int)tip_current_max - i_setpoint,TRUE);
    out_smart = 1;
	sprintf(string,"%.5f",i_set);
    SetDlgItemText(hDlg,TIP_CURRENT_AMPS_EDIT,string);
    out_smart = 0;
}

static void repaint_bias_delay(HWND hDlg)
{
	out_smart = 1;
    SetDlgItemText(hDlg,IO_SQUARE_DELAY,gcvt(bias_delay,7,string));
	out_smart = 0;
}

static void repaint_zoff_ch( HWND hDlg)
{
  if(z_offset_ch == 1) {
	CheckDlgButton(hDlg,IO_ZOFFSET_1,1);
	CheckDlgButton(hDlg,IO_ZOFFSET_7,0);
  }
  else {
	CheckDlgButton(hDlg,IO_ZOFFSET_1,0);
	CheckDlgButton(hDlg,IO_ZOFFSET_7,1);
  }
  repaint_offset(hDlg);
}

static void switch_zoff_ranges_biases()
{
    // switch channel 7  <--> channel 1 
    int ch_7_range;
    int ch_1_range;
    unsigned int ch_7_bias;
    unsigned int ch_1_bias;

    ch_7_range = get_range(7);
    ch_1_range = get_range(1);
    ch_7_bias = dac_data[7];
    ch_1_bias = dac_data[1];

/*
    set_range(7,ch_1_range);
    set_range(1,ch_7_range);
    dio_out(7,ch_1_bias);
    dio_out(1,ch_7_bias);
*/
    set_range_serial(7,ch_1_range);
    set_range_serial(1,ch_7_range);
    dio_out_serial(7,ch_1_bias);
    dio_out_serial(1,ch_7_bias);
}    

static void repaint_bias_ch(HWND hDlg)
{
	if(bit16 == 1) {
		CheckDlgButton(hDlg,IO_BIAS_CH0,0);
		CheckDlgButton(hDlg,IO_BIAS_CH10,1);
		// at present, dsp is programmed to do bias square wave on ch0
		EnableWindow(GetDlgItem(hDlg,BIAS_SQUARE_WAVE),FALSE);
	}
	else {
		CheckDlgButton(hDlg,IO_BIAS_CH0,1);
		CheckDlgButton(hDlg,IO_BIAS_CH10,0);
		EnableWindow(GetDlgItem(hDlg,BIAS_SQUARE_WAVE),TRUE);
	}

}


// ******************************************************************************************
//		O L D * O L D * O L D * O L D * O L D * O L D * O L D * O L D * O L D * O L D
// ******************************************************************************************


#ifdef OLD

void bias_square_wave_old()
{
// Just cut this a segment of old code. Not sure it would compile as a function.
    unsigned int bias_min,bias_max;
    MSG Message2;

	bias_min=vtod(BIAS_SQUARE_MIN,get_range(sample_bias_ch));
                bias_max=vtod(BIAS_SQUARE_MAX,get_range(sample_bias_ch));
                done=0;
                while(!done)
                {
                    bias(bias_min);
                    dio_start_clock(bias_delay);
                   PeekMessage(&Message2, hDlg, NULL, NULL, PM_REMOVE);
                    if (LOWORD(Message2.wParam) == VK_ESCAPE)
                    {
                        sprintf(string, "Sqaure Wave terminated.");
                        MessageBox(hDlg, string, "Warning", MB_ICONEXCLAMATION);
                        done=1;
                        
                    }
                    dio_wait_clock();
                    bias(bias_max);
                    dio_start_clock(bias_delay);
                   PeekMessage(&Message2, hDlg, NULL, NULL, PM_REMOVE);
                    if (LOWORD(Message2.wParam) == VK_ESCAPE)
                    {
                        sprintf(string, "Sqaure Wave terminated.");
                        MessageBox(hDlg, string, "Warning", MB_ICONEXCLAMATION);
                        done=1;
                        
                    }
                    dio_wait_clock();
        
                }
                bias(sample_bias);
}

#endif