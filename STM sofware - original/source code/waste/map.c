#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm.h"
#include "file.h"
#include "map.h"

#ifdef SUPER_PROBLEM
offset_pt *offsets_map;
int	  map_size;         // the number of points in the map
#endif

extern HANDLE   hInst;
extern char string[];

extern char *current_file_map;

static int first_time_map = 1;

BOOL FAR PASCAL MapDlg( HWND, unsigned, WPARAM, LPARAM);

static void repaint_map_list( HWND hDlg);

BOOL FAR PASCAL MapDlg( HWND hDlg, unsigned Message, WPARAM wParam, LPARAM lParam)
{
  int w;

  switch(Message) {
    case WM_INITDIALOG:
#ifdef SUPER_PROBLEM
	if( first_time_map) {

	  // allocate memory for map and initialize it
	  offsets_map = (offset_pt*) malloc( MAX_MAP_SIZE * sizeof(offset_pt));
	  for( w = 0; w < MAX_MAP_SIZE; w++) {
	    offsets_map[w].x_offset = 0;
	    offsets_map[w].y_offset = 0;
	  }  
	  map_size = 0;

	  first_time_map = 0;
	}
	repaint_map_list( hDlg);
#endif
	break;
    case WM_COMMAND:
      switch (LOWORD(wParam)) {
        case MAP_LOAD:
#ifdef SUPER_PROBLEM
	  if( file_open( hDlg, hInst, FTYPE_MAP, 0, current_file_map)) {
	    repaint_map_list( hDlg);
	  }
#endif
	  break;
        case MAP_SAVE:
#ifdef SUPER_PROBLEM
	  MessageBox( hDlg, "Create and save *.map's not implemented yet.", "Debug", MB_OK);
#endif
	  break;
        case MAP_EXIT:
#ifdef SUPER_PROBLEM
          EndDialog(hDlg,TRUE);
	  return(TRUE);
#endif
	  break;
      }
      break;
  }
  return(FALSE);

}

static void repaint_map_list( HWND hDlg)
{
#ifdef SUPER_PROBLEM
  int m = 0;
  char list_string[1000];

  strcpy( list_string, "");
  SetDlgItemText( hDlg, MAP_PTS_EDIT, "");  // clear area

  for( m = 0; m < map_size; m++) {
     sprintf( string, "%d %d\n", offsets_map[m].x_offset, offsets_map[m].y_offset);
     strcat( list_string, string);
  }
  strcat( list_string, '\0');

  SetDlgItemText( hDlg, MAP_PTS_EDIT, list_string);
#endif
}
