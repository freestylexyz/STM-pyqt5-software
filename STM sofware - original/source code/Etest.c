#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "stm.h"
#include "dio.h"
#include "clock.h"
#include "etest.h"
#include "serial.h"

static unsigned int ch = 0;			// current "analog" output ch (0<=ch<=11)
static unsigned int ch_data;
static unsigned int digi_ch = 12;	// current "digital" output ch (12<=digi_ch<=15)
static unsigned int digi_ch_data;

static unsigned int dac_ranges;
static unsigned int out_range;
static unsigned int old_dac_data[16];

// Four tests (calib., ramp, input, output) are supposed to be run
// with a bnc connection between one output (ramp_output_ch) and one
// input (ramp_input_ch).
static unsigned int ramp_from_bits = 0;
static unsigned int ramp_to_bits = 4095;
static unsigned int ramp_output_ch = 0;
static unsigned int ramp_input_ch = 0;
static int ramp_data[MAX+1];

extern char string[];

extern unsigned int dac_data[16];
extern unsigned int out1;
extern int feedback,x_range,y_range,z_range,x_offset_range,
    y_offset_range,z_offset_range,i_set_range,sample_bias_range;
 
static int out_smart = 0;

static void repaint_all(HWND);
static void repaint_output(HWND);
static void repaint_digi(HWND);
static void calc_digi_ch(HWND);
static void repaint_ramp(HWND);
static void etest_ramp(HWND);
static void repaint_ramp_graph(HWND);
static void etest_test_input(HWND);
static void etest_test_output(HWND);
static void etest_calibration(HWND);

BOOL FAR PASCAL EtestDlg(HWND,unsigned,WPARAM,LPARAM);
// Allows for primitive verification of opto and comp interface functions via four
// tests (ramp, calibration, test input, test output) and direct input
// and output. The input portion duplicates the Inputs dialog and the output
// portion duplicates the Outputs dialog, though here the outputs are divided
// into "analog" channels (0..11) and "digital" channels (12..15).

