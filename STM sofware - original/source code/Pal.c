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

BOOL FAR PASCAL PalDlg(HWND,unsigned,WPARAM,LPARAM);

void histogram();

HWND PalWnd=NULL;
BOOL PalOn=FALSE;
extern HWND ImgDlg;
extern HWND LogpalWnd;
extern BOOL LogpalOn;

/*
unsigned char   dacbox[LOGPALSIZE + 1][3];
*/
//unsigned char   *dacbox;
extern unsigned char   *image_dacbox;
GENPAL *glob_genpal;
extern char *current_file_pal;
extern char *pal_image;
extern datadef **glob_data;
extern datadef *gendata[];
extern int current_image;
extern BITMAPINFO *bitmapinfo;
extern LOGPALETTE              *pLogPal;
extern HPALETTE                hPal;
extern HANDLE hInst;
int ColorLUT[256];
extern char     string[];
extern int zoomed_in;
int histogram0[256];
int histogram1[DATA_HIST_MAX];
extern int im_src_x[],im_src_y[],im_screenlines[];
extern unsigned char *im_bitmaps[];
static    double gamma=1;

extern int scan_current_palette;

BOOL FAR PASCAL PalDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{
#ifdef DEBUG
            FILE *fp;
#endif
    float datasize;
    int anim_delay=10;
    int i,j;
    int max_hist;
//    int hist_pos;
    
  switch(Message) {
    case WM_INITDIALOG:
      sprintf(string,"%.2lf",gamma);
      SetDlgItemText(hDlg,GAMMA_VALUE,string);
      sprintf(string,"%d",anim_delay);
      SetDlgItemText(hDlg,ANIMATE_DELAY,string);
//      zoomed_in=0;
      break;
    case WM_COMMAND:
      switch (LOWORD(wParam)) {
    case I_UPDATE:
      break;
    case LOAD_PAL:
        if (gendata[current_image]->valid && 
            gendata[current_image]->type==DATATYPE_3D)
        {
            glob_data=&(gendata[current_image]);
            file_open(hDlg,hInst,FTYPE_PAL,0,current_file_pal);
            if (gendata[current_image]->pal.type==PALTYPE_MAPPED)
            {
                memcpy(image_dacbox,gendata[current_image]->pal.dacbox,
                    SIZEOFMPAL);
            }
            calc_bitmap(current_image);
            SendMessage(ImgDlg,WM_COMMAND,
                IM_REDRAW_IMAGE,current_image);
            if (LogpalOn)
                SendMessage(LogpalWnd,WM_COMMAND,
                    LOGPAL_NEW_IMAGE,current_image);
            
//            repaint_image(ImgDlg,current_image,(RECT *)NULL);
            
        }
        break;
    case SAVE_PAL:
        if (gendata[current_image]->valid && 
            gendata[current_image]->type==DATATYPE_3D)
        {
            glob_data=&(gendata[current_image]);
            file_save_as(hDlg,hInst,FTYPE_PAL);
        }
        break;    
    case GAMMA_VALUE:
        break;
    case GAMMA:
        GetDlgItemText(hDlg,GAMMA_VALUE,string,9);
        gamma=(double) atof(string);
        if (gamma>0)
        {
            MakeColorLUT(ColorLUT,gamma);
            if (gendata[current_image]->valid && 
                gendata[current_image]->type==DATATYPE_3D)
            {
              apply_gamma(gendata[current_image]->pal.dacbox);
              repaint_image(ImgDlg,current_image,(RECT *)NULL);
            }
        }
        break;
    case CLIP:
      if (gendata[current_image]->valid && 
        gendata[current_image]->type==DATATYPE_3D)
      {
          datasize=gendata[current_image]->clip_max-gendata[current_image]->clip_min+1;
          gendata[current_image]->clip_min+=datasize/10.0;
          gendata[current_image]->clip_max-=datasize/10.0;
          if (gendata[current_image]->clip_min>gendata[current_image]->clip_max)
              gendata[current_image]->clip_min=gendata[current_image]->clip_max;
          calc_bitmap(current_image);
          repaint_image(ImgDlg,current_image,(RECT *)NULL);
      }
      break;
    case UNCLIP:
      gendata[current_image]->clip_min=gendata[current_image]->min_z;
      gendata[current_image]->clip_max=gendata[current_image]->max_z;
      calc_bitmap(current_image);
      repaint_image(ImgDlg,current_image,(RECT *)NULL);
      break;
    case ANIMATE:
      GetDlgItemText(hDlg,ANIMATE_DELAY,string,9);
      anim_delay=atoi(string);
/*
      animate_pal(hDlg,anim_delay);
*/
      break;                                                     
    case EQUALIZE:
      if (gendata[current_image]->valid && 
        gendata[current_image]->type==DATATYPE_3D)
      {
          equalize(gendata[current_image],gendata[current_image]->min_z,
            gendata[current_image]->max_z,gendata[current_image]->fhist);
          gendata[current_image]->equalized=1;
          calc_bitmap(current_image);      
          repaint_image(ImgDlg,current_image,NULL);
      }
      break;
    case UNEQUALIZE:
      if (gendata[current_image]->valid && 
        gendata[current_image]->type==DATATYPE_3D)
      {
          unequalize(gendata[current_image]);
          calc_bitmap(current_image);
          repaint_image(ImgDlg,current_image,NULL);
      }
      break;
    case HISTOGRAM:
      if (gendata[current_image]->valid && 
        gendata[current_image]->type==DATATYPE_3D && current_image!=3)
      {
          histogram();
          copy_data(&gendata[3],&gendata[current_image]);
          max_hist=0;
          for(i=0;i<256;i++) max_hist=max(max_hist,histogram0[i]);
          gendata[3]->size=256;
          gendata[3]->min_z=gendata[3]->clip_min=0;
          gendata[3]->max_z=gendata[3]->clip_max=255;
          for(i=0;i<256;i++)
            for(j=0;j<256;j++)
            {
                if (j<histogram0[i]*256/max_hist) *(gendata[3]->ptr+i+j*256)=128;
                else *(gendata[3]->ptr+i+j*256)=0;
            }
          gendata[3]->valid=1;
          unequalize(gendata[3]);
          reset_image(ImgDlg,3);
/*
          im_src_x[3]=im_src_y[3]=0;
          im_screenlines[3]=gendata[3]->size;
          unequalize(gendata[3]);
          calc_bitmap(3);
          repaint_image(ImgDlg,3,NULL);
*/
      }
      break;
    case CLIP_TO_HIST:
      if (gendata[current_image]->valid && 
        gendata[current_image]->type==DATATYPE_3D)
      {
          max_hist=0;
          for(i=0;i<256;i++) max_hist=max(max_hist,histogram0[i]);
          i=0;
          while(histogram0[i]*5<max_hist) i++;
          gendata[current_image]->clip_min=i*(gendata[current_image]->max_z-
                   gendata[current_image]->min_z+1)/256+gendata[current_image]->min_z;
          i=255;
          while(histogram0[i]*5<max_hist) i--;
          gendata[current_image]->clip_max=i*(gendata[current_image]->max_z-
                   gendata[current_image]->min_z+1)/256+gendata[current_image]->min_z;
          calc_bitmap(current_image);
          repaint_image(ImgDlg,current_image,NULL);
      }
      break;
    case RANDPAL:
      
      set_random_pal(image_dacbox);
      if (gendata[current_image]->valid && 
        gendata[current_image]->type==DATATYPE_3D)
      {
        gendata[current_image]->pal.type=PALTYPE_MAPPED;
        memcpy(gendata[current_image]->pal.dacbox,image_dacbox,SIZEOFMPAL);
        repaint_image(ImgDlg,current_image,NULL);
      }
      break;
    case RANDOM_1:
      set_random_one_pal(image_dacbox);
      if (gendata[current_image]->valid && 
        gendata[current_image]->type==DATATYPE_3D)
      {
        gendata[current_image]->pal.type=PALTYPE_MAPPED;
        memcpy(gendata[current_image]->pal.dacbox,image_dacbox,SIZEOFMPAL);
        repaint_image(ImgDlg,current_image,NULL);
      }
      break;
    case PAL_EXIT:
      DestroyWindow(hDlg);
      PalWnd = NULL;
      PalOn = FALSE;
      break;
      }
    break;
  }
  return(FALSE);
}


