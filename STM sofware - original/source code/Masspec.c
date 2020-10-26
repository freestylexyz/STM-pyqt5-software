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
#include "masspec.h"
#include "oscill.h"
#include "clock.h"
#include "spec.h"

extern HANDLE   hInst;
extern char     string[];

extern struct commentdef *gcomment;

extern unsigned int input_ch,out1;
static out_smart=0;
static HPEN hpen_black,hpen_red,hpen_green,hpen_blue,hpen_old,hpen_dash,hpen_white;
static HRGN masspec_area;
static PAINTSTRUCT ps;
int m_scan_min;
int m_scan_max;
float m_scan_speed=1.0;
int m_out_range; /* 1 is +-5 V, 2 is +-10V */
int m_input_ch=M_IN_CH;
int m_output_ch=M_OUT_CH;
int m_p1_vbits=M_MIN_MIN;
int m_p2_vbits=M_MAX_MAX;
int m_background=IN_ZERO;
float m_p1_press=0;
float m_p2_press=0;
float m_p1_calib=-1;
float m_p2_calib=-1;
float m_speed=0.5;
float m_intercept;  /* in volts */
float m_slope;      /* in volts/amu */
int m_tot_integral=0;
int m_part_integral=0;
int m_interval=0; /* interval between steps in microsec */
int m_data_valid=0;
int m_screen_min;
int m_screen_max;
int m_scanning=0;
int m_first_time=1;
int m_scale=11;

datadef *m_data;
extern datadef **glob_data;
unsigned int m_vert_max=M_VERT_MAX;
extern datadef *data;
extern char *current_file_mas;
extern int file_mas_count;

extern BOOL FAR PASCAL CommentDlg(HWND, unsigned, WPARAM, LPARAM);

BOOL FAR PASCAL MassDlg(HWND, unsigned, WPARAM, LPARAM);

void repaint_channels(HWND);
void repaint_scan_min(HWND);
void repaint_scan_max(HWND);
void repaint_speed(HWND);
void repaint_calib(HWND);
void repaint_background(HWND);
void repaint_p1(HWND);
void repaint_graphics(HWND);
void repaint_p2(HWND);
void repaint_vert_scale(HWND);
void repaint_spec_scale(HWND);
void repaint_screen_scroll(HWND);
void repaint_integral(HWND);
void get_calib();
void m_scan_enable(HWND);
void m_scan_disable(HWND);
void calc_integral();
void calc_part_integral();
void m_calibrate();
void find_background();

