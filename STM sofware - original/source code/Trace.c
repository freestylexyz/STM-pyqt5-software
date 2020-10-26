#include <time.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include <i86.h>
#include "common.h"
#include "data.h"
#include "dio.h"
#include "sel.h"
#include "stm.h"
#include "scan.h"
#include "image.h"
#include "trace.h"

BOOL FAR PASCAL TraceDlg(HWND,unsigned,WPARAM,LPARAM);

HWND TraceWnd=NULL;
BOOL TraceOn=FALSE;
extern HWND ImgDlg;
extern datadef *gendata[];
extern int constrain;
extern int current_image;
extern SEL_REGION sel_r1,sel_r2;

extern char *initial_dir_trc;

extern char     string[];
int trace_reject=0;
int trace_reject_that=0;
float trace_bin_width=1;
float trace_reject_time=1;
float trace_reject_that_time=1;
int trace_num=1;
int trace_out_num=1;
double trace_state_total=0;
double trace_total=0;
unsigned int trace_from=0;
unsigned int trace_to=IN_MAX;
double trace_avg_v=0;
double trace_avg_out_v=0;
int trace_init_state=TRACE_START_UNDEFINED;
int trace_state=TRACE_HIGH;
int trace_num_sum=0;
int trace_out_num_sum=0;
double trace_avg_ms_sum=0;
double trace_avg_ms2_sum=0;
double trace_avg_v_sum=IN_ZERO;
double trace_avg_v2_sum=IN_ZERO;

static int first_time=1;
static int out_smart=0;
static int trace_in_range(datadef *,int);
static TRACE_EL *this_width_list=NULL;
static TRACE_EL *that_width_list=NULL;
static TRACE_BIN this_bin;
static TRACE_BIN that_bin;
static TRACE_BIN this_bin_sum;
static TRACE_BIN that_bin_sum;
static int last_analyzed;

static void new_trace_el(TRACE_EL **,float );
static void remove_last_trace_el(TRACE_EL **); 
static void destroy_trace_list(TRACE_EL **);
static void trace_add_bin(TRACE_BIN *,TRACE_BIN);
static void trace_display_bin(TRACE_BIN);
static void trace_clear_bin(TRACE_BIN *);

static void repaint_all(HWND);
static void trace_save_binned(TRACE_BIN, char *);
static void trace_save_bin();
static void trace_time_bin(datadef *);

