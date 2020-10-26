#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <dos.h>
#include <windows.h>
#include "hop.h"
#include "data.h"
#include "dio.h"
#include "stm.h"
#include "common.h"
#include "file.h"
#include "clock.h"
#include "serial.h"
#include <intrin.h>

BOOL FAR PASCAL HopTrackDlg( HWND  hDlg, unsigned Message, WPARAM wParam, LPARAM lParam);
static void measure_local_heights( unsigned int x, unsigned int y, float z_values[]);
static int move_toward_extreme( unsigned int *x, unsigned int *y, float z_values[]);
static unsigned int hop_add_and_check(unsigned int current, float modifier);

void calculate_new_coordinates(unsigned int *,unsigned int *,int,int);

static void repaint_params( HWND hDlg);
static void enable_all( HWND hDlg, int status);

extern char string[];
extern char *current_file_hop;
extern unsigned int dac_data[];
extern datadef *data, **glob_data;
extern BITMAPINFO *bitmapinfo;
extern HPALETTE hPal;
extern unsigned char *bitmap_data;
extern HANDLE   hInst;
extern char *current_file_exp;
extern unsigned int	scan_fine_x,
                	scan_fine_y;


#ifdef OLD_HOP_TIMING
long *time_hop;
#else
float *time_hop;
#endif

float	hop_timer;

//float *hop_timer_array;

#ifdef OLD_HOP_TIMING
long 	  hop_start_time, hop_current_time;
#endif

float	  hop_card_clock, hop_last_clock;

static int	feature_type = HOP_BUMP;
static int	hop_step_delay = 1000;  /* in microsecs */
static int	hop_inter_step_delay = 1000;  /* in microsecs */
static int	hop_radius = 25; /* radius of search circle, in bits */
static int	hop_avg_data_pts = 100;
static float	hop_exponent = 1.0;
static int	hop_tracking = 0; /* boolean: are we tracking or not? */
static int	hop_index = 0;
static int	hop_every = 1000; /* in microsecs */
static float	pixels_per_bit = 1.0; /* how many pixels on screen correspond to one bit */
static float	hop_plane_a = 0.0; /* user supplies plane z = ax + by + c */
static float	hop_plane_b = 0.0;
static float	hop_plane_c = 0.0;

unsigned int hop_lobyte, hop_hibyte;

datadef *hop_data;

static int out_smart = 0;
static int first_time = 1;

