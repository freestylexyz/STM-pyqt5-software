#include <time.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include <i86.h>
#include "file.h"
#include "data.h"
#include "stm.h"
#include "dio.h"
#include "scan.h"
#include "clock.h"
#include "track.h"
#include "serial.h"

BOOL FAR PASCAL TrackDlg(HWND,unsigned,WPARAM,LPARAM);

HWND TrackWnd=NULL;
BOOL TrackOn=FALSE;
extern HWND ScnDlg;
extern unsigned int dac_data[];
extern unsigned int out1;
extern datadef *scan_defaults_data[];
extern int scan_current_default;

#ifdef OLD
extern int inter_step_delay,step_delay;
extern float z_limit_percent;
extern unsigned int scan_num;
#endif

extern unsigned int scan_fine_x,scan_fine_y;

extern char     string[];
int track_offset_x=0,track_offset_y=0;
int track_limit_x_min,track_limit_x_max;
int track_limit_y_min,track_limit_y_max;
int track_max_bits=200;
int track_sample_every=100;
int track_step_delay=1000;
int tracking=0;
int tracking_mode=TRACK_MAX;
int track_auto_auto=0;
int track_avg_data_pts=200;
float track_plane_a = 0.0;
float track_plane_b = 0.0;
static int high_limit,low_limit;
static int out_smart=0;
static int track_debug_count = 0;

int track_check = 0;	// is 0 if tracking was never called
int current_x, current_y;

#ifdef OLD
float read_compare(float);
#endif
static void recalc_track_limits();
static void repaint_track_max_bits();
static void repaint_track_limits();
static void enable_all(HWND, int);
static void repaint_all(HWND);
static void repaint_planes(HWND);
static void repaint_current_pos(HWND);

