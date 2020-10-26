#include <math.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <dos.h>
#include "common.h"
#include "file.h"
#include "data.h"
#include "stm.h"
#include "dio.h"
#include "heater.h"
#include "clock.h"

extern HANDLE   hInst;
extern char     string[];

extern struct commentdef *gcomment;

extern unsigned int input_ch,out1;
static out_smart=0;
static HPEN hpen_black,hpen_red,hpen_green,hpen_blue,hpen_old,hpen_dash,hpen_white;
static HRGN heater_area;
static PAINTSTRUCT ps;
heaterdata heater_data;
int heater_first_time=1;

extern BOOL FAR PASCAL CommentDlg(HWND, unsigned, WPARAM, LPARAM);

BOOL FAR PASCAL HeaterDlg(HWND, unsigned, WPARAM, LPARAM);

BOOL FAR PASCAL HeaterDlg(HWND hDlg, unsigned Message, WPARAM wParam, LPARAM lParam)
{
    int id,result;
    long int i,mini,maxi,delt,ddelt;
    double d,minid,maxid,deltd,ddeltd;
    static FARPROC lpfnDlgProc;
//    POINT           mouse_pos;
    
    switch(Message)
    {
    case WM_INITDIALOG:
        /* cfg3 has a value of 0x2400 before we begin */
        if (clock_used)
        {
            MessageBox(hDlg, "Clock already in use!", "Oops", MB_ICONEXCLAMATION);
            EndDialog(hDlg,TRUE);
            return(TRUE);
        }
        else
        {
            if (heater_first_time)
            {
                heater_first_time=0;
                heater_data.comment.ptr=(char *) malloc(sizeof(heater_data.comment.ptr[0])*COMMENTMAXSIZE);
                heater_data.data=NULL;
            }
            
/*
            SetScrollRange(GetDlgItem(hDlg, M_MIN_SCROLL), SB_CTL, 0,32767, TRUE);
            
            repaint_screen_scroll(hDlg);
*/
            
            hpen_black=CreatePen(PS_SOLID,1,RGB(0,0,0));
            hpen_red=CreatePen(PS_SOLID,1,RGB(255,0,0));
            hpen_green=CreatePen(PS_SOLID,1,RGB(0,255,0));
            hpen_blue=CreatePen(PS_SOLID,1,RGB(0,0,255));
            hpen_black=CreatePen(PS_SOLID,1,RGB(0,0,0));
            hpen_white=CreatePen(PS_SOLID,1,RGB(255,255,255));
            hpen_dash=CreatePen(PS_DASH,1,RGB(0,0,0));
            heater_area=CreateRectRgn(HEATER_X_MIN,HEATER_Y_MIN,HEATER_X_MAX,HEATER_Y_MAX);

        }    
    case WM_PAINT:
        BeginPaint(hDlg,&ps);
        if (RectInRegion(heater_area,&(ps.rcPaint)))  
        {
/*
            repaint_graphics(hDlg);
*/
        }
        EndPaint(hDlg,&ps);
        break;
#ifdef OLD
    case WM_LBUTTONDOWN:
        mouse_pos.x = LOWORD(lParam);
        mouse_pos.y = HIWORD(lParam);
        if (mouse_pos.x >= M_X_MIN && mouse_pos.x <= M_X_MAX &&
            mouse_pos.y >= M_Y_MIN && mouse_pos.y <= M_Y_MAX)
        {
            m_p1_vbits = (mouse_pos.x-M_X_MIN)*(m_screen_max-m_screen_min+1)/
                        (M_X_MAX-M_X_MIN+1)+m_screen_min;
            repaint_graphics(hDlg);
            repaint_integral(hDlg);
                    
        }
        break;
    case WM_RBUTTONDOWN:
        mouse_pos = MAKEPOINT(lParam);
        if (mouse_pos.x >= M_X_MIN && mouse_pos.x <= M_X_MAX &&
            mouse_pos.y >= M_Y_MIN && mouse_pos.y <= M_Y_MAX)
        {
            m_p2_vbits = (mouse_pos.x-M_X_MIN)*(m_screen_max-m_screen_min+1)/
                        (M_X_MAX-M_X_MIN+1)+m_screen_min;
            repaint_graphics(hDlg);
            repaint_integral(hDlg);
                    
        }
        break;
#endif
    case WM_VSCROLL:
    case WM_HSCROLL:
        if (!out_smart)
        {
            out_smart=1;
            id = getscrollid();
            switch (id)
            {
/*
                case M_MIN_SCROLL:
                    d=(double) m_scan_min;
                    minid=M_MIN_MIN;
                    maxid=m_scan_max-1;
                    deltd=m_amu_to_v(M_SCAN_DELT)*409.6/m_out_range;
                    ddeltd=m_amu_to_v(M_SCAN_DDELT)*409.6/m_out_range;
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
                case M_MIN_SCROLL:
                    m_scan_min = (int) floor(d+0.5);
                    repaint_scan_min(hDlg);
                    break;
                    
*/
            }
            out_smart=0;
        }
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
/*
        case M_INPUT_CH:
            if (!out_smart)
            {
                out_smart=1;
                GetDlgItemText(hDlg, M_INPUT_CH, string, 9);
                m_input_ch = atoi(string);
                m_input_ch = min(max(m_input_ch,0),3);
                out_smart=0;
            }
            break;
*/
        case HEATER_LOAD:
            result = file_open(hDlg, hInst,FTYPE_TP,0,NULL);
            if (result)
            {
/*
                m_data_valid=1;
                m_out_range = m_data.v_range;
                m_speed = m_data.speed;
*/
            }
            break;
        case HEATER_SAVE:
            file_save_as(hDlg,hInst,FTYPE_TP);
            break;    
        case HEATER_COMMENT:
            gcomment=&(heater_data.comment);
            lpfnDlgProc = MakeProcInstance(CommentDlg, hInst);
            DialogBox(hInst, "COMMENTDLG", hDlg, lpfnDlgProc);
            FreeProcInstance(lpfnDlgProc);
            break;
        case ENTER:
/*
            repaint_scan_min(hDlg);
            repaint_scan_max(hDlg);
            repaint_p1(hDlg);
            repaint_speed(hDlg);
*/
            break;
        case HEATER_EXIT:
            outpw(cfg3,0x2400);
            outp(cntrcmd,0xB4); /* cntr3 in rate generation mode */
            clock_used=0;
            DeleteObject(hpen_black);
            DeleteObject(hpen_white);
            DeleteObject(hpen_red);
            DeleteObject(hpen_green);
            DeleteObject(hpen_blue);
            DeleteObject(hpen_dash);
            DeleteObject(heater_area);
            EndDialog(hDlg, TRUE);
            return (TRUE);
            break;
        }
        break;
    }
    return (FALSE);
}                                                                  

