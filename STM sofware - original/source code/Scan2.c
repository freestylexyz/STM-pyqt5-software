#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>
#include <sys\timeb.h>
#include <string.h>
#include <dos.h>
#include <math.h>
#include <windows.h>
//#include <toolhelp.h>
#include "common.h"
#include "stm.h"
#include "dio.h"
#include "data.h"
#include "file.h"
#include "scan.h"
#include "pal.h"
#include "clock.h"
#include "serial.h"
#include "scan2.h"

//#define NA
#define MOH
//#define MOH2
//#define MOH3
//#define TEST
#define SCAN_TIME_TEST
#define STRETCH
//#define TIMING
//#define TIMING2
//#define TIME_ERROR

extern datadef *data,**glob_data,*all_data[];
extern unsigned int dac_data[];
extern double   i_set;
//extern unsigned char *dacbox;
extern int x_range,y_range,z_range,z_offset_range,
           x_offset_range,y_offset_range;
extern int i_set_range,sample_bias_range;
extern unsigned int scan_x,scan_y,scan_z,
                scan_fine_x,scan_fine_y;
extern unsigned int scan_step;
extern unsigned int scan_size;
extern unsigned int scan_scale;
extern int scan_freq;
extern unsigned int sample_bias;
extern unsigned int tip_gain;
extern unsigned int i_setpoint;
extern float scan_x_gain;
extern float scan_y_gain;
extern float scan_z_gain;
extern float min_z,max_z;
extern unsigned int old_x_offset,old_y_offset,old_z_offset;
extern int scan_sharpen;
extern int sharp_lines;
extern int sharp_cycles;
extern int sharp_bias1;
extern int sharp_bias2;
extern int track_limit_x_min,track_limit_x_max;
extern int track_limit_y_min,track_limit_y_max;

extern HANDLE hInst;
extern HBITMAP  hCross;
extern HBITMAP  hXcross;
//extern HBITMAP hImage;
extern LOGPALETTE *pLogPal;
extern HPALETTE hPal;
extern char string[];
extern unsigned int out1;
extern int image_caller;
extern struct commentdef *gcomment;
extern int bitmap_size;
//char sample_type[SAMPLETYPESIZE]="";
extern int image_orig_x,image_orig_y;
extern char *current_file_stm;
extern int digital_abort;
extern BITMAPINFO *bitmapinfo;
extern char    *pal_image;
extern unsigned char *scan_dacbox;
extern unsigned char *image_data;
extern unsigned char *bitmap_data;
extern datadef *scan_defaults_data[];
extern int scan_current_default;
extern int scan_current_data;
extern int scan_max_data;
extern int image_x_min,image_y_min,image_x_max,image_y_max;
extern int scan_display_factor;

extern BOOL FAR PASCAL PrescanDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL Prescan2Dlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL ImageDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL DepDlg(HWND, unsigned, WPARAM, LPARAM);
//extern BOOL FAR PASCAL NewDepDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL CommentDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL SharpDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL SpecDlg(HWND, unsigned, WPARAM, LPARAM);
extern BOOL FAR PASCAL TrackDlg(HWND, unsigned, WPARAM, LPARAM);

extern HWND TrackWnd;
extern BOOL TrackOn;

static int      out_smart = 0;
//float           image_gain = 10;
static int dither0_before;
static int dither1_before;

static void set_scan_parameters(int);
static void update_line(int /*,int*/);
static void update_line_time(HWND,float);

