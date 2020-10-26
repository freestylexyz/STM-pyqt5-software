#include <math.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "file.h"
#include "data.h"
#include "stm.h"
#include "dio.h"
#include "oscill.h"
#include "clock.h"

#define ONE_CLOCK
#define NEW

extern char     string[];
double oscill_time_offset=0;
unsigned int cntr2value=65535;
int oscill_reading=0;
oscill_data *oscill_ch_data[4];
int data_pos[4]={0,0,0,0};
int lowerv[4]={0,0,0,0};
int higherv[4]={65535,65535,65535,65535};
unsigned int oscill_read_ch=0,last_read=0;
int ch_selected[4]={0,0,0,0};
int ch_offset[4]={0,0,0,0};
int total_selected;
int old_time=0;
int trigger_ch=0;
int trigger=0;
extern unsigned int input_ch,out1;
extern unsigned int dac_data[];
long int time_per_div=100000; /* in micro seconds */
double time_origin=0;
static out_smart=0;
int trigger_val=32768;
static HPEN hpen_black,hpen_red,hpen_green,hpen_blue,hpen_old,hpen_dash,hpen_white;
static HRGN oscill_area;
static PAINTSTRUCT ps;
int oscill_connect[4]={0,0,0,0};
HDC oscill_hdc;

BOOL FAR PASCAL OscillDlg(HWND, unsigned, WPARAM, LPARAM);

void repaint_chs(HWND, long int);
void repaint_time(HWND);
void repaint_trigger(HWND);

