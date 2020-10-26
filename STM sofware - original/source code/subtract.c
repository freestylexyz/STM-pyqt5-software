#include <time.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include <i86.h>
#include "file.h"
#include "data.h"
#include "dio.h"
#include "stm.h"
#include "scan.h"
#include "image.h"
#include "sel.h"
#include "subtract.h"

BOOL FAR PASCAL SubDlg(HWND,unsigned,WPARAM,LPARAM);

HWND SubWnd=NULL;
BOOL SubOn=FALSE;
extern HWND ImgDlg;
extern BOOL SelOn;

extern float image_lowerv[],image_higherv[],image_lowert[],image_highert[];
extern int current_image;
extern datadef *gendata[];
extern char string[];
static int out_smart=0;
float sub_x=0,sub_y=0;
float sub_z=IN_ZERO;
extern int sel1_on,sel2_on;
extern SEL_POINT sel_p1,sel_p2,sel_p3,sel_p4;
extern SEL_REGION sel_r1,sel_r2;
extern int constrain;
int sub_mode=SUB_SUBTRACT;


static datadef *create_single_step(datadef *);
static void repaint_parameters(HWND);
static void integrate_data(HWND, int);
static void scale_data(int, float);
static void repaint_all(HWND);
void decide(HWND, int, int);

BOOL FAR PASCAL SubDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{
    int i;
    int id;
    int delt,ddelt,mini,maxi;
    double d,minid,maxid,deltd,ddeltd;
    int end;
    
    switch(Message)
    {
        case WM_INITDIALOG:
/*
            SetScrollRange(GetDlgItem(hDlg, VSCALE_MIN_SC), SB_CTL, 0,32767, TRUE);
*/
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
/*
                case VSCALE_MIN_SC:
                    d=(double) image_lowerv[current_image];
                    minid=min(gendata[current_image]->min_z,IN_ZERO);
                    maxid=image_higherv[current_image];
                    deltd=(maxid-minid)/100;
                    ddeltd=(maxid-minid)/10;
                    break;
                case VSCALE_MAX_SC:
                    d=(double) image_higherv[current_image];
                    minid=image_lowerv[current_image];
                    maxid=max(gendata[current_image]->max_z,IN_MAX);
                    deltd=(maxid-minid)/100;
                    ddeltd=(maxid-minid)/10;
                    break;
*/
    
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
/*
                case VSCALE_MIN_SC:
                    image_lowerv[current_image]=d;
                    repaint_all(hDlg);
                    SendMessage(ImgDlg, WM_COMMAND, IM_NEW_VSCALE, 0);
                    break;
                    
                case VSCALE_MAX_SC:
                    image_higherv[current_image]=d;
                    repaint_all(hDlg);
                    SendMessage(ImgDlg, WM_COMMAND, IM_NEW_VSCALE, 0);
                    break;
*/
            }
            out_smart=0;
        }
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
            case SUB_SUBTRACT:
            case SUB_AVERAGE:
            case SUB_DIVIDE:
            case SUB_DIDV:
            case SUB_SMOOTH:
            case SUB_INTEGRATE:
            case SUB_SCALE:
                sub_mode=LOWORD(wParam);
                repaint_parameters(hDlg);
                break;
            case SUB_GET:
                if (SelOn)
                {
                    sub_x=sel_r1.pt1->x-sel_r2.pt1->x;
                    sub_y=sel_r1.pt1->y-sel_r2.pt1->y;
                    repaint_all(hDlg);
                }
                break;
            case SUB_1:                                 
              decide(hDlg,current_image,0);
              break;
            case SUB_2:                                  
              decide(hDlg,current_image,1);
              break;
            case SUB_3:                                 
              decide(hDlg,current_image,2);
              break;
            case SUB_4:                                 
              decide(hDlg,current_image,3);
              break;
            