void scan(HWND hDlg)
{
// On ENTRY, expects pre_scan() have just executed successfully.

  HDC hDC;
  LOGBRUSH lplb;
  HBRUSH hBrush,hBrushOld;
  HPEN hPen,hPenOld;
  MSG Message;
  int i,j,k;
  int stop_now = SCANLINE_NOERR;
  int src_x,src_y;
  int dest_offset_x,dest_offset_y;
  int dest_x,dest_y,dest_width,dest_height,src_width,src_height;
  float stime,time_per_line;
  unsigned int first_ch,second_ch,first_offset,second_offset;
  int first_line = SCANLINE_FIRSTLINE;
  int read_it = SCANLINE_READON;
  int line_size = find_size(scan_size);
  float scan_buffer[MAX];
//  int scan_buffer_index = 0;
  unsigned int first_pos_now = ZERO - (scan_size * scan_step) / 2; 
  unsigned int second_pos_now = ZERO - (scan_size * scan_step) / 2;
  unsigned int next_second_pos;
  unsigned int current_bias = sample_bias;
  int scan_result = SCANLINE_NOERR;
  time_t scan_time;

  // initialize min_z and max_z
  min_z = 7000000;
  max_z = -7000000;

  // Initialize graphics variables
  src_x = src_y = 0;
  hDC = GetDC(hDlg);
  SetMapMode(hDC,MM_TEXT);
  SelectPalette(hDC,hPal,0);
  RealizePalette(hDC);
  lplb.lbStyle = BS_SOLID;
  lplb.lbColor = RGB(0,0,0);
  hBrush = CreateBrushIndirect(&lplb);
  hBrushOld = SelectObject(hDC,hBrush);
  hPen = CreatePen(PS_SOLID,1,RGB(0,0,0));
  hPenOld = SelectObject(hDC,hPen);
  Rectangle(hDC,IMAGE_X_POS,IMAGE_Y_POS,IMAGESIZE + IMAGE_X_POS,IMAGESIZE + IMAGE_Y_POS);
  SelectObject(hDC,hBrushOld);
  SelectObject(hDC,hPenOld);
  DeleteObject(hBrush);
  DeleteObject(hPen);
#ifdef STRETCH
  scan_display_factor = IMAGESIZE / scan_size;
#else
  scan_display_factor = 1;
#endif
  bitmap_size = line_size;
  if(bitmap_data == NULL)
    bitmap_data = (unsigned char *)malloc(bitmap_size * bitmap_size * sizeof(unsigned char));
  else
    bitmap_data = (unsigned char *)realloc((void *)bitmap_data,bitmap_size * bitmap_size *
									      sizeof(unsigned char));
  dest_offset_x = IMAGE_X_POS;
  dest_offset_x += (IMAGESIZE - scan_size * scan_display_factor) / 2;
  dest_offset_y = dest_offset_x;
  bitmapinfo->bmiHeader.biBitCount = 8;
  bitmapinfo->bmiHeader.biWidth = line_size;
  bitmapinfo->bmiHeader.biHeight = line_size;

  // Initialize timing info
  stime = calc_time(scan_size);
  time_per_line = stime / (float)scan_size;

  // Other Initialization
  if (SD->x_first)
  {
    first_ch = x_ch;
    first_offset = x_offset_ch;
    second_ch = y_ch;
    second_offset = y_offset_ch;
    src_width = scan_size;
    src_height = 1;
    dest_width = scan_size * scan_display_factor;
    dest_height = scan_display_factor;
    dest_x = 0;    
    dest_y = (scan_size - 1) * scan_display_factor;
  } else
  {
    first_ch = y_ch;
    first_offset = y_offset_ch;
    second_ch = x_ch;
    second_offset = x_offset_ch;
    src_width = 1;
    src_height = scan_size;
    dest_height = scan_size * scan_display_factor;
    dest_width = scan_display_factor;
    dest_x = 0;    
    dest_y = 0;
  }

  for(i = 0;i < scan_size && stop_now == SCANLINE_NOERR;i++)
  {
    //scan_buffer_index = 0;
	 	// peek for message to abort (ESC key)
    PeekMessage(&Message,hDlg,NULL,NULL,PM_REMOVE);
    if(LOWORD(Message.wParam) == VK_ESCAPE)
    {
	  stop_now = STOP_NOW_ABORTED;
      break;
    }

    // line delay
	if(SD->inter_line_delay > 0)
	{
	  serial_soft_delay(SD->inter_line_delay);
	}

	// update status
    sprintf(string,"Scanning line %d",i + 1);
	scan_status(hDlg,string,-1.0);

	

	// scan_line() forward
	if(SD->scan_dir == FORWARD_DIR || SD->scan_dir == BOTH_DIR1 ||
	   SD->scan_dir == BOTH_DIR2_POLAR || SD->scan_dir == BOTH_DIR2) 
	   read_it = SCANLINE_READON;
	else read_it = SCANLINE_READOFF;

    // call scan_setup();

	readseq_setup(SD);
    scan_setup(SD->crash_protection,SD->z_limit_percent,SD->inter_step_delay,
		     SD->step_delay);
	scan_result = scan_line(first_ch,read_it,SCANLINE_FORWARD,first_line,
		      &(scan_buffer[0]),&first_pos_now);
	if(scan_result == SCANLINE_NOERR)
	{ 
      if(first_line == SCANLINE_FIRSTLINE) first_line = SCANLINE_OTHERLINE;
      if(read_it == SCANLINE_READON) {
        // -> update all_data[i]
	    if(SD->scan_dir == FORWARD_DIR || SD->scan_dir == BOTH_DIR1)
		{
          for(k = 0;k < num_data();k++)
		  {
			for(j = 0;j < scan_size;j++)
			{
	          if(SD->x_first)
              {
				*(all_data[k]->ptr + i * scan_size + j) = scan_buffer[num_data() * j + k];
              }
			  else
			  {
                *(all_data[k]->ptr + j * scan_size + i) = scan_buffer[num_data() * j + k];
              }
              if(k == 0)
			  {
                if(scan_buffer[num_data() * j + k] > max_z)
				  max_z = scan_buffer[num_data() * j + k];
                if(scan_buffer[num_data() * j + k] < min_z)
				  min_z = scan_buffer[num_data() * j + k];
              }
			}
          }
		} 

		

	    if(SD->scan_dir == BOTH_DIR2 || SD->scan_dir == BOTH_DIR2_POLAR)
		{
          for(k = 0;k < num_data()/2;k++)
		  {
			for(j = 0;j < scan_size;j++)
			{
	          if(SD->x_first)
              {
				*(all_data[k]->ptr + i * scan_size + j) = scan_buffer[num_data()/2 * j + k];
              }
			  else
			  {
                *(all_data[k]->ptr + j * scan_size + i) = scan_buffer[num_data()/2 * j + k];
              }
              if(k == 0)
			  {
                if(scan_buffer[num_data()/2 * j + k] > max_z)
				  max_z = scan_buffer[num_data()/2 * j + k];
                if(scan_buffer[num_data()/2 * j + k] < min_z)
				  min_z = scan_buffer[num_data()/2 * j + k];
			  }
			}
          }
		} 

		//scan_buffer_index += scan_size;
	  }
	} 
	else
	{
      stop_now = scan_result;
	  break;
	}

	// flip bias if we're scanning BOTH_DIR2_POLAR
	if(SD->scan_dir == BOTH_DIR2_POLAR)
	{
      current_bias = flipped_bias(current_bias);
      bias_serial(current_bias);
    }

    // if we're doing BOTH_DIR1, increment slow direction & i
	if(SD->scan_dir == BOTH_DIR1)
	{
	  next_second_pos = second_pos_now + scan_step;
	  if(next_second_pos > MAX) next_second_pos = MAX;  // safety
	  second_pos_now = move_to_protect_serial(second_ch,second_pos_now,next_second_pos,
		SD->inter_step_delay,SD->crash_protection,SD->z_limit_percent);

	  // draw line on screen
	  update_line(i);
#ifdef STRETCH
      StretchDIBits(hDC,dest_x + dest_offset_x,dest_y + dest_offset_y, 
        dest_width,dest_height,src_x,src_y,src_width,src_height, 
        bitmap_data,bitmapinfo,DIB_PAL_COLORS,SRCCOPY);
#else
      SetDIBitsToDevice(hDC,dest_x + dest_offset_x,dest_y + dest_offset_y,
        src_width,src_height,src_x,src_y,0,src_height, 
        bitmap_data,bitmapinfo,DIB_PAL_COLORS);
#endif
      if(SD->x_first)
	  {
        dest_y -= scan_display_factor;
	  }
      else dest_x += scan_display_factor;

	  // update status and timing display for BOTH_DIR1
      stime -= time_per_line;
      update_line_time(hDlg,stime);
      //sprintf(string,"%0.2f",stime);
      //SetDlgItemText(hDlg,SCAN_TIME,string);
	  i++;
      sprintf(string,"Scanning line %d",i + 1);
	  scan_status(hDlg,string,-1.0);

	}

	// line delay
	if(SD->inter_line_delay > 0)
	{
	  serial_soft_delay(SD->inter_line_delay);
	}

	// call scan_line() backward
	if(SD->scan_dir == BACKWARD_DIR || SD->scan_dir == BOTH_DIR1 ||
	   SD->scan_dir == BOTH_DIR2_POLAR || SD->scan_dir == BOTH_DIR2) 
	   read_it = SCANLINE_READON;
	else read_it = SCANLINE_READOFF;
    // call scan_setup();
	readseq_setup(SD);
    scan_setup(SD->crash_protection,SD->z_limit_percent,SD->inter_step_delay,
		     SD->step_delay);
	scan_result = scan_line(first_ch,read_it,SCANLINE_BACKWARD,first_line,
		      &(scan_buffer[0]),&first_pos_now);
	if(scan_result == SCANLINE_NOERR)
	{
	  if(read_it == SCANLINE_READON)
	  {
		if(SD->scan_dir == BACKWARD_DIR || SD->scan_dir == BOTH_DIR1)
          for(k = 0;k < num_data();k++)
		  {
			for(j = 0;j < scan_size;j++)
			{
              if(SD->x_first)
              {
				*(all_data[k]->ptr + i * scan_size + scan_size - j - 1) = scan_buffer[num_data() * j + k];
              }
		      else
              {
				*(all_data[k]->ptr + (scan_size - j - 1) * scan_size + i) = scan_buffer[num_data() * j + k];
              }
              if(k == 0)
			  {
                if(scan_buffer[num_data() * j + k] > max_z)
				  max_z = scan_buffer[num_data() * j + k];
                if(scan_buffer[num_data() * j + k] < min_z)
				  min_z = scan_buffer[num_data() * j + k];
			  }
			}
		  }
		else if(SD->scan_dir == BOTH_DIR2 || SD->scan_dir == BOTH_DIR2_POLAR)
          for(k = num_data()/2;k < num_data();k++)
		  {
            for(j = 0;j < scan_size;j++)
			{
              if(SD->x_first)
              {
				*(all_data[k]->ptr + i * scan_size + scan_size - j - 1) = scan_buffer[num_data()/2 * j + k - (num_data()/2)];
              }
		      else
              {
                *(all_data[k]->ptr + (scan_size - j - 1) * scan_size + i) = scan_buffer[num_data()/2 * j + k - (num_data()/2)];
              }
			  if(k == 0)
			  {
                if(scan_buffer[num_data()/2 * j + k - (num_data()/2)] > max_z)
				  max_z = scan_buffer[num_data()/2 * j + k - (num_data()/2)];
                if(scan_buffer[num_data()/2 * j + k - (num_data()/2)] < min_z)
				  min_z = scan_buffer[num_data()/2 * j + k - (num_data()/2)];
			  }
			}
          }
	  }
	} 
	else
	{
      stop_now = scan_result;
	  break;
	}

	// flip bias back if we're scanning BOTH_DIR2_POLAR
	if(SD->scan_dir == BOTH_DIR2_POLAR)
	{
      current_bias = flipped_bias(current_bias);
      bias_serial(current_bias);
    }
	
	// increment slow direction
	next_second_pos = second_pos_now + scan_step;
	if(next_second_pos > MAX) next_second_pos = MAX;  // safety
	second_pos_now = move_to_protect_serial(second_ch,second_pos_now,next_second_pos,
		SD->inter_step_delay,SD->crash_protection,SD->z_limit_percent);

	// draw line on screen
	update_line(i);
#ifdef STRETCH
    StretchDIBits(hDC,dest_x + dest_offset_x,dest_y + dest_offset_y, 
      dest_width,dest_height,src_x,src_y,src_width,src_height, 
      bitmap_data,bitmapinfo,DIB_PAL_COLORS,SRCCOPY);
#else
    SetDIBitsToDevice(hDC,dest_x + dest_offset_x,dest_y + dest_offset_y,
      src_width,src_height,src_x,src_y,0,src_height, 
      bitmap_data,bitmapinfo,DIB_PAL_COLORS);
#endif
    if(SD->x_first)
    {
      dest_y -= scan_display_factor;
    }
    else dest_x += scan_display_factor;

	// update timing display
    stime -= time_per_line;
    update_line_time(hDlg,stime);
    //sprintf(string,"%0.2f",stime);
    //SetDlgItemText(hDlg,SCAN_TIME,string);

	// peek for message to abort (ESC key) check once again. added by Joon 10/31/2003
    PeekMessage(&Message,hDlg,NULL,NULL,PM_REMOVE);
    if(LOWORD(Message.wParam) == VK_ESCAPE)
    {
	  stop_now = STOP_NOW_ABORTED;
      break;
    }

  }

  // clean up if we had to stop_now
  if(stop_now != SCANLINE_NOERR)
  {
	switch(stop_now)
	{
	  case STOP_NOW_ABORTED:
		// scan was aborted by user
        MessageBox(hDlg,"Scanning terminated.","ESC Key",MB_ICONEXCLAMATION);
		break;
	  case SCANLINE_CRASH:
		// scan was ended during crash protection
		MessageBox(hDlg,"Termination of scan. Crash protection.",
			      "Crashed",MB_ICONEXCLAMATION);
		break;
	  case SCANLINE_COMMERR:
		// comm error... don't try anymore serial communications
		MessageBox(hDlg,"Communications error! Abandoning scan!","Serious Error",MB_ICONEXCLAMATION);
		goto SKIP_MORE_SERIAL;
		break;
	  case SCANLINE_ERROR:
	  default:
        MessageBox(hDlg,"Unknown error. Scanning terminated.","Error",MB_ICONEXCLAMATION);
		// unknown error
		break;
	}
  }
  else
  {
	scan_status(hDlg,"Scan complete.",-1.0);
  }

  // restore physical variables: bias, tip position
  bias_serial(sample_bias);
  dac_data[first_ch] = move_to_protect_serial(first_ch,first_pos_now,ZERO,
	  SD->inter_step_delay,SD->crash_protection,SD->z_limit_percent);
  dac_data[second_ch] = move_to_protect_serial(second_ch,second_pos_now,ZERO,
	  SD->inter_step_delay,SD->crash_protection,SD->z_limit_percent);

SKIP_MORE_SERIAL:
  // add date & time to comment
  scan_time = time(NULL);
  strcpy(string,ctime(&scan_time));
  string[strlen(string) - 1] = '\0';
  strcpy(data->comment.ptr, string);
  data->comment.size = strlen(string);

  for(i = 0;i < num_data();i++)
  {
    set_scan_parameters(i);
  }

  // clean up graphics variables
  ReleaseDC(hDlg,hDC);

}

