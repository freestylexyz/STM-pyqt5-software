#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <dos.h>
#include <sys\types.h>
#include <direct.h>
#include "common.h"
#include "stm.h"
#include "dio.h"
#include "data.h"
#include "summary.h"
#include "pre_scan.h"
#include "scan.h"

extern datadef **glob_data;
extern int sample_bias_range;
extern datadef *scan_defaults_data[];
extern int scan_current_default;
extern unsigned int dac_data[];

BOOL FAR PASCAL SummaryDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{

  char string[5000],temp[500];  
  int i = 0;
  char ret[5]; // string for a newline (carriage return)
  int ini_feedback,ini_dither0,ini_dither1;
  unsigned int ini_outch[16];
  int cur_feedback,cur_dither0,cur_dither1;
  unsigned int cur_outch[16];

  switch(Message) {
    case WM_INITDIALOG:
	  ret[0] = 0x0D;
      ret[1] = 0x0A;
      ret[2] = '\0';

      for(i = 0;i < 16;i++)
		  cur_outch[i] = ini_outch[i] = dac_data[i];
	  cur_feedback = ini_feedback = (*glob_data)->scan_feedback;
	  cur_dither0 = ini_dither0 = (*glob_data)->scan_dither0;
	  cur_dither1 = ini_dither1 = (*glob_data)->scan_dither1;
	  i = 0;

	  SetWindowText(hDlg,"Sequence Summary");
      sprintf(temp,"Before scan: Feedback is ");
      if((*glob_data)->scan_feedback) strcat(temp,"On.");
      else strcat(temp,"Off.");
      strcat(temp," Dither Ch0 is ");
      if((*glob_data)->scan_dither0) strcat(temp,"On.");
      else strcat(temp,"Off.");
	  strcat(temp," Dither Ch1 is ");
      if((*glob_data)->scan_dither1) strcat(temp,"On.");
      else strcat(temp,"Off.");
      strcpy(string,temp);
      sprintf(temp,"%s",ret);
      strcat(string,temp);
      strcat(string,temp);

      sprintf(temp,"%d) Wait %d (step delay) %s",i,(*glob_data)->step_delay,ret);
      strcat(string,temp);
	  
/* DIGITAL FEEDBACK NOT IMPLEMENTED WITH DSP
      if((*glob_data)->digital_feedback) 
      {
        sprintf(temp,"Wait for current to settle (digital feedback).%s",ret);
        strcat(string,temp);
      }
*/

	  for(i = 0;i < SD->total_steps;i++)
	  {
	    switch(SD->sequence[i].type)
		{
		  case WAIT_TYPE:
            sprintf(temp,"%d) Wait %d.%s",i + 1,SD->sequence[i].wait,ret);
            strcat(string,temp);
			break;
		  case READ_TYPE:
            sprintf(temp,"%d) Read Ch. %d, averaged %d times.%s",i + 1,
                    SD->sequence[i].in_ch,SD->sequence[i].num_reads,ret);
            strcat(string,temp);
			break;
		  case STEP_OUTPUT_TYPE:
			sprintf(temp,"%d) Step ch %d to %f volts.%s",i + 1,SD->sequence[i].out_ch,
				    dtov(SD->sequence[i].step_bias,get_range(SD->sequence[i].out_ch)),
					ret);
            strcat(string,temp);
			cur_outch[SD->sequence[i].out_ch] = SD->sequence[i].step_bias;
			break;
		  case DITHER0_TYPE:
		    if(SD->sequence[i].state)
			{
			  sprintf(temp,"%d) Turn ch. 0 dither on.%s",i + 1,ret);
			  cur_dither0 = 1; 
            }
			else
			{
			  sprintf(temp,"%d) Turn ch. 0 dither off.%s",i + 1,ret);
			  cur_dither0 = 0;
			}
		    strcat(string,temp);
			break;
		  case DITHER1_TYPE:
		    if(SD->sequence[i].state)
			{
			  sprintf(temp,"%d) Turn ch. 1 dither on.%s",i + 1,ret);
			  cur_dither1 = 1; 
            }
			else
			{
			  sprintf(temp,"%d) Turn ch. 1 dither off.%s",i + 1,ret);
			  cur_dither1 = 0; 
			}
		    strcat(string,temp);
			break;
		  case FEEDBACK_TYPE:
		    if(SD->sequence[i].state)
			{
			  sprintf(temp,"%d) Turn feedback on.%s",i + 1,ret);
			  cur_feedback = 1;
            }
			else
			{
			  sprintf(temp,"%d) Turn feedback off.%s",i + 1,ret);
			  cur_feedback = 0;
			}
		    strcat(string,temp);
			break;
		}
	  }

	  for(i = 0;i < 16;i++)
	  {
		if(cur_outch[i] != ini_outch[i])
		{
		  sprintf(temp,"%sWarning!!! Final output not equal to initial value on ch %d.%s",
			  ret,i,ret);
		  strcat(string,temp);
		}
	  }
	  if(cur_feedback != ini_feedback)
	  {
		  sprintf(temp,"%sWarning!!! Final feedback state not equal to initial state.%s",
			  ret,ret);
		  strcat(string,temp);
	  }
	  if(cur_dither0 != ini_dither0)
	  {
		  sprintf(temp,"%sWarning!!! Final dither0 state not equal to initial state.%s",
			  ret,ret);
		  strcat(string,temp);
	  }
	  if(cur_dither1 != ini_dither1)
	  {
		  sprintf(temp,"%sWarning!!! Final dither1 state not equal to initial state.%s",
			  ret,ret);
		  strcat(string,temp);
	  }

      SetDlgItemText(hDlg,COMMENT_EDIT,string);
      SetFocus(GetDlgItem(hDlg,COMMENT_EXIT));
      break;

    case WM_COMMAND:
      switch (LOWORD(wParam)) {
        case COMMENT_EXIT:
          EndDialog(hDlg,TRUE);
          return(TRUE);
      }
      break;
  }
  return(FALSE);
}

