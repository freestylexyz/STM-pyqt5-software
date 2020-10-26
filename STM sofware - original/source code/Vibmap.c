#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file.h"
#include "data.h"
#include "dio.h"
#include "stm.h"
#include "spec.h"
#include "count.h"
#include "image.h"
#include "vibmap.h"

extern datadef *data;
extern char string[];
extern char *current_file_map;
extern HANDLE hInst;
extern BITMAPINFO *bitmapinfo;
extern HBITMAP  hXcross;
extern HPALETTE hPal;
extern unsigned char *bitmap_data;
extern char *current_file_stm,*current_file_cnt;
extern COUNT_DATA *glob_count_data;
extern unsigned int scan_fine_x,scan_fine_y;

// list of offsets at which to take spec scan, in bits
extern OFFSET_PT sp_offsets_map[];

extern int sp_scan_count; // 1 based
int current_vm_index = 0; // 0 based

// position of tracking during spec scan
// doesn't affect spec algorithm: user must put tip here manually before scan
// given in bits, with (0,0) as center of image
int vm_reference_x = 0;
int vm_reference_y = 0;

// offsets corresponding to current index
int vm_offset_x = 0;
int vm_offset_y = 0;

int vm_dot_red = 0;
int vm_dot_blue = 0;
int vm_dot_green = 0;
int vm_dot_size = 4;
int vm_line_pts = 4;
int vm_making_pattern = FALSE;
int vm_pattern_x1 = 0;
int vm_pattern_x2 = 0;
int vm_pattern_y1 = 0;
int vm_pattern_y2 = 0;
int vm_num_lines = 4;
int vm_trans_x = 10;
int vm_trans_y = 10;
int vm_pattern_type = VM_PATTERN_LINE;
COUNT_DATA vm_count_data;

float vm_pixels_per_bit = 1.0; /* how many pixels on screen correspond to one bit */

static int out_smart = 0;

static void vm_insert_line(int,int,int,int);
static void vm_insert_mesh(int,int,int,int);
static void vm_insert_wheel(int,int,int,int);
static void vm_insert_point(int,int);
static void paint_circles_vm(HDC,int,int,float,OFFSET_PT,int,COLORREF);
static void repaint_dot_parameters(HWND);
static void repaint_vm_image(HDC);
static void vm2cnt_coords(int *,int *,int,int);
static void repaint_vm_refs(HWND);
static void repaint_vm_offsets(HWND);
static void repaint_pattern_params(HWND);

BOOL FAR PASCAL VibMapDlg(HWND,unsigned,WPARAM,LPARAM);
// Vibrational Mapping dialog used to set up automated multiple spec. scans