void animate_pal(HWND hDlg,int del)
{
    MSG Message;
    int done=0;
    while(!done)
    {        
        PeekMessage(&Message, hDlg, NULL, NULL, PM_REMOVE);
        done=(Message.message==WM_CHAR || Message.message==WM_KEYDOWN ||
            Message.message==WM_KEYUP || Message.message==WM_LBUTTONDOWN);
        
        dio_start_clock(del*1000);
        dio_wait_clock();
//        spindac(1,1,dacbox);
    }
}
void apply_gamma(unsigned char *dacbox)
{
   int i;
   
    for(i=0;i<LOGPALUSED;i++)
    {
        dacbox[i*3+0]=ColorLUT[dacbox[i*3+0]];
        dacbox[i*3+1]=ColorLUT[dacbox[i*3+1]];
        dacbox[i*3+2]=ColorLUT[dacbox[i*3+2]];
    }
    spindac(0,1,dacbox);
}

void MakeColorLUT (
           int * pLut, /* array to hold LUT */
           double gamma /* exponent of stretch */
)
{
  int index;            /* LUT index */
  double scale;         /* scale factor (K) */
  double lutVal;        /* LUT value */

  pLut[0]=0;
  scale = 255.0 / pow (255.0, gamma);
  for (index = 1; index < 256; index++)
    {
        lutVal = scale * pow (index, gamma);
        pLut[index]  = (int) (lutVal + 0.5);
    }
}