BOOL FAR PASCAL EtestDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{
  unsigned int i_data;
  int id;
  int i,mini,maxi,delt,ddelt;
  PAINTSTRUCT ps;

  switch(Message) {
    case WM_INITDIALOG:
	  // save dac_data on entry; will restore on exit
	  for(i = 0;i < 16;i++) old_dac_data[i] = dac_data[i];

	  // init test variables
	  ch_data = dac_data[ch];
      digi_ch_data = dac_data[digi_ch];
      dac_ranges = dac_data[range_ch];

	  // init controls and graphics
	  SetScrollRange(GetDlgItem(hDlg,O_CH_SCROLL),SB_CTL,0,11,FALSE);
      SetScrollRange(GetDlgItem(hDlg,O_CH_SCROLL2),SB_CTL,12,15,FALSE);
      SetScrollRange(GetDlgItem(hDlg,O_DATA_SCROLL),SB_CTL,0,4095,FALSE);
      for(i=0;i<4;i++) {
        SetDlgItemInt(hDlg,I_DATA0+i,0,FALSE);
        SetDlgItemText(hDlg,I_BITS0+i,in_dtob(0));
		sprintf(string,"%.5f",in_dtov(0.0));
        SetDlgItemText(hDlg,I_VOLTS0+i,string);
      }
      repaint_all(hDlg);
      break;

    case WM_PAINT:
        BeginPaint(hDlg,&ps);
        repaint_all(hDlg);
        EndPaint(hDlg,&ps);
      break;

    case WM_HSCROLL:
      if(!out_smart)
      {
          id = getscrollid();
          switch(id) {
            case O_CH_SCROLL:
              i = (int)ch;
              mini = 0;
              maxi = 11;
              delt = 1;
              ddelt = 2;
              break;
            case O_CH_SCROLL2:
              i = (int)digi_ch;
              mini = 12;
              maxi = 15;
              delt = 1;
              ddelt = 2;
              break;
            case O_DATA_SCROLL:
              i = (int)ch_data;
              mini = 0;
              maxi = 4095;
              delt = 1;
              ddelt = 100;
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
          switch(id)
          {
            case O_CH_SCROLL:
              ch = (unsigned int)i;
              ch_data = dac_data[ch];
              repaint_output(hDlg);
              break;
            case O_CH_SCROLL2:
              digi_ch = (unsigned int)i;
              digi_ch_data = dac_data[digi_ch];
              repaint_digi(hDlg);
              break;
            case O_DATA_SCROLL:
              ch_data = (unsigned int)i;
              repaint_output(hDlg);
              break;
		  }
      }
      break;

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
		// Inputs
	    case I_UPDATE:
          for(i = 0;i < 4;i++)
            if(IsDlgButtonChecked(hDlg,I_CH0 + i)) {
              //dio_in_ch(i);
              //dio_in_data(&i_data);
              dio_in_serial(i,&i_data);
              SetDlgItemInt(hDlg,I_DATA0 + i,i_data,FALSE);
              SetDlgItemText(hDlg,I_BITS0 + i,in_dtob(i_data));
			  sprintf(string,"%.5f",in_dtov((float)i_data));
              SetDlgItemText(hDlg,I_VOLTS0 + i,string);
            }
          break;

		// "Analog" Outputs (Ch 0 -> Ch 11)
        case SEND_OUTPUT:
          if(ch == range_ch)
            dac_ranges = ch_data;
          dac_data[ch] = ch_data;
          //dio_out(ch,ch_data);
          dio_out_serial(ch,ch_data);
          break;
        case FIVE_RANGE:
          CheckDlgButton(hDlg,FIVE_RANGE,1);
          CheckDlgButton(hDlg,TEN_RANGE,0);
          out_range = 5;
          dac_ranges |= 1 << ch;
          dac_data[range_ch] = dac_ranges;
          //dio_out(range_ch,dac_ranges);
          dio_out_serial(range_ch,dac_ranges);
          repaint_output(hDlg); 
		  if(digi_ch == range_ch)
		  {
			digi_ch_data = dac_ranges;
			repaint_digi(hDlg);		// in case digi_ch == range ch (12)
		  }
          break;
        case TEN_RANGE:
          CheckDlgButton(hDlg,FIVE_RANGE,0);
          CheckDlgButton(hDlg,TEN_RANGE,1);
          out_range = 10;
          dac_ranges &= ~(1 << ch);
          dac_data[range_ch] = dac_ranges;
          //dio_out(range_ch,dac_ranges);
          dio_out_serial(range_ch,dac_ranges);
          repaint_output(hDlg);
		  if(digi_ch == range_ch)
		  {
			digi_ch_data = dac_ranges;
			repaint_digi(hDlg);		// in case digi_ch == range ch (12)
		  }
          break;
        case O_CH_EDIT:
          if(!out_smart)
          {
              GetDlgItemText(hDlg,O_CH_EDIT,string,5);
              i = atoi(string);
              i = min(max(i,0),11);
              ch = (unsigned int)i;
              repaint_output(hDlg);
          }
          break;
        case O_DATA_EDIT:
          if(!out_smart)
          {
              GetDlgItemText(hDlg,O_DATA_EDIT,string,5);
              i = atoi(string);
              i = min(max(i,0),4095);
              ch_data = (unsigned int)i;
            }
          break;
        case O_BITS_EDIT:
          if(!out_smart)
          {
              GetDlgItemText(hDlg,O_BITS_EDIT,string,13);
              i = btod(string);
              i = min(max(i,0),4095);
              ch_data = (unsigned int)i;
          }
          break;
        case O_VOLTS_EDIT:
          if(!out_smart)
          {
              GetDlgItemText(hDlg,O_VOLTS_EDIT,string,9);
              i = vtod(atof(string),get_range(ch));
              i = min(max(i,0),4095);
              ch_data = (unsigned int)i;
          }
          break;

		// "Digital" Outputs (Ch 12 -> Ch 15)
        case O_CH_EDIT2:
          if(!out_smart)
          {
              GetDlgItemText(hDlg,O_CH_EDIT2,string,5);
              i = atoi(string);
              i = min(max(i,12),15);
              digi_ch = (unsigned int)i;
              digi_ch_data = dac_data[digi_ch];
              repaint_digi(hDlg);
            }
          break;
        case ETEST_DIGITAL_OUTPUT:
          dac_data[digi_ch] = digi_ch_data;
          //dio_out(digi_ch,digi_ch_data);
          dio_out_serial(digi_ch,digi_ch_data);
		  if(digi_ch == 12)
		  {
			dac_ranges = digi_ch_data;
			repaint_output(hDlg);
		  }
          break;
        case ETEST_ON12:
        case ETEST_ON11:
        case ETEST_ON10:
        case ETEST_ON9:
        case ETEST_ON8:
        case ETEST_ON7:
        case ETEST_ON6:
        case ETEST_ON5:
        case ETEST_ON4:
        case ETEST_ON3:
        case ETEST_ON2:
        case ETEST_ON1:
          if(!out_smart)
            calc_digi_ch(hDlg);
          break;

		// Tests
        case ETEST_CALIBRATION:
#ifdef DAC16
			if(ramp_output_ch==10 || ramp_output_ch==11)
			{
				if( MessageBox(GetFocus(),"Are you sure?  Its best to test the 16 bit channels with a DVM.","Warning",MB_OKCANCEL)
					== IDOK) etest_calibration(hDlg);
			}
			else
            etest_calibration(hDlg);
#else
			etest_calibration(hDlg);
#endif
            //etest_calibration(hDlg);
            break;
        case ETEST_TEST_OUTPUT:
#ifdef DAC16
			if(ramp_output_ch==10 || ramp_output_ch==11)
			{
				if( MessageBox(GetFocus(),"Are you sure?  Its best to test the 16 bit channels with a DVM.","Warning",MB_OKCANCEL)
					== IDOK)  etest_test_output(hDlg);
			}
			else
			etest_test_output(hDlg);	
#else
			etest_test_output(hDlg);
#endif
            //etest_test_output(hDlg);
            break;
        case ETEST_TEST_INPUT:
#ifdef DAC16
			if(ramp_output_ch==10 || ramp_output_ch==11)
			{
				if( MessageBox(GetFocus(),"Are you sure?  Its best to test the 16 bit channels with a DVM.","Warning",MB_OKCANCEL)
					== IDOK)  etest_test_input(hDlg);
			}
			else
            etest_test_input(hDlg);
#else
			etest_test_input(hDlg);
#endif
            break;
        case ETEST_RAMP_DO_IT:
#ifdef DAC16
			if(ramp_output_ch==10 || ramp_output_ch==11)
			{
				if( MessageBox(GetFocus(),"Are you sure?  Its best to test the 16 bit channels with a DVM.","Warning",MB_OKCANCEL)
					== IDOK) etest_ramp(hDlg);
			}
			else
            etest_ramp(hDlg);
#else
			etest_ramp(hDlg);
#endif
            break;
        case ETEST_RAMP_FROM_BITS:
            if(!out_smart)
            {
              GetDlgItemText(hDlg,LOWORD(wParam),string,9);
              i = atoi(string);
              ramp_from_bits = min(max(i,0),4095);
              if(i != ramp_from_bits) repaint_ramp(hDlg);
            }
            break;
        case ETEST_RAMP_TO_BITS:
            if(!out_smart)
            {
              GetDlgItemText(hDlg,LOWORD(wParam),string,9);
              i = atoi(string);
              ramp_to_bits = min(max(i,0),4095);
              if(i != ramp_to_bits) repaint_ramp(hDlg);
            }
            break;
        case ETEST_RAMP_OUTPUT_CH:
            if(!out_smart)
            {
              GetDlgItemText(hDlg,LOWORD(wParam),string,9);
              i = atoi(string);
              ramp_output_ch = min(max(i,0),11);
		      if(i != ramp_output_ch) repaint_ramp(hDlg);
            }
            break;     
        case ETEST_RAMP_INPUT_CH:
            if(!out_smart)
            {
              GetDlgItemText(hDlg,LOWORD(wParam),string,9);
              i = atoi(string);
              ramp_input_ch = min(max(i,0),3);
              if(i != ramp_input_ch) repaint_ramp(hDlg);
            }
            break;     

        case ENTER:
          repaint_all(hDlg);
          break;

        case ETEST_EXIT:
          for(i = 15;i >= 0;i--)
          {
            dac_data[i] = old_dac_data[i];
            //dio_out(i,dac_data[i]);
            dio_out_serial(i,dac_data[i]);
          }
          EndDialog(hDlg,TRUE);
          return(TRUE);
          break;
      }
      break;
  }
  return(FALSE);
}
      
