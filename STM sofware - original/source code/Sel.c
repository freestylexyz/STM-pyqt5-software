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

BOOL FAR PASCAL SelDlg(HWND,unsigned,WPARAM,LPARAM);

HWND SelWnd=NULL;
BOOL SelOn=FALSE;
//extern HWND VscaleDlg;
extern HWND VscaleWnd;
extern BOOL VscaleOn;
extern HWND ImgDlg;
extern HWND AnlWnd;
extern HBITMAP  hCross;
extern HBITMAP  hXcross;

extern datadef *gendata[];
extern int current_image;
extern BITMAPINFO *bitmapinfo;
extern LOGPALETTE              *pLogPal;
extern HPALETTE                hPal;
extern HANDLE hInst;
extern char     string[];
extern int zoomed_in;
extern int im_src_x[],im_src_y[],im_screenlines[];
extern unsigned char *im_bitmaps[];
extern int sel1_on,sel2_on;
extern int last_button;
extern int constrain;
extern BOOL AnlOn;
extern float image_lowert[],image_highert[],image_lowerv[],image_higherv[];
int sel_type=SEL_POINTS;
SEL_POINT sel_p1,sel_p2,sel_p3,sel_p4;
SEL_REGION sel_r1,sel_r2;
SEL_REGION *cur_sel_region;
static int sel_first_time=1;
static int out_smart=0;
int cur_sel=1;

void repaint_sel_change(HWND);
void repaint_constrain(HWND);