BOOL FAR PASCAL TrackDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{
    int i,j,k,delt,ddelt,id;
    double d,deltd,ddeltd,maxid,minid;
    float this_z,best_z;
    int mini,maxi;
    int track_update, track_max_update;
    

  switch(Message) {
    case WM_INITDIALOG:
/*
        current_x=scan_fine_x;
        current_y=scan_fine_y;
*/
        SetScrollRange(GetDlgItem(hDlg,TRACK_MAX_BITS_SCROLL),SB_CTL, 0,
                        MAX/2,FALSE);
        CheckDlgButton(hDlg,TRACK_AUTO_AUTO,track_auto_auto);
        repaint_all(hDlg);
      break;
    case WM_HSCROLL:
    case WM_VSCROLL:
        if (!out_smart)
        {
            out_smart=1;
            id = getscrollid();
            switch (id)
            {
                case TRACK_MAX_BITS_SCROLL:
                    i=track_max_bits;
                    mini=0;
                    maxi=MAX/2;
                    delt=TRACK_MAX_BITS_DELT;
                    ddelt=TRACK_MAX_BITS_DDELT;
                    break;
#ifdef OLD
                case VSCALE_MAX_SC:
                    d=(double) image_higherv[current_image];
                    minid=image_lowerv[current_image];
                    maxid=max(gendata[current_image]->max_z,IN_MAX);
                    deltd=(maxid-minid)/100;
                    ddeltd=(maxid-minid)/10;
                    break;
#endif
    
            }
            switch (getscrollcode())
            {       
                case SB_LINELEFT:
                    i -= delt;
                    d -= deltd;
                    break;
                case SB_LINERIGHT:
                    i += delt;
                    d += deltd;
                    break;
                case SB_PAGELEFT:
                    i -= ddelt;
                    d -= ddeltd;
                    break;
                case SB_PAGERIGHT:
                    i += ddelt;
                    d += ddeltd;
                    break;
                case SB_THUMBPOSITION:
                    i = getscrollpos();
                    d = ITOD(getscrollpos(),minid,maxid);
                    break;
                case SB_THUMBTRACK:
                    i = getscrollpos();
                    d = ITOD(getscrollpos(),minid,maxid);
                    break;
            }
            i = min(max(i, mini), maxi);
//            d=floor(d+0.5);
            d = min(max(d, minid), maxid);
            switch (id)
            {
                case TRACK_MAX_BITS_SCROLL:
                    track_max_bits=i;
                    repaint_track_max_bits(hDlg);
                    break;
#ifdef OLD
                case VSCALE_MAX_SC:
                    image_higherv[current_image]=d;
                    repaint_all(hDlg);
                    SendMessage(ImgDlg, WM_COMMAND, IM_NEW_VSCALE, 0);
                    break;
#endif
            }
            out_smart=0;
        }
        break;
    case WM_COMMAND:
      switch (LOWORD(wParam)) {
        case TRACK_MAX:
        case TRACK_MIN:
            tracking_mode=LOWORD(wParam);
            break;
        case TRACK_OFFSET_X:
            if (!out_smart)
            {
                out_smart=1;
                GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
                track_offset_x=atoi(string);
                if (track_offset_x<-MAX) 
                {
                    track_offset_x=-MAX;
                    sprintf(string,"%d",track_offset_x);
                    SetDlgItemText(hDlg,LOWORD(wParam),string);
                }
                if (track_offset_x>MAX) 
                {
                    track_offset_x=MAX;
                    sprintf(string,"%d",track_offset_x);
                    SetDlgItemText(hDlg,LOWORD(wParam),string);
                }
                out_smart=0;
            }
            break;
        case TRACK_OFFSET_Y:
            if (!out_smart)
            {
                out_smart=1;
                GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
                track_offset_y=atoi(string);
                if (track_offset_y<-MAX) 
                {
                    track_offset_y=-MAX;
                    sprintf(string,"%d",track_offset_y);
                    SetDlgItemText(hDlg,LOWORD(wParam),string);
                }
                if (track_offset_y>MAX) 
                {
                    track_offset_y=MAX;
                    sprintf(string,"%d",track_offset_y);
                    SetDlgItemText(hDlg,LOWORD(wParam),string);
                }
                out_smart=0;
            }
            break;
        case TRACK_SAMPLE_EVERY:
            if (!out_smart)
            {
                out_smart=1;
                GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
                track_sample_every=atoi(string);
                if (track_sample_every<0) 
                {
                    track_sample_every=0;
                    SetDlgItemText(hDlg,LOWORD(wParam),"0");
                }
                out_smart=0;
            }
            break;
        case TRACK_STEP_DELAY:
            if (!out_smart)
            {
                out_smart=1;
                GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
                track_step_delay=atoi(string);
                if (track_sample_every<0) 
                {
                    track_step_delay=0;
                    SetDlgItemText(hDlg,LOWORD(wParam),"0");
                }
                out_smart=0;
            }
            break;
        case TRACK_AVG_DATA_PTS:
            if (!out_smart)
            {
                out_smart=1;
                GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
                track_avg_data_pts=atoi(string);
                if (track_avg_data_pts<1) 
                {
                    track_avg_data_pts=1;
                    SetDlgItemText(hDlg,LOWORD(wParam),"1");
                }
                out_smart=0;
            }
            break;
                
        case TRACK_MAX_BITS:
            if (!out_smart)
            {
                out_smart=1;
                GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
                track_max_bits=atoi(string);
                if (track_max_bits<0) 
                {
                    track_max_bits=0;
                    SetDlgItemText(hDlg,LOWORD(wParam),"0");
                }
                if (track_max_bits>MAX/2)
                {
                    track_max_bits=MAX/2;
                    sprintf(string,"%d",track_max_bits);
                    SetDlgItemText(hDlg,LOWORD(wParam),string);
                }
                out_smart=0;
                SendMessage(ScnDlg, WM_COMMAND, SCAN_REPAINT_TRACK_LIMITS, 0);
                recalc_track_limits();
                SendMessage(ScnDlg, WM_COMMAND, SCAN_REPAINT_TRACK_LIMITS, 0);
                repaint_track_limits(hDlg);
                
            }
            break;
	case TRACK_PLANE_A:
	    if (!out_smart) {
	        out_smart=1;
	        GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
	        track_plane_a=atof(string);
	        out_smart=0;
	    }
	    break;
            
	case TRACK_PLANE_B:
	    if (!out_smart) {
	        out_smart=1;
	        GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
	        track_plane_b=atof(string);
	        out_smart=0;
	    }
	    break;
            
        case TRACK_AUTO_AUTO:
            track_auto_auto=IsDlgButtonChecked(hDlg,TRACK_AUTO_AUTO);
            break;
        case TRACK_START_AGAIN:
            i=(int)scan_fine_x-track_offset_x;
            if (i<0) i=0;
            if (i>MAX) i=MAX;
            scan_fine_x=i;
            i=(int)scan_fine_y-track_offset_y;
            if (i<0) i=0;
            if (i>MAX) i=MAX;
            scan_fine_y=i;
        case TRACK_START:
            track_update=0;  // update counter
			track_debug_count = 0;
            //track_max_update=500000/(track_sample_every+
            //    11*track_avg_data_pts*22); 

            //if (!track_max_update) track_max_update=1; //determines how often to update 
													   //the scan dialog
            
            SendMessage(ScnDlg, WM_COMMAND, SEND_X_Y_OFFSETS, 0);
            SendMessage(ScnDlg, WM_COMMAND, SCAN_REPAINT_FINE_POS, 0);
/*
            best_x=current_x=dac_data[x_ch];
            best_y=current_y=dac_data[y_ch];
*/
            high_limit=IN_MAX-(unsigned int) ((float)IN_MAX*SD->z_limit_percent/100);
            low_limit=(unsigned int)((float)IN_MAX*SD->z_limit_percent/100);
            tracking=1;
            repaint_all(hDlg);
            SendMessage(ScnDlg, WM_COMMAND, SCAN_REPAINT_CURSOR, 0);
            strcpy(string,"Tracking");
            SendMessage(ScnDlg, WM_COMMAND, SCAN_REPAINT_STATUS, 0);
            SendMessage(ScnDlg, WM_COMMAND, SCAN_TRACKING_START, 0);
            enable_all(hDlg,0);

			track_setup(track_avg_data_pts, track_sample_every, track_step_delay,
						track_plane_a, track_plane_b);	//initialization of tracking parameters

			
			current_x=dac_data[x_ch];
			current_y=dac_data[y_ch];
			
			if(current_y>=(track_limit_y_min+10) && 
                current_y<=(track_limit_y_max-10) && 
                current_x>=(track_limit_x_min+10) &&  
                current_x<=(track_limit_x_max-10)) 
					{
					track_start(current_x, current_y, tracking_mode);
					track_check++;
					}
				else 
				{
					tracking=0;
				}

            while(tracking)
            {
				track_update++;	
				current_x=dac_data[x_ch];
				current_y=dac_data[y_ch];

				if(current_y>=(track_limit_y_min+10) && 
					current_y<=(track_limit_y_max-10) && 
					current_x>=(track_limit_x_min+10) &&  
					current_x<=(track_limit_x_max-10))
						{
						track_again();
						track_debug_count++;
						}
					else
					{
						tracking=0;
					}
#ifdef OLD
                 track(track_auto_auto,high_limit,low_limit,track_avg_data_pts,
                    tracking_mode==TRACK_MAX,track_step_delay,track_step_delay,
                    track_limit_x_min,track_limit_x_max,track_limit_y_min,
                    track_limit_y_max,track_plane_a,track_plane_b);

                dio_in_ch(zi_ch);
                best_z=dio_read(track_avg_data_pts);
                if (track_auto_auto)
                {
                    if (best_z>high_limit || best_z<low_limit)
                    {
                        auto_z_below(IN_ZERO);
                        best_z=dio_read(track_avg_data_pts);
                    }
                }
                current_y++;
                best_z=read_compare(best_z);
                current_x++;
                best_z=read_compare(best_z);
                current_y--;
                best_z=read_compare(best_z);
                current_y--;
                best_z=read_compare(best_z);
                current_x--;
                best_z=read_compare(best_z);
                current_x--;
                best_z=read_compare(best_z);
                current_y++;
                best_z=read_compare(best_z);
                current_y++; 
                best_z=read_compare(best_z);
                current_x++;
                current_y--;
                best_z=read_compare(best_z);
                current_x=best_x;
                current_y=best_y;
                dio_out(x_ch,current_x);
                dio_start_clock(SD->inter_step_delay);
                dac_data[x_ch]=(unsigned int)current_x;
                dio_wait_clock();
                dio_out(y_ch,current_y);
                dio_start_clock(step_delay);
                dac_data[y_ch]=(unsigned int)current_y;
                dio_in_ch(zi_ch);
                dio_wait_clock();

                
                //dio_start_clock(track_sample_every);
                
                if (track_update>=track_max_update)
                {
                
                    SendMessage(ScnDlg, WM_COMMAND, SCAN_REPAINT_FINE_POS, 0);
                    repaint_current_pos(hDlg);
                    CheckMsgQ();
                    
                    track_update=0;
                }
                //dio_wait_clock();
#endif
			SendMessage(ScnDlg, WM_COMMAND, SCAN_REPAINT_FINE_POS, 0);
            repaint_current_pos(hDlg);
            CheckMsgQ();

            }
			if (track_check)
				track_terminate();
            enable_all(hDlg,1);
            SendMessage(ScnDlg, WM_COMMAND, SCAN_TRACKING_STOP, 0);
            
            strcpy(string,"Idle");
            SendMessage(ScnDlg, WM_COMMAND, SCAN_REPAINT_STATUS, 0);
            i=(int)dac_data[x_ch]+track_offset_x;
            if (i<0) i=0;
            if (i>MAX) i=MAX;
            scan_fine_x=i;
            i=(int)dac_data[y_ch]+track_offset_y;
            if (i<0) i=0;
            if (i>MAX) i=MAX;
            scan_fine_y=i;
            SendMessage(ScnDlg, WM_COMMAND, SEND_X_Y_OFFSETS, 0);
            SendMessage(ScnDlg, WM_COMMAND, SCAN_REPAINT_FINE_POS, 0);
            
/*
			sprintf(string,"tracked %d times",track_debug_count);
			mprintf(string);
*/              
            break;        
        case TRACK_STOP:
            tracking=0;

/* test 10/25/99
	    {
		int a,b;
		unsigned int ua, ub;
		float c = 20464.400391;
		float result;

		a=dac_data[x_ch];
		ua=dac_data[x_ch];
		b=dac_data[y_ch];
		ub=dac_data[y_ch];
		sprintf( string, "int: ( %d %d ) \n u int ( %d %d )", a, b, ua, ub);
		MessageBox( hDlg, string, "Debug 1", MB_OK);

		result = c;
		result -= (float)a*track_plane_a + (float)b*track_plane_b;
		sprintf( string, "result = %f", result);
		MessageBox( hDlg, string, "Debug 2", MB_OK);

		result = c;
		result -= track_plane_a*(float)a + track_plane_b*(float)b;
		sprintf( string, "result = %f", result);
		MessageBox( hDlg, string, "Debug 3", MB_OK);

		result = c;
		result -= (float)ua*track_plane_a + (float)ub*track_plane_b;
		sprintf( string, "result = %f", result);
		MessageBox( hDlg, string, "Debug 4", MB_OK);

		result = c;
		result -= track_plane_a*(float)ua + track_plane_b*(float)ub;
		sprintf( string, "result = %f", result);
		MessageBox( hDlg, string, "Debug 5", MB_OK);
	    }
*/
            break;
        case TRACK_EXIT:
          DestroyWindow(hDlg);
          TrackWnd = NULL;
          TrackOn = FALSE;
          break;
      }
    break;
  }
  return(FALSE);
}