BOOL FAR PASCAL MassDlg(HWND hDlg, unsigned Message, WPARAM wParam, LPARAM lParam)
{
    int id,result;
    POINT mouse_pos;
    long int i,mini,maxi,delt,ddelt;
    int xs,ys,start,end,done;
    float yfs;
    double d,minid,maxid,deltd,ddeltd;
    unsigned int data_max;
    unsigned int unsigned_data;
    MSG msg;
    HDC masspec_hdc;
    time_t scan_time;
    static FARPROC lpfnDlgProc;
    
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
            get_calib();
            if (m_first_time)
            {
                m_first_time=0;
                alloc_data(&m_data,DATATYPE_MASS,SP_NUM_CH,GEN2D_NONE,GEN2D_NONE,0);
                m_data->version=SP_DATA_VERSION;
                m_data->bias=0;
                m_data->dz_offset=0;
                m_data->samples=1;
                m_data->skip=0;
            }

            SetScrollRange(GetDlgItem(hDlg, M_MIN_SCROLL), SB_CTL, 0,32767, TRUE);
            SetScrollRange(GetDlgItem(hDlg, M_MAX_SCROLL), SB_CTL, 0,32767, TRUE);
            SetScrollRange(GetDlgItem(hDlg, M_AMU_SCROLL), SB_CTL, 0,32767, TRUE);
            SetScrollRange(GetDlgItem(hDlg, M_P1_SCROLL), SB_CTL, 0,32767, TRUE);
            SetScrollRange(GetDlgItem(hDlg, M_P2_SCROLL), SB_CTL, 0,32767, TRUE);
            SetScrollRange(GetDlgItem(hDlg, M_SCREEN_SCROLL), SB_CTL, 0,32767, TRUE);
            SetScrollRange(GetDlgItem(hDlg, M_V_SCALE_SCROLL), SB_CTL, 0,32767, TRUE);
            m_out_range=1; /* +- 5 V */
            set_range(m_output_ch,m_out_range);
//            m_out_range=get_range(m_output_ch);
            m_scan_max=vtod(m_amu_to_v(45),m_out_range);
            m_scan_min=vtod(m_amu_to_v(3),m_out_range);
/*
            switch(m_out_range)
            {
                case 1:
                    CheckDlgButton(hDlg,M_5V,1);
                    break;
                case 2:
                    CheckDlgButton(hDlg,M_10V,1);
                    break;
            }
*/
            repaint_calib(hDlg);
            repaint_background(hDlg);
            repaint_scan_min(hDlg);
            repaint_scan_max(hDlg);
            repaint_speed(hDlg);
            repaint_channels(hDlg);
            repaint_p1(hDlg);
            repaint_p2(hDlg);
            repaint_integral(hDlg);
            repaint_vert_scale(hDlg);
            repaint_screen_scroll(hDlg);
            hpen_black=CreatePen(PS_SOLID,1,RGB(0,0,0));
            hpen_red=CreatePen(PS_SOLID,1,RGB(255,0,0));
            hpen_green=CreatePen(PS_SOLID,1,RGB(0,255,0));
            hpen_blue=CreatePen(PS_SOLID,1,RGB(0,0,255));
            hpen_black=CreatePen(PS_SOLID,1,RGB(0,0,0));
            hpen_white=CreatePen(PS_SOLID,1,RGB(255,255,255));
            hpen_dash=CreatePen(PS_DASH,1,RGB(0,0,0));
//            hpen_old=SelectObject(oscill_hdc,hpen_black);
            masspec_area=CreateRectRgn(M_X_MIN,M_Y_MIN,M_X_MAX,M_Y_MAX);

        }    
    case WM_PAINT:
        BeginPaint(hDlg,&ps);
        if (RectInRegion(masspec_area,&(ps.rcPaint)))  
        {
            repaint_graphics(hDlg);
        }
        EndPaint(hDlg,&ps);
        break;
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
        mouse_pos.x  = LOWORD(lParam);
        mouse_pos.y  = HIWORD(lParam);
        if (mouse_pos.x >= M_X_MIN && mouse_pos.x <= M_X_MAX &&
            mouse_pos.y >= M_Y_MIN && mouse_pos.y <= M_Y_MAX)
        {
            m_p2_vbits = (mouse_pos.x-M_X_MIN)*(m_screen_max-m_screen_min+1)/
                        (M_X_MAX-M_X_MIN+1)+m_screen_min;
            repaint_graphics(hDlg);
            repaint_integral(hDlg);
                    
        }
        break;
    case WM_VSCROLL:
    case WM_HSCROLL:
        if (!out_smart)
        {
            out_smart=1;
            id = getscrollid();
            switch (id)
            {
                case M_MIN_SCROLL:
                    d=(double) m_scan_min;
                    minid=M_MIN_MIN;
                    maxid=m_scan_max-1;
                    deltd=m_amu_to_v(M_SCAN_DELT)*409.6/m_out_range;
                    ddeltd=m_amu_to_v(M_SCAN_DDELT)*409.6/m_out_range;
                    break;
                case M_MAX_SCROLL:
                    d=(double) m_scan_max;
                    if (!m_scanning)
                        minid=m_scan_min+1;
                    else    
                        minid=i;
                    maxid=M_MAX_MAX;
                    deltd=m_amu_to_v(M_SCAN_DELT)*409.6/m_out_range;
                    ddeltd=m_amu_to_v(M_SCAN_DDELT)*409.6/m_out_range; 
                    break;
                case M_AMU_SCROLL:
                    d=(double) m_speed;
                    minid=M_SPEED_MIN;
                    maxid=M_SPEED_MAX;
                    deltd=M_SPEED_DELT;
                    ddeltd=M_SPEED_DDELT;
                    break;
                case M_V_SCALE_SCROLL:
                    d = (double) m_vert_max;
                    minid=M_VERT_MIN;
                    maxid=M_VERT_MAX;
                    deltd=M_VERT_DELT;
                    ddeltd=M_VERT_DDELT;
                    break;
                case M_P1_SCROLL:
                    d = (double) m_p1_vbits;
                    minid=ZERO;
                    maxid=ZERO+M_NUM_CH-1;
                    deltd=1;
                    ddeltd=(double)(m_screen_max-m_screen_min)/10;
                    break;
                case M_P2_SCROLL:
                    d = (double) m_p2_vbits;
                    minid=ZERO;
                    maxid=ZERO+M_NUM_CH-1;
                    deltd=1;
                    ddeltd=(double)(m_screen_max-m_screen_min)/10;
                    break;
                case M_SCREEN_SCROLL:
                    d = (double) m_screen_min;
                    minid=m_data->start;
                    maxid=m_data->end-(m_screen_max-m_screen_min);
                    deltd=1;
                    ddeltd=(double)(m_screen_max-m_screen_min)/10;
                    break;
    
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
                case M_MIN_SCROLL:
                    m_scan_min = (int) floor(d+0.5);
                    repaint_scan_min(hDlg);
                    break;
                    
                case M_MAX_SCROLL:
                    m_scan_max = (int) floor(d+0.5);
                    repaint_scan_max(hDlg);
                    break;
                    
                case M_AMU_SCROLL:
                    m_speed = (float) d;
                    repaint_speed(hDlg);
                    break;
                case M_V_SCALE_SCROLL:
                    m_vert_max = d;
                    repaint_vert_scale(hDlg);
                    repaint_graphics(hDlg);
                    break;
                case M_P1_SCROLL:
                    m_p1_vbits = (unsigned int) d;
                    SetScrollPos(GetDlgItem(hDlg, M_P1_SCROLL), SB_CTL, DTOI(m_p1_vbits,M_MIN_MIN,M_MAX_MAX), TRUE);
                    sprintf(string,"%0.1f", m_v_to_amu(dtov(m_p1_vbits,m_out_range)));
                    SetDlgItemText(hDlg, M_P1_MASS, string);
                    repaint_integral(hDlg);
                    repaint_graphics(hDlg);
                    break;
                case M_P2_SCROLL:
                    m_p2_vbits = (unsigned int) d;
                    SetScrollPos(GetDlgItem(hDlg, M_P2_SCROLL), SB_CTL, DTOI(m_p2_vbits,M_MIN_MIN,M_MAX_MAX), TRUE);
                    sprintf(string,"%0.1f", m_v_to_amu(dtov(m_p2_vbits,m_out_range)));
                    SetDlgItemText(hDlg, M_P2_MASS, string);
                    repaint_integral(hDlg);
                    repaint_graphics(hDlg);
                    break;
                case M_SCREEN_SCROLL:
                    m_screen_max += (unsigned int) d-m_screen_min;
                    m_screen_min = (unsigned int) d;
                    repaint_screen_scroll(hDlg);
                    repaint_graphics(hDlg);
                    break;
            }
            out_smart=0;
        }
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case M_BACKGROUND:
            if (!out_smart)
            {
                out_smart=1;
                GetDlgItemText(hDlg, M_BACKGROUND, string, 9);
                m_background=in_vtod(atof(string));
                
                out_smart=0;
            }
            break;
        case M_MIN_RANGE:
            if (!out_smart)
            {
                out_smart=1;
                GetDlgItemText(hDlg, M_MIN_RANGE, string, 9);
                m_scan_min = (unsigned int) vtod(m_amu_to_v(atof(string)), m_out_range);
                m_scan_min = min(max(m_scan_min,M_MIN_MIN),M_MIN_MAX);
                SetScrollPos(GetDlgItem(hDlg, M_MIN_SCROLL), SB_CTL, DTOI(m_scan_min,M_MIN_MIN,M_MIN_MAX), TRUE);
                out_smart=0;
            }
            break;
        case M_MAX_RANGE:
            if (!out_smart)
            {
                out_smart=1;
                GetDlgItemText(hDlg, M_MAX_RANGE, string, 9);
                m_scan_max = (unsigned int) vtod(m_amu_to_v(atof(string)), m_out_range);
                m_scan_max = min(max(m_scan_max,M_MAX_MIN),M_MAX_MAX);
                SetScrollPos(GetDlgItem(hDlg, M_MAX_SCROLL), SB_CTL, DTOI(m_scan_max,M_MAX_MIN,M_MAX_MAX), TRUE);
                out_smart=0;
            }
            break;
        case M_SPEED:
            if (!out_smart)
            {
                out_smart=1;
                GetDlgItemText(hDlg, M_SPEED, string, 9);
                m_speed = atof(string);
                m_speed = min(max(m_speed,M_SPEED_MIN),M_SPEED_MAX);
                SetScrollPos(GetDlgItem(hDlg, M_AMU_SCROLL), SB_CTL, DTOI(m_speed,M_SPEED_MIN,M_SPEED_MAX), TRUE);
                out_smart=0;
            }
            break;
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
        case M_OUTPUT_CH:
            if (!out_smart)
            {
                out_smart=1;
                GetDlgItemText(hDlg, M_OUTPUT_CH, string, 9);
                m_output_ch = atoi(string);
                m_output_ch = min(max(m_output_ch,0),11);
                out_smart=0;
            }
            break;
        case M_V_SCALE:
            if (!out_smart)
            {
                out_smart=1;
                GetDlgItemText(hDlg, M_V_SCALE, string, 9);
                m_vert_max = (unsigned int) (atof(string)/20*(float)IN_MAX);
                m_vert_max = min(max(m_vert_max,M_VERT_MIN),M_VERT_MAX);
                SetScrollPos(GetDlgItem(hDlg, M_V_SCALE_SCROLL), SB_CTL,DTOI(m_vert_max,M_VERT_MIN,M_VERT_MAX) , TRUE);
                repaint_graphics(hDlg);
                out_smart=0;
            }
            break;
        case M_P1_MASS:
            if (!out_smart)
            {
                out_smart=1;
                GetDlgItemText(hDlg, M_P1_MASS, string, 9);
                m_p1_vbits = (unsigned int) vtod(m_amu_to_v(atof(string)),m_out_range);
                m_p1_vbits = min(max(m_p1_vbits,M_MIN_MIN),M_MAX_MAX);
                SetScrollPos(GetDlgItem(hDlg, M_P1_SCROLL), SB_CTL, DTOI(m_p1_vbits,M_MIN_MIN,M_MAX_MAX), TRUE);
                repaint_integral(hDlg);
                repaint_graphics(hDlg);
                out_smart=0;
            }
            break;
        case M_P2_MASS:
            if (!out_smart)
            {
                out_smart=1;
                GetDlgItemText(hDlg, M_P2_MASS, string, 9);
                m_p2_vbits = (unsigned int) vtod(m_amu_to_v(atof(string)),m_out_range);
                m_p2_vbits = min(max(m_p2_vbits,M_MIN_MIN),M_MAX_MAX);
                SetScrollPos(GetDlgItem(hDlg, M_P2_SCROLL), SB_CTL, DTOI(m_p2_vbits,M_MIN_MIN,M_MAX_MAX), TRUE);
                repaint_integral(hDlg);
                repaint_graphics(hDlg);
                out_smart=0;
            }
            break;
        case M_P1_CALIB:
            if (!out_smart)
            {
                out_smart=1;
                GetDlgItemText(hDlg, M_P1_CALIB, string, 9);
                m_p1_calib = atof(string);
                m_p1_calib = min(max(m_p1_calib,0),200);
                out_smart=0;
            }
            break;
        case M_P2_CALIB:
            if (!out_smart)
            {
                out_smart=1;
                GetDlgItemText(hDlg, M_P2_CALIB, string, 9);
                m_p2_calib = atof(string);
                m_p2_calib = min(max(m_p2_calib,0),200);
                out_smart=0;
            }
            break;
        case M_SPEC_SCALE:
            if (!out_smart)
            {
                out_smart=1;
                GetDlgItemText(hDlg, M_SPEC_SCALE, string, 9);
                m_scale = atoi(string);
                out_smart=0;
            }
            break;
        case M_CALIBRATE:
            m_calibrate();
            repaint_scan_min(hDlg);
            repaint_scan_max(hDlg);
            repaint_calib(hDlg);
            repaint_p1(hDlg);
            repaint_p2(hDlg);
            break;
        case M_LOAD_CALIB:
            get_calib();
            repaint_scan_min(hDlg);
            repaint_scan_max(hDlg);
            repaint_calib(hDlg);
            repaint_p1(hDlg);
            repaint_p2(hDlg);
            break;
        case M_BACK_FIND:
            if (m_p1_vbits<m_data->start || m_p1_vbits>m_data->end
                || m_p2_vbits<m_data->start || m_p2_vbits>m_data->end)
            {
            }
            else find_background();
            calc_integral();
            calc_part_integral();
            repaint_integral(hDlg);
            repaint_background(hDlg);
            break;
            
        
        case M_LOAD:
            glob_data=&m_data;
            result = file_open(hDlg, hInst,FTYPE_MAS,0,current_file_mas);
            if (result)
            {
                m_data_valid=1;
                m_out_range = m_data->v_range;
                m_speed = m_data->speed;
                m_intercept = m_data->calib_int;
                m_slope = m_data->calib_slope;
                m_scale= m_data->scale;
                m_p1_vbits = m_screen_min = m_scan_min = m_data->start;
                m_p2_vbits = m_screen_max = m_scan_max = m_data->end;
                m_vert_max=M_VERT_MAX;
/*
            sprintf(string,"min %ld  max %ld", m_scan_min,m_scan_max);
            MessageBox(hDlg, string, "Test",MB_ICONEXCLAMATION);
*/
                calc_integral();
                repaint_spec_scale(hDlg);
                repaint_calib(hDlg);
                repaint_p1(hDlg);
                repaint_p2(hDlg);
                repaint_calib(hDlg);
                repaint_scan_min(hDlg);
                repaint_scan_max(hDlg);
                repaint_speed(hDlg);
                repaint_channels(hDlg);
                repaint_graphics(hDlg);
                repaint_integral(hDlg);
                repaint_vert_scale(hDlg);
            }
            break;
        case M_ZOOM_IN:
            if (m_p1_vbits<m_p2_vbits)
            {
                m_screen_min=m_p1_vbits;
                m_screen_max=m_p2_vbits;
            }
            else if (m_p2_vbits<m_p1_vbits)
            {
                m_screen_min=m_p2_vbits;
                m_screen_max=m_p1_vbits;
            }
            repaint_screen_scroll(hDlg);
            repaint_graphics(hDlg);
            break;

        case M_ZOOM_OUT:
            m_screen_min=m_data->start;
            if (!m_scanning)
            {
                m_screen_max=m_data->end;
            }
            else
            {
                m_screen_max=m_scan_max;
            }
         
            repaint_screen_scroll(hDlg);
            repaint_graphics(hDlg);
            break;
        case M_P1_PEAK:
            if (m_data_valid)
            {
                if (m_p1_vbits>m_data->start) 
                {
                    i=m_p1_vbits;
                    if (m_data->ptr[i-1]<m_data->ptr[i] &&
                        m_data->ptr[i]>m_data->ptr[i+1]) i++;
                }
                else i=m_data->start+1;
                done=0;
                while (!done && i<m_data->end-1)
                {
                    if (m_data->ptr[i-1]<m_data->ptr[i] &&
                        m_data->ptr[i]>m_data->ptr[i+1])
                    done=1;
                    else i++;
                }
                if (i==m_data->end-1 && m_data->ptr[end]>m_data->ptr[i])
                i++;
                if (done) m_p1_vbits=i;
                repaint_graphics(hDlg);
            }
            break;
        case M_P2_PEAK:
            if (m_data_valid)
            {
                if (m_p2_vbits>m_data->start) 
                {
                    i=m_p2_vbits;
                    if (m_data->ptr[i-1]<m_data->ptr[i] &&
                        m_data->ptr[i]>m_data->ptr[i+1]) i++;
                }
                else i=m_data->start+1;
                done=0;
                while (!done && i<m_data->end-1)
                {
                    if (m_data->ptr[i-1]<m_data->ptr[i] &&
                        m_data->ptr[i]>m_data->ptr[i+1])
                    done=1;
                    else i++;
                }
                if (i==m_data->end-1 && m_data->ptr[end]>m_data->ptr[i])
                i++;
                if (done) m_p2_vbits=i;
                repaint_graphics(hDlg);
            }
            break;
        case M_ZOOM_PEAK:
            if (m_screen_min>m_data->start) start=m_screen_min;
            else start=m_data->start;
            if (m_screen_max<m_data->end) end=m_screen_max;
            else end=m_data->end;
            data_max=IN_MAX/2-1;
            for(i=start;i<=end;i++)
            {
                if (m_data->ptr[i]>data_max) data_max=m_data->ptr[i];
            }
            if (data_max>IN_MAX/2-1)  m_vert_max=data_max-IN_MAX/2;
            sprintf(string,"max %ld ", m_vert_max);
            repaint_vert_scale(hDlg);
            repaint_graphics(hDlg);
            break;
            
        case M_SAVE:
            glob_data=&m_data;
            if (file_save_as(hDlg,hInst,FTYPE_MAS))
                inc_file(current_file_mas,&file_mas_count);
            break;    
            
        case M_COMMENT:
            gcomment=&(m_data->comment);
            lpfnDlgProc = MakeProcInstance(CommentDlg, hInst);
            DialogBox(hInst, "COMMENTDLG", hDlg, lpfnDlgProc);
            FreeProcInstance(lpfnDlgProc);
            break;
        case M_SCAN:
