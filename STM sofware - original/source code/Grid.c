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
#include "grid.h"

BOOL FAR PASCAL GridDlg(HWND,unsigned,WPARAM,LPARAM);

HWND GridWnd=NULL;
BOOL GridOn=FALSE;
extern HWND ImgDlg;
extern BOOL SelOn;

extern HANDLE hInst;

extern float image_lowerv[],image_higherv[],image_lowert[],image_highert[];
extern int current_image;
extern datadef *gendata[];
extern char string[];
extern SEL_REGION sel_r1,sel_r2;
extern int sel2_on,sel1_on;
extern char *current_file_grd;
static int out_smart=0;

GRID grids[4];
GRID *grd_data;
static int first_time=1;
int grid_num_lines[GRID_MAX_LINES];

static void repaint_all(HWND);
static void enable_all(HWND, int);
void grid_get_line(int);
void grid_get_clip_line();
void grid_match3(int, int, int);

BOOL FAR PASCAL GridDlg(HWND hDlg,unsigned Message,WPARAM wParam,LPARAM lParam)
{
    int i,j;
    int id;
    int delt,ddelt,mini,maxi;
    float data_max,temp_f;
    double d,minid,maxid,deltd,ddeltd;
    BOOL dlg_result=FALSE;    
    char *temp_char;

/*
    if (Message==307 || (Message==WM_COMMAND && (LOWORD(wParam)==GRID_DIST_0 ||
        LOWORD(wParam)==GRID_DIST_1 || LOWORD(wParam)==GRID_DIST_2)))
    {
    }
    else
    {
        sprintf(string,"%d",Message);
        SetDlgItemText(hDlg,GRID_DIST_0,string);
        sprintf(string,"%d",lParam);
        SetDlgItemText(hDlg,GRID_DIST_1,string);
        sprintf(string,"%d",wParam);
        SetDlgItemText(hDlg,GRID_DIST_2,string);
    }
*/
    switch(Message)
    {
        case WM_INITDIALOG:
            dlg_result=TRUE;
            if (first_time)
            {
                first_time=0;
                for(i=0;i<GRID_MAX_LINES;i++) grid_num_lines[i]=1;
            }
            SetScrollRange(GetDlgItem(hDlg, GRID_ANGLE_0_SC), SB_CTL, 0,32767, TRUE);
            SetScrollRange(GetDlgItem(hDlg, GRID_ANGLE_1_SC), SB_CTL, 0,32767, TRUE);
            SetScrollRange(GetDlgItem(hDlg, GRID_ANGLE_2_SC), SB_CTL, 0,32767, TRUE);
            SetScrollRange(GetDlgItem(hDlg, GRID_DIST_0_SC), SB_CTL, 0,32767, TRUE);
            SetScrollRange(GetDlgItem(hDlg, GRID_DIST_1_SC), SB_CTL, 0,32767, TRUE);
            SetScrollRange(GetDlgItem(hDlg, GRID_DIST_2_SC), SB_CTL, 0,32767, TRUE);
            SetScrollRange(GetDlgItem(hDlg, GRID_YINT_0_SC), SB_CTL, 0,32767, TRUE);
            SetScrollRange(GetDlgItem(hDlg, GRID_YINT_1_SC), SB_CTL, 0,32767, TRUE);
            SetScrollRange(GetDlgItem(hDlg, GRID_YINT_2_SC), SB_CTL, 0,32767, TRUE);
            SetScrollRange(GetDlgItem(hDlg, GRID_VERT_SC), SB_CTL, 0,32767, TRUE);
            SetScrollRange(GetDlgItem(hDlg, GRID_HORIZ_SC), SB_CTL, 0,32767, TRUE);
            SetScrollPos(GetDlgItem(hDlg, GRID_VERT_SC), 
                   SB_CTL,32768/2,TRUE);
            SetScrollPos(GetDlgItem(hDlg, GRID_HORIZ_SC),  
                   SB_CTL,32768/2,TRUE);
            repaint_all(hDlg);
            break;
    case WM_HSCROLL:
    case WM_VSCROLL:
        dlg_result=TRUE;
        if (!out_smart)
        {
            out_smart=1;
            id = getscrollid();
            switch (id)
            {
                case GRID_VERT_SC:
                    d=(double) 32768/2;
                    minid=0;
                    maxid=32768;
                    deltd=GRID_MOVE_DELT;
                    ddeltd=GRID_MOVE_DDELT;
                    break;
                case GRID_HORIZ_SC:
                    d=(double) 32768/2;
                    minid=0;
                    maxid=32768;
                    deltd=GRID_MOVE_DELT;
                    ddeltd=GRID_MOVE_DDELT;
                    break;
                case GRID_ANGLE_0_SC:
                    d=(double) grids[current_image].line[0].theta;
                    minid=GRID_ANGLE_MIN;
                    maxid=GRID_ANGLE_MAX;
                    deltd=GRID_ANGLE_DELT;
                    ddeltd=GRID_ANGLE_DDELT;
                    break;
                case GRID_YINT_0_SC:
                    d=(double) grids[current_image].line[0].r;
                    minid=GRID_YINT_MIN;
                    maxid=grids[current_image].line[0].dist;
                    deltd=GRID_YINT_DELT;
                    ddeltd=GRID_YINT_DDELT;
                    break;
                case GRID_DIST_0_SC:
                    d=(double) grids[current_image].line[0].dist;
                    minid=GRID_DIST_MIN;
                    maxid=gendata[current_image]->size;
                    deltd=GRID_DIST_DELT;
                    ddeltd=GRID_DIST_DDELT;
                    break;
                case GRID_ANGLE_1_SC:
                    d=(double) grids[current_image].line[1].theta;
                    minid=GRID_ANGLE_MIN;
                    maxid=GRID_ANGLE_MAX;
                    deltd=GRID_ANGLE_DELT;
                    ddeltd=GRID_ANGLE_DDELT;
                    break;
                case GRID_YINT_1_SC:
                    d=(double) grids[current_image].line[1].r;
                    minid=GRID_YINT_MIN;
                    maxid=grids[current_image].line[1].dist;
                    deltd=GRID_YINT_DELT;
                    ddeltd=GRID_YINT_DDELT;
                    break;
                case GRID_DIST_1_SC:
                    d=(double) grids[current_image].line[1].dist;
                    minid=GRID_DIST_MIN;
                    maxid=gendata[current_image]->size;
                    deltd=GRID_DIST_DELT;
                    ddeltd=GRID_DIST_DDELT;
                    break;
                case GRID_ANGLE_2_SC:
                    d=(double) grids[current_image].line[2].theta;
                    minid=GRID_ANGLE_MIN;
                    maxid=GRID_ANGLE_MAX;
                    deltd=GRID_ANGLE_DELT;
                    ddeltd=GRID_ANGLE_DDELT;
                    break;
                case GRID_YINT_2_SC:
                    d=(double) grids[current_image].line[2].r;
                    minid=GRID_YINT_MIN;
                    maxid=grids[current_image].line[2].dist;
                    deltd=GRID_YINT_DELT;
                    ddeltd=GRID_YINT_DDELT;
                    break;
                case GRID_DIST_2_SC:
                    d=(double) grids[current_image].line[2].dist;
                    minid=GRID_DIST_MIN;
                    maxid=gendata[current_image]->size;
                    deltd=GRID_DIST_DELT;
                    ddeltd=GRID_DIST_DDELT;
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
            if (getscrollcode()!=SB_ENDSCROLL) switch (id)
            {
                case GRID_VERT_SC:
                    d=32768/2-d;
                    for(i=0;i<GRID_MAX_LINES;i++)
                    {
                        grids[current_image].line[i].r+=d*cos(
                            RADIAN(grids[current_image].line[i].theta));
                        while (grids[current_image].line[i].r<0)
                            grids[current_image].line[i].r+=
                            grids[current_image].line[i].dist;
                        while (grids[current_image].line[i].r>=
                            grids[current_image].line[i].dist)
                            grids[current_image].line[i].r-=
                            grids[current_image].line[i].dist;
                    }
                    repaint_all(hDlg);
                    if (!grid_hidden(current_image))
                        SendMessage(ImgDlg,WM_COMMAND,
                            IM_REDRAW_IMAGE,current_image);
                    break;
                case GRID_HORIZ_SC:
                    d=32768/2-d;
                    for(i=0;i<GRID_MAX_LINES;i++)
                    {
                        grids[current_image].line[i].r+=d*sin(
                            RADIAN(grids[current_image].line[i].theta));
                        while (grids[current_image].line[i].r<0)
                            grids[current_image].line[i].r+=
                            grids[current_image].line[i].dist;
                        while (grids[current_image].line[i].r>=
                            grids[current_image].line[i].dist)
                            grids[current_image].line[i].r-=
                            grids[current_image].line[i].dist;
                    }
                    repaint_all(hDlg);
                    if (!grid_hidden(current_image))
                        SendMessage(ImgDlg,WM_COMMAND,
                            IM_REDRAW_IMAGE,current_image);
                    break;
                case GRID_ANGLE_0_SC:
                    grids[current_image].line[0].theta=d;
                    sprintf(string,"%0.2f",grids[current_image].line[0].theta);
                    SetDlgItemText(hDlg, GRID_ANGLE_0, string);
                    SetScrollPos(GetDlgItem(hDlg, GRID_ANGLE_0_SC), 
                        SB_CTL,DTOI(grids[current_image].line[0].theta,
                            GRID_ANGLE_MIN,GRID_ANGLE_MAX) , TRUE);
                    if (!grids[current_image].line[0].hide) 
                        SendMessage(ImgDlg,WM_COMMAND,
                            IM_REDRAW_IMAGE,current_image);
                    break;
                    
                case GRID_YINT_0_SC:
                    grids[current_image].line[0].r=d;
                    sprintf(string,"%0.2f",grids[current_image].line[0].r);
                    SetDlgItemText(hDlg, GRID_YINT_0, string);
                    SetScrollPos(GetDlgItem(hDlg, GRID_YINT_0_SC), 
                        SB_CTL,DTOI(grids[current_image].line[0].r,
                            GRID_YINT_MIN,grids[current_image].line[0].dist) , TRUE);
                    if (!grids[current_image].line[0].hide) 
                        SendMessage(ImgDlg,WM_COMMAND,
                            IM_REDRAW_IMAGE,current_image);
                    break;
                case GRID_DIST_0_SC:
                    grids[current_image].line[0].dist=d;
                    sprintf(string,"%0.2f",grids[current_image].line[0].dist);
                    SetDlgItemText(hDlg, GRID_DIST_0, string);
                    SetScrollPos(GetDlgItem(hDlg, GRID_DIST_0_SC), 
                        SB_CTL,DTOI(grids[current_image].line[0].dist,
                        GRID_DIST_MIN,gendata[current_image]->size) , TRUE);
                    
                    if (!grids[current_image].line[0].hide) 
                        SendMessage(ImgDlg,WM_COMMAND,
                            IM_REDRAW_IMAGE,current_image);
                    break;
                case GRID_ANGLE_1_SC:
                    grids[current_image].line[1].theta=d;
                    sprintf(string,"%0.2f",grids[current_image].line[1].theta);
                    SetDlgItemText(hDlg, GRID_ANGLE_1, string);
                    SetScrollPos(GetDlgItem(hDlg, GRID_ANGLE_1_SC), 
                        SB_CTL,DTOI(grids[current_image].line[1].theta,
                            GRID_ANGLE_MIN,GRID_ANGLE_MAX) , TRUE);
                    if (!grids[current_image].line[1].hide) 
                        SendMessage(ImgDlg,WM_COMMAND,
                            IM_REDRAW_IMAGE,current_image);
                    break;
                    
                case GRID_YINT_1_SC:
                    grids[current_image].line[1].r=d;
                    sprintf(string,"%0.2f",grids[current_image].line[1].r);
                    SetDlgItemText(hDlg, GRID_YINT_1, string);
                    SetScrollPos(GetDlgItem(hDlg, GRID_YINT_1_SC), 
                        SB_CTL,DTOI(grids[current_image].line[1].r,
                            GRID_YINT_MIN,grids[current_image].line[1].dist) , TRUE);
                    if (!grids[current_image].line[1].hide) 
                        SendMessage(ImgDlg,WM_COMMAND,
                            IM_REDRAW_IMAGE,current_image);
                    break;
                case GRID_DIST_1_SC:
                    grids[current_image].line[1].dist=d;
                    sprintf(string,"%0.2f",grids[current_image].line[1].dist);
                    SetDlgItemText(hDlg, GRID_DIST_1, string);
                    SetScrollPos(GetDlgItem(hDlg, GRID_DIST_1_SC), 
                        SB_CTL,DTOI(grids[current_image].line[1].dist,
                        GRID_DIST_MIN,gendata[current_image]->size) , TRUE);
                    
                    if (!grids[current_image].line[1].hide) 
                        SendMessage(ImgDlg,WM_COMMAND,
                            IM_REDRAW_IMAGE,current_image);
                    break;
                case GRID_ANGLE_2_SC:
                    grids[current_image].line[2].theta=d;
                    sprintf(string,"%0.2f",grids[current_image].line[2].theta);
                    SetDlgItemText(hDlg, GRID_ANGLE_2, string);
                    SetScrollPos(GetDlgItem(hDlg, GRID_ANGLE_2_SC), 
                        SB_CTL,DTOI(grids[current_image].line[2].theta,
                            GRID_ANGLE_MIN,GRID_ANGLE_MAX) , TRUE);
                    if (!grids[current_image].line[2].hide) 
                        SendMessage(ImgDlg,WM_COMMAND,
                            IM_REDRAW_IMAGE,current_image);
                    break;
                    
                case GRID_YINT_2_SC:
                    grids[current_image].line[2].r=d;
                    sprintf(string,"%0.2f",grids[current_image].line[2].r);
                    SetDlgItemText(hDlg, GRID_YINT_2, string);
                    SetScrollPos(GetDlgItem(hDlg, GRID_YINT_2_SC), 
                        SB_CTL,DTOI(grids[current_image].line[2].r,
                            GRID_YINT_MIN,grids[current_image].line[2].dist) , TRUE);
                    if (!grids[current_image].line[2].hide) 
                        SendMessage(ImgDlg,WM_COMMAND,
                            IM_REDRAW_IMAGE,current_image);
                    break;
                case GRID_DIST_2_SC:
                    grids[current_image].line[2].dist=d;
                    sprintf(string,"%0.2f",grids[current_image].line[2].dist);
                    SetDlgItemText(hDlg, GRID_DIST_2, string);
                    SetScrollPos(GetDlgItem(hDlg, GRID_DIST_2_SC), 
                        SB_CTL,DTOI(grids[current_image].line[2].dist,
                        GRID_DIST_MIN,gendata[current_image]->size) , TRUE);
                    
                    if (!grids[current_image].line[2].hide) 
                        SendMessage(ImgDlg,WM_COMMAND,
                            IM_REDRAW_IMAGE,current_image);
                    break;
                    
            }
            out_smart=0;
        }
        break;
    case WM_KEYDOWN:
        switch(wParam)
        {
            
            case VK_RETURN:
                dlg_result=TRUE;
    MessageBox(hDlg,"Pressed enter","Warning",MB_OKCANCEL);            
                repaint_all(hDlg);
                break;
        }
        break;
    case WM_COMMAND:
        dlg_result=TRUE;
        switch (LOWORD(wParam)) {
            case GRID_TOGGLE:
                for(i=0;i<4;i++)
                {
                    for(j=0;j<GRID_MAX_LINES;j++)
                    {
                        grids[i].line[j].hide=1;
                    }
                }
                break;
            case GRID_SAVE:
                if (gendata[current_image]->valid &&
                    gendata[current_image]->type==DATATYPE_3D)
                {
                    strcpy(grids[current_image].scan_filename,
                        gendata[current_image]->filename);
                    strcpy(current_file_grd,
                        gendata[current_image]->filename);
                    temp_char=strrchr(current_file_grd,'.');
                    if (temp_char!=NULL)
                    {
                        *(temp_char+1)='\0';
                        strcat(current_file_grd,"grd");
                    }
                    else strcat(current_file_grd,".grd");
                    grd_data=&(grids[current_image]);
                    file_save_as(hDlg,hInst,FTYPE_GRD);
                }
                break;    
            case GRID_LOAD:
                if (gendata[current_image]->valid &&
                    gendata[current_image]->type==DATATYPE_3D)
                {
                    strcpy(current_file_grd,
                        gendata[current_image]->filename);
                    temp_char=strrchr(current_file_grd,'.');
                    if (temp_char!=NULL)
                    {
                        *(temp_char+1)='\0';
                        strcat(current_file_grd,"grd");
                    }
                    else strcat(current_file_grd,".grd");
                    grd_data=&(grids[current_image]);
                    file_open(hDlg,hInst,FTYPE_GRD,0,current_file_grd);
                    repaint_all(hDlg);
                    SendMessage(ImgDlg,WM_COMMAND,
                        IM_REDRAW_IMAGE,current_image);
                }
                break;    
                
            case GRID_MATCH_3:
                grid_match3(0,1,2);
                repaint_all(hDlg);
                if (!grids[current_image].line[2].hide)
                    SendMessage(ImgDlg,WM_COMMAND,
                        IM_REDRAW_IMAGE,current_image);
                break;
                
            case GRID_COPY_1:
                if (current_image!=0)
                {
                    copy_grid(&(grids[0]),&(grids[current_image]));
                    SendMessage(ImgDlg,WM_COMMAND,
                        IM_REDRAW_IMAGE,0);
                }
                break;
            case GRID_COPY_2:
                if (current_image!=1)
                {
                    copy_grid(&(grids[1]),&(grids[current_image]));
                    SendMessage(ImgDlg,WM_COMMAND,
                        IM_REDRAW_IMAGE,1);
                }
                break;
            case GRID_COPY_3:
                if (current_image!=2)
                {
                    copy_grid(&(grids[2]),&(grids[current_image]));
                    SendMessage(ImgDlg,WM_COMMAND,
                        IM_REDRAW_IMAGE,2);
                }
                break;
            case GRID_COPY_4:
                if (current_image!=3)
                {
                    copy_grid(&(grids[3]),&(grids[current_image]));
                    SendMessage(ImgDlg,WM_COMMAND,
                        IM_REDRAW_IMAGE,3);
                }
                break;
            case GRID_ANGLE_0:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,GRID_ANGLE_0,string,9);
                    temp_f=grids[current_image].line[0].theta=atof(string);
                    grids[current_image].line[0].theta=max(
                        min(grids[current_image].line[0].theta,GRID_ANGLE_MAX),
                        GRID_ANGLE_MIN);
                    if (grids[current_image].line[0].theta!=temp_f)
                    {
                        sprintf(string,"%0.2f",grids[current_image].line[0].theta);
                        SetDlgItemText(hDlg, GRID_ANGLE_0, string);
                    }

                    SetScrollPos(GetDlgItem(hDlg, GRID_ANGLE_0_SC), 
                        SB_CTL,DTOI(grids[current_image].line[0].theta,
                        GRID_ANGLE_MIN,GRID_ANGLE_MAX) , TRUE);
                    SendMessage(ImgDlg,WM_COMMAND,
                        IM_REDRAW_IMAGE,current_image);
                    out_smart=0;
                }
                break;
            case GRID_DIST_0:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,GRID_DIST_0,string,9);
                    temp_f=grids[current_image].line[0].dist=atof(string);
                    grids[current_image].line[0].dist=max(
                        min(grids[current_image].line[0].dist,gendata[current_image]->size),
                        GRID_DIST_MIN);
                    if (grids[current_image].line[0].dist!=temp_f)
                    {
                        sprintf(string,"%0.2f",grids[current_image].line[0].dist);
                        SetDlgItemText(hDlg, GRID_DIST_0, string);
                    }
                    SetScrollPos(GetDlgItem(hDlg, GRID_DIST_0_SC), 
                        SB_CTL,DTOI(grids[current_image].line[0].dist,
                        GRID_DIST_MIN,gendata[current_image]->size) , TRUE);
                    SendMessage(ImgDlg,WM_COMMAND,
                        IM_REDRAW_IMAGE,current_image);

                    out_smart=0;
                }
                break;
            case GRID_YINT_0:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,GRID_YINT_0,string,9);
                    temp_f=grids[current_image].line[0].r=atof(string);
                    grids[current_image].line[0].r=max(
                        min(grids[current_image].line[0].r,
                        grids[current_image].line[0].dist),
                        GRID_YINT_MIN);
                    if (grids[current_image].line[0].r!=temp_f)
                    {
                        sprintf(string,"%0.2f",grids[current_image].line[0].r);
                        SetDlgItemText(hDlg, GRID_YINT_0, string);
                    }
                    SetScrollPos(GetDlgItem(hDlg, GRID_YINT_0_SC), 
                        SB_CTL,DTOI(grids[current_image].line[0].r,
                            GRID_YINT_MIN,grids[current_image].line[0].dist) , TRUE);
                    SendMessage(ImgDlg,WM_COMMAND,
                        IM_REDRAW_IMAGE,current_image);

                    out_smart=0;
                }
                break;
            case GRID_NUM_LINES_0:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,GRID_NUM_LINES_0,string,9);
                    grid_num_lines[0]=atoi(string);
                    out_smart=0;
                }
                break;
            case GRID_GET_0:
                grid_get_line(0);
                repaint_all(hDlg);
                SendMessage(ImgDlg,WM_COMMAND,
                    IM_REDRAW_IMAGE,current_image);
                break;
                
            case GRID_GET_LINE:
                grid_get_clip_line();
                repaint_all(hDlg);
                break;
            case GRID_CLIP_TO_LINE:
                grids[current_image].clip=
                    IsDlgButtonChecked(hDlg,GRID_CLIP_TO_LINE);
                break;
            case GRID_CLIP_Y_LESS:
                grids[current_image].clip_y_less=
                    IsDlgButtonChecked(hDlg,GRID_CLIP_Y_LESS);
                break;
            case GRID_HIDE_0:
                grids[current_image].line[0].hide=
                    IsDlgButtonChecked(hDlg,GRID_HIDE_0);
                SendMessage(ImgDlg,WM_COMMAND,
                    IM_REDRAW_IMAGE,current_image);
                break;
            case GRID_LOCK_0:
                grids[current_image].line[0].lock=
                    IsDlgButtonChecked(hDlg,GRID_LOCK_0);
                break;
            case GRID_ANGLE_1:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,GRID_ANGLE_1,string,9);
                    temp_f=grids[current_image].line[1].theta=atof(string);
                    grids[current_image].line[1].theta=max(
                        min(grids[current_image].line[1].theta,GRID_ANGLE_MAX),
                        GRID_ANGLE_MIN);
                    if (grids[current_image].line[1].theta!=temp_f)
                    {
                        sprintf(string,"%0.2f",grids[current_image].line[1].theta);
                        SetDlgItemText(hDlg, GRID_ANGLE_1, string);
                    }

                    SetScrollPos(GetDlgItem(hDlg, GRID_ANGLE_1_SC), 
                        SB_CTL,DTOI(grids[current_image].line[1].theta,
                        GRID_ANGLE_MIN,GRID_ANGLE_MAX) , TRUE);
                    SendMessage(ImgDlg,WM_COMMAND,
                        IM_REDRAW_IMAGE,current_image);
                    out_smart=0;
                }
                break;
            case GRID_DIST_1:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,GRID_DIST_1,string,9);
                    temp_f=grids[current_image].line[1].dist=atof(string);
                    grids[current_image].line[1].dist=max(
                        min(grids[current_image].line[1].dist,gendata[current_image]->size),
                        GRID_DIST_MIN);
                    if (grids[current_image].line[1].dist!=temp_f)
                    {
                        sprintf(string,"%0.2f",grids[current_image].line[1].dist);
                        SetDlgItemText(hDlg, GRID_DIST_1, string);
                    }
                    SetScrollPos(GetDlgItem(hDlg, GRID_DIST_1_SC), 
                        SB_CTL,DTOI(grids[current_image].line[1].dist,
                        GRID_DIST_MIN,gendata[current_image]->size) , TRUE);
                    SendMessage(ImgDlg,WM_COMMAND,
                        IM_REDRAW_IMAGE,current_image);

                    out_smart=0;
                }
                break;
            case GRID_YINT_1:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,GRID_YINT_1,string,9);
                    temp_f=grids[current_image].line[1].r=atof(string);
                    grids[current_image].line[1].r=max(
                        min(grids[current_image].line[1].r,
                        grids[current_image].line[1].dist),
                        GRID_YINT_MIN);
                    if (grids[current_image].line[1].r!=temp_f)
                    {
                        sprintf(string,"%0.2f",grids[current_image].line[1].r);
                        SetDlgItemText(hDlg, GRID_YINT_1, string);
                    }
                    SetScrollPos(GetDlgItem(hDlg, GRID_YINT_1_SC), 
                        SB_CTL,DTOI(grids[current_image].line[1].r,
                            GRID_YINT_MIN,grids[current_image].line[1].dist) , TRUE);
                    SendMessage(ImgDlg,WM_COMMAND,
                        IM_REDRAW_IMAGE,current_image);

                    out_smart=0;
                }
                break;
            case GRID_NUM_LINES_1:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,GRID_NUM_LINES_1,string,9);
                    grid_num_lines[1]=atoi(string);
                    out_smart=0;
                }
                break;
            case GRID_GET_1:
                grid_get_line(1);
                repaint_all(hDlg);
                SendMessage(ImgDlg,WM_COMMAND,
                    IM_REDRAW_IMAGE,current_image);
                break;
                
            case GRID_HIDE_1:
                grids[current_image].line[1].hide=
                    IsDlgButtonChecked(hDlg,GRID_HIDE_1);
                SendMessage(ImgDlg,WM_COMMAND,
                    IM_REDRAW_IMAGE,current_image);
                break;
            case GRID_LOCK_1:
                grids[current_image].line[1].lock=
                    IsDlgButtonChecked(hDlg,GRID_LOCK_1);
                break;
            case GRID_ANGLE_2:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,GRID_ANGLE_2,string,9);
                    temp_f=grids[current_image].line[2].theta=atof(string);
                    grids[current_image].line[2].theta=max(
                        min(grids[current_image].line[2].theta,GRID_ANGLE_MAX),
                        GRID_ANGLE_MIN);
                    if (grids[current_image].line[2].theta!=temp_f)
                    {
                        sprintf(string,"%0.2f",grids[current_image].line[2].theta);
                        SetDlgItemText(hDlg, GRID_ANGLE_2, string);
                    }

                    SetScrollPos(GetDlgItem(hDlg, GRID_ANGLE_2_SC), 
                        SB_CTL,DTOI(grids[current_image].line[2].theta,
                        GRID_ANGLE_MIN,GRID_ANGLE_MAX) , TRUE);
                    SendMessage(ImgDlg,WM_COMMAND,
                        IM_REDRAW_IMAGE,current_image);
                    out_smart=0;
                }
                break;
            case GRID_DIST_2:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,GRID_DIST_2,string,9);
                    temp_f=grids[current_image].line[2].dist=atof(string);
                    grids[current_image].line[2].dist=max(
                        min(grids[current_image].line[2].dist,gendata[current_image]->size),
                        GRID_DIST_MIN);
                    if (grids[current_image].line[2].dist!=temp_f)
                    {
                        sprintf(string,"%0.2f",grids[current_image].line[2].dist);
                        SetDlgItemText(hDlg, GRID_DIST_2, string);
                    }
                    SetScrollPos(GetDlgItem(hDlg, GRID_DIST_2_SC), 
                        SB_CTL,DTOI(grids[current_image].line[2].dist,
                        GRID_DIST_MIN,gendata[current_image]->size) , TRUE);
                    SendMessage(ImgDlg,WM_COMMAND,
                        IM_REDRAW_IMAGE,current_image);

                    out_smart=0;
                }
                break;
            case GRID_YINT_2:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,GRID_YINT_2,string,9);
                    temp_f=grids[current_image].line[2].r=atof(string);
                    grids[current_image].line[2].r=max(
                        min(grids[current_image].line[2].r,
                        grids[current_image].line[2].dist),
                        GRID_YINT_MIN);
                    if (grids[current_image].line[2].r!=temp_f)
                    {
                        sprintf(string,"%0.2f",grids[current_image].line[2].r);
                        SetDlgItemText(hDlg, GRID_YINT_2, string);
                    }
                    SetScrollPos(GetDlgItem(hDlg, GRID_YINT_2_SC), 
                        SB_CTL,DTOI(grids[current_image].line[2].r,
                            GRID_YINT_MIN,grids[current_image].line[2].dist) , TRUE);
                    SendMessage(ImgDlg,WM_COMMAND,
                        IM_REDRAW_IMAGE,current_image);

                    out_smart=0;
                }
                break;
            case GRID_NUM_LINES_2:
                if (!out_smart)
                {
                    out_smart=1;
                    GetDlgItemText(hDlg,GRID_NUM_LINES_2,string,9);
                    grid_num_lines[2]=atoi(string);
                    out_smart=0;
                }
                break;
            case GRID_GET_2:
                grid_get_line(2);
                repaint_all(hDlg);
                SendMessage(ImgDlg,WM_COMMAND,
                    IM_REDRAW_IMAGE,current_image);
                break;
                
            case GRID_HIDE_2:
                grids[current_image].line[2].hide=
                    IsDlgButtonChecked(hDlg,GRID_HIDE_2);
                SendMessage(ImgDlg,WM_COMMAND,
                    IM_REDRAW_IMAGE,current_image);
                break;
            case GRID_LOCK_2:
                grids[current_image].line[2].lock=
                    IsDlgButtonChecked(hDlg,GRID_LOCK_2);
                break;
            case GRID_NEW_IMAGE:
                repaint_all(hDlg);
                break;