static void enable_all(HWND hDlg,int status)
{
    EnableWindow(GetDlgItem(hDlg, TRACK_START), status);
    EnableWindow(GetDlgItem(hDlg, TRACK_START_AGAIN), status);
    EnableWindow(GetDlgItem(hDlg, TRACK_EXIT), status);
    EnableWindow(GetDlgItem(hDlg, TRACK_MAX_BITS), status);
    EnableWindow(GetDlgItem(hDlg, TRACK_SAMPLE_EVERY), status);
    EnableWindow(GetDlgItem(hDlg, TRACK_STEP_DELAY), status);
    EnableWindow(GetDlgItem(hDlg, TRACK_PLANE_A), status);
    EnableWindow(GetDlgItem(hDlg, TRACK_PLANE_B), status);
/*
    EnableWindow(GetDlgItem(hDlg, TRACK_), status);
    EnableWindow(GetDlgItem(hDlg, TRACK_), status);
    EnableWindow(GetDlgItem(hDlg, TRACK_), status);
*/
}

static void repaint_all(HWND hDlg)
{
    out_smart=1;
    CheckDlgButton(hDlg,tracking_mode,1);
    sprintf(string,"%d",track_offset_x);
    SetDlgItemText(hDlg,TRACK_OFFSET_X,string);
    sprintf(string,"%d",track_offset_y);
    SetDlgItemText(hDlg,TRACK_OFFSET_Y,string);
    sprintf(string,"%d",track_sample_every);
    SetDlgItemText(hDlg,TRACK_SAMPLE_EVERY,string);
    sprintf(string,"%d",track_step_delay);
    SetDlgItemText(hDlg,TRACK_STEP_DELAY,string);
    sprintf(string,"%d",track_avg_data_pts);
    SetDlgItemText(hDlg,TRACK_AVG_DATA_PTS,string);
    SetScrollPos(GetDlgItem(hDlg,TRACK_MAX_BITS_SCROLL),SB_CTL,track_max_bits,TRUE);
    repaint_planes( hDlg);
    out_smart=0;
    
    repaint_current_pos(hDlg);
    repaint_track_max_bits(hDlg);
}    

    