BOOL FAR PASCAL VibMapDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{

  char *temp_char;
  int value;
  int i;
  HDC hDC;
  PAINTSTRUCT ps;
  POINT vm_mouse;
  int calc_x,calc_y;
  int converted_x,converted_y;
    
  switch(Message) {
    case WM_INITDIALOG:
      if(sp_scan_count == 0) {
	    sp_scan_count = 1;
	    current_vm_index = 0;
      }
      repaint_vm_offsets(hDlg);
      repaint_vm_refs(hDlg);
      repaint_dot_parameters(hDlg);
      repaint_pattern_params(hDlg);
      if(data->valid) {
	    vm_pixels_per_bit = ((float)VM_DISPLAY_WIDTH) / (data->size * data->step);
      }
      init_count_data(&vm_count_data);
      break;

    case WM_PAINT:
      hDC = BeginPaint(hDlg,&ps);
      repaint_vm_image(hDC);
      EndPaint(hDlg,&ps);

      repaint_vm_offsets(hDlg);
      repaint_dot_parameters(hDlg);
      repaint_pattern_params(hDlg);
      repaint_vm_refs(hDlg);
      break;

    case WM_LBUTTONDOWN:
      vm_mouse.x = LOWORD(lParam);
      vm_mouse.y = HIWORD(lParam);
	  // if click is inside image,insert an offset
	  if(vm_mouse.x >= VM_DISPLAY_LEFT && vm_mouse.x <= (VM_DISPLAY_LEFT + VM_DISPLAY_WIDTH) &&
	    vm_mouse.y >= VM_DISPLAY_TOP && vm_mouse.y <= (VM_DISPLAY_TOP + VM_DISPLAY_HEIGHT))
	  {
	    calc_x = (int)((vm_mouse.x - (VM_DISPLAY_LEFT + VM_DISPLAY_WIDTH/2))/vm_pixels_per_bit - vm_reference_x);
	    calc_y = (int)(-(vm_mouse.y - (VM_DISPLAY_TOP + VM_DISPLAY_HEIGHT/2))/vm_pixels_per_bit - vm_reference_y);
        if(wParam & MK_SHIFT)
		{
          SetCapture(hDlg);
          vm_making_pattern = TRUE;
          vm_pattern_x1 = vm_pattern_x2 = calc_x;
          vm_pattern_y1 = vm_pattern_y2 = calc_y;
		}
        else vm_insert_point(calc_x,calc_y);
	    hDC = GetDC(hDlg);
	    repaint_vm_image(hDC);
	    ReleaseDC(hDlg,hDC);
        repaint_vm_offsets(hDlg);
	  }
	  break;

    case WM_LBUTTONUP:
      vm_mouse.x = LOWORD(lParam);
      vm_mouse.y = HIWORD(lParam);
      vm_mouse.x = min(max(VM_DISPLAY_LEFT,vm_mouse.x),VM_DISPLAY_LEFT + VM_DISPLAY_WIDTH);
      vm_mouse.y = min(max(VM_DISPLAY_TOP,vm_mouse.y),VM_DISPLAY_TOP + VM_DISPLAY_HEIGHT);

      if(vm_making_pattern)
      {
        ReleaseCapture();
        vm_making_pattern = FALSE;
        vm_pattern_x2 = (vm_mouse.x - (VM_DISPLAY_LEFT + VM_DISPLAY_WIDTH / 2)) / vm_pixels_per_bit - vm_reference_x;
        vm_pattern_y2 = -(vm_mouse.y - (VM_DISPLAY_TOP + VM_DISPLAY_HEIGHT / 2)) / vm_pixels_per_bit - vm_reference_y;

        if(wParam & MK_SHIFT)
        {
          switch(vm_pattern_type)
          {
            case VM_PATTERN_LINE:
              vm_insert_line(vm_pattern_x1,vm_pattern_y1,vm_pattern_x2,vm_pattern_y2);
              repaint_vm_offsets(hDlg);
              hDC = GetDC(hDlg);
              repaint_vm_image(hDC);
              ReleaseDC(hDlg,hDC);
              break;
            case VM_PATTERN_WHEEL:
              vm_insert_wheel(vm_pattern_x1,vm_pattern_y1,vm_pattern_x2,vm_pattern_y2);
              repaint_vm_offsets(hDlg);
              hDC = GetDC(hDlg);
              repaint_vm_image(hDC);
              ReleaseDC(hDlg,hDC);
              break;
            case VM_PATTERN_MESH:
              vm_insert_mesh(vm_pattern_x1,vm_pattern_y1,vm_pattern_x2,vm_pattern_y2);
              repaint_vm_offsets(hDlg);
              hDC = GetDC(hDlg);
              repaint_vm_image(hDC);
              ReleaseDC(hDlg,hDC);
              break;
          }
        }
	  }
      break;

    case WM_RBUTTONDOWN:
      vm_mouse.x = LOWORD(lParam);
      vm_mouse.y = HIWORD(lParam);
	  // if click is inside image,update the reference
	  if(vm_mouse.x >= VM_DISPLAY_LEFT && vm_mouse.x <= (VM_DISPLAY_LEFT + VM_DISPLAY_WIDTH) &&
	    vm_mouse.y >= VM_DISPLAY_TOP && vm_mouse.y <= (VM_DISPLAY_TOP + VM_DISPLAY_HEIGHT) )
	  {
	    vm_reference_x = (int)((vm_mouse.x - (VM_DISPLAY_LEFT + VM_DISPLAY_WIDTH/2))/vm_pixels_per_bit);
	    vm_reference_y = (int)(-(vm_mouse.y - (VM_DISPLAY_TOP + VM_DISPLAY_HEIGHT/2))/vm_pixels_per_bit);
	    hDC = GetDC(hDlg);
	    repaint_vm_image(hDC);
	    ReleaseDC(hDlg,hDC);
        repaint_vm_refs(hDlg);
	  }
	  break;

    case WM_MOUSEMOVE:
      // continue drawing arrow
      vm_mouse.x = LOWORD(lParam);
      vm_mouse.y = HIWORD(lParam);
      vm_mouse.x = min(max(VM_DISPLAY_LEFT,vm_mouse.x),VM_DISPLAY_LEFT + VM_DISPLAY_WIDTH);
      vm_mouse.y = min(max(VM_DISPLAY_TOP,vm_mouse.y),VM_DISPLAY_TOP + VM_DISPLAY_HEIGHT);

      if(vm_making_pattern)
      {    
        // convert mouse coords in pixels to bits
        // mouse coords: (0,0) in upper left of window
        // bits (2048,2048) is in center of image
        vm_pattern_x2 = (vm_mouse.x - (VM_DISPLAY_LEFT + VM_DISPLAY_WIDTH / 2)) / vm_pixels_per_bit - vm_reference_x;
        vm_pattern_y2 = -(vm_mouse.y - (VM_DISPLAY_TOP + VM_DISPLAY_HEIGHT / 2)) / vm_pixels_per_bit - vm_reference_y;
                
        // repaint
        hDC = GetDC(hDlg);
        repaint_vm_image(hDC);
        ReleaseDC(hDlg,hDC);
      }
      break;

    case WM_COMMAND:
      switch(LOWORD(wParam))
	  {
		// begin of Mod. #8, X. Cao, 2004-06-15, Cycle the points
		// original code is completely commented out and replaced with new code
	    case VM_NEXT:
		  
	      //if(current_vm_index+1 < sp_scan_count) current_vm_index++;
	      //vm_offset_x = sp_offsets_map[current_vm_index].x;
	      //vm_offset_y = sp_offsets_map[current_vm_index].y;
	      //repaint_vm_offsets(hDlg);
		  if(current_vm_index+1 < sp_scan_count)
		    current_vm_index++;
		  else if(current_vm_index+1 == sp_scan_count)	// if this is the last point, go to first
			current_vm_index = 0;

		  vm_offset_x = sp_offsets_map[current_vm_index].x;
	      vm_offset_y = sp_offsets_map[current_vm_index].y;
	      repaint_vm_offsets(hDlg);

		  hDC = GetDC(hDlg);
	      repaint_vm_image(hDC);
	      ReleaseDC(hDlg,hDC);
		  break; 

	    case VM_PREV:
	      //if(current_vm_index > 0) current_vm_index--;
	      //vm_offset_x = sp_offsets_map[current_vm_index].x;
	      //vm_offset_y = sp_offsets_map[current_vm_index].y;
	      //repaint_vm_offsets(hDlg);
		  if(current_vm_index > 0)
			current_vm_index--;
		  else if(current_vm_index == 0)		// if this is the 1st point already, go to last
			current_vm_index = sp_scan_count - 1;

		  vm_offset_x = sp_offsets_map[current_vm_index].x;
	      vm_offset_y = sp_offsets_map[current_vm_index].y;
	      repaint_vm_offsets(hDlg);

		  hDC = GetDC(hDlg);
	      repaint_vm_image(hDC);
	      ReleaseDC(hDlg,hDC);
		  break; 
		// end of Mod. #8, X. Cao, 2004-06-15, Cycle the points

	    case VM_OFF_X:
	      if(!out_smart)
          {
            out_smart = 1;
            GetDlgItemText(hDlg,VM_OFF_X,string,9);
		    value = atoi(string);
            vm_offset_x = min(max(value,-ZERO - vm_reference_x),MAX - ZERO - vm_reference_x);
		    //vm_offset_x = min(max(value,VM_MIN_OFFSET),VM_MAX_OFFSET);
		    sp_offsets_map[current_vm_index].x = vm_offset_x;
		    if(vm_offset_x != value) repaint_vm_offsets(hDlg);
            out_smart = 0;
          }
	      break;

		case VM_OFF_Y:
	      if(!out_smart)
          {
            out_smart = 1;
            GetDlgItemText(hDlg,VM_OFF_Y,string,9);
		    value = atoi(string);
            vm_offset_y = min(max(value,- ZERO - vm_reference_y),MAX - ZERO - vm_reference_y);
		    //vm_offset_y = min(max(value,VM_MIN_OFFSET),VM_MAX_OFFSET);
		    sp_offsets_map[current_vm_index].y = vm_offset_y;
		    if(vm_offset_y != value) repaint_vm_offsets(hDlg);
            out_smart = 0;
          }
	      break;

        case VM_LINE_PTS:
          if(!out_smart)
		  {
            out_smart = 1;
            GetDlgItemText(hDlg,VM_LINE_PTS,string,9);
            value = atoi(string);
            vm_line_pts = min(max(value,VM_MIN_LINE_POINTS),VM_MAX_LINE_POINTS);
            out_smart = 0;
		  }
          break;
        case VM_NUM_LINES:
          if(!out_smart)
		  {
            out_smart = 1;
            GetDlgItemText(hDlg,VM_NUM_LINES,string,9);
            value = atoi(string);
            vm_num_lines = min(max(value,VM_MIN_LINE_POINTS),VM_MAX_LINE_POINTS);
            out_smart = 0;
		  }
          break;
        case VM_TRANS_X:
          if(!out_smart)
		  {
            out_smart = 1;
            GetDlgItemText(hDlg,VM_TRANS_X,string,9);
            value = atoi(string);
            vm_trans_x = min(max(value,VM_MIN_OFFSET),VM_MAX_OFFSET);
            out_smart = 0;
		  }
          break;
        case VM_TRANS_Y:
          if(!out_smart)
		  {
            out_smart = 1;
            GetDlgItemText(hDlg,VM_TRANS_Y,string,9);
            value = atoi(string);
            vm_trans_y = min(max(value,VM_MIN_OFFSET),VM_MAX_OFFSET);
            out_smart = 0;
		  }
          break;
        case VM_PATTERN_LINE:
        case VM_PATTERN_WHEEL:
        case VM_PATTERN_MESH:
          vm_pattern_type = LOWORD(wParam);
          repaint_pattern_params(hDlg);
          break;

	    case VM_LOAD:
	      if(data->valid && data->saved)
		  {
	        strcpy(current_file_map,data->filename);
	        temp_char=strrchr(current_file_map,'.');
	        if(temp_char!=NULL)
	        {
	            *(temp_char+1)='\0';
	            strcat(current_file_map,"vmp");
	        }
	        else strcat(current_file_map,".vmp");
		  }
	      file_open(hDlg,hInst,FTYPE_MAP,0,current_file_map);
	      current_vm_index = 0;
	      vm_offset_x = sp_offsets_map[current_vm_index].x;
	      vm_offset_y = sp_offsets_map[current_vm_index].y;

	      hDC = GetDC(hDlg);
	      repaint_vm_image(hDC);
	      ReleaseDC(hDlg,hDC);

	      repaint_vm_offsets(hDlg);
	      break;

	    case VM_SAVE:
	      if(data->valid && data->saved)
		  {
	        strcpy(current_file_map,data->filename);
	        temp_char=strrchr(current_file_map,'.');
	        if(temp_char!=NULL)
	        {
	            *(temp_char+1)='\0';
	            strcat(current_file_map,"vmp");
	        }
	        else strcat(current_file_map,".vmp");
		  }
	      file_save_as(hDlg,hInst,FTYPE_MAP);
	      break;

	    case ENTER:
	      if(sp_scan_count > 0)
		  {
            GetDlgItemText(hDlg,VM_REF_X,string,9);
            value = (int) atoi(string);
            vm_reference_x = min(max(value,VM_MIN_OFFSET),VM_MAX_OFFSET);
            GetDlgItemText(hDlg,VM_REF_Y,string,9);
            value = (int) atoi(string);
            vm_reference_y = min(max(value,VM_MIN_OFFSET),VM_MAX_OFFSET);

            GetDlgItemText(hDlg,VM_OFF_X,string,9);
            value = (int) atoi(string);
            vm_offset_x = min(max(value,- ZERO - vm_reference_x),MAX - ZERO - vm_reference_x);
            sp_offsets_map[current_vm_index].x = vm_offset_x;
            if(vm_offset_x != value) repaint_vm_offsets(hDlg);
            GetDlgItemText(hDlg,VM_OFF_Y,string,9);
            value = (int) atoi(string);
            vm_offset_y = min(max(value,- ZERO - vm_reference_y),MAX - ZERO - vm_reference_y);
            sp_offsets_map[current_vm_index].y = vm_offset_y;
            if(vm_offset_y != value) repaint_vm_offsets( hDlg);

	        hDC = GetDC(hDlg);
	        repaint_vm_image(hDC);
	        ReleaseDC(hDlg,hDC);
		  }
	      break;

	    case VM_CLEAR:
	      for(i = 0; i < sp_scan_count; i++) {
		    sp_offsets_map[i].x = 0;
		    sp_offsets_map[i].y = 0;
		  }
	      sp_scan_count = 1;
	      current_vm_index = 0;
	      vm_offset_x = sp_offsets_map[current_vm_index].x = 0;
	      vm_offset_y = sp_offsets_map[current_vm_index].y = 0;

	      hDC = GetDC(hDlg);
	      repaint_vm_image(hDC);
	      ReleaseDC(hDlg,hDC);

	      repaint_vm_offsets(hDlg);
	      break;

	    case VM_ADD:
	      if(sp_scan_count < SP_MAX_MAP_SIZE) {
	        sp_scan_count++;
	        current_vm_index = sp_scan_count-1;
	        vm_offset_x = sp_offsets_map[current_vm_index].x = 0;
	        vm_offset_y = sp_offsets_map[current_vm_index].y = 0;
	        repaint_vm_offsets(hDlg);

	        hDC = GetDC(hDlg);
	        repaint_vm_image(hDC);
	        ReleaseDC(hDlg,hDC);
		  }
	      break;

	    case VM_INSERT:
	      vm_insert_point(0,0);

	      repaint_vm_offsets(hDlg);
	      hDC = GetDC(hDlg);
	      repaint_vm_image(hDC);
	      ReleaseDC(hDlg,hDC);
	      break;

	    case VM_DELETE:
	      if(sp_scan_count > 1) {
	        if(current_vm_index == sp_scan_count-1) {
		      sp_offsets_map[current_vm_index].x = 0;
		      sp_offsets_map[current_vm_index].y = 0;
		      current_vm_index--;
			}
	        else {
	          // shift all values from current index to next to last index backward by one slot
	          for(i=current_vm_index; i<sp_scan_count-1; i++) {
		        sp_offsets_map[i].x = sp_offsets_map[i+1].x;
		        sp_offsets_map[i].y = sp_offsets_map[i+1].y;
			  }  
			}

	        sp_offsets_map[sp_scan_count-1].x = sp_offsets_map[sp_scan_count-1].y = 0;
	        sp_scan_count--;

	        vm_offset_x = sp_offsets_map[current_vm_index].x;
	        vm_offset_y = sp_offsets_map[current_vm_index].y;
	        repaint_vm_offsets(hDlg);

	        hDC = GetDC(hDlg);
	        repaint_vm_image(hDC);
	        ReleaseDC(hDlg,hDC);

    	  }
	      break;

	    case VM_SAVE_AS_CNT:
	      // count data needs to be cleared if previously used
	      if(vm_count_data.list[0] != NULL) destroy_count(&vm_count_data);

	      // set up the fields of vm_count_data and then save it
	      if(data->valid) {
		    strcpy(vm_count_data.filename,data->filename);
		  }
	      else {
		    strcpy(vm_count_data.filename,current_file_stm);
		  }

	      vm_count_data.comment.size = strlen(VM_CNT_COMMENT);
	      strcpy(vm_count_data.comment.ptr,VM_CNT_COMMENT);
	      vm_count_data.hide = 0;
	      vm_count_data.version = COUNT_DATA_VERSION;

	      vm_count_data.size = vm_dot_size;
	      vm_count_data.color[0].r = vm_dot_red;
	      vm_count_data.color[0].g = vm_dot_green;
	      vm_count_data.color[0].b = vm_dot_blue;
	      vm_count_data.total[0] = sp_scan_count;
  
	      for(i = 0; i < sp_scan_count; i++) {
		    // put the points in the list
		    vm2cnt_coords(&converted_x,&converted_y,sp_offsets_map[i].x + vm_reference_x,
				sp_offsets_map[i].y + vm_reference_y);
		    new_count_el(&(vm_count_data.list[0]),converted_x,converted_y);
		  }
            
          if(data->valid) {
            strcpy(current_file_cnt,data->filename);
		  }
	      else {
		    strcpy(current_file_cnt,current_file_stm);
		  }

          temp_char=strrchr(current_file_cnt,'.');
          if(temp_char != NULL)
          {
            *(temp_char + 1) = '\0';
            strcat(current_file_cnt,"cnt");
          }
          else strcat(current_file_cnt,".cnt");

	      glob_count_data=&(vm_count_data);
          file_save_as(hDlg,hInst,FTYPE_CNT);
	      break;

	    case VM_DOT_R:
	      if(!out_smart) {
		    out_smart = 1;
            GetDlgItemText(hDlg,VM_DOT_R,string,9);
		    value = atoi(string);
		    vm_dot_red = min(max(0,value),255);

	        hDC = GetDC(hDlg);
	        repaint_vm_image(hDC);
	        ReleaseDC(hDlg,hDC);

		    out_smart = 0;
		  }
	      break;

	    case VM_DOT_G:
	      if(!out_smart) {
		    out_smart = 1;
            GetDlgItemText(hDlg,VM_DOT_G,string,9);
		    value = atoi(string);
		    vm_dot_green = min(max(0,value),255);

	        hDC = GetDC(hDlg);
	        repaint_vm_image(hDC);
	        ReleaseDC(hDlg,hDC);

		    out_smart = 0;
		  }
	      break;

	    case VM_DOT_B:
	      if(!out_smart) {
		    out_smart = 1;
            GetDlgItemText(hDlg,VM_DOT_B,string,9);
		    value = atoi(string);
		    vm_dot_blue = min(max(0,value),255);

	        hDC = GetDC(hDlg);
	        repaint_vm_image(hDC);
	        ReleaseDC(hDlg,hDC);

		    out_smart = 0;
		  }
	      break;

	    case VM_DOT_SIZE:
	      if(!out_smart) {
		    out_smart = 1;
            GetDlgItemText(hDlg,VM_DOT_SIZE,string,9);
		    value = atoi(string);
		    vm_dot_size = max(0,value);

	        hDC = GetDC(hDlg);
	        repaint_vm_image(hDC);
	        ReleaseDC(hDlg,hDC);

		    out_smart = 0;
		  }
	      break;

	    case VM_REF_X:
	      if(!out_smart)
          {
            out_smart = 1;
            GetDlgItemText(hDlg,VM_REF_X,string,9);
		    value = atoi(string);
		    vm_reference_x = min(max(value,VM_MIN_OFFSET),VM_MAX_OFFSET);
		    if(vm_reference_x != value) repaint_vm_refs(hDlg);
            out_smart = 0;
          }
	      break;

	    case VM_REF_Y:
	      if(!out_smart)
          {
            out_smart = 1;
            GetDlgItemText(hDlg,VM_REF_Y,string,9);
		    value = atoi(string);
		    vm_reference_y = min(max(value,VM_MIN_OFFSET),VM_MAX_OFFSET);
		    if(vm_reference_y != value) repaint_vm_refs(hDlg);
            out_smart = 0;
          }
	      break;

		case VM_IMPORT_REF:
		  if(!out_smart)
		  {
		    out_smart = 1;
			vm_reference_x = scan_fine_x - ZERO;
            vm_reference_y = scan_fine_y - ZERO;
		    repaint_vm_refs(hDlg);
	        hDC = GetDC(hDlg);
	        repaint_vm_image(hDC);
	        ReleaseDC(hDlg,hDC);
			out_smart = 0;
		  }
		  break;

        case VM_EXIT:
          EndDialog(hDlg,TRUE);
	      return(TRUE);
      }
      break;
  }
  return(FALSE);
}

