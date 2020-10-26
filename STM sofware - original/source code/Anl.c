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
#include "anl.h"

HWND AnlWnd=NULL;
BOOL AnlOn=FALSE;
extern HWND ImgDlg;
extern BOOL SelOn;

extern datadef *gendata[];
extern int current_image;
extern BITMAPINFO *bitmapinfo;
extern LOGPALETTE *pLogPal;
extern HPALETTE hPal;
extern HANDLE hInst;
extern char string[];
extern int zoomed_in;
extern int im_src_x[],im_src_y[],im_screenlines[];
extern unsigned char *im_bitmaps[];
extern int sel1_on,sel2_on;
extern int last_button;
extern int constrain;
extern int sel_type;
extern SEL_POINT sel_p1,sel_p2,sel_p3,sel_p4;
extern SEL_REGION sel_r1,sel_r2;
extern int cur_sel;
extern double a_per_v,z_a_per_v;

void repaint_sel_delta1(HWND);
void repaint_sel_delta2(HWND);
void repaint_2points(HWND);
void repaint_avg_z1(HWND);
void repaint_avg_z2(HWND);

BOOL FAR PASCAL AnlDlg(HWND,unsigned,WPARAM,LPARAM);

BOOL FAR PASCAL AnlDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{

/*
  int i;
  int id;
  int delt,ddelt,mini,maxi;
*/  

  switch(Message) {
    case WM_INITDIALOG:
/*
      sprintf(string,"%.2lf",gamma);
      SetDlgItemText(hDlg,GAMMA_VALUE,string);
      sprintf(string,"%d",anim_delay);
      SetDlgItemText(hDlg,ANIMATE_DELAY,string);
*/
      if(SelOn) repaint_sel_delta1(hDlg);
      if(SelOn) repaint_sel_delta2(hDlg);
      break;

    case WM_HSCROLL:
    case WM_VSCROLL:
/*
      id = getscrollid();
      switch(id) {      

        case IM_X_SCROLL:
          i = (int) im_src_x[current_image];
          mini = 0;
          maxi = gendata[current_image]->size-im_screenlines[current_image];
          delt = IM_SCROLL_DELT;
          ddelt = IM_SCROLL_DDELT;
          break;
        case IM_Y_SCROLL:
          i = (int) im_src_y[current_image];
          mini = 0;
          maxi = gendata[current_image]->size-im_screenlines[current_image];
          delt = -IM_SCROLL_DELT;
          ddelt = -IM_SCROLL_DDELT;
          break;
	  }

      switch(getscrollcode())
      {
        case SB_LINELEFT:
          i -= delt;
          break;
        case SB_LINERIGHT:
          i += delt;
          break;
        case SB_PAGELEFT:
          i -= ddelt;
          break;
        case SB_PAGERIGHT:
          i += ddelt;
          break;
        case SB_THUMBPOSITION:
          i = getscrollpos();
          break;
        case SB_THUMBTRACK:
          i = getscrollpos();
          break;
        default:
          id = -1;
          break;
      }
      i = min(max(i,mini),maxi);
      switch(id)  {

        case IM_X_SCROLL:
          im_src_x[current_image] = (int) i;
          SetScrollPos(GetDlgItem(hDlg,IM_X_SCROLL),SB_CTL,im_src_x[current_image],TRUE);
          repaint_image(hDlg,current_image,NULL);
          break;
        case IM_Y_SCROLL:
          im_src_y[current_image] = (int) i;
          SetScrollPos(GetDlgItem(hDlg,IM_Y_SCROLL),SB_CTL,
            maxi-i,TRUE);
          repaint_image(hDlg,current_image,NULL);
          break;

      }
*/
      break;

	case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case ANL_AVG_Z1:
          repaint_avg_z1(hDlg);
          break;
        case ANL_AVG_Z2:
          repaint_avg_z2(hDlg);
          break;
        case SEL_REPAINT_L1:
        case SEL_REPAINT_P1:
          repaint_sel_delta1(hDlg);
          break;
        case SEL_REPAINT_L2:
        case SEL_REPAINT_P2:
          repaint_sel_delta2(hDlg);
          break;
        case ANL_EXIT:
/*
                PostMessage(ImgDlg,WM_COMMAND,IM_ANL_EXIT,0);
*/
          DestroyWindow(hDlg);
          AnlWnd = NULL;
          AnlOn = FALSE;
          break;
	  }
      break;
  }
  return(FALSE);
}

