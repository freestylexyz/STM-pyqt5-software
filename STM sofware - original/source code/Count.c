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
#include "count.h"

BOOL FAR PASCAL CountDlg(HWND,unsigned,WPARAM,LPARAM);

extern struct commentdef *gcomment;
extern BOOL FAR PASCAL CommentDlg(HWND, unsigned, WPARAM, LPARAM);
extern HANDLE hInst;

HWND CountWnd=NULL;
BOOL CountOn=FALSE;
extern HWND ImgDlg;
extern BOOL SelOn;

extern float image_lowerv[],image_higherv[],image_lowert[],image_highert[];
extern int current_image;
extern datadef *gendata[];
extern char string[];
static int out_smart=0;
extern int sel1_on,sel2_on;
extern SEL_POINT sel_p1,sel_p2,sel_p3,sel_p4;
extern SEL_REGION sel_r1,sel_r2;
extern char *current_file_cnt;

COUNT_DATA *glob_count_data;


//int count_symbol_size=2;
COUNT_DATA count_data[4];
int count_mode=COUNT_ADD;
static int first_time=1;
int count_sloppy_erase=1;

//static void destroy_all(int);
static void repaint_all(HWND);
void count_horiz_shift(int);
void count_vert_shift(int);


BOOL FAR PASCAL CountDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{
    int i;
//  int j,k;
    int id;
    int delt,ddelt,mini,maxi;
//  int size;
//  int done;
//  float z,data_max,data_min;
    double d,minid,maxid,deltd,ddeltd;
//  int start;
//  int end;
    char *temp_char, *temp_cnt_filename;
    static FARPROC  lpfnDlgProc;
    
//  int tmp_image;
    

    switch(Message)
    {
        case WM_INITDIALOG:
            if (first_time)
            {
                first_time=0;
            }

	    temp_cnt_filename = (char *)malloc(sizeof(char) * CURRENT_FILENAME_MAX);

            for(i=0;i<4;i++)
            {
                    if (gendata[i]->valid &&
                        gendata[i]->type==DATATYPE_3D)
                    {
                        strcpy(count_data[i].filename,gendata[i]->filename);
                    }
            }
            CheckDlgButton(hDlg,COUNT_SLOPPY_ERASE,count_sloppy_erase);
            SetScrollRange(GetDlgItem(hDlg, COUNT_HORIZ_SCROLL), SB_CTL, 0,512, TRUE);

            
/*
            SetScrollRange(GetDlgItem(hDlg, VSCALE_MIN_SC), SB_CTL, 0,32767, TRUE);
*/
            repaint_all(hDlg);
            break;
    case WM_HSCROLL:
    case WM_VSCROLL:
        if (!out_smart)
        {
            out_smart=1;
            id = getscrollid();
            switch (id)
            {
                case COUNT_HORIZ_SCROLL:
                case COUNT_VERT_SCROLL:
                    i=0;
                    mini=-10;
                    maxi=10;
                    delt=1;
                    ddelt=1;
                    break;

/*
                case VSCALE_MIN_SC:
                    d=(double) image_lowerv[current_image];
                    minid=min(gendata[current_image]->min_z,IN_ZERO);
                    maxid=image_higherv[current_image];
                    deltd=(maxid-minid)/100;
                    ddeltd=(maxid-minid)/10;
                    break;
                case VSCALE_MAX_SC:
                    d=(double) image_higherv[current_image];
                    minid=image_lowerv[current_image];
                    maxid=max(gendata[current_image]->max_z,IN_MAX);
                    deltd=(maxid-minid)/100;
                    ddeltd=(maxid-minid)/10;
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
                case COUNT_HORIZ_SCROLL:
                    if (getscrollcode()==SB_LINELEFT ||
                        getscrollcode()==SB_LINERIGHT) 
                        count_horiz_shift(i);
                    break;
                case COUNT_VERT_SCROLL:
                    if (getscrollcode()==SB_LINELEFT ||
                        getscrollcode()==SB_LINERIGHT) 
                        count_vert_shift(-i);
                    break;
/*
                case VSCALE_MIN_SC:
                    image_lowerv[current_image]=d;
                    repaint_all(hDlg);
                    SendMessage(ImgDlg, WM_COMMAND, IM_NEW_VSCALE, 0);
                    break;
                    
                case VSCALE_MAX_SC:
                    image_higherv[current_image]=d;
                    repaint_all(hDlg);
                    SendMessage(ImgDlg, WM_COMMAND, IM_NEW_VSCALE, 0);
                    break;
*/
            }
            out_smart=0;
        }
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
            case COUNT_SLOPPY_ERASE:
                count_sloppy_erase=IsDlgButtonChecked(hDlg,COUNT_SLOPPY_ERASE);
                break;
            case COUNT_HIDE:
                count_data[current_image].hide=
                    IsDlgButtonChecked(hDlg,COUNT_HIDE);
                SendMessage(ImgDlg,WM_COMMAND,
                    IM_REDRAW_IMAGE,current_image);
                break;
            case COUNT_NEW_IMAGE:
                if (gendata[current_image]->valid &&
                    gendata[current_image]->type==DATATYPE_3D)
                {
                    strcpy(count_data[current_image].filename,
                        gendata[current_image]->filename);
                }
                repaint_all(hDlg);
                break;
            case COUNT_COMMENT:
                gcomment=&(count_data[current_image].comment);
                lpfnDlgProc = MakeProcInstance(CommentDlg, hInst);
                DialogBox(hInst, "COMMENTDLG", hDlg, lpfnDlgProc);
                FreeProcInstance(lpfnDlgProc);  
                break;
            case COUNT_SAVE:
                if (gendata[current_image]->valid &&
                    gendata[current_image]->type==DATATYPE_3D)
                {
                    strcpy(current_file_cnt,
                        gendata[current_image]->filename);
                    temp_char=strrchr(current_file_cnt,'.');
                    if (temp_char!=NULL)
                    {
                        *(temp_char+1)='\0';
                        strcat(current_file_cnt,"cnt");
                    }
                    else strcat(current_file_cnt,".cnt");
                    glob_count_data=&(count_data[current_image]);
                    file_save_as(hDlg,hInst,FTYPE_CNT);
                }
                break;
	    case COUNT_EXPORT:
                if (gendata[current_image]->valid &&
                    gendata[current_image]->type==DATATYPE_3D)
                {
                    strcpy(current_file_cnt, gendata[current_image]->filename);
                    temp_char = strrchr(current_file_cnt, '.');
                    if(temp_char != NULL) {
                        *(temp_char + 1) = '\0';
                        strcat(current_file_cnt, "cxp");
                    }
                    else strcat(current_file_cnt,".cxp");
                    glob_count_data = &(count_data[current_image]);
                    file_save_as(hDlg, hInst, FTYPE_CNT_AS_TXT);
		}
		break;
            case COUNT_LOAD:
                if (gendata[current_image]->valid &&
                    gendata[current_image]->type==DATATYPE_3D)
                {
                    strcpy(current_file_cnt,
                        gendata[current_image]->filename);
                    temp_char=strrchr(current_file_cnt,'.');
                    if (temp_char!=NULL)
                    {
                        *(temp_char+1)='\0';
                        strcat(current_file_cnt,"cnt");
                    }
                    else strcat(current_file_cnt,".cnt");
                    glob_count_data=&(count_data[current_image]);
                    file_open(hDlg,hInst,FTYPE_CNT,0,current_file_cnt);
                    repaint_all(hDlg);
                    SendMessage(ImgDlg,WM_COMMAND,
                        IM_REDRAW_IMAGE,current_image);
                }
                break;    
            case COUNT_CLEAR_ALL:
                destroy_count(&(count_data[current_image]));
                *(count_data[current_image].comment.ptr)=0;
                count_data[current_image].comment.size=0;
                repaint_all(hDlg);
                SendMessage(ImgDlg, WM_COMMAND, IM_REDRAW_IMAGE, current_image);
                break;
            case COUNT_ADD:
            case COUNT_ERASE:
            case COUNT_CONVERT:
                count_mode=LOWORD(wParam);
                break;
            case COUNT_SYMBOL_SIZE:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
                    count_data[current_image].size=max(atoi(string),1);
                    SendMessage(ImgDlg, WM_COMMAND, IM_REDRAW_IMAGE, current_image);
                    out_smart=0;
                }
                break;
            case COUNT_R_1:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
                    i=atoi(string);
                    count_data[current_image].color[0].r=min(max(i,0),255);
                    SendMessage(ImgDlg, WM_COMMAND, IM_REDRAW_IMAGE, current_image);
                    out_smart=0;
                }
                break;
            case COUNT_G_1:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
                    i=atoi(string);
                    count_data[current_image].color[0].g=min(max(i,0),255);
                    SendMessage(ImgDlg, WM_COMMAND, IM_REDRAW_IMAGE, current_image);
                    out_smart=0;
                }
                break;
            case COUNT_B_1:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
                    i=atoi(string);
                    count_data[current_image].color[0].b=min(max(i,0),255);
                    SendMessage(ImgDlg, WM_COMMAND, IM_REDRAW_IMAGE, current_image);
                    out_smart=0;
                }
                break;
            case COUNT_R_2:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
                    i=atoi(string);
                    count_data[current_image].color[1].r=min(max(i,0),255);
                    SendMessage(ImgDlg, WM_COMMAND, IM_REDRAW_IMAGE, current_image);
                    out_smart=0;
                }
                break;
            case COUNT_G_2:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
                    i=atoi(string);
                    count_data[current_image].color[1].g=min(max(i,0),255);
                    SendMessage(ImgDlg, WM_COMMAND, IM_REDRAW_IMAGE, current_image);
                    out_smart=0;
                }
                break;
            case COUNT_B_2:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
                    i=atoi(string);
                    count_data[current_image].color[1].b=min(max(i,0),255);
                    SendMessage(ImgDlg, WM_COMMAND, IM_REDRAW_IMAGE, current_image);
                    out_smart=0;
                }
                break;
            case COUNT_R_3:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
                    i=atoi(string);
                    count_data[current_image].color[2].r=min(max(i,0),255);
                    SendMessage(ImgDlg, WM_COMMAND, IM_REDRAW_IMAGE, current_image);
                    out_smart=0;
                }
                break;
            case COUNT_G_3:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
                    i=atoi(string);
                    count_data[current_image].color[2].g=min(max(i,0),255);
                    SendMessage(ImgDlg, WM_COMMAND, IM_REDRAW_IMAGE, current_image);
                    out_smart=0;
                }
                break;
            case COUNT_B_3:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
                    i=atoi(string);
                    count_data[current_image].color[2].b=min(max(i,0),255);
                    SendMessage(ImgDlg, WM_COMMAND, IM_REDRAW_IMAGE, current_image);
                    out_smart=0;
                }
                break;
            case COUNT_R_4:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
                    i=atoi(string);
                    count_data[current_image].color[3].r=min(max(i,0),255);
                    SendMessage(ImgDlg, WM_COMMAND, IM_REDRAW_IMAGE, current_image);
                    out_smart=0;
                }
                break;
            case COUNT_G_4:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
                    i=atoi(string);
                    count_data[current_image].color[3].g=min(max(i,0),255);
                    SendMessage(ImgDlg, WM_COMMAND, IM_REDRAW_IMAGE, current_image);
                    out_smart=0;
                }
                break;
            case COUNT_B_4:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg, LOWORD(wParam), string, 9);
                    i=atoi(string);
                    count_data[current_image].color[3].b=min(max(i,0),255);
                    SendMessage(ImgDlg, WM_COMMAND, IM_REDRAW_IMAGE, current_image);
                    out_smart=0;
                }
                break;
