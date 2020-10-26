#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "common.h"
#include "stm.h"
#include "dio.h"
#include "coarse.h"

BOOL coarse_edit_enable;
unsigned int step_bits;
unsigned int period;
unsigned int num_steps;
double coarse_x;
double coarse_y;
unsigned int wave;

extern int z_offset_ch;
extern char string[];
extern unsigned int dac_data[];
extern int feedback,x_range,y_range,z_range,x_offset_range,
    y_offset_range,z_offset_range,i_set_range,sample_bias_range;

BOOL FAR PASCAL CoarseDlg(HWND,unsigned,WPARAM,LPARAM);

BOOL FAR PASCAL CoarseDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{
  int id;
  int i,mini,maxi,delt,ddelt;
  
  switch(Message) {
    case WM_INITDIALOG:
      pre_coarse();
      CheckDlgButton(hDlg,COARSE_TRANS_X,1);
      if(wave==paracut)
	CheckDlgButton(hDlg,COARSE_PARACUT,1);
      else
	CheckDlgButton(hDlg,COARSE_SAWTOOTH,1);
      if(coarse_edit_enable)
	CheckDlgButton(hDlg,COARSE_EDIT_ENABLE,1);
      SetScrollRange(GetDlgItem(hDlg,COARSE_STEPS_SCROLL),SB_CTL,coarse_step_min,coarse_step_max,FALSE);
      SetScrollRange(GetDlgItem(hDlg,COARSE_PERIOD_SCROLL),SB_CTL,coarse_period_min,coarse_period_max,FALSE);
      SetScrollRange(GetDlgItem(hDlg,COARSE_NUM_STEPS_SCROLL),SB_CTL,coarse_num_steps_min,coarse_num_steps_max,FALSE);
      SetFocus(GetDlgItem(hDlg,COARSE_EXIT));
      repaint_step(hDlg,step_bits);
      repaint_period(hDlg,period);
      repaint_position(hDlg,coarse_x,coarse_y);
      repaint_num(hDlg,period,step_bits,num_steps);
      coarse_window_enable(hDlg,coarse_edit_enable);
      break;
    case WM_HSCROLL:
      id = getscrollid();
      switch(id) {
	case COARSE_STEPS_SCROLL:
	  i =  (int) step_bits;
	  mini = coarse_step_min;
	  maxi = coarse_step_max;
	  delt = coarse_step_delt;
	  ddelt = coarse_step_ddelt;
	  break;
	case COARSE_PERIOD_SCROLL:
	  i =  (int) period;
	  mini = coarse_period_min;
	  maxi = coarse_period_max;
	  delt = coarse_period_delt;
	  ddelt = coarse_period_ddelt;
	  break;
	case COARSE_NUM_STEPS_SCROLL:
	  i = (int) num_steps;
	  mini = coarse_num_steps_min;
	  maxi = coarse_num_steps_max;
	  delt = coarse_num_steps_delt;
	  ddelt = coarse_num_steps_ddelt;
	  break;
      }
      switch( getscrollcode() ) 
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
      switch(id) {
	case COARSE_STEPS_SCROLL:
	  step_bits = (unsigned int) i;
	  repaint_step(hDlg,step_bits);
	  repaint_num(hDlg,period,step_bits,num_steps);
	  break;
	case COARSE_PERIOD_SCROLL:
	  period = (unsigned int) i;
	  repaint_period(hDlg,period);
	  repaint_num(hDlg,period,step_bits,num_steps);
	  break;
	case COARSE_NUM_STEPS_SCROLL:
	  num_steps = (unsigned int) i;
	  repaint_num(hDlg,period,step_bits,num_steps);
	  break;
      }
      break;
    case WM_COMMAND:
      switch (LOWORD(wParam)) {
	case COARSE_EDIT_ENABLE:
	  if(coarse_edit_enable)
	    coarse_edit_enable = FALSE;
	  else
	    coarse_edit_enable = TRUE;
	  coarse_window_enable(hDlg,coarse_edit_enable);
	  break;
	case COARSE_PARACUT:
	  wave = paracut;
	  break;
	case COARSE_SAWTOOTH:
	  wave = sawtooth;
	  break;
	case COARSE_STEPS_EDIT:
	  GetDlgItemText(hDlg,COARSE_STEPS_EDIT,string,5);
	  i = atoi(string);
	  i = min(max(i,coarse_step_min),coarse_step_max);
	  step_bits = (unsigned int) i;
	  break;
	case COARSE_ANGS_EDIT:
	  GetDlgItemText(hDlg,COARSE_ANGS_EDIT,string,9);
	  i = (int) (atof(string)/(double) ANGS_PER_BIT+0.5);
	  i = min(max(i,coarse_step_min),coarse_step_max);
	  step_bits = (unsigned int) i;
	  break;
	case COARSE_PERIOD_EDIT:
	  GetDlgItemText(hDlg,COARSE_PERIOD_EDIT,string,5);
	  i = atoi(string);
	  i = min(max(i,coarse_period_min),coarse_period_max);
	  period = (unsigned int) i;
	  break;
	case COARSE_FREQ_EDIT:
	  GetDlgItemText(hDlg,COARSE_FREQ_EDIT,string,7);
	  i = (int) ((double) FREQ_PER_BIT/atof(string)+0.5);
	  i = min(max(i,coarse_period_min),coarse_period_max);
	  period = (unsigned int) i;
	  break;
	case COARSE_NUM_STEPS_EDIT:
	  GetDlgItemText(hDlg,COARSE_NUM_STEPS_EDIT,string,6);
	  i = atoi(string);
	  i = min(max(i,coarse_num_steps_min),coarse_num_steps_max);
	  num_steps = (unsigned int) i;
	  break;
	case ENTER:
	  repaint_step(hDlg,step_bits);
	  repaint_period(hDlg,period);
	  repaint_num(hDlg,period,step_bits,num_steps);
	  break;
	case COARSE_LSTEP:
	  coarse_go(hDlg,wave,period,step_bits,1,LEFT);
	  break;
	case COARSE_RSTEP:
	  coarse_go(hDlg,wave,period,step_bits,1,RIGHT);
	  break;
	case COARSE_LGO:
	  coarse_go(hDlg,wave,period,step_bits,num_steps,LEFT);
	  break;
	case COARSE_RGO:
	  coarse_go(hDlg,wave,period,step_bits,num_steps,RIGHT);
	  break;
	case COARSE_X_RESET:
	  coarse_x = 0;
	  repaint_position(hDlg,coarse_x,coarse_y);
	  break;
	case COARSE_Y_RESET:
	  coarse_y = 0;
	  repaint_position(hDlg,coarse_x,coarse_y);
	  break;
	case COARSE_EXIT:
	  post_coarse();
	  EndDialog(hDlg,TRUE);
	  return(TRUE);
      }
    break;
  }
  return(FALSE);
}