void repaint_sel_delta1(HWND hDlg)
{
	// Repaints the top half of the analysis functions dialog.
	// If both selection 1 & 2 are points, the top half of the 
	// dialog refers to the difference between the two selections.
	// If selection 1 is a line or square, the top half of the
	// dialog refers to the difference between the enpoints or
	// corners, respectively.

	float x_volts,y_volts;
    
    if(sel_r1.type == SEL_POINTS && sel_r2.type == SEL_POINTS)
        repaint_2points(hDlg);
    else
    if (sel_r1.pt2->x>=0 && sel_r1.pt2->y>=0 
        && sel_r1.pt1->x>=0 && sel_r1.pt1->y>=0 && sel_r1.type!=SEL_CIRCLE)
    {
        sprintf_s(string,100,"%d",sel_r1.pt2->x-sel_r1.pt1->x);
        SetDlgItemText(hDlg,ANL_DEL1_X_INT,string); 
        sprintf(string,"%d",sel_r1.pt2->y-sel_r1.pt1->y);
        SetDlgItemText(hDlg,ANL_DEL1_Y_INT,string); 
        sprintf(string,"%f",sel_r1.pt2->z-sel_r1.pt1->z);
        SetDlgItemText(hDlg,ANL_DEL1_Z_FLOAT,string); 
        x_volts=-dtov(gendata[sel_r1.image]->x,2)*10-
            dtov(ZERO+(sel_r1.pt1->x-gendata[sel_r1.image]->size/2)*
            (gendata[sel_r1.image]->step),gendata[sel_r1.image]->
            x_range)*gendata[sel_r1.image]->x_gain +
            dtov(gendata[sel_r1.image]->x,2)*10+
            dtov(ZERO+(sel_r1.pt2->x-gendata[sel_r1.image]->size/2)*
            (gendata[sel_r1.image]->step),gendata[sel_r1.image]->
            x_range)*gendata[sel_r1.image]->x_gain;
        sprintf(string,"%f",x_volts);
        SetDlgItemText(hDlg,ANL_DEL1_X_VOLTS,string); 
        y_volts=-dtov(gendata[sel_r1.image]->y,2)*10 -
            dtov(ZERO+(sel_r1.pt1->y-gendata[sel_r1.image]->size/2)*
            (gendata[sel_r1.image]->step),gendata[sel_r1.image]->
            y_range)*gendata[sel_r1.image]->y_gain +
            dtov(gendata[sel_r1.image]->y,2)*10+
            dtov(ZERO+(sel_r1.pt2->y-gendata[sel_r1.image]->size/2)*
            (gendata[sel_r1.image]->step),gendata[sel_r1.image]->
            y_range)*gendata[sel_r1.image]->y_gain;
        sprintf(string,"%f",y_volts);
        SetDlgItemText(hDlg,ANL_DEL1_Y_VOLTS,string); 
        sprintf(string,"%f",in_dtov(sel_r1.pt2->z)-in_dtov(sel_r1.pt1->z));
        SetDlgItemText(hDlg,ANL_DEL1_Z_VOLTS,string); 
        sprintf(string,"%f",x_volts*A_PER_V);
        SetDlgItemText(hDlg,ANL_X1_ANGS,string); 
        sprintf(string,"%f",y_volts*A_PER_V);
        SetDlgItemText(hDlg,ANL_Y1_ANGS,string); 
        sprintf(string,"%f",sqrt(x_volts*x_volts+y_volts*y_volts));
        SetDlgItemText(hDlg,ANL_DEL1_DIST_VOLTS,string); 
        sprintf(string,"%f",sqrt(x_volts*x_volts+y_volts*y_volts)*A_PER_V);
        SetDlgItemText(hDlg,ANL_DIST1_ANGS,string); 
        
    }
    else
    {
        SetDlgItemText(hDlg,ANL_DEL1_X_INT,""); 
        SetDlgItemText(hDlg,ANL_DEL1_Y_INT,""); 
        SetDlgItemText(hDlg,ANL_DEL1_Z_FLOAT,""); 
        SetDlgItemText(hDlg,ANL_DEL1_X_VOLTS,""); 
        SetDlgItemText(hDlg,ANL_DEL1_Y_VOLTS,""); 
        SetDlgItemText(hDlg,ANL_DEL1_Z_VOLTS,""); 
        SetDlgItemText(hDlg,ANL_X1_ANGS,""); 
        SetDlgItemText(hDlg,ANL_Y1_ANGS,""); 
        SetDlgItemText(hDlg,ANL_DEL1_DIST_VOLTS,""); 
        SetDlgItemText(hDlg,ANL_DIST1_ANGS,""); 
        
    }
}