BOOL FAR PASCAL SelDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{
    int i;
    int id;
    int delt,ddelt,mini,maxi;
    int done;
    SEL_POINT tmp_pt1,tmp_pt2,tmp_pt3;
    int tmp_image;
    

  switch(Message) {
    case WM_INITDIALOG:
        if (sel_first_time)
        {
            sel_first_time=0;
            sel_p1.x=-1;
            sel_p1.y=-1;
            sel_p2.x=-1;
            sel_p2.y=-1;
            sel_p3.x=-1;
            sel_p3.y=-1;
            sel_p4.x=-1;
            sel_p4.y=-1;
            sel_r1.pt1=&sel_p1;
            sel_r1.pt2=&sel_p2;
            sel_r2.pt1=&sel_p3;
            sel_r2.pt2=&sel_p4;  
            sel_r1.image=0;
            sel_r2.image=0;
            sel_r1.flags=0;
            sel_r2.flags=SEL_SECONDSEL;
            sel_r1.type=SEL_POINTS;
            sel_r2.type=SEL_POINTS;
            cur_sel_region=&sel_r1;
            cur_sel=1;
        }
        repaint_sel_points(hDlg);
        repaint_sel_change(hDlg);
        repaint_constrain(hDlg);
        if (cur_sel==constrain) CheckDlgButton(hDlg, SEL_CONSTRAIN,1);
        else CheckDlgButton(hDlg, SEL_CONSTRAIN,0);
/*
      sprintf(string,"%.2lf",gamma+0.05);
      SetDlgItemText(hDlg,GAMMA_VALUE,string);
      sprintf(string,"%d",anim_delay);
      SetDlgItemText(hDlg,ANIMATE_DELAY,string);
*/
      break;
    case WM_HSCROLL:
    case WM_VSCROLL:
      id = getscrollid();
      switch(id) {      
        case SEL_P1_HSCROLL:
            if (cur_sel_region->flags & SEL_SECONDSEL)
            {
                if (sel_r2.type==SEL_POINTS)
                {
                    cur_sel_region=&sel_r1;
                    cur_sel=1;
                    repaint_sel_change(hDlg);
                    repaint_sel_points(hDlg);
                    repaint_constrain(hDlg);
                }
            }
            if (gendata[cur_sel_region->image]->valid)
            {
                i=cur_sel_region->pt1->x;
                mini=0;
                maxi=gendata[cur_sel_region->image]->size-1;
                delt=1;
                ddelt=gendata[cur_sel_region->image]->size/8;
            }
            break;
        case SEL_P1_VSCROLL:
            if (gendata[cur_sel_region->image]->valid && 
                gendata[cur_sel_region->image]->type==DATATYPE_3D)
            {
                i=gendata[cur_sel_region->image]->size-1-cur_sel_region->pt1->y;
                mini=0;
                maxi=gendata[cur_sel_region->image]->size-1;
                delt=1;
                ddelt=gendata[cur_sel_region->image]->size/8;
            }
            break;
        case SEL_P2_HSCROLL:
            if (sel2_on && sel_r1.type==SEL_POINTS && sel_r2.type==SEL_POINTS)
            {
                    cur_sel_region=&sel_r2;
                    cur_sel=2;
                    repaint_sel_change(hDlg);
                    repaint_sel_points(hDlg);
                    repaint_constrain(hDlg);
            }
            if ((cur_sel_region->flags & SEL_SECONDSEL) &&
                cur_sel_region->type==SEL_POINTS)
            {
                if (gendata[cur_sel_region->image]->valid)
                {
                    i=cur_sel_region->pt1->x;
                    mini=0;
                    maxi=gendata[cur_sel_region->image]->size-1;
                    delt=1;
                    ddelt=gendata[cur_sel_region->image]->size/8;
                }
            }                
            else if (gendata[cur_sel_region->image]->valid)
            {
                i=cur_sel_region->pt2->x;
                mini=0;
                maxi=gendata[cur_sel_region->image]->size-1;
                delt=1;
                ddelt=gendata[cur_sel_region->image]->size/8;
            }
            break;
        case SEL_P2_VSCROLL:
            if (sel2_on && sel_r1.type==SEL_POINTS && sel_r2.type==SEL_POINTS
            )
            {
                    cur_sel_region=&sel_r2;
                    cur_sel=2;
                    repaint_sel_change(hDlg);
                    repaint_sel_points(hDlg);
                    repaint_constrain(hDlg);
            }
            if (gendata[cur_sel_region->image]->valid && 
                gendata[cur_sel_region->image]->type==DATATYPE_3D)
            {
                if ((cur_sel_region->flags & SEL_SECONDSEL) &&
                    cur_sel_region->type==SEL_POINTS)
                {
                    i=gendata[cur_sel_region->image]->size-1-cur_sel_region->pt1->y;
                    mini=0;
                    maxi=gendata[cur_sel_region->image]->size-1;
                    delt=1;
                    ddelt=gendata[cur_sel_region->image]->size/8;
                }                
                else 
                {
                    i=gendata[cur_sel_region->image]->size-1-cur_sel_region->pt2->y;
                    mini=0;
                    maxi=gendata[cur_sel_region->image]->size-1;
                    delt=1;
                    ddelt=gendata[cur_sel_region->image]->size/8;
                }
            }
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
        case SEL_P1_HSCROLL:
            if (gendata[cur_sel_region->image]->valid)
            {
                if ((cur_sel_region->flags & SEL_SECONDSEL)
                    && sel2_on) SendMessage(ImgDlg, WM_COMMAND, IM_SEL_REDRAW, 0);
                else if (sel1_on) SendMessage(ImgDlg, WM_COMMAND, IM_SEL_REDRAW, 0);                cur_sel_region->pt1->x=i;
                cur_sel_region->pt1->x=i;
                if (cur_sel_region->pt1->y<0) cur_sel_region->pt1->y=0;
                switch(gendata[cur_sel_region->image]->type)
                {
                    case DATATYPE_3D:
                        cur_sel_region->pt1->z=*(gendata[cur_sel_region->image]->ptr+
                            cur_sel_region->pt1->y*gendata[cur_sel_region->image]->size+cur_sel_region->pt1->x);
                        break;
                    case DATATYPE_2D:
                        cur_sel_region->pt1->z=*(gendata[cur_sel_region->image]->data2d+
                            cur_sel_region->pt1->x);
                        cur_sel_region->pt1->t=*(gendata[cur_sel_region->image]->time2d+
                            cur_sel_region->pt1->x);
                        break;
                    case DATATYPE_GEN2D:
                        cur_sel_region->pt1->z=*(gendata[cur_sel_region->image]->yf+
                            cur_sel_region->pt1->x);
                        break;
                }
                                    
                SendMessage(ImgDlg, WM_COMMAND, IM_SEL_REDRAW, 0);
                if (cur_sel==1) sel1_on=1;
                if (cur_sel==2) sel2_on=1;
                repaint_sel_p1(hDlg);
                if (AnlOn) 
                {
                    if (cur_sel==1) 
                        SendMessage(AnlWnd,WM_COMMAND,SEL_REPAINT_P1,0);
                    else
                        SendMessage(AnlWnd,WM_COMMAND,SEL_REPAINT_P2,0);
                }
            }
            break;
        case SEL_P1_VSCROLL:
            if (gendata[cur_sel_region->image]->valid &&
             gendata[cur_sel_region->image]->type==DATATYPE_3D)
            {
                if (sel1_on) SendMessage(ImgDlg, WM_COMMAND, IM_SEL_REDRAW, 0);
                cur_sel_region->pt1->y=gendata[cur_sel_region->image]->size-1-i;
                if (cur_sel_region->pt1->x<0) cur_sel_region->pt1->x=0;
                cur_sel_region->pt1->z=*(gendata[cur_sel_region->image]->ptr+
                    cur_sel_region->pt1->y*gendata[cur_sel_region->image]->size+cur_sel_region->pt1->x);
                SendMessage(ImgDlg, WM_COMMAND, IM_SEL_REDRAW, 0);
                sel1_on=1;
                repaint_sel_p1(hDlg);
                if (AnlOn) 
                {
                    if (cur_sel==1) 
                        SendMessage(AnlWnd,WM_COMMAND,SEL_REPAINT_P1,0);
                    else
                        SendMessage(AnlWnd,WM_COMMAND,SEL_REPAINT_P2,0);
                }
                
            }
            break;
        case SEL_P2_HSCROLL:
            if ((cur_sel_region->flags & SEL_SECONDSEL) &&
                cur_sel_region->type==SEL_POINTS)
            {
                if (gendata[cur_sel_region->image]->valid)
                {
                    if (sel2_on) SendMessage(ImgDlg, WM_COMMAND, IM_SEL_REDRAW, 0);
                    cur_sel_region->pt1->x=i;
                    if (cur_sel_region->pt1->y<0) cur_sel_region->pt1->y=0;
                    switch(gendata[cur_sel_region->image]->type)
                    {
                        case DATATYPE_3D:
                            cur_sel_region->pt1->z=*(gendata[cur_sel_region->image]->ptr+
                                cur_sel_region->pt1->y*gendata[cur_sel_region->image]->size+cur_sel_region->pt1->x);
                            break;
                        case DATATYPE_2D:
                            cur_sel_region->pt1->z=*(gendata[cur_sel_region->image]->data2d+
                                cur_sel_region->pt1->x);
                            cur_sel_region->pt1->t=*(gendata[cur_sel_region->image]->time2d+
                                cur_sel_region->pt1->x);
                            break;
                        case DATATYPE_GEN2D:
                            cur_sel_region->pt1->z=*(gendata[cur_sel_region->image]->yf+
                                cur_sel_region->pt1->x);
                            break;
                    }
                                        
                    SendMessage(ImgDlg, WM_COMMAND, IM_SEL_REDRAW, 0);
                    if (cur_sel==2) sel2_on=1;
                    repaint_sel_p1(hDlg);
                    if (AnlOn) 
                    {
                        if (cur_sel==1) 
                            SendMessage(AnlWnd,WM_COMMAND,SEL_REPAINT_P1,0);
                        else
                            SendMessage(AnlWnd,WM_COMMAND,SEL_REPAINT_P2,0);
                    }
                    
                }
            }                
            else if (gendata[cur_sel_region->image]->valid)
            {
                if (cur_sel==2 && sel2_on) SendMessage(ImgDlg, WM_COMMAND, IM_SEL_REDRAW, 0);
                if (cur_sel==1 && sel1_on) SendMessage(ImgDlg, WM_COMMAND, IM_SEL_REDRAW, 0);
                cur_sel_region->pt2->x=i;
                if (cur_sel_region->pt2->y<0) cur_sel_region->pt2->y=0;
                switch(gendata[cur_sel_region->image]->type)
                {
                    case DATATYPE_3D:
                        cur_sel_region->pt2->z=*(gendata[cur_sel_region->image]->ptr+
                            cur_sel_region->pt2->y*gendata[cur_sel_region->image]->size+cur_sel_region->pt2->x);
                        break;
                    case DATATYPE_GEN2D:
                    case DATATYPE_2D:
                        break;
                }
                SendMessage(ImgDlg, WM_COMMAND, IM_SEL_REDRAW, 0);
                if (cur_sel==1) sel1_on=1;
                if (cur_sel==2) sel2_on=1;                                    
                repaint_sel_p2(hDlg);
                if (AnlOn) 
                {
                    if (cur_sel==1) 
                        SendMessage(AnlWnd,WM_COMMAND,SEL_REPAINT_P1,0);
                    else
                        SendMessage(AnlWnd,WM_COMMAND,SEL_REPAINT_P2,0);
                }
                
            }
            break;
        case SEL_P2_VSCROLL:
            if (gendata[cur_sel_region->image]->valid &&
                gendata[cur_sel_region->image]->type==DATATYPE_3D)
            {
                if (sel2_on) SendMessage(ImgDlg, WM_COMMAND, IM_SEL_REDRAW, 0);
                if ((cur_sel_region->flags & SEL_SECONDSEL) &&
                    cur_sel_region->type==SEL_POINTS)
                {
                    cur_sel_region->pt1->y=gendata[cur_sel_region->image]->size-1-i;
                    if (cur_sel_region->pt1->x<0) cur_sel_region->pt1->x=0;
                    cur_sel_region->pt1->z=*(gendata[cur_sel_region->image]->ptr+
                        cur_sel_region->pt1->y*gendata[cur_sel_region->image]->size+cur_sel_region->pt1->x);
                }
                else
                {
                    cur_sel_region->pt2->y=gendata[cur_sel_region->image]->size-1-i;
                    if (cur_sel_region->pt2->x<0) cur_sel_region->pt2->x=0;
                    cur_sel_region->pt2->z=*(gendata[cur_sel_region->image]->ptr+
                        cur_sel_region->pt2->y*gendata[cur_sel_region->image]->size+cur_sel_region->pt2->x);
                }
                SendMessage(ImgDlg, WM_COMMAND, IM_SEL_REDRAW, 0);
                sel2_on=1;
                repaint_sel_p2(hDlg);
                if (AnlOn) 
                {
                    if (cur_sel==1) 
                        SendMessage(AnlWnd,WM_COMMAND,SEL_REPAINT_P1,0);
                    else
                        SendMessage(AnlWnd,WM_COMMAND,SEL_REPAINT_P2,0);
                }
                
            }
            break;
                
      }
      break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
            case SEL_INVERT:
                if (cur_sel_region->type == SEL_SQUARE ||
                    cur_sel_region->type == SEL_CIRCLE)
                {
                    SendMessage(ImgDlg, WM_COMMAND, IM_SEL_REDRAW, 0);
                    
                    cur_sel_region->flags ^= SEL_INVERTED;
                    SendMessage(ImgDlg, WM_COMMAND, IM_SEL_REDRAW, 0);
                }
                break;
            case SEL_1_2:
                if (cur_sel==1)
                {
                    cur_sel=2;
                    cur_sel_region=&sel_r2;
                }
                else
                {
                    cur_sel=1;
                    cur_sel_region=&sel_r1;
                }
                repaint_sel_points(hDlg);
                repaint_sel_change(hDlg);
                repaint_constrain(hDlg);
                break;
            case SEL_SEL_CHANGED:
                repaint_sel_change(hDlg);
                repaint_sel_points(hDlg);
                repaint_constrain(hDlg);
                break;
            
            case SEL_CROP:
                PostMessage(ImgDlg,WM_COMMAND,SEL_CROP,0);
                break;
            case SEL_COPY:
                PostMessage(ImgDlg,WM_COMMAND,SEL_COPY,0);
                break;
            case SEL_CONSTRAIN:
                if (IsDlgButtonChecked(hDlg, SEL_CONSTRAIN))
                    constrain=cur_sel;
                else constrain=0;
                break;
            case SEL_REPAINT_L1:
            case SEL_REPAINT_P1:
            case SEL_REPAINT_P2:
                repaint_sel_p1(hDlg);
                if (AnlOn) 
                {
                    if (cur_sel==1) 
                        SendMessage(AnlWnd,WM_COMMAND,SEL_REPAINT_P1,0);
                    else
                        SendMessage(AnlWnd,WM_COMMAND,SEL_REPAINT_P2,0);
                }
                    
                break;
            case SEL_REPAINT_L2:
                repaint_sel_p2(hDlg);
                if (AnlOn) 
                {
                    if (cur_sel==1) 
                        SendMessage(AnlWnd,WM_COMMAND,SEL_REPAINT_P1,0);
                    else
                        SendMessage(AnlWnd,WM_COMMAND,SEL_REPAINT_P2,0);
                }
                break;
            case SEL_POINTS:
            case SEL_LINE:
            case SEL_SQUARE:
                SendMessage(ImgDlg, WM_COMMAND, IM_SEL_REDRAW, 0);
                if (cur_sel_region->type==SEL_CIRCLE)
                {
                    cur_sel_region->pt1->x=-1;
                    cur_sel_region->pt2->x=-1;
                    cur_sel_region->pt1->y=-1;
                    cur_sel_region->pt2->y=-1;
                }
                if (cur_sel_region->type!=LOWORD(wParam)) 
                    cur_sel_region->flags &= ~(SEL_INVERTED);
                
                cur_sel_region->type=LOWORD(wParam);
                SendMessage(ImgDlg, WM_COMMAND, IM_SEL_REDRAW, 0);
                break;
                    
            case SEL_CIRCLE:
                SendMessage(ImgDlg, WM_COMMAND, IM_SEL_REDRAW, 0);
                if (cur_sel_region->type!=LOWORD(wParam)) 
                    cur_sel_region->flags &= ~(SEL_INVERTED);
                if (cur_sel_region->type!=SEL_CIRCLE)
                {
                    cur_sel_region->pt1->x=-1;
                    cur_sel_region->pt2->x=-1;
                    cur_sel_region->pt1->y=-1;
                    cur_sel_region->pt2->y=-1;
                }
                cur_sel_region->type=LOWORD(wParam);
                SendMessage(ImgDlg, WM_COMMAND, IM_SEL_REDRAW, 0);
                break;

            case SEL_MIN:
                if (last_button>=0)
                {
#define Z1 (*(gendata[tmp_image]->ptr+tmp_pt1.x+tmp_pt1.y*gendata[tmp_image]->size))
#define Z2 (*(gendata[tmp_image]->ptr+tmp_pt2.x+tmp_pt2.y*gendata[tmp_image]->size))
#define Z3 (*(gendata[tmp_image]->ptr+tmp_pt3.x+tmp_pt3.y*gendata[tmp_image]->size))
                    tmp_pt1.x=cur_sel_region->pt1->x;
                    tmp_pt1.y=cur_sel_region->pt1->y;
                    tmp_image=cur_sel_region->image;
                    switch(gendata[tmp_image]->type)
                    {
                        case DATATYPE_3D:
                            if (tmp_pt1.x>=0 && tmp_pt1.y>=0 && 
                                gendata[tmp_image]->valid)
                            {
                                done=0;
                                tmp_pt3.x=tmp_pt1.y;
                                tmp_pt3.y=tmp_pt1.y;
                                while(!done)
                                {
                                    tmp_pt2.x=tmp_pt1.x-1;
                                    tmp_pt2.y=tmp_pt1.y-1;
                                    if (tmp_pt2.x>=0 && tmp_pt2.y>=0)
                                    {
                                        if (Z2<Z3) 
                                        {
                                            tmp_pt3.x=tmp_pt2.x;
                                            tmp_pt3.y=tmp_pt2.y;
                                        }
                                    }
                                    tmp_pt2.x=tmp_pt1.x+1;
                                    tmp_pt2.y=tmp_pt1.y-1;
                                    if (tmp_pt2.x<gendata[tmp_image]->size
                                        && tmp_pt2.y>=0)
                                    {
                                        if (Z2<Z3) 
                                        {
                                            tmp_pt3.x=tmp_pt2.x;
                                            tmp_pt3.y=tmp_pt2.y;
                                        }
                                    }
                                    tmp_pt2.x=tmp_pt1.x+1;
                                    tmp_pt2.y=tmp_pt1.y+1;
                                    if (tmp_pt2.y<gendata[tmp_image]->size &&
                                        tmp_pt2.x<gendata[tmp_image]->size)
                                    {
                                        if (Z2<Z3) 
                                        {
                                            tmp_pt3.x=tmp_pt2.x;
                                            tmp_pt3.y=tmp_pt2.y;
                                        }
                                    }       
                                    tmp_pt2.x=tmp_pt1.x-1;
                                    tmp_pt2.y=tmp_pt1.y+1;
                                    if (tmp_pt2.y<gendata[tmp_image]->size
                                        && tmp_pt2.x>=0)
                                    {
                                        if (Z2<Z3) 
                                        {
                                            tmp_pt3.x=tmp_pt2.x;
                                            tmp_pt3.y=tmp_pt2.y;
                                        }
                                    }
                                    tmp_pt2.x=tmp_pt1.x-1;
                                    tmp_pt2.y=tmp_pt1.y;
                                    if (tmp_pt2.x>=0)
                                    {
                                        if (Z2<Z3) 
                                        {
                                            tmp_pt3.x=tmp_pt2.x;
                                            tmp_pt3.y=tmp_pt2.y;
                                        }
                                    }           
                                    tmp_pt2.x=tmp_pt1.x+1;
                                    tmp_pt2.y=tmp_pt1.y;
                                    if (tmp_pt2.x<gendata[tmp_image]->size)
                                    {
                                        if (Z2<Z3) 
                                        {
                                            tmp_pt3.x=tmp_pt2.x;
                                            tmp_pt3.y=tmp_pt2.y;
                                        }
                                    }
                                    tmp_pt2.x=tmp_pt1.x;
                                    tmp_pt2.y=tmp_pt1.y-1;
                                    if (tmp_pt2.y>=0)
                                    {
                                        if (Z2<Z3) 
                                        {
                                            tmp_pt3.x=tmp_pt2.x;
                                            tmp_pt3.y=tmp_pt2.y;
                                        }
                                    }
                                    tmp_pt2.x=tmp_pt1.x;
                                    tmp_pt2.y=tmp_pt1.y+1;
                                    if (tmp_pt2.y<gendata[tmp_image]->size)
                                    {
                                        if (Z2<Z3) 
                                        {
                                            tmp_pt3.x=tmp_pt2.x;
                                            tmp_pt3.y=tmp_pt2.y;
                                        }
                                    }
                                    done=(tmp_pt1.x==tmp_pt3.x && tmp_pt1.y==tmp_pt3.y);
                                    if (!done)
                                    {
                                        tmp_pt1.x=tmp_pt3.x;
                                        tmp_pt1.y=tmp_pt3.y;
                                    }
                                }
                                switch (last_button)
                                {
                                    case L_BUTTON:
                                        if (sel1_on) SendMessage(ImgDlg, WM_COMMAND, IM_SEL_REDRAW, 0);
                                        cur_sel_region->pt1->x=tmp_pt1.x;
                                        cur_sel_region->pt1->y=tmp_pt1.y;
                                        cur_sel_region->pt1->z=Z1;
                                        SendMessage(ImgDlg, WM_COMMAND, IM_SEL_REDRAW, 0);
                                        repaint_sel_p1(hDlg);
                                        if (AnlOn) PostMessage(AnlWnd,WM_COMMAND,SEL_REPAINT_P1,0);
                                        sel1_on=1;
                                        break;
                                    case R_BUTTON:
                                        if (sel2_on) SendMessage(ImgDlg, WM_COMMAND, IM_SEL_REDRAW, 0);
                                        cur_sel_region->pt1->x=tmp_pt1.x;
                                        cur_sel_region->pt1->y=tmp_pt1.y;
                                        cur_sel_region->pt1->z=Z1;
                                        SendMessage(ImgDlg, WM_COMMAND, IM_SEL_REDRAW, 0);
                                        sel2_on=1;
                                        repaint_sel_p2(hDlg);
                                        if (AnlOn) PostMessage(AnlWnd,WM_COMMAND,SEL_REPAINT_P2,0);
                                        break;
                                }
                            }
                            break;
                        case DATATYPE_2D:
                            image_higherv[current_image]=IN_MAX;
                            image_lowerv[current_image]=IN_MIN;
                            SendMessage(ImgDlg, WM_COMMAND, IM_NEW_VSCALE, 0);
                            //if (VscaleOn) SendMessage(VscaleDlg, WM_COMMAND, VSCALE_NEW_IMAGE, 0);
                            if (VscaleOn) SendMessage(VscaleWnd, WM_COMMAND, VSCALE_NEW_IMAGE, 0);
                            
                            break;
                        case DATATYPE_GEN2D:
                            image_higherv[current_image]=IN_MAX;
                            image_lowerv[current_image]=IN_MIN;
                            SendMessage(ImgDlg, WM_COMMAND, IM_NEW_VSCALE, 0);
                            //if (VscaleOn) SendMessage(VscaleDlg, WM_COMMAND, VSCALE_NEW_IMAGE, 0);
                            if (VscaleOn) SendMessage(VscaleWnd, WM_COMMAND, VSCALE_NEW_IMAGE, 0);
                            
                            break;
                    }
                }
                break;
            case SEL_MAX:
                if (last_button>=0)
                {
                    tmp_pt1.x=cur_sel_region->pt1->x;
                    tmp_pt1.y=cur_sel_region->pt1->y;
                    tmp_image=cur_sel_region->image;
                    switch(gendata[tmp_image]->type)
                    {
                        case DATATYPE_3D:
                    if (tmp_pt1.x>=0 && tmp_pt1.y>=0 && 
                        gendata[tmp_image]->valid)
                    {
                        done=0;
                        tmp_pt3.x=tmp_pt1.y;
                        tmp_pt3.y=tmp_pt1.y;
                        while(!done)
                        {
                            tmp_pt2.x=tmp_pt1.x-1;
                            tmp_pt2.y=tmp_pt1.y-1;
                            if (tmp_pt2.x>=0 && tmp_pt2.y>=0)
                            {
                                if (Z2>Z3) 
                                {
                                    tmp_pt3.x=tmp_pt2.x;
                                    tmp_pt3.y=tmp_pt2.y;
                                }
                            }
                            tmp_pt2.x=tmp_pt1.x+1;
                            tmp_pt2.y=tmp_pt1.y-1;
                            if (tmp_pt2.x<gendata[tmp_image]->size
                                && tmp_pt2.y>=0)
                            {
                                if (Z2>Z3) 
                                {
                                    tmp_pt3.x=tmp_pt2.x;
                                    tmp_pt3.y=tmp_pt2.y;
                                }
                            }
                            tmp_pt2.x=tmp_pt1.x+1;
                            tmp_pt2.y=tmp_pt1.y+1;
                            if (tmp_pt2.y<gendata[tmp_image]->size &&
                                tmp_pt2.x<gendata[tmp_image]->size)
                            {
                                if (Z2>Z3) 
                                {
                                    tmp_pt3.x=tmp_pt2.x;
                                    tmp_pt3.y=tmp_pt2.y;
                                }
                            }
                            tmp_pt2.x=tmp_pt1.x-1;
                            tmp_pt2.y=tmp_pt1.y+1;
                            if (tmp_pt2.y<gendata[tmp_image]->size
                                && tmp_pt2.x>=0)
                            {
                                if (Z2>Z3) 
                                {
                                    tmp_pt3.x=tmp_pt2.x;
                                    tmp_pt3.y=tmp_pt2.y;
                                }
                            }
                            tmp_pt2.x=tmp_pt1.x-1;
                            tmp_pt2.y=tmp_pt1.y;
                            if (tmp_pt2.x>=0)
                            {
                                if (Z2>Z3) 
                                {
                                    tmp_pt3.x=tmp_pt2.x;
                                    tmp_pt3.y=tmp_pt2.y;
                                }
                            }
                            tmp_pt2.x=tmp_pt1.x+1;
                            tmp_pt2.y=tmp_pt1.y;
                            if (tmp_pt2.x<gendata[tmp_image]->size)
                            {
                                if (Z2>Z3) 
                                {
                                    tmp_pt3.x=tmp_pt2.x;
                                    tmp_pt3.y=tmp_pt2.y;
                                }
                            }
                            tmp_pt2.x=tmp_pt1.x;
                            tmp_pt2.y=tmp_pt1.y-1;
                            if (tmp_pt2.y>=0)
                            {
                                if (Z2>Z3) 
                                {
                                    tmp_pt3.x=tmp_pt2.x;
                                    tmp_pt3.y=tmp_pt2.y;
                                }
                            }
                            tmp_pt2.x=tmp_pt1.x;
                            tmp_pt2.y=tmp_pt1.y+1;
                            if (tmp_pt2.y<gendata[tmp_image]->size)
                            {
                                if (Z2>Z3) 
                                {
                                    tmp_pt3.x=tmp_pt2.x;
                                    tmp_pt3.y=tmp_pt2.y;
                                }
                            }
                            done=(tmp_pt1.x==tmp_pt3.x && tmp_pt1.y==tmp_pt3.y);
                            if (!done)
                            {
                                tmp_pt1.x=tmp_pt3.x;
                                tmp_pt1.y=tmp_pt3.y;
                            }
                        }
                        switch (last_button)
                        {
                            case L_BUTTON:
                                if (sel1_on) SendMessage(ImgDlg, WM_COMMAND, IM_SEL_REDRAW, 0);
                                cur_sel_region->pt1->x=tmp_pt1.x;
                                cur_sel_region->pt1->y=tmp_pt1.y;
                                cur_sel_region->pt1->z=Z1;
                                SendMessage(ImgDlg, WM_COMMAND, IM_SEL_REDRAW, 0);
                                sel1_on=1;
                                repaint_sel_p1(hDlg);
                                if (AnlOn) PostMessage(AnlWnd,WM_COMMAND,SEL_REPAINT_P1,0);
                                break;
                            case R_BUTTON:
                                if (sel2_on) SendMessage(ImgDlg, WM_COMMAND, IM_SEL_REDRAW, 0);
                                cur_sel_region->pt1->x=tmp_pt1.x;
                                cur_sel_region->pt1->y=tmp_pt1.y;
                                cur_sel_region->pt1->z=Z1;
                                SendMessage(ImgDlg, WM_COMMAND, IM_SEL_REDRAW, 0);
                                sel2_on=1;
                                repaint_sel_p2(hDlg);
                                if (AnlOn) PostMessage(AnlWnd,WM_COMMAND,SEL_REPAINT_P2,0);
                                break;
                        }
                    }
                    break;
                    case DATATYPE_GEN2D:
                    case DATATYPE_2D:
                        break;
                }
                }
                break;
/*
    case I_UPDATE:
      break;
    case GAMMA:
        GetDlgItemText(hDlg,GAMMA_VALUE,string,9);
        gamma=(double) atof(string);
        if (gamma>0)
        {
            MakeColorLUT(ColorLUT,gamma);
            apply_gamma();
        }
        break;
*/
            case SEL_EXIT:
                constrain=0;
                PostMessage(ImgDlg,WM_COMMAND,IM_SEL_EXIT,0);
          
                DestroyWindow(hDlg);
                SelWnd = NULL;
                SelOn = FALSE;
                break;
    }
    break;
  }
  return(FALSE);
}
int StartSelection(HWND hWnd, POINT ptCurrent, LPRECT lpSelectRect, int fFlags)
{
    if (lpSelectRect->left != lpSelectRect->right ||
	    lpSelectRect->top != lpSelectRect->bottom)
	ClearSelection(hWnd, lpSelectRect, fFlags);

    lpSelectRect->right = ptCurrent.x;
    lpSelectRect->bottom = ptCurrent.y;

    /* If you are extending the box, then invert the current rectangle */

    if ((fFlags & SEL_SPECIAL) == SEL_EXTEND)
	ClearSelection(hWnd, lpSelectRect, fFlags);

    /* Otherwise, set origin to current location */

    else {
	lpSelectRect->left = ptCurrent.x;
	lpSelectRect->top = ptCurrent.y;
    }
    SetCapture(hWnd);
    return(0);
}

