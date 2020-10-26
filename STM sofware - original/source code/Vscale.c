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

HWND VscaleWnd = NULL;
BOOL VscaleOn = FALSE;
extern HWND ImgDlg;


extern float image_lowerv[],image_higherv[],image_lowert[],image_highert[];
// ranges shown in graphs. "v" is vertical axis,"t" is horizontal axis

extern int current_image;
extern datadef *gendata[];
extern char string[];

static int out_smart = 0;

static void enable_all(HWND,int);
static void repaint_all(HWND);

BOOL FAR PASCAL VscaleDlg(HWND,unsigned,WPARAM,LPARAM);
// vertical scale dialog used to set max and min on vertical axis of 
// 2d data graphs (such as I vs t and I vs V)

BOOL FAR PASCAL VscaleDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{
    int i;
	unsigned int j;
    int id;
    int delt,ddelt,mini,maxi;
    float data_max = IN_MAX;
	float data_min = IN_MIN;
    float tempf,image_cut_factor;
    double d,minid,maxid,deltd,ddeltd;
    unsigned int start = 0;
	unsigned int end = 0;

    switch(Message)
    {
        case WM_INITDIALOG:
            out_smart = 1;
            SetScrollRange(GetDlgItem(hDlg,VSCALE_MIN_SC),SB_CTL,0,32767,TRUE);
            SetScrollRange(GetDlgItem(hDlg,VSCALE_MAX_SC),SB_CTL,0,32767,TRUE);
            out_smart = 0;
            if(!gendata[current_image]->valid || gendata[current_image]->type == DATATYPE_3D)
            {
                enable_all(hDlg,0);
            }
            else
            {
                enable_all(hDlg,1);
                repaint_all(hDlg);
            }
            break;
		case WM_HSCROLL:
		case WM_VSCROLL:
			if(!out_smart)
			{
				out_smart = 1;
				id = getscrollid();
				switch(id)
				{
					case VSCALE_MIN_SC:
						d = (double)image_lowerv[current_image];
						minid = min(gendata[current_image]->min_z,IN_MIN);
						maxid = image_higherv[current_image];
						deltd = (gendata[current_image]->max_z -
									gendata[current_image]->min_z) / 100;
						ddeltd = (gendata[current_image]->max_z -
									gendata[current_image]->min_z) / 10;
						break;
					case VSCALE_MAX_SC:
						d = (double)image_higherv[current_image];
						minid = image_lowerv[current_image];
						maxid = max(gendata[current_image]->max_z,IN_MAX);
						deltd = (gendata[current_image]->max_z -
									gendata[current_image]->min_z) / 100;
						ddeltd = (gendata[current_image]->max_z -
									gendata[current_image]->min_z) / 10;
						break;
				}

				switch (getscrollcode())
				{       
					case SB_ENDSCROLL:
						goto VSCALE_END_SCROLL;
						break;
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
				i = min(max(i,mini),maxi);
//            d=floor(d + 0.5);
				d = min(max(d,minid),maxid);

			    switch(id)
				{
					case VSCALE_MIN_SC:
						image_lowerv[current_image] = (float)d;
						sprintf(string,"%0.5f",in_dtov(image_lowerv[current_image]));
						SetDlgItemText(hDlg,VSCALE_MIN,string);
						SetScrollPos(GetDlgItem(hDlg,VSCALE_MIN_SC),
							SB_CTL,DTOI(image_lowerv[current_image],
                            min(gendata[current_image]->min_z,IN_MIN),
                            image_higherv[current_image]),TRUE);
						SendMessage(ImgDlg,WM_COMMAND,IM_NEW_VSCALE,0);
						break;
                    
					case VSCALE_MAX_SC:
						image_higherv[current_image] = (float)d;
						sprintf(string,"%0.5f",in_dtov(image_higherv[current_image]));
						SetDlgItemText(hDlg,VSCALE_MAX,string);
						SetScrollPos(GetDlgItem(hDlg,VSCALE_MAX_SC),
							SB_CTL,DTOI(image_higherv[current_image],image_lowerv[current_image],
                            max(gendata[current_image]->max_z,IN_MAX)),TRUE);
						SendMessage(ImgDlg,WM_COMMAND,IM_NEW_VSCALE,0);
						break;
				}

VSCALE_END_SCROLL:
				out_smart = 0;
			}
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)){
				case VSCALE_SET_CENTER:
					out_smart = 1;
					switch(gendata[current_image]->type)
					{
						case DATATYPE_2D:
							start = time_index(current_image,image_lowert[current_image]);
							end = time_index(current_image,image_highert[current_image]);
							data_max = gendata[current_image]->data2d[start];
							data_min = gendata[current_image]->data2d[start];
							for(j = start;j <= end;j++)
							{
								if(gendata[current_image]->data2d[j]<data_min) 
									data_min=gendata[current_image]->data2d[j];
								if(gendata[current_image]->data2d[j]>data_max) 
									data_max=gendata[current_image]->data2d[j];
							}
							break;
						case DATATYPE_GEN2D:
							switch(gendata[current_image]->type2d)
							{
								case TYPE2D_SPEC_I:
								case TYPE2D_SPEC_Z:
								case TYPE2D_SPEC_2:
								case TYPE2D_SPEC_3:
								case TYPE2D_MASSPEC:
                        
									if(image_lowert[current_image] > gendata[current_image]->min_x) 
									{
										start = (int)((image_lowert[current_image] - gendata[current_image]->min_x) /
											gendata[current_image]->step);
									}
									else
									{
										start = 0;
									}
									if(image_highert[current_image] < (gendata[current_image]->start + (gendata[current_image]->size - 1) * gendata[current_image]->step)) 
									{
										end = (int)((image_highert[current_image]-gendata[current_image]->start)/
											gendata[current_image]->step);
									}
									else 
									{
										end=gendata[current_image]->size;
									}
									break;
								case TYPE2D_CUT:
									image_cut_factor = gendata[current_image]->size / gendata[current_image]->max_x;
									if(image_lowert[current_image] > gendata[current_image]->min_x) 
									{
										start = (int)((image_lowert[current_image] - gendata[current_image]->min_x) *
											image_cut_factor);
									}
									else
									{
										start = 0;
									}
									if(image_highert[current_image] < gendata[current_image]->max_x)
									{
										end = (int)((image_highert[current_image] - gendata[current_image]->min_x) *
											image_cut_factor + 1);
										if(end > gendata[current_image]->size) end = gendata[current_image]->size;
									}
									else 
									{
										end = gendata[current_image]->size;
									}
									break;
							}
							data_max = gendata[current_image]->yf[start];
							data_min = gendata[current_image]->yf[start];
							for(j = start;j < end;j++)
							{
								if(gendata[current_image]->yf[j] < data_min) data_min = gendata[current_image]->yf[j];
								if(gendata[current_image]->yf[j] > data_max) data_max = gendata[current_image]->yf[j];
							}
							break;
					}
                        
					if(data_min >= IN_ZERO) data_min = IN_ZERO - (data_max - IN_ZERO);
					if(data_max <= IN_ZERO) data_max = IN_ZERO + (IN_ZERO - data_min);
					if(IN_ZERO - data_min > data_max - IN_ZERO)
					{
						data_max = IN_ZERO + (IN_ZERO - data_min);
					}
					else
					{
						data_min = IN_ZERO - (data_max - IN_ZERO);
					}
					image_lowerv[current_image] = data_min;
					image_higherv[current_image] = data_max;
					repaint_all(hDlg);
					SendMessage(ImgDlg,WM_COMMAND,IM_NEW_VSCALE,0);
					out_smart = 0;
            
					break;
				case VSCALE_SET_MAX:
					out_smart = 1;
					image_higherv[current_image] = gendata[current_image]->max_z;
					repaint_all(hDlg);
					SendMessage(ImgDlg,WM_COMMAND,IM_NEW_VSCALE,0);
					out_smart = 0;
					break;
				case VSCALE_SET_MIN:
					out_smart = 1;
					image_lowerv[current_image] = gendata[current_image]->min_z;
					repaint_all(hDlg);
					SendMessage(ImgDlg,WM_COMMAND,IM_NEW_VSCALE,0);
					out_smart = 0;
					break;
                
				case VSCALE_MIN:
					if(!out_smart)
					{
						out_smart = 1;
						GetDlgItemText(hDlg,VSCALE_MIN,string,9);
						tempf = (float)in_vtod(atof(string));

						tempf = min(max(tempf,
							min(gendata[current_image]->min_z,IN_MIN)),
							image_higherv[current_image]-(float)abs((int)(max(gendata[current_image]->max_z,
							IN_MAX) - min(gendata[current_image]->min_z,IN_MIN))) / 100);
						if(tempf!=image_lowerv[current_image])
						{
							image_lowerv[current_image] = tempf; 
							SetScrollPos(GetDlgItem(hDlg,VSCALE_MIN_SC),
								SB_CTL,DTOI(image_lowerv[current_image],
									min(gendata[current_image]->min_z,IN_MIN),
									image_higherv[current_image]),TRUE);
							SendMessage(ImgDlg,WM_COMMAND,IM_NEW_VSCALE,0);
						}
						out_smart = 0;
					}
					break;
				case VSCALE_MAX:
					if(!out_smart)
					{
						out_smart = 1;
						GetDlgItemText(hDlg,VSCALE_MAX,string,9);
						tempf = (float)in_vtod(atof(string));
						tempf = min(max(tempf,
							image_lowerv[current_image] + (float)abs((int)(max(gendata[current_image]->max_z,IN_MAX) -
							min(gendata[current_image]->min_z,IN_MIN))) / 100),
							max(gendata[current_image]->max_z,IN_MAX));
						if(tempf != image_higherv[current_image])
						{
							image_higherv[current_image] = tempf;            
							SetScrollPos(GetDlgItem(hDlg,VSCALE_MAX_SC),
								SB_CTL,DTOI(image_higherv[current_image],image_lowerv[current_image],
								max(gendata[current_image]->max_z,IN_MAX)),TRUE);
							SendMessage(ImgDlg,WM_COMMAND,IM_NEW_VSCALE,0);
						}
						out_smart = 0;
					}
					break;
/*
				case SEL_INVERT:
					if(cur_sel_region->type == SEL_SQUARE ||
						cur_sel_region->type == SEL_CIRCLE)
					{
						SendMessage(ImgDlg,WM_COMMAND,IM_SEL_REDRAW,0);
                    
						cur_sel_region->flags ^= SEL_INVERTED;
						SendMessage(ImgDlg,WM_COMMAND,IM_SEL_REDRAW,0);
					}
					break;
*/
				case VSCALE_NEW_IMAGE:
					if(!gendata[current_image]->valid ||
						gendata[current_image]->type == DATATYPE_3D)
					{
						enable_all(hDlg,0);
					}
					else
					{
						enable_all(hDlg,1);
						repaint_all(hDlg);
					}
				case ENTER:
					repaint_all(hDlg);
					break;
				case VSCALE_EXIT:
//					 PostMessage(ImgDlg,WM_COMMAND,IM_VSCALE_EXIT,0);
          
					DestroyWindow(hDlg);
					VscaleWnd = NULL;
					VscaleOn = FALSE;
					break;
			}
		break;
	}
	return(FALSE);
}

