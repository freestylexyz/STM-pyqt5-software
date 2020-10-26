#include <time.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include <i86.h>
#include "common.h"
#include "file.h"
#include "data.h"
#include "dio.h"
#include "stm.h"
#include "scan.h"
#include "sharp.h"

BOOL FAR PASCAL SharpDlg(HWND,unsigned,WPARAM,LPARAM);

HWND SharpWnd=NULL;
int sharp_lines=SHARP_LINES_DEF;
int sharp_cycles=SHARP_CYCLES_DEF;
int sharp_bias1=SHARP_BIAS1_DEF;
int sharp_bias2=SHARP_BIAS2_DEF;

extern unsigned int scan_size;
extern int sample_bias_range;
extern unsigned int bit16;

extern char     string[];

void repaint_sharp_lines(HWND);
void repaint_sharp_cycles(HWND);
void repaint_sharp_bias1(HWND);
void repaint_sharp_bias2(HWND);

BOOL FAR PASCAL SharpDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{
    int i;
    int id;
    int delt,ddelt,mini,maxi;
    

  switch(Message) {
    case WM_INITDIALOG:
        SetScrollRange(GetDlgItem(hDlg, SHARP_BIAS1_SCROLL), SB_CTL, sample_bias_min, sample_bias_max, FALSE);
        SetScrollRange(GetDlgItem(hDlg, SHARP_BIAS2_SCROLL), SB_CTL, sample_bias_min, sample_bias_max, FALSE);
        SetScrollRange(GetDlgItem(hDlg, SHARP_LINES_SCROLL), SB_CTL, SHARP_LINES_MIN, scan_size/sharp_cycles, FALSE);
        SetScrollRange(GetDlgItem(hDlg, SHARP_CYCLES_SCROLL), SB_CTL, SHARP_CYCLES_MIN, scan_size/sharp_lines, FALSE);
        repaint_sharp_cycles(hDlg);
        repaint_sharp_lines(hDlg);
        repaint_sharp_bias1(hDlg);
        repaint_sharp_bias2(hDlg);
      break;
    case WM_HSCROLL:
    case WM_VSCROLL:
      id = getscrollid();
      switch(id) {      
        case SHARP_LINES_SCROLL:
          i = sharp_lines;
          mini = SHARP_LINES_MIN;
          maxi = scan_size/sharp_cycles;
          delt = SHARP_LINES_DELT;
          ddelt = SHARP_LINES_DDELT;
          break;
        case SHARP_CYCLES_SCROLL:
          i = sharp_cycles;
          mini = SHARP_CYCLES_MIN;
          maxi = scan_size/sharp_lines;
          delt = SHARP_CYCLES_DELT;
          ddelt = SHARP_CYCLES_DDELT;
          break;
        case SHARP_BIAS1_SCROLL:
            i = (int) sharp_bias1;
            mini = sample_bias_min;
            maxi = sample_bias_max;
            delt = sample_bias_delt;
            ddelt = sample_bias_ddelt;
            break;
        case SHARP_BIAS2_SCROLL:
            i = (int) sharp_bias2;
            mini = sample_bias_min;
            maxi = sample_bias_max;
            delt = sample_bias_delt;
            ddelt = sample_bias_ddelt;
            break;
/*
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
*/
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
        case SHARP_LINES_SCROLL:
            sharp_lines=i;
            repaint_sharp_lines(hDlg);
            break;
        case SHARP_CYCLES_SCROLL:
            sharp_cycles=i;
            repaint_sharp_cycles(hDlg);
            break;
        case SHARP_BIAS1_SCROLL:
            sharp_bias1=i;
            repaint_sharp_bias1(hDlg);
            break;
        case SHARP_BIAS2_SCROLL:
            sharp_bias2=i;
            repaint_sharp_bias2(hDlg);
            break;
/*
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
*/
      }
      break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
            case SHARP_LINES:
                GetDlgItemText(hDlg,SHARP_LINES,string,10);
                sharp_lines=atoi(string);
                sharp_lines=max(min (sharp_lines,scan_size/sharp_cycles),
                                SHARP_LINES_MIN);
                                
                break;
            case SHARP_CYCLES:
                GetDlgItemText(hDlg,SHARP_CYCLES,string,10);
                sharp_cycles=atoi(string);
                sharp_cycles=max(min (sharp_cycles,scan_size/sharp_lines),
                                SHARP_CYCLES_MIN);
                                
                break;
            case SHARP_BIAS1:
                GetDlgItemText(hDlg,SHARP_BIAS1,string,10);
                i = vtod(atof(string),sample_bias_range);
                i = min(max(i, sample_bias_min), sample_bias_max);
                sharp_bias1=i;
                break;
            case SHARP_BIAS2:
                GetDlgItemText(hDlg,SHARP_BIAS2,string,10);
                i = vtod(atof(string),sample_bias_range);
                i = min(max(i, sample_bias_min), sample_bias_max);
                sharp_bias2=i;
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
        case ENTER:
            repaint_sharp_lines(hDlg);
            repaint_sharp_cycles(hDlg);
            repaint_sharp_bias1(hDlg);
            repaint_sharp_bias2(hDlg);
            
            break;
            
        case SHARP_EXIT:
            EndDialog(hDlg,TRUE);
            return(TRUE);
            break;
    }
    break;
  }
  return(FALSE);
}

void repaint_sharp_lines(HWND hDlg)
{
    SetScrollRange(GetDlgItem(hDlg, SHARP_LINES_SCROLL), SB_CTL, SHARP_LINES_MIN, scan_size/sharp_cycles, FALSE);
    SetScrollPos(GetDlgItem(hDlg,SHARP_LINES_SCROLL),SB_CTL,
        sharp_lines,TRUE);
    sprintf(string,"%d",sharp_lines);
    SetDlgItemText(hDlg,SHARP_LINES,string); 
    SetScrollRange(GetDlgItem(hDlg, SHARP_CYCLES_SCROLL), SB_CTL, SHARP_CYCLES_MIN, scan_size/sharp_lines, FALSE);
    SetScrollPos(GetDlgItem(hDlg,SHARP_CYCLES_SCROLL),SB_CTL,
        sharp_cycles,TRUE);
    
    
}

void repaint_sharp_cycles(HWND hDlg)
{
    SetScrollPos(GetDlgItem(hDlg,SHARP_CYCLES_SCROLL),SB_CTL,
        sharp_cycles,TRUE);
    sprintf(string,"%d",sharp_cycles);
    SetDlgItemText(hDlg,SHARP_CYCLES,string); 
    SetScrollRange(GetDlgItem(hDlg, SHARP_LINES_SCROLL), SB_CTL, SHARP_LINES_MIN, scan_size/sharp_cycles, FALSE);
    SetScrollPos(GetDlgItem(hDlg,SHARP_LINES_SCROLL),SB_CTL,
        sharp_lines,TRUE);
    
}

void repaint_sharp_bias1(HWND hDlg)
{
    SetScrollPos(GetDlgItem(hDlg,SHARP_BIAS1_SCROLL),SB_CTL,
        sharp_bias1,TRUE);
    SetDlgItemText(hDlg, SHARP_BIAS1, gcvt(dtov(sharp_bias1,sample_bias_range), 5, string));
}

void repaint_sharp_bias2(HWND hDlg)
{
    SetScrollPos(GetDlgItem(hDlg,SHARP_BIAS2_SCROLL),SB_CTL,
        sharp_bias2,TRUE);
    SetDlgItemText(hDlg, SHARP_BIAS2, gcvt(dtov(sharp_bias2,sample_bias_range), 5, string));
}

