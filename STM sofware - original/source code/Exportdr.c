#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "stm.h"
#include "file.h"
#include "exportdr.h"

char exp_src_dir[CURRENT_DIR_MAX];
char exp_dest_dir[CURRENT_DIR_MAX];

BOOL FAR PASCAL ExportDirDlg(hDlg,Message,wParam,lParam);
// For user to type in src and dest directories for export multiple program
// Code for actual exporting in stm.c

BOOL FAR PASCAL ExportDirDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{
  char buffer[CURRENT_DIR_MAX];

  switch(Message) {
    case WM_INITDIALOG:
	  SetDlgItemText(hDlg,EXPORT_SRC_DIR,exp_src_dir);
	  SetDlgItemText(hDlg,EXPORT_DEST_DIR,exp_dest_dir);
	  break;
    case WM_COMMAND:
	  switch(LOWORD(wParam)) {
	    case EXP_DIR_OK:
	      if(!GetDlgItemText(hDlg,EXPORT_SRC_DIR,buffer,CURRENT_DIR_MAX)) {
	    	MessageBox(hDlg,"Source directory too long","Too Many Characters",MB_OK);
		    break;
		  }
	      strcpy(exp_src_dir,buffer);
	      if(!GetDlgItemText(hDlg,EXPORT_DEST_DIR,buffer,CURRENT_DIR_MAX)) {
	    	MessageBox(hDlg,"Dest. directory too long","Too Many Characters",MB_OK);
		    break;
		  }
	      strcpy(exp_dest_dir,buffer);
          EndDialog(hDlg,EXPORT_OK);
          return(TRUE);

	    case EXP_DIR_CANCEL:
          EndDialog(hDlg,EXPORT_CANCELLATION);
	      return(TRUE);
	  }
	  break;
  }
  return(FALSE);
}