BOOL FAR PASCAL HopTrackDlg( HWND hDlg, unsigned Message, WPARAM wParam, LPARAM lParam)
{

  unsigned int current_x, current_y;  /* current position, i.e. center of circle */
  int x_slope,y_slope;
  unsigned int NE_x, NE_y; /* "Northeast" of current position */
  MSG             hop_Message;
  int j,n;
  float z_values[HOP_NUM_NEIGHBORS];
  PAINTSTRUCT ps;
  HPEN hPen, hPenOld;
  HBRUSH hBrush, hBrushOld;
  HDC hDC, hop_DC;
  int screen_x, screen_y;
  WORD err;

  switch( Message) {

    case WM_INITDIALOG:

      if( first_time) {

        // allocate memory and initialize hop_data
	alloc_data( &hop_data, DATATYPE_HOP, HOP_MAX_ITERATIONS, GEN2D_USHORT, GEN2D_USHORT, 0);
#ifdef OLD_HOP_TIMING
	time_hop = (long*) malloc( HOP_MAX_ITERATIONS * sizeof(long));
#else
	time_hop = (float*) malloc( HOP_MAX_ITERATIONS * sizeof(float));
#endif
	//hop_timer_array = (float*) malloc( HOP_MAX_ITERATIONS * sizeof(float));

	if( hop_data == NULL) {
	  MessageBox( hDlg, "Unable to alloc_data for hop_data", "WARNING", MB_OKCANCEL);
	  break;
	}
	for( n = 0; n < HOP_MAX_ITERATIONS; n++) {
          hop_data->xd[n] = (unsigned short) 0;
          hop_data->yd[n] = (unsigned short) 0;
#ifdef OLD_HOP_TIMING
	  *(time_hop+n) = (long) 0;
#else
	  *(time_hop+n) = 0.0;
#endif
#ifdef HOP_TIMER_CALIBRATION
	*(hop_timer_array+n) = 0;
#endif
        }

	hop_data->saved = 1; /* since there is no data, pretend it's saved so message box won't appear */

#ifdef HOP_BOGUS_INIT

	*(time_hop) = hop_timer = 0.0;
	RESET_CARD_CLOCK();
	for( hop_index = 1; hop_index < 1000; hop_index++) {
		delay_calibrate( 1 + hop_index/100);
		READ_HOP_TIMER();
		*(time_hop + hop_index) = hop_timer;
		dio_in_ch( zi_ch);
	}
	clock_used = 0;
	hop_data->saved = 0;

#endif

	SetDlgItemText( hDlg, HOP_ESC_MESSAGE, HOP_EMPTY_STR);

	first_time = 0;
      } 

      if( data->valid) {
	hop_data->temperature = data->temperature;
	
	/* place to copy additional relevant parameters from data to hop_data */
	pixels_per_bit = ((float) HOP_DISPLAY_WIDTH) / ((float) ( data->size * data->step));
      }
      else {
	pixels_per_bit = 1;
      }
      hop_data->size = hop_index;
      repaint_params( hDlg);
      break;

    case WM_PAINT:
      hDC = BeginPaint( hDlg, &ps);

      // draw image rectangle, 256 x 256 pixels
      SetMapMode( hDC, MM_TEXT);
      hPen = GetStockObject( BLACK_PEN);
      hBrush = GetStockObject( WHITE_BRUSH);
      hBrushOld = SelectObject( hDC, hBrush);
      hPenOld = SelectObject( hDC, hPen);
      Rectangle( hDC, HOP_DISPLAY_LEFT, HOP_DISPLAY_TOP, HOP_DISPLAY_RIGHT, HOP_DISPLAY_BOTTOM);
      SelectObject(hDC, hBrushOld);
      SelectObject(hDC, hPenOld);

      // draw scanned image, if there is one
      if( data->valid) {
        SelectPalette(hDC,hPal,0);
        RealizePalette(hDC);
        err = StretchDIBits(hDC, HOP_DISPLAY_LEFT, HOP_DISPLAY_TOP, 
          HOP_DISPLAY_WIDTH, HOP_DISPLAY_HEIGHT, 0, 0, data->size, data->size, 
          bitmap_data, bitmapinfo, DIB_PAL_COLORS, SRCCOPY);	
      }

      // draw data points
      for( j = 0; j < hop_index; j++) {
	screen_x = (((int)hop_data->xd[j])-ZERO)*pixels_per_bit + ZERO_PIXEL + HOP_DISPLAY_LEFT;
	screen_y = -(((int)hop_data->yd[j])-ZERO)*pixels_per_bit + ZERO_PIXEL + HOP_DISPLAY_TOP;
	if( ( screen_x > HOP_DISPLAY_LEFT) && ( screen_x < HOP_DISPLAY_RIGHT) &&
		( screen_y > HOP_DISPLAY_TOP) && ( screen_y < HOP_DISPLAY_BOTTOM) ){
	            SetPixel(hDC, screen_x, screen_y, RGB(0,0,0) );
        }
      }

      DeleteObject(hBrush);
      DeleteObject(hPen);
      EndPaint( hDlg, &ps);
      break;

    case WM_COMMAND:
      switch( LOWORD( wParam)) {

        case HOP_START_TRACKING:

#ifdef HOP_TIMER_CALIBRATION // only works with OLD_HOP_TIMING
	  clock_used = 0;
	  hop_timer = 0.0;
	  //RESET_CARD_CLOCK();
	  hop_start_time = hop_current_time = GetTickCount();
	  for( hop_index = 0; hop_index < 100; hop_index ++) {
	  	hop_delay_for( 500000);
		//READ_HOP_TIMER();
		*(hop_timer_array + hop_index) = hop_timer;
	        *(time_hop + hop_index) = hop_current_time;	    
	  }
	  clock_used = 0;
#endif
          if( !hop_data->saved) {
	    if( MessageBox( hDlg, "Data not saved!", "Warning", MB_OKCANCEL | MB_ICONEXCLAMATION) == IDCANCEL) {
	      break;
	    }
	  }

	  hop_DC = GetDC( hDlg);
	  SetMapMode( hop_DC, MM_TEXT);
          SelectPalette(hop_DC,hPal,0);
          RealizePalette(hop_DC);
          StretchDIBits(hop_DC, HOP_DISPLAY_LEFT, HOP_DISPLAY_TOP, 
                HOP_DISPLAY_WIDTH, HOP_DISPLAY_HEIGHT, 0, 0, data->size, data->size, 
                bitmap_data, bitmapinfo, DIB_PAL_COLORS,SRCCOPY);	

	  enable_all( hDlg, 0);

	  hop_index = 0;
	  hop_tracking = 1;

	  __MACHINE(void __cdecl _disable()); // disable interrupts

	  // set and record current position
	  current_x = hop_data->xd[hop_index] = dac_data[x_ch];
	  current_y = hop_data->yd[hop_index] = dac_data[y_ch];
	  hop_index++;

#ifdef OLD_HOP_TIMING
	  hop_start_time = hop_current_time = GetTickCount();
	  *(time_hop+hop_index) = hop_current_time - hop_start_time;
#else
	  *(time_hop + hop_index) = hop_timer = 0.0;
	  clock_used = 0;
	  //RESET_CARD_CLOCK();
#endif
	  //start of the code for the DSP communication
	  hop_setup(hop_avg_data_pts, hop_step_delay, hop_inter_step_delay, hop_every, hop_radius, 
			   hop_plane_a, hop_plane_b);

	  hop_start();
	  while(hop_tracking) {
	
		  hop_data->xd[hop_index] = current_x;
	      hop_data->yd[hop_index] = current_y;
		 
		  *(time_hop + hop_index) = hop(current_x, current_y, &x_slope, &y_slope);
		  calculate_new_coordinates(&current_x, &current_y, x_slope, y_slope);
		  
		  

	      screen_x = (((int)hop_data->xd[hop_index])-ZERO)*pixels_per_bit + ZERO_PIXEL + HOP_DISPLAY_LEFT;
	      screen_y = -(((int)hop_data->yd[hop_index])-ZERO)*pixels_per_bit + ZERO_PIXEL + HOP_DISPLAY_TOP;
   
	      if( (screen_x > HOP_DISPLAY_LEFT) && (screen_x < HOP_DISPLAY_RIGHT)
	     	     && (screen_y < HOP_DISPLAY_BOTTOM) && (screen_y > HOP_DISPLAY_TOP) ) {
	  	 SetPixel(hop_DC, screen_x, screen_y, RGB(0,0,0) );
	      }

		  PeekMessage(&hop_Message, hDlg, NULL, NULL, PM_REMOVE);
		   if ( (LOWORD(hop_Message.wParam) == VK_ESCAPE) || (LOWORD(hop_Message.wParam) == HOP_STOP_TRACKING))
              {
				hop_tracking = 0;
              }

		   if( ++hop_index > (HOP_MAX_ITERATIONS-HOP_PADDING)) {
	        MessageBox( hDlg,"Molecule hopped more times than allowed.","TOO MANY HOPS!",MB_OKCANCEL);
	        hop_tracking = 0;
	      }
		   if (hop_tracking) cont();
	  }
	  track_terminate();	//will also work for terminating the hop

	/*	
	  // move tip "northeast" of current position
          NE_x = current_x + hop_radius * cos( PI/4);
	  NE_y = current_y + hop_radius * sin( PI/4);
	  dac_data[x_ch] = move_to_protect_hop( x_ch, current_x, NE_x,
				   hop_inter_step_delay, CRASH_STOP,
				   HOP_LIMIT_PERCENT);
	  dac_data[y_ch] = move_to_protect_hop( y_ch, current_y, NE_y,
				   hop_inter_step_delay, CRASH_STOP,
				   HOP_LIMIT_PERCENT);
	  if( (dac_data[x_ch] != NE_x) || (dac_data[y_ch] != NE_y) ) {
	    hop_tracking = 0;
	    MessageBox( GetFocus(),"Unable to move tip to desired location","ABORTING",MB_OKCANCEL);
	    enable_all( hDlg, 1);
	    hop_data->saved = 0;
	    break;
	  }
	
          while( hop_tracking) {
	    READ_HOP_TIMER();
	    measure_local_heights( current_x, current_y, z_values);

	    READ_HOP_TIMER();
	    if( move_toward_extreme( &current_x, &current_y, z_values) && hop_tracking) {

	      hop_data->xd[hop_index] = current_x;
	      hop_data->yd[hop_index] = current_y;

	      READ_HOP_TIMER();	      
	      *(time_hop + hop_index) = hop_timer;

	      screen_x = (((int)hop_data->xd[hop_index])-ZERO)*pixels_per_bit + ZERO_PIXEL + HOP_DISPLAY_LEFT;
	      screen_y = -(((int)hop_data->yd[hop_index])-ZERO)*pixels_per_bit + ZERO_PIXEL + HOP_DISPLAY_TOP;
	      READ_HOP_TIMER();	      
	      if( (screen_x > HOP_DISPLAY_LEFT) && (screen_x < HOP_DISPLAY_RIGHT)
	     	     && (screen_y < HOP_DISPLAY_BOTTOM) && (screen_y > HOP_DISPLAY_TOP) ) {
	  	 SetPixel(hop_DC, screen_x, screen_y, RGB(0,0,0) );
	      }

	      READ_HOP_TIMER();	      
	      //CheckMsgQ(); // really want to do this every time?? 
	      //READ_HOP_TIMER();	      

              PeekMessage(&hop_Message, hDlg, NULL, NULL, PM_REMOVE);
	      READ_HOP_TIMER();	      
              if ( (LOWORD(hop_Message.wParam) == VK_ESCAPE) || (LOWORD(hop_Message.wParam) == HOP_STOP_TRACKING))
              {
		hop_tracking = 0;
              }

	      READ_HOP_TIMER();	      

	      if( ++hop_index > (HOP_MAX_ITERATIONS-HOP_PADDING)) {
	        MessageBox( hDlg,"Molecule hopped more times than allowed.","TOO MANY HOPS!",MB_OKCANCEL);
	        hop_tracking = 0;
	      }
		

#ifdef OLD_HOP_TIMING
	      dio_start_clock( hop_every);
	      dio_wait_clock();
#else
	      hop_delay_for( hop_every);
#endif
	    }
	    else {
	      hop_tracking = 0;
	    }
	  }
*/
	  __MACHINE(void __cdecl _enable()); // turn interrupts back on

	  ReleaseDC( hDlg,hop_DC);
	  enable_all( hDlg, 1);

	  clock_used = 0;
/*
	  // move tip so that it's really at current_x, current_y
	  // move tip "northeast" of current position
          NE_x = current_x + hop_radius * cos( PI/4);
	  NE_y = current_y + hop_radius * sin( PI/4);
	  dac_data[x_ch] = move_to_protect( x_ch, NE_x, current_x,
				   hop_inter_step_delay, CRASH_STOP,
				   HOP_LIMIT_PERCENT);
	  dac_data[y_ch] = move_to_protect( y_ch, NE_y, current_y,
				   hop_inter_step_delay, CRASH_STOP,
				   HOP_LIMIT_PERCENT);
*/
	  hop_data->saved = 0;
	  hop_data->size = hop_index;
	  break;
/*
	case HOP_STOP_TRACKING:
	  hop_tracking = 0;
	  break;
*/
	case HOP_SAVE:
	  glob_data = &hop_data;
          strcpy( current_file_hop,"data.hop");
	  if( !file_save_as( hDlg, hInst, FTYPE_HOP_DATA)) {
	    MessageBox( hDlg, "Unable to save file.", "Warning", MB_OKCANCEL);
	  }
	  else { 
	    hop_data->saved = 1;
	  }
	  break;

	case HOP_STEP_DELAY:
	  if( !out_smart) {
	    out_smart = 1;
	    GetDlgItemText( hDlg, LOWORD( wParam), string, 9);
	    hop_step_delay = atoi( string);
	    out_smart = 0;
	  }
	  break;

	case HOP_INTERSTEP_DELAY:
	  if( !out_smart) {
	    out_smart = 1;
	    GetDlgItemText( hDlg, LOWORD( wParam), string, 9);
	    hop_inter_step_delay = atoi( string);
	    out_smart = 0;
	  }
	  break;

	case HOP_EVERY:
	  if( !out_smart) {
	    out_smart = 1;
	    GetDlgItemText( hDlg, LOWORD( wParam), string, 9);
	    hop_every = atoi( string);
	    if( hop_every < 0) {
	      hop_every = 0;
	      SetDlgItemText( hDlg, LOWORD( wParam), "0");
	    }
	    out_smart = 0;
	  }
	  break;

	case HOP_AVG_DATA_PTS:
	  if( !out_smart) {
	    out_smart = 1;
	    GetDlgItemText( hDlg, LOWORD( wParam), string, 9);
	    hop_avg_data_pts = atoi( string);
	    if( hop_avg_data_pts < 1) {
		hop_avg_data_pts = 1;
		SetDlgItemText( hDlg, LOWORD( wParam), "1");
	    }
	    out_smart = 0;
	  }
	  break;
	
	case HOP_EXPONENT:
	  if( !out_smart) {
	    out_smart = 1;
	    GetDlgItemText( hDlg, LOWORD( wParam), string, 9);
	    hop_exponent = atof( string);
	    out_smart = 0;
	  }
	  break;

	case HOP_RADIUS:
	  if( !out_smart) {
	    out_smart = 1;
	    GetDlgItemText( hDlg, LOWORD( wParam), string, 9);
	    hop_radius = atoi( string);
	    if( hop_radius < 1) {
	     	hop_radius = 1;
		SetDlgItemText( hDlg, LOWORD( wParam), "1");
	    }
	    out_smart = 0;
	  }
	  break;

	case HOP_A:
	  if( !out_smart) {
	    out_smart = 1;
	    GetDlgItemText( hDlg, LOWORD( wParam), string, 9);
	    hop_plane_a = atof( string);
	    out_smart = 0;
	  }
	  break;

	case HOP_B:
	  if( !out_smart) {
	    out_smart = 1;
	    GetDlgItemText( hDlg, LOWORD( wParam), string, 9);
	    hop_plane_b = atof( string);
	    out_smart = 0;
	  }
	  break;

	case HOP_C:
	  if( !out_smart) {
	    out_smart = 1;
	    GetDlgItemText( hDlg, LOWORD( wParam), string, 9);
	    hop_plane_c = atof( string);
	    out_smart = 0;
	  }
	  break;

	case HOP_MIN:
	  feature_type = HOP_HOLE;
	  break;

	case HOP_MAX:
	  feature_type = HOP_BUMP;
	  break;

	case HOP_EXIT:
	  scan_fine_x = dac_data[x_ch];
          scan_fine_y = dac_data[y_ch];
	  EndDialog( hDlg, TRUE);
	  break;

      }
      break;
  }
  return( FALSE);
}