static void repaint_vm_refs(HWND hDlg)
{
    sprintf(string,"%d",vm_reference_x);
    SetDlgItemText(hDlg,VM_REF_X,string);
    sprintf(string,"%d",vm_reference_y);
    SetDlgItemText(hDlg,VM_REF_Y,string);
}

static void repaint_vm_offsets(HWND hDlg)
{
    sprintf(string,"%d of %d",current_vm_index+1,sp_scan_count);
    SetDlgItemText(hDlg,VM_POINT_NUM,string);
    sprintf(string,"%d",vm_offset_x);
    SetDlgItemText(hDlg,VM_OFF_X,string);
    sprintf(string,"%d",vm_offset_y);
    SetDlgItemText(hDlg,VM_OFF_Y,string);
}

static void repaint_dot_parameters(HWND hDlg)
{
    sprintf(string,"%d",vm_dot_red);
    SetDlgItemText(hDlg,VM_DOT_R,string);
    sprintf(string,"%d",vm_dot_green);
    SetDlgItemText(hDlg,VM_DOT_G,string);
    sprintf(string,"%d",vm_dot_blue);
    SetDlgItemText(hDlg,VM_DOT_B,string);
    sprintf(string,"%d",vm_dot_size);
    SetDlgItemText(hDlg,VM_DOT_SIZE,string);
}