BOOL FAR PASCAL OscillDlg(HWND hDlg, unsigned Message, WPARAM wParam, LPARAM lParam)
{
    int id;
    long int i,j,mini,maxi,delt,ddelt;
    double d,minid,maxid,deltd,ddeltd;
    double oscill_time;    
    double start_time;
    unsigned int lobyte,hibyte;
    MSG msg;
        
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
			SetScrollRange(GetDlgItem(hDlg, O_LOWER_SCROLL_0), SB_CTL, 0,32767, TRUE);
            SetScrollRange(GetDlgItem(hDlg, O_HIGHER_SCROLL_0), SB_CTL, 0,32767, TRUE);
            SetScrollRange(GetDlgItem(hDlg, O_OFFSET_0_SCROLL), SB_CTL, 0,32767, TRUE);
            SetScrollRange(GetDlgItem(hDlg, O_LOWER_SCROLL_1), SB_CTL, 0,32767, TRUE);
            SetScrollRange(GetDlgItem(hDlg, O_HIGHER_SCROLL_1), SB_CTL, 0,32767, TRUE);
            SetScrollRange(GetDlgItem(hDlg, O_OFFSET_1_SCROLL), SB_CTL, 0,32767, TRUE);
            SetScrollRange(GetDlgItem(hDlg, O_LOWER_SCROLL_2), SB_CTL, 0,32767, TRUE);
            SetScrollRange(GetDlgItem(hDlg, O_HIGHER_SCROLL_2), SB_CTL, 0,32767, TRUE);
            SetScrollRange(GetDlgItem(hDlg, O_OFFSET_2_SCROLL), SB_CTL, 0,32767, TRUE);
            SetScrollRange(GetDlgItem(hDlg, O_LOWER_SCROLL_3), SB_CTL, 0,32767, TRUE);
            SetScrollRange(GetDlgItem(hDlg, O_HIGHER_SCROLL_3), SB_CTL, 0,32767, TRUE);
            SetScrollRange(GetDlgItem(hDlg, O_OFFSET_3_SCROLL), SB_CTL, 0,32767, TRUE);
            SetScrollRange(GetDlgItem(hDlg, TIME_PER_DIVISION_SCROLL), SB_CTL, 0,32767, TRUE);
            SetScrollRange(GetDlgItem(hDlg, O_TRIGGER_SCROLL), SB_CTL, 0,32767, TRUE);
            EnableWindow(GetDlgItem(hDlg, O_START), 1);
            EnableWindow(GetDlgItem(hDlg, O_STOP), 0);
            switch(trigger_ch)
            {
                case 0:
                    CheckDlgButton(hDlg,O_TRIG_CH_0,1);
                    break;
                case 1:
                    CheckDlgButton(hDlg,O_TRIG_CH_1,1);
                    break;
                case 2:
                    CheckDlgButton(hDlg,O_TRIG_CH_2,1);
                    break;
                case 3:
                    CheckDlgButton(hDlg,O_TRIG_CH_3,1);
                    break;
            }
            if (trigger) CheckDlgButton(hDlg,O_TRIGGER,1);
            repaint_time(hDlg);
            repaint_trigger(hDlg);
            for(i=0;i<4;i++) data_pos[i]=oscill_connect[i]=0;
            for (i=0;i<4;i++) repaint_chs(hDlg,i);
            
            for(i=0;i<4;i++) ch_selected[i]=0;
            total_selected=0;
            for(i=0;i<4;i++) oscill_ch_data[i]=(oscill_data *)
                        malloc(sizeof(oscill_data)*O_DATA_MAX);
            hpen_black=CreatePen(PS_SOLID,1,RGB(0,0,0));
            hpen_red=CreatePen(PS_SOLID,1,RGB(255,0,0));
            hpen_green=CreatePen(PS_SOLID,1,RGB(0,255,0));
            hpen_blue=CreatePen(PS_SOLID,1,RGB(0,0,255));
            hpen_black=CreatePen(PS_SOLID,1,RGB(0,0,0));
            hpen_white=CreatePen(PS_SOLID,1,RGB(255,255,255));
            hpen_dash=CreatePen(PS_DASH,1,RGB(0,0,0));
//            hpen_old=SelectObject(oscill_hdc,hpen_black);
			oscill_area=CreateRectRgn(O_X_MIN,O_Y_MIN,O_X_MAX,O_Y_MAX);

        }    
    case WM_PAINT:
        BeginPaint(hDlg,&ps);
        if (RectInRegion(oscill_area,&(ps.rcPaint)))  
        {
            repaint_grid(hDlg,O_X_MIN,O_X_MAX,O_Y_MIN,O_Y_MAX);
        }
        EndPaint(hDlg,&ps);
        break;
    case WM_VSCROLL:
    case WM_HSCROLL:
        id = getscrollid();
        switch (id)
        {
            case O_LOWER_SCROLL_0:
                d=lowerv[0];
                minid=O_LOWER_MIN;
                maxid=higherv[0]-1;
                deltd=O_VOLT_DELT;
                ddeltd=O_VOLT_DDELT;
                break;
            case O_HIGHER_SCROLL_0:
                d=higherv[0];
                minid=lowerv[0]+1;
                maxid=O_HIGHER_MAX;
                deltd=O_VOLT_DELT;
                ddeltd=O_VOLT_DDELT;
                break;
            case O_OFFSET_0_SCROLL:
                d=ch_offset[0];
                minid=O_OFFSET_MIN;
                maxid=O_OFFSET_MAX;
                deltd=O_VOLT_DELT;
                ddeltd=O_VOLT_DDELT;
                break;
            case O_LOWER_SCROLL_1:
                d=lowerv[1];
                minid=O_LOWER_MIN;
                maxid=higherv[1]-1;
                deltd=O_VOLT_DELT;
                ddeltd=O_VOLT_DDELT;
                break;
            case O_HIGHER_SCROLL_1:
                d=higherv[1];
                minid=lowerv[1]+1;
                maxid=O_HIGHER_MAX;
                deltd=O_VOLT_DELT;
                ddeltd=O_VOLT_DDELT;
                break;
            case O_OFFSET_1_SCROLL:
                d=ch_offset[1];
                minid=O_OFFSET_MIN;
                maxid=O_OFFSET_MAX;
                deltd=O_VOLT_DELT;
                ddeltd=O_VOLT_DDELT;
                break;
            case O_LOWER_SCROLL_2:
                d=lowerv[2];
                minid=O_LOWER_MIN;
                maxid=higherv[2]-1;
                deltd=O_VOLT_DELT;
                ddeltd=O_VOLT_DDELT;
                break;
            case O_HIGHER_SCROLL_2:
                d=higherv[2];
                minid=lowerv[2]+1;
                maxid=O_HIGHER_MAX;
                deltd=O_VOLT_DELT;
                ddeltd=O_VOLT_DDELT;
                break;
            case O_OFFSET_2_SCROLL:
                d=ch_offset[2];
                minid=O_OFFSET_MIN;
                maxid=O_OFFSET_MAX;
                deltd=O_VOLT_DELT;
                ddeltd=O_VOLT_DDELT;
                break;
            case O_LOWER_SCROLL_3:
                d=lowerv[3];
                minid=O_LOWER_MIN;
                maxid=higherv[3]-1;
                deltd=O_VOLT_DELT;
                ddeltd=O_VOLT_DDELT;
                break;
            case O_HIGHER_SCROLL_3:
                d=higherv[3];
                minid=lowerv[3]+1;
                maxid=O_HIGHER_MAX;
                deltd=O_VOLT_DELT;
                ddeltd=O_VOLT_DDELT;
                break;
            case O_OFFSET_3_SCROLL:
                d=ch_offset[3];
                minid=O_OFFSET_MIN;
                maxid=O_OFFSET_MAX;
                deltd=O_VOLT_DELT;
                ddeltd=O_VOLT_DDELT;
                break;
            case TIME_PER_DIVISION_SCROLL:
                d=(double) time_per_div;
                minid=O_TIME_MIN;
                maxid=O_TIME_MAX;
                deltd=O_TIME_DELT;
                ddeltd=O_TIME_DDELT;
                break;
            case O_TRIGGER_SCROLL:
                d=(double) trigger_val;
                minid=O_TRIGGER_MIN;
                maxid=O_TRIGGER_MAX;
                deltd=O_VOLT_DELT;
                ddeltd=O_VOLT_DDELT;
                break;

/*
            case IO_POS_SCROLL:
                i=unsent_pos;
                mini=IO_POS_MIN;
                maxi=IO_POS_MAX;
                delt=IO_POS_DELT;
                ddelt=IO_POS_DDELT;
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
        d=floor(d+0.5);
        d = min(max(d, minid), maxid);
        switch (id)
        {
            case O_LOWER_SCROLL_0:
                lowerv[0]=(int) d;
                repaint_chs(hDlg,0L);
                break;
            case O_HIGHER_SCROLL_0:
                higherv[0]=(int) d;
                repaint_chs(hDlg,0L);
                break;
            case O_OFFSET_0_SCROLL:
                ch_offset[0]=(int) d;
                repaint_chs(hDlg,0L);
                break;
            case O_LOWER_SCROLL_1:
                lowerv[1]=(int) d;
                repaint_chs(hDlg,1L);
                break;
            case O_HIGHER_SCROLL_1:
                higherv[1]=(int) d;
                repaint_chs(hDlg,1L);
                break;
            case O_OFFSET_1_SCROLL:
                ch_offset[1]=(int) d;
                repaint_chs(hDlg,1L);
                break;
            case O_LOWER_SCROLL_2:
                lowerv[2]=(int) d;
                repaint_chs(hDlg,2L);
                break;
            case O_HIGHER_SCROLL_2:
                higherv[2]=(int) d;
                repaint_chs(hDlg,2L);
                break;
            case O_OFFSET_2_SCROLL:
                ch_offset[2]=(int) d;
                repaint_chs(hDlg,2L);
                break;
            case O_LOWER_SCROLL_3:
                lowerv[3]=(int) d;
                repaint_chs(hDlg,3L);
                break;
            case O_HIGHER_SCROLL_3:
                higherv[3]=(int) d;
                repaint_chs(hDlg,3L);
                break;
            case O_OFFSET_3_SCROLL:
                ch_offset[3]=(int) d;
                repaint_chs(hDlg,3L);
                break;
            case O_TRIGGER_SCROLL:
                trigger_val=(int) d;
                repaint_trigger(hDlg);
                break;
            case TIME_PER_DIVISION_SCROLL:
                if (!out_smart)
                {
                    time_per_div=(int) floor(d+0.5);
                    repaint_time(hDlg);
                }
                break;
/*
            case IO_POS_SCROLL:
                unsent_pos=i;
                repaint_pos(hDlg);
                break;
*/
        }
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case O_START:
            if (total_selected)
            {
                EnableWindow(GetDlgItem(hDlg, O_START), 0);
                EnableWindow(GetDlgItem(hDlg, O_STOP), 1);
                
                oscill_reading=1;
                for(i=0;i<4;i++) 
                {
                    data_pos[i]=oscill_connect[i]=0;
                    (oscill_ch_data[i]+O_DATA_MAX-1)->time=0;
                }
                old_time=0;
                cntr2value=65535;
                time_origin=0;
                oscill_read_ch=0;
                last_read=0;
                oscill_time_offset=0;
                while (!ch_selected[oscill_read_ch])
                {
                    oscill_read_ch++;
                    if (oscill_read_ch>3) oscill_read_ch=0;
                } 
                outpw(cfg1,oscill_read_ch&1);         /* Set out1 = bit 0 of input channel number */
                outpw(cfg2,oscill_read_ch>>1);            /* Set out2 = bit 1 of input channel number */
                input_ch = 1<<(oscill_read_ch*2+1);       /* Set A/D start convert bit */
                out1 = oscill_read_ch&1;              /* Update current out1 */
                oscill_hdc=GetDC(hDlg);
                hpen_old=SelectObject(oscill_hdc,hpen_white);
                Rectangle(oscill_hdc,O_X_MIN,O_Y_MIN,O_X_MAX,O_Y_MAX);
                repaint_grid(hDlg,O_X_MIN,O_X_MAX,O_Y_MIN,O_Y_MAX);
                oscill_time=0;
#ifdef TWO_CLOCKS
                outp(cntrcmd,0x34);  /*cntr1 16 bit operation, rate generation */
                outp(cntrcmd,0x74);  /*cntr2 16 bit operation, rate generation */
                outp(cntrcmd,0xB6);  /*cntr3 16 bit operation, square wave gen. */
                outp(cntr1,0xFF);
                outp(cntr1,0xFF);
                outp(cntr2,0xFF);
                outp(cntr2,0xFF);
                outp(cntr3,0x33);
                outp(cntr3,0x33);
                outpw(cfg3,0x260C); /* A,B write mode, C,D read mode*/
                                    /* start cntr1,cntr2, source for 1 is 10Mhz*/
                                    /* source for 2 is cntr3 */
#endif
#ifdef ONE_CLOCK
                outpw(cfg3,0x2400);
                outp(cntrcmd,0xB4); /* cntr3 16 bit rate generation */
                outp(cntr3,0xFF);
                outp(cntr3,0xFF);
#endif                
                while (oscill_reading)
                {
#ifdef NEW
                    for(i=0;i<4;i++) data_pos[i]=0;
                    oscill_time=0;
                    oscill_time_offset=0;
                    cntr2value=65535;
                    outp(cntr3,0xFF);
                    outp(cntr3,0xFF);
                    if (trigger)
                    {
                        int triggered;
                        int last_data,this_data;
                        outpw(cfg1,trigger_ch&1);         /* Set out1 = bit 0 of input channel number */
                        outpw(cfg2,trigger_ch>>1);            /* Set out2 = bit 1 of input channel number */
                        input_ch = 1<<(trigger_ch*2+1);       /* Set A/D start convert bit */
                        out1 = trigger_ch&1;              /* Update current out1 */
                        triggered=0;
                        dio_out(AD_ch,input_ch|dac_data[AD_ch]);      /* Start A/D conversion (bit => 1) */
                        dio_out(AD_ch,AD_strobe|dac_data[AD_ch]);     /* Pulse (bit => 0) */
                        while(PeekMessage(&msg,hDlg,0,0,PM_REMOVE))
                        {
                            TranslateMessage(&msg);
                            DispatchMessage(&msg);
                        }
                        outp(cntrcmd,0x74); /* delay!! */
                        outp(cntrcmd,0x74); /* delay!! */
                        outp(cntrcmd,0x74); /* delay!! */
                        outp(cntrcmd,0x84); /*latch cntr3 */
                        lobyte=inp(cntr3);
                        hibyte=inp(cntr3)*256+lobyte;
                        if (cntr2value<hibyte)
                        {
                            oscill_time_offset+=65535.0/2;
                        }
                        cntr2value=hibyte;
                        last_data=inpw(portc);
                        while(!triggered && oscill_reading)
                        {
                            dio_out(AD_ch,input_ch|dac_data[AD_ch]);      /* Start A/D conversion (bit => 1) */
                            dio_out(AD_ch,AD_strobe|dac_data[AD_ch]);     /* Pulse (bit => 0) */
                            while(PeekMessage(&msg,hDlg,0,0,PM_REMOVE))
                            {
                                TranslateMessage(&msg);
                                DispatchMessage(&msg);
                            }
                            outp(cntrcmd,0x84); /*latch cntr3 */
                            lobyte=inp(cntr3);
                            hibyte=inp(cntr3)*256+lobyte;
                            if (cntr2value<hibyte)
                            {
                                oscill_time_offset+=65535.0/2;
                            }
                            cntr2value=hibyte;
                            this_data=inpw(portc);
                            triggered=(trigger_val>=last_data && 
                                trigger_val<=this_data) || (trigger_val<=last_data
                                && trigger_val>=this_data);
                            last_data=this_data;    
                            oscill_read_ch=trigger_ch;
                        }
                        if (total_selected>1 || !ch_selected[oscill_read_ch])
                        {
                            do
                            {
                                oscill_read_ch++;
                                if (oscill_read_ch>3) oscill_read_ch=0;
                            } while (!ch_selected[oscill_read_ch]);
                            outpw(cfg1,oscill_read_ch&1);         /* Set out1 = bit 0 of input channel number */
                            outpw(cfg2,oscill_read_ch>>1);            /* Set out2 = bit 1 of input channel number */
                            input_ch = 1<<(oscill_read_ch*2+1);       /* Set A/D start convert bit */
                            out1 = oscill_read_ch&1;              /* Update current out1 */
                        }
                    }
                    outp(cntrcmd,0x84); /*latch cntr3 */
                    lobyte=inp(cntr3);
                    hibyte=inp(cntr3)*256+lobyte;
                    if (cntr2value<hibyte)
                    {
                        oscill_time_offset+=65535.0/2;
                    }
                    cntr2value=hibyte;
                    start_time=((double)(65535-hibyte))*0.5+
                        oscill_time_offset;
                    while(oscill_time-start_time<=time_per_div*10)
                    {
                        if (total_selected>1 || !ch_selected[oscill_read_ch])
                        {
                            do
                            {
                                oscill_read_ch++;
                                if (oscill_read_ch>3) oscill_read_ch=0;
                            } while (!ch_selected[oscill_read_ch]);
                            outpw(cfg1,oscill_read_ch&1);         /* Set out1 = bit 0 of input channel number */
                            outpw(cfg2,oscill_read_ch>>1);            /* Set out2 = bit 1 of input channel number */
                            input_ch = 1<<(oscill_read_ch*2+1);       /* Set A/D start convert bit */
                            out1 = oscill_read_ch&1;              /* Update current out1 */
//                            if (!ch_selected[last_read]) last_read=oscill_read_ch;
                        }           
                        dio_out(AD_ch,input_ch|dac_data[AD_ch]);      /* Start A/D conversion (bit => 1) */
                        dio_out(AD_ch,AD_strobe|dac_data[AD_ch]);     /* Pulse (bit => 0) */
                        outp(cntrcmd,0x74); /* delay!! */
                        outp(cntrcmd,0x74); /* delay!! */
                        outp(cntrcmd,0x74); /* delay!! */
//                        outp(cntrcmd,0x74); /* delay!! */
                        outp(cntrcmd,0x84); /*latch cntr3 */
                        lobyte=inp(cntr3);
                        hibyte=inp(cntr3)*256+lobyte;
                        if (cntr2value<hibyte)
                        {
                            oscill_time_offset+=65535.0/2;
                        }
                        cntr2value=hibyte;
                        oscill_time=((double)(65535-hibyte))*0.5+
                            oscill_time_offset;
                        (oscill_ch_data[oscill_read_ch]+data_pos[oscill_read_ch])->time=
                            (unsigned int) floor(oscill_time);
                        (oscill_ch_data[oscill_read_ch]+data_pos[oscill_read_ch])->data=
                            inpw(portc);
                        data_pos[oscill_read_ch]++;
//                        if (data_pos[oscill_read_ch]>=O_DATA_MAX) data_pos[oscill_read_ch]=0;
                    } /* got the data */
                    sprintf(string,"%d",(-oscill_ch_data[oscill_read_ch]->time+(oscill_ch_data[oscill_read_ch]+data_pos[oscill_read_ch]-1)->time)/data_pos[oscill_read_ch]);
                    SetDlgItemText(hDlg,SAMPLE_RATE,string);
                    SelectObject(oscill_hdc,hpen_white);
                    Rectangle(oscill_hdc,O_X_MIN,O_Y_MIN,O_X_MAX,O_Y_MAX);
                    repaint_grid(hDlg,O_X_MIN,O_X_MAX,O_Y_MIN,O_Y_MAX);

                    for(i=0;i<4;i++) if (ch_selected[i]) 
                    {
                        switch(i)
                        {
                            case 0:
                                SelectObject(oscill_hdc,hpen_black);
                                break;
                            case 1:
                                SelectObject(oscill_hdc,hpen_red);
                                break;
                            case 2:
                                SelectObject(oscill_hdc,hpen_green);
                                break;
                            case 3:
                                SelectObject(oscill_hdc,hpen_blue);
                                break;
                        }       
                        for(j=1;j<data_pos[i];j++)
                        {
                            int xs,ys;
                            xs=((oscill_ch_data[i]+j-1)->time-start_time
                                    )*(O_X_MAX-O_X_MIN+1)/10/time_per_div+
                                    O_X_MIN;
                            ys=-((oscill_ch_data[i]+j-1)->data+ch_offset[i]
                                -higherv[i])*(O_Y_MAX-O_Y_MIN+1)/(higherv[i]-
                                lowerv[i]+1)+O_Y_MIN;
                            xs=max(min(xs,O_X_MAX),O_X_MIN);
                            ys=max(min(ys,O_Y_MAX),O_Y_MIN);
//                            MoveTo(oscill_hdc,xs,ys);
                            MoveToEx(oscill_hdc,xs,ys,NULL);
                            xs=((oscill_ch_data[i]+j)->time-start_time
                                    )*(O_X_MAX-O_X_MIN+1)/10/time_per_div+
                                    O_X_MIN;
                            ys=-((oscill_ch_data[i]+j)->data+ch_offset[i]
                                -higherv[i])*(O_Y_MAX-O_Y_MIN+1)/(higherv[i]-
                                lowerv[i]+1)+O_Y_MIN;
                            xs=max(min(xs,O_X_MAX),O_X_MIN);
                            ys=max(min(ys,O_Y_MAX),O_Y_MIN);
                            LineTo(oscill_hdc,xs,ys);
                        }
                        while(PeekMessage(&msg,hDlg,0,0,PM_REMOVE))
                        {
                            TranslateMessage(&msg);
                            DispatchMessage(&msg);
                        }
                    }
#endif /*NEW*/                             
                    
                }
                SelectObject(oscill_hdc,hpen_old);
                ReleaseDC(hDlg,oscill_hdc);
            }
            break;
        case O_STOP:
            outpw(cfg3,0x2400);
            oscill_reading=0;
            EnableWindow(GetDlgItem(hDlg, O_START), 1);
            EnableWindow(GetDlgItem(hDlg, O_STOP), 0);
            break;
        case O_TRIG_CH_0:
            trigger_ch=0;
            break;
        case O_TRIG_CH_1:
            trigger_ch=1;
            break;
        case O_TRIG_CH_2:
            trigger_ch=2;
            break;
        case O_TRIG_CH_3:
            trigger_ch=3;
            break;
        case O_TRIGGER:
            trigger=IsDlgButtonChecked(hDlg,O_TRIGGER);
            break;
        case O_CH_0:
            if (ch_selected[0]=IsDlgButtonChecked(hDlg, O_CH_0))
                total_selected++;
            else total_selected--;
            data_pos[0]=0;
            oscill_connect[0]=0;
            if (!total_selected && oscill_reading)
                PostMessage(hDlg,WM_COMMAND,O_STOP,0);
            break; 
        case O_CH_1:
            if (ch_selected[1]=IsDlgButtonChecked(hDlg, O_CH_1))
                total_selected++;
            else total_selected--;
            data_pos[1]=0;
            oscill_connect[1]=0;
            if (!total_selected && oscill_reading)
                PostMessage(hDlg,WM_COMMAND,O_STOP,0);
            break;
        case O_CH_2:
            if (ch_selected[2]=IsDlgButtonChecked(hDlg, O_CH_2))
                total_selected++;
            else total_selected--;
            data_pos[2]=0;
            oscill_connect[2]=0;
            if (!total_selected && oscill_reading)
                PostMessage(hDlg,WM_COMMAND,O_STOP,0);            
            break;
        case O_CH_3:
            if (ch_selected[3]=IsDlgButtonChecked(hDlg, O_CH_3))
                total_selected++;
            else total_selected--;
            data_pos[3]=0;
            oscill_connect[3]=0;
            if (!total_selected && oscill_reading)
                PostMessage(hDlg,WM_COMMAND,O_STOP,0);            
            break;