/*
            sprintf(string,"min %ld  max %ld", m_scan_min,m_scan_max);
            MessageBox(hDlg, string, "Test",MB_ICONEXCLAMATION);
*/
            
            if (!m_scanning)
            {
                m_scanning=1;
                m_interval = (int) ((float) m_out_range*10000000.0/4096.0/m_speed/M_V_PER_AMU);
                m_tot_integral=0;
                dio_in_ch(m_input_ch);
                scan_time = time(NULL);
                strcpy(string, ctime(&scan_time));
                string[strlen(string)-1] = '\0';
                strcpy(m_data->comment.ptr, string);
                m_data->comment.size = strlen(string);
                m_data->scale=m_scale;
                m_data->start=m_scan_min;
                m_data->end = m_scan_min; /* not a mistake! */
                m_data_valid=1;
                m_data->v_range = m_out_range;
                m_data->speed=m_speed;
                m_data->calib_int = m_intercept;
                m_data->calib_slope = m_slope;
                m_screen_min=m_scan_min;
                m_screen_max=m_scan_max;
                m_vert_max=M_VERT_MAX;
                repaint_screen_scroll(hDlg);  
                repaint_vert_scale(hDlg);
                masspec_hdc=GetDC(hDlg);
                hpen_old=SelectObject(masspec_hdc,hpen_white);
                repaint_grid(hDlg,M_X_MIN,M_X_MAX,M_Y_MIN,M_Y_MAX);
                SelectObject(masspec_hdc,hpen_green);
                m_scan_disable(hDlg);
                for(i=m_scan_min;i<=m_scan_max && m_scanning;i++)
                {
                    dio_out(m_output_ch,i);
                    dio_start_clock(m_interval);
                    if (i==m_scan_min+1)
                    {
                        xs=M_X_MIN;
                        yfs=(((float)m_vert_max-(float)m_data->ptr[m_scan_min]+(float)IN_MAX/2)*
                            (float)(M_Y_MAX-M_Y_MIN+1)/
                            (float)(m_vert_max+1))+M_Y_MIN;
                        xs=max(min(xs,M_X_MAX),M_X_MIN);
                        yfs=max(min(yfs,M_Y_MAX),M_Y_MIN);
                        ys=(int) yfs;
//                        MoveTo(masspec_hdc,xs,ys);
                        MoveToEx(masspec_hdc,xs,ys,NULL);
                    }
                    else if (i!=m_scan_min)
                    {
                        xs=(i-m_screen_min)*(M_X_MAX-M_X_MIN+1)/
                            (m_screen_max-m_screen_min+1)+M_X_MIN;
                        yfs=(((float)m_vert_max-(float)m_data->ptr[i-1]+(float)IN_MAX/2)*
                            (float)(M_Y_MAX-M_Y_MIN+1)/
                            (float)(m_vert_max+1))+M_Y_MIN;
                        xs=max(min(xs,M_X_MAX),M_X_MIN);
                        yfs=max(min(yfs,M_Y_MAX),M_Y_MIN);
                        ys=(int) yfs;
                        LineTo(masspec_hdc,xs,ys);
                    }
                    while(PeekMessage(&msg,hDlg,0,0,PM_REMOVE))
                    {
                                TranslateMessage(&msg);
                                DispatchMessage(&msg);
                    }       
                            
                    dio_wait_clock();
                    dio_in_data(&(unsigned_data));
                    m_data->ptr[i]=(float) unsigned_data;
                    m_data->end = i;
                    m_tot_integral+=m_data->ptr[i];
                }
                dio_out(m_output_ch,ZERO);
                SelectObject(masspec_hdc,hpen_old);
                ReleaseDC(hDlg,masspec_hdc);
                m_scan_enable(hDlg);
            }
            m_scanning=0;
            
            break;

        case ENTER:
/*
            for (i=0;i<4;i++) repaint_chs(hDlg,i);
            repaint_time(hDlg);
            repaint_trigger(hDlg);
*/            
            repaint_scan_min(hDlg);
            repaint_scan_max(hDlg);
            repaint_p1(hDlg);
            repaint_speed(hDlg);
            break;
        case M_EXIT:
            DeleteObject(hpen_black);
            DeleteObject(hpen_white);
            DeleteObject(hpen_red);
            DeleteObject(hpen_green);
            DeleteObject(hpen_blue);
            DeleteObject(hpen_dash);
            DeleteObject(masspec_area);
            EndDialog(hDlg, TRUE);
            return (TRUE);
            break;
        }
        break;
    }
    return (FALSE);
}                                                                  

void repaint_channels(HWND hDlg)
{
    sprintf(string,"%ld",m_input_ch);
    SetDlgItemText(hDlg, M_INPUT_CH, string);
    sprintf(string,"%ld",m_output_ch);
    SetDlgItemText(hDlg, M_OUTPUT_CH, string);
}

#ifdef OLD
static void repaint_grid(HWND hDlg)
{
    int i;
    HPEN hpen_old;
    HDC masspec_hdc;
    
    masspec_hdc=GetDC(hDlg);
    hpen_old=SelectObject(masspec_hdc,hpen_white);
    Rectangle(masspec_hdc,M_X_MIN,M_Y_MIN,M_X_MAX+1,M_Y_MAX);

    SelectObject(masspec_hdc,hpen_dash);
        
    for(i=1;i<10;i++)
    {
//        MoveTo(masspec_hdc,M_X_MIN,i*(M_Y_MAX-M_Y_MIN+1)/10+M_Y_MIN);
        MoveToEx(masspec_hdc,M_X_MIN,i*(M_Y_MAX-M_Y_MIN+1)/10+M_Y_MIN,NULL);
        LineTo(masspec_hdc,M_X_MAX+1,i*(M_Y_MAX-M_Y_MIN+1)/10+M_Y_MIN);
//        MoveTo(masspec_hdc,i*(M_X_MAX-M_X_MIN+1)/10+M_X_MIN,M_Y_MIN);
        MoveToEx(masspec_hdc,i*(M_X_MAX-M_X_MIN+1)/10+M_X_MIN,M_Y_MIN,NULL);
        LineTo(masspec_hdc,i*(M_X_MAX-M_X_MIN+1)/10+M_X_MIN,M_Y_MAX+1);
    }
    SelectObject(masspec_hdc,hpen_old);
    ReleaseDC(hDlg,masspec_hdc);
}
#endif