void measure_local_heights( unsigned int x, unsigned int y, float z_values[])
{
  // Measure z for 8 neighbors of {x,y} and put in z_values.
  // Assumes that tip is at upper right neighbor (northeast) of {x,y}.
  // Tip is returned to starting point after measurements.

  int i = 1;
  unsigned int dest_x, dest_y;
  float angle = PI/4; /* angle of location on the circle; measured cw from "top" */
/*
  unsigned int position_x = x + hop_radius * cos( angle);
  unsigned int position_y = y + hop_radius * sin(angle);
*/
  unsigned int position_x = hop_add_and_check(x, hop_radius * cos(angle));
  unsigned int position_y = hop_add_and_check(y, hop_radius * sin(angle));

  for( i = 0; i < HOP_NUM_NEIGHBORS; i++) {

    // measure z
#ifdef OLD_HOP_TIMING
    dio_start_clock( hop_step_delay);
    dio_in_ch( zi_ch);
    dio_wait_clock();
#else
    hop_delay_for( hop_step_delay);
    dio_in_ch( zi_ch);
#endif
    z_values[i] = dio_read_hop( hop_avg_data_pts);

    // adjust z for plane fit
    z_values[i] -= hop_plane_a*(float)position_x + hop_plane_b*(float)position_y + hop_plane_c; 

    READ_HOP_TIMER();	      
    // calculate new location, but be sure it's within [MIN,MAX]
    angle += PI/4;
/*
    dest_x = x + hop_radius * cos(angle);
    dest_y = y + hop_radius * sin(angle);
*/
    dest_x = hop_add_and_check(x, hop_radius * cos(angle));
    dest_y = hop_add_and_check(y, hop_radius * sin(angle));
    if(dest_x == MIN || dest_x == MAX || dest_y == MIN || dest_y == MAX) {
      MessageBox(GetFocus(), "Hop tracking has moved to boundary", "ABORTING", MB_OK);
      hop_tracking = 0;
      return;
    }

    READ_HOP_TIMER();	      
    // move to new location
    if( (i==1) || (i==2) || (i==5) || (i==6) ) {
      position_y = dac_data[y_ch] = move_to_protect_hop( y_ch, position_y,
					dest_y, hop_inter_step_delay,
					CRASH_STOP,HOP_LIMIT_PERCENT);
    }
    READ_HOP_TIMER();	      
    position_x = dac_data[x_ch] = move_to_protect_hop( x_ch, position_x,
					dest_x, hop_inter_step_delay,
					CRASH_STOP,HOP_LIMIT_PERCENT);
    if( (i!=1) && (i!=2) && (i!=5) && (i!=6)) {
      position_y = dac_data[y_ch] = move_to_protect_hop( y_ch, position_y,
					dest_y, hop_inter_step_delay,
					CRASH_STOP,HOP_LIMIT_PERCENT);
    }
    READ_HOP_TIMER();	      
	  
    if( (position_x != dest_x) || (position_y != dest_y) ) {
      hop_tracking = 0;
      MessageBox( GetFocus(),"Unable to move tip to destination\n while measuring z","ABORTING",MB_OKCANCEL);
      break;
    }
  }
}