/*
            case ENTER:
    MessageBox(hDlg,"Pressed enter","Warning",MB_OKCANCEL);            
                repaint_all(hDlg);
                break;
*/
            case GRID_EXIT:
//                PostMessage(ImgDlg,WM_COMMAND,IM_VSCALE_EXIT,0);
          
                DestroyWindow(hDlg);
                GridWnd = NULL;
                GridOn = FALSE;
                break;
    }
    break;
  }
  
  return(dlg_result);
}

void copy_grid(GRID *dest,GRID *source)
{
    memcpy(dest,source,sizeof(GRID));
}

static void repaint_all(HWND hDlg)
{
        
    if (!gendata[current_image]->valid ||
        gendata[current_image]->type!=DATATYPE_3D)
    {
        enable_all(hDlg,0);
    }
    else
    {
        enable_all(hDlg,1);
        out_smart=1;
        sprintf(string,"%0.2f",grids[current_image].line[0].theta);
        SetDlgItemText(hDlg, GRID_ANGLE_0, string);
        SetScrollPos(GetDlgItem(hDlg, GRID_ANGLE_0_SC), 
            SB_CTL,DTOI(grids[current_image].line[0].theta,
                GRID_ANGLE_MIN,GRID_ANGLE_MAX) , TRUE);
        sprintf(string,"%0.2f",grids[current_image].line[0].r);
        SetDlgItemText(hDlg, GRID_YINT_0, string);
        SetScrollPos(GetDlgItem(hDlg, GRID_YINT_0_SC), 
            SB_CTL,DTOI(grids[current_image].line[0].r,
                GRID_YINT_MIN,grids[current_image].line[0].dist) , TRUE);
        sprintf(string,"%0.2f",grids[current_image].line[0].dist);
        SetDlgItemText(hDlg, GRID_DIST_0, string);
        SetScrollPos(GetDlgItem(hDlg, GRID_DIST_0_SC), 
            SB_CTL,DTOI(grids[current_image].line[0].dist,
                GRID_DIST_MIN,gendata[current_image]->size) , TRUE);
        CheckDlgButton(hDlg,GRID_HIDE_0,grids[current_image].line[0].hide);
        CheckDlgButton(hDlg,GRID_LOCK_0,grids[current_image].line[0].lock);
        sprintf(string,"%d",grid_num_lines[0]);
        SetDlgItemText(hDlg, GRID_NUM_LINES_0, string);
        sprintf(string,"%0.2f",grids[current_image].line[1].theta);
        SetDlgItemText(hDlg, GRID_ANGLE_1, string);
        SetScrollPos(GetDlgItem(hDlg, GRID_ANGLE_1_SC), 
            SB_CTL,DTOI(grids[current_image].line[1].theta,
                GRID_ANGLE_MIN,GRID_ANGLE_MAX) , TRUE);
        sprintf(string,"%0.2f",grids[current_image].line[1].r);
        SetDlgItemText(hDlg, GRID_YINT_1, string);
        SetScrollPos(GetDlgItem(hDlg, GRID_YINT_1_SC), 
            SB_CTL,DTOI(grids[current_image].line[1].r,
                GRID_YINT_MIN,grids[current_image].line[1].dist) , TRUE);
        sprintf(string,"%0.2f",grids[current_image].line[1].dist);
        SetDlgItemText(hDlg, GRID_DIST_1, string);
        SetScrollPos(GetDlgItem(hDlg, GRID_DIST_1_SC), 
            SB_CTL,DTOI(grids[current_image].line[1].dist,
                GRID_DIST_MIN,gendata[current_image]->size) , TRUE);
        CheckDlgButton(hDlg,GRID_HIDE_1,grids[current_image].line[1].hide);
        CheckDlgButton(hDlg,GRID_LOCK_1,grids[current_image].line[1].lock);
        sprintf(string,"%d",grid_num_lines[1]);
        SetDlgItemText(hDlg, GRID_NUM_LINES_1, string);
        sprintf(string,"%0.2f",grids[current_image].line[2].theta);
        SetDlgItemText(hDlg, GRID_ANGLE_2, string);
        SetScrollPos(GetDlgItem(hDlg, GRID_ANGLE_2_SC), 
            SB_CTL,DTOI(grids[current_image].line[2].theta,
                GRID_ANGLE_MIN,GRID_ANGLE_MAX) , TRUE);
        sprintf(string,"%0.2f",grids[current_image].line[2].r);
        SetDlgItemText(hDlg, GRID_YINT_2, string);
        SetScrollPos(GetDlgItem(hDlg, GRID_YINT_2_SC), 
            SB_CTL,DTOI(grids[current_image].line[2].r,
                GRID_YINT_MIN,grids[current_image].line[2].dist) , TRUE);
        sprintf(string,"%0.2f",grids[current_image].line[2].dist);
        SetDlgItemText(hDlg, GRID_DIST_2, string);
        SetScrollPos(GetDlgItem(hDlg, GRID_DIST_2_SC), 
            SB_CTL,DTOI(grids[current_image].line[2].dist,
                GRID_DIST_MIN,gendata[current_image]->size) , TRUE);
        CheckDlgButton(hDlg,GRID_HIDE_2,grids[current_image].line[2].hide);
        CheckDlgButton(hDlg,GRID_LOCK_2,grids[current_image].line[2].lock);
        sprintf(string,"%d",grid_num_lines[2]);
        SetDlgItemText(hDlg, GRID_NUM_LINES_2, string);
        CheckDlgButton(hDlg,GRID_CLIP_TO_LINE,grids[current_image].clip);
        CheckDlgButton(hDlg,GRID_CLIP_Y_LESS,grids[current_image].clip_y_less);
        sprintf(string,"%d",grids[current_image].x[0]);
        SetDlgItemText(hDlg, GRID_LINE_X1, string);
        sprintf(string,"%d",grids[current_image].x[1]);
        SetDlgItemText(hDlg, GRID_LINE_X2, string);
        sprintf(string,"%d",grids[current_image].y[0]);
        SetDlgItemText(hDlg, GRID_LINE_Y1, string);
        sprintf(string,"%d",grids[current_image].y[1]);
        SetDlgItemText(hDlg, GRID_LINE_Y2, string);
    
            
        out_smart=0;
    }
}