static void repaint_pattern_params(HWND hDlg)
{
  sprintf(string,"%d",vm_line_pts);
  SetDlgItemText(hDlg,VM_LINE_PTS,string);
  sprintf(string,"%d",vm_num_lines);
  SetDlgItemText(hDlg,VM_NUM_LINES,string);
  sprintf(string,"%d",vm_trans_x);
  SetDlgItemText(hDlg,VM_TRANS_X,string);
  sprintf(string,"%d",vm_trans_y);
  SetDlgItemText(hDlg,VM_TRANS_Y,string);
  if(vm_pattern_type == VM_PATTERN_LINE)
  {
    CheckDlgButton(hDlg,VM_PATTERN_LINE,1);
    CheckDlgButton(hDlg,VM_PATTERN_WHEEL,0);
    CheckDlgButton(hDlg,VM_PATTERN_MESH,0);
  }
  if(vm_pattern_type == VM_PATTERN_WHEEL)
  {
    CheckDlgButton(hDlg,VM_PATTERN_LINE,0);
    CheckDlgButton(hDlg,VM_PATTERN_WHEEL,1);
    CheckDlgButton(hDlg,VM_PATTERN_MESH,0);
  }
  if(vm_pattern_type == VM_PATTERN_MESH)
  {
    CheckDlgButton(hDlg,VM_PATTERN_LINE,0);
    CheckDlgButton(hDlg,VM_PATTERN_WHEEL,0);
    CheckDlgButton(hDlg,VM_PATTERN_MESH,1);
  }
}