void repaint_sel_delta2(HWND hDlg)
{
        
    if (sel_r1.type==SEL_POINTS && sel_r1.type==SEL_POINTS)
        repaint_2points(hDlg);
    else
    if (sel_r2.pt2->x>=0 && sel_r2.pt2->y>=0 
        && sel_r2.pt1->x>=0 && sel_r2.pt1->y>=0 && sel_r1.type!=SEL_CIRCLE)
    {
        sprintf(string,"%d",sel_r2.pt2->x-sel_r2.pt1->x);
        SetDlgItemText(hDlg,ANL_DEL2_X_INT,string); 
        sprintf(string,"%d",sel_r2.pt2->y-sel_r2.pt1->y);
        SetDlgItemText(hDlg,ANL_DEL2_Y_INT,string); 
        sprintf(string,"%f",sel_r2.pt2->z-sel_r2.pt1->z);
        SetDlgItemText(hDlg,ANL_DEL2_Z_FLOAT,string); 
        sprintf(string,"%f",-dtov(gendata[sel_r2.image]->x,2)*10-
            dtov(ZERO+(sel_r2.pt1->x-gendata[sel_r2.image]->size/2)*
            (gendata[sel_r2.image]->step),gendata[sel_r2.image]->
            x_range)*gendata[sel_r2.image]->x_gain +
            dtov(gendata[sel_r2.image]->x,2)*10+
            dtov(ZERO+(sel_r2.pt2->x-gendata[sel_r2.image]->size/2)*
            (gendata[sel_r2.image]->step),gendata[sel_r2.image]->
            x_range)*gendata[sel_r2.image]->x_gain            );
        SetDlgItemText(hDlg,ANL_DEL2_X_VOLTS,string); 
        sprintf(string,"%f",-dtov(gendata[sel_r2.image]->y,2)*10 -
            dtov(ZERO+(sel_r2.pt1->y-gendata[sel_r2.image]->size/2)*
            (gendata[sel_r2.image]->step),gendata[sel_r2.image]->
            y_range)*gendata[sel_r2.image]->y_gain +
            dtov(gendata[sel_r2.image]->y,2)*10+
            dtov(ZERO+(sel_r2.pt2->y-gendata[sel_r2.image]->size/2)*
            (gendata[sel_r2.image]->step),gendata[sel_r2.image]->
            y_range)*gendata[sel_r2.image]->y_gain );
        SetDlgItemText(hDlg,ANL_DEL2_Y_VOLTS,string); 
        sprintf(string,"%f",in_dtov(sel_r2.pt2->z)-in_dtov(sel_r2.pt1->z));
        SetDlgItemText(hDlg,ANL_DEL2_Z_VOLTS,string); 
    }
    else
    {
        SetDlgItemText(hDlg,ANL_DEL2_X_INT,""); 
        SetDlgItemText(hDlg,ANL_DEL2_Y_INT,""); 
        SetDlgItemText(hDlg,ANL_DEL2_Z_FLOAT,""); 
        SetDlgItemText(hDlg,ANL_DEL2_X_VOLTS,""); 
        SetDlgItemText(hDlg,ANL_DEL2_Y_VOLTS,""); 
        SetDlgItemText(hDlg,ANL_DEL2_Z_VOLTS,""); 
        
    }
}