int            spindac(int direction, int step,unsigned char *dacbox)
{
    int             i,
                    j,
                    k,result;
    HDC             hDC;
    unsigned char temp;

    for (k = 0; k < step; k++)
    {
        if (direction > 0)
        {
            for (j = 0; j < 3; j++)
            {
                temp=dacbox[(LOGPALUSED-1)*3+j];
                for (i = (LOGPALUSED - 2); i >= 0; i--)
                    dacbox[(i + 1)*3+j] = dacbox[i*3+j];
                dacbox[0*3+j] = temp;
            }
        }
        if (direction < 0)
        {
// BROKEN! copy direction>0
            for (j = 0; j < 3; j++)
            {
                dacbox[LOGPALSIZE*3+j] = dacbox[1*3+j];
                for (i = 1; i < LOGPALSIZE; i++)
                    dacbox[i*3+j] = dacbox[i + 1*3+j];
            }
        }
    }

    /* fill in intensities for all palette entry colors */
    for (i = 0; i < LOGPALUSED; i++)
    {
        pLogPal->palPalEntry[i].peRed = ((BYTE) dacbox[i*3+0]);
        pLogPal->palPalEntry[i].peGreen = ((BYTE) dacbox[i*3+1]);
        pLogPal->palPalEntry[i].peBlue = ((BYTE) dacbox[i*3+2]);
        pLogPal->palPalEntry[i].peFlags = PC_RESERVED;
    }
    hDC = GetDC(GetFocus());
    SetPaletteEntries(hPal, 0, pLogPal->palNumEntries, pLogPal->palPalEntry);
    SelectPalette(hDC, hPal, 0);
    result=RealizePalette(hDC);
    ReleaseDC(GetFocus(), hDC);
    return result;
}
#define REDMIN 100
#define REDMAX 255
#define GREENMIN 50
#define GREENMAX 255
#define BLUEMIN 70
#define BLUEMAX 255
#define MIXLENGTH 100



void ramp(int colindex,int mincol,int maxcol,int start,int end,
    unsigned char *dacbox)
{
    int i;
    
 
    for(i=start;i<end;i++)
    {
        dacbox[i*3+colindex]=mincol+(i-start)*(maxcol-mincol)/(end-start+1);
    }
}

