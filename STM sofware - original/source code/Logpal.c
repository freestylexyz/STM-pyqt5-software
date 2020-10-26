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
#include "image.h"
#include "pal.h"
#include "clock.h"

HWND LogpalWnd=NULL;
BOOL LogpalOn=FALSE;
extern HWND ImgDlg;

extern unsigned char *image_dacbox;
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

static double gamma=1;
//unsigned char dacbox[LOGPALSIZE + 1][3];

static LOGPAL_EL *current_logpal_el;
static void remove_logpal_el(HWND,LOGPAL_EL *);
static void enable_buttons(HWND);
static void repaint_current(HWND);

BOOL FAR PASCAL LogpalDlg(HWND,unsigned,WPARAM,LPARAM);

BOOL FAR PASCAL LogpalDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{
  int anim_delay = 10;
  int i;
  LOGPAL_EL *this;
  double d;

  switch(Message) {
    case WM_INITDIALOG:
      current_logpal_el = NULL;
      if(gendata[current_image]->valid)
      {
        if(gendata[current_image]->type == DATATYPE_3D &&
                gendata[current_image]->pal.type == PALTYPE_LOGICAL)
        {
          current_logpal_el = gendata[current_image]->pal.logpal;
        }
      }
                        
      enable_buttons(hDlg);
      repaint_current(hDlg);
      break;

    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case LOGPAL_NEW_IMAGE:
          if(gendata[current_image]->pal.type == PALTYPE_LOGICAL)
                current_logpal_el = gendata[current_image]->pal.logpal;
          else current_logpal_el = NULL;
          enable_buttons(hDlg);
          repaint_current(hDlg);
          break;

        case LOGPAL_CREATE:
          gendata[current_image]->pal.type = PALTYPE_LOGICAL;
          current_logpal_el = new_logpal_el(&(gendata[current_image]->pal.logpal),0);
          current_logpal_el->color.r=gendata[current_image]->pal.dacbox[0];
          current_logpal_el->color.g=gendata[current_image]->pal.dacbox[1];
          current_logpal_el->color.b=gendata[current_image]->pal.dacbox[2];
          current_logpal_el = new_logpal_el(
              &(gendata[current_image]->pal.logpal),LOGPAL_NUM_INDICES - 1);
          current_logpal_el->color.r =
              gendata[current_image]->pal.dacbox[(LOGPALUSED - 1) * 3];
          current_logpal_el->color.g =
              gendata[current_image]->pal.dacbox[(LOGPALUSED - 1) * 3 + 1];
          current_logpal_el->color.b =
              gendata[current_image]->pal.dacbox[(LOGPALUSED - 1) * 3 + 2];
          current_logpal_el = gendata[current_image]->pal.logpal;
          enable_buttons(hDlg);
          repaint_current(hDlg);
          calc_bitmap(current_image);
          SendMessage(ImgDlg,WM_COMMAND,IM_REDRAW_IMAGE,current_image);
          break;

        case LOGPAL_REPAINT:
          calc_bitmap(current_image);
          SendMessage(ImgDlg,WM_COMMAND,IM_REDRAW_IMAGE,current_image);
          break;

        case LOGPAL_DESTROY:
          destroy_logpal(&(gendata[current_image]->pal.logpal));
          gendata[current_image]->pal.type = PALTYPE_MAPPED;
          current_logpal_el = NULL;
          enable_buttons(hDlg);
          repaint_current(hDlg);
          calc_bitmap(current_image);
          SendMessage(ImgDlg,WM_COMMAND,IM_REDRAW_IMAGE,current_image);
          break;

        case LOGPAL_CURRENT_INDEX:
          GetDlgItemText(hDlg,LOGPAL_CURRENT_INDEX,string,9);
          i = atoi(string);
          if(i > 0 && i < LOGPAL_NUM_INDICES - 1)
          {
            this = gendata[current_image]->pal.logpal->next;
            do
            {
              if(i == this->index) break;
              this = this->next;
            } while(this != gendata[current_image]->pal.logpal);
            if(i != this->index)
            {
              current_logpal_el->index = i;
              sort_logpal_els(&(gendata[current_image]->pal.logpal));
            }
          }
          break;

        case LOGPAL_REMOVE:
          i = current_logpal_el->index;
          if(i > 0 && i < LOGPAL_NUM_INDICES - 1)
          {
            current_logpal_el = current_logpal_el->prev;
            remove_logpal_el(hDlg,current_logpal_el->next);
            repaint_current(hDlg);
            calc_bitmap(current_image);
            SendMessage(ImgDlg,WM_COMMAND,IM_REDRAW_IMAGE,current_image);
          }
          break;
           
        case LOGPAL_ADD_POINT:
		  GetDlgItemText(hDlg,LOGPAL_ADD_INDEX,string,9);
          i = atoi(string);
          if(i > 0 && i < LOGPAL_NUM_INDICES - 1)
          {
            this = gendata[current_image]->pal.logpal;
            do
            {
              if(i == this->index) break;
              this = this->next;
            } while(this != gendata[current_image]->pal.logpal);
            if(this == gendata[current_image]->pal.logpal)
            {
              this = gendata[current_image]->pal.logpal;
              while(i > this->index) this = this->next;
              this = new_logpal_el(&(gendata[current_image]->pal.logpal),i);
              sort_logpal_els(&(gendata[current_image]->pal.logpal));
              this->color.r = (this->prev->color.r + this->next->color.r) / 2;
              this->color.g = (this->prev->color.g + this->next->color.g) / 2;
              this->color.b = (this->prev->color.b + this->next->color.b) / 2;
              current_logpal_el = this;
              repaint_current(hDlg);
            }
          }
          break;

        case LOGPAL_R:
          GetDlgItemText(hDlg,LOGPAL_R,string,9);
          i = atoi(string);
          if(i < 0) i = 0;
          if(i > 255) i = 255;
          current_logpal_el->color.r = i;
          break;

        case LOGPAL_G:
          GetDlgItemText(hDlg,LOGPAL_G,string,9);
          i = atoi(string);
          if(i < 0) i = 0;
          if(i > 255) i = 255;
          current_logpal_el->color.g = i;
          break;

        case LOGPAL_B:
          GetDlgItemText(hDlg,LOGPAL_B,string,9);
          i = atoi(string);
          if(i < 0) i = 0;
          if(i > 255) i = 255;
          current_logpal_el->color.b = i;
          break;

        case LOGPAL_GAMMA:
          GetDlgItemText(hDlg,LOGPAL_GAMMA,string,9);
          d = atof(string);
          current_logpal_el->gamma = (float)d;
          break;
            
        case LOGPAL_VISUALIZE:
          break;
        case LOGPAL_EQUALIZED:
          current_logpal_el->equalized = IsDlgButtonChecked(hDlg,LOGPAL_EQUALIZED);
          if(current_logpal_el->equalized && 
                current_logpal_el->index<current_logpal_el->next->index)
          {
            equalize(gendata[current_image],
                INDEX2Z(current_logpal_el->index,gendata[current_image]),
                INDEX2Z(current_logpal_el->next->index,gendata[current_image]),
                current_logpal_el->fhist);
            calc_bitmap(current_image);
            SendMessage(ImgDlg,WM_COMMAND,IM_REDRAW_IMAGE,current_image);
          }
          break;

        case LOGPAL_NEXT:
          current_logpal_el = current_logpal_el->next;
          repaint_current(hDlg);
          break;

        case LOGPAL_PREV:
          current_logpal_el = current_logpal_el->prev;
          repaint_current(hDlg);
          break;
        
        case LOGPAL_EXIT:
          current_logpal_el = NULL;
          LogpalWnd = NULL;
          LogpalOn = FALSE;
        case LOGPAL_EXIT2:
          DestroyWindow(hDlg);
          break;
      }
      break;
  }
  return(FALSE);
}