void repaint_2points(HWND hDlg)
{
    int y1,y2;
    float x_volts,y_volts;
    if (sel_r2.pt1->x>=0 && sel_r2.pt1->y>=0 
        && sel_r1.pt1->x>=0 && sel_r1.pt1->y>=0
        && gendata[sel_r1.image]->type==gendata[sel_r2.image]->type)
    {
        switch(gendata[sel_r1.image]->type)
        {
            case DATATYPE_3D:
                sprintf(string,"%f",in_dtov(sel_r2.pt1->z)-in_dtov(sel_r1.pt1->z));
                SetDlgItemText(hDlg,ANL_DEL1_Z_VOLTS,string); 
                sprintf(string,"%d",sel_r2.pt1->x-sel_r1.pt1->x);
                SetDlgItemText(hDlg,ANL_DEL1_X_INT,string); 
                sprintf(string,"%d",sel_r2.pt1->y-sel_r1.pt1->y);
                SetDlgItemText(hDlg,ANL_DEL1_Y_INT,string); 
                sprintf(string,"%f",sel_r2.pt1->z-sel_r1.pt1->z);
                SetDlgItemText(hDlg,ANL_DEL1_Z_FLOAT,string); 
                x_volts=-dtov(gendata[sel_r1.image]->x,2)*10-
                    dtov(ZERO+(sel_r1.pt1->x-gendata[sel_r1.image]->size/2)*
                    (gendata[sel_r1.image]->step),gendata[sel_r1.image]->
                    x_range)*gendata[sel_r1.image]->x_gain +
                    dtov(gendata[sel_r2.image]->x,2)*10+
                    dtov(ZERO+(sel_r2.pt1->x-gendata[sel_r2.image]->size/2)*
                    (gendata[sel_r2.image]->step),gendata[sel_r2.image]->
                    x_range)*gendata[sel_r2.image]->x_gain;
                sprintf(string,"%f",x_volts);
                SetDlgItemText(hDlg,ANL_DEL1_X_VOLTS,string); 
                y_volts=-dtov(gendata[sel_r1.image]->y,2)*10 -
                    dtov(ZERO+(sel_r1.pt1->y-gendata[sel_r1.image]->size/2)*
                    (gendata[sel_r1.image]->step),gendata[sel_r1.image]->
                    y_range)*gendata[sel_r1.image]->y_gain +
                    dtov(gendata[sel_r1.image]->y,2)*10+
                    dtov(ZERO+(sel_r2.pt1->y-gendata[sel_r2.image]->size/2)*
                    (gendata[sel_r2.image]->step),gendata[sel_r2.image]->
                    y_range)*gendata[sel_r2.image]->y_gain;
                sprintf(string,"%f",y_volts);
                SetDlgItemText(hDlg,ANL_DEL1_Y_VOLTS,string); 
                sprintf(string,"%f",x_volts*A_PER_V);
                SetDlgItemText(hDlg,ANL_X1_ANGS,string); 
                sprintf(string,"%f",y_volts*A_PER_V);
                SetDlgItemText(hDlg,ANL_Y1_ANGS,string); 
                sprintf(string,"%f",sqrt(x_volts*x_volts+y_volts*y_volts));
                SetDlgItemText(hDlg,ANL_DEL1_DIST_VOLTS,string); 
                sprintf(string,"%f",sqrt(x_volts*x_volts+y_volts*y_volts)*A_PER_V);
                SetDlgItemText(hDlg,ANL_DIST1_ANGS,string); 
                break;
            case DATATYPE_2D:
                sprintf(string,"%d",sel_r2.pt1->x-sel_r1.pt1->x);
                SetDlgItemText(hDlg,ANL_DEL1_X_INT,string); 
                sprintf(string,"%f",sel_r2.pt1->t-sel_r1.pt1->t);
                SetDlgItemText(hDlg,ANL_DEL1_Y_INT,string); 
                sprintf(string,"%f",sel_r2.pt1->z-sel_r1.pt1->z);
                SetDlgItemText(hDlg,ANL_DEL1_Z_FLOAT,string); 
                SetDlgItemText(hDlg,ANL_DEL1_X_VOLTS,""); 
                SetDlgItemText(hDlg,ANL_DEL1_Y_VOLTS,""); 
                sprintf(string,"%f",in_dtov(sel_r2.pt1->z)-in_dtov(sel_r1.pt1->z));
                SetDlgItemText(hDlg,ANL_DEL1_Z_VOLTS,string); 
                break;
            case DATATYPE_GEN2D:
                y1=gendata[sel_r1.image]->start+
                        sel_r1.pt1->x*gendata[sel_r1.image]->step;
                y2=gendata[sel_r2.image]->start+
                        sel_r2.pt1->x*gendata[sel_r2.image]->step;
                sprintf(string,"%d",sel_r2.pt1->x-sel_r1.pt1->x);
                SetDlgItemText(hDlg,ANL_DEL1_X_INT,string); 
                sprintf(string,"%d",y2-y1);
                SetDlgItemText(hDlg,ANL_DEL1_Y_INT,string); 
                sprintf(string,"%f",sel_r2.pt1->z-sel_r1.pt1->z);
                SetDlgItemText(hDlg,ANL_DEL1_Z_FLOAT,string); 
                SetDlgItemText(hDlg,ANL_DEL1_X_VOLTS,""); 
                sprintf(string,"%f",dtov(y2,gendata[sel_r2.image]->bias_range)-
                    dtov(y1,gendata[sel_r1.image]->bias_range));
                SetDlgItemText(hDlg,ANL_DEL1_Y_VOLTS,string); 
                sprintf(string,"%f",in_dtov(sel_r2.pt1->z)-in_dtov(sel_r1.pt1->z));
                SetDlgItemText(hDlg,ANL_DEL1_Z_VOLTS,string); 
                break;
        }
    }
    else
    {
        SetDlgItemText(hDlg,ANL_DEL1_X_INT,""); 
        SetDlgItemText(hDlg,ANL_DEL1_Y_INT,""); 
        SetDlgItemText(hDlg,ANL_DEL1_Z_FLOAT,""); 
        SetDlgItemText(hDlg,ANL_DEL1_X_VOLTS,""); 
        SetDlgItemText(hDlg,ANL_DEL1_Y_VOLTS,""); 
        SetDlgItemText(hDlg,ANL_DEL1_Z_VOLTS,""); 
        
    }
    
}

