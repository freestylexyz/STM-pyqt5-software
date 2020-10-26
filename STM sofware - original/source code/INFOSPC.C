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
#include "spec.h"
#include "infospc.h"

int infospc_caller; // spec info dialog may be accessed via Data Manip. or Spec. dialog

extern datadef *gendata[];
extern datadef *second_data;
extern int current_image;
extern HANDLE hInst;
extern char string[];
extern int z_offset_range;

static void enable_track_offsets(HWND,int);
static void enable_lockin_fields(HWND,int);

BOOL FAR PASCAL InfospcDlg(HWND,unsigned,WPARAM,LPARAM);
// Spec info dialog displays prominent data fields of spec datadef

BOOL FAR PASCAL InfospcDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{

  datadef *ldat = NULL;

  switch(Message) {
    case WM_INITDIALOG:
	  switch(infospc_caller) {
	    case INFOSPC_CALLER_IMAGE:
          ldat = gendata[current_image];
	      break;
	    case INFOSPC_CALLER_SPC:
	      ldat = second_data;
	      break;
	  }

      if(ldat != NULL)
	  {
		SetDlgItemText(hDlg,INFOSPC_FILENAME,ldat->filename);
        sprintf(string,"%d",ldat->version);
		SetDlgItemText(hDlg,INFOSPC_VERSION,string);

		// Tunneling Conditions
		sprintf(string,"%0.2f",ldat->temperature);
        SetDlgItemText(hDlg,INFOSPC_TEMPERATURE,string);
        sprintf(string,"%0.4f",dtov(ldat->bias,ldat->bias_range));
        SetDlgItemText(hDlg,INFOSPC_SAMPLE_BIAS,string);
        sprintf(string,"%0.2lf",calc_i_set(ldat->i_setpoint,ldat->i_set_range,ldat->amp_gain));
        SetDlgItemText(hDlg,INFOSPC_I_SETPOINT,string);

		// Scan
        sprintf(string,"%d",ldat->step);
        SetDlgItemText(hDlg,INFOSPC_BITS_PER_STEP,string);
        sprintf(string,"%d",ldat->num_passes);
        SetDlgItemText(hDlg,INFOSPC_PASSES,string);
        sprintf(string,"%d",ldat->move_time);
        SetDlgItemText(hDlg,INFOSPC_MOVE_TIME,string);
        sprintf(string,"%d",(int)ldat->speed);
        SetDlgItemText(hDlg,INFOSPC_READ_TIME,string);
		if(ldat->bit16 && (ldat->output_ch == 10 || ldat->output_ch == 11))
		{
	      sprintf(string,"Ch %d from %.3f to %.3f",ldat->output_ch,
				dtov16(ldat->start),
				dtov16(ldat->start + (ldat->size - 1) * ldat->step));
		}
		else
		{
		  sprintf(string,"Ch %d from %.3f to %.3f",ldat->output_ch,
				dtov(ldat->start,ldat->v_range),
				dtov(ldat->start + (ldat->size - 1) * ldat->step,ldat->v_range));
		}
		SetDlgItemText(hDlg,INFOSPC_OUT,string);
        switch(ldat->type2d)
        {
          case TYPE2D_SPEC_I:
            sprintf(string,"Ch 0");
            break;
          case TYPE2D_SPEC_Z:
            sprintf(string,"Ch 1");
            break;
          case TYPE2D_SPEC_2:
            sprintf(string,"Ch 2");
            break;
          case TYPE2D_SPEC_3:
            sprintf(string,"Ch 3");
            break;
        }
		if(ldat->scan_dir & SP_AVG_BOTH_DIR)
		{
          strcat(string," (averaged both directions)");
		}
		else if(ldat->scan_dir & SP_BACKWARD_DATA)
		{
          strcat(string," (backward direction)");
		}
		else
		{
		  strcat(string," (forward direction)");
		}
        SetDlgItemText(hDlg,INFOSPC_MEASURED,string);

		// Adjustments
		if(ldat->z_offset_bias == ZERO)
		  sprintf(string,"Move %d bits",ldat->delta_z_offset);
		else 		
		  sprintf(string,"Move %d bits at %0.3f V",ldat->delta_z_offset,
		          dtov(ldat->z_offset_bias,ldat->bias_range));
        SetDlgItemText(hDlg,INFOSPC_Z_OFFSET,string);
		switch(ldat->scan_feedback)
		{
		  case 0: //SP_FEEDBACK_OFF
			strcpy(string,"Off");
			break;
		  case 1: //SP_FEEDBACK_ON
			if(ldat->track_every != 0)
			  strcpy(string,"On");
			else 
			  strcpy(string,"Off");
			break;
		  case 2: //SP_FEEDBACK_ON_EVERY
			if(ldat->track_every != 0)
			  sprintf(string,"On every %d passes",ldat->feedback_every);
			else
			  strcpy(string,"Off");
			break;
		}
		SetDlgItemText(hDlg,INFOSPC_FEEDBACK_DESC,string);
        switch(ldat->dither0)
        {
            case SP_DITHER0_OFF:
                sprintf(string,"Off");
                break;
            case SP_DITHER0_ON:
                sprintf(string,"On");
                break;
        }
        SetDlgItemText(hDlg,INFOSPC_DITHER0,string);
        switch(ldat->dither1)
        {
            case SP_DITHER1_OFF:
                sprintf(string,"Off");
                break;
            case SP_DITHER1_ON:
                sprintf(string,"On");
                break;
        }
        SetDlgItemText(hDlg,INFOSPC_DITHER1,string);
        if(ldat->track_every != 0)
		{
          if (ldat->tracking_mode == SP_TRACK_MAX)
		  {
            sprintf(string,"for Max every %d passes",ldat->track_every);
		  }
          else
		  {
			sprintf(string,"for Min every %d passes",ldat->track_every);
		  }

		  SetDlgItemText(hDlg,INFOSPC_TRACK_DESC,string);
		  enable_track_offsets(hDlg,1);
		  sprintf(string,"%d",ldat->track_offset_x);
          SetDlgItemText(hDlg,INFOSPC_TRACK_OFFSET_X,string);
          sprintf(string,"%d",ldat->track_offset_y);
          SetDlgItemText(hDlg,INFOSPC_TRACK_OFFSET_Y,string);
		}
		else
		{
          SetDlgItemText(hDlg,INFOSPC_TRACK_DESC,"Off");
		  enable_track_offsets(hDlg,0);
          SetDlgItemText(hDlg,INFOSPC_TRACK_OFFSET_X,"");
          SetDlgItemText(hDlg,INFOSPC_TRACK_OFFSET_Y,"");
		}
	  }
		  
	  // lockin settings
	  if(ldat->lockin_in_use == LOCKIN_USED)
	  {
		SetDlgItemText(hDlg,INFOSPC_SETTINGS_RECORDED,"Lock-in used"); 
		enable_lockin_fields(hDlg,1);
		sprintf(string,"%.3f",ldat->lockin_sen1);
		SetDlgItemText(hDlg,INFOSPC_SEN1,string); 
		sprintf(string,"%.3f",ldat->lockin_sen2);
		SetDlgItemText(hDlg,INFOSPC_SEN2,string); 
		sprintf(string,"%.1f",ldat->lockin_osc);
		SetDlgItemText(hDlg,INFOSPC_OSC,string); 
		sprintf(string,"%.1f",ldat->lockin_w);
		SetDlgItemText(hDlg,INFOSPC_W,string); 
		sprintf(string,"%.3f",ldat->lockin_tc);
		SetDlgItemText(hDlg,INFOSPC_TC,string); 
		sprintf(string,"%.3f",ldat->lockin_n1);
		SetDlgItemText(hDlg,INFOSPC_N1,string); 
		sprintf(string,"%.2f",ldat->lockin_pha1);
		SetDlgItemText(hDlg,INFOSPC_PHA1,string); 
		sprintf(string,"%.2f",ldat->lockin_pha2);
		SetDlgItemText(hDlg,INFOSPC_PHA2,string); 
	  }
	  else
	  {
		SetDlgItemText(hDlg,INFOSPC_SETTINGS_RECORDED,"Lock-in not used or settings not recorded"); 
		enable_lockin_fields(hDlg,0);
		SetDlgItemText(hDlg,INFOSPC_SEN1,""); 
		SetDlgItemText(hDlg,INFOSPC_SEN2,""); 
		SetDlgItemText(hDlg,INFOSPC_OSC,""); 
		SetDlgItemText(hDlg,INFOSPC_W,""); 
		SetDlgItemText(hDlg,INFOSPC_TC,""); 
		SetDlgItemText(hDlg,INFOSPC_N1,""); 
		SetDlgItemText(hDlg,INFOSPC_PHA1,""); 
		SetDlgItemText(hDlg,INFOSPC_PHA2,""); 
	  }
	  
	  // sample, dosed
      SetDlgItemText(hDlg,INFOSPC_SAMPLE_TYPE,ldat->sample_type.ptr);
      SetDlgItemText(hDlg,INFOSPC_DOSED_WITH,ldat->dosed_type.ptr);
	  
	  // comments are editable text field so they may be scrolled
	  // changes made to comments in this dialog do not get saved
	  SetDlgItemText(hDlg,INFOSPC_COMMENT,ldat->comment.ptr);
 		
      break;

	case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case INFOSPC_EXIT:
          EndDialog(hDlg,TRUE);
          return(TRUE);
          break;
	  }
      break;
  }
  return(FALSE);
}

static void enable_track_offsets(HWND hDlg,int status)
{
  EnableWindow(GetDlgItem(hDlg,INFOSPC_TRACK_OFFSET_X),status);
  EnableWindow(GetDlgItem(hDlg,INFOSPC_TRACK_OFFSET_Y),status);
}

static void enable_lockin_fields(HWND hDlg,int status)
{
  EnableWindow(GetDlgItem(hDlg,INFOSPC_SEN1),status);
  EnableWindow(GetDlgItem(hDlg,INFOSPC_SEN2),status);
  EnableWindow(GetDlgItem(hDlg,INFOSPC_OSC),status);
  EnableWindow(GetDlgItem(hDlg,INFOSPC_W),status);
  EnableWindow(GetDlgItem(hDlg,INFOSPC_TC),status);
  EnableWindow(GetDlgItem(hDlg,INFOSPC_N1),status);
  EnableWindow(GetDlgItem(hDlg,INFOSPC_PHA1),status);
  EnableWindow(GetDlgItem(hDlg,INFOSPC_PHA2),status);
}

