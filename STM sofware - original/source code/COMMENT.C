#include <windows.h>
#include <stdio.h>
#include "stm.h"
#include "data.h"
#include "common.h"

extern datadef *data;
extern struct commentdef *gcomment;

BOOL FAR PASCAL CommentDlg(HWND,unsigned,WPARAM,LPARAM);

BOOL FAR PASCAL CommentDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{

//    char string[500];  
    
  switch(Message) {
    case WM_INITDIALOG:
      if(gcomment->size)
        SetDlgItemText(hDlg,COMMENT_EDIT,gcomment->ptr);
      SetFocus(GetDlgItem(hDlg,COMMENT_EXIT));
      break;
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case COMMENT_EXIT:
          gcomment->size = GetDlgItemText(hDlg,COMMENT_EDIT,gcomment->ptr,COMMENTMAXSIZE);
/*
sprintf(string,"%d",gcomment->size);
mprintf(gcomment->ptr);
*/
          EndDialog(hDlg,TRUE);
	      return(TRUE);
	  }
      break;
  }
  return(FALSE);
}