void init_grids()
{
    int i,j;
    
    for(i=0;i<4;i++)
    {
        grids[i].version=3;
        grids[i].scan_filename[0]='\0';
        grids[i].x[0]=grids[i].x[1]=grids[i].y[0]=grids[i].y[1]=0;
        grids[i].clip=grids[i].clip_y_less=0;
        grids[i].clip_num_pts=2;
        grids[i].clip_type=GRID_CLIP_TYPE_LINE;
        for(j=0;j<GRID_MAX_LINES;j++)
        {
            grids[i].line[j].hide=1;
            grids[i].line[j].lock=0;
            grids[i].line[j].dist=GRID_INIT_DIST;
            grids[i].line[j].theta=0;
            grids[i].line[j].r=1;
        }
    }
}

static void enable_all(HWND hDlg,int status)
{
/*
    EnableWindow(GetDlgItem(hDlg, GRID_), status);
    EnableWindow(GetDlgItem(hDlg, GRID_), status);
    EnableWindow(GetDlgItem(hDlg, GRID_), status);
    EnableWindow(GetDlgItem(hDlg, GRID_), status);
    EnableWindow(GetDlgItem(hDlg, GRID_), status);
    EnableWindow(GetDlgItem(hDlg, GRID_), status);
    EnableWindow(GetDlgItem(hDlg, GRID_), status);
    EnableWindow(GetDlgItem(hDlg, GRID_), status);
    EnableWindow(GetDlgItem(hDlg, GRID_), status);
    EnableWindow(GetDlgItem(hDlg, GRID_), status);
    EnableWindow(GetDlgItem(hDlg, GRID_), status);
    EnableWindow(GetDlgItem(hDlg, GRID_), status);
    EnableWindow(GetDlgItem(hDlg, GRID_), status);
    EnableWindow(GetDlgItem(hDlg, GRID_), status);
    EnableWindow(GetDlgItem(hDlg, GRID_), status);
    EnableWindow(GetDlgItem(hDlg, GRID_), status);
    EnableWindow(GetDlgItem(hDlg, GRID_), status);
    EnableWindow(GetDlgItem(hDlg, GRID_), status);
    EnableWindow(GetDlgItem(hDlg, GRID_), status);
    EnableWindow(GetDlgItem(hDlg, GRID_), status);
    EnableWindow(GetDlgItem(hDlg, GRID_), status);
    EnableWindow(GetDlgItem(hDlg, GRID_), status);
    EnableWindow(GetDlgItem(hDlg, GRID_), status);
    EnableWindow(GetDlgItem(hDlg, GRID_), status);
    EnableWindow(GetDlgItem(hDlg, GRID_), status);
    EnableWindow(GetDlgItem(hDlg, GRID_), status);
    EnableWindow(GetDlgItem(hDlg, GRID_), status);
*/
}