void repaint_scan_min(HWND hDlg)
{
    out_smart=1;
    SetScrollPos(GetDlgItem(hDlg, M_MIN_SCROLL), SB_CTL, DTOI(m_scan_min,M_MIN_MIN,M_MIN_MAX), TRUE);
    sprintf(string,"%0.3f", m_v_to_amu(dtov(m_scan_min,m_out_range)));
    SetDlgItemText(hDlg, M_MIN_RANGE, string);
    out_smart=0;
}

void repaint_scan_max(HWND hDlg)
{
    out_smart=1;
    SetScrollPos(GetDlgItem(hDlg, M_MAX_SCROLL), SB_CTL, DTOI(m_scan_max,M_MAX_MIN,M_MAX_MAX), TRUE);
    sprintf(string,"%0.3f", m_v_to_amu(dtov(m_scan_max,m_out_range)));
    SetDlgItemText(hDlg, M_MAX_RANGE, string);
    out_smart=0;
}

void repaint_speed(HWND hDlg)
{
    out_smart=1;
    SetScrollPos(GetDlgItem(hDlg, M_AMU_SCROLL), SB_CTL, DTOI(m_speed,M_SPEED_MIN,M_SPEED_MAX), TRUE);
    sprintf(string,"%0.3f", m_speed);
    SetDlgItemText(hDlg, M_SPEED, string);
    out_smart=0;
}

void repaint_calib(HWND hDlg)
{
    out_smart=1;
    sprintf(string,"%0.3f", m_slope);
    SetDlgItemText(hDlg, M_SLOPE, string);
    sprintf(string,"%0.3f", m_intercept);
    SetDlgItemText(hDlg, M_INTERCEPT, string);
    out_smart=0;
}

void repaint_background(HWND hDlg)
{
    out_smart=1;
    sprintf(string,"%0.2lf", in_dtov((float)m_background));
    SetDlgItemText(hDlg, M_BACKGROUND, string);
    out_smart=0;
}

        
void repaint_data(HWND hDlg)
{
    int i;
    HDC masspec_hdc;
    HPEN hpen_old;
    int xs,ys;    
    float yfs;
    int start,end;
    
    if (m_data_valid)
    {
        masspec_hdc=GetDC(hDlg);
        hpen_old=SelectObject(masspec_hdc,hpen_green);
        if (m_screen_min>m_data->start) start=m_screen_min;
        else start=m_data->start;
        if (m_screen_max<m_data->end) end=m_screen_max;
        else end=m_data->end;
        
        i=start;
        xs=(i-m_screen_min)*(M_X_MAX-M_X_MIN+1)/
          (m_screen_max-m_screen_min+1)+M_X_MIN;
        yfs=(((float)m_vert_max-(float)m_data->ptr[i]+(float)IN_MAX/2)*
            (float)(M_Y_MAX-M_Y_MIN+1)/
            (float)(m_vert_max+1))+(float)M_Y_MIN;
        xs=max(min(xs,M_X_MAX),M_X_MIN);
        yfs=max(min(yfs,M_Y_MAX),M_Y_MIN);
        ys=(int) yfs;
//        MoveTo(masspec_hdc,xs,ys);
        MoveToEx(masspec_hdc,xs,ys,NULL);
        for(i=start+1;i<=end;i++)
        {
            xs=(i-m_screen_min)*(M_X_MAX-M_X_MIN+1)/
               (m_screen_max-m_screen_min+1)+M_X_MIN;
            yfs=(((float)m_vert_max-(float)m_data->ptr[i]+(float)IN_MAX/2)*
                (float)(M_Y_MAX-M_Y_MIN+1)/
                (float)(m_vert_max+1))+(float)M_Y_MIN;
            xs=max(min(xs,M_X_MAX),M_X_MIN);
            yfs=max(min(yfs,M_Y_MAX),M_Y_MIN);
            ys=(int) yfs;
            LineTo(masspec_hdc,xs,ys);
        }
        SelectObject(masspec_hdc,hpen_old);
        ReleaseDC(hDlg,masspec_hdc);
        
    }
            
    
}

void repaint_p1(HWND hDlg)
{
    HDC masspec_hdc;
    HPEN hpen_old;
    int xs;    
    
    if (!out_smart)
    {
        out_smart=1;
        SetScrollPos(GetDlgItem(hDlg, M_P1_SCROLL), SB_CTL, DTOI(m_p1_vbits,M_MIN_MIN,M_MAX_MAX), TRUE);
        sprintf(string,"%0.1f", m_v_to_amu(dtov(m_p1_vbits,m_out_range)));
        SetDlgItemText(hDlg, M_P1_MASS, string);
        if (m_p1_calib<0)
            SetDlgItemText(hDlg,M_P1_CALIB,"");
        out_smart=0;
    }
    if (m_data_valid)
    {
        if (m_p1_vbits>=m_data->start && m_p1_vbits<=m_data->end)
        {
            sprintf(string,"%0.2lf", in_dtov(m_data->ptr[m_p1_vbits]));
            SetDlgItemText(hDlg, M_P1_PRESS, string);
        }
        else SetDlgItemText(hDlg,M_P1_PRESS, "N/A");
    }
    if (m_p1_vbits>=m_screen_min && m_p1_vbits<=m_screen_max)
    {
        masspec_hdc=GetDC(hDlg);
        hpen_old=SelectObject(masspec_hdc,hpen_blue);
        xs=(m_p1_vbits-m_screen_min)*(M_X_MAX-M_X_MIN+1)/
           (m_screen_max-m_screen_min+1)+M_X_MIN;
        xs=max(min(xs,M_X_MAX),M_X_MIN);
//        MoveTo(masspec_hdc,xs,M_Y_MIN);
        MoveToEx(masspec_hdc,xs,M_Y_MIN,NULL);
        LineTo(masspec_hdc,xs,M_Y_MAX);
        
         
         
        
        SelectObject(masspec_hdc,hpen_old);
        ReleaseDC(hDlg,masspec_hdc);
    }
        
    
    
}