#ifdef OLD
              
              if (current_image!=0)                     
              { 
                switch(sub_mode)
                {
                    case SUB_SUBTRACT:
                        subtract_data(current_image,0,sub_x,sub_y,sub_z);         
                        reset_image(ImgDlg,3);
                        break;
                            
                    case SUB_DIDV:
                        didv(current_image,0,sub_x,sub_y,sub_z);
                        reset_image(ImgDlg,3);
                        break;
                    case SUB_AVERAGE:
                        average_data(current_image,0,sub_x,sub_y,sub_z);
                        reset_image(ImgDlg,3);
                        break;
                }
/*
                im_src_x[3]=im_src_y[3]=0;              
                im_screenlines[3]=gendata[3]->size;     
                calc_bitmap(3);                         
                repaint_image(hDlg,3,NULL);             
*/
              }
              else
              {
                switch(sub_mode)
                {
                    case SUB_SMOOTH:
                        if (sub_x>1) smooth_1_data(current_image,sub_x);
                        reset_image(ImgDlg,3);
                        break;
                            
                    case SUB_AVERAGE:
                        if (sub_x>1) average_1_data(current_image,sub_x);
                        reset_image(ImgDlg,3);
                        break;
                }
              }
              break;                                    
            case SUB_2:                                 
              if (current_image!=1)                     
              {                                         
                switch(sub_mode)
                {
                    case SUB_SUBTRACT:
                        subtract_data(current_image,1,sub_x,sub_y,sub_z);         
                        reset_image(ImgDlg,3);
                        break;
                            
                    case SUB_DIDV:
                        didv(current_image,1,sub_x,sub_y,sub_z);
                        reset_image(ImgDlg,3);
                        break;
                    case SUB_AVERAGE:
                        average_data(current_image,1,sub_x,sub_y,sub_z);
                        reset_image(ImgDlg,3);
                        break;
                }
              }                                         
              else
              {
                switch(sub_mode)
                {
                    case SUB_SMOOTH:
                        if (sub_x>1) smooth_1_data(current_image,sub_x);
                        reset_image(ImgDlg,3);
                        break;
                            
                    case SUB_AVERAGE:
                        if (sub_x>1) average_1_data(current_image,sub_x);
                        reset_image(ImgDlg,3);
                        break;
                }
              }
              break;                                    
            case SUB_3:                                 
              if (current_image!=2)                     
              {                                         
                switch(sub_mode)
                {
                    case SUB_SUBTRACT:
                        subtract_data(current_image,2,sub_x,sub_y,sub_z);         
                        reset_image(ImgDlg,3);
                        break;
                            
                    case SUB_DIDV:
                        didv(current_image,2,sub_x,sub_y,sub_z);
                        reset_image(ImgDlg,3);
                        break;
                    case SUB_AVERAGE:
                        average_data(current_image,2,sub_x,sub_y,sub_z);
                        reset_image(ImgDlg,3);
                        break;
                }
              }                                         
              else
              {
                switch(sub_mode)
                {
                    case SUB_SMOOTH:
                        if (sub_x>1) smooth_1_data(current_image,sub_x);
                        reset_image(ImgDlg,3);
                        break;
                            
                    case SUB_AVERAGE:
                        if (sub_x>1) average_1_data(current_image,sub_x);
                        reset_image(ImgDlg,3);
                        break;
                }
              }
              break;                                    
#ifdef OLD
            case SUB_4:                                 
              if (current_image!=3)                     
              {                                         
                switch(sub_mode)
                {
                    case SUB_SUBTRACT:
                        subtract_data(current_image,3,sub_x,sub_y,sub_z);         
                        reset_image(ImgDlg,3);
                        break;
                            
                    case SUB_DIDV:
                        didv(current_image,3,sub_x,sub_y,sub_z);
                        reset_image(ImgDlg,3);
                        break;
                    case SUB_AVERAGE:
                        average_data(current_image,3,sub_x,sub_y,sub_z);
                        reset_image(ImgDlg,3);
                        break;
                }
              }                                         
              else
              {
                switch(sub_mode)
                {
                    case SUB_SMOOTH:
                        if (sub_x>1) smooth_1_data(current_image,sub_x);
                        reset_image(ImgDlg,3);
                        break;
                            
                    case SUB_AVERAGE:
                        if (sub_x>1) average_1_data(current_image,sub_x);
                        reset_image(ImgDlg,3);
                        break;
                }
              }
              break;                                    