void grid_get_line(int line)
{
    float theta1,theta2,r;
    
    if (!SelOn || !sel1_on) return;
    if (sel_r1.type!=SEL_LINE || (sel_r1.pt1->x== sel_r1.pt2->x &&
        sel_r1.pt1->y == sel_r1.pt2->y )) return;
    if (sel_r1.pt2->x==sel_r1.pt1->x) theta1=-PI/2;
    else 
        theta1=atan(((float)(sel_r1.pt2->y-sel_r1.pt1->y))/((float)(sel_r1.pt2->x-
            sel_r1.pt1->x)));
    grids[current_image].line[line].theta=DEGREE(theta1);
    if (sel2_on && sel_r2.image==current_image && (sel_r2.type==SEL_LINE || 
        sel_r2.type==SEL_POINTS) && grid_num_lines[line])
    {
        if (sel_r2.type==SEL_POINTS || (sel_r2.pt1->x==sel_r2.pt2->x &&
            sel_r2.pt1->y==sel_r2.pt2->y))
        {
            grids[current_image].line[line].dist=
                point_line_dist(sel_r1.pt1->x,sel_r1.pt1->y,sel_r1.pt2->x,
                sel_r1.pt2->y,sel_r2.pt1->x,sel_r2.pt1->y)/grid_num_lines[line];
        }
        else
        {
            if (sel_r2.pt2->x==sel_r2.pt1->x) theta2=-PI/2;
            else 
                theta2=atan(((float)(sel_r2.pt2->y-sel_r2.pt1->y))/
                    ((float)(sel_r2.pt2->x-sel_r2.pt1->x)));
            theta1=(theta1+theta2)/2;
            grids[current_image].line[line].theta=DEGREE(theta1);
            grids[current_image].line[line].dist=
                (point_line_dist(sel_r1.pt1->x,sel_r1.pt1->y,sel_r1.pt2->x,
                sel_r1.pt2->y,sel_r2.pt1->x,sel_r2.pt1->y)+
                point_line_dist(sel_r1.pt1->x,sel_r1.pt1->y,sel_r1.pt2->x,
                sel_r1.pt2->y,sel_r2.pt2->x,sel_r2.pt2->y))/2/
                grid_num_lines[line];
        }
    }
    r=point_line_dist(sel_r1.pt1->x,sel_r1.pt1->y,sel_r1.pt2->x,
        sel_r1.pt2->y,0,0);
    r=fmod(r,grids[current_image].line[line].dist);
    if (theta1>0 || sel_r1.pt1->x!=sel_r1.pt2->x) 
        if (sel_r1.pt1->y<((float)(sel_r1.pt2->y-sel_r1.pt1->y))/
            ((float)(sel_r1.pt2->x-sel_r1.pt1->x))*sel_r1.pt1->x)
        
            r=grids[current_image].line[line].dist-r;
    grids[current_image].line[line].r=r;
}    

