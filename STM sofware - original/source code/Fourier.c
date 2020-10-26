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
#include "fft.h"
#include "fht\fht.h"
#include "fht\ideal.h"

BOOL FAR PASCAL FourierDlg(HWND,unsigned,WPARAM,LPARAM);

HWND FourierWnd=NULL;
BOOL FourierOn=FALSE;
extern HWND ImgDlg;

extern int current_image;
extern datadef *gendata[];
extern char string[];
static int out_smart=0;

static int rev_size(int size);
static void mult_filter(datadef *this_data,float *filter);

int fourier_window= FOURIER_HANNING;
int fourier_filter= FOURIER_LOW;
float fourier_low_cutoff = 0.2;
float fourier_high_cutoff = 1.0;
int fourier_order = 10;

static void repaint_all(HWND);
static void enable_all(HWND, int);

BOOL FAR PASCAL FourierDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{
    int i;
    int start,end;

//    int id;
//    int delt,ddelt,mini,maxi;
//    double d,minid,maxid,deltd,ddeltd;
    
    switch(Message)
    {
        case WM_INITDIALOG:
            if (!gendata[current_image]->valid ||
                gendata[current_image]->type==DATATYPE_3D)
            {
                enable_all(hDlg,1);
            }
            else
            {
                enable_all(hDlg,0);
            }
            repaint_all(hDlg);
            break;
#ifdef OLD
    case WM_HSCROLL:
    case WM_VSCROLL:
        if (!out_smart)
        {
            out_smart=1;
            id = getscrollid();
            switch (id)
            {
                case VSCALE_MIN_SC:
                    d=(double) image_lowerv[current_image];
                    minid=min(gendata[current_image]->min_z,IN_MIN);
                    maxid=image_higherv[current_image];
                    deltd=(gendata[current_image]->max_z-
                        gendata[current_image]->min_z)/100;
                    ddeltd=(gendata[current_image]->max_z-
                        gendata[current_image]->min_z)/10;
                    break;
                case VSCALE_MAX_SC:
                    d=(double) image_higherv[current_image];
                    minid=image_lowerv[current_image];
                    maxid=max(gendata[current_image]->max_z,IN_MAX);
                    deltd=(gendata[current_image]->max_z-
                        gendata[current_image]->min_z)/100;
                    ddeltd=(gendata[current_image]->max_z-
                        gendata[current_image]->min_z)/10;
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
            i = min(max(i, mini), maxi);
//            d=floor(d+0.5);
            d = min(max(d, minid), maxid);
            switch (id)
            {
                case VSCALE_MIN_SC:
                    image_lowerv[current_image]=d;
                    sprintf(string,"%0.5f",in_dtov(image_lowerv[current_image]));
                    SetDlgItemText(hDlg, VSCALE_MIN, string);
                    SetScrollPos(GetDlgItem(hDlg, VSCALE_MIN_SC), 
                        SB_CTL,DTOI(image_lowerv[current_image],
                            min(gendata[current_image]->min_z,IN_MIN),
                            image_higherv[current_image]) , TRUE);
                    SendMessage(ImgDlg, WM_COMMAND, IM_NEW_VSCALE, 0);
                    break;
                    
                case VSCALE_MAX_SC:
                    image_higherv[current_image]=d;
                    sprintf(string,"%0.5f",in_dtov(image_higherv[current_image]));
                    SetDlgItemText(hDlg, VSCALE_MAX, string);
                    SetScrollPos(GetDlgItem(hDlg, VSCALE_MAX_SC), 
                        SB_CTL,DTOI(image_higherv[current_image],image_lowerv[current_image],
                            max(gendata[current_image]->max_z,IN_MAX)),TRUE);
                    SendMessage(ImgDlg, WM_COMMAND, IM_NEW_VSCALE, 0);
                    break;
            }
VSCALE_END_SCROLL:
            out_smart=0;
        }
        break;
#endif
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
            case FOURIER_NEW_IMAGE:
                if (!gendata[current_image]->valid ||
                    gendata[current_image]->type==DATATYPE_3D)
                {
                    enable_all(hDlg,1);
                }
                else
                {
                    enable_all(hDlg,0);
                    repaint_all(hDlg);
                }
                break;
            
            case FOURIER_LOW_CUTOFF:
                GetDlgItemText(hDlg,LOWORD(wParam),string,9);
                fourier_low_cutoff = atof(string);
                break;
            case FOURIER_HIGH_CUTOFF:
                GetDlgItemText(hDlg,LOWORD(wParam),string,9);
                fourier_high_cutoff = atof(string);
                break;
            case FOURIER_ORDER:
                GetDlgItemText(hDlg,LOWORD(wParam),string,9);
                fourier_order = atoi(string);
                break;
            case FOURIER_LOW:
            case FOURIER_HIGH:
            case FOURIER_BAND_PASS:
            case FOURIER_BAND_STOP:
                fourier_filter = LOWORD(wParam);
                break;
            case FOURIER_RECTANGLE:
            case FOURIER_TRIANGLE:
            case FOURIER_BARTLETT:
            case FOURIER_HANNING:
            case FOURIER_HAMMING:
                fourier_window=LOWORD(wParam);
                break;
            case FOURIER_FILTER:
                if (current_image!=3 && gendata[current_image]->valid &&
                    gendata[current_image]->type==DATATYPE_3D)
                {
                    copy_data(&gendata[3],&gendata[current_image]);
                    gendata[3]->equalized=0;
                    i=rev_size(gendata[3]->size);
                    if (i) 
                    {
                        float *filter;
                        window_type window;
                        
                        switch(fourier_window)
                        {
                            case FOURIER_TRIANGLE:
                                window=TRIANGLE;
                                break;
                            case FOURIER_RECTANGLE:
                                window=RECTANGLE;
                                break;
                            case FOURIER_BARTLETT:
                                window=BARTLETT;
                                break;
                            case FOURIER_HANNING:
                                window=HANNING;
                                break;
                            case FOURIER_HAMMING:
                                window=HAMMING;
                                break;
                        }
                        
                        fht2d(gendata[3]->ptr,gendata[3]->ptr,1,
                            gendata[3]->size);
                
                        filter = (float *) malloc(sizeof(float)*gendata[3]->size*
                            gendata[3]->size);
                        switch(fourier_filter)
                        {
                            case FOURIER_LOW:
                                lowpass(filter, fourier_low_cutoff, fourier_order, 
                                    window, gendata[3]->size);
                                break;
                            case FOURIER_HIGH:
                                highpass(filter, fourier_high_cutoff, &fourier_order,
                                    window, gendata[3]->size);
                                repaint_all(hDlg);
                                break;
                            case FOURIER_BAND_PASS:
                                bandpass(filter, fourier_low_cutoff, fourier_high_cutoff,
                                    fourier_order, window, gendata[3]->size);
                                break;
                            case FOURIER_BAND_STOP:
                                bandstop(filter, fourier_low_cutoff, fourier_high_cutoff,
                                    &fourier_order, window, gendata[3]->size);
                                repaint_all(hDlg);
                                break;
                        }
                        fht2d(filter,filter,0,gendata[3]->size);
                        mult_filter(gendata[3],filter);
                        free(filter);
                        fht2d(gendata[3]->ptr,gendata[3]->ptr,0,gendata[3]->size);
                    }
                        
                    
                    reset_image(ImgDlg,3);
                }
                break;
            case FOURIER_FFT:
                if (current_image!=3 && gendata[current_image]->valid &&
                    gendata[current_image]->type==DATATYPE_3D)
                {
                    copy_data(&gendata[3],&gendata[current_image]);
                    gendata[3]->equalized=0;
                    i=rev_size(gendata[3]->size);
                    
                    if (i) fft_2d(gendata[3]->ptr,i,1);
                    reset_image(ImgDlg,3);
                }
                break;
            case FOURIER_IFFT:
                if (current_image!=3 && gendata[current_image]->valid &&
                    gendata[current_image]->type==DATATYPE_3D)
                {
                    copy_data(&gendata[3],&gendata[current_image]);
                    gendata[3]->equalized=0;
                    i=rev_size(gendata[3]->size);
                    
                    if (i) fft_2d(gendata[3]->ptr,i,-1);
                    reset_image(ImgDlg,3);
                }
                break;
            case FOURIER_FHT:
                if (current_image!=3 && gendata[current_image]->valid &&
                    gendata[current_image]->type==DATATYPE_3D)
                {
                    copy_data(&gendata[3],&gendata[current_image]);
                    gendata[3]->equalized=0;
                    i=rev_size(gendata[3]->size);
                    
                    if (i) 
                        fht2d(gendata[3]->ptr,gendata[3]->ptr,1,
                            gendata[3]->size);
                    reset_image(ImgDlg,3);
                }
                break;
            case FOURIER_IFHT:
                if (current_image!=3 && gendata[current_image]->valid &&
                    gendata[current_image]->type==DATATYPE_3D)
                {
                    copy_data(&gendata[3],&gendata[current_image]);
                    gendata[3]->equalized=0;
                    i=rev_size(gendata[3]->size);
                    
                    if (i) 
                        fht2d(gendata[3]->ptr,gendata[3]->ptr,0,
                            gendata[3]->size);
                    reset_image(ImgDlg,3);
                }
                break;
            
            case ENTER:
                repaint_all(hDlg);
                break;
            case FOURIER_EXIT:
//                PostMessage(ImgDlg,WM_COMMAND,IM_VSCALE_EXIT,0);
          
                DestroyWindow(hDlg);
                FourierWnd = NULL;
                FourierOn = FALSE;
                break;
    }
    break;
  }
  return(FALSE);
}