static void repaint_vm_image(HDC hDC)
{
  int j;
  int ref_x_pix,ref_y_pix;
  int OldROP;
  float x1,x2,y1,y2;
  HPEN hPen,hPenOld;
  HBRUSH hBrush,hBrushOld;
  HDC hMemDC;

  // draw image rectangle,256 x 256 pixels
  SetMapMode(hDC,MM_TEXT);
  hPen = GetStockObject(BLACK_PEN);
  hBrush = GetStockObject(WHITE_BRUSH);
  hBrushOld = SelectObject(hDC,hBrush);
  hPenOld = SelectObject(hDC,hPen);
  Rectangle(hDC,VM_DISPLAY_LEFT,VM_DISPLAY_TOP,VM_DISPLAY_RIGHT,VM_DISPLAY_BOTTOM);
  SelectObject(hDC,hBrushOld);
  SelectObject(hDC,hPenOld);

  // draw scanned image,if there is one
  if(data->valid) {
  SelectPalette(hDC,hPal,0);
  RealizePalette(hDC);
  StretchDIBits(hDC,VM_DISPLAY_LEFT,VM_DISPLAY_TOP,
          VM_DISPLAY_WIDTH,VM_DISPLAY_HEIGHT,0,0,data->size,data->size,
          bitmap_data,bitmapinfo,DIB_PAL_COLORS,SRCCOPY);	
  }

  // draw line on screen if we're making a pattern
  if(vm_making_pattern)
  {
    if(!(vm_pattern_x1 == vm_pattern_x2 && vm_pattern_y1 == vm_pattern_y2))
    {
      // paint line
      OldROP = SetROP2(hDC,R2_NOTXORPEN);
      hPen = CreatePen(PS_SOLID,/*thickness*/2,RGB(0,0,0));
      hPenOld = SelectObject(hDC,hPen);
      x1 = x2 = (float)(VM_DISPLAY_LEFT + VM_DISPLAY_WIDTH/2);
      y1 = y2 = (float)(VM_DISPLAY_TOP + VM_DISPLAY_HEIGHT/2);
      x1 += ((float)(vm_pattern_x1 + vm_reference_x) * vm_pixels_per_bit);
      x2 += ((float)(vm_pattern_x2 + vm_reference_x) * vm_pixels_per_bit);
      y1 += -((float)(vm_pattern_y1 + vm_reference_y) * vm_pixels_per_bit);
      y2 += -((float)(vm_pattern_y2 + vm_reference_y) * vm_pixels_per_bit);
      MoveToEx(hDC,x1,y1,NULL);
      LineTo(hDC,x2,y2);
      SelectObject(hDC,hPenOld);
	}
  }

  // paint on the circles that represent the offset points
  for(j = 0;j < sp_scan_count;j++) {
    paint_circles_vm(hDC,VM_DISPLAY_LEFT + VM_DISPLAY_WIDTH / 2,
		    VM_DISPLAY_TOP + VM_DISPLAY_HEIGHT / 2,vm_pixels_per_bit,
		    sp_offsets_map[j],max(vm_dot_size / 2,1),RGB(vm_dot_red,vm_dot_green,vm_dot_blue));
  }

  // begin of Mod. #8, X. Cao, 2004-06-16, Mark the current
  // the current point is made 50% larger in radius
  paint_circles_vm(hDC,VM_DISPLAY_LEFT + VM_DISPLAY_WIDTH/2,
	    VM_DISPLAY_TOP + VM_DISPLAY_HEIGHT/2,vm_pixels_per_bit,
	    sp_offsets_map[current_vm_index],max(vm_dot_size*3/4,1),RGB(vm_dot_red,vm_dot_green,vm_dot_blue));
  // end of Mod. #8, X. Cao, 2004-06-16, Mark the current

  // draw cross at reference location
  hMemDC = CreateCompatibleDC(hDC);
  SelectPalette(hMemDC,hPal,0);
  RealizePalette(hMemDC);
  SelectObject(hMemDC,hXcross);
  ref_x_pix = (int)(vm_reference_x * vm_pixels_per_bit + VM_DISPLAY_LEFT +
											VM_DISPLAY_WIDTH/2 - VM_CROSS_SIZE / 2);
  ref_y_pix = (int)(-vm_reference_y * vm_pixels_per_bit + VM_DISPLAY_TOP +
											VM_DISPLAY_HEIGHT / 2 - VM_CROSS_SIZE / 2);
  if((ref_x_pix) >= VM_DISPLAY_LEFT && (ref_x_pix) <= (VM_DISPLAY_LEFT + VM_DISPLAY_WIDTH) &&
		(ref_y_pix) >= VM_DISPLAY_TOP && (ref_y_pix) <= (VM_DISPLAY_TOP + VM_DISPLAY_HEIGHT))
  {
    BitBlt(hDC,ref_x_pix,ref_y_pix,VM_CROSS_SIZE,VM_CROSS_SIZE,hMemDC,0,0,SRCINVERT);
  }
  DeleteDC(hMemDC);

  DeleteObject(hBrush);
  DeleteObject(hPen);
}

