#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "stm.h"
#include "dio.h"
#include "serial.h"
#include "highres.h"

unsigned int ch;
unsigned int ch_data;
unsigned int dac_data[16];
unsigned int dac_ranges;
unsigned int out_range;
unsigned int bit16 = 0; // 0 for bias Ch0 (12 bit), 1 for bias Ch10 (16 bit)

extern int z_offset_ch;
extern unsigned int sample_bias;
extern unsigned int i_setpoint;
extern char string[];
extern unsigned int out1;
extern int feedback,x_range,y_range,z_range,x_offset_range,
    y_offset_range,z_offset_range,i_set_range,sample_bias_range;

BOOL FAR PASCAL OutputDlg(HWND,unsigned,WPARAM,LPARAM);
// Allows lowest level (bit by bit) control of output

BOOL FAR PASCAL OutputDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{
  HWND handle;
  int id;
  int i,mini,maxi,delt,ddelt;
  BOOL repaint_ch = FALSE;
  BOOL repaint_data = FALSE;

  switch(Message) {
    case WM_INITDIALOG:
      ch_data = dac_data[ch];
      dac_ranges = dac_data[range_ch];
      handle = GetDlgItem(hDlg,O_CH_SCROLL);
      SetScrollRange(handle,SB_CTL,0,15,FALSE);
      handle = GetDlgItem(hDlg,O_DATA_SCROLL);
      SetScrollRange(handle,SB_CTL,0,4095,FALSE);
      handle = GetDlgItem(hDlg,OUTPUT_EXIT);
      SetFocus(handle);
      repaint_ch = TRUE;
      repaint_data = TRUE;
      break;
    case WM_HSCROLL:
      id = getscrollid();
      switch(id) {
		case O_CH_SCROLL:
		  i = (int) ch;
		  mini = 0;
		  maxi = 15;
		  delt = 1;
		  ddelt = 2;
		  break;
		case O_DATA_SCROLL:
		  i = (int) ch_data;
		  mini = 0;
		  maxi = 4095;
		  delt = 1;
		  ddelt = 100;
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
      switch(id) {
		case O_CH_SCROLL:
		  ch = (unsigned int) i;
		  ch_data = dac_data[ch];
		  repaint_ch = TRUE;
		  break;
		case O_DATA_SCROLL:
		  ch_data = (unsigned int) i;
		  break;
      }
      repaint_data = TRUE;
      break;
    case WM_COMMAND:
      switch (LOWORD(wParam)) {

		case O_CH_EDIT:
		  GetDlgItemText(hDlg,O_CH_EDIT,string,5);
		  i = atoi(string);
		  i = min(max(i,0),15);
		  ch = (unsigned int) i;
		  break;
		case O_DATA_EDIT:
		  GetDlgItemText(hDlg,O_DATA_EDIT,string,5);
		  i = atoi(string);
		  i = min(max(i,0),4095);
		  ch_data = (unsigned int) i;
		  break;
		case O_BITS_EDIT:
		  GetDlgItemText(hDlg,O_BITS_EDIT,string,13);
		  i = btod(string);
		  i = min(max(i,0),4095);
		  ch_data = (unsigned int) i;
		  break;
	    case O_VOLTS_EDIT:
		  GetDlgItemText(hDlg,O_VOLTS_EDIT,string,9);
		  i = vtod(atof(string),get_range(ch));
		  i = min(max(i,0),4095);
		  ch_data = (unsigned int) i;
		  break;
	    case ENTER:
		  repaint_ch = TRUE;
		  repaint_data = TRUE;
		  break;

		case FIVE_RANGE:
		  CheckDlgButton(hDlg,FIVE_RANGE,1);
		  CheckDlgButton(hDlg,TEN_RANGE,0);
		  out_range = 5;
		  dac_ranges |= 1<<ch;
		  dac_data[range_ch] = dac_ranges;
//	  dio_out(range_ch,dac_ranges);
		  dio_out_serial(range_ch,dac_ranges);
		  repaint_data = TRUE;
		  break;
		case TEN_RANGE:
		  CheckDlgButton(hDlg,FIVE_RANGE,0);
		  CheckDlgButton(hDlg,TEN_RANGE,1);
		  out_range = 10;
		  dac_ranges &= ~(1<<ch);
		  dac_data[range_ch] = dac_ranges;
//	  dio_out(range_ch,dac_ranges);
		  dio_out_serial(range_ch,dac_ranges);
		  repaint_data = TRUE;
		  break;

		case SEND_OUTPUT:
		  if(ch==range_ch)
		    dac_ranges = ch_data;
		  dac_data[ch] = ch_data;
//	  dio_out(ch,ch_data);
		  dio_out_serial(ch,ch_data);

		  break;

		case OUTPUT_EXIT:
		  // Update some global variables so changes made here will be reflected
		  // elsewhere. Have tested that changes made here are reflected in io controls
		  // and vice versa.
		  if(bit16 == 0)
		  {
			sample_bias = dac_data[sample_bias_ch];
		  }
		  else
		  {
			sample_bias = dac_data[hires_bias_ch] + ((dac_data[extra_ch] & upper4_mask) << 4);
		  }
		  i_setpoint = dac_data[i_setpoint_ch];
		  feedback = ((dac_data[feedback_ch] & feedback_bit) && 1);
	      x_range = get_range(x_ch);
	      y_range = get_range(y_ch);
	      z_range = get_range(zo_ch);
	      x_offset_range = get_range(x_offset_ch);
	      y_offset_range = get_range(y_offset_ch);
	      z_offset_range = get_range(z_offset_ch);
	      sample_bias_range = get_range(sample_bias_ch);
	      i_set_range = get_range(i_setpoint_ch);
	      EndDialog(hDlg,TRUE);
		  return(TRUE);
	 	  break;
      }
    break;
  }
  if(repaint_ch)
  {
    if(ch>11)
	{
      EnableWindow(GetDlgItem(hDlg,FIVE_RANGE),FALSE);
      EnableWindow(GetDlgItem(hDlg,TEN_RANGE),FALSE);
      EnableWindow(GetDlgItem(hDlg,RANGE),FALSE);
      EnableWindow(GetDlgItem(hDlg,O_VOLTS_EDIT),FALSE);
      CheckDlgButton(hDlg,FIVE_RANGE,0);
      CheckDlgButton(hDlg,TEN_RANGE,0);
    }
    else
	{
      EnableWindow(GetDlgItem(hDlg,FIVE_RANGE),TRUE);
      EnableWindow(GetDlgItem(hDlg,TEN_RANGE),TRUE);
      EnableWindow(GetDlgItem(hDlg,RANGE),TRUE);
      EnableWindow(GetDlgItem(hDlg,O_VOLTS_EDIT),TRUE);
      if(dac_ranges&(1<<ch))
	  {
	    CheckDlgButton(hDlg,FIVE_RANGE,1);
	    CheckDlgButton(hDlg,TEN_RANGE,0);
	    out_range = 5;
	  }
      else
	  {
		CheckDlgButton(hDlg,FIVE_RANGE,0);
		CheckDlgButton(hDlg,TEN_RANGE,10);
		out_range = 10;
      }
    }
    handle = GetDlgItem(hDlg,O_CH_SCROLL);
    SetScrollPos(handle,SB_CTL,ch,TRUE);
    SetDlgItemInt(hDlg,O_CH_EDIT,ch,FALSE);
  }
  if(repaint_data)
  {
    handle = GetDlgItem(hDlg,O_DATA_SCROLL);
    SetScrollPos(handle,SB_CTL,ch_data,TRUE);
    SetDlgItemInt(hDlg,O_DATA_EDIT,ch_data,FALSE);
    SetDlgItemText(hDlg,O_BITS_EDIT,dtob(ch_data));
	sprintf(string,"%.5f",dtov(ch_data,get_range(ch)));
    SetDlgItemText(hDlg,O_VOLTS_EDIT,string);
  }
  return(FALSE);
}