#ifdef OLD

static void repaint_current(HWND hDlg)
{
    if (current_logpal_el!=NULL)
    {
        sprintf(string,"%.2lf",current_logpal_el->gamma);
        SetDlgItemText(hDlg,LOGPAL_GAMMA,string);
        sprintf(string,"%d",current_logpal_el->index);
        SetDlgItemText(hDlg,LOGPAL_CURRENT_INDEX,string);
        sprintf(string,"%d",(current_logpal_el->next->index+current_logpal_el->index)/2);
        SetDlgItemText(hDlg,LOGPAL_ADD_INDEX,string);
         sprintf(string,"%d",current_logpal_el->color.r);
        SetDlgItemText(hDlg,LOGPAL_R,string);
        sprintf(string,"%d",current_logpal_el->color.g);
        SetDlgItemText(hDlg,LOGPAL_G,string);
        sprintf(string,"%d",current_logpal_el->color.b);
        SetDlgItemText(hDlg,LOGPAL_B,string);
        CheckDlgButton(hDlg,LOGPAL_EQUALIZED,current_logpal_el->equalized);
        
    }

}
#endif

#ifdef OLD
float read_compare(float best)
{
    float z;
    if (current_y>=track_limit_y_min && 
                current_y<=track_limit_y_max && 
                current_x>=track_limit_x_min && 
                current_x<=track_limit_x_max)
    {    
        dio_out(x_ch,current_x);
        dio_start_clock(SD->inter_step_delay);
        dac_data[x_ch]=(unsigned int)current_x;
        dio_wait_clock();
        dio_out(y_ch,current_y);
        dio_start_clock(step_delay);
        dac_data[y_ch]=(unsigned int)current_y;
        dio_in_ch(zi_ch);
        dio_wait_clock();
        
        z=dio_read(track_avg_data_pts);
        switch(tracking_mode)
        {
            case TRACK_MAX:
                if (z>best) 
                {
                    best=z;
                    best_x=current_x;
                    best_y=current_y;
                }
                break;
            case TRACK_MIN:
                if (z<best) 
                {
                    best=z;
                    best_x=current_x;
                    best_y=current_y;
                }
                break;
        }
    }
    return best;
}
#endif