#endif
#endif
            case SUB_X:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg, SUB_X, string, 9);
                    sub_x=atof(string);
                    out_smart=0;
                }
                break;
            case SUB_Y:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg, SUB_Y, string, 9);
                    sub_y=atof(string);
                    out_smart=0;
                }
                break;
            case SUB_Z:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg, SUB_Z, string, 9);
                    sub_z=in_vtod(atof(string));
                    out_smart=0;
                }
                break;
            
            case SUB_EXIT:
//                PostMessage(ImgDlg,WM_COMMAND,IM_VSCALE_EXIT,0);
          
                DestroyWindow(hDlg);
                SubWnd = NULL;
                SubOn = FALSE;
                break;
    }
    break;
  }
  return(FALSE);
}

static void enable_all(HWND hDlg,int status)
{
/*
    EnableWindow(GetDlgItem(hDlg, VSCALE_MAX), status);
    EnableWindow(GetDlgItem(hDlg, VSCALE_MIN), status);
    EnableWindow(GetDlgItem(hDlg, VSCALE_MAX_SC), status);
    EnableWindow(GetDlgItem(hDlg, VSCALE_MIN_SC), status);
    EnableWindow(GetDlgItem(hDlg, VSCALE_SET_MAX), status);
    EnableWindow(GetDlgItem(hDlg, VSCALE_SET_CENTER), status);
    EnableWindow(GetDlgItem(hDlg, VSCALE_SET_MIN), status);
*/
}    

static void repaint_all(HWND hDlg)
{
        out_smart=1;
        sprintf(string,"%0.2f",sub_x);
        SetDlgItemText(hDlg, SUB_X, string);
        sprintf(string,"%0.2f",sub_y);
        SetDlgItemText(hDlg, SUB_Y, string);
        sprintf(string,"%0.3f",in_dtov(sub_z));
        SetDlgItemText(hDlg, SUB_Z, string);
        CheckDlgButton(hDlg,SUB_SUBTRACT,0);
        CheckDlgButton(hDlg,SUB_AVERAGE,0);
        CheckDlgButton(hDlg,SUB_DIVIDE,0);
        CheckDlgButton(hDlg,SUB_DIDV,0);
        CheckDlgButton(hDlg,sub_mode,1);
        repaint_parameters(hDlg);
        
        out_smart=0;
}