static void paint_circles_vm(HDC hDC,int x_offset,int y_offset,float pixels_per_bit,
			      OFFSET_PT point,int radius,COLORREF color)
{
  // x_offset and y_offset are the center of the image, in pixels
 
  LOGBRUSH lplb;
  HPEN hpen_old;
  HBRUSH hBrush,hBrushOld;
  int OldROP;
  int x,y,x2,y2;
   
  SelectPalette(hDC,hPal,0);
  RealizePalette(hDC);
  OldROP = SetROP2(hDC,R2_COPYPEN);
  lplb.lbStyle = BS_SOLID;
  lplb.lbColor = color;
  hBrush = CreateBrushIndirect(&lplb);
  hBrushOld = SelectObject(hDC,hBrush);
  hpen_old = SelectObject(hDC,GetStockObject(NULL_PEN));

  x = (int)(((point.x + vm_reference_x) * pixels_per_bit) - radius);
  y = (int)(-((point.y + vm_reference_y) * pixels_per_bit) - radius);

  x2 = x + 2 * radius;
  y2 = y + 2 * radius;

  if((x + x_offset) >= VM_DISPLAY_LEFT && 
	(x2 + x_offset) <= (VM_DISPLAY_LEFT + VM_DISPLAY_WIDTH) &&
	(y + y_offset) >= VM_DISPLAY_TOP &&
	(y2 + y_offset) <= (VM_DISPLAY_TOP + VM_DISPLAY_HEIGHT))
  {
    Ellipse(hDC,x + x_offset,y + y_offset,x2 + x_offset,y2 + y_offset);                
  }        
        
        
  SetROP2(hDC,OldROP);
  SelectObject(hDC,hBrushOld);
  SelectObject(hDC,hpen_old);
    
  DeleteObject(hBrush);
}