static void repaint_planes( HWND hDlg)
{
    sprintf(string,"%.4f",track_plane_a);
    SetDlgItemText(hDlg,TRACK_PLANE_A,string);
    sprintf(string,"%.4f",track_plane_b);
    SetDlgItemText(hDlg,TRACK_PLANE_B,string);
}

static void repaint_current_pos(HWND hDlg)
{
    sprintf(string,"%d",dac_data[x_ch]);
    SetDlgItemText(hDlg,TRACK_CUR_POS_X,string);
    sprintf(string,"%d",dac_data[y_ch]);
    SetDlgItemText(hDlg,TRACK_CUR_POS_Y,string);
}    

static void repaint_track_max_bits(HWND hDlg)
{
    out_smart=1;    
    SetScrollPos(GetDlgItem(hDlg,TRACK_MAX_BITS_SCROLL),SB_CTL,track_max_bits,TRUE);
    SendMessage(ScnDlg, WM_COMMAND, SCAN_REPAINT_TRACK_LIMITS, 0);
    recalc_track_limits();
    SendMessage(ScnDlg, WM_COMMAND, SCAN_REPAINT_TRACK_LIMITS, 0);
    sprintf(string,"%d",track_max_bits);
    SetDlgItemText(hDlg,TRACK_MAX_BITS,string);
    repaint_track_limits(hDlg);
    out_smart=0;
}        
static void repaint_track_limits(HWND hDlg)
{
    sprintf(string,"%d",track_limit_x_min);
    SetDlgItemText(hDlg,TRACK_CUR_LIMIT_X_MIN,string);
    sprintf(string,"%d",track_limit_x_max);
    SetDlgItemText(hDlg,TRACK_CUR_LIMIT_X_MAX,string);
    sprintf(string,"%d",track_limit_y_min);
    SetDlgItemText(hDlg,TRACK_CUR_LIMIT_Y_MIN,string);
    sprintf(string,"%d",track_limit_y_max);
    SetDlgItemText(hDlg,TRACK_CUR_LIMIT_Y_MAX,string);
}    
static void recalc_track_limits()
{
    track_limit_x_max=dac_data[x_ch]+track_max_bits;
    if (track_limit_x_max>MAX) track_limit_x_max=MAX;
    track_limit_x_min=((int)dac_data[x_ch])-track_max_bits;
    if (track_limit_x_min<0) track_limit_x_min=0;
    track_limit_y_max=dac_data[y_ch]+track_max_bits;
    if (track_limit_y_max>MAX) track_limit_y_max=MAX;
    track_limit_y_min=((int)dac_data[y_ch])-track_max_bits;
    if (track_limit_y_min<0) track_limit_y_min=0;
}