void subtract_data(int a,int b,float sub_x,float sub_y,float sub_z) 
/* calculate a-b and puts it in 4 */
{                           
    int i,j;
    float min_z,max_z;
    int start,step,size;
    int delta_x_a, delta_x_b,delta_y_a,delta_y_b;
    float deltaf_x_a, deltaf_x_b,deltaf_y_a,deltaf_y_b;
    datadef *data_a,*data_b;
    
    if (gendata[a]->type!=gendata[b]->type) return;
    switch(gendata[a]->type)
    {
        case DATATYPE_3D:
            copy_data(&gendata[3],&gendata[a]);    
            delta_x_a=delta_x_b=delta_y_a=delta_y_b=0;
            deltaf_x_a=deltaf_x_b=deltaf_y_a=deltaf_y_b=0;
            if (abs(sub_x)>=abs(sub_y))
            {
                gendata[3]->size=min(gendata[a]->size,gendata[b]->size)
                    -abs(sub_x);
            }
            else
            {
                gendata[3]->size=min(gendata[a]->size,gendata[b]->size)
                    -abs(sub_y);
            }
            if (sub_x>0)
            {
                deltaf_x_a=sub_x;
                deltaf_x_b=0;
            }
            else if (sub_x<0)
            {
                deltaf_x_a=0;
                deltaf_x_b=-sub_x;
            }
            if (sub_y>0)
            {
                deltaf_y_a=sub_y;
                deltaf_y_b=0;
            }
            else if (sub_y<0)
            {
                deltaf_y_a=0;
                deltaf_y_b=-sub_y;
            }
            delta_x_a=(int) deltaf_x_a;
            delta_x_b=(int) deltaf_x_b;
            delta_y_a=(int) deltaf_y_a;
            delta_y_b=(int) deltaf_y_b;
                        
/*
sprintf(string,"%d %d %d %d",delta_x_a,delta_x_b,delta_y_a,delta_y_b);
mprintf(string);                
*/
            if (floor(sub_x)!=sub_x || floor(sub_y)!=sub_y) 
            {
                for (i=0;i<gendata[3]->size;i++)
                    for(j=0;j<gendata[3]->size;j++)
                    {
                        *(gendata[3]->ptr+i*gendata[3]->size+j)= 
                        interp_z(gendata[a],
                            ((float)(j)+deltaf_x_a),
                            ((float)(i)+deltaf_y_a))
                            -                    
                            interp_z(gendata[b],
                            ((float)(j)+deltaf_x_b),
                            ((float)(i)+deltaf_y_b));
                            
                    }
            }
            else
            {
                for (i=0;i<gendata[3]->size;i++)
                    for(j=0;j<gendata[3]->size;j++)
                    {
                        int x_a,x_b;
                        
                        x_a = (i+delta_y_a)*gendata[a]->size+j+delta_x_a;
                        x_b = (i+delta_y_b)*gendata[b]->size+j+delta_x_b;
                            
                        *(gendata[3]->ptr+i*gendata[3]->size+j)= 
                         *(gendata[a]->ptr+x_a)-
                        *(gendata[b]->ptr+x_b);
                    }            
            }
                
            find_min_max(gendata[3],&min_z,&max_z);
//            linearize(3);
            unequalize(gendata[3]);
            break;
        case DATATYPE_GEN2D:
            switch(gendata[a]->type2d)
            {
                case TYPE2D_SPEC_I:
                case TYPE2D_SPEC_Z:
                case TYPE2D_SPEC_2:
                case TYPE2D_SPEC_3:
                case TYPE2D_MASSPEC:
                    data_a=create_single_step(gendata[a]);
                    data_b=create_single_step(gendata[b]);
                    copy_data(&gendata[3],&data_a);    
                    start=max(data_a->min_x,data_b->min_x);
                    step=max(data_a->step,data_b->step);
                    size=(min(data_a->max_x,data_b->max_x)-start)/step+1;
                    free_data_ptrs(&gendata[3]);
                    alloc_data_ptrs(&gendata[3],DATATYPE_GEN2D,size,GEN2D_NONE,GEN2D_FLOAT,0);
                    
                    gendata[3]->start=gendata[3]->min_x=start;
                    gendata[3]->step=step;
                    gendata[3]->max_x=gendata[3]->start+
                        (gendata[3]->size-1)*gendata[3]->step;
/*
sprintf(string,"size %d start %d max_x %f step %d",size,start,gendata[3]->max_x,step);
mprintf(string);
*/
                    for(i=start;i<=gendata[3]->max_x;i+=step)
                    {
                        int x_a,x_b;
                        int dest;
                        
                        x_a = (int)((i-data_a->min_x)/data_a->step);
                        x_b = (int)((i-data_b->min_x)/data_b->step);
                        if (x_a<0 || x_a>=data_a->size)
                            mprintf("problem a");
                            
                        if (x_b<0 || x_b>=data_b->size)
                            mprintf("problem b");
                            
                        dest = (i-start)/step;
                        if (dest<0 || dest>=gendata[3]->size)
                            mprintf("problem dest");                        

                        *(gendata[3]->yf+dest)=
                            *(data_a->yf+x_a)-
                            *(data_b->yf+x_b)+
                            IN_ZERO;
                    }
/*
sprintf(string,"end %d a:start %d a:end %d",(int)(gendata[3]->max_x-start)/step,
    (int)(start-data_a->min_x)/data_a->step,(int)(gendata[3]->max_x-data_a->min_x)/
    data_a->step);
mprintf(string);
*/
                    find_min_max(gendata[3],&min_z,&max_z);
                    free_data(&data_a);
                    free_data(&data_b);
                    break;
            }
            break;
    }
    
}