static void repaint_all(HWND hDlg)
{
    repaint_output(hDlg);
    repaint_digi(hDlg);
    repaint_ramp(hDlg);
    clear_area(hDlg,ETEST_X_MIN,ETEST_Y_MIN,ETEST_X_MAX,ETEST_Y_MAX,
        RGB(255,255,255));
    repaint_grid(hDlg,ETEST_X_MIN,ETEST_X_MAX,ETEST_Y_MIN,ETEST_Y_MAX);
}

static void repaint_output(HWND hDlg)
{      
    HWND handle;

    out_smart = 1;    
    if(dac_ranges & (1<<ch))
    {
        CheckDlgButton(hDlg,FIVE_RANGE,1);
        CheckDlgButton(hDlg,TEN_RANGE,0);
        out_range = 5;
    }
    else
    {
        CheckDlgButton(hDlg,FIVE_RANGE,0);
        CheckDlgButton(hDlg,TEN_RANGE,10);
        out_range = 10;
    }
    handle = GetDlgItem(hDlg,O_CH_SCROLL);
    SetScrollPos(handle,SB_CTL,ch,TRUE);
    SetDlgItemInt(hDlg,O_CH_EDIT,ch,FALSE);
    
    handle = GetDlgItem(hDlg,O_DATA_SCROLL);
    SetScrollPos(handle,SB_CTL,ch_data,TRUE);
    SetDlgItemInt(hDlg,O_DATA_EDIT,ch_data,FALSE);
    SetDlgItemText(hDlg,O_BITS_EDIT,dtob(ch_data));
	sprintf(string,"%.5f",dtov(ch_data,get_range(ch)));
    SetDlgItemText(hDlg,O_VOLTS_EDIT,string);
    out_smart = 0;
}

