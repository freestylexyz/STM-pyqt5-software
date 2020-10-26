#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <conio.h>
#include "common.h"
#include "stm.h"
#include "dio.h"
#include "data.h"
#include "scan.h"

#define fine_x_min 0
#define fine_x_max 4095
#define fine_x_delt 1
#define fine_x_ddelt 50
#define fine_y_min 0
#define fine_y_max 4095
#define fine_y_delt 1
#define fine_y_ddelt 50

extern int z_offset_ch;
extern char string[];
extern datadef *data;
extern unsigned int dac_data[];
extern unsigned char *image_data;
HBITMAP hImage;
extern HBITMAP hCross;
extern LOGPALETTE *pLogPal;
extern HPALETTE hPal;
extern unsigned int out1;
BOOL fine_auto_ramp = TRUE;
unsigned int fine_x,fine_y;
extern int feedback,x_range,y_range,z_range,x_offset_range,
    y_offset_range,z_offset_range,i_set_range,sample_bias_range;

BOOL FAR PASCAL FineDlg(HWND,unsigned,WPARAM,LPARAM);
void repaint_cross(HWND);
void repaint_x(HWND);
void repaint_y(HWND);
void repaint_auto_ramp(HWND);
void pre_fine();
void post_fine();

BOOL FAR PASCAL FineDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{
  unsigned int i,j;
  HDC           hDC;
  HDC           hMemDC;
  PAINTSTRUCT   PtStr;
  int id;
  int mini,maxi,delt,ddelt;
  POINT mouse_pos;

  switch(Message) {
    case WM_INITDIALOG:
      fine_x = dac_data[x_ch];
      fine_y = dac_data[y_ch];
      pre_fine();
      if(fine_auto_ramp)
        CheckDlgButton(hDlg,FINE_AUTO_RAMP,1);
      SetScrollRange(GetDlgItem(hDlg,FINE_X_SCROLL),SB_CTL,fine_x_min,fine_x_max,FALSE);
      SetScrollRange(GetDlgItem(hDlg,FINE_Y_SCROLL),SB_CTL,fine_y_min,fine_y_max,FALSE);
      SetFocus(GetDlgItem(hDlg,FINE_EXIT));
      image_data = (unsigned char *) malloc(IMAGESIZE*IMAGESIZE*sizeof(unsigned char));
      pLogPal = (NPLOGPALETTE) GlobalAlloc(LMEM_FIXED,(sizeof(LOGPALETTE)+(sizeof(PALETTEENTRY)*(PALETTESIZE))));
      pLogPal->palVersion = 0x300;
      pLogPal->palNumEntries = PALETTESIZE;
      for(i=0;i<PALETTESIZE;i++) {
        pLogPal->palPalEntry[i].peRed = (BYTE) i/0.9;
        pLogPal->palPalEntry[i].peGreen = (BYTE) i;
        pLogPal->palPalEntry[i].peBlue = (BYTE) i/0.9;
        pLogPal->palPalEntry[i].peFlags = NULL;
      }
      hPal = CreatePalette(pLogPal);
      hDC = GetDC(hDlg);
      SelectPalette(hDC,hPal,0);
      RealizePalette(hDC);
      ReleaseDC(hDlg,hDC);
      for(i=0;i<IMAGESIZE;i++)
        for(j=0;j<IMAGESIZE;j++)
      *(image_data+j*IMAGESIZE+i) = (BYTE) ((unsigned long) *(data->ptr+j*IMAGESIZE+i)*150/65536+63);
      hImage = CreateBitmap(IMAGESIZE,IMAGESIZE,1,8,image_data);
      CreateCaret(hDlg,hCross,15,15);
      repaint_x(hDlg);
      repaint_y(hDlg);
      repaint_auto_ramp(hDlg);
      break;
    case WM_PAINT:
      hDC = BeginPaint(hDlg,&PtStr);
      hMemDC = CreateCompatibleDC(hDC);
      SelectObject(hMemDC,hImage);
      BitBlt(hDC,IMAGE_X_POS,IMAGE_Y_POS,IMAGESIZE,IMAGESIZE,hMemDC,0,0,SRCCOPY);
      SelectObject(hMemDC,hCross);
      BitBlt(hDC,fine_x*(IMAGESIZE-1)/fine_x_max+30.5,fine_y*(IMAGESIZE-1)/fine_y_max+30.5,15,15,hMemDC,0,0,SRCPAINT);
      DeleteDC(hMemDC);
      EndPaint(hDlg,&PtStr);
      break;
    case WM_LBUTTONDOWN:
        mouse_pos.x = LOWORD(lParam);
        mouse_pos.y = HIWORD(lParam);
      mouse_pos.x -= IMAGE_X_POS;
      mouse_pos.y -= IMAGE_Y_POS;
      if(mouse_pos.x>=0 && mouse_pos.x<IMAGESIZE &&
         mouse_pos.y>=0 && mouse_pos.y<IMAGESIZE) {
        fine_x = mouse_pos.x;
        fine_x = fine_x*fine_x_max/(IMAGESIZE-1)+0.5;
        fine_y = mouse_pos.y;
        fine_y = fine_y*fine_y_max/(IMAGESIZE-1)+0.5;
      }
      repaint_x(hDlg);
      repaint_y(hDlg);
      break;
    case WM_HSCROLL:
    case WM_VSCROLL:
      id = getscrollid();
      switch(id) {
        case FINE_X_SCROLL:
          i =  (int) fine_x;
          mini = fine_x_min;
          maxi = fine_x_max;
          delt = fine_x_delt;
          ddelt = fine_x_ddelt;
          break;
        case FINE_Y_SCROLL:
          i =  (int) fine_y;
          mini = fine_y_min;
          maxi = fine_y_max;
          delt = fine_y_delt;
          ddelt = fine_y_ddelt;
          break;
      }
      switch(getscrollcode()) {
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
      }
      i = min(max(i,mini),maxi);
      switch(id) {
        case FINE_X_SCROLL:
          fine_x = (unsigned int) i;
          repaint_x(hDlg);
          break;
        case FINE_Y_SCROLL:
          fine_y = (unsigned int) i;
          repaint_y(hDlg);
          break;
      }
      break;
    case WM_COMMAND:
      switch (LOWORD(wParam)) {
        case FINE_X_BITS_EDIT:
          GetDlgItemText(hDlg,FINE_X_BITS_EDIT,string,5);
          i = atoi(string);
          i = min(max(i,fine_x_min),fine_x_max);
          fine_x = (unsigned int) i;
          break;
        case FINE_X_VOLTS_EDIT:
          GetDlgItemText(hDlg,FINE_X_VOLTS_EDIT,string,9);
      i = vtod(atof(string),x_range);
          i = min(max(i,fine_x_min),fine_x_max);
          fine_x = (unsigned int) i;
          break;
        case FINE_X_DISTANCE_EDIT:
          GetDlgItemText(hDlg,FINE_X_DISTANCE_EDIT,string,9);
          i = (int) (ZERO+atof(string)/(double) ANGS_PER_BIT+0.5);
          i = min(max(i,fine_x_min),fine_x_max);
          fine_x = (unsigned int) i;
          break;
        case FINE_Y_BITS_EDIT:
          GetDlgItemText(hDlg,FINE_Y_BITS_EDIT,string,5);
          i = atoi(string);
          i = min(max(i,fine_y_min),fine_y_max);
          fine_y = (unsigned int) i;
          break;
        case FINE_Y_VOLTS_EDIT:
          GetDlgItemText(hDlg,FINE_Y_VOLTS_EDIT,string,9);
      i = vtod(atof(string),y_range);
          i = min(max(i,fine_y_min),fine_y_max);
          fine_y = (unsigned int) i;
          break;
        case FINE_Y_DISTANCE_EDIT:
          GetDlgItemText(hDlg,FINE_Y_DISTANCE_EDIT,string,9);
          i = (int) (ZERO+atof(string)/(double) ANGS_PER_BIT+0.5);
          i = min(max(i,fine_y_min),fine_y_max);
          fine_y = (unsigned int) i;
          break;
        case ENTER:
          SetFocus(GetDlgItem(hDlg,FINE_EXIT));
          repaint_x(hDlg);
          repaint_y(hDlg);
          break;
        case FINE_X_RAMP:
          move_to(x_ch,dac_data[x_ch],fine_x);
          dac_data[x_ch] = fine_x;
          break;
        case FINE_X_JUMP:
          dio_out(x_ch,fine_x);
          dac_data[x_ch] = fine_x;
          break;
        case FINE_X_ZERO:
          move_to(x_ch,dac_data[x_ch],ZERO);
          dac_data[x_ch] = ZERO;
          fine_x = ZERO;
          repaint_x(hDlg);
          break;
        case FINE_Y_RAMP:
          move_to(y_ch,dac_data[y_ch],fine_y);
          dac_data[y_ch] = fine_y;
          break;
        case FINE_Y_JUMP:
          dio_out(y_ch,fine_y);
          dac_data[y_ch] = fine_y;
          break;
        case FINE_Y_ZERO:
          move_to(y_ch,dac_data[y_ch],ZERO);
          dac_data[y_ch] = ZERO;
          fine_y = ZERO;
          repaint_y(hDlg);
          break;
        case FINE_AUTO_RAMP:
          repaint_auto_ramp(hDlg);
          break;
        case FINE_EXIT:
          post_fine();
          free(image_data);
          GlobalFree((HGLOBAL) pLogPal);
          DeleteObject(hPal);
          DeleteObject(hImage);
          DestroyCaret();
          EndDialog(hDlg,TRUE);
          return(TRUE);
      }
      break;
  }
  return(FALSE);
}
void repaint_cross(HWND hDlg)
{
  HDC           hDC;
  HDC           hMemDC;
  
  hDC = GetDC(hDlg);
  hMemDC = CreateCompatibleDC(hDC);
  SelectObject(hMemDC,hImage);
  BitBlt(hDC,IMAGE_X_POS,IMAGE_Y_POS,IMAGESIZE,IMAGESIZE,hMemDC,0,0,SRCCOPY);
  SelectObject(hMemDC,hCross);
  BitBlt(hDC,fine_x*(IMAGESIZE-1)/fine_x_max+30.5,fine_y*(IMAGESIZE-1)/fine_y_max+30.5,15,15,hMemDC,0,0,SRCPAINT);
  DeleteDC(hMemDC);
  ReleaseDC(hDlg,hDC);
}