void repaint_graphics(HWND hDlg)
{
    clear_area(hDlg,M_X_MIN,M_Y_MIN,M_X_MAX,M_Y_MAX,RGB(255,255,255));
    repaint_grid(hDlg,M_X_MIN,M_X_MAX,M_Y_MIN,M_Y_MAX);
    repaint_p1(hDlg);
    repaint_p2(hDlg);
    repaint_data(hDlg);
}

void repaint_p2(HWND hDlg)
{
    HDC masspec_hdc;
    HPEN hpen_old;
    int xs;    
    
    
    if (!out_smart)
    {
        out_smart=1;
        SetScrollPos(GetDlgItem(hDlg, M_P2_SCROLL), SB_CTL, DTOI(m_p2_vbits,M_MIN_MIN,M_MAX_MAX), TRUE);
        sprintf(string,"%0.1f", m_v_to_amu(dtov(m_p2_vbits,m_out_range)));
        SetDlgItemText(hDlg, M_P2_MASS, string);
        if (m_p2_calib<0)
            SetDlgItemText(hDlg,M_P2_CALIB,"");
        out_smart=0;
    }
    if (m_data_valid)
    {
        if (m_p2_vbits>=m_data->start && m_p2_vbits<=m_data->end)
        {
            sprintf(string,"%0.2lf", in_dtov(m_data->ptr[m_p2_vbits]));
            SetDlgItemText(hDlg, M_P2_PRESS, string);
        }
        else SetDlgItemText(hDlg,M_P2_PRESS, "N/A");
    }
    if (m_p2_vbits>=m_screen_min && m_p2_vbits<=m_screen_max)
    {
        masspec_hdc=GetDC(hDlg);
        hpen_old=SelectObject(masspec_hdc,hpen_red);
        xs=(m_p2_vbits-m_screen_min)*(M_X_MAX-M_X_MIN+1)/
           (m_screen_max-m_screen_min+1)+M_X_MIN;
        xs=max(min(xs,M_X_MAX),M_X_MIN);
//        MoveTo(masspec_hdc,xs,M_Y_MIN);
        MoveToEx(masspec_hdc,xs,M_Y_MIN,NULL);
        LineTo(masspec_hdc,xs,M_Y_MAX);
        
         
         
        
        SelectObject(masspec_hdc,hpen_old);
        ReleaseDC(hDlg,masspec_hdc);
    }
        
    
    
}

void repaint_vert_scale(HWND hDlg)
{
    out_smart=1;
    sprintf(string,"%0.2f",((float) m_vert_max)/(float)IN_MAX*20);
    SetDlgItemText(hDlg, M_V_SCALE, string);
    SetScrollPos(GetDlgItem(hDlg, M_V_SCALE_SCROLL), SB_CTL,DTOI(m_vert_max,M_VERT_MIN,M_VERT_MAX) , TRUE);
    out_smart=0;
}

void repaint_spec_scale(HWND hDlg)
{
    out_smart=1;
    sprintf(string,"%d",m_scale);
    SetDlgItemText(hDlg, M_SPEC_SCALE, string);
    out_smart=0;
}

void repaint_screen_scroll(HWND hDlg)
{
    out_smart=1;                                                                                                           
    SetScrollPos(GetDlgItem(hDlg, M_SCREEN_SCROLL), SB_CTL,DTOI(m_screen_min,m_data->start,m_data->end-m_screen_max+m_screen_min) , TRUE);
    out_smart=0;
}

void repaint_integral(HWND hDlg)
{
    calc_part_integral();
    if (m_tot_integral)
        sprintf(string,"%0.2f", (float)m_part_integral/(float)m_tot_integral*100);
    else strcpy(string,"N/A");

    SetDlgItemText(hDlg, M_INTEGRAL, string);

}
float m_v_to_amu(float v)
{
    return((float) (v-m_intercept)/m_slope);
}

float m_amu_to_v(float amu)
{
    return((float)(amu*m_slope+m_intercept));
}

void get_calib()
{
    FILE *fp;
    
    fp=fopen(MASSPEC_INI,"rb");
    fgets(string,100,fp);
    sscanf(string,"%f",&m_intercept);
    fgets(string,100,fp);
    sscanf(string,"%f",&m_slope);
    fclose(fp);
}

void m_scan_enable(HWND hDlg)
{
    EnableWindow(GetDlgItem(hDlg, M_SPEED), TRUE);
    EnableWindow(GetDlgItem(hDlg, M_AMU_SCROLL), TRUE);
    EnableWindow(GetDlgItem(hDlg, M_INPUT_CH), TRUE);
    EnableWindow(GetDlgItem(hDlg, M_OUTPUT_CH), TRUE);
    EnableWindow(GetDlgItem(hDlg, M_COMMENT), TRUE);
    EnableWindow(GetDlgItem(hDlg, M_PRINT), TRUE);
    EnableWindow(GetDlgItem(hDlg, M_SAVE), TRUE);
    EnableWindow(GetDlgItem(hDlg, M_EXIT), TRUE);
    EnableWindow(GetDlgItem(hDlg, M_LOAD), TRUE);

}

void m_scan_disable(HWND hDlg)
{
    EnableWindow(GetDlgItem(hDlg, M_SPEED), FALSE);
    EnableWindow(GetDlgItem(hDlg, M_AMU_SCROLL), FALSE);
    EnableWindow(GetDlgItem(hDlg, M_INPUT_CH), FALSE);
    EnableWindow(GetDlgItem(hDlg, M_OUTPUT_CH), FALSE);
    EnableWindow(GetDlgItem(hDlg, M_COMMENT), FALSE);
    EnableWindow(GetDlgItem(hDlg, M_PRINT), FALSE);
    EnableWindow(GetDlgItem(hDlg, M_SAVE), FALSE);
    EnableWindow(GetDlgItem(hDlg, M_LOAD), FALSE);
    EnableWindow(GetDlgItem(hDlg, M_EXIT), FALSE);
}

