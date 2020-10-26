//#define DEBUG
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include <i86.h>
#include "stm.h"
#include "common.h"
#include "dio.h"
#include "data.h"
#include "file.h"
#include "scan.h"
#include "pal.h"
#include "clock.h"
#include "count.h"

extern char string[];
extern HPALETTE                hPal;

void CheckMsgQ()
{
    MSG msg;
    
    while (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
    {
        TranslateMessage( &msg);
        DispatchMessage(&msg);
    }
}
void init_listbox(HWND hDlg, int listboxid, LISTBOX *listbox)
{
    int i;
    int num;
    
    num=SendDlgItemMessage(hDlg, listboxid, CB_GETCOUNT, 0,
        (LONG) 0);
    for(i=0;i<num;i++) SendDlgItemMessage(hDlg, listboxid, CB_DELETESTRING,
        0,0);
    for(i=0;i<listbox->num;i++)
    {
#ifdef DEBUG
      sprintf(string,"%s %d ",listbox->str[i],i);
      MessageBox(hDlg,string,"test",MB_ICONHAND);
#endif
        num=SendDlgItemMessage(hDlg, listboxid, CB_ADDSTRING, 0, 
            (LPARAM) listbox->str[i]);
#ifdef DEBUG
        sprintf(string,"%d",num);
        MessageBox(hDlg,string,"test",MB_ICONHAND);
#endif
    }
#ifdef DEBUG
    num=SendDlgItemMessage(hDlg, listboxid, CB_GETCOUNT, 0,
        0);
      sprintf(string,"%d",num);
      MessageBox(hDlg,string,"test",MB_ICONHAND);
#endif
    for(i=0;i<listbox->num;i++)
    {
        listbox->index[SendDlgItemMessage(hDlg, listboxid, CB_FINDSTRINGEXACT, 0,
            (LPARAM) listbox->str[i])]=i;
    }
    SendDlgItemMessage(hDlg, listboxid, CB_SETCURSEL, 0 , 0);
}

char *scan_freq_str(char *buf,unsigned int scale,unsigned int freq)
{
    switch(scale)
    {
        case 0:
            switch(freq)
            {
                case 0:
                    strcpy(buf,"10 Hz");
                    break;
                case 1:
                    strcpy(buf,"100 Hz");
                    break;
                case 2:
                    strcpy(buf,"1 kHz");
                    break;
            }
            break;
        case 1:
            switch(freq)
            {
                case 0:
                    strcpy(buf,"100 Hz");
                    break;
                case 1:
                    strcpy(buf,"1 kHz");
                    break;
                case 2:
                    strcpy(buf,"10 kHz");
                    break;
            }
            break;
        case 2:
            switch(freq)
            {
                case 0:
                    strcpy(buf,"1 kHz");
                    break;
                case 1:
                    strcpy(buf,"10 kHz");
                    break;
                case 2:
                    strcpy(buf,"100 kHz");
                    break;
            }
            break;
    }
    return(buf);
}

void repaint_grid(HWND hDlg,int x_min,int x_max,int y_min,int y_max)
{
    int i;
    HPEN hpen_old,hpen_dash;
    HDC dep_pulse_hdc;
    
    dep_pulse_hdc=GetDC(hDlg);
    hpen_dash=CreatePen(PS_DASH,1,RGB(0,0,0));

    hpen_old=SelectObject(dep_pulse_hdc,hpen_dash);
        
    for(i=1;i<10;i++)
    {
//        MoveTo(dep_pulse_hdc,x_min,
//            i*(y_max-y_min+1)/10+y_min);
        MoveToEx(dep_pulse_hdc,x_min,
            i*(y_max-y_min+1)/10+y_min,NULL);
        LineTo(dep_pulse_hdc,x_max+1,
            i*(y_max-y_min+1)/10+y_min);
//        MoveTo(dep_pulse_hdc,
//            i*(x_max-x_min+1)/10+x_min,
//            y_min);
        MoveToEx(dep_pulse_hdc,
            i*(x_max-x_min+1)/10+x_min,
            y_min,NULL);
        LineTo(dep_pulse_hdc,
            i*(x_max-x_min+1)/10+x_min,
            y_max+1);
    }
    SelectObject(dep_pulse_hdc,hpen_old);
    DeleteObject(hpen_dash);
    
    ReleaseDC(hDlg,dep_pulse_hdc);
}

void fit_plane_simple(datadef *this_data, double *a, double *b, double *c)
{
    int i,j;
    double gamma,sz,sxz,syz,z;
 
    float n=this_data->size;
    sz=sxz=syz=0;
    for (i=0;i<this_data->size;i++)
        for(j=0;j<this_data->size;j++)
        {
            z=*(this_data->ptr+this_data->size*i+j);
            sz+=z;
            sxz+= i*z;
            syz+= j*z;
        }
        
    *c=(-6*(sxz+syz)+(7*n-5)*sz)/(n*n+n*n*n);
    gamma=6* sz /(n*n+n*n*n);
    *a=12*sxz/(n*n*n*n-n*n)-gamma;
    *b=12*syz/(n*n*n*n-n*n)-gamma;

}

void copy_step(read_step *dest_step,read_step *src_step)
{
	dest_step->type = src_step->type;
	dest_step->state = src_step->state;
	dest_step->wait = src_step->wait;
	dest_step->out_ch = src_step->out_ch;
	dest_step->step_bias = src_step->step_bias;
	dest_step->in_ch = src_step->in_ch;
	dest_step->num_reads = src_step->num_reads;
	strcpy(dest_step->step_type_string,src_step->step_type_string);
}

void copy_data(datadef **dest, datadef **source)
{
    char *cp,*sp,*dp;
    float *fptr;
    unsigned short *dptr;
    unsigned char *dacbox;
    float *xf,*yf;
    unsigned short *xd,*yd;
    READ_SEQ *read_seq;
	int i;

    if (!((*source)->valid)) return;
    switch((*source)->type)
    {
        case DATATYPE_2D:
        case DATATYPE_SPEC_I:
        case DATATYPE_SPEC_Z:
        case DATATYPE_SPEC_DIDV:
        case DATATYPE_MASS:
            free_data(dest);
            alloc_data(dest,(*source)->type,(*source)->size,GEN2D_NONE,GEN2D_NONE,0);
            break;
        case DATATYPE_GEN2D:
            free_data(dest);
            alloc_data(dest,(*source)->type,(*source)->size,(*source)->x_type,
                (*source)->y_type,0);
            break;
        case DATATYPE_3D:
            if ((*dest)==NULL)
            {
                alloc_data(dest,DATATYPE_3D,DATA_MAX,GEN2D_NONE,GEN2D_NONE,
                (*source)->read_seq_num);
            }
            else if ((*dest)->type!=DATATYPE_3D)
            {
                free_data(dest);
                alloc_data(dest,DATATYPE_3D,DATA_MAX,GEN2D_NONE,GEN2D_NONE,
                (*source)->read_seq_num);
            }
            else 
            {
                destroy_logpal(&((*dest)->pal.logpal));
                if ((*dest)->read_seq_num!=(*source)->read_seq_num)
                {
                    free_data_seq(dest);
                    alloc_data_seq(dest,(*source)->read_seq_num);
                }
            }
            break;
    }
    
    switch((*dest)->type)
    {
        case DATATYPE_3D:
            fptr=(*dest)->ptr;
            break;
        case DATATYPE_2D:
            fptr=(*dest)->time2d;
            dptr=(*dest)->data2d;
            break;
        case DATATYPE_GEN2D:
            xd=(*dest)->xd;
            yd=(*dest)->yd;
            xf=(*dest)->xf;
            yf=(*dest)->yf;
            break;
    }
        
    read_seq=(*dest)->read_seq;
    cp=(*dest)->comment.ptr;
    sp=(*dest)->sample_type.ptr;
    dp=(*dest)->dosed_type.ptr;
    dacbox=(*dest)->pal.dacbox;
    memcpy((*dest),(*source),sizeof(datadef));
    (*dest)->pal.logpal=NULL;
    if ((*source)->type==DATATYPE_3D &&
        (*source)->pal.type==PALTYPE_LOGICAL) 
        copy_logpal(&((*dest)->pal.logpal),(*source)->pal.logpal);
    (*dest)->comment.ptr=cp;
    (*dest)->sample_type.ptr=sp;
    (*dest)->dosed_type.ptr=dp;
    (*dest)->pal.dacbox=dacbox;
    (*dest)->valid=(*source)->valid;
    memcpy((*dest)->comment.ptr,(*source)->comment.ptr,COMMENTMAXSIZE);
    memcpy((*dest)->sample_type.ptr,(*source)->sample_type.ptr,
        SAMPLETYPESIZE);
    memcpy((*dest)->dosed_type.ptr,(*source)->dosed_type.ptr,
        SAMPLETYPESIZE);
    
    switch((*dest)->type)
    {
        case DATATYPE_3D:
            (*dest)->ptr=fptr;    
            memcpy((*dest)->ptr,(*source)->ptr,sizeof(float)*(*source)->size*(*source)->size);
            memcpy((*dest)->pal.dacbox,(*source)->pal.dacbox
                ,SIZEOFMPAL);
/* no break on purpose */
        case DATATYPE_3D_PARAMETERS:
            (*dest)->read_seq=read_seq;
            memcpy((*dest)->read_seq,(*source)->read_seq,sizeof(READ_SEQ)*
                (*source)->read_seq_num);
            for(i = 0;i < (*source)->total_steps;i++)
			  copy_step(&((*dest)->sequence[i]),&((*dest)->sequence[i]));
            break;
        case DATATYPE_SPEC_I:
        case DATATYPE_SPEC_Z:
        case DATATYPE_SPEC_DIDV:
        case DATATYPE_MASS:
            (*dest)->ptr=fptr;    
            memcpy((*dest)->ptr,(*source)->ptr,sizeof(float)*(*source)->size);
            break;
        case DATATYPE_2D:
            (*dest)->time2d=fptr;
            (*dest)->data2d=dptr;
            memcpy((*dest)->time2d,(*source)->time2d,sizeof(float)*(*source)->size);
            memcpy((*dest)->data2d,(*source)->data2d,sizeof(unsigned short)*(*source)->size);
            break;
        case DATATYPE_GEN2D:
            (*dest)->xf=xf;
            (*dest)->yf=yf;
            (*dest)->xd=xd;
            (*dest)->yd=yd;
            switch((*source)->x_type)
            {
                case GEN2D_USHORT:
                    memcpy((*dest)->xd,(*source)->xd,sizeof(unsigned short)*(*source)->size);
                    break;
                case GEN2D_FLOAT:
                    memcpy((*dest)->xf,(*source)->xf,sizeof(float)*(*source)->size);
                    break;
            }
            switch((*source)->y_type)
            {
                case GEN2D_USHORT:
                    memcpy((*dest)->yd,(*source)->yd,sizeof(unsigned short)*(*source)->size);
                    break;
                case GEN2D_FLOAT:
                    memcpy((*dest)->yf,(*source)->yf,sizeof(float)*(*source)->size);
                    break;
            }
            break;
    }
    
//    strcpy((*dest)->filename,(*source)->filename);
    
/*
    (*dest)->size=(*source)->size;   
    (*dest)->step=     (*source)->step;   
    (*dest)->x=        (*source)->x;      
    (*dest)->y=        (*source)->y;      
    (*dest)->z=        (*source)->z;      
    (*dest)->x_gain=   (*source)->x_gain; 
    (*dest)->y_gain=   (*source)->y_gain; 
    (*dest)->z_gain=   (*source)->z_gain; 
    (*dest)->z_freq=   (*source)->z_freq; 
    (*dest)->max_z=    (*source)->max_z;  
    (*dest)->min_z=    (*source)->min_z;  
    (*dest)->comment.size=(*source)->comment.size;
*/
}   

void repaint_rect(HWND hDlg,int on,int current_image,int border,int size)
{
  HDC hDC;
  LOGBRUSH      lplb;
  HBRUSH        hBrush,hBrushOld;
  HPEN          hPen,hPenOld;
  int x,y,x2,y2;
      
  hDC = GetDC(hDlg);
/*
  SelectPalette(hDC,hPal,0);
  RealizePalette(hDC);
*/
  lplb.lbStyle = BS_HOLLOW;
  hBrush = CreateBrushIndirect(&lplb);
  hBrushOld = SelectObject(hDC,hBrush);
  x=border-1+(current_image % 2)*(size+border);
  y=border-1+(current_image/2)*(size+border);
  x2=x+size+border-2;
  y2=y+size+border-2;
  switch(on) {
	case 0:
	  hPen =  CreatePen(PS_SOLID,2,RGB(255,255,255));
	  break;
	case 1:
	  hPen =  CreatePen(PS_SOLID,2,RGB(255,0,0));
	  break;
	default:
	  hPen =  CreatePen(PS_SOLID,1,RGB(0,0,0));
	  break;
  }
/*
  if (on) hPen = CreatePen(PS_SOLID,2,RGB(255,0,0));
  else hPen = CreatePen(PS_SOLID,2,RGB(255,255,255));
*/
  hPenOld = SelectObject(hDC,hPen);
  Rectangle(hDC,x,y,x2,y2);
  SelectObject(hDC,hBrushOld);
  SelectObject(hDC,hPenOld);
  DeleteObject(hBrush);
  DeleteObject(hPen);
  ReleaseDC(hDlg,hDC);
    
}

void clear_area(HWND hDlg,int x1,int y1,int x2,int y2,COLORREF color)
{
    HDC hDC;
    LOGBRUSH        lplb;
    HBRUSH          hBrush,
                    hBrushOld;
    HPEN            hPen,
                    hPenOld;
    hDC=GetDC(hDlg);
    SelectPalette(hDC,hPal,0);
    RealizePalette(hDC);
    SetMapMode(hDC,MM_TEXT);
    lplb.lbStyle = BS_SOLID;
    lplb.lbColor = color;
    hBrush = CreateBrushIndirect(&lplb);
    hBrushOld = SelectObject(hDC, hBrush);
    hPen = CreatePen(PS_SOLID, 1, color);
    hPenOld = SelectObject(hDC, hPen);
    Rectangle(hDC,x1,y1,x2, y2);
    SelectObject(hDC, hBrushOld);
    SelectObject(hDC, hPenOld);
    DeleteObject(hBrush);
    DeleteObject(hPen);
    ReleaseDC(hDlg,hDC);
}

float distance(float x1,float y1,float x2,float y2)
{
    return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}

#if 0
float interp_z(datadef *this_data,float row,float col)
{
    float z,z1,z2;
    int x1,y1,x2,y2;
    
    row -=0.5;
    col -=0.5;

    if (row<-0.5) row=0;
    if (row>=this_data->size-0.5) row=this_data->size-1;
    if (col<-0.5) col=0;
    if (col>=this_data->size-0.5) col=this_data->size-1;
    
    
    x1=floor(row);
    y1=floor(col);
    x2=x1+1;
    y2=y1+1;
    if (x1<0) x1=0;
    if (y1<0) y1=0;
    if (x2>=this_data->size) x2=this_data->size-1;
    if (y2>=this_data->size) y2=this_data->size-1;
    
    z1=(*(this_data->ptr+x2+this_data->size*y1)-
        *(this_data->ptr+x1+this_data->size*y1))*(row-x1)+
        *(this_data->ptr+x1+this_data->size*y1);
    z2=(*(this_data->ptr+x2+this_data->size*y2)-
        *(this_data->ptr+x1+this_data->size*y2))*(row-x1)+
        *(this_data->ptr+x1+this_data->size*y2);
        
    z=(z2-z1)*(col-y1)+z1;
    
    return(z);
    
}
#endif

float interp_z(datadef *this_data,float row,float col)
{
    float z,z1,z2;
    int x1,y1,x2,y2;
    
    if (row<0) row=0;
    if (row>=this_data->size) row=this_data->size-1;
    if (col<0) col=0;
    if (col>=this_data->size) col=this_data->size-1;

    x1=floor(row);
    y1=floor(col);
    x2=x1+1;
    y2=y1+1;
    
    if (x2>=this_data->size) x2=this_data->size-1;
    if (y2>=this_data->size) y2=this_data->size-1;

    z1=(*(this_data->ptr+x2+this_data->size*y1)-
        *(this_data->ptr+x1+this_data->size*y1))*(row-x1)+
        *(this_data->ptr+x1+this_data->size*y1);
    z2=(*(this_data->ptr+x2+this_data->size*y2)-
        *(this_data->ptr+x1+this_data->size*y2))*(row-x1)+
        *(this_data->ptr+x1+this_data->size*y2);
        
    z=(z2-z1)*(col-y1)+z1;
    
    return(z);
    
}

float raw_interp_z(datadef *this_data,float row,float col)
{
    float z,z1,z2;
    int x1,y1,x2,y2;
    
    x1=floor(row);
    y1=floor(col);

    z = *(this_data->ptr+x1+this_data->size*y1);
        
    return(z);
    
}

float interp_z3(datadef *this_data,float row,float col)
{
    float z,d1,d2;
    int x1,y1,x2,y2;
    int dirx=1;
    int diry=1;
    float az,bz;

    if (row<0) row=0;
    if (row>=this_data->size) row=this_data->size-1;
    if (col<0) col=0;
    if (col>=this_data->size) col=this_data->size-1;
    
    
    x1=floor(row);
    y1=floor(col);
    x2=x1+1;
    y2=y1+1;
    if (x1<0) x1=0;
    if (y1<0) y1=0;
    if (x2>=this_data->size) x2=this_data->size-1;
    if (y2>=this_data->size) y2=this_data->size-1;
    
    d1=(row-x1)*(row-x1)+(col-y1)*(col-y1);
    d2=(row-x2)*(row-x2)+(col-y2)*(col-y2);
    if (d2<=d1)
    {
        x1=x2;
        y1=y2;
        dirx=diry=-1;
    } 
    if (x1==this_data->size-1)
    {
        dirx=-1;
    }
    if (y1==this_data->size-1)
    {
        diry=-1;
    }
    if (x1<0 || (x1+dirx)<0 || x1>=this_data->size || 
        (x1+dirx)>=this_data->size || y1<0 || (y1+diry)<0 || 
        y1>=this_data->size || (y1+diry)>=this_data->size)
    {
        sprintf(string,"%f %f %d %d %d %d",row,col,x1,y1,dirx,diry);
        mprintf(string);
    }
#ifdef DEBUG
    if (row<1 && col<1)
    {
        sprintf(string,"%f %f %d %d %d %d",row,col,x1,y1,dirx,diry);
        mprintf(string);
    }
#endif    
    
    
    az=-*(this_data->ptr+x1+this_data->size*y1)+
        *(this_data->ptr+(x1+dirx)+this_data->size*y1);
    bz=-*(this_data->ptr+x1+this_data->size*y1)+
        *(this_data->ptr+x1+this_data->size*(y1+diry));
    
    z=*(this_data->ptr+x1+this_data->size*y1)+dirx*bz*(col-y1)
        -diry*az*(row-x1);
/*
    z=*(this_data->ptr+x1*this_data->size+y1)-diry*az*(row-x1)
        -dirx*bz*(col-y1);
*/
    
    return(z);
    
}
float interp_z2(datadef *this_data,float row,float col)
{
    float z,d1,d2,d3,d4,w1,w2,w3,w4;
    int x1,y1,x2,y2;
    
    
    x1=floor(row-0.5);
    y1=floor(col-0.5);
    x2=ceil(row-0.5);
    y2=ceil(col-0.5);
    if (x1<0) x1=x2;
    if (y1<0) y1=y2;
    if (x2>=this_data->size) x2=x1;
    if (y2>=this_data->size) y2=y1;
    d1=distance(row,col,x1,y1);
    d2=distance(row,col,x1,y2);
    d3=distance(row,col,x2,y1);
    d4=distance(row,col,x2,y2);
    w1=(SQRT2-d1)*(SQRT2-d1);
    w2=(SQRT2-d2)*(SQRT2-d2);
    w3=(SQRT2-d3)*(SQRT2-d3);
    w4=(SQRT2-d4)*(SQRT2-d4);
    z=(*(this_data->ptr+y1*this_data->size+x1)*w1+
       *(this_data->ptr+y1*this_data->size+x2)*w2+
       *(this_data->ptr+y2*this_data->size+x1)*w3+
       *(this_data->ptr+y2*this_data->size+x2)*w4)/(w1+w2+w3+w4);
    return(z);
    
}
int bin_find(float *ptr, int find_min, int find_max, float goal)
{
    int result;
    
    if (find_min>=find_max || ptr[find_min]>=ptr[find_max]) 
        return find_max;
    result=(find_max+find_min)/2;
    if (result==find_max || result == find_min) return(result);
    if (result<=0) return(0);
    if (result>=LOGPALUSED-1) return(LOGPALUSED-1);
    if (ptr[result]<=goal && ptr[result+1]> goal) return result;
    else if (ptr[result]<goal) return(bin_find(ptr,result,find_max,goal));
    else return(bin_find(ptr,find_min,result,goal));
}

char *bgr(datadef *this_data,float this_z,int *color32)
{
    char *color;
    int z;
    float data_size;
    LOGPAL_EL *this_logpal;
    int pos;
    float fpos;
    float zf,w1,w2;
    int fzf;
    
    color=(char *) color32;
    switch(this_data->pal.type)
    {
        case PALTYPE_MAPPED:
            data_size=this_data->max_z-this_data->min_z;
            if (data_size<=0) data_size=1;
    
            if (this_data->equalized)
            {
                zf=float_bin_find(&(this_data->fhist[0]),0,LOGPALUSED-1,this_z);
            }
            else
            {    
                zf=((this_z-this_data->min_z)*
                    LOGPALUSED/data_size - 1.0);
                if(zf < 0) zf = 0;
				if(zf >= LOGPALUSED) zf = LOGPALUSED - 1;
            }
            fzf=floor(zf);
            w1=1-zf+fzf;
            w2=1-w1;
            color[0]=this_data->pal.dacbox[fzf*3+2]*w1+
                this_data->pal.dacbox[(fzf+1)*3+2]*w2;
            color[1]=this_data->pal.dacbox[fzf*3+1]*w1+
                this_data->pal.dacbox[(fzf+1)*3+1]*w2;
            color[2]=this_data->pal.dacbox[fzf*3]*w1+
                this_data->pal.dacbox[(fzf+1)*3]*w2;
            break;
        case PALTYPE_LOGICAL:
            data_size=this_data->max_z-this_data->min_z;
            if (data_size==0) data_size=1;
            fpos=((this_z -this_data->min_z)*
                    ((float)LOGPAL_NUM_INDICES)/data_size);
            pos=floor(fpos);
            if (pos>=LOGPAL_NUM_INDICES) pos=LOGPAL_NUM_INDICES-1;
            if (pos<0) pos=0;
            this_logpal=this_data->pal.logpal;
            while(pos<this_logpal->index || pos>this_logpal->next->index)
            {
                this_logpal=this_logpal->next;
            }
            if (this_logpal->equalized)
            {
                z=bin_find(&(this_logpal->fhist[0]),0,LOGPALUSED-1,this_z);
                fpos=((float)z)/(float)(LOGPALUSED-1);
            }
            else
            {
                fpos=(fpos-this_logpal->index)/(float)(this_logpal->next->index-
                    this_logpal->index+1);
            }
            fpos=pow(fpos,this_logpal->gamma);
            *color=this_logpal->color.b+(this_logpal->next->color.b-
                this_logpal->color.b)*fpos;
            *(color+1)=this_logpal->color.g+(this_logpal->next->color.g-
                this_logpal->color.g)*fpos;
            *(color+2)=this_logpal->color.r+(this_logpal->next->color.r-
                this_logpal->color.r)*fpos;            
            break;
    }
            
    return color;
}

float linear_data(datadef *this_data,float row,float col)
{
    float z;
    float data_size;
    int color32;
    char *color;
    
/*
Y = 0.212671 * R + 0.715160 * G + 0.072169 * B;
*/
    data_size=this_data->max_z-this_data->min_z;
    if (data_size==0) data_size=1;
    z=interp_z(this_data,row*(float)this_data->size,col*(float)this_data->size);
    if (this_data->pal.type==PALTYPE_LOGICAL)
    {
        color = (char *) &color32;
        bgr(this_data,z,&color32);
        return(((float)color[0]*0.212671+(float)color[1]*0.71516+(float)
            color[2]*0.072169)/(float)255.0);
    }
    if (this_data->equalized)
    {
        z=float_bin_find(&(this_data->fhist[0]),0,LOGPALUSED-1,z);
        return(z/(float)(LOGPALUSED-1));
    }
    return((z-this_data->min_z)/data_size);
}

float raw_linear_data(datadef *this_data,float row,float col)
{
    float z;
    float data_size;
    int color32;
    char *color;
    
/*
Y = 0.212671 * R + 0.715160 * G + 0.072169 * B;
*/
    data_size = this_data->max_z - this_data->min_z;
    if(data_size == 0) data_size = 1;
    z = raw_interp_z(this_data,row * (float)this_data->size,col * (float)this_data->size);
    if(this_data->pal.type == PALTYPE_LOGICAL)
    {
        color = (char *)&color32;
        bgr(this_data,z,&color32);
        return(((float)color[0] * 0.212671 + (float)color[1] * 0.71516 + (float)
            color[2] * 0.072169) / (float)255.0);
    }
    if(this_data->equalized)
    {
        z = float_bin_find(&(this_data->fhist[0]),0,LOGPALUSED - 1,z);
        return(z / (float)(LOGPALUSED - 1));
    }
    return((z - this_data->min_z) / data_size);
}

int linear_data3(datadef *this_data,float row,float col)
{
    float z;
    float data_size;
    int color32;
    
    data_size=this_data->max_z-this_data->min_z;
    if (data_size==0) data_size=1;
    z=interp_z(this_data,row*(float)this_data->size,col*(float)this_data->size);
    bgr(this_data,z,&color32);
    return(color32);
}

int raw_linear_data3(datadef *this_data,float row,float col)
{
    float z;
    float data_size;
    int color32;
    
    data_size=this_data->max_z-this_data->min_z;
    if (data_size==0) data_size=1;
    z=raw_interp_z(this_data,row*(float)this_data->size,col*(float)this_data->size);
    bgr(this_data,z,&color32);
    return(color32);
}

int vert_int(float x1,float y1,float theta,float x2,float *y2)
{
    if (theta==90 || theta==-90) return 0;
    *y2=tan(RADIAN(theta))*(x2-x1)+y1;
    return 1;
}

int horiz_int(float x1,float y1,float theta,float *x2,float y2)
{
    if (theta==0) return 0;
    *x2=(y2-y1)/tan(RADIAN(theta))+x1;
    return 1;
}

PRINT_EL *new_print_el(PRINT_EL **this_list,int type)
{
    PRINT_EL *next;
    PRINT_EL *prev;
    
    if (*this_list==NULL)
    {
        *this_list=(PRINT_EL *) malloc(sizeof(PRINT_EL));
        (*this_list)->next=*this_list;
        (*this_list)->prev=*this_list;
    }
    else
    {
        next=*this_list;
        prev=(*this_list)->prev;
        (*this_list)=(PRINT_EL *) malloc(sizeof(PRINT_EL));
        (*this_list)->next=next;
        (*this_list)->prev=prev;
        next->prev=(*this_list);
        prev->next=(*this_list);
    } 
    (*this_list)->type=type;
    (*this_list)->version=PRINT_DATA_VERSION;
    (*this_list)->selected=0;
    (*this_list)->maybe_selected=0;
    switch(type)
    {
        case PRINT_TYPE_IMAGE:  
        case PRINT_TYPE_IMAGE_LINK:
            (*this_list)->sizex=PRINT_DEF_SIZE;
            (*this_list)->x1=PRINT_DEF_X_POS;
            (*this_list)->y1=PRINT_DEF_Y_POS;
            (*this_list)->print_image=NULL;
            (*this_list)->print_grid=NULL;
            (*this_list)->count_data=NULL;
	    (*this_list)->image_resolution_type=PRINT_IMAGE_RES_SMOOTH;
            break;
	case PRINT_TYPE_BAR:
	    (*this_list)->bar_pal.dacbox=NULL;
	    (*this_list)->bar_pal.logpal=NULL;
            (*this_list)->print_image=NULL;
	    (*this_list)->gamma=1.0;
    	    break;
    }
    return ((*this_list));
}

LOGPAL_EL *new_logpal_el(LOGPAL_EL **this_list,int index)
{
    LOGPAL_EL *next;
    
    if (*this_list==NULL)
    {
        *this_list=(LOGPAL_EL *) malloc(sizeof(LOGPAL_EL));
        (*this_list)->next=*this_list;
        (*this_list)->prev=*this_list;
    }
    else
    {
        next=(*this_list)->next;
        (*this_list)->next=(LOGPAL_EL *) malloc(sizeof(LOGPAL_EL));
        (*this_list)->next->next=next;
        (*this_list)->next->prev=*this_list;
        next->prev=(*this_list)->next;
    } 
    (*this_list)->next->index=index;
    (*this_list)->next->equalized=0;
    (*this_list)->next->gamma=1;
    return((*this_list)->next);
}

int new_count_el(COUNT_EL **this_list,int x,int y)
{
    COUNT_EL *next;
    COUNT_EL *prev;
    int result=1;
    
    if (*this_list==NULL)
    {
        *this_list=(COUNT_EL *) malloc(sizeof(COUNT_EL));
        (*this_list)->next=*this_list;
        (*this_list)->prev=*this_list;
    }
    else
    {
        next=*this_list;
        do
        {
            if (next->x==x && next->y==y)
            {
                result=0;
                break;
            }
            next=next->next;
        } while (next!=*this_list);
        if (result)
        {
            next=*this_list;
            prev=(*this_list)->prev;
            (*this_list)=(COUNT_EL *) malloc(sizeof(COUNT_EL));
            (*this_list)->next=next;
            (*this_list)->prev=prev;
            next->prev=(*this_list);
            prev->next=(*this_list);
        }
    } 
    if (result)
    {
        (*this_list)->x=x;
        (*this_list)->y=y;
    }
    return (result);
}  
   
int remove_count_el(COUNT_EL **count_list,int x,int y)
{
    COUNT_EL *this_el;
    int result=0;
    
    if ((*count_list)!=NULL)
    {
        this_el=*count_list;
        do
        {
            if (this_el->x==x && this_el->y==y)
            {
                if (this_el==(*count_list)) (*count_list)=(*count_list)->next;
                if (this_el->next==this_el) *count_list=NULL;
                this_el->next->prev=this_el->prev;
                this_el->prev->next=this_el->next;
                free(this_el);
                result=1;
                break;
            }
            this_el=this_el->next;
        }while (this_el!=(*count_list));
    }
    return result;
}

void destroy_logpal(LOGPAL_EL **this_list)
{
    LOGPAL_EL *temp;
    
    if (*this_list!=NULL)
    {
        temp=(*this_list)->next;
        while(temp!=*this_list)
        {
            temp=temp->next;
            free(temp->prev);
        }
        free(*this_list);
        *this_list=NULL;
    }
}

void copy_logpal(LOGPAL_EL **dest,LOGPAL_EL *source)
{
    LOGPAL_EL *temp,*next,*prev,*this;
    
    if (source!=NULL)
    {
        temp=source;
        do
        {
            this=new_logpal_el(dest,temp->index);
            next=this->next;
            prev=this->prev;
            memcpy(this,temp,sizeof(LOGPAL_EL));
            this->next=next;
            this->prev=prev;
            
            temp=temp->next;
        } while(temp!=source);
        sort_logpal_els(dest);
    }
}
    

LOGPAL_EL *sort_logpal_els(LOGPAL_EL **this_list)
{
    LOGPAL_EL *all[LOGPAL_NUM_INDICES];
    LOGPAL_EL *temp;
    int i;
    
    for(i=0;i<LOGPAL_NUM_INDICES;i++) all[i]=NULL;
    all[(*this_list)->index]=*this_list;
    temp=(*this_list)->next;
    while(temp!=*this_list)
    {
        all[temp->index]=temp;
        temp=temp->next;
    }
    i=0;
    while(all[i]==NULL) i++;
    *this_list=all[i];
    i++;
    temp=*this_list;
    for(;i<LOGPAL_NUM_INDICES;i++)
    {
        if (all[i]!=NULL)
        {
            temp->next=all[i];
            all[i]->prev=temp;
            temp=all[i];
        }
    }
    temp->next=*this_list;
    (*this_list)->prev=temp;
    return(*this_list);        
}


float float_bin_find(float *ptr, int find_min, int find_max, float goal)
{
    int result;
    
    if (find_min>=find_max || ptr[find_min]>=ptr[find_max]) 
        return find_max;
    result=(find_max+find_min)/2;
    if (result==find_max || result == find_min) return(result);
    if (result<=0) return(0);
    if (result>=LOGPALUSED-1) return(LOGPALUSED-1);
    if (ptr[result]==ptr[result+1]) return result;
    if (ptr[result]<=goal && ptr[result+1]> goal) 
        return(((float)result)+(goal-ptr[result])/(ptr[result+1]-ptr[result]));
    else if (ptr[result]<goal) return(float_bin_find(ptr,result,find_max,goal));
    else return(float_bin_find(ptr,find_min,result,goal));
}
float point_line_dist(float x1,float y1,float x2,float y2,float x3,float y3)
{
    float v1,v2,v12,phi,l;
        
    v12=(x2-x1)*(x3-x1)+(y2-y1)*(y3-y1);
    if (fabs(v12)<GRID_ZERO_TOLERANCE)
    {
        return sqrt((x3-x1)*(x3-x1)+(y3-y1)*(y3-y1));
    }
    else
    {
        v1=sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
        v2=sqrt((x3-x1)*(x3-x1)+(y3-y1)*(y3-y1));
        l=v12/v1;
        phi=acos(v12/v2/v1);
        return(l*tan(phi));
        
    }
}
void calc_r_theta(GRID this_grid,float *r,float *theta)
{
    float slope,intercept;
    
    if (this_grid.x[0]==this_grid.x[1]) 
    {
        *theta=-PI/2;
        intercept=this_grid.x[0];
    }
    else 
    {
        *theta=atan(((float)(this_grid.y[1]-this_grid.y[0]))/((float)(this_grid.x[1]-
            this_grid.x[0])));
        slope=((float)(this_grid.y[1]-this_grid.y[0]))/
            (float)(this_grid.x[1]-this_grid.x[0]);
        intercept=((float)this_grid.y[1])-((float)(this_grid.y[1]-this_grid.y[0]))/
            (float)(this_grid.x[1]-this_grid.x[0])*((float)this_grid.x[1]);
    }
    *theta=DEGREE(*theta);
    *r=point_line_dist(this_grid.x[0],this_grid.y[0],this_grid.x[1],
        this_grid.y[1],0,0);
    if (intercept<0) *r*=-1;
        
}

void wait_cursor()
{
    HCURSOR wait_cursor;
    
    wait_cursor=LoadCursor(NULL,IDC_WAIT);
    SetCursor(wait_cursor);
}

void arrow_cursor()
{
    HCURSOR arrow_cursor;
    
    arrow_cursor=LoadCursor(NULL,IDC_ARROW);
    SetCursor(arrow_cursor);
}    

void find_min_max(datadef * data, float *min_z, float *max_z)
{
    unsigned int    i,
                    j;

    *min_z = 7000000;
    *max_z = -7000000;
    switch (data->type)
    {
        case DATATYPE_3D:
            for (j = 0; j < data->size; j++)
            {
                for (i = 0; i < data->size; i++)
                {
                    if (*(data->ptr + j * data->size + i) < *min_z)
                        *min_z = *(data->ptr + j * data->size + i);
                    else if (*(data->ptr + j * data->size + i) > *max_z)
                        *max_z = *(data->ptr + j * data->size + i);
                }
            }
            break;
        case DATATYPE_2D:
           for(i=0;i<data->size;i++)
           {
               if (*(data->data2d+i)<*min_z) *min_z=*(data->data2d+i);
               else if (*(data->data2d+i)>*max_z) *max_z=*(data->data2d+i);
           }
           break;
        case DATATYPE_GEN2D:
           for(i=0;i<data->size;i++)
           {
               if (*(data->yf+i)<*min_z) *min_z=*(data->yf+i);
               else if (*(data->yf+i)>*max_z) *max_z=*(data->yf+i);
           }
           break;
        
    }
    data->min_z = *min_z;
    data->max_z = *max_z;
    data->clip_min = *min_z;
    data->clip_max = *max_z;
}


int color_pal(GENPAL this_pal)
{
    int i;
    int result=0;
    int done=0;
    LOGPAL_EL *this_el;
    
    switch(this_pal.type)
    {
        case PALTYPE_MAPPED:
            i=0;
            while(i<LOGPALUSED && !done)
            {
                if (this_pal.dacbox[i*3]!=this_pal.dacbox[i*3+1] ||
                    this_pal.dacbox[i*3]!=this_pal.dacbox[i*3+2] ||
                    this_pal.dacbox[i*3+1]!=this_pal.dacbox[i*3+2])
                {
                    done=result=1;
                }
                i++;
            }
            break;
        case PALTYPE_LOGICAL:
            this_el=this_pal.logpal;
            do
            {
                if (this_el->color.r!=this_el->color.g ||
                    this_el->color.g!=this_el->color.b ||
                    this_el->color.r!=this_el->color.b)
                {
                    done=result=1;
                }
                this_el=this_el->next;
            } while(this_el!=this_pal.logpal || !done);
            break;
    }
    return (result);
}

double calc_i_set(unsigned int i_setpoint,int i_set_range,unsigned int tip_gain)
{
    return (pow(10, -(dtov(i_setpoint,i_set_range) / 10)) / pow(10, tip_gain) * 1E9);
}
            
void destroy_count(COUNT_DATA *list)
{
    int i;
    
    for(i=0;i<COUNT_COLORS;i++)
    {
        while(list->list[i])
        {
            remove_count_el(&(list->list[i]),
                list->list[i]->x,
                list->list[i]->y);
        }
        list->total[i]=0;
    }
}    

int index2d(datadef *this_data,int index)
{
    int result=1;
    
    if (this_data->type==DATATYPE_GEN2D)
    {
        switch (this_data->type2d)
        {
                    case TYPE2D_SPEC_I:
                    case TYPE2D_SPEC_Z:
                    case TYPE2D_SPEC_2:
                    case TYPE2D_SPEC_3:
                    case TYPE2D_MASSPEC:
                        switch(this_data->sp_mode)
                        {
                            case SP_MODE_ABSOLUTE:
                                result=this_data->start+index*this_data->step;
                                break;
                            case SP_MODE_RELATIVE:
                                result=this_data->start+
                                    this_data->pre_dac_data[this_data->output_ch]+index*this_data->step;
                                break;
                        }
                        break;
        }
    }
            
    return(result);
}

void init_count_data(COUNT_DATA *this_data)
{
    int j;
    for(j=0;j<COUNT_COLORS;j++)
    {
        this_data->list[j]=NULL;
        this_data->total[j]=0;
        
    }
    this_data->color[0].r=255;
    this_data->color[0].g=0;
    this_data->color[0].b=0;
    this_data->color[1].r=0;
    this_data->color[1].g=255;
    this_data->color[1].b=0;
    this_data->color[2].r=0;
    this_data->color[2].g=255;
    this_data->color[2].b=255;
    this_data->color[3].r=255;
    this_data->color[3].g=0;
    this_data->color[3].b=255;
    
    this_data->comment.size = 0;
    this_data->comment.ptr = (char *) 
        malloc(COMMENTMAXSIZE*sizeof(char));
    *(this_data->comment.ptr)=0;
    this_data->hide=0;
    this_data->version=COUNT_DATA_VERSION;
    this_data->size=4;
}    

void copy_count(COUNT_DATA **dest,COUNT_DATA **src)
{
    int i;
    COUNT_EL *this_el;
    
    destroy_count(*dest);
    for(i=0;i<COUNT_COLORS;i++)
    {
        this_el=(*src)->list[i];
        if (this_el)
        {
            do
            {
                new_count_el(&((*dest)->list[i]),
                    this_el->x,
                    this_el->y);
                this_el=this_el->next;
            }
            while(this_el!=(*src)->list[i]);
        }
        (*dest)->total[i]=(*src)->total[i];
        
        (*dest)->color[i]=(*src)->color[i];
    }
    
    (*dest)->comment.size = (*src)->comment.size;
    strcpy((*dest)->comment.ptr,(*src)->comment.ptr);
    (*dest)->hide=(*src)->hide;
    (*dest)->version=(*src)->version;
    (*dest)->size=(*src)->size;    

}    

void free_count(COUNT_DATA **this_data)
{
    if (*this_data)
    {
        free((*this_data)->comment.ptr);
        destroy_count(*this_data);
    }
    free(*this_data);
    *this_data=NULL;
}

int rect_intersect_gen(float r1x1,float r1y1,float r1x2,float r1y2,
    float r2x1,float r2y1,float r2x2,float r2y2)
{
    float tmp;
    int result=0;
    
    if (r1x1>r1x2)
    {
        tmp=r1x2;
        r1x2=r1x1;
        r1x1=tmp;
    }
    if (r1y1>r1y2)
    {
        tmp=r1y2;
        r1y2=r1y1;
        r1y1=tmp;
    }
    if (r2x1>r2x2)
    {
        tmp=r2x2;
        r2x2=r2x1;
        r2x1=tmp;
    }
    if (r2y1>r2y2)
    {
        tmp=r2y2;
        r2y2=r2y1;
        r2y1=tmp;
    }
    result=(r2x1>=r1x1 && r2x1<=r1x2 && r2y1>=r1y1 && r2y1<=r1y2 &&
            r2x2>=r1x1 && r2x2<=r1x2 && r2y2>=r1y1 && r2y2<=r1y2);
    if (!result)
        result=(r1x1>=r2x1 && r1x1<=r2x2);
    if (!result)
        result=(r1x2>=r2x1 && r1x2<=r2x2);
    if (!result)
        result=(r1y1>=r2y1 && r1y1<=r2y2);
    if (!result)
        result=(r1y2>=r2y1 && r1y2<=r2y2);
#ifdef OLD
    
    result=((r1x1>=r2x1 && r1x1<=r2x2) && (r1y1>=r2y2 && r1y1<=r2y1));
    if (!result)
        result=((r1x1>=r2x1 && r1x1<=r2x2) && (r1y2>=r2y2 && r1y2<=r2y1));
    if (!result)
        result=((r2x1>=r1x1 && r2x1<=r1x2) && (r2y2>=r1y1 && r2y2<=r1y2));
    if (!result)
        result=((r2x1>=r1x1 && r2x1<=r1x2) && (r2y1>=r1y1 && r2y1<=r1y2));
    if (!result)
        result=((r2x2>=r1x1 && r2x2<=r1x2) && (r2y2>=r1y1 && r2y2<=r1y2));
    if (!result)
        result=(r2x1<r1x1 && r2x2>r1x2 && r2y2>r1y1 && r2y1<r1y2);
    if (!result)
        result=(r2x1>r1x1 && r2x2<r1x2 && r2y2<r1y1 && r2y1>r1y2);
    if (!result)
        result=(r2x1>r1x1 && r2x1<r1x2 && r2y2<r1y1 && r2y1>r1y2);
#endif
    
    return(result);
}        

void paint_circles_gen(HDC hDC,int x_offset,int y_offset,float pixel_size,
    COUNT_EL *this_list,int size,COLORREF color)
{
    LOGBRUSH        lplb;
    HPEN hpen_old;
    HBRUSH          hBrush,
                    hBrushOld;
    int OldROP;
    int x,y,x2,y2;
    int im_x1,im_y1; 
    COUNT_EL *this_el;
    int circle_size;
   
    SelectPalette(hDC, hPal, 0);
    RealizePalette(hDC);
    OldROP = SetROP2(hDC, R2_COPYPEN);
    lplb.lbStyle = BS_SOLID;
    lplb.lbColor = color;
    hBrush = CreateBrushIndirect(&lplb);
    hBrushOld = SelectObject(hDC, hBrush);
    hpen_old = SelectObject(hDC,GetStockObject(NULL_PEN));
    this_el=this_list;
    circle_size=size*pixel_size;
    do
    {
        im_x1=this_el->x-size/2;
        im_y1=this_el->y-size/2;
        
        x = ((float)im_x1+0.5*(1-size%2))*pixel_size; 
        y = -((float)im_y1+0.5*(1-size%2))*pixel_size; 
        x2 = x+circle_size;
        y2 = y-circle_size;
/*
        x2 = ((float)im_x2)*pixel_size+pixel_size-1; 
        y2 = -((float)im_y2)*pixel_size-pixel_size-1; 
*/
        Ellipse(hDC,(int) (x+x_offset), (int) (y+y_offset),
            (int) (x2+x_offset), (int) (y2+y_offset));                
        this_el=this_el->next;
    } while(this_el!=this_list);
        
        
        
    SetROP2(hDC, OldROP);
    SelectObject(hDC, hBrushOld);
    SelectObject(hDC, hpen_old);
    
    DeleteObject(hBrush);
    
        
        
}