void repaint_x(HWND hDlg)
{
  if(IsDlgButtonChecked(hDlg,FINE_AUTO_RAMP)) {
    move_to(x_ch,dac_data[x_ch],fine_x);
    dac_data[x_ch] = fine_x;
  }
  SetDlgItemInt(hDlg,FINE_X_BITS_EDIT,fine_x,FALSE);
  SetScrollPos(GetDlgItem(hDlg,FINE_X_SCROLL),SB_CTL,fine_x,TRUE);
  SetDlgItemText(hDlg,FINE_X_VOLTS_EDIT,gcvt(dtov(fine_x,x_range),5,string));
  SetDlgItemText(hDlg,FINE_X_DISTANCE_EDIT,gcvt(((double) fine_x-ZERO)*ANGS_PER_BIT,5,string));
  repaint_cross(hDlg);
}

void repaint_y(HWND hDlg)
{
  if(IsDlgButtonChecked(hDlg,FINE_AUTO_RAMP)) {
    move_to(y_ch,dac_data[y_ch],fine_y);
    dac_data[y_ch] = fine_y;
  }
  SetDlgItemInt(hDlg,FINE_Y_BITS_EDIT,fine_y,FALSE);
  SetScrollPos(GetDlgItem(hDlg,FINE_Y_SCROLL),SB_CTL,fine_y,TRUE);
  SetDlgItemText(hDlg,FINE_Y_VOLTS_EDIT,gcvt(dtov(fine_y,y_range),5,string));
  SetDlgItemText(hDlg,FINE_Y_DISTANCE_EDIT,gcvt(((double) fine_y-ZERO)*ANGS_PER_BIT,5,string));
  repaint_cross(hDlg);
}