void load_mas_old(char *fname)
{
    FILE *fp;
    int version;
    float tempf[4096];
    int i;
    
    char *name;
  
    strcpy((*glob_data)->full_filename,fname);  
    name=strrchr(fname,'\\');
    if (name==NULL) strcpy((*glob_data)->filename,fname);
    else strcpy((*glob_data)->filename,name+1);
    *(strrchr((*glob_data)->filename,'.'))='\0';
    fp=fopen(fname,"rb");
    fread(&version,sizeof(version),1,fp);
    
    (*glob_data)->x_type=GEN2D_NONE;
    (*glob_data)->y_type=GEN2D_FLOAT;
    if ((*glob_data)->xf!=NULL) 
    {
        free((*glob_data)->xf);
        (*glob_data)->xf=NULL;
    }
    if ((*glob_data)->yf!=NULL)
    {
        free((*glob_data)->yf);
        (*glob_data)->yf=NULL;
    }
    if ((*glob_data)->xd!=NULL) 
    {
        free((*glob_data)->xd);
        (*glob_data)->xd=NULL;
    }
    if ((*glob_data)->yd!=NULL) 
    {
        free((*glob_data)->yd);
        (*glob_data)->yd=NULL;
    }

    
    fread(tempf,sizeof(tempf[0]),(size_t) SP_NUM_CH,fp);
    fread(&((*glob_data)->start),sizeof((*glob_data)->start),1,fp);
    fread(&((*glob_data)->end),sizeof((*glob_data)->end),1,fp);
    (*glob_data)->size=(*glob_data)->end-(*glob_data)->start+1;
    (*glob_data)->yf = (float *) malloc((*glob_data)->size*sizeof(float));
    memcpy((*glob_data)->yf,tempf+(*glob_data)->start,(*glob_data)->size*sizeof(float));
    
    fread(&((*glob_data)->skip),sizeof((*glob_data)->skip),1,fp);
    fread(&((*glob_data)->v_range),sizeof((*glob_data)->v_range),1,fp);
    fread(&((*glob_data)->speed),sizeof((*glob_data)->speed),1,fp);
    fread(&((*glob_data)->calib_int),sizeof((*glob_data)->calib_int),1,fp);
    fread(&((*glob_data)->calib_slope),sizeof((*glob_data)->calib_slope),1,fp);
    fread(&((*glob_data)->scale),sizeof((*glob_data)->scale),1,fp);
    fread(&((*glob_data)->comment.size),sizeof((*glob_data)->comment.size),1,fp);
    fread((*glob_data)->comment.ptr,sizeof((*glob_data)->comment.ptr[0]),(size_t) (*glob_data)->comment.size,fp);
    (*glob_data)->comment.ptr[(*glob_data)->comment.size]='\0';
    fread(&((*glob_data)->sample_type.size),sizeof((*glob_data)->sample_type.size),1,fp);
    fread((*glob_data)->sample_type.ptr,sizeof(char),(size_t)(*glob_data)->sample_type.size,fp);
    (*glob_data)->sample_type.ptr[(*glob_data)->sample_type.size]='\0';
    fread(&(*glob_data)->dosed_langmuir,sizeof((*glob_data)->dosed_langmuir),1,fp);  
    fread(&((*glob_data)->dosed_type.size),sizeof((*glob_data)->dosed_type.size),1,fp);
    fread((*glob_data)->dosed_type.ptr,sizeof(char),(size_t)(*glob_data)->dosed_type.size,fp);
    *((*glob_data)->dosed_type.ptr+(*glob_data)->dosed_type.size) = '\0';
    if (version>2)
    {
        fread(&((*glob_data)->bias),sizeof((*glob_data)->bias),1,fp);
        fread(&((*glob_data)->dz_offset),sizeof((*glob_data)->dz_offset),1,fp);
        fread(&((*glob_data)->i_setpoint),sizeof((*glob_data)->i_setpoint),1,fp);
        fread(&((*glob_data)->samples),sizeof((*glob_data)->samples),1,fp);
        }
    else
    {
        (*glob_data)->bias=0;
        (*glob_data)->dz_offset=0;
        (*glob_data)->i_setpoint=0;
        (*glob_data)->samples=1;
            
    }
    if (version>3)
    {
        fread(&((*glob_data)->type),sizeof((*glob_data)->type),1,fp);
    }
    else
    {
        if (fname[strlen(fname)-1]=='c') /* .spc */
        {
            (*glob_data)->type=DATATYPE_SPEC_I;
        }
        else (*glob_data)->type=DATATYPE_MASS;
    }
    (*glob_data)->measure_i=0;
    (*glob_data)->measure_z=0;
    (*glob_data)->measure_2=0;
    (*glob_data)->measure_3=0;
    switch ((*glob_data)->type)
    {
        case DATATYPE_SPEC_I:
            (*glob_data)->type=DATATYPE_GEN2D;
            (*glob_data)->scan_feedback=0;
            (*glob_data)->type2d=TYPE2D_SPEC_I;
            (*glob_data)->measure_i=1;
            break;
        case DATATYPE_SPEC_Z:
            (*glob_data)->type=DATATYPE_GEN2D;
            (*glob_data)->scan_feedback=1;
            (*glob_data)->type2d=TYPE2D_SPEC_I;
            (*glob_data)->measure_z=1;
            break;
        case DATATYPE_MASS:
            (*glob_data)->type=DATATYPE_GEN2D;
            (*glob_data)->scan_feedback=1;
            (*glob_data)->type2d=TYPE2D_MASSPEC;
            (*glob_data)->measure_2=1;
            break;
    }
    if (version>4)
    {
        fread(&((*glob_data)->delta_z_offset),sizeof((*glob_data)->delta_z_offset),1,fp);
        fread(&((*glob_data)->temperature),sizeof((*glob_data)->temperature),1,fp);
        fread(&((*glob_data)->version),sizeof((*glob_data)->version),1,fp);
    }
    else 
    {
        (*glob_data)->delta_z_offset=0;
        (*glob_data)->temperature=0;
        (*glob_data)->version=version;
    }
    (*glob_data)->bias_range=(*glob_data)->v_range;
    (*glob_data)->i_set_range=2; 
    (*glob_data)->amp_gain=(*glob_data)->scale; 
    (*glob_data)->track_offset_x=0;
    (*glob_data)->track_offset_y=0;
    (*glob_data)->track_max_bits=0;
    (*glob_data)->track_sample_every=0;
    (*glob_data)->tracking_mode=SP_TRACK_MAX;
    (*glob_data)->track_auto_auto=0;
    (*glob_data)->track_iterations=0;
    (*glob_data)->track_every=0;
    (*glob_data)->z_offset_bias=(*glob_data)->bias;
    (*glob_data)->num_passes=1; 
    (*glob_data)->scan_dir=1<<2;
    (*glob_data)->scan_num=(*glob_data)->samples;
    (*glob_data)->crash_protection=SP_STOP_CRASHING;
    (*glob_data)->dither0=SP_DITHER0_OFF;
    (*glob_data)->dither1=SP_DITHER1_OFF;
    (*glob_data)->step=1;
    (*glob_data)->move_time=(*glob_data)->speed;
    (*glob_data)->dither_wait=0;
    (*glob_data)->feedback_wait=0;
    (*glob_data)->feedback_every=0;
    (*glob_data)->track_avg_data_pts=200;
    (*glob_data)->saved=1;
    
    (*glob_data)->min_x=(*glob_data)->start;
    (*glob_data)->max_x=(*glob_data)->start+
                    ((*glob_data)->size-1)*(*glob_data)->step;
    (*glob_data)->sp_mode=SP_MODE_ABSOLUTE;
    for (i=0;i<16;i++) (*glob_data)->pre_dac_data[i]=0;
    
    (*glob_data)->x=0;
    (*glob_data)->y=0; 
    (*glob_data)->dep_filename[0]='\0';
    (*glob_data)->track_step_delay=1000;
    (*glob_data)->output_ch=sample_bias_ch;
    (*glob_data)->lockin_sen1=0;
    (*glob_data)->lockin_sen2=0;
    (*glob_data)->lockin_osc=0;
    (*glob_data)->lockin_w=0;
    (*glob_data)->lockin_tc=0;
    (*glob_data)->lockin_n1=0;
    (*glob_data)->lockin_pha1=0;
    (*glob_data)->lockin_pha2=0;
        
    
    (*glob_data)->valid=1;
    
    fclose(fp);
    
}