static void update_line(int num /*,int back*/)
{
  int i;
  float data_size;
  float z;

  data_size = max_z - min_z + 1.0;
/* OLD
  if(SD->scan_dir == BOTH_DIR1)
  {
    num *= 2;
    if(back) num += 1;
  }
*/
  for(i = 0;i < scan_size;i++)
  {
    if(SD->x_first)
    {
      z = *(data->ptr + num * scan_size + i);
/*
      *(bitmap_data + num * bitmap_size + i) = 
*/
      *(bitmap_data + i) = 
                (unsigned char)((unsigned long)((z - min_z) * LOGPALUSED) / data_size);
    }
    else
    {
      z = *(data->ptr + i * scan_size + num);
/*
      *(bitmap_data + i * bitmap_size + num) = 
*/
      *(bitmap_data + i * bitmap_size ) = 
                (unsigned char)((unsigned long)((z - min_z) * LOGPALUSED) / data_size);
    }
  }                                        
}

static void update_line_time(HWND hDlg,float the_time)
{
  if(the_time < 0.0) the_time = 0.0;
  sprintf(string,"%0.2f",the_time);
  SetDlgItemText(hDlg,SCAN_TIME,string);
}

static void set_scan_parameters(int num)
{
  float big_min_z,big_max_z;
  int i;
  int this_dir = FORWARD_DIR;
  int target;
  int data_count;
    
  if(SD->scan_dir == BACKWARD_DIR) this_dir = BACKWARD_DIR;
  if(SD->scan_dir == BOTH_DIR1) this_dir = BOTH_DIR1;
  target = num;
  if((SD->scan_dir == BOTH_DIR2 || SD->scan_dir == BOTH_DIR2_POLAR) &&
        num >= num_data() / 2)
  {
    this_dir = BACKWARD_DIR;
    target = num-num_data() / 2;
  }
    
  all_data[num]->valid = 1; 
  all_data[num]->saved = 0;
  strcpy(all_data[num]->filename,"Unsaved"); 
  strcpy(all_data[num]->full_filename,"Unsaved"); 
  all_data[num]->x = scan_x; 
  all_data[num]->y = scan_y; 
  all_data[num]->z = scan_z; 
  all_data[num]->x_range = x_range; 
  all_data[num]->y_range = y_range; 
  all_data[num]->step = scan_step; 
  all_data[num]->size = scan_size; 
  all_data[num]->x_gain = scan_x_gain; 
  all_data[num]->y_gain = scan_y_gain; 
  all_data[num]->z_gain = calc_z_gain(scan_scale); 
  all_data[num]->z_freq = scan_freq; 
  all_data[num]->x_offset_range = x_offset_range; 
  all_data[num]->y_offset_range = y_offset_range; 
  all_data[num]->bias = sample_bias; 
  all_data[num]->bias_range = sample_bias_range; 
  all_data[num]->i_setpoint = i_setpoint; 
  all_data[num]->i_set_range = i_set_range; 
  all_data[num]->amp_gain = tip_gain; 
    
  free_data_seq(&all_data[num]);
  alloc_data_seq(&all_data[num],SD->read_seq_num);
  all_data[num]->read_seq_num = SD->read_seq_num;
  memcpy(all_data[num]->read_seq,SD->read_seq,
        sizeof(READ_SEQ) * SD->read_seq_num);
    
  all_data[num]->digital_feedback_max = SD->digital_feedback_max;
  all_data[num]->digital_feedback_reread = SD->digital_feedback_reread;
  all_data[num]->z_limit_percent = SD->z_limit_percent; 
  all_data[num]->x_first = SD->x_first; 
  all_data[num]->step_delay = SD->step_delay; 
  all_data[num]->step_delay_fixed = SD->step_delay_fixed; 
  all_data[num]->inter_step_delay = SD->inter_step_delay; 
  all_data[num]->digital_feedback = SD->digital_feedback; 
  all_data[num]->tip_spacing = SD->tip_spacing; 
  all_data[num]->inter_line_delay = SD->inter_line_delay; 
  all_data[num]->scan_feedback = SD->scan_feedback; 
  all_data[num]->crash_protection = SD->crash_protection; 
  all_data[num]->overshoot = SD->overshoot; 
  all_data[num]->overshoot_percent = SD->overshoot_percent; 
  all_data[num]->overshoot_wait1 = SD->overshoot_wait1; 
  all_data[num]->overshoot_wait2 = SD->overshoot_wait2; 
  all_data[num]->scan_dither0 = SD->scan_dither0; 
  all_data[num]->scan_dither1 = SD->scan_dither1; 
  all_data[num]->version = STM_DATA_VERSION;

  data_count = 0;
  for(i = 0;i < SD->read_seq_num;i++)
  {
    if(SD->read_seq[i].record && data_count == target)
    {
      all_data[num]->scan_num = SD->read_seq[i].num_samples; 
      all_data[num]->read_ch = SD->read_seq[i].read_ch; 
      all_data[num]->read_feedback = SD->read_seq[i].feedback; 
      all_data[num]->dither0 = SD->read_seq[i].dither0; 
      all_data[num]->dither1 = SD->read_seq[i].dither1; 
      all_data[num]->current_read_seq = i;
    }
    if(SD->read_seq[i].record) data_count++;
  }
  if (num!=0)
  {
      all_data[num]->temperature = all_data[0]->temperature; 
      strcpy(all_data[num]->comment.ptr,all_data[0]->comment.ptr);
      all_data[num]->comment.size = all_data[0]->comment.size;
      strcpy(all_data[num]->sample_type.ptr,all_data[0]->sample_type.ptr);
      all_data[num]->sample_type.size = strlen(all_data[0]->sample_type.ptr);
      strcpy(all_data[num]->dosed_type.ptr,all_data[0]->dosed_type.ptr);
      all_data[num]->dosed_type.size = strlen(all_data[num]->dosed_type.ptr);
      all_data[num]->dosed_langmuir  = all_data[0]->dosed_langmuir;
      if(SD->scan_dir == BOTH_DIR2_POLAR && num >= num_data() / 2) 
      all_data[num]->bias = flipped_bias(sample_bias);
  }
    
  all_data[num]->scan_dir = this_dir; 
    
  find_min_max(all_data[num],&big_min_z,&big_max_z);
  all_data[num]->clip_min = big_min_z; 
  all_data[num]->clip_max = big_max_z; 

}
   