BOOL FAR PASCAL TraceDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{
    int i,delt,ddelt,id;
    double d,deltd,ddeltd,maxid,minid;
    int mini,maxi;
    double this_in_total=0;
    double this_out_total=0;
    double this_avg_v=0;      
    double this_avg_out_v=0;
    double this_in_total_old=0;
    double this_out_total_old=0;
    double this_avg_v_old=0;      
    double this_avg_out_v_old=0;
    double delta_t;
    double temp_avg_v;
    int this_state,prev_state;
    int this_state_old,prev_state_old;
    
    int start,end;
    int pos;
    datadef *this_data;
    

  switch(Message) {
    case WM_INITDIALOG:
#ifdef OLD
        current_x=scan_fine_x;
        current_y=scan_fine_y;
        SetScrollRange(GetDlgItem(hDlg,TRACK_MAX_BITS_SCROLL),SB_CTL, 0,
                        MAX/2,FALSE);
        CheckDlgButton(hDlg,TRACK_AUTO_AUTO,track_auto_auto);
#endif
        if (first_time)
        {
            this_bin.array=NULL;
            this_bin.size=0;
            that_bin.array=NULL;
            that_bin.size=0;
            this_bin_sum.array=NULL;
            this_bin_sum.size=0;
            that_bin_sum.array=NULL;
            that_bin_sum.size=0;
            first_time=0;
        }
        SendMessage(ImgDlg, WM_COMMAND, IMAGE_REPAINT_TRACE_LIMITS, 0);
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
#ifdef OLD
                case TRACK_MAX_BITS_SCROLL:
                    i=track_max_bits;
                    mini=0;
                    maxi=MAX/2;
                    delt=TRACK_MAX_BITS_DELT;
                    ddelt=TRACK_MAX_BITS_DDELT;
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
#ifdef OLD
                case TRACK_MAX_BITS_SCROLL:
                    track_max_bits=i;
                    repaint_track_max_bits(hDlg);
                    break;
#endif
            }
            out_smart=0;
        }
        break;
    case WM_COMMAND:
      switch (LOWORD(wParam)) {
        case TRACE_TIME_BIN:
            this_data=gendata[current_image];
            if (this_data->valid && 
                this_data->type==DATATYPE_2D)
                trace_time_bin(this_data);
            break;
            
        case TRACE_SAVE_BIN:
            trace_save_bin();
            break;
        case TRACE_REJECT:
            trace_reject=IsDlgButtonChecked(hDlg,TRACE_REJECT);
            break;
        case TRACE_REJECT_THAT:
            trace_reject_that=IsDlgButtonChecked(hDlg,TRACE_REJECT_THAT);
            break;
        case TRACE_NUM:
            if (!out_smart)
            {
                out_smart=1;
                GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
                trace_num=atof(string);
                out_smart=0;
            }
            break;
        case TRACE_NUM2:
            if (!out_smart)
            {
                out_smart=1;
                GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
                trace_out_num=atof(string);
                out_smart=0;
            }
            break;
        case TRACE_AVG_MS:
            if (!out_smart)
            {
                out_smart=1;
                GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
                trace_state_total=atof(string)*1000*trace_num;
                out_smart=0;
            }
            break;
        case TRACE_AVG_MS2:
            if (!out_smart)
            {
                out_smart=1;
                GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
                trace_total=atof(string)*1000*trace_out_num+trace_state_total;
                out_smart=0;
            }
            break;
        case TRACE_AVG_V:
            if (!out_smart)
            {
                out_smart=1;
                GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
                trace_avg_v=in_vtod(atof(string));
                out_smart=0;
            }
            break;
        case TRACE_AVG_V2:
            if (!out_smart)
            {
                out_smart=1;
                GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
                trace_avg_out_v=in_vtod(atof(string));
                out_smart=0;
            }
            break;
        case TRACE_BIN_WIDTH:
            if (!out_smart)
            {
                out_smart=1;
                GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
                trace_bin_width=atof(string);
                if (trace_bin_width<=0)
                {
                    trace_bin_width=1;
                    SetDlgItemText(hDlg,TRACE_BIN_WIDTH,"0");
                }
                out_smart=0;
            }
            break;
        case TRACE_REJECT_TIME:
            if (!out_smart)
            {
                out_smart=1;
                GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
                trace_reject_time=atof(string);
                if (trace_reject_time<0)
                {
                    trace_reject_time=0;
                    SetDlgItemText(hDlg,TRACE_REJECT_TIME,"0");
                }
                out_smart=0;
            }
            break;
        case TRACE_REJECT_THAT_TIME:
            if (!out_smart)
            {
                out_smart=1;
                GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
                trace_reject_that_time=atof(string);
                if (trace_reject_that_time<0)
                {
                    trace_reject_that_time=0;
                    SetDlgItemText(hDlg,TRACE_REJECT_THAT_TIME,"0");
                }
                out_smart=0;
            }
            break;
        
        case TRACE_ANALYZE:
            this_data=gendata[current_image];
            if (this_data->valid && 
                this_data->type==DATATYPE_2D)
            {
                destroy_trace_list(&this_width_list);
                destroy_trace_list(&that_width_list);
                last_analyzed=current_image;
                
                if (constrain)
                {
                    if (sel_r2.pt1->x>=0 && sel_r2.image==current_image
                        && sel_r1.pt1->x>=0 && sel_r1.image==current_image)
                    {
                        if (sel_r2.pt1->x>sel_r1.pt1->x)
                        {
                            start=sel_r1.pt1->x;
                            end=sel_r2.pt1->x;
                        }
                        else if (sel_r2.pt1->x<sel_r1.pt1->x)
                        {
                            start=sel_r2.pt1->x;
                            end=sel_r1.pt1->x;
                        }
                    }
                    else if (sel_r1.pt1->x>=0 && sel_r1.image==current_image)
                    {
                        start=sel_r1.pt1->x;
                        end=this_data->size-1;
                    }
                    else
                    {
                        start=0;
                        end=sel_r2.pt1->x;
                    }
                }
                else
                {
                    start=0;
                    end=this_data->size-1;
                }
                pos=start;
                if (!pos) pos++;
                trace_state_total=trace_total=0;
                trace_out_num=trace_num=0;
                trace_avg_out_v=trace_avg_v=0;
                this_in_total=this_out_total=0;
                this_avg_out_v=this_avg_v=0;
                switch(trace_init_state)
                {
                    case TRACE_START_UNDEFINED:
                        this_state=TRACE_UNDEFINED;
                        break;
                    case TRACE_START_HIGH:
                        if (trace_state==TRACE_HIGH)
                            this_state=TRACE_IN_STATE;
                        else this_state=TRACE_OUT_STATE;
                        break;
                    case TRACE_START_LOW:
                        if (trace_state==TRACE_HIGH)
                            this_state=TRACE_OUT_STATE;
                        else this_state=TRACE_IN_STATE;
                        break;
                }
                this_state_old=prev_state_old=prev_state=this_state;
                while(pos<=end)
                {
                    switch (trace_in_range(this_data,pos))
                    {
                        case TRACE_UNDEFINED:
                            switch(this_state)
                            {
                                case TRACE_UNDEFINED:
                                    break;
                                case TRACE_OUT_STATE:
                                    prev_state=this_state;
                                    this_state=TRACE_UNDEFINED;
                                    this_avg_out_v=0;
                                    this_out_total=0;
                                    break;
                                case TRACE_IN_STATE:
                                    prev_state=this_state;
                                    this_state=TRACE_UNDEFINED;
                                    this_in_total=0;
                                    this_avg_v=0;
                                    break;
                            }
                            break;
                        case TRACE_OUT_STATE:
                            switch(this_state)
                            {
                                case TRACE_UNDEFINED:
                                    prev_state=this_state;
                                    this_state=TRACE_OUT_STATE;
                                    
                                    break;
                                case TRACE_OUT_STATE:
                                    if (prev_state==TRACE_IN_STATE)
                                    {
                                        delta_t=this_data->time2d[pos]-
                                                this_data->time2d[pos-1];
                                        this_out_total+=delta_t;
                                        this_avg_out_v+=(this_data->data2d[pos]+
                                            this_data->data2d[pos-1])*delta_t/2;
                                    }
                                    break;
                                case TRACE_IN_STATE:
                                    if (prev_state==TRACE_OUT_STATE)
                                    {
                                        if (trace_reject && 
                                            this_in_total<trace_reject_time*1000)
                                        {
                                            if (trace_out_num>0) trace_out_num--;
                                            if (trace_total-trace_state_total>1)
                                            {
                                                temp_avg_v=trace_avg_out_v/
                                                    (trace_total-trace_state_total);
                                                trace_avg_out_v=temp_avg_v*
                                                    (trace_total-trace_state_total-
                                                    this_out_total_old);
                                                this_avg_out_v=temp_avg_v*(
                                                    this_out_total_old+this_in_total);
                                                
                                                remove_last_trace_el(&that_width_list);

                                                    
/*
                                                trace_avg_out_v*=
                                                    (trace_total+this_in_total-
                                                     trace_state_total)/
                                                    (trace_total-trace_state_total);
*/
                                                
                                            }
                                            this_state=this_state_old;
                                            prev_state=prev_state_old;
                                            this_out_total=this_out_total_old+this_in_total;
                                            trace_total-=this_out_total_old;
                                        }
                                        else
                                        {
                                            trace_state_total+=this_in_total;
                                            trace_avg_v+=this_avg_v;
                                            trace_num++;
                                            new_trace_el(&this_width_list,
                                                this_in_total);
                                            
                                            this_in_total_old=this_in_total;
                                            this_state_old=this_state;
                                            prev_state_old=prev_state;
                                            prev_state=this_state;
                                            this_state=TRACE_OUT_STATE;
                                            
                                            trace_total+=this_in_total;
                                        }
                                        this_in_total=0;
                                        this_avg_v=0;
                                    }
                                    else
                                    {
                                        prev_state=this_state;
                                        this_state=TRACE_OUT_STATE;
                                        this_in_total=0;
                                        this_avg_v=0;
                                    }
                                    break;
                            }
                            break;
                        case TRACE_IN_STATE:
                            switch(this_state)
                            {
                                case TRACE_UNDEFINED:
                                    prev_state=this_state;
                                    this_state=TRACE_IN_STATE;
                                    break;
                                case TRACE_OUT_STATE:
                                    if (prev_state==TRACE_IN_STATE)
                                    {
                                        if (trace_reject_that &&
                                            this_out_total<trace_reject_that_time*1000)
                                        {
                                            if (trace_num>0) trace_num--;
                                            if (trace_state_total>1) /*time granularity >> 1 */
                                            {
                                                temp_avg_v=trace_avg_v/trace_state_total;
                                                trace_avg_v=temp_avg_v*
                                                    (trace_state_total-this_in_total_old);
                                                this_avg_v=temp_avg_v*(
                                                    this_in_total_old+this_out_total);
                                                remove_last_trace_el(&this_width_list);
/*
                                                trace_avg_v*=(trace_state_total+this_out_total)/
                                                    trace_state_total;
*/
                                            }
                                            this_in_total=this_in_total_old+this_out_total;
                                            trace_total-=this_in_total_old;
                                            trace_state_total-=this_in_total_old;
                                            
                                            prev_state=prev_state_old;
                                            this_state=this_state_old;
                                            
                                        }
                                        else
                                        {
                                            trace_avg_out_v+=this_avg_out_v;
                                            trace_out_num++;
                                            new_trace_el(&that_width_list,
                                                this_out_total);
                                            this_state_old=this_state;
                                            prev_state_old=prev_state;
                                            this_out_total_old=this_out_total;
                                            
                                            trace_total+=this_out_total;
                                            prev_state=this_state;
                                            this_state=TRACE_IN_STATE;
                                        }
                                        this_out_total=0;
                                        this_avg_out_v=0;                    
                                    }
                                    else
                                    {
                                        prev_state=this_state;
                                        this_state=TRACE_IN_STATE;
                                    }
                                    break;
                                case TRACE_IN_STATE:
                                    if (prev_state==TRACE_OUT_STATE)
                                    {
                                        delta_t=this_data->time2d[pos]-
                                                this_data->time2d[pos-1];
                                        this_in_total+=delta_t;
                                        this_avg_v+=(this_data->data2d[pos]+
                                            this_data->data2d[pos-1])*delta_t/2;
                                    }
                                    break;
                            }
                            break;
                    }
                                    
                                    
                                    
                                
                    pos++;
                }
                if (trace_state_total>0) 
                {
                    trace_avg_v=trace_avg_v/trace_state_total;
                }
                if (trace_total-trace_state_total>0)
                {
                    trace_avg_out_v/=(trace_total-trace_state_total);
                }
                repaint_all(hDlg);
            }
            
            break;
        case TRACE_CLEAR:
            trace_num_sum=0;
            trace_out_num_sum=0;
            trace_avg_ms_sum=0;
            trace_avg_ms2_sum=0;
            trace_avg_v_sum=IN_ZERO;
            trace_avg_v2_sum=IN_ZERO;
            trace_clear_bin(&this_bin_sum);
            trace_clear_bin(&that_bin_sum);
            repaint_all(hDlg);
            break;
        case TRACE_ADD:
            trace_avg_v_sum=(trace_avg_v_sum*trace_avg_ms_sum*trace_num_sum)+
                trace_avg_v*trace_state_total;
            trace_avg_v2_sum=(trace_avg_v2_sum*trace_avg_ms2_sum*trace_out_num_sum)+
                trace_avg_out_v*(trace_total-trace_state_total);
            trace_avg_ms_sum=(trace_avg_ms_sum*trace_num_sum)+trace_state_total;
            trace_avg_ms2_sum=(trace_avg_ms2_sum*trace_out_num_sum)+
                (trace_total-trace_state_total);
            trace_num_sum+=trace_num;
            trace_out_num_sum+=trace_out_num;
            if (trace_avg_ms_sum>0)
            {
                trace_avg_v_sum/=trace_avg_ms_sum;
            } else trace_avg_v_sum=IN_ZERO;
            if (trace_avg_ms2_sum>0)
            {
                trace_avg_v2_sum/=trace_avg_ms2_sum;
            } else trace_avg_v2_sum=IN_ZERO;
            if (trace_num_sum)
            {
                trace_avg_ms_sum/=trace_num_sum;
            } else trace_avg_ms_sum=0;
            if (trace_out_num_sum)
            {
                trace_avg_ms2_sum/=trace_out_num_sum;
            } else trace_avg_ms2_sum=0;
            trace_add_bin(&this_bin_sum,this_bin);
            trace_add_bin(&that_bin_sum,that_bin);
            if (this_bin_sum.size) trace_save_binned(this_bin_sum,"bs1");
            if (that_bin_sum.size) trace_save_binned(that_bin_sum,"bs2");
            if (this_bin_sum.size) trace_display_bin(this_bin_sum);
            trace_clear_bin(&this_bin);
            trace_clear_bin(&that_bin);
            repaint_all(hDlg);
            break;            
                    
        
                
        case TRACE_HIGH:
        case TRACE_LOW:
            trace_state=LOWORD(wParam);
            break;
        case TRACE_START_HIGH:
        case TRACE_START_LOW:
        case TRACE_START_UNDEFINED:
            trace_init_state=LOWORD(wParam);
            break;
        case TRACE_FROM:
            if (!out_smart)
            {
                out_smart=1;
                GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
                i = in_vtod(atof(string));
                if (i<0) 
                {
                    i=0;
                    sprintf(string,"%0.3f",in_dtov((float)i));
                    SetDlgItemText(hDlg,LOWORD(wParam),string);
                }
                if (i>trace_to) 
                {
                    i=trace_to;
                    sprintf(string,"%0.3f",in_dtov((float)i));
                    SetDlgItemText(hDlg,LOWORD(wParam),string);
                }
                if (trace_from!=i)
                {
                    SendMessage(ImgDlg, WM_COMMAND, IMAGE_REPAINT_TRACE_LIMITS, 0);
                    trace_from=i;
                    SendMessage(ImgDlg, WM_COMMAND, IMAGE_REPAINT_TRACE_LIMITS, 0);
                }
                    
                out_smart=0;
            }
            break;
        case TRACE_TO:
            if (!out_smart)
            {
                out_smart=1;
                GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
                i = in_vtod(atof(string));
                if (i<trace_from) 
                {
                    i=trace_from;
                    sprintf(string,"%0.3f",in_dtov((float)i));
                    SetDlgItemText(hDlg,LOWORD(wParam),string);
                }
                if (i>IN_MAX) 
                {
                    i=IN_MAX;
                    sprintf(string,"%0.3f",in_dtov((float)i));
                    SetDlgItemText(hDlg,LOWORD(wParam),string);
                }
                if (trace_to!=i)
                {
                    SendMessage(ImgDlg, WM_COMMAND, IMAGE_REPAINT_TRACE_LIMITS, 0);
                    trace_to=i;
                    SendMessage(ImgDlg, WM_COMMAND, IMAGE_REPAINT_TRACE_LIMITS, 0);
                }
                out_smart=0;
            }
            break;
#ifdef OLD
        case TRACE_OFFSET_X:
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
#endif
        case TRACE_EXIT:
          SendMessage(ImgDlg, WM_COMMAND, IMAGE_REPAINT_TRACE_LIMITS, 0);
          DestroyWindow(hDlg);
          TraceWnd = NULL;
          TraceOn = FALSE;
          break;
      }
    break;
  }
  return(FALSE);
}
 