static void repaint_digi(HWND hDlg)
{
    out_smart = 1;
    SetDlgItemInt(hDlg,O_CH_EDIT2,digi_ch,FALSE);
    SetScrollPos(GetDlgItem(hDlg,O_CH_SCROLL2),SB_CTL,digi_ch,TRUE);
    if(digi_ch_data & 1<<11)
    {
        CheckDlgButton(hDlg,ETEST_ON12,1);
    }
    else CheckDlgButton(hDlg,ETEST_ON12,0);
    if(digi_ch_data & 1<<10)
    {
        CheckDlgButton(hDlg,ETEST_ON11,1);
    }
    else CheckDlgButton(hDlg,ETEST_ON11,0);
    if(digi_ch_data & 1<<9)
    {
        CheckDlgButton(hDlg,ETEST_ON10,1);
    }
    else CheckDlgButton(hDlg,ETEST_ON10,0);
    if(digi_ch_data & 1<<8)
    {                                    
        CheckDlgButton(hDlg,ETEST_ON9,1);
    }
    else CheckDlgButton(hDlg,ETEST_ON9,0);
    if(digi_ch_data & 1<<7)
    {
        CheckDlgButton(hDlg,ETEST_ON8,1);
    }
    else CheckDlgButton(hDlg,ETEST_ON8,0);
    if(digi_ch_data & 1<<6)
    {
        CheckDlgButton(hDlg,ETEST_ON7,1);
    }
    else CheckDlgButton(hDlg,ETEST_ON7,0);
    if(digi_ch_data & 1<<5)
    {
        CheckDlgButton(hDlg,ETEST_ON6,1);
    }
    else CheckDlgButton(hDlg,ETEST_ON6,0);
    if(digi_ch_data & 1<<4)
    {
        CheckDlgButton(hDlg,ETEST_ON5,1);
    }
    else CheckDlgButton(hDlg,ETEST_ON5,0);
    if(digi_ch_data & 1<<3)
    {
        CheckDlgButton(hDlg,ETEST_ON4,1);
    }
    else CheckDlgButton(hDlg,ETEST_ON4,0);
    if(digi_ch_data & 1<<2)
    {
        CheckDlgButton(hDlg,ETEST_ON3,1);
    }
    else CheckDlgButton(hDlg,ETEST_ON3,0);
    if(digi_ch_data & 1<<1)
    {
        CheckDlgButton(hDlg,ETEST_ON2,1);
    }
    else CheckDlgButton(hDlg,ETEST_ON2,0);
    if(digi_ch_data & 1)
    {
        CheckDlgButton(hDlg,ETEST_ON1,1);
    }
    else CheckDlgButton(hDlg,ETEST_ON1,0);

    out_smart = 0;
}

static void calc_digi_ch(HWND hDlg)
{
    int i;
    
    i = 0;

    if(IsDlgButtonChecked(hDlg,ETEST_ON12)) i += 1<<11;
    if(IsDlgButtonChecked(hDlg,ETEST_ON11)) i += 1<<10;
    if(IsDlgButtonChecked(hDlg,ETEST_ON10)) i += 1<<9;
    if(IsDlgButtonChecked(hDlg,ETEST_ON9)) i += 1<<8;
    if(IsDlgButtonChecked(hDlg,ETEST_ON8)) i += 1<<7;
    if(IsDlgButtonChecked(hDlg,ETEST_ON7)) i += 1<<6;
    if(IsDlgButtonChecked(hDlg,ETEST_ON6)) i += 1<<5;
    if(IsDlgButtonChecked(hDlg,ETEST_ON5)) i += 1<<4;
    if(IsDlgButtonChecked(hDlg,ETEST_ON4)) i += 1<<3;
    if(IsDlgButtonChecked(hDlg,ETEST_ON3)) i += 1<<2;
    if(IsDlgButtonChecked(hDlg,ETEST_ON2)) i += 1<<1;
    if(IsDlgButtonChecked(hDlg,ETEST_ON1)) i += 1;
    
    digi_ch_data = i;
}

static void repaint_ramp(HWND hDlg)
{
    out_smart = 1;
    SetDlgItemInt(hDlg,ETEST_RAMP_FROM_BITS,ramp_from_bits,FALSE);
    SetDlgItemInt(hDlg,ETEST_RAMP_TO_BITS,ramp_to_bits,FALSE);
    SetDlgItemInt(hDlg,ETEST_RAMP_OUTPUT_CH,ramp_output_ch,FALSE);
    SetDlgItemInt(hDlg,ETEST_RAMP_INPUT_CH,ramp_input_ch,FALSE);
    out_smart = 0;
}

static void repaint_ramp_graph(HWND hDlg)
{
    int start,end,dir;
    int min,max,i,j,min1,max1;
    int xs,ys;
    HPEN hpen_old,hpen_blue,hpen_red;
    HDC hdc;
	float y_scale,x_scale;
    
    clear_area(hDlg,ETEST_X_MIN,ETEST_Y_MIN,ETEST_X_MAX,ETEST_Y_MAX,
        RGB(255,255,255));
    repaint_grid(hDlg,ETEST_X_MIN,ETEST_X_MAX,ETEST_Y_MIN,ETEST_Y_MAX);

	start = ramp_from_bits;
    end = ramp_to_bits;
    if(ramp_to_bits > ramp_from_bits)
    {
		dir = 1;
    }
    else
    {
		dir = -1;
    }

	max = -1;
    min = IN_MAX + 1;
    for(i = start;i != (end + dir);i += dir)
    {
        if(ramp_data[i] < min) min = ramp_data[i];
        if(ramp_data[i] > max) max = ramp_data[i];
    }
    
	if(end > start)
	{
		min1 = (start<<4) - 1;
		max1 = (end<<4) + 1;
    }
	else
	{
		max1 = (start<<4) - 1;
		min1 = (end<<4) + 1;
    }

	if(get_range(ramp_output_ch) == 1)
	{
			min1 = IN_ZERO / 2 + min1 / 2;
			max1 = IN_ZERO / 2 + max1 / 2;
	}

    min = min(min1,min);
    max = max(max1,max);
    
	x_scale = (float)(ETEST_X_MAX - ETEST_X_MIN + 1) / (abs(end - start) + 1);
	y_scale = (float)(ETEST_Y_MAX - ETEST_Y_MIN + 1) / (max - min + 1);
    
	hdc = GetDC(hDlg);
    hpen_blue = CreatePen(PS_SOLID,1,RGB(0,0,255));	// blue pen (theory)

    hpen_old = SelectObject(hdc,hpen_blue);
    xs = ETEST_X_MIN;
	if(ramp_to_bits > ramp_from_bits)
		ys = -(int)((min1 + 1 - max) * y_scale) + ETEST_Y_MIN;
	else
		ys = -(int)((max1 + 1 - max) * y_scale) + ETEST_Y_MIN;
	//    MoveTo(hdc,xs,ys);
    MoveToEx(hdc,xs,ys,NULL);
    xs = ETEST_X_MAX;
	if(ramp_to_bits > ramp_from_bits)
		ys = -(int)((max1 + 1 - max) * y_scale) + ETEST_Y_MIN;
	else
		ys = -(int)((min1 + 1 - max) * y_scale) + ETEST_Y_MIN;
    LineTo(hdc,xs,ys);
    SelectObject(hdc,hpen_old);
    DeleteObject(hpen_blue);
    
    hpen_red = CreatePen(PS_SOLID,1,RGB(255,0,0));	// red pen (actual data)

	hpen_old = SelectObject(hdc,hpen_red);
    i = start;
	j = 0;
    xs = (int)((i - start) * x_scale) + ETEST_X_MIN;
    ys = -(int)((ramp_data[j] - max) * y_scale) + ETEST_Y_MIN;
//    MoveTo(hdc,xs,ys);
    MoveToEx(hdc,xs,ys,NULL);
    for(i = start + dir;i != end;i += dir)
    {
        xs = (int)(abs(i - start) * x_scale) + ETEST_X_MIN;
        ys = -(int)((ramp_data[j++] - max) * y_scale) + ETEST_Y_MIN;
        LineTo(hdc,xs,ys);
    }
        
    SelectObject(hdc,hpen_old);
    DeleteObject(hpen_red);
    
    ReleaseDC(hDlg,hdc);
}