static void enable_all(HWND hDlg,int status)
{
//    EnableWindow(GetDlgItem(hDlg, LOGPAL_), status);
    EnableWindow(GetDlgItem(hDlg,LOGPAL_REPAINT),status);
    EnableWindow(GetDlgItem(hDlg,LOGPAL_DESTROY),status);
    EnableWindow(GetDlgItem(hDlg,LOGPAL_VISUALIZE),status);
    EnableWindow(GetDlgItem(hDlg,LOGPAL_CURRENT_INDEX),status);
    EnableWindow(GetDlgItem(hDlg,LOGPAL_R),status);
    EnableWindow(GetDlgItem(hDlg,LOGPAL_G),status);
    EnableWindow(GetDlgItem(hDlg,LOGPAL_B),status);
    EnableWindow(GetDlgItem(hDlg,LOGPAL_REMOVE),status);
    EnableWindow(GetDlgItem(hDlg,LOGPAL_GAMMA),status);
    EnableWindow(GetDlgItem(hDlg,LOGPAL_EQUALIZED),status);
    EnableWindow(GetDlgItem(hDlg,LOGPAL_NEXT),status);
    EnableWindow(GetDlgItem(hDlg,LOGPAL_PREV),status);
    EnableWindow(GetDlgItem(hDlg,LOGPAL_ADD_POINT),status);
    EnableWindow(GetDlgItem(hDlg,LOGPAL_ADD_INDEX),status);
}

static void enable_buttons(HWND hDlg)
{
    if(gendata[current_image]->valid)
    {
        if(gendata[current_image]->type == DATATYPE_3D &&
            gendata[current_image]->pal.type == PALTYPE_LOGICAL)
        {
            enable_all(hDlg,1);
            EnableWindow(GetDlgItem(hDlg,LOGPAL_CREATE),0);
            
        }
        else if (gendata[current_image]->type == DATATYPE_3D)
        {
            enable_all(hDlg,0);
            EnableWindow(GetDlgItem(hDlg,LOGPAL_CREATE),1);
        }
        else 
        {
            enable_all(hDlg,0);
            EnableWindow(GetDlgItem(hDlg,LOGPAL_CREATE),0);
        }
    }
    else 
    {
        enable_all(hDlg,0);
        EnableWindow(GetDlgItem(hDlg,LOGPAL_CREATE),0);
    }
}

static void repaint_current(HWND hDlg)
{
    if (current_logpal_el != NULL)
    {
        sprintf(string,"%.2lf",current_logpal_el->gamma);
        SetDlgItemText(hDlg,LOGPAL_GAMMA,string);
        sprintf(string,"%d",current_logpal_el->index);
        SetDlgItemText(hDlg,LOGPAL_CURRENT_INDEX,string);
        sprintf(string,"%d",(current_logpal_el->next->index + current_logpal_el->index) / 2);
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

static void remove_logpal_el(HWND hDlg,LOGPAL_EL *remove_el)
{
    LOGPAL_EL *this_el,**start;
    
    start = &(gendata[current_image]->pal.logpal);
    if(*start == NULL) return;
    if((*start)->next == *start) 
    {
        SendMessage(hDlg,WM_COMMAND,LOGPAL_DESTROY,0);
        return;
    }
    this_el = *start;
    do
    {
        if(this_el == remove_el)
        {
            if(this_el == *start) *start = (*start)->next;
            this_el->next->prev = this_el->prev;
            this_el->prev->next = this_el->next;
            free(this_el);
            break;
        }
        this_el = this_el->next;
    }while (this_el != *start);
}

