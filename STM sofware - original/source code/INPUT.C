#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "stm.h"
#include "dio.h"
#include "serial.h"

extern HWND InputWnd;
extern BOOL InputOn;

extern char string[];

BOOL FAR PASCAL InputDlg(HWND,unsigned,WPARAM,LPARAM);
// Allows lowest level (bit by bit) reading of input

BOOL FAR PASCAL InputDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{
  unsigned int i_data;
  unsigned int i;

  switch(Message) {
    case WM_INITDIALOG:
      for(i=0;i<4;i++) {
        SetDlgItemInt(hDlg,I_DATA0+i,0,FALSE);
        SetDlgItemText(hDlg,I_BITS0+i,in_dtob(0));
        SetDlgItemText(hDlg,I_VOLTS0+i,gcvt(in_dtov(0.0),5,string));
      }
      break;
    case WM_COMMAND:
      switch (LOWORD(wParam)) {
		case I_UPDATE:
/*
			for(i=0;i<4;i++)
			  if(IsDlgButtonChecked(hDlg,I_CH0+i)) {
				dio_in_ch(i);
				dio_in_data(&i_data);
				SetDlgItemInt(hDlg,I_DATA0+i,i_data,FALSE);
				SetDlgItemText(hDlg,I_BITS0+i,in_dtob(i_data));
				SetDlgItemText(hDlg,I_VOLTS0+i,gcvt(in_dtov((float)i_data),5,string));
			}
*/
		    for(i=0;i<4;i++)
			  if(IsDlgButtonChecked(hDlg,I_CH0+i)) {
			    dio_in_serial(i,&i_data);
			    SetDlgItemInt(hDlg,I_DATA0+i,i_data,FALSE);
			    SetDlgItemText(hDlg,I_BITS0+i,in_dtob(i_data));
				sprintf(string,"%.5f",in_dtov((float)i_data));
			    SetDlgItemText(hDlg,I_VOLTS0+i,string);
			}
			break;

		case INPUT_CLOSE:
		  DestroyWindow(hDlg);
		  InputWnd = NULL;
		  InputOn = FALSE;
		  break;
      }
      break;
  }
  return(FALSE);
}