int move_toward_extreme( unsigned int *x, unsigned int *y, float z_values[])
{
  // z_values is an array of measured z's around the point *x,*y
  // The tip climbs uphill along the steepest slope. {*x,*y} are set to final posittion,
  // but the final tip position is actually NE of {*x,*y}
  // returns 0 if something goes wrong
  
  float  x_slope, y_slope;
  unsigned int    dest_x, dest_y;
  unsigned int    tip_x = dac_data[x_ch];
  unsigned int    tip_y = dac_data[y_ch];

  x_slope = ( z_values[7] + (1/2)*(z_values[0] + z_values[6]) -
               z_values[3] - (1/2)*(z_values[4] + z_values[2]) ) / (2*2*hop_radius);
  y_slope = ( z_values[1] + (1/2)*(z_values[0] + z_values[2]) -
               z_values[5] - (1/2)*(z_values[4] + z_values[6]) ) / (2*2*hop_radius);
  
  // set *x and *y to be the new locations
  if( (x_slope < 0 && feature_type == HOP_BUMP) || (x_slope > 0 && feature_type == HOP_HOLE)) {
    *x = hop_add_and_check(*x, -hop_radius * (1 - exp( (-1)*fabs( x_slope/hop_exponent))));
  }
  else {
    *x = hop_add_and_check( *x, hop_radius * (1 - exp( (-1)*fabs( x_slope/hop_exponent))));
  }

  READ_HOP_TIMER();	      
  if( (y_slope < 0 && feature_type == HOP_BUMP) || (y_slope > 0 && feature_type == HOP_HOLE)) {
    *y = hop_add_and_check( *y, -hop_radius * (1 - exp( (-1)*fabs(x_slope/hop_exponent))));
  }
  else {
    *y = hop_add_and_check( *y, hop_radius * (1 - exp( (-1)*fabs(x_slope/hop_exponent))));
  }

  if(*x == MIN || *x == MAX || *y == MIN || *y == MAX) {
    MessageBox(GetFocus(), "Hop tracking has moved to boundary", "ABORTING", MB_OK);
    return(0);
  }

  // calculate "Northeast" of new location and move the tip there
  dest_x = hop_add_and_check( *x, hop_radius * cos( PI/4));
  dest_y = hop_add_and_check( *y, hop_radius * sin( PI/4));
  if(dest_x == MIN || dest_x == MAX || dest_y == MIN || dest_y == MAX) {
    MessageBox(GetFocus(), "Hop tracking has moved to boundary", "ABORTING", MB_OK);
    return(0);
  }

  READ_HOP_TIMER();	      
  dac_data[x_ch] = move_to_protect_hop( x_ch, tip_x, dest_x,
				   hop_inter_step_delay, CRASH_STOP,
				   HOP_LIMIT_PERCENT);
  dac_data[y_ch] = move_to_protect_hop( y_ch, tip_y, dest_y,
				   hop_inter_step_delay, CRASH_STOP,
				   HOP_LIMIT_PERCENT);
  if( (dac_data[x_ch] != dest_x) || (dac_data[y_ch] != dest_y) ) {
    hop_tracking = 0;
    MessageBox( GetFocus(),"Unable to move tip to new destination","ABORTING",MB_OKCANCEL);
    return(0);
  }
  
  READ_HOP_TIMER();	      

  return(1);

}