void set_Plasma_palette(unsigned char *dacbox)
{
    static unsigned char Red[] = {255,0,0};
    static unsigned char Green[] = {0,255,0};
    static unsigned char Blue[] = {0,0,255};
    static unsigned char Orange[] = {255,128,0};
    static unsigned char Yellow[] = {255,255,0};
    static unsigned char Violet[] = {255,0,255};
	static unsigned char Black[] = {0,0,0};
	static unsigned char Dark_Grey[] = {32,32,32};
	static unsigned char Medium_Grey[] = {128,128,128};
	static unsigned char White[] = {255,255,255};

//    int i;

    dacbox[0*3+0]=dacbox[0*3+1]=dacbox[0*3+2]=0; /* black background */

///*
	// NORMAL (Blue/Violet = High & Red/Orange = Low)

	if(scan_current_palette==0){
    // ramp(0,REDMAX,Orange[0],1,LOGPALUSED/5,dacbox);
	ramp(0,Red[0],Orange[0],0,LOGPALUSED/5,dacbox);		// X. Cao, 2004-05-15
    ramp(0,Orange[0],Yellow[0],LOGPALUSED/5,LOGPALUSED*2/5,dacbox);
    ramp(0,Yellow[0],Green[0],LOGPALUSED*2/5,LOGPALUSED*3/5,dacbox);
    ramp(0,Green[0],Blue[0],LOGPALUSED*3/5,LOGPALUSED*4/5,dacbox);
    ramp(0,Blue[0],Violet[0],LOGPALUSED*4/5,LOGPALUSED,dacbox);

    // ramp(1,Red[1],Orange[1],1,LOGPALUSED/5,dacbox);
	ramp(1,Red[1],Orange[1],0,LOGPALUSED/5,dacbox);		// X. Cao, 2004-05-15
    ramp(1,Orange[1],Yellow[1],LOGPALUSED/5,LOGPALUSED*2/5,dacbox);
    ramp(1,Yellow[1],Green[1],LOGPALUSED*2/5,LOGPALUSED*3/5,dacbox);
    ramp(1,Green[1],Blue[1],LOGPALUSED*3/5,LOGPALUSED*4/5,dacbox);
    ramp(1,Blue[1],Violet[1],LOGPALUSED*4/5,LOGPALUSED,dacbox);

    // ramp(2,Red[2],Orange[2],1,LOGPALUSED/5,dacbox);
	ramp(2,Red[2],Orange[2],0,LOGPALUSED/5,dacbox);		// X. Cao, 2004-05-15
    ramp(2,Orange[2],Yellow[2],LOGPALUSED/5,LOGPALUSED*2/5,dacbox);
    ramp(2,Yellow[2],Green[2],LOGPALUSED*2/5,LOGPALUSED*3/5,dacbox);
    ramp(2,Green[2],Blue[2],LOGPALUSED*3/5,LOGPALUSED*4/5,dacbox);
    ramp(2,Blue[2],Violet[2],LOGPALUSED*4/5,LOGPALUSED,dacbox);
	}
//*/

///*
	// mw_greysA
	if(scan_current_palette==2){
	ramp(0,Dark_Grey[0],Medium_Grey[0],0,LOGPALUSED*13/20,dacbox);
    ramp(0,Medium_Grey[0],White[0],LOGPALUSED*13/20,LOGPALUSED,dacbox);

  	ramp(1,Dark_Grey[1],Medium_Grey[1],0,LOGPALUSED*13/20,dacbox);
    ramp(1,Medium_Grey[1],White[1],LOGPALUSED*13/20,LOGPALUSED,dacbox);

  	ramp(2,Dark_Grey[2],Medium_Grey[2],0,LOGPALUSED*13/20,dacbox);
    ramp(2,Medium_Grey[2],White[2],LOGPALUSED*13/20,LOGPALUSED,dacbox);
	}
//*/

/*
	// Fire (Black = Low & Red->Yellow = High)
	ramp(0,Black[0],Red[0],0,LOGPALUSED*1/5,dacbox);
    ramp(0,Red[0],Orange[0],LOGPALUSED*1/5,LOGPALUSED*2/5,dacbox);
    ramp(0,Orange[0],Yellow[0],LOGPALUSED*2/5,LOGPALUSED*3/5,dacbox);
    ramp(0,Yellow[0],White[0],LOGPALUSED*3/5,LOGPALUSED,dacbox);

	ramp(1,Black[1],Red[1],0,LOGPALUSED*1/5,dacbox);
    ramp(1,Red[1],Orange[1],LOGPALUSED*1/5,LOGPALUSED*2/5,dacbox);
    ramp(1,Orange[1],Yellow[1],LOGPALUSED*2/5,LOGPALUSED*3/5,dacbox);
    ramp(1,Yellow[1],White[1],LOGPALUSED*3/5,LOGPALUSED,dacbox);

	ramp(2,Black[2],Red[2],0,LOGPALUSED*1/5,dacbox);
    ramp(2,Red[2],Orange[2],LOGPALUSED*1/5,LOGPALUSED*2/5,dacbox);
    ramp(2,Orange[2],Yellow[2],LOGPALUSED*2/5,LOGPALUSED*3/5,dacbox);
    ramp(2,Yellow[2],White[2],LOGPALUSED*3/5,LOGPALUSED,dacbox);
*/

///*

	// Flipped (Blue/Violet = Low & Red/Orange = High)
	if(scan_current_palette==1){
	ramp(0,Violet[0],Blue[0],0,LOGPALUSED/5,dacbox);
    ramp(0,Blue[0],Green[0],LOGPALUSED/5,LOGPALUSED*2/5,dacbox);
    ramp(0,Green[0],Yellow[0],LOGPALUSED*2/5,LOGPALUSED*3/5,dacbox);
    ramp(0,Yellow[0],Orange[0],LOGPALUSED*3/5,LOGPALUSED*4/5,dacbox);
    ramp(0,Orange[0],Red[0],LOGPALUSED*4/5,LOGPALUSED,dacbox);

  	ramp(1,Violet[1],Blue[1],0,LOGPALUSED/5,dacbox);
    ramp(1,Blue[1],Green[1],LOGPALUSED/5,LOGPALUSED*2/5,dacbox);
    ramp(1,Green[1],Yellow[1],LOGPALUSED*2/5,LOGPALUSED*3/5,dacbox);
    ramp(1,Yellow[1],Orange[1],LOGPALUSED*3/5,LOGPALUSED*4/5,dacbox);
    ramp(1,Orange[1],Red[1],LOGPALUSED*4/5,LOGPALUSED,dacbox);

  	ramp(2,Violet[2],Blue[2],0,LOGPALUSED/5,dacbox);
    ramp(2,Blue[2],Green[2],LOGPALUSED/5,LOGPALUSED*2/5,dacbox);
    ramp(2,Green[2],Yellow[2],LOGPALUSED*2/5,LOGPALUSED*3/5,dacbox);
    ramp(2,Yellow[2],Orange[2],LOGPALUSED*3/5,LOGPALUSED*4/5,dacbox);
    ramp(2,Orange[2],Red[2],LOGPALUSED*4/5,LOGPALUSED,dacbox);
	}
//*/


/*
    ramp(0,50,REDMAX,1,LOGPALUSED);
    ramp(1,50,GREENMAX,1,LOGPALUSED);
    ramp(2,50,BLUEMAX,1,LOGPALUSED);
*/
/*
    ramp(0,REDMAX,REDMIN,1,(LOGPALUSED-MIXLENGTH)/3+MIXLENGTH/2);
    ramp(1,GREENMIN,GREENMAX,(LOGPALUSED-MIXLENGTH)/3,(LOGPALUSED-MIXLENGTH)/2+MIXLENGTH/2);
    ramp(1,GREENMAX,GREENMIN+(GREENMAX-GREENMIN)/5,(LOGPALUSED-MIXLENGTH)/2+MIXLENGTH/2,(LOGPALUSED-MIXLENGTH)/2+MIXLENGTH);
    ramp(2,BLUEMIN,BLUEMAX,(LOGPALUSED-MIXLENGTH)/2+MIXLENGTH/2,LOGPALUSED);
*/
/*
    ramp(0,REDMAX,REDMIN,1,LOGPALUSED/3);
    ramp(1,GREENMIN,GREENMAX,LOGPALUSED/3,2*LOGPALUSED/3);
    ramp(2,BLUEMAX,BLUEMIN,LOGPALUSED*2/3,LOGPALUSED);
*/
    
/*
    for(i=1;i<(LOGPALUSED-MIXLENGTH)/3;i++)
    {
        dacbox[i*3+0]=REDMAX-(i-1)*(REDMAX-REDMIN)*4*3/5/(LOGPALUSED-MIXLENGTH);
        dacbox[i*3+1]=0;
        dacbox[i*3+2]=0;
    }
    for(i=0;i<MIXLENGTH/2;i++)
    {
        dacbox[(i+(LOGPALUSED-MIXLENGTH)/3))*3+0]=REDMAX-(REDMAX-REDMIN)*4/5
            -i*
        dacbox[(i+(LOGPALUSED-MIXLENGTH)/3))*3+1]=GREENMIN+i*(GREENMAX-GREENMIN)*2/5/MIXLENGTH;
        dacbox[(i+(LOGPALUSED-MIXLENGTH)/3))*3+2]=0;
    }
    for(i=1;i<LOGPALUSED/3;i++)
    {
        dacbox[i*3+0]=REDMIN+(i-1)*(REDMAX-REDMIN)*3/LOGPALUSED;
        dacbox[i*3+1]=0;
        dacbox[i*3+2]=0;
    }
    for(i=LOGPALUSED/3;i<2*LOGPALUSED/3;i++)
    {
        dacbox[i*3+0]=0;
        dacbox[i*3+1]=GREENMIN+(i-LOGPALUSED/3)*(GREENMAX-GREENMIN)*3/LOGPALUSED;
        dacbox[i*3+2]=0;
    }
    for(i=2*LOGPALUSED/3;i<LOGPALUSED;i++)
    {
        dacbox[i*3+0]=0;
        dacbox[i*3+1]=0;
        dacbox[i*3+2]=BLUEMIN+(i-2*LOGPALUSED/3)*(BLUEMAX-BLUEMIN)*3/LOGPALUSED;
    }
*/
    spindac(0,1,dacbox);
}