/****************************************************************************

    FUNCTION: UpdateSelection(HWND, POINT, LPRECT, int) - update selection area

    PURPOSE: Update selection

****************************************************************************/

int UpdateSelection(HWND hWnd, POINT ptCurrent, LPRECT lpSelectRect, int fFlags)
{
    HDC hDC;
    int OldROP;

    hDC = GetDC(hWnd);

    switch (fFlags & SEL_TYPE) {

    case SEL_BOX:
	    OldROP = SetROP2(hDC, R2_NOTXORPEN);
//	    MoveTo(hDC, lpSelectRect->left, lpSelectRect->top);
	    MoveToEx(hDC, lpSelectRect->left, lpSelectRect->top,NULL);
	    LineTo(hDC, lpSelectRect->right, lpSelectRect->top);
	    LineTo(hDC, lpSelectRect->right, lpSelectRect->bottom);
	    LineTo(hDC, lpSelectRect->left, lpSelectRect->bottom);
	    LineTo(hDC, lpSelectRect->left, lpSelectRect->top);
	    LineTo(hDC, ptCurrent.x, lpSelectRect->top);
	    LineTo(hDC, ptCurrent.x, ptCurrent.y);
	    LineTo(hDC, lpSelectRect->left, ptCurrent.y);
	    LineTo(hDC, lpSelectRect->left, lpSelectRect->top);
	    SetROP2(hDC, OldROP);
	    break;
    
    case SEL_BLOCK:
	    PatBlt(hDC,
		lpSelectRect->left,
		lpSelectRect->bottom,
		lpSelectRect->right - lpSelectRect->left,
		ptCurrent.y - lpSelectRect->bottom,
		DSTINVERT);
	    PatBlt(hDC,
		lpSelectRect->right,
		lpSelectRect->top,
		ptCurrent.x - lpSelectRect->right,
		ptCurrent.y - lpSelectRect->top,
		DSTINVERT);
	    break;
    }
    lpSelectRect->right = ptCurrent.x;
    lpSelectRect->bottom = ptCurrent.y;
    ReleaseDC(hWnd, hDC);
    return(0);
}