void average_data(int a,int b,float sub_x,float sub_y,float sub_z) 
/* calculate a-b and puts it in 4 */
{                           
    int i,j;
    float min_z,max_z;
    int start,step,size;
    int delta_x_a, delta_x_b,delta_y_a,delta_y_b;
    datadef *data_a,*data_b;
    int a_left, a_right, a_top, a_bottom;
    int b_left, b_right, b_top, b_bottom;
    int vert_size, horz_size;   

    if (gendata[a]->type!=gendata[b]->type) return;
    switch(gendata[a]->type)
    {
        case DATATYPE_3D:
            copy_data(&gendata[3],&gendata[a]);    
            delta_x_a=delta_x_b=delta_y_a=delta_y_b=0;

	    // for the moment, floor all sub_x and sub_y
	    sub_x = floor(sub_x);
	    sub_y = floor(sub_y);

// sizing

	    a_left = 0;
	    a_bottom = 0;
	    b_left = sub_x;
	    b_bottom = sub_y;
	    a_right = gendata[a]->size;
	    a_top = gendata[a]->size;
	    b_right = sub_x + gendata[b]->size;
	    b_top = sub_y + gendata[b]->size;

	    vert_size = min( a_top, b_top) - max( a_bottom, b_bottom);
	    horz_size = min( a_right, b_right) - max( a_left, b_left);
	    gendata[3]->size = min( vert_size, horz_size);

#ifdef OLD // sizing
            if (abs(sub_x)>=abs(sub_y))
            {
                gendata[3]->size=gendata[a]->size-abs(sub_x);
            }
            else
            {
                gendata[3]->size=gendata[a]->size-abs(sub_y);
            }
#endif

            if (sub_x>0)
            {
                delta_x_a=sub_x;
                delta_x_b=0;
            }
            else if (sub_x<0)
            {
                delta_x_a=0;
                delta_x_b=-sub_x;
            }
            if (sub_y>0)
            {
                delta_y_a=sub_y;
                delta_y_b=0;
            }
            else if (sub_y<0)
            {
                delta_y_a=0;
                delta_y_b=-sub_y;
            }
/*
sprintf(string,"%d %d %d %d",delta_x_a,delta_x_b,delta_y_a,delta_y_b);
mprintf(string);                
*/
            
            for (i=0;i<gendata[3]->size;i++)
                for(j=0;j<gendata[3]->size;j++)
                {
	            int x_a,x_b;
                       
                    x_a = (i+delta_y_a)*gendata[a]->size+j+delta_x_a;
                    x_b = (i+delta_y_b)*gendata[b]->size+j+delta_x_b;
                            
                    *(gendata[3]->ptr+i*gendata[3]->size+j) = (
                    	*(gendata[a]->ptr+x_a) + *(gendata[b]->ptr+x_b)
		    ) / 2;
                }            
            find_min_max(gendata[3],&min_z,&max_z);
//            linearize(3);
            unequalize(gendata[3]);
            break;
        case DATATYPE_GEN2D:
            switch(gendata[a]->type2d)
            {
                case TYPE2D_SPEC_I:
                case TYPE2D_SPEC_Z:
                case TYPE2D_SPEC_2:
                case TYPE2D_SPEC_3:
                case TYPE2D_MASSPEC:
                    data_a=create_single_step(gendata[a]);
                    data_b=create_single_step(gendata[b]);
                    copy_data(&gendata[3],&data_a);    
                    start=max(data_a->min_x,data_b->min_x);
                    step=max(data_a->step,data_b->step);
                    size=(min(data_a->max_x,data_b->max_x)-start)/step+1;
                    free_data_ptrs(&gendata[3]);
                    alloc_data_ptrs(&gendata[3],DATATYPE_GEN2D,size,GEN2D_NONE,GEN2D_FLOAT,0);
                    
                    gendata[3]->start=gendata[3]->min_x=start;
                    gendata[3]->step=step;
                    gendata[3]->num_passes=data_a->num_passes+data_b->num_passes;
                    gendata[3]->max_x=gendata[3]->start+
                        (gendata[3]->size-1)*gendata[3]->step;
                    for(i=start;i<=gendata[3]->max_x;i+=step)
                    {
                        *(gendata[3]->yf+(i-start)/step)=
                            (*(data_a->yf+(int)((i-data_a->min_x)/data_a->step))*data_a->num_passes+
                            *(data_b->yf+(int)((i-data_b->min_x)/data_b->step))*data_b->num_passes)
                            /(data_a->num_passes+data_b->num_passes);
                    }
                    find_min_max(gendata[3],&min_z,&max_z);
                    free_data(&data_a);
                    free_data(&data_b);
                    break;
            }
            break;
    }
    
}
void didv(int a,int b,float sub_x,float sub_y,float sub_z) 
/* calculate dI/dV/(I/V) and puts it in 4 */
{                           
    int i;
    float min_z,max_z;
    int start,step,size;
    datadef *data_a,*data_b;
    
    if (gendata[a]->type!=gendata[b]->type) return;
    switch(gendata[a]->type)
    {
        case DATATYPE_3D:
            break;
        case DATATYPE_GEN2D:
            switch(gendata[a]->type2d)
            {
                case TYPE2D_SPEC_I:
                case TYPE2D_SPEC_Z:
                case TYPE2D_SPEC_2:
                case TYPE2D_SPEC_3:
                case TYPE2D_MASSPEC:
                    data_a=create_single_step(gendata[a]);
                    data_b=create_single_step(gendata[b]);
                    copy_data(&gendata[3],&data_a);    
                    start=max(data_a->min_x,data_b->min_x);
                    step=max(data_a->step,data_b->step);
                    size=(min(data_a->max_x,data_b->max_x)-start)/step+1;
                    free_data_ptrs(&gendata[3]);
                    alloc_data_ptrs(&gendata[3],DATATYPE_GEN2D,size,GEN2D_NONE,GEN2D_FLOAT,0);
                    
                    gendata[3]->start=gendata[3]->min_x=start;
                    gendata[3]->step=step;
                    gendata[3]->max_x=gendata[3]->start+
                        (gendata[3]->size-1)*gendata[3]->step;
                    for(i=start;i<=gendata[3]->max_x;i+=step)
                    {
                        if (*(data_b->yf+(int)((i-data_b->min_x)/data_b->step))==IN_ZERO)
                            *(gendata[3]->yf+(i-start)/step)=IN_ZERO;
                        else *(gendata[3]->yf+(i-start)/step)=
                            in_dtov(*(data_a->yf+(int)((i-data_a->min_x)/data_a->step)))/
                            in_dtov(*(data_b->yf+(int)((i-data_b->min_x)/data_b->step)))*
                            dtov(i,data_b->v_range)*IN_MAX/2+
                            IN_ZERO;
                    }
                    find_min_max(gendata[3],&min_z,&max_z);
                    free_data(&data_a);
                    free_data(&data_b);
                    break;
            }
            break;
    }
    
}