#ifdef BLAH
        case O_READING:
                outp(cntrcmd,0x04); /* latch cntr1 */
                outp(cntrcmd,0x44); /* latch cntr2 */
                lobyte=(unsigned int) inp(cntr1);
                hibyte=(unsigned int) inp(cntr1);
                start_time=((double)(65535-(hibyte *256)-lobyte))*1e-1;
                lobyte=(unsigned int) inp(cntr2);
                hibyte=(unsigned int) inp(cntr2);
                start_time+=((double)(65535-(hibyte*256)-lobyte))*65535.0*1e-1
                    +oscill_time_offset;
            PeekMessage(&msg,hDlg,0,0,PM_REMOVE);
            if (msg.message!=WM_COMMAND || LOWORD(msg.wParam)!=O_READING)
                DispatchMessage(&msg);
            if (!PeekMessage(&msg,hDlg,0,0,PM_NOREMOVE)) 
                PostMessage(hDlg,WM_COMMAND,O_READING,0);
                
            break;
#endif                
        case O_LOWER_V_0:
            if (!out_smart)
            {
                GetDlgItemText(hDlg, O_LOWER_V_0, string, 10);
                d = atof(string);
                i = (unsigned int) in_vtod(d);
                lowerv[0]= min(max(i, O_LOWER_MIN), higherv[0]-1);
            }
            break;
        case O_HIGHER_V_0:
            if (!out_smart)
            {
                GetDlgItemText(hDlg, O_HIGHER_V_0, string, 10);
                d = atof(string);
                i = (unsigned int) in_vtod(d);
                higherv[0]= min(max(i, lowerv[0]+1), O_HIGHER_MAX);
            }
            break;
        case O_OFFSET_0:
            if (!out_smart)
            {
                GetDlgItemText(hDlg, O_OFFSET_0, string, 10);
                d = atof(string);
                i = (unsigned int) in_vtod(d+10.0)-65536;
                ch_offset[0]= min(max(i, O_OFFSET_MIN), O_OFFSET_MAX);
            }
            break;
        case O_LOWER_V_1:
            if (!out_smart)
            {
                GetDlgItemText(hDlg, O_LOWER_V_1, string, 10);
                d = atof(string);
                i = (unsigned int) in_vtod(d);
                lowerv[1]= min(max(i, O_LOWER_MIN), higherv[1]-1);
            }
            break;
        case O_HIGHER_V_1:
            if (!out_smart)
            {
                GetDlgItemText(hDlg, O_HIGHER_V_1, string, 10);
                d = atof(string);
                i = (unsigned int) in_vtod(d);
                higherv[1]= min(max(i, lowerv[1]+1), O_HIGHER_MAX);
            }
            break;
        case O_OFFSET_1:
            if (!out_smart)
            {
                GetDlgItemText(hDlg, O_OFFSET_1, string, 10);
                d = atof(string);
                i = (unsigned int) in_vtod(d+10.0)-65536;
                ch_offset[1]= min(max(i, O_OFFSET_MIN), O_OFFSET_MAX);
            }
            break;
        case O_LOWER_V_2:
            if (!out_smart)
            {
                GetDlgItemText(hDlg, O_LOWER_V_2, string, 10);
                d = atof(string);
                i = (unsigned int) in_vtod(d);
                lowerv[2]= min(max(i, O_LOWER_MIN), higherv[2]-1);
            }
            break;
        case O_HIGHER_V_2:
            if (!out_smart)
            {
                GetDlgItemText(hDlg, O_HIGHER_V_2, string, 10);
                d = atof(string);
                i = (unsigned int) in_vtod(d);
                higherv[2]= min(max(i, lowerv[2]+1), O_HIGHER_MAX);
            }
            break;
        case O_OFFSET_2:
            if (!out_smart)
            {
                GetDlgItemText(hDlg, O_OFFSET_2, string, 10);
                d = atof(string);
                i = (unsigned int) in_vtod(d+10.0)-65536;
                ch_offset[2]= min(max(i, O_OFFSET_MIN), O_OFFSET_MAX);
            }
            break;
        case O_LOWER_V_3:
            if (!out_smart)
            {
                GetDlgItemText(hDlg, O_LOWER_V_3, string, 10);
                d = atof(string);
                i = (unsigned int) in_vtod(d);
                lowerv[3]= min(max(i, O_LOWER_MIN), higherv[3]-1);
            }
            break;
        case O_HIGHER_V_3:
            if (!out_smart)
            {
                GetDlgItemText(hDlg, O_HIGHER_V_3, string, 10);
                d = atof(string);
                i = (unsigned int) in_vtod(d);
                higherv[3]= min(max(i, lowerv[3]+1), O_HIGHER_MAX);
            }
            break;
        case O_OFFSET_3:
            if (!out_smart)
            {
                GetDlgItemText(hDlg, O_OFFSET_3, string, 10);
                d = atof(string);
                i = (unsigned int) in_vtod(d+10.0)-65536;
                ch_offset[3]= min(max(i, O_OFFSET_MIN), O_OFFSET_MAX);
            }
            break;
        case O_TRIGGER_VALUE:
            if (!out_smart)
            {
                GetDlgItemText(hDlg, O_TRIGGER_VALUE, string, 10);
                d = atof(string);
                i = (unsigned int) in_vtod(d);
                trigger_val= min(max(i, O_TRIGGER_MIN), O_TRIGGER_MAX);
            }
            break;
        case TIME_PER_DIVISION:
            if (!out_smart)
            {
                GetDlgItemText(hDlg, TIME_PER_DIVISION, string, 10);
                d = atof(string);
                i = (unsigned int) d;
                time_per_div= min(max(i, O_TIME_MIN), O_TIME_MAX);
            }
            break;
        case ENTER:
            for (i=0;i<4;i++) repaint_chs(hDlg,i);
            repaint_time(hDlg);
            repaint_trigger(hDlg);