/****************************************************************************

    FUNCTION: EndSelection(POINT, LPRECT)

    PURPOSE: End selection of region, release capture of mouse movement

****************************************************************************/

int EndSelection(POINT ptCurrent, LPRECT lpSelectRect)
{
    lpSelectRect->right = ptCurrent.x;
    lpSelectRect->bottom = ptCurrent.y;
    ReleaseCapture();
    return(0);
}

/****************************************************************************

    FUNCTION: ClearSelection(HWND, LPRECT, int) - clear selection area

    PURPOSE: Clear the current selection

****************************************************************************/

int ClearSelection(HWND hWnd, LPRECT lpSelectRect, int fFlags)
{
    HDC hDC;
    int OldROP;

    hDC = GetDC(hWnd);
    switch (fFlags & SEL_TYPE) {

    case SEL_BOX:
	    OldROP = SetROP2(hDC, R2_NOTXORPEN);
//	    MoveTo(hDC, lpSelectRect->left, lpSelectRect->top);
	    MoveToEx(hDC, lpSelectRect->left, lpSelectRect->top,NULL);
	    LineTo(hDC, lpSelectRect->right, lpSelectRect->top);
	    LineTo(hDC, lpSelectRect->right, lpSelectRect->bottom);
	    LineTo(hDC, lpSelectRect->left, lpSelectRect->bottom);
	    LineTo(hDC, lpSelectRect->left, lpSelectRect->top);
	    SetROP2(hDC, OldROP);
	    break;

    case SEL_BLOCK:
	    PatBlt(hDC,
		lpSelectRect->left,
		lpSelectRect->top,
		lpSelectRect->right - lpSelectRect->left,
		lpSelectRect->bottom - lpSelectRect->top,
		DSTINVERT);
	    break;
    }
    ReleaseDC(hWnd, hDC);
    return(0);
}
int find_coords(SEL_POINT *pt, LPARAM lParam)
{
    int x,y,this_image,xi,yi,i;
    float t;
    
    xi=yi=-1;
    x = LOWORD(lParam);                               
    y = HIWORD(lParam);
    this_image=-1;
//    if (!gendata[this_image]->valid) return this_image;
    if (!zoomed_in)
    {
        
        x-=SMALL_BORDER_SIZE;
        y-=SMALL_BORDER_SIZE;
        if (x<SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE
             && y<(SMALL_IMAGE_SIZE*2+3*SMALL_BORDER_SIZE)) 
        {
            this_image=0;
            
            if (y>SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE
                && y<(SMALL_IMAGE_SIZE*2+SMALL_BORDER_SIZE*3)) 
            {
                this_image+=2;
                y-=SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE;
            }
        }
        else if (x<(SMALL_IMAGE_SIZE*2+3*SMALL_BORDER_SIZE) 
            && y<(SMALL_IMAGE_SIZE*2+3*SMALL_BORDER_SIZE))
        {
            this_image=1;
            x-=SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE;
            if (y>SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE 
                && y<(SMALL_IMAGE_SIZE*2+2*SMALL_BORDER_SIZE)) 
            {
                this_image+=2;
                y-=SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE;
            }
                
        }
        if (x>=(-SMALL_IMAGE_SIZE) && x<SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE
             && y>=(-SMALL_IMAGE_SIZE) && y<SMALL_IMAGE_SIZE+SMALL_BORDER_SIZE
            && this_image>=0)
        if (gendata[this_image]->valid)
        {
            switch(gendata[this_image]->type)
            {
                case DATATYPE_3D:
                    xi=(x*im_screenlines[this_image])/SMALL_IMAGE_SIZE+
                    im_src_x[this_image];
                    yi=((SMALL_IMAGE_SIZE-y)*im_screenlines[this_image])/SMALL_IMAGE_SIZE+
                        im_src_y[this_image];
                    if (xi<0) xi=0;
                    if (xi>=gendata[this_image]->size) xi=gendata[this_image]->size-1;
                    if (yi<0) yi=0;
                    if (yi>=gendata[this_image]->size) yi=gendata[this_image]->size-1;
                    if (yi>=im_src_y[this_image]+im_screenlines[this_image])
                        yi=im_src_y[this_image]+im_screenlines[this_image]-1;
                    break;
                case DATATYPE_GEN2D:
                case DATATYPE_2D:
                    t=x*(image_highert[this_image]-image_lowert[this_image])/
                        SMALL_IMAGE_SIZE+image_lowert[this_image];
                    break;
            }
        }
    }
    else if (x>=0 && x< LARGE_IMAGE_SIZE+2*LARGE_BORDER_SIZE &&
            y>=0 && y< LARGE_IMAGE_SIZE+2*LARGE_BORDER_SIZE
            && gendata[current_image]->valid)
    {
        x-=LARGE_BORDER_SIZE;
        y-=LARGE_BORDER_SIZE;
        this_image=current_image; 
        switch(gendata[this_image]->type)
        {
            case DATATYPE_3D:
                xi=(x*im_screenlines[this_image])/LARGE_IMAGE_SIZE+
                    im_src_x[this_image];
                yi=((LARGE_IMAGE_SIZE-y)*im_screenlines[this_image])/LARGE_IMAGE_SIZE+
                    im_src_y[this_image];
                if (xi<0) xi=0;
                if (xi>=gendata[this_image]->size) xi=gendata[this_image]->size-1;
                if (yi<0) yi=0;
                if (yi>=gendata[this_image]->size) yi=gendata[this_image]->size-1;
                if (yi>=im_src_y[this_image]+im_screenlines[this_image])
                    yi=im_src_y[this_image]+im_screenlines[this_image]-1;
                break;
            case DATATYPE_GEN2D:
            case DATATYPE_2D:
                t=x*(image_highert[this_image]-image_lowert[this_image])/
                    LARGE_IMAGE_SIZE+image_lowert[this_image];
                break;
        }
    }
    if (this_image<0) return this_image;
    switch(gendata[this_image]->type)
    {
        case DATATYPE_3D:
            if (xi>=0 && yi>=0)
            {
                pt->x=xi;
                pt->y=yi;
                if (gendata[this_image]->valid) 
                    pt->z=*(gendata[this_image]->ptr+yi*gendata[this_image]->size+xi);
            }
            break;
        case DATATYPE_2D:
                for(i=0;i<gendata[this_image]->size-1;i++)
                {
                    if (*(gendata[this_image]->time2d+i)<=t &&
                        *(gendata[this_image]->time2d+i+1)> t)
                    {
                        pt->t=*(gendata[this_image]->time2d+i);
                        pt->z=*(gendata[this_image]->data2d+i);
                        pt->x=i;
                        pt->y=1;
                    }
                }
            break;
        case DATATYPE_GEN2D:
            switch(gendata[this_image]->type2d)
            {
                case TYPE2D_SPEC_I:
                case TYPE2D_SPEC_Z:
                case TYPE2D_SPEC_2:
                case TYPE2D_SPEC_3:
                case TYPE2D_MASSPEC:
                    for(i=0;i<gendata[this_image]->size-1;i++)
                    {
                        if (gendata[this_image]->start+
                            i*gendata[this_image]->step<=t &&
                            gendata[this_image]->start+(i+1)*
                            gendata[this_image]->step> t)
                        {
                            pt->z=*(gendata[this_image]->yf+i);
                            pt->x=i;
                            pt->y=1;
                        }
                    }
                    break;
                case TYPE2D_CUT:
                    i=t*gendata[this_image]->size/gendata[this_image]->max_x;
                    pt->z=*(gendata[this_image]->yf+i);
                    pt->x=i;
                    pt->y=1;
                    break;
            }
            break;
    }
    return(this_image);
//MessageBeep(MB_OK);    
}
void repaint_sel_points(HWND hDlg)
{
        
    repaint_sel_p1(hDlg);
    repaint_sel_p2(hDlg);
    if (AnlOn) PostMessage(AnlWnd,WM_COMMAND,SEL_REPAINT_P1,0);
}