static int trace_in_range(datadef* this_data,int pos)
{   
    int result=TRACE_UNDEFINED;

    if (this_data->data2d[pos]>=trace_from && this_data->data2d[pos]<=trace_to)
        result=TRACE_IN_STATE;
    else switch(trace_state)
    {
        case TRACE_HIGH:
            if (this_data->data2d[pos]<trace_from)
                result=TRACE_OUT_STATE;
            else result=TRACE_UNDEFINED;
            break;
        case TRACE_LOW:
            if (this_data->data2d[pos]<trace_from)
                result=TRACE_UNDEFINED;
            else result=TRACE_OUT_STATE;
            break;
    }
           
    return result;
}

static void repaint_all(HWND hDlg)
{
    out_smart=1;
    CheckDlgButton(hDlg,TRACE_HIGH,0);
    CheckDlgButton(hDlg,TRACE_LOW,0);
    CheckDlgButton(hDlg,TRACE_START_LOW,0);
    CheckDlgButton(hDlg,TRACE_START_HIGH,0);
    CheckDlgButton(hDlg,TRACE_START_UNDEFINED,0);
    
    CheckDlgButton(hDlg,TRACE_REJECT,trace_reject);
    CheckDlgButton(hDlg,TRACE_REJECT_THAT,trace_reject_that);

    
    CheckDlgButton(hDlg,trace_init_state,1);
    CheckDlgButton(hDlg,trace_state,1);
    sprintf(string,"%0.3f",trace_reject_time);
    SetDlgItemText(hDlg,TRACE_REJECT_TIME,string);
    sprintf(string,"%0.3f",trace_reject_that_time);
    SetDlgItemText(hDlg,TRACE_REJECT_THAT_TIME,string);
    
    sprintf(string,"%0.3f",in_dtov((float)trace_from));
    SetDlgItemText(hDlg,TRACE_FROM,string);
    sprintf(string,"%0.3f",in_dtov((float)trace_to));
    SetDlgItemText(hDlg,TRACE_TO,string);
    
    sprintf(string,"%0.3f",trace_state_total/1000);
    SetDlgItemText(hDlg,TRACE_STATE_TOTAL,string);
    sprintf(string,"%0.3f",(trace_total-trace_state_total)/1000);
    SetDlgItemText(hDlg,TRACE_STATE_TOTAL2,string);
    sprintf(string,"%0.3f",trace_total/1000);
    SetDlgItemText(hDlg,TRACE_TOTAL,string);
    sprintf(string,"%d",trace_num);
    SetDlgItemText(hDlg,TRACE_NUM,string);
    sprintf(string,"%d",trace_out_num);
    SetDlgItemText(hDlg,TRACE_NUM2,string);
    if (trace_num>0)
    {
        sprintf(string,"%0.3f",trace_state_total/1000/trace_num);
        SetDlgItemText(hDlg,TRACE_AVG_MS,string);
    }
    else SetDlgItemText(hDlg,TRACE_AVG_MS,"NA");
    if (trace_out_num>0)
    {
        sprintf(string,"%0.3f",(trace_total-trace_state_total)/1000/trace_out_num);
        SetDlgItemText(hDlg,TRACE_AVG_MS2,string);
    }
    else SetDlgItemText(hDlg,TRACE_AVG_MS2,"NA");
    sprintf(string,"%0.3f",in_dtov((float)trace_avg_v));
    SetDlgItemText(hDlg,TRACE_AVG_V,string);
    sprintf(string,"%0.3f",in_dtov((float)trace_avg_out_v));
    SetDlgItemText(hDlg,TRACE_AVG_V2,string);
    
    sprintf(string,"%d",trace_num_sum);
    SetDlgItemText(hDlg,TRACE_NUM_SUM,string);
    sprintf(string,"%d",trace_out_num_sum);
    SetDlgItemText(hDlg,TRACE_NUM2_SUM,string);

    sprintf(string,"%0.3f",in_dtov((float)trace_avg_v_sum));
    SetDlgItemText(hDlg,TRACE_AVG_V_SUM,string);
    sprintf(string,"%0.3f",in_dtov((float)trace_avg_v2_sum));
    SetDlgItemText(hDlg,TRACE_AVG_V2_SUM,string);

    sprintf(string,"%0.3f",trace_avg_ms_sum/1000);
    SetDlgItemText(hDlg,TRACE_AVG_MS_SUM,string);
    sprintf(string,"%0.3f",trace_avg_ms2_sum/1000);
    SetDlgItemText(hDlg,TRACE_AVG_MS2_SUM,string);

    sprintf(string,"%0.3f",trace_bin_width);
    SetDlgItemText(hDlg,TRACE_BIN_WIDTH,string);

    
#ifdef OLD
    sprintf(string,"%d",track_offset_x);
    SetDlgItemText(hDlg,TRACE_OFFSET_X,string);
    sprintf(string,"%d",track_offset_y);
    SetDlgItemText(hDlg,TRACE_OFFSET_Y,string);
    sprintf(string,"%d",track_sample_every);
    SetDlgItemText(hDlg,TRACE_SAMPLE_EVERY,string);
    sprintf(string,"%d",track_avg_data_pts);
    SetDlgItemText(hDlg,TRACE_AVG_DATA_PTS,string);
    SetScrollPos(GetDlgItem(hDlg,TRACE_MAX_BITS_SCROLL),SB_CTL,track_max_bits,TRUE);
    
    repaint_current_pos(hDlg);
    repaint_track_max_bits(hDlg);
#endif
    out_smart=0;
}    
#ifdef OLD