static void vm_insert_point(int ins_x,int ins_y)
{
  // insert an offset (ins_x,ins_y) at current index
	
  int i;
  
  if(sp_scan_count == SP_MAX_MAP_SIZE) return;
                
  if(ins_x + vm_reference_x + ZERO > MAX) return;
  if(ins_x + vm_reference_x + ZERO < 0) return;
  if(ins_y + vm_reference_y + ZERO > MAX) return;
  if(ins_y + vm_reference_y + ZERO < 0) return;
  sp_scan_count++;

  // shift all values from current index to the end forward by one slot
  for(i = sp_scan_count - 1; i >= current_vm_index; i--) {
    sp_offsets_map[i].x = sp_offsets_map[i - 1].x;
    sp_offsets_map[i].y = sp_offsets_map[i - 1].y;
  }
  vm_offset_x = sp_offsets_map[current_vm_index].x = ins_x;
  vm_offset_y = sp_offsets_map[current_vm_index].y = ins_y;
}

static void vm_insert_line(int x1,int y1,int x2,int y2)
{
  int x_vals[VM_MAX_LINE_POINTS];
  int y_vals[VM_MAX_LINE_POINTS];
  int i;
  float x_bits_per_point,y_bits_per_point;
  int start_x = x1;
  int start_y = y1;

  // determine "slopes" for x_vals and y_vals
  x_bits_per_point = (x2 - x1) / (float)(vm_line_pts - 1);
  y_bits_per_point = (y2 - y1) / (float)(vm_line_pts - 1);

  for(i = 0;i < vm_line_pts;i++)
  {
    x_vals[i] = start_x + (int)(x_bits_per_point * i);
    y_vals[i] = start_y + (int)(y_bits_per_point * i);
    vm_insert_point(x_vals[i],y_vals[i]);
  }
}