static datadef *create_single_step(datadef *this_data)
{
    datadef * result=NULL;
    int i,j;
    
    copy_data(&result,&this_data);
    if (result->step!=1)
    {
        free_data_ptrs(&result);
        result->step=1;
        result->size=(result->size-1)*(this_data->step)+1;
        alloc_data_ptrs(&result,DATATYPE_GEN2D,result->size,GEN2D_NONE,GEN2D_FLOAT,0);
        for(i=0;i<this_data->size-1;i++)
        {
            result->yf[i*(this_data->step)]=this_data->yf[i];
            for(j=1;j<this_data->step;j++)
            {
                result->yf[i*(this_data->step)+j]=this_data->yf[i]+
                    (this_data->yf[i+1]-this_data->yf[i])/(this_data->step)*j;
            }
        }
        result->yf[(this_data->size-1)*(this_data->step)]=
            this_data->yf[this_data->size-1];
    }
    return result;
}
 
static void average_1_data(int num,float x)
{
    int i,size;
    int fraction;
    double delta_t,avg_vt;
    float min_z,max_z;
    int n=x;
    
    if (gendata[num]->valid)
    {
        switch(gendata[num]->type)
        {
            case DATATYPE_2D:
                copy_data(&gendata[3],&gendata[num]);
                free_data_ptrs(&gendata[3]);
                size=gendata[num]->size/n;
                fraction=gendata[num]->size % n;
                if (fraction) size++;
                alloc_data_ptrs(&gendata[3],DATATYPE_2D,size,GEN2D_NONE,GEN2D_NONE,0);
                avg_vt=0;
                for(i=1;i<gendata[num]->size;i++)
                {
                    delta_t=gendata[num]->time2d[i]-gendata[num]->time2d[i-1];
                    avg_vt+=(gendata[num]->data2d[i]+
                            gendata[num]->data2d[i-1])*delta_t/2;
                    if (!(i%n))
                    {
                        delta_t=gendata[num]->time2d[i]-gendata[num]->time2d[i-n];
                        gendata[3]->time2d[i/n-1]=gendata[num]->time2d[i-n]+delta_t/2;
                        gendata[3]->data2d[i/n-1]=avg_vt/delta_t;
                        avg_vt=0;
                    }
                }
                if (fraction)
                {
                    delta_t=gendata[num]->time2d[gendata[num]->size-1]-
                        gendata[num]->time2d[gendata[num]->size-1-fraction];
                    gendata[3]->time2d[size-1]=gendata[num]->time2d[gendata[num]->size-1-fraction]+
                        delta_t/2;
                    gendata[3]->data2d[size-1]=avg_vt/delta_t;
                }
                find_min_max(gendata[3],&min_z,&max_z);
                break;
        }
    }
}