static void enable_all(HWND hDlg,int status)
{
    out_smart=1;
    EnableWindow(GetDlgItem(hDlg, FOURIER_FFT), status);
    EnableWindow(GetDlgItem(hDlg, FOURIER_FHT), status);
    EnableWindow(GetDlgItem(hDlg, FOURIER_IFFT), status);
    EnableWindow(GetDlgItem(hDlg, FOURIER_IFHT), status);
    EnableWindow(GetDlgItem(hDlg, FOURIER_FILTER), status);
//    EnableWindow(GetDlgItem(hDlg, _), status);
    out_smart=0;
}    

static void repaint_all(HWND hDlg)
{
        out_smart=1;
        sprintf(string,"%0.2f",fourier_low_cutoff);
        SetDlgItemText(hDlg, FOURIER_LOW_CUTOFF, string);
        sprintf(string,"%0.2f",fourier_high_cutoff);
        SetDlgItemText(hDlg, FOURIER_HIGH_CUTOFF, string);
        sprintf(string,"%d",fourier_order);
        SetDlgItemText(hDlg, FOURIER_ORDER, string);
        CheckDlgButton(hDlg, FOURIER_LOW, 0);
        CheckDlgButton(hDlg, FOURIER_HIGH, 0);
        CheckDlgButton(hDlg, FOURIER_BAND_PASS, 0);
        CheckDlgButton(hDlg, FOURIER_BAND_STOP, 0);
        
        
        CheckDlgButton(hDlg, fourier_filter, 1);

                
        
        CheckDlgButton(hDlg, FOURIER_RECTANGLE, 0);
        CheckDlgButton(hDlg, FOURIER_TRIANGLE, 0);
        CheckDlgButton(hDlg, FOURIER_BARTLETT, 0);
        CheckDlgButton(hDlg, FOURIER_HANNING, 0);
        CheckDlgButton(hDlg, FOURIER_HAMMING, 0);
        
        CheckDlgButton(hDlg, fourier_window, 1);

        
        out_smart=0;
}

static int rev_size(int size)
{
    int i,j;
    
    for(i=1;i<12;i++)
    {
        j=1L << i;
        if ((j & size)==j)
            return i;
    }
    return 0;
}

static void mult_filter(datadef *this_data,float *filter)
{
    int i,j;
    
    for(i=0;i<this_data->size;i++) for(j=0;j<this_data->size;j++)
    {
        *(this_data->ptr+i*this_data->size+j) *= *(filter+i*this_data->size+j);
    }
}