// ******************************************************************************************
//		O L D * O L D * O L D * O L D * O L D * O L D * O L D * O L D * O L D * O L D
// ******************************************************************************************


#ifdef OLD

void scan_old(HWND hDlg)
{
    HDC             hDC;
    LOGBRUSH        lplb;
    HBRUSH          hBrush,
                    hBrushOld;
    HPEN            hPen,
                    hPenOld;
    int delta_z_offset=0;
    float delta_z_factor=1;
    float last_line_min,last_line_max,this_line_min,this_line_max,
        temp_min,temp_max,temp_range;
    int             i,
                    j,
                    k;
    unsigned int high_limit,low_limit,high_target,low_target;
    unsigned int    offset;
    unsigned int    n,
                    first_ch,
                    second_ch, first_offset, second_offset;
    unsigned int   *x,
                   *y,
                   *para_ptr,
                    end_y;
    float           z;
    int             forward = 1;
    MSG             Message;
    time_t          scan_time;
    int             max_x;
    float           stime,time_per_line;
    unsigned int old_scan_z;
    float delta_z;
    int stop_now=0;
    int     Imax,Imin;
    int dest_x, dest_y, dest_width, dest_height, src_x, 
        src_y, src_width, src_height, dest_offset_x, dest_offset_y;
    int line_size;
    int z_offset_crashed;
    unsigned int    x_gain,y_gain,z_gain,z2_gain;
    unsigned int current_bias=sample_bias;
    float *this_data;
    int feedback_now;
    int dither0_now , dither1_now;
    int read_ch_now;
    int this_seq;
    int second_pass_offset;
    int data_per_pass;
    int data_pos=0;
    int z_scan_num=1;
    
#ifdef TIME_ERROR
    int time_error_line=0;
    int time_error_step=0;
    int time_error_inter=0;
#endif    
    
    second_pass_offset=num_data()/2;
    if (SD->scan_dir==BOTH_DIR2 || SD->scan_dir==BOTH_DIR2_POLAR)
    {
        second_pass_offset=data_per_pass=num_data()/2;
    } else 
    {
        data_per_pass=num_data();
        second_pass_offset=0;
    }
    
    
    

    feedback_now=SD->scan_feedback;
    dither0_now=dio_dither_status(0);
    dither1_now=dio_dither_status(1);
        
    if (SD->inter_line_delay) dio_start_clock(SD->inter_line_delay);
    scan_status(hDlg,"Scanning line 1",-1.0);
    delta_z=0;
    src_x=src_y=0;
    
    hDC = GetDC(hDlg);
    SetMapMode(hDC, MM_TEXT);

//    set_Plasma_palette();
    SelectPalette(hDC, hPal, 0);
    RealizePalette(hDC);
    lplb.lbStyle = BS_SOLID;
    lplb.lbColor = RGB(0,0,0);
    hBrush = CreateBrushIndirect(&lplb);
    hBrushOld = SelectObject(hDC, hBrush);
    hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    hPenOld = SelectObject(hDC, hPen);
    Rectangle(hDC, IMAGE_X_POS, IMAGE_Y_POS, IMAGESIZE + IMAGE_X_POS, IMAGESIZE + IMAGE_Y_POS);
    SelectObject(hDC, hBrushOld);
    SelectObject(hDC, hPenOld);
    DeleteObject(hBrush);
    DeleteObject(hPen);
    
    stime=calc_time(scan_size);
    time_per_line=stime/(float)scan_size;
    Imax=in_vtod(pow(10,SD->tip_spacing-dtov(i_setpoint,i_set_range)/10.0));
    Imin=in_vtod(pow(10,-SD->tip_spacing-dtov(i_setpoint,i_set_range)/10.0));
    
    high_limit=IN_MAX-(unsigned int) ((float)IN_MAX*SD->z_limit_percent/100);
    low_limit=(unsigned int)((float)IN_MAX*SD->z_limit_percent/100);
    low_target=DEFAULT_Z_OFFSET_MOVE*low_limit;
    high_target=IN_MAX-(unsigned int) ((float)IN_MAX*SD->z_limit_percent*DEFAULT_Z_OFFSET_MOVE/100);
    
    this_data=(float *)malloc(sizeof(float)*num_data());

/*
    sprintf(string,"low target %d high target %d",low_target,high_target);
    if (digital_feedback) MessageBox(hDlg, string, "Digital feedback settings", MB_ICONEXCLAMATION);
    sprintf(string,"min %d max %d",Imin,Imax);
    if (digital_feedback) MessageBox(hDlg, string, "Digital feedback settings", MB_ICONEXCLAMATION);
*/

    switch (scan_scale)
    {
        case 0:
            delta_z_factor=100;
            break;
        case 1:
            delta_z_factor=10;
            break;
        case 2:
            delta_z_factor=1;
            break;
    }
    line_size=find_size(scan_size);
#ifdef STRETCH
    scan_display_factor=IMAGESIZE/scan_size;
#else
    scan_display_factor = 1;
#endif
    bitmap_size=line_size;
    if (bitmap_data == NULL)
        bitmap_data = (unsigned char *) malloc(bitmap_size * bitmap_size * sizeof(unsigned char));
    else
        bitmap_data = (unsigned char *) realloc((void *) bitmap_data, bitmap_size * bitmap_size * sizeof(unsigned char));
    dest_offset_x=IMAGE_X_POS;
    dest_offset_x+=(IMAGESIZE-scan_size*scan_display_factor)/2;
    dest_offset_y = dest_offset_x;
    bitmapinfo->bmiHeader.biBitCount = 8;
    bitmapinfo->bmiHeader.biWidth = line_size;
    bitmapinfo->bmiHeader.biHeight = line_size;
    
    if (SD->x_first)
    {
        first_ch = x_ch;
        first_offset = x_offset_ch;
        second_ch = y_ch;
        second_offset = y_offset_ch;
        src_width = scan_size;
        src_height = 1;
        dest_width = scan_size*scan_display_factor;
        dest_height = scan_display_factor;
        dest_x = 0;    
        dest_y = scan_size*scan_display_factor;
    } else
    {
        first_ch = y_ch;
        first_offset = y_offset_ch;
        second_ch = x_ch;
        second_offset = x_offset_ch;
        src_width = 1;
        src_height = scan_size;
        dest_height = scan_size*scan_display_factor;
        dest_width = scan_display_factor;
        dest_x = 0;    
        dest_y = 0;
    }

    // initialize min_z and max_z
    min_z = 7000000;
    max_z = -7000000;

    offset = ZERO - scan_step * scan_size / 2;
    x = y = para_ptr = stair_move_setup(scan_step, scan_size, offset);
    n = stair_size(scan_step);
    _disable();
    if (SD->scan_dir!=BOTH_DIR1) max_x=scan_size;
    else max_x=scan_size/2;   /* ## WARNING! need to do extra for odd sized scans ! */
#ifdef TIME_ERROR
    if ((inpw(stat)&0x4000))    time_error_line++;
#endif

    dio_in_ch(zi_ch);
    read_ch_now=zi_ch;
    
    if (SD->inter_line_delay) dio_wait_clock();
    for (j = 0; j < max_x && !stop_now; j++)
    {
        
        PeekMessage(&Message, hDlg, NULL, NULL, PM_REMOVE);
        if (LOWORD(Message.wParam) == VK_ESCAPE)
        {
            MessageBox(hDlg, "Scanning terminated.", "Warning", 
                MB_ICONEXCLAMATION);
            break;
        }

        for (k = 0; k < scan_size && !stop_now; k++)
        {

            if (SD->scan_dir == FORWARD_DIR || SD->scan_dir == BOTH_DIR1 || 
                SD->scan_dir == BOTH_DIR2 || SD->scan_dir==BOTH_DIR2_POLAR)
            {    
                dio_start_clock(SD->step_delay); 
#ifdef TIME_ERROR 
    if ((inpw(stat)&0x4000))    time_error_step++; 
#endif 
                dio_wait_clock(); 
                data_pos=0;
                READ_Z();
                Z_CALC_MINMAX();
                Z_CRASH_PROTECT();
            }
            if (k < scan_size - 1)
            {
                for (i = 0; i < n-1 && !stop_now; i++)
                {
                    y++;
                    dio_out(first_ch, *y);
                    dio_start_clock(SD->inter_step_delay);
#ifdef TIME_ERROR
    if ((inpw(stat)&0x4000))    time_error_inter++;
#endif
                    dio_wait_clock();
                    DO_DIGITAL_FEEDBACK();
                }                
                if (stop_now) goto STOP_NOW;
                y++;
                dio_out(first_ch,*y);
                if (SD->scan_dir==BACKWARD_DIR) 
                {
                    dio_start_clock(SD->inter_step_delay);
#ifdef TIME_ERROR
    if ((inpw(stat)&0x4000))    time_error_inter++;
#endif
                    dio_wait_clock();
                    DO_DIGITAL_FEEDBACK();
                    if (stop_now) goto STOP_NOW;
                    if (SD->crash_protection!=CRASH_DO_NOTHING)
                    {
                        if (read_ch_now!=zi_ch)
                        {
                            dio_in_ch(zi_ch);
                            read_ch_now=zi_ch;
                        }
                        z = dio_read(1);
                        Z_CALC_MINMAX();
                        Z_CRASH_PROTECT_BACK();
                    }
                }
            }

            if (SD->scan_dir == FORWARD_DIR || SD->scan_dir==BOTH_DIR2 || SD->scan_dir==BOTH_DIR2_POLAR)
            {
                for(i=0;i<data_per_pass;i++)
                {
                    if (SD->x_first)
                        *(all_data[i]->ptr + j * scan_size + k) = this_data[i];
                    else
                        *(all_data[i]->ptr + k * scan_size + j) = this_data[i];
                }
            }
            if (SD->scan_dir == BOTH_DIR1)
            {
                for(i=0;i<data_per_pass;i++)
                {
                    if (SD->x_first)
                        *(all_data[i]->ptr + j*2 * scan_size + k) = this_data[i];
                    else
                        *(all_data[i]->ptr + k * scan_size + j*2) = this_data[i];
                }
            }
                
        }
        if (stop_now) goto STOP_NOW;
        end_y = *y;
        if (SD->inter_line_delay) dio_start_clock(SD->inter_line_delay);
        if (read_ch_now!=zi_ch) 
        {
            dio_in_ch(zi_ch);
            read_ch_now=zi_ch;
        }
        if (SD->scan_dir == BOTH_DIR1)
        {
            stime-=time_per_line;
            sprintf(string,"%0.2f",stime);
            SetDlgItemText(hDlg, SCAN_TIME, string);
            scan_status(hDlg,"Scanning line %0.0f",(float) (j*2+1));
        }
        if (SD->scan_dir==BOTH_DIR2_POLAR)
        {
            current_bias=flipped_bias(current_bias);
            bias(current_bias);
        }
        if (SD->scan_dir!=BACKWARD_DIR)
        {
            update_line(j,0);
/*
    sprintf(string,"%d %d %d %d",dest_x,dest_y,dest_width,dest_height);
    MessageBox(hDlg, string, "Test", MB_ICONEXCLAMATION);
*/
#ifdef STRETCH
            StretchDIBits(hDC, dest_x+dest_offset_x, dest_y+dest_offset_y, 
                dest_width, dest_height, src_x, src_y, src_width, src_height, 
                bitmap_data, bitmapinfo, DIB_PAL_COLORS,SRCCOPY);
#else
            SetDIBitsToDevice(hDC,dest_x+dest_offset_x, dest_y+dest_offset_y,
                src_width, src_height, src_x, src_y, 0, src_height, 
                bitmap_data, bitmapinfo, DIB_PAL_COLORS);
#endif
            if (SD->x_first)
            {
                dest_y-=scan_display_factor;
            }
            else dest_x+=scan_display_factor;
        }
#ifdef TIME_ERROR
    if ((inpw(stat)&0x4000))    time_error_line++;
#endif
        if (SD->inter_line_delay) dio_wait_clock();
        if (SD->scan_dir == BOTH_DIR1)
        {
            for (i = 0; i < n; i++)
            {
                x++;
                dio_out(second_ch, *x);
                dio_start_clock(SD->inter_step_delay);
#ifdef TIME_ERROR
    if ((inpw(stat)&0x4000))    time_error_inter++;
#endif
                dio_wait_clock();
                DO_DIGITAL_FEEDBACK(); 
            }
            
        }
        for (k = 0; k < scan_size && !stop_now; k++) /* move y back */
        {
            if (SD->scan_dir == BACKWARD_DIR || SD->scan_dir == BOTH_DIR1 ||
                SD->scan_dir==BOTH_DIR2 || SD->scan_dir==BOTH_DIR2_POLAR)
            {
                dio_start_clock(SD->step_delay);
#ifdef TIME_ERROR
    if ((inpw(stat)&0x4000))    time_error_step++;
#endif
                dio_wait_clock();
                data_pos=second_pass_offset;
                
                READ_Z();
                Z_CALC_MINMAX();
                Z_CRASH_PROTECT();
            }
            if (k < scan_size - 1)
            {
                for (i = 0; i < n-1 && !stop_now; i++)
                {
                    y--;
                    dio_out(first_ch, *y);
                    dio_start_clock(SD->inter_step_delay);
#ifdef TIME_ERROR
    if ((inpw(stat)&0x4000))    time_error_inter++;
#endif
                    dio_wait_clock();
                    DO_DIGITAL_FEEDBACK();
                }
                if (stop_now) goto STOP_NOW;
                y--;
                dio_out(first_ch,*y);
                if (SD->scan_dir==FORWARD_DIR)
                {
                    dio_start_clock(SD->inter_step_delay);
#ifdef TIME_ERROR
    if ((inpw(stat)&0x4000))    time_error_inter++;
#endif
                    dio_wait_clock();
                    DO_DIGITAL_FEEDBACK();
                    if (stop_now) goto STOP_NOW;
                    if (SD->crash_protection!=CRASH_DO_NOTHING)
                    {
                        if (read_ch_now!=zi_ch)
                        {
                            dio_in_ch(zi_ch);
                            read_ch_now=zi_ch;
                        }
                        z = dio_read(1);
                        Z_CALC_MINMAX();
                        Z_CRASH_PROTECT_BACK();
                    }
                }
            }
            if (SD->scan_dir == BACKWARD_DIR)
                for(i=0;i<data_per_pass;i++)
                {
                    if (SD->x_first)
                        *(all_data[i]->ptr + j * scan_size + scan_size-k-1) = this_data[i];
                    else
                        *(all_data[i]->ptr + (scan_size-k-1) * scan_size + j) = this_data[i];
                }
            else if (SD->scan_dir == BOTH_DIR1)
                for(i=0;i<data_per_pass;i++)
                {
            
                    if (SD->x_first)
                        *(all_data[i]->ptr + (2*j+1) * scan_size + scan_size - k - 1) = this_data[i];
                    else
                        *(all_data[i]->ptr + (scan_size-k-1) * scan_size + 2*j+1) = this_data[i];
                        
                }
            else if (SD->scan_dir == BOTH_DIR2 || SD->scan_dir==BOTH_DIR2_POLAR)
                for(i=second_pass_offset;i<data_per_pass+second_pass_offset;i++)
                {
                    if (SD->x_first)
                        *(all_data[i]->ptr + j * scan_size + scan_size - k - 1) = this_data[i];
                    else
                        *(all_data[i]->ptr + (scan_size-k-1) * scan_size + j) = this_data[i];
                }
        }
        if (j<max_x-1) for (i = 0; i < n ; i++)
        {
            x++;
            dio_out(second_ch, *x);
            dio_start_clock(SD->inter_step_delay);
#ifdef TIME_ERROR
    if ((inpw(stat)&0x4000))    time_error_inter++;
#endif
            dio_wait_clock();
            DO_DIGITAL_FEEDBACK();
        }
        if (stop_now) goto STOP_NOW;
        if (SD->inter_line_delay) dio_start_clock(SD->inter_line_delay);
        dio_in_ch(zi_ch);
        read_ch_now=zi_ch;
        stime-=time_per_line;
        sprintf(string,"%0.2f",stime);
        SetDlgItemText(hDlg, SCAN_TIME, string);
        if (SD->scan_dir!=BOTH_DIR1) scan_status(hDlg,"Scanning line %0.0f",(float)(j+1));
        else scan_status(hDlg,"Scanning line %0.0f",(float)(2*j+1));
        if (SD->scan_dir==BOTH_DIR2_POLAR)
        {
            current_bias=flipped_bias(current_bias);
            bias(current_bias);
        }
        if (SD->scan_dir == BACKWARD_DIR || SD->scan_dir == BOTH_DIR1)
        {
            update_line(j,1);
#ifdef STRETCH
            StretchDIBits(hDC, dest_x+dest_offset_x, dest_y+dest_offset_y, 
                dest_width, dest_height, src_x, src_y, src_width, src_height, 
                bitmap_data, bitmapinfo, DIB_PAL_COLORS,SRCCOPY);
#else
            SetDIBitsToDevice(hDC,dest_x+dest_offset_x, dest_y+dest_offset_y,
                src_width, src_height, src_x, src_y, 0, src_height, 
                bitmap_data, bitmapinfo, DIB_PAL_COLORS);
#endif
            if (SD->x_first)
            {
                dest_y-=scan_display_factor;
            }
            else dest_x+=scan_display_factor;
        }   
#ifdef TIME_ERROR
    if ((inpw(stat)&0x4000))    time_error_line++;
#endif
        if (SD->inter_line_delay) dio_wait_clock();
    }
STOP_NOW:
    if (stop_now==Z_CRASHED)
    {
        MessageBox(hDlg, "Z Crash Protection! Scanning Terminated.", "Warning", MB_ICONEXCLAMATION);
        z_offset_crashed=0;
        switch (scan_scale)
        {
            case 0:
                scan_freq-=2;
                if (scan_freq<0) scan_freq=0;
                if (auto_z_above(IN_ZERO))
                {
                    scan_scale=1;
                    calc_gains(&x_gain, &y_gain, &z_gain, &z2_gain);
                    set_gain(x_gain, y_gain, z_gain, z2_gain);
                    z_offset_crashed=!auto_z_above(IN_ZERO);
                }
                else 
                {
                    z_offset_crashed=1;
                }
                break;
            case 1:
                scan_freq-=1;
                if (scan_freq<0) scan_freq=0;
                z_offset_crashed=!auto_z_above(IN_ZERO);
                break;
        }
        if (z_offset_crashed)
        {
            MessageBox(hDlg, "Could not change Z-gain to 10!", "Warning", 
                MB_ICONEXCLAMATION);
            /* move back with no digital feedback/abort, force it */
            move_to_protect2(first_ch, *y, 
                dac_data[first_ch], SD->inter_step_delay,SD->crash_protection,
                SD->z_limit_percent,0,Imin,Imax,0,1,
                SD->digital_feedback_max,SD->digital_feedback_reread);
            move_to_protect2(second_ch, *x, 
                dac_data[second_ch], SD->inter_step_delay,SD->crash_protection,
                SD->z_limit_percent,0,Imin,Imax,0,1,
                SD->digital_feedback_max,SD->digital_feedback_reread);
        }
        else
        {
            scan_scale=2;
            calc_gains(&x_gain, &y_gain, &z_gain, &z2_gain);
            set_gain(x_gain, y_gain, z_gain, z2_gain);
            move_to_protect2(first_ch, *y, 
                dac_data[first_ch], SD->inter_step_delay,SD->crash_protection,
                SD->z_limit_percent,SD->digital_feedback,Imin,Imax,0,1,
                SD->digital_feedback_max,SD->digital_feedback_reread);
            move_to_protect2(second_ch, *x, 
                dac_data[second_ch], SD->inter_step_delay,SD->crash_protection,
                SD->z_limit_percent,SD->digital_feedback,Imin,Imax,0,1,
                SD->digital_feedback_max,SD->digital_feedback_reread);
        }
    }
    else
    {
        if (stop_now)
        {
            MessageBox(hDlg, "I Crash Protection! Scanning Terminated.", 
                "Warning", MB_ICONEXCLAMATION);
        }
        move_to_protect2(first_ch, *y, 
            dac_data[first_ch], SD->inter_step_delay,SD->crash_protection,
            SD->z_limit_percent,SD->digital_feedback,Imin,Imax,0,1,
            SD->digital_feedback_max,SD->digital_feedback_reread);
        move_to_protect2(second_ch, *x, 
            dac_data[second_ch], SD->inter_step_delay,SD->crash_protection,
            SD->z_limit_percent,SD->digital_feedback,Imin,Imax,0,1,
            SD->digital_feedback_max,SD->digital_feedback_reread);
    }
    _enable();
/*
    MessageBox(hDlg,"Test 1","Test Title",MB_OKCANCEL|MB_ICONEXCLAMATION);
    _enable();
    MessageBox(hDlg,"Test 2","Test Title",MB_OKCANCEL|MB_ICONEXCLAMATION);
*/
#ifdef TIME_ERROR
    sprintf(string,"%d %d %d",time_error_line,time_error_step,time_error_inter);
    MessageBox(hDlg,string,"Errors",MB_OKCANCEL|MB_ICONEXCLAMATION);
    SetDlgItemInt(hDlg, SCAN_STEP_ANGS_EDIT, time_error_line, FALSE);
#endif
    stair_free(para_ptr);
    bias(sample_bias);
    
    scan_time= time(NULL);
    strcpy(string, ctime(&scan_time));
    string[strlen(string)-1] = '\0';
    strcpy(data->comment.ptr, string);
    data->comment.size = strlen(string);
    for(i=0;i<num_data();i++)
    {
        set_scan_parameters(i);
    }
#ifdef OLD
    SET_DATA(data);    
    
/*
    strcpy(data->sample_type.ptr, sample_type);
    data->sample_type.size = strlen(sample_type);
*/
    if (SD->scan_dir == BOTH_DIR2 || SD->scan_dir==BOTH_DIR2_POLAR)
    {
        SET_DATA(data2);
        strcpy(data2->comment.ptr,data->comment.ptr);
        data2->comment.size=data->comment.size;
        if (SD->scan_dir==BOTH_DIR2_POLAR) data2->bias=flipped_bias(sample_bias);
    }
#endif    
    
    ReleaseDC(hDlg, hDC);
    
}

#endif