static void smooth_1_data(int num,float x)
{
    int i;
    double delta_t,avg_vt;
    float min_z,max_z;
    int n=x;
    
    if (gendata[num]->valid)
    {
        switch(gendata[num]->type)
        {
            case DATATYPE_2D:
                copy_data(&gendata[3],&gendata[num]);
                avg_vt=0;
                for(i=1;i<=n;i++)
                {
                    delta_t=gendata[num]->time2d[i]-gendata[num]->time2d[i-1];
                    avg_vt+=(gendata[num]->data2d[i]+
                            gendata[num]->data2d[i-1])*delta_t/2;
                    delta_t=gendata[num]->time2d[i]-gendata[num]->time2d[0];
                    gendata[3]->time2d[i]=gendata[num]->time2d[0]+delta_t/2;
                    gendata[3]->data2d[i]=avg_vt/delta_t;
                }
                
                for(i=n+1;i<gendata[num]->size;i++)
                {
                    delta_t=gendata[num]->time2d[i-n]-gendata[num]->time2d[i-n-1];
                    avg_vt-=(gendata[num]->data2d[i-n]+
                            gendata[num]->data2d[i-n-1])*delta_t/2;
                    delta_t=gendata[num]->time2d[i]-gendata[num]->time2d[i-1];
                    avg_vt+=(gendata[num]->data2d[i]+
                            gendata[num]->data2d[i-1])*delta_t/2;
                    delta_t=gendata[num]->time2d[i]-gendata[num]->time2d[i-n];
                    gendata[3]->time2d[i]=gendata[num]->time2d[i-n]+delta_t/2;
                    gendata[3]->data2d[i]=avg_vt/delta_t;
                }
                find_min_max(gendata[3],&min_z,&max_z);
                break;
        }
    }
}
static void scale_data(int num,float factor)
{
    int i,size;
    float min_z,max_z;
    
    if (gendata[num]->valid)
    {
        switch(gendata[num]->type)
        {
            case DATATYPE_GEN2D:
                switch(gendata[num]->type2d)
                {
                    case TYPE2D_SPEC_I:
                    case TYPE2D_SPEC_Z:
                    case TYPE2D_SPEC_2:
                    case TYPE2D_SPEC_3:
                    case TYPE2D_MASSPEC:
                    copy_data(&gendata[3],&gendata[num]);
                    for(i=0;i<gendata[num]->size;i++)
                    {
                        gendata[3]->yf[i]=(gendata[3]->yf[i]-IN_ZERO)*factor
                            +IN_ZERO;
                    }
                    break;
                }
                find_min_max(gendata[3],&min_z,&max_z);
                break;
        }
    }
}
static float find_integral_gen2d(int num,int start,int end)
{
    int i;
    float sum=0;
    
    for(i=start;i<=end;i++)
        sum+=in_dtov(gendata[num]->yf[i]);
        
    sum*=dtov_len(gendata[num]->step,gendata[num]->v_range);
    return sum;
}