static void etest_ramp(HWND hDlg)
{
// etest_ramp assumes there is a bnc connection between ramp_output_ch and
// ramp_input_ch. For each bit between ramp_from_bits and ramp_to_bits,
// output on ramp_output_ch and read ramp_input_ch. Results are given
// graphically (see repaint_ramp_graph above).

    unsigned int old_data;
//    int i;

	if(ramp_from_bits == ramp_to_bits) return;

	// ramp to initial ramp value (ramp_from_bits)
    old_data = dac_data[ramp_output_ch];
	ramp_serial(ramp_output_ch,old_data,ramp_from_bits,ETEST_MOVE_WAIT,0);
   
	// bitwise ramp; output and input at each point
	ramp_read_serial(ramp_input_ch,ramp_output_ch,
			ramp_from_bits,ramp_to_bits,ETEST_MOVE_WAIT,ramp_data,0);

//  for debugging...
//	for(i = 0;i < MAX;i++)
//		ramp_data[i] = ((MAX - i)<<4)/2;

	// ramp to pre-test value (old_data)
	ramp_serial(ramp_output_ch,ramp_to_bits,old_data,ETEST_MOVE_WAIT,0);

    repaint_ramp_graph(hDlg);
}

static void etest_test_input(HWND hDlg)
{
    unsigned int old_data;
    int old_range,i,j;
    unsigned int read_data,read_data2;
    int bit[16];
    int bad;
    char results[1000];
    int test_bits;
    int offset;
    
	// save data and range for output ch
	old_data = dac_data[ramp_output_ch];
    old_range = get_range(ramp_output_ch);
 
	// clear bit flags
    for(i = 0;i < 16;i++) bit[i] = 0;

// BITS 0,1,2: not tested

// BIT 3 Test
	set_range_serial(ramp_output_ch,1);
    dio_out_serial(ramp_output_ch,0);
    dio_in_serial(ramp_input_ch,&read_data);
    bad = 1;
    if(read_data & (1<<4))
    {
        for(i = 1;i < 4;i++)
        {
            dio_out_serial(ramp_output_ch,i);
            dio_in_serial(ramp_input_ch,&read_data);
            if(!(read_data & (1<<4))) bad = 0;
        }
    }
    else
    {
        for(i = 1;i < 4;i++)
        {
            dio_out_serial(ramp_output_ch,i);
            dio_in_serial(ramp_input_ch,&read_data);
            if(read_data & (1<<4)) bad = 0;
        }
    }
    bit[3] = bad;

// BIT 4 Test
    set_range_serial(ramp_output_ch,2);
    dio_out_serial(ramp_output_ch,ZERO + 16 - ETEST_INPUT_TEST_BITS);
    dio_in_serial(ramp_input_ch,&read_data);
    for(i = 1;i<ETEST_INPUT_TEST_BITS * 2 + 1;i++)
    {
        dio_out_serial(ramp_output_ch,ZERO + 16 - ETEST_INPUT_TEST_BITS + i);
        dio_in_serial(ramp_input_ch,&read_data2);
        if(read_data & (1<<4))
        {
            if(!(read_data2 & (1<<4))) break;
        }
        else
        {
            if(read_data2 & (1<<4)) break;
        }
    }
    if(i == ETEST_INPUT_TEST_BITS * 2 + 1) bit[4] = 1;

// BITS 5 - 16:
	for(j = 5;j < 16;j++)
    {
        if(j < 15) offset = ZERO;
        else offset = 0;
        test_bits = (float)(1<<(j - 4)) * (float)(ETEST_INPUT_TEST_BITS) / 100.0;
        if(test_bits <= 1) test_bits = 2;
        dio_out_serial(ramp_output_ch,(1<<(j - 4)) - test_bits + offset);
        dio_in_serial(ramp_input_ch,&read_data);
        for(i = 1;i < test_bits * 2 + 1;i++)
        {
            dio_out_serial(ramp_output_ch,(1<<(j - 4)) - test_bits + i + offset);
            dio_in_serial(ramp_input_ch,&read_data2);
            if(read_data & (1<<j))
            {
                if(!(read_data2 & (1<<j))) break;
            }
            else
            {
                if(read_data2 & (1<<j)) break;
            }
        }
        if(i == test_bits * 2 + 1) bit[j] = 1;
    }

	// show test results
	bad = 0;
    strcpy(results,"Bit(s) ");
    for(i = 0;i < 16;i++)
    {
        if(bit[i]) 
        {
            sprintf(string,"%d ",i);
            strcat(results,string);
            bad = 1;
        }
    }
    strcat(results," is/are bad.");
    if(bad) MessageBox(hDlg,results,"Test Results",MB_OK);
    else MessageBox(hDlg,"All bits ok.","Test Results",MB_OK);
        
	// restore data and range for output ch
    set_range_serial(ramp_output_ch,old_range);
    dio_out_serial(ramp_output_ch,old_data);
}