void get_rand_color(unsigned char *color)
{
    int i;
    
    for(i=0;i<3;i++) color[i]=rand()*256/(RAND_MAX+1);
}
void get_rand_high_color(unsigned char *color)
{
    int i;
    
    for(i=0;i<3;i++) color[i]=255-rand()*70/(RAND_MAX+1);
}

void set_random_pal(unsigned char *dacbox)
{
    unsigned char color1[3];
    unsigned char color2[3];
    int num_col;
    int i,j;
    int size;
    
    srand(clock() % RAND_MAX);
    num_col=rand()*RAND_PAL_MAX/RAND_MAX+1;
    
    get_rand_color(&(color1[0]));
    size=(LOGPALUSED-1)/num_col;
    for(i=0;i<num_col;i++)
    {
        get_rand_color(&(color2[0]));
        for(j=0;j<3;j++)
        {
            ramp(j,color1[j],color2[j],i*size+1,(i+1)*size+1,dacbox);
            color1[j]=color2[j];
        }
    }
    spindac(0,1,dacbox);
}

void set_random_one_pal(unsigned char *dacbox)
{
    unsigned char color1[3];
    unsigned char color2[3];
    int num_col;
    int i,j;
    int size;
    
    srand(clock() % RAND_MAX);
    num_col=2;
    
    color1[0]=0;
    color1[1]=0;
    color1[2]=0;
    size=(LOGPALUSED-1)/num_col;
    for(i=0;i<1;i++)
    {
        get_rand_color(&(color2[0]));
        for(j=0;j<3;j++)
        {
            ramp(j,color2[j]/2,color2[j],i*size+1,(i+1)*size+1,dacbox);
            color1[j]=color2[j];
        }
    }
#ifdef BLAH
    for(j=0;j<3;j++)
    {
/*
        color2[j]*=2;
        if (color2[j]>255) color2[j]=255;
*/
        color2[j]=255;
    }
#endif
    get_rand_high_color(&(color2[0]));
    for(j=0;j<3;j++)
    {                   
        ramp(j,color1[j],color2[j],i*size+1,(i+1)*size+1,dacbox);
//        color1[j]=color2[j];
    }
    spindac(0,1,dacbox);
}