static void integrate_data(HWND hDlg,int num)
{
    int size;
    float sum;
    int start,end;
    
    if (gendata[num]->valid)
    {
        switch(gendata[num]->type)
        {
            case DATATYPE_GEN2D:
                switch(gendata[num]->type2d)
                {
                    case TYPE2D_SPEC_I:
                    case TYPE2D_SPEC_Z:
                    case TYPE2D_SPEC_2:
                    case TYPE2D_SPEC_3:
                    case TYPE2D_MASSPEC:
                        start=0;
                        end=gendata[num]->size-1;
                        if (SelOn && constrain)
                        {
                            if (sel_r2.pt1->x>=0 && sel_r2.image==num
                                && sel_r1.pt1->x>=0 && sel_r1.image==num)
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
                            else if (sel_r1.pt1->x>=0 && sel_r1.image==num)
                            {
                                start=sel_r1.pt1->x;
                            }
                            else if (sel_r2.pt1->x>=0 && sel_r2.image==num)
                            {
                                end=sel_r2.pt1->x;
                            }
                        }
                        sum=find_integral_gen2d(num,start,end);
                        sprintf(string,"%f",sum);
                        SetDlgItemText(hDlg,SUB_Z,string);
                        break;
                }
                break;
        }
    }
}
                    
static void repaint_parameters(HWND hDlg)
{
    switch(sub_mode)
    {
        case SUB_SUBTRACT:
            SetDlgItemText(hDlg,SUB_PARAM1,"X Offset");
            SetDlgItemText(hDlg,SUB_PARAM2,"Y Offset");
            SetDlgItemText(hDlg,SUB_PARAM3,"");
            break;
        case SUB_DIVIDE:
            SetDlgItemText(hDlg,SUB_PARAM1,"");
            SetDlgItemText(hDlg,SUB_PARAM2,"");
            SetDlgItemText(hDlg,SUB_PARAM3,"");
            break;
        case SUB_AVERAGE:
            SetDlgItemText(hDlg,SUB_PARAM1,"X Off/Bin");
            SetDlgItemText(hDlg,SUB_PARAM2,"Y Offset");
            SetDlgItemText(hDlg,SUB_PARAM3,"");
            break;
        case SUB_DIDV:
            SetDlgItemText(hDlg,SUB_PARAM1,"");
            SetDlgItemText(hDlg,SUB_PARAM2,"");
            SetDlgItemText(hDlg,SUB_PARAM3,"");
            break;
        case SUB_SMOOTH:
            SetDlgItemText(hDlg,SUB_PARAM1,"Bin");
            SetDlgItemText(hDlg,SUB_PARAM2,"");
            SetDlgItemText(hDlg,SUB_PARAM3,"");
            break;
        case SUB_INTEGRATE:
            SetDlgItemText(hDlg,SUB_PARAM1,"");
            SetDlgItemText(hDlg,SUB_PARAM2,"");
            SetDlgItemText(hDlg,SUB_PARAM3,"Result");
            break;
        case SUB_SCALE:
            SetDlgItemText(hDlg,SUB_PARAM1,"Multiplier");
            SetDlgItemText(hDlg,SUB_PARAM2,"");
            SetDlgItemText(hDlg,SUB_PARAM3,"");
            break;
    }
}

void decide(HWND hDlg,int current,int button)
{
    if (current!=button)                     
    { 
        switch(sub_mode)
        {
            case SUB_SUBTRACT:
                if (current!=3 && button!=3)
                    subtract_data(current,button,sub_x,sub_y,sub_z);         
                reset_image(ImgDlg,3);
                break;
                    
            case SUB_DIDV:
                if (current!=3 && button!=3)
                    didv(current,button,sub_x,sub_y,sub_z);
                reset_image(ImgDlg,3);
                break;
            case SUB_AVERAGE:
                if (current!=3 && button!=3)
                    average_data(current,button,sub_x,sub_y,sub_z);
                reset_image(ImgDlg,3);
                break;
        }
    }
    else
    {
        switch(sub_mode)
        {
            case SUB_INTEGRATE:
                integrate_data(hDlg,current);
                break;
            case SUB_SCALE:
                if (current!=3) scale_data(current,sub_x);
                reset_image(ImgDlg,3);
                break;
            case SUB_SMOOTH:
                if (sub_x>1 && current!=3) smooth_1_data(current,sub_x);
                reset_image(ImgDlg,3);
                break;
                    
            case SUB_AVERAGE:
                if (sub_x>1 && current!=3) average_1_data(current,sub_x);
                reset_image(ImgDlg,3);
                break;
        }
    }
}