#ifdef OLD
            case SUB_GET:
                if (SelOn)
                {
                    sub_x=sel_r1.pt1->x-sel_r2.pt1->x;
                    sub_y=sel_r1.pt1->y-sel_r2.pt1->y;
                    repaint_all(hDlg);
                }
                break;
            case SUB_1:                                 
              if (current_image!=0)                     
              {                                         
                subtract_data(current_image,0,sub_x,sub_y,sub_z);         
                reset_image(ImgDlg,3);
/*
                im_src_x[3]=im_src_y[3]=0;              
                im_screenlines[3]=gendata[3]->size;     
                calc_bitmap(3);                         
                repaint_image(hDlg,3,NULL);             
*/
              }                                         
              break;                                    
            case SUB_2:                                 
              if (current_image!=1)                     
              {                                         
                subtract_data(current_image,1,sub_x,sub_y,sub_z);         
                reset_image(ImgDlg,3);
/*
                im_src_x[3]=im_src_y[3]=0;              
                im_screenlines[3]=gendata[3]->size;     
                calc_bitmap(3);                         
                repaint_image(hDlg,3,NULL);             
*/
              }                                         
              break;                                    
            case SUB_3:                                 
              if (current_image!=2)                     
              {                                         
                subtract_data(current_image,2,sub_x,sub_y,sub_z);         
                reset_image(ImgDlg,3);
/*
                im_src_x[3]=im_src_y[3]=0;              
                im_screenlines[3]=gendata[3]->size;     
                calc_bitmap(3);                         
                repaint_image(hDlg,3,NULL);             
*/
              }                                         
              break;                                    
            case SUB_4:                                 
              if (current_image!=3)                     
              {                                         
                subtract_data(current_image,3,sub_x,sub_y,sub_z);         
                reset_image(ImgDlg,3);
/*
                im_src_x[3]=im_src_y[3]=0;              
                im_screenlines[3]=gendata[3]->size;     
                calc_bitmap(3);                         
                repaint_image(hDlg,3,NULL);             
*/
              }                                         
              break;                                    

            case SUB_X:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg, SUB_X, string, 9);
                    sub_x=atoi(string);
                    out_smart=0;
                }
                break;
            case SUB_Y:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg, SUB_Y, string, 9);
                    sub_y=atoi(string);
                    out_smart=0;
                }
                break;
            case SUB_Z:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg, SUB_Z, string, 9);
                    sub_z=in_vtod(atof(string));
                    out_smart=0;
                }
                break;