float avg_z(SEL_REGION *region)
{
    int image=region->image;
    int i,j;
    int points=0;
    float z=0;
    
    if (region==NULL) return 0;
    if (gendata[image]->valid && gendata[image]->type==DATATYPE_3D)
    {
        for(i=0;i<gendata[image]->size;i++)
            for(j=0;j<gendata[image]->size;j++)
            {
                if (pt_in_region(i,j,region))
                {
                    z+=*(gendata[image]->ptr+j*gendata[image]->size+i);
                    points++;
                }
            }
        if (points) return z/(float)points;
        else return 0;
    }
    else return 0;
    
}

float avg_z_gen2d(SEL_REGION *region1, SEL_REGION *region2)
{
    int image=region1->image;
    int i;
    int points=0;
    int start,end;
    float z=0;
    
    if (region1==NULL) return 0;
    start=0;
    end=gendata[current_image]->size-1;
    if (region2->pt1->x>=0 && region2->image==region1->image
        && region1->pt1->x>=0)
    {
        if (region2->pt1->x>region1->pt1->x)
        {
            start=region1->pt1->x;
            end=region2->pt1->x;
        }
        else if (region2->pt1->x<region1->pt1->x)
        {
            start=region2->pt1->x;
            end=region1->pt1->x;
        }
    }
    else if (region1->pt1->x>=0 && region1->image==current_image)
    {
        start=region1->pt1->x;
        end=gendata[current_image]->size-1;
    }
    else if (region2->pt1->x>=0 && region2->image==current_image)
    {
        start=0;
        end=region2->pt1->x;
    }
    if (gendata[image]->valid && gendata[image]->type==DATATYPE_GEN2D)
    {
        for(i=start;i<=end;i++)
        {
            z+=*(gendata[image]->yf+i);
        }
        points=end-start+1;
        if (points) return z/(float)points;
        else return 0;
    }
    else return 0;
    
}