static void repaint_params( HWND hDlg)
{
  out_smart = 1;
  sprintf( string, "%d", hop_step_delay);
  SetDlgItemText( hDlg, HOP_STEP_DELAY, string);
  sprintf( string, "%d", hop_inter_step_delay);
  SetDlgItemText( hDlg, HOP_INTERSTEP_DELAY, string);
  sprintf( string, "%d", hop_radius);
  SetDlgItemText( hDlg, HOP_RADIUS, string);
  sprintf( string, "%.2f", hop_exponent);
  SetDlgItemText( hDlg, HOP_EXPONENT, string);
  sprintf( string, "%d", hop_avg_data_pts);
  SetDlgItemText( hDlg, HOP_AVG_DATA_PTS, string);
  sprintf( string, "%d", hop_every);
  SetDlgItemText( hDlg, HOP_EVERY, string);
  sprintf( string, "%.2f", hop_plane_a);
  SetDlgItemText( hDlg, HOP_A, string);
  sprintf( string, "%.2f", hop_plane_b);
  SetDlgItemText( hDlg, HOP_B, string);
  sprintf( string, "%.2f", hop_plane_c);
  SetDlgItemText( hDlg, HOP_C, string);

  if( feature_type == HOP_BUMP) {
    CheckDlgButton( hDlg, HOP_MIN, 0);
    CheckDlgButton( hDlg, HOP_MAX, 1);
  }
  else {
    CheckDlgButton( hDlg, HOP_MAX, 0);
    CheckDlgButton( hDlg, HOP_MIN, 1);
  }
  out_smart = 0;
}