void repaint_sel_p1(HWND hDlg)
{
    int status;
//    int *debug;
    
    out_smart=1;
    if (cur_sel_region->pt1->x>=0 && cur_sel_region->pt1->y>=0)
    {
        if ((cur_sel_region->flags & SEL_SECONDSEL) && 
            sel_r2.type==SEL_POINTS)
        {
            switch(gendata[cur_sel_region->image]->type)
            {
                case DATATYPE_3D:
                    sprintf(string,"%d",cur_sel_region->pt1->x);
                    SetDlgItemText(hDlg,SEL_P2_X_INT,string); 
                    sprintf(string,"%d",cur_sel_region->pt1->y);
                    SetDlgItemText(hDlg,SEL_P2_Y_INT,string); 
                    sprintf(string,"%f",cur_sel_region->pt1->z);
                    SetDlgItemText(hDlg,SEL_P2_Z_FLOAT,string); 
                    sprintf(string,"%f",dtov(gendata[cur_sel_region->image]->x,2)*10+
                        dtov(ZERO+(cur_sel_region->pt1->x-gendata[cur_sel_region->image]->size/2)*
                            (gendata[cur_sel_region->image]->step),gendata[cur_sel_region->image]->
                        x_range)*gendata[cur_sel_region->image]->x_gain);
                    SetDlgItemText(hDlg,SEL_P2_X_VOLTS,string); 
                    sprintf(string,"%f",dtov(gendata[cur_sel_region->image]->y,2)*10+
                        dtov(ZERO+(cur_sel_region->pt1->y-gendata[cur_sel_region->image]->size/2)*
                        (gendata[cur_sel_region->image]->step),gendata[cur_sel_region->image]->
                        y_range)*gendata[cur_sel_region->image]->y_gain);
                    SetDlgItemText(hDlg,SEL_P2_Y_VOLTS,string); 
                    sprintf(string,"%f",in_dtov(cur_sel_region->pt1->z));
                    SetDlgItemText(hDlg,SEL_P2_Z_VOLTS,string); 
                    EnableWindow(GetDlgItem(hDlg, SEL_LINE), 1);
                    EnableWindow(GetDlgItem(hDlg, SEL_SQUARE), 1);
                    EnableWindow(GetDlgItem(hDlg, SEL_CIRCLE), 1);
                    EnableWindow(GetDlgItem(hDlg, SEL_P2_VSCROLL), 1);
                    SetScrollRange(GetDlgItem(hDlg,SEL_P2_HSCROLL),SB_CTL, 0,
                        gendata[cur_sel_region->image]->size-1,FALSE);
                    SetScrollRange(GetDlgItem(hDlg,SEL_P2_VSCROLL),SB_CTL, 0,
                        gendata[cur_sel_region->image]->size-1,FALSE);
                    SetScrollPos(GetDlgItem(hDlg,SEL_P2_HSCROLL),SB_CTL,
                        cur_sel_region->pt1->x,TRUE);
                    SetScrollPos(GetDlgItem(hDlg,SEL_P2_VSCROLL),SB_CTL,
                        gendata[cur_sel_region->image]->size-1-cur_sel_region->pt1->y,TRUE);
                    break;
                case DATATYPE_2D:
                    sprintf(string,"%d",cur_sel_region->pt1->x);
                    SetDlgItemText(hDlg,SEL_P2_X_INT,string); 
                    sprintf(string,"%f",cur_sel_region->pt1->t);
                    SetDlgItemText(hDlg,SEL_P2_Y_INT,string); 
                    sprintf(string,"%f",cur_sel_region->pt1->z);
                    SetDlgItemText(hDlg,SEL_P2_Z_FLOAT,string); 
                    SetDlgItemText(hDlg,SEL_P2_X_VOLTS,""); 
                    SetDlgItemText(hDlg,SEL_P2_Y_VOLTS,""); 
                    sprintf(string,"%f",in_dtov(cur_sel_region->pt1->z));
                    SetDlgItemText(hDlg,SEL_P2_Z_VOLTS,string); 
                    cur_sel_region->type=SEL_POINTS;
                    EnableWindow(GetDlgItem(hDlg, SEL_LINE), 0);
                    EnableWindow(GetDlgItem(hDlg, SEL_SQUARE), 0);
                    EnableWindow(GetDlgItem(hDlg, SEL_CIRCLE), 0);
                    if (gendata[cur_sel_region->image]->size<32000)
                    {
                        SetScrollRange(GetDlgItem(hDlg,SEL_P2_HSCROLL),SB_CTL, 0,
                            gendata[cur_sel_region->image]->size-1,FALSE);
                        SetScrollPos(GetDlgItem(hDlg,SEL_P2_HSCROLL),SB_CTL,
                            cur_sel_region->pt1->x,TRUE);
                    }
                    else
                    {
                        SetScrollRange(GetDlgItem(hDlg,SEL_P2_HSCROLL),SB_CTL, 0,
                            (gendata[cur_sel_region->image]->size-1)/100,FALSE);
                        SetScrollPos(GetDlgItem(hDlg,SEL_P2_HSCROLL),SB_CTL,
                            cur_sel_region->pt1->x/100,TRUE);
                    }
                    break;                   
                case DATATYPE_GEN2D:
                    switch(gendata[cur_sel_region->image]->type2d)
                    {
                        case TYPE2D_SPEC_I:
                        case TYPE2D_SPEC_Z:
                        case TYPE2D_SPEC_2:
                        case TYPE2D_SPEC_3:
                        case TYPE2D_MASSPEC:
                            sprintf(string,"%f",dtov(gendata[cur_sel_region->image]->start+
                                cur_sel_region->pt1->x*gendata[cur_sel_region->image]->step,
                                gendata[cur_sel_region->image]->v_range));
                            SetDlgItemText(hDlg,SEL_P2_Y_VOLTS,string); 
                            sprintf(string,"%f",dtov(gendata[cur_sel_region->image]->start+
                                cur_sel_region->pt1->x*gendata[cur_sel_region->image]->step,
                                gendata[cur_sel_region->image]->v_range));
                            SetDlgItemText(hDlg,SEL_P2_Y_INT,string); 
                            break;
                        case TYPE2D_CUT:
                            break;
                    }
                    sprintf(string,"%d",cur_sel_region->pt1->x);
                    SetDlgItemText(hDlg,SEL_P2_X_INT,string); 
/*
                    debug=(int *)&(cur_sel_region->pt1->z);
                    sprintf(string,"%x",*debug);
*/
                    sprintf(string,"%f",cur_sel_region->pt1->z);
                    SetDlgItemText(hDlg,SEL_P2_Z_FLOAT,string); 
                    SetDlgItemText(hDlg,SEL_P2_X_VOLTS,""); 
                    sprintf(string,"%f",in_dtov(cur_sel_region->pt1->z));
                    SetDlgItemText(hDlg,SEL_P2_Z_VOLTS,string); 
                    cur_sel_region->type=SEL_POINTS;
                    EnableWindow(GetDlgItem(hDlg, SEL_LINE), 0);
                    EnableWindow(GetDlgItem(hDlg, SEL_SQUARE), 0);
                    EnableWindow(GetDlgItem(hDlg, SEL_CIRCLE), 0);
                    if (gendata[cur_sel_region->image]->size<32000)
                    {
                        SetScrollRange(GetDlgItem(hDlg,SEL_P2_HSCROLL),SB_CTL, 0,
                            gendata[cur_sel_region->image]->size-1,FALSE);
                        SetScrollPos(GetDlgItem(hDlg,SEL_P2_HSCROLL),SB_CTL,
                            cur_sel_region->pt1->x,TRUE);
                    }
                    else
                    {
                        SetScrollRange(GetDlgItem(hDlg,SEL_P2_HSCROLL),SB_CTL, 0,
                            (gendata[cur_sel_region->image]->size-1)/100,FALSE);
                        SetScrollPos(GetDlgItem(hDlg,SEL_P2_HSCROLL),SB_CTL,
                            cur_sel_region->pt1->x/100,TRUE);
                    }
                    break;                   
            }
        }
        else
        {
            switch(gendata[cur_sel_region->image]->type)
            {
                case DATATYPE_3D:
                    sprintf(string,"%d",cur_sel_region->pt1->x);
                    SetDlgItemText(hDlg,SEL_P1_X_INT,string); 
                    sprintf(string,"%d",cur_sel_region->pt1->y);
                    SetDlgItemText(hDlg,SEL_P1_Y_INT,string); 
                    sprintf(string,"%f",cur_sel_region->pt1->z);
                    SetDlgItemText(hDlg,SEL_P1_Z_FLOAT,string); 
                    sprintf(string,"%f",dtov(gendata[cur_sel_region->image]->x,2)*10+
                        dtov(ZERO+(cur_sel_region->pt1->x-gendata[cur_sel_region->image]->size/2)*
                        (gendata[cur_sel_region->image]->step),gendata[cur_sel_region->image]->
                        x_range)*gendata[cur_sel_region->image]->x_gain);
                    SetDlgItemText(hDlg,SEL_P1_X_VOLTS,string); 
                    sprintf(string,"%f",dtov(gendata[cur_sel_region->image]->y,2)*10+
                        dtov(ZERO+(cur_sel_region->pt1->y-gendata[cur_sel_region->image]->size/2)*
                        (gendata[cur_sel_region->image]->step),gendata[cur_sel_region->image]->
                        y_range)*gendata[cur_sel_region->image]->y_gain);
                    SetDlgItemText(hDlg,SEL_P1_Y_VOLTS,string); 
                    sprintf(string,"%f",in_dtov(cur_sel_region->pt1->z));
                    SetDlgItemText(hDlg,SEL_P1_Z_VOLTS,string); 
                    EnableWindow(GetDlgItem(hDlg, SEL_LINE), 1);
                    EnableWindow(GetDlgItem(hDlg, SEL_SQUARE), 1);
                    EnableWindow(GetDlgItem(hDlg, SEL_CIRCLE), 1);
                    EnableWindow(GetDlgItem(hDlg, SEL_P1_VSCROLL), 1);
                    SetScrollRange(GetDlgItem(hDlg,SEL_P1_HSCROLL),SB_CTL, 0,
                        gendata[cur_sel_region->image]->size-1,FALSE);
                    SetScrollRange(GetDlgItem(hDlg,SEL_P1_VSCROLL),SB_CTL, 0,
                        gendata[cur_sel_region->image]->size-1,FALSE);
                    SetScrollPos(GetDlgItem(hDlg,SEL_P1_HSCROLL),SB_CTL,
                        cur_sel_region->pt1->x,TRUE);
                    SetScrollPos(GetDlgItem(hDlg,SEL_P1_VSCROLL),SB_CTL,
                        gendata[cur_sel_region->image]->size-1-cur_sel_region->pt1->y,TRUE);
                    break;
                case DATATYPE_2D:
                    sprintf(string,"%d",cur_sel_region->pt1->x);
                    SetDlgItemText(hDlg,SEL_P1_X_INT,string); 
                    sprintf(string,"%f",cur_sel_region->pt1->t);
                    SetDlgItemText(hDlg,SEL_P1_Y_INT,string); 
                    sprintf(string,"%f",cur_sel_region->pt1->z);
                    SetDlgItemText(hDlg,SEL_P1_Z_FLOAT,string); 
                    SetDlgItemText(hDlg,SEL_P1_X_VOLTS,""); 
                    SetDlgItemText(hDlg,SEL_P1_Y_VOLTS,""); 
                    sprintf(string,"%f",in_dtov(cur_sel_region->pt1->z));
                    SetDlgItemText(hDlg,SEL_P1_Z_VOLTS,string); 
                    cur_sel_region->type=SEL_POINTS;
                    EnableWindow(GetDlgItem(hDlg, SEL_LINE), 0);
                    EnableWindow(GetDlgItem(hDlg, SEL_SQUARE), 0);
                    EnableWindow(GetDlgItem(hDlg, SEL_CIRCLE), 0);
                    EnableWindow(GetDlgItem(hDlg, SEL_P1_VSCROLL), 0);
                    if (gendata[cur_sel_region->image]->size<32000)
                    {
                        SetScrollRange(GetDlgItem(hDlg,SEL_P1_HSCROLL),SB_CTL, 0,
                            gendata[cur_sel_region->image]->size-1,FALSE);
                        SetScrollPos(GetDlgItem(hDlg,SEL_P1_HSCROLL),SB_CTL,
                            cur_sel_region->pt1->x,TRUE);
                    }
                    else
                    {
                        SetScrollRange(GetDlgItem(hDlg,SEL_P1_HSCROLL),SB_CTL, 0,
                            (gendata[cur_sel_region->image]->size-1)/100,FALSE);
                        SetScrollPos(GetDlgItem(hDlg,SEL_P1_HSCROLL),SB_CTL,
                            cur_sel_region->pt1->x/100,TRUE);
                    }
                    
                    break;
                case DATATYPE_GEN2D:
                    switch(gendata[cur_sel_region->image]->type2d)
                    {
                        case TYPE2D_SPEC_I:
                        case TYPE2D_SPEC_Z:
                        case TYPE2D_SPEC_2:
                        case TYPE2D_SPEC_3:
                        case TYPE2D_MASSPEC:
                            sprintf(string,"%d",gendata[cur_sel_region->image]->start+
                                cur_sel_region->pt1->x*gendata[cur_sel_region->image]->step);
                            SetDlgItemText(hDlg,SEL_P1_Y_INT,string); 
                            sprintf(string,"%f",dtov(gendata[cur_sel_region->image]->start+
                                cur_sel_region->pt1->x*gendata[cur_sel_region->image]->step,
                                gendata[cur_sel_region->image]->v_range));
                            SetDlgItemText(hDlg,SEL_P1_Y_VOLTS,string); 
                            break;
                        case TYPE2D_CUT:
                            break;
                    }
                    sprintf(string,"%d",cur_sel_region->pt1->x);
                    SetDlgItemText(hDlg,SEL_P1_X_INT,string); 
                    sprintf(string,"%f",cur_sel_region->pt1->z);
                    SetDlgItemText(hDlg,SEL_P1_Z_FLOAT,string); 
                    SetDlgItemText(hDlg,SEL_P1_X_VOLTS,""); 
                    sprintf(string,"%f",in_dtov(cur_sel_region->pt1->z));
                    SetDlgItemText(hDlg,SEL_P1_Z_VOLTS,string); 
                    cur_sel_region->type=SEL_POINTS;
                    EnableWindow(GetDlgItem(hDlg, SEL_LINE), 0);
                    EnableWindow(GetDlgItem(hDlg, SEL_SQUARE), 0);
                    EnableWindow(GetDlgItem(hDlg, SEL_CIRCLE), 0);
                    if (gendata[cur_sel_region->image]->size<32000)
                    {
                        SetScrollRange(GetDlgItem(hDlg,SEL_P1_HSCROLL),SB_CTL, 0,
                            gendata[cur_sel_region->image]->size-1,FALSE);
                        SetScrollPos(GetDlgItem(hDlg,SEL_P1_HSCROLL),SB_CTL,
                            cur_sel_region->pt1->x,TRUE);
                    }
                    else
                    {
                        SetScrollRange(GetDlgItem(hDlg,SEL_P1_HSCROLL),SB_CTL, 0,
                            (gendata[cur_sel_region->image]->size-1)/100,FALSE);
                        SetScrollPos(GetDlgItem(hDlg,SEL_P1_HSCROLL),SB_CTL,
                            cur_sel_region->pt1->x/100,TRUE);
                    }
                    break;                   
            }
        }
        switch(cur_sel_region->type)
        {
            case SEL_POINTS:
                EnableWindow(GetDlgItem(hDlg, SEL_CROP), 0);
                EnableWindow(GetDlgItem(hDlg, SEL_COPY), 0);
                break;
            case SEL_CIRCLE:
            case SEL_LINE:
                EnableWindow(GetDlgItem(hDlg, SEL_COPY), 0);
                EnableWindow(GetDlgItem(hDlg, SEL_CROP), TRUE);
                break;
            case SEL_SQUARE:
                status=(abs(cur_sel_region->pt1->x-cur_sel_region->pt2->x)
                    ==abs(cur_sel_region->pt1->y-cur_sel_region->pt2->y));
                EnableWindow(GetDlgItem(hDlg, SEL_CROP), TRUE);
                EnableWindow(GetDlgItem(hDlg, SEL_COPY), (BOOL) status);
                break;
        }                
    }
    else if (!(cur_sel_region->flags & SEL_SECONDSEL))

    {
        EnableWindow(GetDlgItem(hDlg, SEL_CROP), 0);
        EnableWindow(GetDlgItem(hDlg, SEL_COPY), 0);
        SetDlgItemText(hDlg,SEL_P1_X_INT,""); 
        SetDlgItemText(hDlg,SEL_P1_Y_INT,""); 
        SetDlgItemText(hDlg,SEL_P1_Z_FLOAT,""); 
        SetDlgItemText(hDlg,SEL_P1_X_VOLTS,""); 
        SetDlgItemText(hDlg,SEL_P1_Y_VOLTS,""); 
        SetDlgItemText(hDlg,SEL_P1_Z_VOLTS,""); 
    }
    out_smart=0;
}