//            Rectangle(oscill_hdc,14,15,312,334);
            
            break;
        case O_EXIT:
            oscill_reading=0;
            for(i=0;i<4;i++) free(oscill_ch_data[i]);
            outpw(cfg3,0x2400);
            outp(cntrcmd,0xB4); /* cntr3 in rate generation mode */
            clock_used=0;
            DeleteObject(hpen_black);
            DeleteObject(hpen_white);
            DeleteObject(hpen_red);
            DeleteObject(hpen_green);
            DeleteObject(hpen_blue);
            DeleteObject(hpen_dash);
            DeleteObject(oscill_area);
//            ReleaseDC(hDlg,oscill_hdc);
            EndDialog(hDlg, TRUE);
            return (TRUE);
            break;
        }
        break;
    }
    return (FALSE);
}                                                                  

void repaint_chs(HWND hDlg,long int ch)
{
    switch(ch)
    {
        case 0:
            sprintf(string,"%0.3f",in_dtov((float)lowerv[0]));
            SetDlgItemText(hDlg, O_LOWER_V_0, string);
            sprintf(string,"%0.3f",in_dtov((float)higherv[0]));
            SetDlgItemText(hDlg, O_HIGHER_V_0, string);
            sprintf(string,"%0.3f",in_dtov((float)ch_offset[0]+65536)-10.0);
            SetDlgItemText(hDlg, O_OFFSET_0, string);
            SetScrollPos(GetDlgItem(hDlg, O_LOWER_SCROLL_0), SB_CTL, DTOI(lowerv[0],O_LOWER_MIN,higherv[0]-1), TRUE);
            SetScrollPos(GetDlgItem(hDlg, O_HIGHER_SCROLL_0), SB_CTL, DTOI(higherv[0],lowerv[0]+1,O_HIGHER_MAX), TRUE);
            SetScrollPos(GetDlgItem(hDlg, O_OFFSET_0_SCROLL), SB_CTL, DTOI(ch_offset[0],O_OFFSET_MIN,O_OFFSET_MAX), TRUE);
            break;
        case 1:
            sprintf(string,"%0.3f",in_dtov((float)lowerv[1]));
            SetDlgItemText(hDlg, O_LOWER_V_1, string);
            sprintf(string,"%0.3f",in_dtov((float)higherv[1]));
            SetDlgItemText(hDlg, O_HIGHER_V_1, string);
            sprintf(string,"%0.3f",in_dtov((float)ch_offset[1]+65536)-10.0);
            SetDlgItemText(hDlg, O_OFFSET_1, string);
            SetScrollPos(GetDlgItem(hDlg, O_LOWER_SCROLL_1), SB_CTL, DTOI(lowerv[1],O_LOWER_MIN,higherv[1]-1), TRUE);
            SetScrollPos(GetDlgItem(hDlg, O_HIGHER_SCROLL_1), SB_CTL, DTOI(higherv[1],lowerv[1]+1,O_HIGHER_MAX), TRUE);
            SetScrollPos(GetDlgItem(hDlg, O_OFFSET_1_SCROLL), SB_CTL, DTOI(ch_offset[1],O_OFFSET_MIN,O_OFFSET_MAX), TRUE);
            break;
        case 2:
            sprintf(string,"%0.3f",in_dtov((float)lowerv[2]));
            SetDlgItemText(hDlg, O_LOWER_V_2, string);
            sprintf(string,"%0.3f",in_dtov((float)higherv[2]));
            SetDlgItemText(hDlg, O_HIGHER_V_2, string);
            sprintf(string,"%0.3f",in_dtov((float)ch_offset[2]+65536)-10.0);
            SetDlgItemText(hDlg, O_OFFSET_2, string);
            SetScrollPos(GetDlgItem(hDlg, O_LOWER_SCROLL_2), SB_CTL, DTOI(lowerv[2],O_LOWER_MIN,higherv[2]-1), TRUE);
            SetScrollPos(GetDlgItem(hDlg, O_HIGHER_SCROLL_2), SB_CTL, DTOI(higherv[2],lowerv[2]+1,O_HIGHER_MAX), TRUE);
            SetScrollPos(GetDlgItem(hDlg, O_OFFSET_2_SCROLL), SB_CTL, DTOI(ch_offset[2],O_OFFSET_MIN,O_OFFSET_MAX), TRUE);
            break;
        case 3:
            sprintf(string,"%0.3f",in_dtov((float)lowerv[3]));
            SetDlgItemText(hDlg, O_LOWER_V_3, string);
            sprintf(string,"%0.3f",in_dtov((float)higherv[3]));
            SetDlgItemText(hDlg, O_HIGHER_V_3, string);
            sprintf(string,"%0.3f",in_dtov((float)ch_offset[3]+65536)-10.0);
            SetDlgItemText(hDlg, O_OFFSET_3, string);
            SetScrollPos(GetDlgItem(hDlg, O_LOWER_SCROLL_3), SB_CTL, DTOI(lowerv[3],O_LOWER_MIN,higherv[3]-1), TRUE);
            SetScrollPos(GetDlgItem(hDlg, O_HIGHER_SCROLL_3), SB_CTL, DTOI(higherv[3],lowerv[3]+1,O_HIGHER_MAX), TRUE);
            SetScrollPos(GetDlgItem(hDlg, O_OFFSET_3_SCROLL), SB_CTL, DTOI(ch_offset[3],O_OFFSET_MIN,O_OFFSET_MAX), TRUE);
            break;
    }
}    