//*********************************************************
// OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD
//*********************************************************

#ifdef OLD

void track(int track_auto_auto,int high_limit,int low_limit,
    int track_avg_data_pts,int tracking_mode,int step_delay,
    int inter_step_delay,int track_limit_x_min,int track_limit_x_max,
    int track_limit_y_min,int track_limit_y_max,
    float plane_a, float plane_b)
{
    float best_z;
    int current_x,current_y;
    int best_x,best_y;

#define read_compare() \
{\
    float z;\
    if (current_y>=track_limit_y_min && \
                current_y<=track_limit_y_max && \
                current_x>=track_limit_x_min &&  \
                current_x<=track_limit_x_max) \
    {    \
        dio_out(x_ch,current_x); \
        dio_start_clock(inter_step_delay); \
        dac_data[x_ch]=(unsigned int)current_x; \
        dio_wait_clock(); \
        dio_out(y_ch,current_y); \
        dio_start_clock(step_delay); \
        dac_data[y_ch]=(unsigned int)current_y; \
        dio_in_ch(zi_ch); \
        dio_wait_clock(); \
        z=dio_read(track_avg_data_pts); \
	z-= plane_a*(float)current_x + plane_b*(float)current_y; \
        if(tracking_mode) \
        { \
                if (z>best_z)  \
                { \
                    best_z=z; \
                    best_x=current_x; \
                    best_y=current_y; \
                } \
        } \
        else \
        { \
                if (z<best_z)  \
                { \
                    best_z=z; \
                    best_x=current_x; \
                    best_y=current_y; \
                } \
        } \
    } \
}
    
    best_x=current_x=dac_data[x_ch];
    best_y=current_y=dac_data[y_ch];
    
    dio_in_ch(zi_ch);
    best_z=dio_read(track_avg_data_pts);
    if (track_auto_auto)
    {
        if (best_z>high_limit || best_z<low_limit)
        {
            auto_z_below(IN_ZERO);
            best_z=dio_read(track_avg_data_pts);
        }
    }
    current_y++;
    read_compare();
    current_x++;
    read_compare();
    current_y--;
    read_compare();
    current_y--;
    read_compare();
    current_x--;
    read_compare();
    current_x--;
    read_compare();
    current_y++;
    read_compare();
    current_y++; 
    read_compare();
    current_x++;
    current_y--;
    read_compare();
    current_x=best_x;
    current_y=best_y;
    dio_out(x_ch,current_x);
    dio_start_clock(inter_step_delay);
    dac_data[x_ch]=(unsigned int)current_x;
    dio_wait_clock();
    dio_out(y_ch,current_y);
    dio_start_clock(step_delay);
    dac_data[y_ch]=(unsigned int)current_y;
    dio_in_ch(zi_ch);
    dio_wait_clock();
    
#ifdef OLD
    dio_start_clock(track_sample_every);
    
    SendMessage(ScnDlg, WM_COMMAND, SCAN_REPAINT_FINE_POS, 0);
    repaint_current_pos(hDlg);
    
    CheckMsgQ();
    dio_wait_clock();
#endif
}

#endif