void repaint_sel_p2(HWND hDlg)
{
    int status;                
    if (cur_sel_region->pt2->x>=0 && cur_sel_region->pt2->y>=0
        && cur_sel_region->type!=SEL_POINTS)
    {
        sprintf(string,"%d",cur_sel_region->pt2->x);
        SetDlgItemText(hDlg,SEL_P2_X_INT,string); 
        sprintf(string,"%d",cur_sel_region->pt2->y);
        SetDlgItemText(hDlg,SEL_P2_Y_INT,string); 
        if (cur_sel_region->type!=SEL_CIRCLE)
        {
            sprintf(string,"%f",cur_sel_region->pt2->z);
            SetDlgItemText(hDlg,SEL_P2_Z_FLOAT,string); 
            sprintf(string,"%f",in_dtov(cur_sel_region->pt2->z));
            SetDlgItemText(hDlg,SEL_P2_Z_VOLTS,string); 
            sprintf(string,"%f",dtov(gendata[cur_sel_region->image]->x,2)*10+
                dtov(ZERO+(cur_sel_region->pt2->x-gendata[cur_sel_region->image]->size/2)*
                (gendata[cur_sel_region->image]->step),gendata[cur_sel_region->image]->
                x_range)*gendata[cur_sel_region->image]->x_gain);
            SetDlgItemText(hDlg,SEL_P2_X_VOLTS,string); 
            sprintf(string,"%f",dtov(gendata[cur_sel_region->image]->y,2)*10+
                dtov(ZERO+(cur_sel_region->pt2->y-gendata[cur_sel_region->image]->size/2)*
                (gendata[cur_sel_region->image]->step),gendata[cur_sel_region->image]->
                y_range)*gendata[cur_sel_region->image]->y_gain);
            SetDlgItemText(hDlg,SEL_P2_Y_VOLTS,string); 
            SetScrollRange(GetDlgItem(hDlg,SEL_P2_HSCROLL),SB_CTL, 0,
                gendata[cur_sel_region->image]->size-1,FALSE);
            SetScrollRange(GetDlgItem(hDlg,SEL_P2_VSCROLL),SB_CTL, 0,
                gendata[cur_sel_region->image]->size-1,FALSE);
        }
        else
        {
            SetDlgItemText(hDlg,SEL_P2_Z_FLOAT,""); 
            SetDlgItemText(hDlg,SEL_P2_Z_VOLTS,""); 
            sprintf(string,"%f",
                dtov(ZERO+(cur_sel_region->pt2->x-gendata[cur_sel_region->image]->size/2)*
                (gendata[cur_sel_region->image]->step),gendata[cur_sel_region->image]->
                x_range)*gendata[cur_sel_region->image]->x_gain);
            SetDlgItemText(hDlg,SEL_P2_X_VOLTS,string); 
            sprintf(string,"%f",
                dtov(ZERO+(cur_sel_region->pt2->y-gendata[cur_sel_region->image]->size/2)*
                (gendata[cur_sel_region->image]->step),gendata[cur_sel_region->image]->
                y_range)*gendata[cur_sel_region->image]->y_gain);
            SetDlgItemText(hDlg,SEL_P2_Y_VOLTS,string); 
            SetScrollRange(GetDlgItem(hDlg,SEL_P2_HSCROLL),SB_CTL, 0,
                (gendata[cur_sel_region->image]->size-1)*1.414,FALSE);
            SetScrollRange(GetDlgItem(hDlg,SEL_P2_VSCROLL),SB_CTL, 0,
                (gendata[cur_sel_region->image]->size-1)*1.414,FALSE);
        }
            
        SetScrollPos(GetDlgItem(hDlg,SEL_P2_HSCROLL),SB_CTL,
            cur_sel_region->pt2->x,TRUE);
        SetScrollPos(GetDlgItem(hDlg,SEL_P2_VSCROLL),SB_CTL,
            gendata[cur_sel_region->image]->size-1-cur_sel_region->pt2->y,TRUE);
        
        switch(cur_sel_region->type)
        {
            case SEL_POINTS:
                EnableWindow(GetDlgItem(hDlg, SEL_CROP), 0);
                EnableWindow(GetDlgItem(hDlg, SEL_COPY), 0);
                break;
            case SEL_LINE:
            case SEL_CIRCLE:
                EnableWindow(GetDlgItem(hDlg, SEL_COPY), 0);
                EnableWindow(GetDlgItem(hDlg, SEL_CROP), TRUE);
                break;
            case SEL_SQUARE:
                status=(abs(cur_sel_region->pt1->x-cur_sel_region->pt2->x)
                    ==abs(cur_sel_region->pt1->y-cur_sel_region->pt2->y));
                EnableWindow(GetDlgItem(hDlg, SEL_CROP), TRUE);
                EnableWindow(GetDlgItem(hDlg, SEL_COPY), (BOOL) status);
                break;
        }                
    }
    else if (sel_r2.type!=SEL_POINTS)
    {
        EnableWindow(GetDlgItem(hDlg, SEL_CROP), 0);
        EnableWindow(GetDlgItem(hDlg, SEL_COPY), 0);
        SetDlgItemText(hDlg,SEL_P2_X_INT,""); 
        SetDlgItemText(hDlg,SEL_P2_Y_INT,""); 
        SetDlgItemText(hDlg,SEL_P2_Z_FLOAT,""); 
        SetDlgItemText(hDlg,SEL_P2_X_VOLTS,""); 
        SetDlgItemText(hDlg,SEL_P2_Y_VOLTS,""); 
        SetDlgItemText(hDlg,SEL_P2_Z_VOLTS,""); 
    }
        
    
}

void repaint_sel_change(HWND hDlg)
{
    CheckDlgButton(hDlg, SEL_POINTS, 0);
    CheckDlgButton(hDlg, SEL_LINE, 0);
    CheckDlgButton(hDlg, SEL_SQUARE, 0);
    CheckDlgButton(hDlg, SEL_CIRCLE, 0);
    CheckDlgButton(hDlg, cur_sel_region->type, 1);
    sprintf(string,"%d",cur_sel);
    SetDlgItemText(hDlg,SEL_CUR_SEL,string);
}

void repaint_constrain(HWND hDlg)
{
        if (cur_sel==constrain) CheckDlgButton(hDlg, SEL_CONSTRAIN,1);
        else CheckDlgButton(hDlg, SEL_CONSTRAIN,0);
}    