#endif
            
            case COUNT_EXIT:
                
                DestroyWindow(hDlg);
                CountWnd = NULL;
                CountOn = FALSE;
                break;
    }
    break;
  }
  return(FALSE);
}

#ifdef OLD
static void enable_all(HWND hDlg,int status)
{
/*
    EnableWindow(GetDlgItem(hDlg, VSCALE_MAX), status);
    EnableWindow(GetDlgItem(hDlg, VSCALE_MIN), status);
    EnableWindow(GetDlgItem(hDlg, VSCALE_MAX_SC), status);
    EnableWindow(GetDlgItem(hDlg, VSCALE_MIN_SC), status);
    EnableWindow(GetDlgItem(hDlg, VSCALE_SET_MAX), status);
    EnableWindow(GetDlgItem(hDlg, VSCALE_SET_CENTER), status);
    EnableWindow(GetDlgItem(hDlg, VSCALE_SET_MIN), status);
*/
}    
#endif

static void repaint_all(HWND hDlg)
{
        out_smart=1;
        if (gendata[current_image]->valid && 
            gendata[current_image]->type==DATATYPE_3D)
        {
            sprintf(string,"%d",count_data[current_image].total[0]);
            SetDlgItemText(hDlg, COUNT_RED, string);
            sprintf(string,"%d",count_data[current_image].total[1]);
            SetDlgItemText(hDlg, COUNT_GREEN, string);
            sprintf(string,"%d",count_data[current_image].total[2]);
            SetDlgItemText(hDlg, COUNT_BLUE, string);
            sprintf(string,"%d",count_data[current_image].total[3]);
            SetDlgItemText(hDlg, COUNT_CYAN, string);
        }
        sprintf(string,"%d",count_data[current_image].size);
        SetDlgItemText(hDlg, COUNT_SYMBOL_SIZE, string);
        CheckDlgButton(hDlg,COUNT_ADD,0);
        CheckDlgButton(hDlg,COUNT_ERASE,0);
        CheckDlgButton(hDlg,count_mode,1);
        CheckDlgButton(hDlg,COUNT_HIDE,count_data[current_image].hide);
        sprintf(string,"%d",count_data[current_image].color[0].r);
        SetDlgItemText(hDlg, COUNT_R_1, string);
        sprintf(string,"%d",count_data[current_image].color[0].g);
        SetDlgItemText(hDlg, COUNT_G_1, string);
        sprintf(string,"%d",count_data[current_image].color[0].b);
        SetDlgItemText(hDlg, COUNT_B_1, string);
        sprintf(string,"%d",count_data[current_image].color[1].r);
        SetDlgItemText(hDlg, COUNT_R_2, string);
        sprintf(string,"%d",count_data[current_image].color[1].g);
        SetDlgItemText(hDlg, COUNT_G_2, string);
        sprintf(string,"%d",count_data[current_image].color[1].b);
        SetDlgItemText(hDlg, COUNT_B_2, string);
        sprintf(string,"%d",count_data[current_image].color[2].r);
        SetDlgItemText(hDlg, COUNT_R_3, string);
        sprintf(string,"%d",count_data[current_image].color[2].g);
        SetDlgItemText(hDlg, COUNT_G_3, string);
        sprintf(string,"%d",count_data[current_image].color[2].b);
        SetDlgItemText(hDlg, COUNT_B_3, string);
        sprintf(string,"%d",count_data[current_image].color[3].r);
        SetDlgItemText(hDlg, COUNT_R_4, string);
        sprintf(string,"%d",count_data[current_image].color[3].g);
        SetDlgItemText(hDlg, COUNT_G_4, string);
        sprintf(string,"%d",count_data[current_image].color[3].b);
        SetDlgItemText(hDlg, COUNT_B_4, string);
        
        
        
#ifdef OLD
        sprintf(string,"%d",sub_x);
        SetDlgItemText(hDlg, SUB_X, string);
        sprintf(string,"%d",sub_y);
        SetDlgItemText(hDlg, SUB_Y, string);
        sprintf(string,"%0.3f",in_dtov(sub_z));
        SetDlgItemText(hDlg, SUB_Z, string);
#endif
        out_smart=0;
}