void repaint_step(HWND hDlg,unsigned int step_bits)
{
  SetScrollPos(GetDlgItem(hDlg,COARSE_STEPS_SCROLL),SB_CTL,step_bits,TRUE);
  SetDlgItemInt(hDlg,COARSE_STEPS_EDIT,step_bits,FALSE);
  SetDlgItemText(hDlg,COARSE_ANGS_EDIT,gcvt(step_bits*ANGS_PER_BIT,5,string));
}

void repaint_period(HWND hDlg,unsigned int period)
{
  SetScrollPos(GetDlgItem(hDlg,COARSE_PERIOD_SCROLL),SB_CTL,period,TRUE);
  SetDlgItemInt(hDlg,COARSE_PERIOD_EDIT,period,FALSE);
  SetDlgItemText(hDlg,COARSE_FREQ_EDIT,gcvt((double) FREQ_PER_BIT/period,5,string));
}

void repaint_position(HWND hDlg,double coarse_x,double coarse_y)
{
    SetDlgItemText(hDlg,COARSE_X_EDIT,GetDistance(coarse_x));
    SetDlgItemText(hDlg,COARSE_Y_EDIT,GetDistance(coarse_y));
}

void repaint_num(HWND hDlg,unsigned int period,unsigned int step_bits,unsigned int num_steps)
{
  SetDlgItemText(hDlg,COARSE_TIME_EDIT,GetTime((double) num_steps*period/FREQ_PER_BIT));
  SetDlgItemText(hDlg,COARSE_DISTANCE_EDIT,GetDistance((double) num_steps*step_bits*ANGS_PER_BIT));
  SetScrollPos(GetDlgItem(hDlg,COARSE_NUM_STEPS_SCROLL),SB_CTL,num_steps,TRUE);
  SetDlgItemInt(hDlg,COARSE_NUM_STEPS_EDIT,num_steps,FALSE);
}