static void vm_insert_mesh(int x1,int y1,int x2,int y2)
{
  int new_x1;
  int new_y1;
  int new_x2;
  int new_y2;
  int i = 0;

  for(i = 0;i < vm_num_lines;i++)
  {
    new_x1 = x1 + i * vm_trans_x;
    new_y1 = y1 + i * vm_trans_y;
    new_x2 = x2 + i * vm_trans_x;
    new_y2 = y2 + i * vm_trans_y;
    vm_insert_line(new_x1,new_y1,new_x2,new_y2);
  }
}

static void vm_insert_wheel(int x1,int y1,int x2,int y2)
{
  int i = 0;
  float center_x,center_y;
  float new_x1,new_y1,new_x2,new_y2;
  float temp_x;
  float theta = 3.141592 / vm_num_lines;

  center_x = (float)x1 + ((float)(x2 - x1) / 2); 
  center_y = (float)y1 + ((float)(y2 - y1) / 2); 

  for(i = 0;i < vm_num_lines;i++)
  {
    new_x1 = (float)x1 - center_x;
    new_y1 = (float)y1 - center_y;
    new_x2 = (float)x2 - center_x;
    new_y2 = (float)y2 - center_y;

    temp_x = new_x1;
    new_x1 = temp_x * cos(i * theta) - new_y1 * sin(i * theta);
    new_y1 = temp_x * sin(i * theta) + new_y1 * cos(i * theta);
    temp_x = new_x2;
    new_x2 = temp_x * cos(i * theta) - new_y2 * sin(i * theta);
    new_y2 = temp_x * sin(i * theta) + new_y2 * cos(i * theta);

    new_x1 += center_x;
    new_y1 += center_y;
    new_x2 += center_x;
    new_y2 += center_y;

    vm_insert_line((int)new_x1,(int)new_y1,(int)new_x2,(int)new_y2);
  }
 
}

static void vm2cnt_coords(int *dest_x,int *dest_y,int src_x,int src_y)
{
// converts (src_x,src_y) in offset bits with (0,0) as image center
// to (*dest_x,*dest_y) in steps with (0,0) as lower left corner of image

  if(data->valid)
  {
    // data->step, data->size casted to int to prevent src_x conversion to uint 
    *dest_x = src_x / (int)data->step + (int)data->size / 2;
    *dest_y = src_y / (int)data->step + (int)data->size / 2;
  }
}