static void etest_test_output(HWND hDlg)
{
    int i;
    int bit[12];			// flags: 0=bit o.k., 1=bit bad
    unsigned int read_data1,read_data2;
    int diff;
    unsigned int old_data;
    int old_range;
    char results[1000];		// string for reporting results
    
	// save old settings
    old_data = dac_data[ramp_output_ch];
    old_range = get_range(ramp_output_ch);

    set_range_serial(ramp_output_ch,2);
    for(i = 0;i < 12;i++)
    {
		// clear flags
        bit[i] = 0;

		// output base value...
        dio_out_serial(ramp_output_ch,ETEST_OUTPUT_OFFSET);
		//...and read it back in
        dio_in_serial(ramp_input_ch,&read_data1);

		// output base value with i'th bit altered...
        dio_out_serial(ramp_output_ch,ETEST_OUTPUT_OFFSET + (1<<i));
		//...and read it back in
        dio_in_serial(ramp_input_ch,&read_data2);

		//compare the two
		diff = (read_data2 - read_data1);

		// set flag if nominal one bit difference is not close enough to actual value
		if(diff < (float)(1<<(i + 4)) * (float)(1.0 - (float)ETEST_OUTPUT_TOLERANCE / 100.0) ||
             diff > (float)(1<<(i + 4))*(float)(1.0 + (float)ETEST_OUTPUT_TOLERANCE / 100.0)) 
            bit[i] = 1;
    }

	// report results
    for(i = 0;i < 12;i++)
    {
        if(bit[i]) break;
    }
    if(i != 12)
    {
        strcpy(results,"Bit(s) ");
        for(i = 0;i < 12;i++)
        {
            if(bit[i])
            {
                sprintf(string,"%d ",i);
                strcat(results,string);
            }
        }
        strcat(results,"is/are bad.");
    }
    else
    {
        strcpy(results,"All bits ok.");
    }
    MessageBox(hDlg,results,"Test Results",MB_OK);
    
	// restore old settings
    set_range_serial(ramp_output_ch,old_range);
    dio_out_serial(ramp_output_ch,old_data);
}

static void etest_calibration(HWND hDlg)
{
    int i;
    int bit[12];
    unsigned int read_data1,read_data2;
    unsigned int old_data;
    int old_range;
    char results[1000];
    
	// save old settings
    old_data = dac_data[ramp_output_ch];
    old_range = get_range(ramp_output_ch);

    set_range_serial(ramp_output_ch,2);
    
	// what is diff of inputs before and after changing output by 1 bit?
    for(i = 0;i < 12;i++)
    {
        bit[i] = 0;
        dio_out_serial(ramp_output_ch,ETEST_OUTPUT_OFFSET);
        dio_in_serial(ramp_input_ch,&read_data1);
        dio_out_serial(ramp_output_ch,ETEST_OUTPUT_OFFSET + (1<<i));
        dio_in_serial(ramp_input_ch,&read_data2);
        bit[i] = (read_data2 - read_data1);
    }

	// show results
    results[0] = '\0';
    for(i = 0;i < 12;i++)
    {
        sprintf(string,"Bit %d is off by %f %%\n",i,
            (float)((1<<(i + 4)) - bit[i]) / (float)(1<<(i + 4)) * 100);
        strcat(results,string);
    }
    MessageBox(hDlg,results,"Test Results",MB_OK);
    
	// restore old settings
    set_range_serial(ramp_output_ch,old_range);
    dio_out_serial(ramp_output_ch,old_data);
}

// ******************************************************************************************
//		O L D * O L D * O L D * O L D * O L D * O L D * O L D * O L D * O L D * O L D
// ******************************************************************************************