static void enable_all(HWND hDlg,int status)
{
    out_smart = 1;
    EnableWindow(GetDlgItem(hDlg,VSCALE_MAX),status);
    EnableWindow(GetDlgItem(hDlg,VSCALE_MIN),status);
    EnableWindow(GetDlgItem(hDlg,VSCALE_MAX_SC),status);
    EnableWindow(GetDlgItem(hDlg,VSCALE_MIN_SC),status);
    EnableWindow(GetDlgItem(hDlg,VSCALE_SET_MAX),status);
    EnableWindow(GetDlgItem(hDlg,VSCALE_SET_CENTER),status);
    EnableWindow(GetDlgItem(hDlg,VSCALE_SET_MIN),status);
    out_smart = 0;
}    

static void repaint_all(HWND hDlg)
{
        out_smart = 1;
        sprintf(string,"%0.5f",in_dtov(image_lowerv[current_image]));
        SetDlgItemText(hDlg,VSCALE_MIN,string);
        SetScrollPos(GetDlgItem(hDlg,VSCALE_MIN_SC),
            SB_CTL,DTOI(image_lowerv[current_image],
                min(gendata[current_image]->min_z,IN_MIN),
                image_higherv[current_image]),TRUE);
        sprintf(string,"%0.5f",in_dtov(image_higherv[current_image]));
        SetDlgItemText(hDlg,VSCALE_MAX,string);
        SetScrollPos(GetDlgItem(hDlg,VSCALE_MAX_SC),
            SB_CTL,DTOI(image_higherv[current_image],image_lowerv[current_image],
                max(gendata[current_image]->max_z,IN_MAX)),TRUE);
        out_smart = 0;
}
