#include <windows.h>
#include <stdio.h>
#include "common.h"
#include "stm.h"
#include "spec.h"

extern datadef *sp_data[];
extern int sp_current;
extern datadef *second_data;
extern sp_cur_selection;

extern char string[];

float lockin_sen1 = 0.0;
float lockin_sen2 = 0.0;
float lockin_osc = 0.0;
float lockin_w = 0.0;
float lockin_tc = 0.0;
float lockin_n1 = 0.0;
float lockin_pha1 = 0.0;
float lockin_pha2 = 0.0;
int lockin_in_use = LOCKIN_NOT_USED;

static int out_smart = 0;	// currently not necessary, but around for future use

static void repaint_all(HWND);
static void get_lockin_data(datadef *);
static void set_lockin_data(datadef *);

BOOL FAR PASCAL LockinDlg(HWND,unsigned,WPARAM,LPARAM);
// Lock-in Settings dialog for recording lock-in parameters for spectroscopy

BOOL FAR PASCAL LockinDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{

  switch(Message) {
    case WM_INITDIALOG:
	  // fetch lockin settings from current spec selection
	  if(sp_cur_selection == SP_UPPER_IMAGE)
	  {
	    get_lockin_data(sp_data[sp_current]);
	  }
	  else if(sp_cur_selection == SP_LOWER_IMAGE)
	  {
	    get_lockin_data(second_data);
	  }
	  else {
        lockin_sen1 = 0.0;
        lockin_sen2 = 0.0;
        lockin_osc = 0.0;
        lockin_w = 0.0;
        lockin_tc = 0.0;
        lockin_n1 = 0.0;
        lockin_pha1 = 0.0;
        lockin_pha2 = 0.0;
		lockin_in_use = LOCKIN_NOT_USED;
	  }

      repaint_all(hDlg);
      break;

    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case LOCKIN_USED:
        case LOCKIN_NOT_USED:
            lockin_in_use = LOWORD(wParam);
            break;

	    case LOCKIN_EXIT:
		  // stuff lock-in parameters into current spec selection before exit
		  if(sp_cur_selection == SP_UPPER_IMAGE)
		  {
	        set_lockin_data(sp_data[sp_current]);
		  }
	      if(sp_cur_selection == SP_LOWER_IMAGE)
		  {
	        set_lockin_data(second_data);
		  }

          EndDialog(hDlg,TRUE);
          return(TRUE);
          break;

		case ENTER:
          // edit text parameters not copied until user hits enter
          GetDlgItemText(hDlg,LOCKIN_SEN1,string,10);
          lockin_sen1 = (float)atof(string);
          GetDlgItemText(hDlg,LOCKIN_SEN2,string,10);
          lockin_sen2 = (float)atof(string);
          GetDlgItemText(hDlg,LOCKIN_OSC,string,10);
          lockin_osc = (float)atof(string);
          GetDlgItemText(hDlg,LOCKIN_W,string,10);
          lockin_w = (float)atof(string);
          GetDlgItemText(hDlg,LOCKIN_TC,string,10);
          lockin_tc = (float)atof(string);
          GetDlgItemText(hDlg,LOCKIN_N1,string,10);
          lockin_n1 = (float)atof(string);
          GetDlgItemText(hDlg,LOCKIN_PHA1,string,10);
          lockin_pha1 = (float)atof(string);
          GetDlgItemText(hDlg,LOCKIN_PHA2,string,10);
          lockin_pha2 = (float)atof(string);
		  repaint_all(hDlg);
		  break;
      }
      break;
  }
  return(FALSE);
}
 
static void repaint_all(HWND hDlg)
{
  out_smart = 1;

  sprintf(string,"%.3f",lockin_sen1);
  SetDlgItemText(hDlg,LOCKIN_SEN1,string); 
  sprintf(string,"%.3f",lockin_sen2);
  SetDlgItemText(hDlg,LOCKIN_SEN2,string); 
  sprintf(string,"%.1f",lockin_osc);
  SetDlgItemText(hDlg,LOCKIN_OSC,string); 
  sprintf(string,"%.1f",lockin_w);
  SetDlgItemText(hDlg,LOCKIN_W,string); 
  sprintf(string,"%.3f",lockin_tc);
  SetDlgItemText(hDlg,LOCKIN_TC,string); 
  sprintf(string,"%.3f",lockin_n1);
  SetDlgItemText(hDlg,LOCKIN_N1,string); 
  sprintf(string,"%.2f",lockin_pha1);
  SetDlgItemText(hDlg,LOCKIN_PHA1,string); 
  sprintf(string,"%.2f",lockin_pha2);
  SetDlgItemText(hDlg,LOCKIN_PHA2,string); 
  if(lockin_in_use == LOCKIN_USED)
  {
    CheckDlgButton(hDlg,LOCKIN_USED,1);
    CheckDlgButton(hDlg,LOCKIN_NOT_USED,0);
  }
  else
  {
    CheckDlgButton(hDlg,LOCKIN_USED,0);
    CheckDlgButton(hDlg,LOCKIN_NOT_USED,1);
  }

  out_smart = 0;
}    

void get_lockin_data(datadef *this_data)
{
// fetch lock-in settings from this_data

  if(this_data)
  {
    if(this_data->valid)
    {
      lockin_sen1 = this_data->lockin_sen1;
      lockin_sen2 = this_data->lockin_sen2;
      lockin_osc = this_data->lockin_osc;
      lockin_w = this_data->lockin_w;
      lockin_tc = this_data->lockin_tc;
      lockin_n1 = this_data->lockin_n1;
      lockin_pha1 = this_data->lockin_pha1;
      lockin_pha2 = this_data->lockin_pha2;
	  lockin_in_use = this_data->lockin_in_use;
    }
	else
	{
      lockin_sen1 = 0.0;
      lockin_sen2 = 0.0;
      lockin_osc = 0.0;
      lockin_w = 0.0;
      lockin_tc = 0.0;
      lockin_n1 = 0.0;
      lockin_pha1 = 0.0;
      lockin_pha2 = 0.0;
	  lockin_in_use = LOCKIN_NOT_USED;
	}
  }
  else
  {
    lockin_sen1 = 0.0;
    lockin_sen2 = 0.0;
    lockin_osc = 0.0;
    lockin_w = 0.0;
    lockin_tc = 0.0;
    lockin_n1 = 0.0;
    lockin_pha1 = 0.0;
    lockin_pha2 = 0.0;
	lockin_in_use = LOCKIN_NOT_USED;
  }

}

static void set_lockin_data(datadef *this_data)
{
// stuff lock-in settings into this_data

  if(this_data)
  {
    if(this_data->valid)
    {
      this_data->lockin_sen1 = lockin_sen1;
      this_data->lockin_sen2 = lockin_sen2;
      this_data->lockin_osc = lockin_osc;
      this_data->lockin_w = lockin_w;
      this_data->lockin_tc = lockin_tc;
      this_data->lockin_n1 = lockin_n1;
      this_data->lockin_pha1 = lockin_pha1;
      this_data->lockin_pha2 = lockin_pha2;
	  this_data->lockin_in_use = lockin_in_use;
    }
  }
}
