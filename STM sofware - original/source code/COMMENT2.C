#include <windows.h>
#include <stdio.h>
#include "stm.h"
#include "data.h"

extern datadef *gendata[];
extern int current_image;
extern char string[];

HWND Comment2Wnd=NULL;
BOOL Comment2On=FALSE;

BOOL FAR PASCAL Comment2Dlg(HWND,unsigned,WPARAM,LPARAM);

BOOL FAR PASCAL Comment2Dlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{
  
  switch(Message) {
    case WM_INITDIALOG:
      if(gendata[current_image]->valid)
      {
        sprintf(string,"Comments: %s",gendata[current_image]->filename);
        SetWindowText(hDlg,string);
        if(gendata[current_image]->comment.size)
          SetDlgItemText(hDlg,COMMENT2_EDIT,gendata[current_image]->comment.ptr);
        else SetDlgItemText(hDlg,COMMENT2_EDIT,"");
      }
      else SetWindowText(hDlg,"Comments");

      SetFocus(GetDlgItem(hDlg,COMMENT2_EXIT));
      break;

    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case COMMENT2_NEW_IMAGE:
          if(gendata[current_image]->valid)
          {
            sprintf(string,"Comments: %s",gendata[current_image]->filename);
            SetWindowText(hDlg,string);
            if  (gendata[current_image]->comment.size)
              SetDlgItemText(hDlg,COMMENT2_EDIT,gendata[current_image]->comment.ptr);
            else SetDlgItemText(hDlg,COMMENT2_EDIT,"");
          }
          else SetWindowText(hDlg,"Comments");

          SetFocus(GetDlgItem(hDlg,COMMENT2_EXIT));
          break;
        
        case COMMENT2_EXIT:
          DestroyWindow(hDlg);
          Comment2Wnd = NULL;
          Comment2On = FALSE;
          return(TRUE);
          break;
      }
      break;
  }
  return(FALSE);
}