#ifdef OLD      
set_Plasma_palette0()
{



    static unsigned char Red[] = {
    255, 0, 0};
    static unsigned char Green[] = {
    0, 255, 0};
    static unsigned char Blue[] = {
    0, 0, 255};
    static unsigned char Orange[] = {
    255, 128, 0};
    static unsigned char Yellow[] = {
    255, 255, 0};
    static unsigned char Violet[] = {
    255, 0, 255};
    int             i;
#define COLRANGE 78

/*
    dacbox[0*3+0] = 0;
    dacbox[0*3+1] = 0;
    dacbox[0*3+2] = 0;
*/
    for (i =0; i <= COLRANGE; i++)
    {
/*
      dacbox[i*3+0]   = i+Red[0];
      dacbox[i*3+1] = 0;
      dacbox[i*3+2]  = 0;

      dacbox[(i+85)*3+0]   = 0;
      dacbox[(i+85)*3+1] = Green[0]+i;
      dacbox[(i+85)*3+2]  = 0;

      dacbox[(i+170)*3+0]   = 0;
      dacbox[(i+170)*3+1] = 0;
      dacbox[(i+170)*3+2]  = Blue[0]+i;

*/
        dacbox[i*3+0] = (i * Green[0] + (COLRANGE + 1 - i) * Blue[0]) / COLRANGE;
        dacbox[i*3+1] = (i * Green[1] + (COLRANGE + 1 - i) * Blue[1]) / COLRANGE;
        dacbox[i*3+2] = (i * Green[2] + (COLRANGE + 1 - i) * Blue[2]) / COLRANGE;

        dacbox[(i + COLRANGE)*3+0] = (i * Red[0] + (COLRANGE + 1 - i) * Green[0]) / COLRANGE;
        dacbox[(i + COLRANGE)*3+1] = (i * Red[1] + (COLRANGE + 1 - i) * Green[1]) / COLRANGE;
        dacbox[(i + COLRANGE)*3+2] = (i * Red[2] + (COLRANGE + 1 - i) * Green[2]) / COLRANGE;

        dacbox[(i + COLRANGE * 2)*3+0] = (i * Blue[0] + (COLRANGE + 1 - i) * Red[0]) / COLRANGE;
        dacbox[(i + COLRANGE * 2)*3+1] = (i * Blue[1] + (COLRANGE + 1 - i) * Red[1]) / COLRANGE;
        dacbox[(i + COLRANGE * 2)*3+2] = (i * Blue[2] + (COLRANGE + 1 - i) * Red[2]) / COLRANGE;

/*
      dacbox[i*3+0]   = (i*Orange[0]   + ((COLRANGE+1)-i)*Red[0])/COLRANGE*4;
      dacbox[i*3+1] = (i*Orange[1] + ((COLRANGE+1)-i)*Red[1])/COLRANGE*4;
      dacbox[i*3+2]  = (i*Orange[2]  + ((COLRANGE+1)-i)*Red[2])/COLRANGE*4;

      dacbox[(i+COLRANGE)*3+0]   = (i*Yellow[0]   + ((COLRANGE+1)-i)*Orange[0])/COLRANGE*4;
      dacbox[(i+COLRANGE)*3+1] = (i*Yellow[1] + ((COLRANGE+1)-i)*Orange[1])/COLRANGE*4;
      dacbox[(i+COLRANGE)*3+2]  = (i*Yellow[2]  + ((COLRANGE+1)-i)*Orange[2])/COLRANGE*4;

      dacbox[(i+COLRANGE*2)*3+0]   = (i*Green[0]   + ((COLRANGE+1)-i)*Yellow[0])/COLRANGE*4;
      dacbox[(i+COLRANGE*2)*3+1] = (i*Green[1] + ((COLRANGE+1)-i)*Yellow[1])/COLRANGE*4;
      dacbox[(i+COLRANGE*2)*3+2]  = (i*Green[2]  + ((COLRANGE+1)-i)*Yellow[2])/COLRANGE*4;

      dacbox[(i+COLRANGE*3)*3+0]   = (i*Blue[0]   + ((COLRANGE+1)-i)*Blue[0])/COLRANGE*4;
      dacbox[(i+COLRANGE*3)*3+1] = (i*Blue[1] + ((COLRANGE+1)-i)*Blue[1])/COLRANGE*4;
      dacbox[(i+COLRANGE*3)*3+2]  = (i*Blue[2]  + ((COLRANGE+1)-i)*Blue[2])/COLRANGE*4;

      dacbox[(i+COLRANGE*4)*3+0]   = (i*Violet[0]   + ((COLRANGE+1)-i)*Blue[0])/COLRANGE*4;
      dacbox[(i+COLRANGE*4)*3+1] = (i*Violet[1] + ((COLRANGE+1)-i)*Blue[1])/COLRANGE*4;
      dacbox[(i+COLRANGE*4)*3+2]  = (i*Violet[2]  + ((COLRANGE+1)-i)*Blue[2])/COLRANGE*4;

      dacbox[(i+COLRANGE*5)*3+0]   = (i*Red[0]   + ((COLRANGE+1)-i)*Violet[0])/COLRANGE*4;
      dacbox[(i+COLRANGE*5)*3+1] = (i*Red[1] + ((COLRANGE+1)-i)*Violet[1])/COLRANGE*4;
      dacbox[(i+COLRANGE*5)*3+2]  = (i*Red[2]  + ((COLRANGE+1)-i)*Violet[2])/COLRANGE*4;
*/
    }
    spindac(0, 1,dacbox);
}
#endif