void grid_get_clip_line()
{
    
    if (!SelOn || !sel1_on) return;
    if (sel_r1.type!=SEL_LINE || (sel_r1.pt1->x== sel_r1.pt2->x &&
        sel_r1.pt1->y == sel_r1.pt2->y )) return;
    grids[current_image].x[0]=sel_r1.pt1->x;
    grids[current_image].y[0]=sel_r1.pt1->y;
    grids[current_image].x[1]=sel_r1.pt2->x;
    grids[current_image].y[1]=sel_r1.pt2->y;
}    


void grid_match3(int one,int two, int three)
{
    float v1,v2,vx,vy,phi,v1x,v1y,v2x,v2y,l2,x3,y3,r;
    
    v1x=cos(RADIAN(grids[current_image].line[one].theta));
    v1y=sin(RADIAN(grids[current_image].line[one].theta));
    v2x=cos(RADIAN(grids[current_image].line[two].theta));
    v2y=sin(RADIAN(grids[current_image].line[two].theta));
    
    phi=acos(v1x*v2x+v1y*v2y);
    v1=grids[current_image].line[two].dist/sin(phi);
    v2=grids[current_image].line[one].dist/sin(phi);
    v1x*=v1;
    v2x*=v2;
    v1y*=v1;
    v2y*=v2;
    if ((v1x-v2x)*(v1x-v2x)+(v1y-v2y)*(v1y-v2y)<
        (v1x+v2x)*(v1x+v2x)+(v1y+v2y)*(v1y+v2y))
    {
        vx=v1x-v2x;
        vy=v1y-v2y;
    }
    else
    {
        vx=v1x+v2x;
        vy=v1y+v2y;
    }
    if (fabs(vx)<GRID_ZERO_TOLERANCE) 
        grids[current_image].line[three].theta=90;
    else grids[current_image].line[three].theta=DEGREE(atan(vy/vx));
    grids[current_image].line[three].dist=point_line_dist(0,0,
        vx,vy,v1x,v1y);
    if (fabs(v1x*v2y-v1y*v2x)<GRID_ZERO_TOLERANCE) return;
    l2=((v1y*(v2x-v1x)-v1x*(v2y-v1y))/(v1x*v2y-v1y*v2x));
    v1x=cos(RADIAN(grids[current_image].line[one].theta));
    v1y=sin(RADIAN(grids[current_image].line[one].theta));
    x3=v1x*l2;
    y3=v1y*l2;
    r=point_line_dist(x3,y3,x3+vx,y3+vy,0,0);
    r=fmod(r,grids[current_image].line[three].dist);
    if (fabs(grids[current_image].line[three].theta)-90>GRID_ZERO_TOLERANCE
        && fabs(vx)>GRID_ZERO_TOLERANCE) 
        if (y3<vy/vx*x3)
            r=grids[current_image].line[three].dist-r;
    
/*
    if (grids[current_image].line[three].theta<0 || x3!=x3+vx) 
        if (y3<((float)(vy))/
            ((float)(vx))*x3)
                r=grids[current_image].line[three].dist-r;
*/    
    grids[current_image].line[three].r=r;
}

int grid_hidden(int this_image)
{
    int i,result=1;
    
    for(i=0;i<GRID_MAX_LINES;i++) result*=grids[this_image].line[i].hide;
    
    return(result);
}
    