static void enable_all(HWND hDlg,int status)
{
    EnableWindow(GetDlgItem(hDlg, TRACE_START), status);
    EnableWindow(GetDlgItem(hDlg, TRACE_START_AGAIN), status);
    EnableWindow(GetDlgItem(hDlg, TRACE_EXIT), status);
    EnableWindow(GetDlgItem(hDlg, TRACE_MAX_BITS), status);
    EnableWindow(GetDlgItem(hDlg, TRACE_SAMPLE_EVERY), status);
/*
    EnableWindow(GetDlgItem(hDlg, TRACE_), status);
    EnableWindow(GetDlgItem(hDlg, TRACE_), status);
    EnableWindow(GetDlgItem(hDlg, TRACE_), status);
*/
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

float read_compare(float best)
{
    float z;
    if (current_y>=track_limit_y_min && 
                current_y<=track_limit_y_max && 
                current_x>=track_limit_x_min && 
                current_x<=track_limit_x_max)
    {    
        dio_out(x_ch,current_x);
        dio_start_clock(inter_step_delay);
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
            case TRACE_MAX:
                if (z>best) 
                {
                    best=z;
                    best_x=current_x;
                    best_y=current_y;
                }
                break;
            case TRACE_MIN:
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

static void repaint_current_pos(HWND hDlg)
{
    sprintf(string,"%d",dac_data[x_ch]);
    SetDlgItemText(hDlg,TRACE_CUR_POS_X,string);
    sprintf(string,"%d",dac_data[y_ch]);
    SetDlgItemText(hDlg,TRACE_CUR_POS_Y,string);
}    

static void repaint_track_max_bits(HWND hDlg)
{
    out_smart=1;    
    SetScrollPos(GetDlgItem(hDlg,TRACE_MAX_BITS_SCROLL),SB_CTL,track_max_bits,TRUE);
    SendMessage(ScnDlg, WM_COMMAND, SCAN_REPAINT_TRACE_LIMITS, 0);
    recalc_track_limits();
    SendMessage(ScnDlg, WM_COMMAND, SCAN_REPAINT_TRACE_LIMITS, 0);
    sprintf(string,"%d",track_max_bits);
    SetDlgItemText(hDlg,TRACE_MAX_BITS,string);
    repaint_track_limits(hDlg);
    out_smart=0;
}        
static void repaint_track_limits(HWND hDlg)
{
    sprintf(string,"%d",track_limit_x_min);
    SetDlgItemText(hDlg,TRACE_CUR_LIMIT_X_MIN,string);
    sprintf(string,"%d",track_limit_x_max);
    SetDlgItemText(hDlg,TRACE_CUR_LIMIT_X_MAX,string);
    sprintf(string,"%d",track_limit_y_min);
    SetDlgItemText(hDlg,TRACE_CUR_LIMIT_Y_MIN,string);
    sprintf(string,"%d",track_limit_y_max);
    SetDlgItemText(hDlg,TRACE_CUR_LIMIT_Y_MAX,string);
}    
static void recalc_track_limits()
{
    track_limit_x_max=current_x+track_max_bits;
    if (track_limit_x_max>MAX) track_limit_x_max=MAX;
    track_limit_x_min=((int)current_x)-track_max_bits;
    if (track_limit_x_min<0) track_limit_x_min=0;
    track_limit_y_max=current_y+track_max_bits;
    if (track_limit_y_max>MAX) track_limit_y_max=MAX;
    track_limit_y_min=((int)current_y)-track_max_bits;
    if (track_limit_y_min<0) track_limit_y_min=0;
}
#endif

static void new_trace_el(TRACE_EL **this_list,float width)
{
    TRACE_EL *next;
    TRACE_EL *prev;
    TRACE_EL *this;
    
    if (*this_list==NULL)
    {
        *this_list=(TRACE_EL *) malloc(sizeof(TRACE_EL));
        (*this_list)->next=*this_list;
        (*this_list)->prev=*this_list;
        this=(*this_list);
    }
    else
    {
        
        next=*this_list;
        prev=(*this_list)->prev;
        this=(TRACE_EL *) malloc(sizeof(TRACE_EL));
        this->next=next;
        this->prev=prev;
        next->prev=this;
        prev->next=this;
    } 
    this->width=width;
}  
   
static void remove_last_trace_el(TRACE_EL **trace_list)
{
    TRACE_EL *this_el;
    
    if ((*trace_list)!=NULL)
    {
        this_el=(*trace_list)->prev;
        if (this_el->next==this_el) *trace_list=NULL;
        this_el->next->prev=this_el->prev;
        this_el->prev->next=this_el->next;
        free(this_el);
    }
}

static void destroy_trace_list(TRACE_EL **trace_list)
{
    while((*trace_list)!=NULL)
    {
        remove_last_trace_el(trace_list);
    }
}

static void trace_save_width(TRACE_EL *this_list,char *ext)
{
    FILE *fp;
    char *temp_char;
    char this_file[100];
    char full_file[300];
    TRACE_EL *this_el;
    
    strcpy(this_file,
        gendata[last_analyzed]->filename);
    temp_char=strrchr(this_file,'.');
    if (temp_char!=NULL)
    {
        *(temp_char+1)='\0';
    }
    else 
    {
        strcat(this_file,".");
    }
    strcat(this_file,ext);
    
    strcpy(full_file,initial_dir_trc);
    strcat(full_file,"\\");
    strcat(full_file,this_file);
    
    this_el=this_list;
    if (this_el!=NULL)
    {
        fp=fopen(full_file,"wb");
        if (fp==NULL) 
        {
            mprintf(full_file);
            return;
        }
        do
        {
            fprintf(fp,"%f\n",this_el->width/1000);
            this_el=this_el->next;
        } while (this_el!=this_list);
        fclose(fp);
    }
    
}
        
static void trace_bin(TRACE_BIN *bin,TRACE_EL *list,float width)
{
    TRACE_EL *this_el;
    float max_width=0;
    int i;
    
    trace_clear_bin(bin);
    bin->width=width;
    if (list!=NULL)
    {
        this_el=list;
        do
        {
            if (max_width<this_el->width) max_width=this_el->width;
            this_el=this_el->next;
        } while (this_el!=list);
        bin->size=max_width/1000/width+1;
        bin->array=(int *) malloc(sizeof(int)*bin->size);
        for(i=0;i<bin->size;i++) bin->array[i]=0;
        this_el=list;
        do
        {
            bin->array[(int) (this_el->width/width/1000)]++;
            this_el=this_el->next;
        } while (this_el!=list);
    }
}
        
static void trace_save_binned(TRACE_BIN bin,char *ext)
{
    FILE *fp;
    char *temp_char;
    char this_file[100];
    char full_file[300];
    int i;
    
    strcpy(this_file,
        gendata[last_analyzed]->filename);
    temp_char=strrchr(this_file,'.');
    if (temp_char!=NULL)
    {
        *(temp_char+1)='\0';
    }
    else 
    {
        strcat(this_file,".");
    }
    strcat(this_file,ext);
    
    strcpy(full_file,initial_dir_trc);
    strcat(full_file,"\\");
    strcat(full_file,this_file);
    if (bin.size>0)
    {
        fp=fopen(full_file,"wb");
        if (fp==NULL) 
        {
            mprintf(full_file);
            return;
        }
        for(i=0;i<bin.size;i++)
        {
            fprintf(fp,"%d %d\n",i,bin.array[i]);
        }
        fclose(fp);
    }
}

static void trace_save_bin()    
{
    
    trace_bin(&this_bin,this_width_list,trace_bin_width);
    trace_bin(&that_bin,that_width_list,trace_bin_width);
    trace_save_width(this_width_list,"t1");
    trace_save_width(that_width_list,"t2");
    trace_save_binned(this_bin,"b1");
    trace_save_binned(that_bin,"b2");
    trace_display_bin(this_bin);
}
static void trace_display_bin(TRACE_BIN this_bin)
{
    int i,max_hist,bin,j;
    if (this_bin.size)
    {
        free_data(&gendata[3]);
        alloc_data(&gendata[3],DATATYPE_3D,DATA_MAX,GEN2D_NONE,GEN2D_NONE,1);
        max_hist=0;
        for(i=0;i<this_bin.size;i++) max_hist=max(max_hist,this_bin.array[i]);
        gendata[3]->size=256;
        gendata[3]->min_z=gendata[3]->clip_min=0;
        gendata[3]->max_z=gendata[3]->clip_max=255;
        for(i=0;i<256;i++)
        {
            bin=i*this_bin.size/256;
            for(j=0;j<256;j++)
            {
                if (j<this_bin.array[bin]*256/max_hist) *(gendata[3]->ptr+i+j*256)=128;
                else *(gendata[3]->ptr+i+j*256)=0;
            }
        }
        gendata[3]->valid=1;
        unequalize(gendata[3]);
        reset_image(ImgDlg,3);
    }
}

static void trace_add_bin(TRACE_BIN *dest,TRACE_BIN src)
{
    int i;
    int *temp_array;
    
    if (src.size)
    {
        if (dest->array==NULL)
        {
            dest->array=(int *) malloc(sizeof(int)*src.size);
            dest->size=src.size;
            dest->width=src.width;
            memcpy(dest->array,src.array,sizeof(int)*src.size);
            return;
        }
        else
        {
            if (dest->width!=src.width) mprintf("Warning! Different bin widths.");
            
            if (dest->size<src.size)
            {
                temp_array=(int *) malloc(sizeof(int)*src.size);
                memcpy(temp_array,dest->array,sizeof(int)*dest->size);
                free(dest->array);
                dest->array=(int *) malloc(sizeof(int)*src.size);
		memset(dest->array,0,sizeof(int)*src.size);
                memcpy(dest->array,temp_array,sizeof(int)*dest->size);
                dest->size=src.size;
                free(temp_array);
            }
            for(i=0;i<src.size;i++)
            {
                dest->array[i]+=src.array[i];
            }
        }
    }
}  

static void trace_clear_bin(TRACE_BIN *bin)
{
    if (bin->array!=NULL)
    {
        free(bin->array);
    }
    bin->array=NULL;
    bin->size=0;
    bin->width=0;
}

int time_bin_cmp(const void *first,const void *second)
{
    TIME_BINS *my_first;
    TIME_BINS *my_second;
    int result=0;
    
    my_first=(TIME_BINS *)first;
    my_second=(TIME_BINS *)second;
    
    if (my_first->time>my_second->time) result = 1;
    if (my_first->time<my_second->time) result = -1;
    
    return(result);
}

static void trace_time_bin(datadef *this_data)
{
    int i,j;
    float *everything;
#define MAX_BINS 1000
    int max_bins = MAX_BINS;
    int found;
    int max_hist;
    TIME_BINS *this_time;
    int already;
    float time_min,time_length;
    int bin;
    FILE *fp;
    char *temp_char;
    char this_file[100];
    char full_file[300];
    
TRACE_TIME_BIN_RESTART:
    everything = (float *) malloc(sizeof(float)*max_bins);
    
    found=0;
    for(i=1;i<this_data->size;i++)
    {
        float time;
        
        time=this_data->time2d[i]-this_data->time2d[i-1];
        
        already=0;
/*
sprintf(string,"%d %f",found,time);
mprintf(string);        
*/
        for(j=0;j<found;j++)
        {
            if (time==everything[j])
            {
                already=1;
                break;
            }
        }
        if (!already)
        {
            everything[found]=time;
            found++;
            if (found>=max_bins)
            {
                free(everything);
                max_bins*=2;
                goto TRACE_TIME_BIN_RESTART;
            }
        }
    }
    free(everything);
//mprintf("1");    
    
    this_time=(TIME_BINS *)malloc(sizeof(TIME_BINS)*found);
    
    found=0;
    for(i=1;i<this_data->size;i++)
    {
        float time;
        int already;
        
        time=this_data->time2d[i]-this_data->time2d[i-1];
        
        already=0;
        for(j=0;j<found;j++)
        {
            if (time==this_time[j].time)
            {
                already=1;
                this_time[j].num++;
                break;
            }
        }
        if (!already)
        {
            this_time[found].time=time;
            this_time[found].num=1;
            found++;
        }
    }
    qsort((void *)this_time,found,sizeof(TIME_BINS),time_bin_cmp);
    
    
    strcpy(this_file,
        gendata[last_analyzed]->filename);
    temp_char=strrchr(this_file,'.');
    if (temp_char!=NULL)
    {
        *(temp_char+1)='\0';
    }
    else 
    {
        strcat(this_file,".");
    }
    strcat(this_file,"TB");
    
    strcpy(full_file,initial_dir_trc);
    strcat(full_file,"\\");
    strcat(full_file,this_file);
    
    fp=fopen(full_file,"wb");
    for(i=0;i<found;i++)
    {
        fprintf(fp,"%f %d\n",this_time[i].time,this_time[i].num);
    }
    fclose(fp);
    
    
        
        free_data(&gendata[3]);
        alloc_data(&gendata[3],DATATYPE_3D,DATA_MAX,GEN2D_NONE,GEN2D_NONE,1);
        max_hist=0;
        time_min=this_time[0].time;
        time_length=this_time[found-1].time-this_time[0].time;
        for(i=0;i<found;i++) 
        {
            max_hist=max(max_hist,this_time[i].num);
        }
        gendata[3]->size=256;
        gendata[3]->min_z=gendata[3]->clip_min=0;
        gendata[3]->max_z=gendata[3]->clip_max=255;
        for(i=0;i<256;i++)
            for(j=0;j<256;j++)
            {
                *(gendata[3]->ptr+i+j*256)=0;   
            }
            
        already=0;
        for(i=0;i<found;i++)
        {
            bin=(this_time[i].time-time_min)*256/time_length;
            for(j=0;j<256;j++)
            {
                if (j<this_time[i].num*256/max_hist) *(gendata[3]->ptr+bin+j*256)=128;
//                else *(gendata[3]->ptr+bin+j*256)=0;
            }
        }
            
#ifdef OLD
        for(i=0;i<256;i++)
        {
            bin=((float)i)*(float)found/(float)256;
            if (bin<(this_time[already].time+0.5) && 
                bin>(this_time[already].time-0.5) )
            {
                for(j=0;j<256;j++)
                {
                    if (j<this_time[already].num*256/max_hist) *(gendata[3]->ptr+i+j*256)=128;
                    else *(gendata[3]->ptr+i+j*256)=0;
                }
                already++;
            }
        }
#endif
        gendata[3]->valid=1;
        unequalize(gendata[3]);
        reset_image(ImgDlg,3);
}