void histogram()
{
    int size,i,j;
    
      size=gendata[current_image]->size;
      for(i=0;i<256;i++) histogram0[i]=0;
      for(i=0;i<size;i++)
        for(j=0;j<size;j++)
        {
            histogram0[(int)floor((*(gendata[current_image]->ptr+i*size+j)-
               gendata[current_image]->min_z)*256/(gendata[current_image]->max_z-
               gendata[current_image]->min_z+1))]++;
        }
}
void equalize(datadef *this_data,float min_z,float max_z,float *fhist)
{
    int size,i,j,k;
    float *sortptr;
//    int done;
    int unique,floats_in_bin,first_bin,filler_bins,filler_float_ratio;
    float filler;
    int start;
        
//        this_data->equalized=1;
        size=this_data->size*this_data->size;
        sortptr=(float *) malloc(sizeof(float)*size);
        memcpy(sortptr,this_data->ptr,sizeof(float)*size);
        qsort(sortptr,size,sizeof(float),floatcmp);
        unique=1;
        i=0;
        while(sortptr[i]<min_z) i++;
        start=i;
        for(;i<size-1 && sortptr[i]<=max_z;i++)
        {
            if (sortptr[i]!=sortptr[i+1]) unique++;
        }
        size=i+1;
        if (unique>LOGPALUSED)
        {     
            floats_in_bin=unique/LOGPALUSED;
            first_bin = floats_in_bin+unique-floats_in_bin*LOGPALUSED;
#ifdef DEBUG
            fp=fopen("test.fff","wb"); 
            fprintf(fp,"unique: %d in bin: %d first bin: %d\n",unique,floats_in_bin,first_bin);
#endif
            filler_bins=0;
            i=start;
            fhist[0]=sortptr[i];
            j=1;
            while(j<first_bin)
            {
                i++;
                if (sortptr[i]!=sortptr[i-1]) j++;
            }       
            fhist[1]=sortptr[i];
            for(k=2;k<LOGPALUSED;k++)
            {
                i++;
                j=1;
                while(j<floats_in_bin && i<size)
                {
                    i++;
                    if (sortptr[i]!=sortptr[i-1]) j++;
                }
                if (i<size) fhist[k]=sortptr[i];
                else fhist[k]=sortptr[i-1];
            }
                    
#ifdef DEBUG
            for(k=0;k<LOGPALUSED;k++)
            {
                fprintf(fp,"%d %f\n",k,fhist[k]);
            }
            fclose(fp);
#endif
        }
        else
        {
#ifdef DEBUG
            fp=fopen("test.fff","wb"); 
#endif
            floats_in_bin=1;
            filler_bins=LOGPALUSED-unique;
            i=start;
            if (filler_bins>unique)
            {
                filler_float_ratio=filler_bins/unique;
#ifdef DEBUG
fprintf(fp,"%d unique %d filler\nfilling \n",unique,filler_bins);
#endif
                for(k=0;k<filler_bins%unique;k++)
                {
                    fhist[k]=sortptr[i]+k-
                        filler_bins%unique;
#ifdef DEBUG
fprintf(fp,"%f\n",fhist[k]);
#endif
                }
                while(k<LOGPALUSED)
                {
                    fhist[k]=sortptr[i];
#ifdef DEBUG
fprintf(fp,"real: %f\n",fhist[k]);
#endif
                    i++;
                    while(sortptr[i]==sortptr[i+1] && i<size) i++;
                    if (i<size)
                    {
                        filler=(sortptr[i]-fhist[k])/
                            (float) filler_float_ratio*0.95; /*0.95 for assurance*/
                    }
                    else filler=1;
                    k++;
#ifdef DEBUG
fprintf(fp,"filling\n");
#endif
                    for(j=0;j<filler_float_ratio && k<LOGPALUSED;j++)
                    {
                        if (i<size)
                        {
                            fhist[k]=sortptr[i]-
                                (float)(filler_float_ratio-j)*filler;
                        }
                        else
                            fhist[k]=sortptr[i-1]+
                                (float)(j+1)*filler;
#ifdef DEBUG
fprintf(fp,"%f\n",fhist[k]);
#endif
                        k++;
                    }
                }                        
            }
            else
            {
                filler_float_ratio=unique/filler_bins;
                for(k=0;k<unique%filler_bins;k++)
                {
                    fhist[k]=sortptr[i];
                    i++;
                    while(sortptr[i]==sortptr[i+1] && i<size) i++;
                }
                while(k<LOGPALUSED)
                {
                    if (i<size)
                    {
                        fhist[k]= (sortptr[i]+
                            fhist[k-1])/2;
                    }
                    else
                    {
                        fhist[k]= 1+
                            fhist[k-1];
                    }
                        
                    k++;
                    for(j=0;j<filler_float_ratio && k<LOGPALUSED;j++)
                    {
                        fhist[k]=sortptr[i];
                        i++;
                        while(sortptr[i]==sortptr[i+1] && i<size) i++;
                        k++;
                    }
                }
            }
                        
                    
        }

#ifdef DEBUG
        { 
            for(k=0;k<LOGPALUSED;k++)
            {
                fprintf(fp,"%f\n",fhist[k]);
            }
            fclose(fp);
        }
#endif
        free(sortptr);
      
}
#ifdef OLD
void equalize_old
        
        for(i=0;i<LOGPALUSED;i++)
        {
            this_data->fhist[i]=sortptr[size*i/LOGPALUSED];
            if (i)
            {
            
                j=0;
                while (this_data->fhist[i]==
                    this_data->fhist[i-1] && 
                        size*i/LOGPALUSED/+j-1<size)
                {
                    j++;
                    this_data->fhist[i]=sortptr[size*i/LOGPALUSED+j];
                }
            }
        }
        done=0;
        while(!done)
        {
            done=1;
            for (i=0;i<LOGPALUSED-2;i++)
            {
                
                if (this_data->fhist[i]==
                    this_data->fhist[i+1])
                {
                    done=0;
                    this_data->fhist[i+1]=
                        (this_data->fhist[i]+
                            this_data->fhist[i+2])/2;
                }
            }
            if (this_data->fhist[LOGPALUSED-2]==
                this_data->fhist[LOGPALUSED-1])
            {
                done=0;
                this_data->fhist[LOGPALUSED-1]+=1;
            }
        }
      size=this_data->size;
      for(i=0;i<DATA_HIST_MAX;i++) histogram1[i]=0;
      for(i=0;i<size;i++)
        for(j=0;j<size;j++)
        {
            histogram1[(int)floor((*(this_data->ptr+i*size+j)-
               this_data->min_z)*DATA_HIST_MAX/(this_data->max_z-
               this_data->min_z+1))]++;
        }
      j=0;
      max_hist=size*size/LOGPALUSED;
      size=0;
      hist_pos=0;
      for(i=0;i<DATA_HIST_MAX;i++)
      {
        if (size<max_hist)
        {
            this_data->hist[i]=j;
        }
        else
        {
            size-=max_hist;
            this_data->hist[i]=++j;
            if (j>=LOGPALUSED) j=LOGPALUSED-1;
        }
        size+=histogram1[hist_pos++];
        if (hist_pos>=DATA_HIST_MAX) hist_pos=DATA_HIST_MAX-1;
      }
}
#endif