void coarse_go(HWND hDlg,unsigned int wave,unsigned int period,unsigned int step_bits,unsigned int num_steps,int dir)
{
  int           i;
  unsigned int  *data;
  MSG           Message;

  EnableWindow(GetDlgItem(hDlg,COARSE_LGO),FALSE);
  EnableWindow(GetDlgItem(hDlg,COARSE_RGO),FALSE);
  if(IsDlgButtonChecked(hDlg,COARSE_TRANS_X)) {
    data = dio_blk_setup(x_ch,wave,period,dtov(ZERO+step_bits,x_range),dir);
    move_to(x_ch,ZERO,*data&0xFFF);
  }
  else {
    data = dio_blk_setup(y_ch,wave,period,dtov(ZERO+step_bits,y_range),dir);
    move_to(y_ch,ZERO,*data&0xFFF);
  }
  for(i=0;i<num_steps;i++) {
    dio_blk_out(period,data);
    PeekMessage(&Message,hDlg,NULL,NULL,PM_REMOVE);
    if(LOWORD(Message.wParam)==VK_ESCAPE) {
      sprintf(string,"Stepping terminated\nafter %d steps.",i);
      MessageBox(hDlg,string,"Warning",MB_ICONEXCLAMATION);
      break;
    }
  }
  if(IsDlgButtonChecked(hDlg,COARSE_TRANS_X)) {
    move_to(x_ch,*(data+period-2)&0xFFF,ZERO);
    coarse_x += (double) dir*step_bits*ANGS_PER_BIT*i;
  }
  else {
    move_to(y_ch,*(data+period-2)&0xFFF,ZERO);
    coarse_y += (double) dir*step_bits*ANGS_PER_BIT*i;
  }
  dio_blk_free(data);
  repaint_position(hDlg,coarse_x,coarse_y);
  EnableWindow(GetDlgItem(hDlg,COARSE_LGO),TRUE);
  EnableWindow(GetDlgItem(hDlg,COARSE_RGO),TRUE);
}

char *GetDistance(double distance)
{
  if(fabs(distance)<1e-5)
    distance = 0;
  if(fabs(distance)>1e7) {
    gcvt(distance*1e-7,5,string);
    strcat(string," mm");
  }
  else if(fabs(distance)>1e4) {
    gcvt(distance*1e-4,5,string);
    strcat(string," um");
  }
  else {
    gcvt(distance,5,string);
    strcat(string," A");
  }
  return(string);
}

char *GetTime(double time)
{
  if(time<1e-3) {
    gcvt(time*1e6,5,string);
    strcat(string," usec");
  }
  else if(time<1) {
    gcvt(time*1e3,5,string);
    strcat(string," msec");
  }
  else {
    gcvt(time,5,string);
    strcat(string," sec");
  }
  return(string);
}