float avg_z_2d(SEL_REGION *region1, SEL_REGION *region2)
{
    int image=region1->image;
    int i;
    int points=0;
    int start,end;
    float z=0;
    
    if (region1==NULL) return 0;
    start=0;
    end=gendata[current_image]->size-1;
    if (region2->pt1->x>=0 && region2->image==region1->image
        && region1->pt1->x>=0)
    {
        if (region2->pt1->x>region1->pt1->x)
        {
            start=region1->pt1->x;
            end=region2->pt1->x;
        }
        else if (region2->pt1->x<region1->pt1->x)
        {
            start=region2->pt1->x;
            end=region1->pt1->x;
        }
    }
    else if (region1->pt1->x>=0 && region1->image==current_image)
    {
        start=region1->pt1->x;
        end=gendata[current_image]->size-1;
    }
    else if (region2->pt1->x>=0 && region2->image==current_image)
    {
        start=0;
        end=region2->pt1->x;
    }
    if (gendata[image]->valid && gendata[image]->type==DATATYPE_2D)
    {
        for(i=start;i<=end;i++)
        {
            z+=*(gendata[image]->data2d+i);
        }
        points=end-start+1;
        if (points) return z/(float)points;
        else return 0;
    }
    else return 0;
    
}

void repaint_avg_z1(HWND hDlg)
{
    float z=0;
    
    switch(gendata[current_image]->type)
    {
        case DATATYPE_3D:
            z=avg_z(&sel_r1);                
            break;
        case DATATYPE_2D:
            z=avg_z_2d(&sel_r1,&sel_r2);
            break;
        case DATATYPE_GEN2D:
            switch(gendata[current_image]->type2d)
            {
                case TYPE2D_SPEC_I:
                case TYPE2D_SPEC_Z:
                case TYPE2D_SPEC_2:
                case TYPE2D_SPEC_3:
                case TYPE2D_MASSPEC:
                    z=avg_z_gen2d(&sel_r1,&sel_r2);
                    break;
            }
            break;
    }
    sprintf(string,"%f",z);
    SetDlgItemText(hDlg,ANL_AVG_Z1_FLOAT,string);
    sprintf(string,"%f",in_dtov(z));
    SetDlgItemText(hDlg,ANL_AVG_Z1_VOLTS,string);
}

void repaint_avg_z2(HWND hDlg)
{
    float z=0;
    
    z=avg_z(&sel_r2);
    sprintf(string,"%f",z);
    SetDlgItemText(hDlg,ANL_AVG_Z2_FLOAT,string);
    sprintf(string,"%f",in_dtov(z));
    SetDlgItemText(hDlg,ANL_AVG_Z2_VOLTS,string);
}