#ifdef OLD
static void etest_ramp_old(HWND hDlg)
{
// etest_ramp assumes there is a bnc connection between ramp_output_ch and
// ramp_input_ch. For each bit between ramp_from_bits and ramp_to_bits,
// output on ramp_output_ch and read ramp_input_ch. Results are given
// graphically (see repaint_ramp_graph above).

    int dir = 1;
    int i;
    unsigned int old_data;
    unsigned int read_data;
    
	// ramp to initial ramp value (ramp_from_bits)
    old_data = dac_data[ramp_output_ch];
    if(ramp_from_bits < old_data) dir = -1;
    for(i = old_data;i != ramp_from_bits;i += dir)
    {
		dio_out(ramp_output_ch,i);
        dio_start_clock(ETEST_MOVE_WAIT);
        dio_wait_clock();
    }
   
	// bitwise ramp; output and input at each point
	dir = 1;
    if(ramp_from_bits > ramp_to_bits) dir = -1;
    dio_in_ch(ramp_input_ch);
    i = ramp_from_bits - dir;
    do
    {
        i += dir;
        dio_out(ramp_output_ch,i);
        dio_start_clock(ETEST_RAMP_WAIT);
        dio_wait_clock();
        dio_in_data(&read_data);
    } while(i != ramp_to_bits);
	
	// ramp to pre-test value (old_data)
    dir = 1;
    if(ramp_to_bits > old_data) dir = -1;
    for(i = ramp_to_bits;i != old_data;i += dir)
    {
        dio_out(ramp_output_ch,i);
        dio_start_clock(ETEST_MOVE_WAIT);
        dio_wait_clock();
    }
    dio_out(ramp_output_ch,old_data);

    repaint_ramp_graph(hDlg);
}

static void etest_test_input_old(HWND hDlg)
{
    unsigned int old_data;
    int old_range,i,j;
    unsigned int read_data,read_data2;
    int bit[16];
    int bad;
    char results[1000];
    int test_bits;
    int offset;
    
    dio_in_ch(ramp_input_ch);

    // save data and range for output ch
	old_data=dac_data[ramp_output_ch];
    old_range=get_range(ramp_output_ch);
    set_range(ramp_output_ch,2);	// unnecessary?

/*
    dio_out(ramp_output_ch,0);
    dio_start_clock(ETEST_RAMP_WAIT);
    dio_wait_clock();
    dio_in_data(&read_data);
    if (read_data>ETEST_INPUT_TOLERANCE)
    {
        bad=0;
        for(i=0;i<16;i++)
        {
            bit[i]=0;
            if (read_data & 1<<(i+1)) 
            {
                bit[i]=1;
                bad=1;
            }
        }
        if (bad)
        {
            strcpy(results,"Bit(s) ");
            for(i=0;i<16;i++)
            {
                if (bit[i]) 
                {
                    sprintf(string,"%d ",i);
                    strcat(results,string);
                }
                
            }
            strcat(results," is/are stuck high");
            MessageBox(hDlg,results,"Test Results",MB_OK);
        }
    }
*/
	// clear bit flags
    for(i=0;i<16;i++) bit[i]=0;

// BITS 0,1,2: don't worry about

// BIT 3: output 0,1,2,3,4 bits with +/- 5 V range (change of about .01 V) 
// If bit 3 is ok, bit 4 should toggle. (?)
	set_range(ramp_output_ch,1);
    dio_out(ramp_output_ch,0);
    dio_start_clock(ETEST_RAMP_WAIT);
    dio_wait_clock();
    dio_in_data(&read_data);
    bad = 1;
    if(read_data & (1<<4))
    {
        for(i=1;i<4;i++)
        {
            dio_out(ramp_output_ch,i);
            dio_start_clock(ETEST_RAMP_WAIT);
            dio_wait_clock();
            dio_in_data(&read_data);
            if (!(read_data & (1<<4))) bad = 0;
        }
    }
    else
    {
        for(i=1;i<4;i++)
        {
            dio_out(ramp_output_ch,i);
            dio_start_clock(ETEST_RAMP_WAIT);
            dio_wait_clock();
            dio_in_data(&read_data);
            if (read_data & (1<<4)) bad = 0;
        }
    }
    bit[3] = bad;

// BIT 4: see if it toggles
    set_range(ramp_output_ch,2);
    dio_out(ramp_output_ch,ZERO+16-ETEST_INPUT_TEST_BITS);
    dio_start_clock(ETEST_RAMP_WAIT);
    dio_wait_clock();
    dio_in_data(&read_data);
    for(i=1;i<ETEST_INPUT_TEST_BITS*2+1;i++)
    {
        dio_out(ramp_output_ch,ZERO+16-ETEST_INPUT_TEST_BITS+i);
        dio_start_clock(ETEST_RAMP_WAIT);
        dio_wait_clock();
        dio_in_data(&read_data2);
        if (read_data & (1<<4))
        {
            if (!(read_data2 & (1<<4))) break;
        }
        else
        {
            if (read_data2 & (1<<4)) break;
        }
    }
    if (i==ETEST_INPUT_TEST_BITS*2+1) bit[4]=1;

// BITS 5 - 16:
	for(j=5;j<16;j++)
    {
        if (j<15) offset=ZERO;
        else offset=0;
        test_bits=(float)(1<<(j-4))*(float)(ETEST_INPUT_TEST_BITS)/100.0;
        if (test_bits<=1) test_bits=2;
/*
        sprintf(string,"j %d test bits %d",j,test_bits);
        mprintf(string);
*/
        dio_out(ramp_output_ch,(1<<(j-4))-test_bits+offset);
        dio_start_clock(ETEST_RAMP_WAIT);
        dio_wait_clock();
        dio_in_data(&read_data);
        for(i=1;i<test_bits*2+1;i++)
        {
            dio_out(ramp_output_ch,(1<<(j-4))-test_bits+i+offset);
            dio_start_clock(ETEST_RAMP_WAIT);
            dio_wait_clock();
            dio_in_data(&read_data2);
            if (read_data & (1<<j))
            {
                if (!(read_data2 & (1<<j))) break;
            }
            else
            {
                if (read_data2 & (1<<j)) break;
            }
        }
        if (i==test_bits*2+1) bit[j]=1;
    }
    bad=0;
    strcpy(results,"Bit(s) ");
    for(i=0;i<16;i++)
    {
        if (bit[i]) 
        {
            sprintf(string,"%d ",i);
            strcat(results,string);
            bad=1;
        }
        
    }
    strcat(results," is/are bad.");
    if (bad) MessageBox(hDlg,results,"Test Results",MB_OK);
    else MessageBox(hDlg,"All bits ok.","Test Results",MB_OK);
        
	// restore data and range for output ch
    set_range(ramp_output_ch,old_range);
    dio_out(ramp_output_ch,old_data);
}