static void enable_all( HWND hDlg, int status)
{
  EnableWindow(GetDlgItem(hDlg, HOP_STEP_DELAY), status);
  EnableWindow(GetDlgItem(hDlg, HOP_INTERSTEP_DELAY), status);
  EnableWindow(GetDlgItem(hDlg, HOP_RADIUS), status);
  EnableWindow(GetDlgItem(hDlg, HOP_EXPONENT), status);
  EnableWindow(GetDlgItem(hDlg, HOP_AVG_DATA_PTS), status);
  EnableWindow(GetDlgItem(hDlg, HOP_MIN), status);
  EnableWindow(GetDlgItem(hDlg, HOP_MAX), status);
  EnableWindow(GetDlgItem(hDlg, HOP_SAVE), status);
  EnableWindow(GetDlgItem(hDlg, HOP_EXIT), status);
  EnableWindow(GetDlgItem(hDlg, HOP_EVERY), status);
  EnableWindow(GetDlgItem(hDlg, HOP_A), status);
  EnableWindow(GetDlgItem(hDlg, HOP_B), status);
  EnableWindow(GetDlgItem(hDlg, HOP_C), status);
  EnableWindow(GetDlgItem(hDlg, HOP_START_TRACKING), status);

  if( status) {
    SetDlgItemText( hDlg, HOP_ESC_MESSAGE, HOP_EMPTY_STR);
  }
  else {
    SetDlgItemText( hDlg, HOP_ESC_MESSAGE, HOP_PRESS_ESC_STR);
  }

/*
  // STOP TRACKING button has been removed, update static message instead
  EnableWindow(GetDlgItem(hDlg, HOP_STOP_TRACKING), !status);
*/

}