void coarse_window_enable(HWND hDlg,BOOL enable)
{
  if(enable) {
    EnableWindow(GetDlgItem(hDlg,COARSE_WAVE_GROUP),TRUE);
    EnableWindow(GetDlgItem(hDlg,COARSE_STEP_GROUP),TRUE);
    EnableWindow(GetDlgItem(hDlg,COARSE_PERIOD_GROUP),TRUE);
    EnableWindow(GetDlgItem(hDlg,COARSE_STEPS_EDIT),TRUE);
    EnableWindow(GetDlgItem(hDlg,COARSE_STEPS_SCROLL),TRUE);
    EnableWindow(GetDlgItem(hDlg,COARSE_ANGS_EDIT),TRUE);
    EnableWindow(GetDlgItem(hDlg,COARSE_PERIOD_EDIT),TRUE);
    EnableWindow(GetDlgItem(hDlg,COARSE_PERIOD_SCROLL),TRUE);
    EnableWindow(GetDlgItem(hDlg,COARSE_FREQ_EDIT),TRUE);
    EnableWindow(GetDlgItem(hDlg,COARSE_PARACUT),TRUE);
    EnableWindow(GetDlgItem(hDlg,COARSE_SAWTOOTH),TRUE);
/*
    EnableWindow(GetDlgItem(hDlg,COARSE_TEXT1),TRUE);
    EnableWindow(GetDlgItem(hDlg,COARSE_TEXT2),TRUE);
    EnableWindow(GetDlgItem(hDlg,COARSE_TEXT3),TRUE);
    EnableWindow(GetDlgItem(hDlg,COARSE_TEXT4),TRUE);
    EnableWindow(GetDlgItem(hDlg,COARSE_TEXT5),TRUE);
*/
  }
  else {
    EnableWindow(GetDlgItem(hDlg,COARSE_WAVE_GROUP),FALSE);
    EnableWindow(GetDlgItem(hDlg,COARSE_STEP_GROUP),FALSE);
    EnableWindow(GetDlgItem(hDlg,COARSE_PERIOD_GROUP),FALSE);
    EnableWindow(GetDlgItem(hDlg,COARSE_STEPS_EDIT),FALSE);
    EnableWindow(GetDlgItem(hDlg,COARSE_STEPS_SCROLL),FALSE);
    EnableWindow(GetDlgItem(hDlg,COARSE_ANGS_EDIT),FALSE);
    EnableWindow(GetDlgItem(hDlg,COARSE_PERIOD_EDIT),FALSE);
    EnableWindow(GetDlgItem(hDlg,COARSE_PERIOD_SCROLL),FALSE);
    EnableWindow(GetDlgItem(hDlg,COARSE_FREQ_EDIT),FALSE);
    EnableWindow(GetDlgItem(hDlg,COARSE_PARACUT),FALSE);
    EnableWindow(GetDlgItem(hDlg,COARSE_SAWTOOTH),FALSE);
/*
    EnableWindow(GetDlgItem(hDlg,COARSE_TEXT1),FALSE);
    EnableWindow(GetDlgItem(hDlg,COARSE_TEXT2),FALSE);
    EnableWindow(GetDlgItem(hDlg,COARSE_TEXT3),FALSE);
    EnableWindow(GetDlgItem(hDlg,COARSE_TEXT4),FALSE);
    EnableWindow(GetDlgItem(hDlg,COARSE_TEXT5),FALSE);
*/
  }
}

void pre_coarse()
{
  hold(TRUE);
  mode(coarse_mode);
  mode(translate_mode);
  move_to(zo_ch,ZERO,MAX);
  move_to(x_ch,dac_data[x_ch],ZERO);
  move_to(y_ch,dac_data[y_ch],ZERO);
  move_to(x_offset_ch,dac_data[x_offset_ch],ZERO);
  move_to(y_offset_ch,dac_data[y_offset_ch],ZERO);
  move_to(z_offset_ch,dac_data[z_offset_ch],ZERO);
  set_gain(X_TEN_GAIN,Y_TEN_GAIN,Z_ONE_GAIN,Z2_ONE_GAIN);
}

void post_coarse()
{
  move_to(z_offset_ch,ZERO,dac_data[z_offset_ch]);
  move_to(y_offset_ch,ZERO,dac_data[y_offset_ch]);
  move_to(x_offset_ch,ZERO,dac_data[x_offset_ch]);
  move_to(y_ch,ZERO,dac_data[y_ch]);
  move_to(x_ch,ZERO,dac_data[x_ch]);
  move_to(zo_ch,MAX,ZERO);
  mode(fine_mode);
  mode(translate_mode);
  hold(FALSE);
}