void repaint_pal(HWND hDlg, int x, int y,unsigned char *dacbox,datadef *this_data)
{
    HDC             hDC;
    int             tempwidth,
                    tempheight;
    unsigned char * this_dacbox;
    int fixed=0;
    char *logpal_image;
    int i,j;
    int color32;
    float data_size;

    if (this_data==NULL)
    {
        this_dacbox=dacbox;
        fixed=1;
    }    
    else
    {
        if (this_data->pal.type==PALTYPE_MAPPED) 
        {
            fixed=1;
            this_dacbox=this_data->pal.dacbox;
        }
        else
        {
            data_size=this_data->max_z-this_data->min_z;
            if (data_size<=0) data_size=1;
            logpal_image=(char *) malloc(3 * PAL_IM_WIDTH * PAL_IM_HEIGHT * sizeof(char));
            for (i = 0; i < PAL_IM_HEIGHT; i++)
                for (j = 0; j < PAL_IM_WIDTH; j++)
                memcpy(logpal_image + (j + i * PAL_IM_WIDTH)*3,
                    bgr(this_data,this_data->min_z+
                    (float)(j) / (float)PAL_IM_WIDTH*data_size,&color32),3);
            hDC = GetDC(hDlg);
            SetMapMode(hDC, MM_TEXT);
            tempwidth = bitmapinfo->bmiHeader.biWidth;
            tempheight = bitmapinfo->bmiHeader.biHeight;
            bitmapinfo->bmiHeader.biBitCount = 24;
            bitmapinfo->bmiHeader.biWidth = PAL_IM_WIDTH;
            bitmapinfo->bmiHeader.biHeight = PAL_IM_HEIGHT;
        
            SetDIBitsToDevice(hDC, x, y, PAL_IM_WIDTH, PAL_IM_HEIGHT, 0, 0, 0, PAL_IM_HEIGHT, logpal_image, bitmapinfo, DIB_RGB_COLORS);
            bitmapinfo->bmiHeader.biBitCount = 8;
            bitmapinfo->bmiHeader.biWidth = tempwidth;
            bitmapinfo->bmiHeader.biHeight = tempheight;
            ReleaseDC(hDlg, hDC);
        
            free(logpal_image);
        }
    }
    if (fixed)
    {
        hDC = GetDC(hDlg);
        SetMapMode(hDC, MM_TEXT);
        spindac(0,1,this_dacbox);
        SelectPalette(hDC, hPal, 0);
        RealizePalette(hDC);
        tempwidth = bitmapinfo->bmiHeader.biWidth;
        tempheight = bitmapinfo->bmiHeader.biHeight;
        bitmapinfo->bmiHeader.biBitCount = 8;
        bitmapinfo->bmiHeader.biWidth = PAL_IM_WIDTH;
        bitmapinfo->bmiHeader.biHeight = PAL_IM_HEIGHT;
    
        SetDIBitsToDevice(hDC, x, y, PAL_IM_WIDTH, PAL_IM_HEIGHT, 0, 0, 0, PAL_IM_HEIGHT, pal_image, bitmapinfo, DIB_PAL_COLORS);
        bitmapinfo->bmiHeader.biWidth = tempwidth;
        bitmapinfo->bmiHeader.biHeight = tempheight;
//      MessageBox(hDlg,"Put palette","hmmmm",MB_OKCANCEL|MB_ICONEXCLAMATION);
        ReleaseDC(hDlg, hDC);
    }
}