static unsigned int hop_add_and_check(unsigned int current, float modifier)
{
  // checks that current+modifier is within [MIN,MAX]
  unsigned int result = 0;

  if((int)floor(-modifier) > (int)current) {
    result = (unsigned int)MIN;
  }
  else if( current + modifier > MAX) {
    result = (unsigned int)MAX;
  }
  else result = (unsigned int)(current + modifier);

  return(result);
}

void calculate_new_coordinates(unsigned int *x,unsigned int *y,int x_slope,int y_slope)
{
	float x_sl;
	float y_sl;
	
	x_sl = (x_slope-0x800000)/(2*hop_avg_data_pts*hop_radius);
	y_sl = (y_slope-0x800000)/(2*hop_avg_data_pts*hop_radius);

	if( (x_sl < 0 && feature_type == HOP_BUMP) || (x_sl > 0 && feature_type == HOP_HOLE)) {
    *x = hop_add_and_check(*x, -hop_radius * (1 - exp( (-1)*fabs( x_sl/hop_exponent))));
  }
  else {
    *x = hop_add_and_check( *x, hop_radius * (1 - exp( (-1)*fabs( x_sl/hop_exponent))));
  }
    
  if( (y_sl < 0 && feature_type == HOP_BUMP) || (y_sl > 0 && feature_type == HOP_HOLE)) {
    *y = hop_add_and_check( *y, -hop_radius * (1 - exp( (-1)*fabs(y_sl/hop_exponent))));
  }
  else {
    *y = hop_add_and_check( *y, hop_radius * (1 - exp( (-1)*fabs(y_sl/hop_exponent))));
  }

  if(*x == MIN || *x == MAX || *y == MIN || *y == MAX) {
    MessageBox(GetFocus(), "Hop tracking has moved to boundary", "ABORTING", MB_OK);
    return(0);
  }

}