void save_mas_old(char *fname)
{
    FILE *fp;
    int version=SP_DATA_VERSION;
    
    char *name;
  
    strcpy((*glob_data)->full_filename,fname);  
    name=strrchr(fname,'\\');
    if (name==NULL) strcpy((*glob_data)->filename,fname);
    else strcpy((*glob_data)->filename,name+1);
    *(strrchr((*glob_data)->filename,'.'))='\0';
    fp=fopen(fname,"wb");
    fwrite(&version,sizeof(version),1,fp);
    fwrite((*glob_data)->ptr,sizeof((*glob_data)->ptr[0]),(size_t) SP_NUM_CH,fp);
    fwrite(&((*glob_data)->start),sizeof((*glob_data)->start),1,fp);
    fwrite(&((*glob_data)->end),sizeof((*glob_data)->end),1,fp);
    fwrite(&((*glob_data)->skip),sizeof((*glob_data)->skip),1,fp);
    fwrite(&((*glob_data)->v_range),sizeof((*glob_data)->v_range),1,fp);
    fwrite(&((*glob_data)->speed),sizeof((*glob_data)->speed),1,fp);
    fwrite(&((*glob_data)->calib_int),sizeof((*glob_data)->calib_int),1,fp);
    fwrite(&((*glob_data)->calib_slope),sizeof((*glob_data)->calib_slope),1,fp);
    fwrite(&((*glob_data)->scale),sizeof((*glob_data)->scale),1,fp);
    fwrite(&((*glob_data)->comment.size),sizeof((*glob_data)->comment.size),1,fp);
    fwrite((*glob_data)->comment.ptr,sizeof((*glob_data)->comment.ptr[0]),(size_t) (*glob_data)->comment.size,fp);
    fwrite(&(data->sample_type.size),sizeof(data->sample_type.size),1,fp);
    fwrite(data->sample_type.ptr,sizeof(char),(size_t)data->sample_type.size,fp);
    fwrite(&data->dosed_langmuir,sizeof(data->dosed_langmuir),1,fp);  
    fwrite(&(data->dosed_type.size),sizeof(data->dosed_type.size),1,fp);
    fwrite(data->dosed_type.ptr,sizeof(char),(size_t)data->dosed_type.size,fp);
    fwrite(&((*glob_data)->bias),sizeof((*glob_data)->bias),1,fp);
    fwrite(&((*glob_data)->dz_offset),sizeof((*glob_data)->dz_offset),1,fp);
    fwrite(&((*glob_data)->i_setpoint),sizeof((*glob_data)->i_setpoint),1,fp);
    fwrite(&((*glob_data)->samples),sizeof((*glob_data)->samples),1,fp);
    fwrite(&((*glob_data)->type),sizeof((*glob_data)->type),1,fp);
/* version 5 */
    fwrite(&((*glob_data)->delta_z_offset),sizeof((*glob_data)->delta_z_offset),1,fp);
    fwrite(&((*glob_data)->temperature),sizeof((*glob_data)->temperature),1,fp);
    fwrite(&((*glob_data)->version),sizeof((*glob_data)->version),1,fp);
    
    fclose(fp);
     
}

void calc_integral()
{
    int i;

  m_tot_integral=0;

  if (m_data_valid) 
  {
    for(i=m_data->start;i<=m_data->end;i++)
    {
      m_tot_integral+=m_data->ptr[i]-m_background;
    }
//    m_tot_integral-=m_background*(m_data->end-m_data->start+1);
  }
  else m_tot_integral=1;
}

void calc_part_integral()
{
  int i,start,end;
  m_part_integral=0;

  if (m_data_valid)
  {
    if (m_p1_vbits<m_p2_vbits)
    {
      if (m_p1_vbits>m_data->start) start=m_p1_vbits;
      else start=m_data->start;
      if (m_p2_vbits<m_data->end) end=m_p2_vbits;
      else end=m_data->end;
    }
    else
    {
      if (m_p2_vbits>m_data->start) start=m_p2_vbits;
      else start=m_data->start;
      if (m_p1_vbits<m_data->end) end=m_p1_vbits;
      else end=m_data->end;
    }
    for(i=start;i<=end;i++)
    {
      m_part_integral+=m_data->ptr[i]-m_background;
    }
//    m_part_integral-=m_background*(end-start+1);
  }
}                                                 
    
void m_calibrate()
{
    FILE *fp;
    m_data->calib_slope=m_slope=(dtov(m_p1_vbits,m_out_range)-dtov(m_p2_vbits,m_out_range))/
            (m_p1_calib-m_p2_calib);
    m_data->calib_int=m_intercept=dtov(m_p1_vbits,m_out_range)-m_p1_calib*m_slope;
    
    fp=fopen(MASSPEC_INI,"wb");
    fprintf(fp,"%f  ; the intercept\n",m_intercept);
    fprintf(fp,"%f  ; the slope\n",m_slope);
    fclose(fp);
    
}

void find_background()
{
    int i,start,end;
    
    if (m_p1_vbits<m_p2_vbits)
    {
        start=m_p1_vbits;
        end=m_p2_vbits;
    }
    else
    {
        start=m_p2_vbits;
        end=m_p1_vbits;
    }
    m_background=m_data->ptr[start];

    for(i=start+1;i<=end;i++)
    {
        m_background+=m_data->ptr[i];
    }
    m_background=(int)floor((float)m_background/(float)(end-start+1)+0.5);

}
        