void repaint_auto_ramp(HWND hDlg)
{
  if(IsDlgButtonChecked(hDlg,FINE_AUTO_RAMP)) {
    EnableWindow(GetDlgItem(hDlg,FINE_X_RAMP),FALSE);
    EnableWindow(GetDlgItem(hDlg,FINE_X_JUMP),FALSE);
    EnableWindow(GetDlgItem(hDlg,FINE_Y_RAMP),FALSE);
    EnableWindow(GetDlgItem(hDlg,FINE_Y_JUMP),FALSE);
  }
  else {
    EnableWindow(GetDlgItem(hDlg,FINE_X_RAMP),TRUE);
    EnableWindow(GetDlgItem(hDlg,FINE_X_JUMP),TRUE);
    EnableWindow(GetDlgItem(hDlg,FINE_Y_RAMP),TRUE);
    EnableWindow(GetDlgItem(hDlg,FINE_Y_JUMP),TRUE);
  }
}

void pre_fine()
{
  move_to(x_offset_ch,dac_data[x_offset_ch],ZERO);
  move_to(y_offset_ch,dac_data[y_offset_ch],ZERO);
  move_to(z_offset_ch,dac_data[z_offset_ch],ZERO);
  set_gain(X_TEN_GAIN,Y_TEN_GAIN,Z_ONE_GAIN,Z2_ONE_GAIN);
  mode(fine_mode);
  mode(translate_mode);
  hold(FALSE);
}

void post_fine()
{
  move_to(z_offset_ch,ZERO,dac_data[z_offset_ch]);
  move_to(y_offset_ch,ZERO,dac_data[y_offset_ch]);
  move_to(x_offset_ch,ZERO,dac_data[x_offset_ch]);
}
