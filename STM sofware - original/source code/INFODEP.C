#include <time.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include <i86.h>
#include <string.h>
#include "file.h"
#include "data.h"
#include "dio.h"
#include "stm.h"
#include "pre_scan.h"
#include "scan.h"
#include "image.h"
#include "common.h"
#include "infodep.h"

BOOL FAR PASCAL InfodepDlg(HWND,unsigned,WPARAM,LPARAM);

int infodep_caller;

extern datadef *gendata[];
extern int current_image;
extern datadef *dep_oscill_data[];
extern int dep_current_channel;
extern HANDLE hInst;
extern char     string[];
extern int z_offset_range;

BOOL FAR PASCAL InfodepDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{
    unsigned int temp_z_scale;
    float point1=0.1;
    datadef *ldat;

//    int i;
//    int id;
//    int delt,ddelt,mini,maxi;

  switch(Message) {
    case WM_INITDIALOG:
	if( infodep_caller == INFODEP_CALLER_IMAGE) {
          ldat=gendata[current_image];
	}
	else {
	  ldat=dep_oscill_data[dep_current_channel];
	}
        SetDlgItemText(hDlg,INFODEP_FILENAME,ldat->filename);
        sprintf(string,"%d",ldat->version);
        wprint(INFODEP_VERSION);
        /* version 1 stuff: */
        SetDlgItemText(hDlg,INFODEP_COMMENT,ldat->comment.ptr);
        sprintf(string,"%0.3f",dtov_len(ldat->delta_z_offset,z_offset_range));
        wprint(INFODEP_Z_OFFSET);
        SetDlgItemText(hDlg,INFODEP_COMMENT,ldat->comment.ptr);
        SetDlgItemText(hDlg,INFODEP_SAMPLE_TYPE,ldat->sample_type.ptr);
        sprintf(string,"%0.4f",ldat->dosed_langmuir);
        wprint(INFODEP_DOSED_LANG);
        SetDlgItemText(hDlg,INFODEP_DOSED_WITH,ldat->dosed_type.ptr);
        sprintf(string,"%0.3f",dtov(ldat->bias,ldat->bias_range));
        wprint(INFODEP_BIAS);
        sprintf(string,"%0.2lf",calc_i_set(ldat->i_setpoint,
            ldat->i_set_range,ldat->amp_gain));
        wprint(INFODEP_CURRENT);
        sprintf(string,"%d",ldat->amp_gain);
        wprint(INFODEP_AMP_GAIN);
        if (ldat->z_gain==point1) temp_z_scale=0;
        else if (ldat->z_gain==1) temp_z_scale=1;
        else if (ldat->z_gain==10) temp_z_scale=2;
        scan_freq_str(string,temp_z_scale,ldat->z_freq);
        wprint(INFODEP_Z_FREQ);
        sprintf(string,"%0.2f",ldat->temperature);
        wprint(INFODEP_TEMPERATURE);
        sprintf(string,"%d",ldat->move_time);
        wprint(INFODEP_MOVE_TIME);
        sprintf(string,"%d",ldat->write_time);
        wprint(INFODEP_WRITE_TIME);
        sprintf(string,"%0.3f",dtov(ldat->move_bias,ldat->bias_range));
        wprint(INFODEP_MOVE_BIAS);
        sprintf(string,"%0.3f",dtov(ldat->write_bias,ldat->bias_range));
        wprint(INFODEP_WRITE_BIAS);
        sprintf(string,"%0.3f",
            calc_i_set(ldat->move_i_setpoint,ldat->i_set_range,ldat->amp_gain));
        wprint(INFODEP_MOVE_CURRENT);
        sprintf(string,"%0.3f",
            calc_i_set(ldat->write_i_setpoint,ldat->i_set_range,ldat->amp_gain));
        wprint(INFODEP_WRITE_CURRENT);
        sprintf(string,"%d",ldat->pulse_duration);
        wprint(INFODEP_PULSE_DURATION);
        sprintf(string,"%0.3f",ldat->bias_speed);          
        if (ldat->bias_speed_step) strcpy(string,"Step");
        wprint(INFODEP_BIAS_SPEED);
        sprintf(string,"%0.3f",ldat->current_speed);
        wprint(INFODEP_CURRENT_SPEED);
        sprintf(string,"%0.3f",ldat->z_offset_speed);
        wprint(INFODEP_Z_OFFSET_SPEED);
        switch (ldat->scan_feedback)
        {
            case DEP_PULSE_FEEDBACK_ON:
                strcpy(string,"On");
                break;
            case DEP_PULSE_FEEDBACK_OFF:
                strcpy(string,"Off");
                break;
            case DEP_PULSE_FEEDBACK_AFTER:
                sprintf(string,"After %d",ldat->feedback_wait);
                break;
        }
        wprint(INFODEP_FEEDBACK);
        if (ldat->version>1)
        {
            if (ldat->z_offset_crash_protect)
                strcpy(string,"Yes");
            else strcpy(string,"No");
            wprint(INFODEP_Z_CRASH_PROT);
            if (ldat->z_offset_move)
                strcpy(string,"Yes");
            else strcpy(string,"No");
            wprint(INFODEP_Z_OFFSET_MOVEMENT_BIAS);
        }
/*
        sprintf(string,"%f %f",ldat->min_z,ldat->max_z);
        wprint(INFODEP_SAMPLE_TYPE);
*/
        if (ldat->version>2)
        {
            string[0]='\0';
            if (ldat->measure_i)
                strcpy(string,"Current");
            if (ldat->measure_z) strcpy(string,"Z");
            if (ldat->measure_2) strcpy(string,"2");
            if (ldat->measure_3) strcpy(string,"3");
            wprint(INFODEP_MEASURED);
        }
        if (ldat->version>3)
        {
            sprintf(string,"%d",ldat->scan_num);
            wprint(INFODEP_SAMPLES);
        }
        if (ldat->version<4)
        {
            strcpy(string,"NA");
            wprint(INFODEP_SAMPLES);
        }
        if (ldat->version<3)
        {
            strcpy(string,"NA");
            wprint(INFODEP_MEASURED);
        }
        if (ldat->version<2)
        {
            strcpy(string,"NA");
            wprint(INFODEP_Z_CRASH_PROT);
            wprint(INFODEP_Z_OFFSET_MOVEMENT_BIAS);
        }       
      break;

#ifdef OLD
    case WM_HSCROLL:
    case WM_VSCROLL:
      id = getscrollid();
      switch(id) {      
/*
        case IM_X_SCROLL:
          i = (int) im_src_x[current_image];
          mini = 0;
          maxi = gendata[current_image]->size-im_screenlines[current_image];
          delt = IM_SCROLL_DELT;
          ddelt = IM_SCROLL_DDELT;
          break;
        case IM_Y_SCROLL:
          i = (int) im_src_y[current_image];
          mini = 0;
          maxi = gendata[current_image]->size-im_screenlines[current_image];
          delt = -IM_SCROLL_DELT;
          ddelt = -IM_SCROLL_DDELT;
          break;
*/
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
      switch(id)  {
/*
        case IM_X_SCROLL:
          im_src_x[current_image] = (int) i;
          SetScrollPos(GetDlgItem(hDlg,IM_X_SCROLL),SB_CTL,im_src_x[current_image],TRUE);
          repaint_image(hDlg,current_image,NULL);
          break;
        case IM_Y_SCROLL:
          im_src_y[current_image] = (int) i;
          SetScrollPos(GetDlgItem(hDlg,IM_Y_SCROLL),SB_CTL,
            maxi-i,TRUE);
          repaint_image(hDlg,current_image,NULL);
          break;
*/
      }
      break;
#endif

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
            case INFODEP_EXIT:
                EndDialog(hDlg,TRUE);
                return(TRUE);
                break;
    }
    break;
  }
  return(FALSE);
}