static void etest_test_output_old(HWND hDlg)
{
    int i;
    int bit[12];			// flags: 0=bit o.k., 1=bit bad
    unsigned int read_data1,read_data2;
    int diff;
    unsigned int old_data;
    int old_range;
    char results[1000];		// string for reporting results
    
	// save old settings
    old_data=dac_data[ramp_output_ch];
    old_range=get_range(ramp_output_ch);

    set_range(ramp_output_ch,2);
    dio_in_ch(ramp_input_ch);
    for(i=0;i<12;i++)
    {
		// clear flags
        bit[i]=0;

		// output base value...
        dio_out(ramp_output_ch,ETEST_OUTPUT_OFFSET);
        dio_start_clock(ETEST_RAMP_WAIT);
        dio_wait_clock();
		//...and read it back in
        dio_in_data(&read_data1);

		// output base value with i'th bit altered...
        dio_out(ramp_output_ch,ETEST_OUTPUT_OFFSET+(1<<i));
        dio_start_clock(ETEST_RAMP_WAIT);
        dio_wait_clock();
		//...and read it back in
        dio_in_data(&read_data2);

		//compare the two
		diff=(read_data2-read_data1);
/*
        sprintf(string,"before %d after %d diff %d bit %d",
            read_data1,read_data2,diff,i+1);
        mprintf(string);
*/

		// set flag if nominal one bit difference is not close enough to actual value
		if (diff<(float)(1<<(i+4))*(float)(1.0-(float)ETEST_OUTPUT_TOLERANCE/100.0) ||
             diff>(float)(1<<(i+4))*(float)(1.0+(float)ETEST_OUTPUT_TOLERANCE/100.0)) 
            bit[i]=1;
    }

	// report results
    for(i=0;i<12;i++)
    {
        if (bit[i]) break;
    }
    if (i!=12)
    {
        strcpy(results,"Bit(s) ");
        for(i=0;i<12;i++)
        {
            if (bit[i])
            {
                sprintf(string,"%d ",i);
                strcat(results,string);
            }
        }
        strcat(results,"is/are bad.");
    }
    else
    {
        strcpy(results,"All bits ok.");
    }
    MessageBox(hDlg,results,"Test Results",MB_OK);
    
	// restore old settings
    set_range(ramp_output_ch,old_range);
    dio_out(ramp_output_ch,old_data);
}

static void etest_calibration_old(HWND hDlg)
{
    int i;
    int bit[12];
    unsigned int read_data1,read_data2;
    unsigned int old_data;
    int old_range;
    char results[1000];
    
    
    dio_in_ch(ramp_input_ch);
    old_data=dac_data[ramp_output_ch];
    old_range=get_range(ramp_output_ch);
    set_range(ramp_output_ch,2);
    
    for(i=0;i<12;i++)
    {
        bit[i]=0;
        dio_out(ramp_output_ch,ETEST_OUTPUT_OFFSET);
        dio_in_data(&read_data1);
        dio_out(ramp_output_ch,ETEST_OUTPUT_OFFSET+(1<<i));
        dio_start_clock(ETEST_RAMP_WAIT);
        dio_wait_clock();
        dio_in_data(&read_data2);
        bit[i]=(read_data2-read_data1);
/*
        sprintf(string,"before %d after %d diff %d bit %d",
            read_data1,read_data2,diff,i+1);
        mprintf(string);
*/
    }

    results[0]='\0';
    for(i=0;i<12;i++)
    {
        sprintf(string,"Bit %d is off by %f %%\n",i,
            (float)((1<<(i+4))-bit[i])/(float)(1<<(i+4))*100);
        strcat(results,string);
    }
    MessageBox(hDlg,results,"Test Results",MB_OK);
    

    set_range(ramp_output_ch,old_range);
    dio_out(ramp_output_ch,old_data);
}
#endif