#ifdef OLD
static void destroy_all(int this_image)
{
    while(count_data[this_image].list[0])
    {
        remove_count_el(&(count_data[this_image].list[0]),
            count_data[this_image].list[0]->x,
            count_data[this_image].list[0]->y);
    }
    count_data[this_image].total[0]=0;
    while(count_data[this_image].list[1])
    {
        remove_count_el(&(count_data[this_image].list[1]),
            count_data[this_image].list[1]->x,
            count_data[this_image].list[1]->y);
    }
    count_data[this_image].total[1]=0;
}    
#endif

void init_count()
{
    int i;
    for(i=0;i<4;i++)
    {
        init_count_data(count_data+i);
    }
}    


void count_horiz_shift(int shift)
{
    int i;
    COUNT_EL *this_el;
    
    for(i=0;i<COUNT_COLORS;i++)
    {
        this_el=count_data[current_image].list[i];
        
        if (this_el!=NULL)
        {
            do
            {
                this_el->x+=shift;
/*
sprintf(string,"%d",this_el->x);
mprintf(string);         
*/
                if (this_el->x<0) this_el->x=0;
                if (this_el->x>=gendata[current_image]->size)
                    this_el->x=gendata[current_image]->size-1;
                this_el=this_el->next;
            } while (this_el!=count_data[current_image].list[i]);
        }
    }
    
    SendMessage(ImgDlg,WM_COMMAND,
        IM_REDRAW_IMAGE,current_image);
}

void count_vert_shift(int shift)
{
    int i;
    COUNT_EL *this_el;
    
    for(i=0;i<COUNT_COLORS;i++)
    {
        this_el=count_data[current_image].list[i];
        
        if (this_el!=NULL)
        {
            do
            {
                this_el->y+=shift;
                if (this_el->y<0) this_el->y=0;
                if (this_el->y>=gendata[current_image]->size)
                    this_el->y=gendata[current_image]->size-1;
                this_el=this_el->next;
            } while (this_el!=count_data[current_image].list[i]);
        }
    }
    
    SendMessage(ImgDlg,WM_COMMAND,
        IM_REDRAW_IMAGE,current_image);
}
    