void repaint_time(HWND hDlg)
{
    out_smart=1;
    SetScrollPos(GetDlgItem(hDlg, TIME_PER_DIVISION_SCROLL), SB_CTL, DTOI(time_per_div,O_TIME_MIN,O_TIME_MAX), TRUE);
    sprintf(string,"%ld",time_per_div);
    SetDlgItemText(hDlg, TIME_PER_DIVISION, string);
    out_smart=0;
}

#ifdef OLD
static void repaint_grid(HWND hDlg)
{
    int i;
    HPEN hpen_old;
    HDC oscill_hdc;
    
    oscill_hdc=GetDC(hDlg);

    hpen_old=SelectObject(oscill_hdc,hpen_dash);
        
    for(i=1;i<10;i++)
    {
//        MoveTo(oscill_hdc,O_X_MIN,i*(O_Y_MAX-O_Y_MIN+1)/10+O_Y_MIN);
        MoveToEx(oscill_hdc,O_X_MIN,i*(O_Y_MAX-O_Y_MIN+1)/10+O_Y_MIN,NULL);
        LineTo(oscill_hdc,O_X_MAX+1,i*(O_Y_MAX-O_Y_MIN+1)/10+O_Y_MIN);
//        MoveTo(oscill_hdc,i*(O_X_MAX-O_X_MIN+1)/10+O_X_MIN,O_Y_MIN);
        MoveToEx(oscill_hdc,i*(O_X_MAX-O_X_MIN+1)/10+O_X_MIN,O_Y_MIN,NULL);
        LineTo(oscill_hdc,i*(O_X_MAX-O_X_MIN+1)/10+O_X_MIN,O_Y_MAX+1);
    }
    SelectObject(oscill_hdc,hpen_old);
    ReleaseDC(hDlg,oscill_hdc);
}
#endif

void repaint_trigger(HWND hDlg)
{
    out_smart=1;
    SetScrollPos(GetDlgItem(hDlg, O_TRIGGER_SCROLL), SB_CTL, DTOI(trigger_val,O_TRIGGER_MIN,O_TRIGGER_MAX), TRUE);
    sprintf(string,"%0.3f",in_dtov((float)trigger_val));
    SetDlgItemText(hDlg, O_TRIGGER_VALUE, string);
    out_smart=0;
}
        
