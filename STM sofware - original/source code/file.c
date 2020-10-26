#define INCLUDE_COMMDLG_H
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <malloc.h>
#include <time.h>
#include <direct.h>
#include "stm.h"
#include "data.h"
#include "dio.h"
#include "file.h"
#include "pal.h"
#include "image.h"
#include "masspec.h"
#include "spec.h"
#include "count.h"
#include "zlib\\zlib.h"
#include "print.h"
#include "hop.h"
#include "vibmap.h"

#define fwrite1(A) fwrite(&(A),sizeof(A),1,fp)
#define fread1(A) fread(&(A),sizeof(A),1,fp)

datadef **glob_data;

extern char string[];
/*
extern datadef *data;
*/
extern int device_res;
extern int output_color;
extern int print_save_links_as_added;
extern COUNT_DATA *glob_count_data;
extern unsigned int bit16;

//extern float *hop_timer_array;
extern int z_offset_ch;

#ifdef OLD_HOP_TIMING
extern long *time_hop;
#else
extern float *time_hop;
#endif

extern int sp_scan_count;
extern OFFSET_PT sp_offsets_map[];

extern GRID *grd_data;
extern unsigned int ch;
extern unsigned int ch_data;
extern unsigned int dac_data[16];
extern unsigned int dac_ranges;
extern unsigned int out_range;
extern unsigned int input_ch;
extern unsigned int out1;
extern unsigned int tip_accel;
extern unsigned int num_steps;
extern double coarse_x;
extern double coarse_y;
extern unsigned int wave;
extern unsigned int scan_x,scan_y,scan_z;
extern unsigned int scan_step;
extern unsigned int scan_size;
#ifdef OLD
extern unsigned int scan_num;
#endif
extern float scan_x_gain;
extern float scan_y_gain;
extern float scan_z_gain;
extern unsigned int scan_scale;
extern unsigned int scan_freq;
unsigned int image_size;
int image_x_offset;
int image_y_offset;
#ifdef OLD
extern BOOL saved;
#endif
extern unsigned int sample_bias;
extern unsigned int tip_gain;
extern unsigned int i_setpoint;
extern unsigned int tip_number;
extern unsigned int tip_delay;
extern unsigned int tip_x_step;
extern unsigned int tip_zo_step;
extern PRINT_EL *print_list;

char *current_dir_stm;
char *current_dir_ps;
char *current_dir_stp;
char *current_dir_cut;
char *current_dir_mas;
char *current_dir_spc;
char *current_dir_pal;
char *current_dir_gif;
char *initial_dir_stm;
char *initial_dir_ps;
char *initial_dir_stp;
char *initial_dir_cut;
char *current_dir_dep;
char *current_dir_grd;
char *current_dir_fig;
char *current_dir_cnt;
char *current_dir_exp;
char *current_dir_hop;
char *current_dir_map;

char *initial_dir_dep;
char *initial_dir_mas;
char *initial_dir_spc;
char *initial_dir_pal;
char *initial_dir_gif;
char *initial_dir_grd;
char *initial_dir_cnt;
char *initial_dir_fig;
char *initial_dir_exp;
char *initial_dir_trc;
char *initial_dir_hop;
char *initial_dir_map;

char *current_file_hop;
char *current_file_stm;
char *current_file_ps;
char *current_file_stp;
char *current_file_cut;
char *current_file_mas;
char *current_file_spc;
char *current_file_pal;
char *current_file_gif;
char *current_file_dep;
char *current_file_grd;
char *current_file_cnt;
char *current_file_fig;
char *current_file_cnt;
char *current_file_exp;
char *current_file_map;

char *rel_path_return, *initial_working_dir;

int file_stm_count=1,file_mas_count=1,file_spc_count=1,file_dep_count=1,
    file_cut_count=1;
extern char *current_file;
FILE_LIST *file_list=NULL;

#ifdef DEBUG
int total_files=0;
FILE_LIST *temp_list[500];
#endif

int valid_filename(char *);

double coarse_z;	// for compatibility with old init files

int load_image_old(char *);
static void sequence_to_scan(int);
int load_image_fp(FILE *);
void save_image_fp(FILE *, int);
static void save_3d_as_text( char *);
static void load_gen2d_fp(FILE *);
static void save_gen2d_fp(FILE *, int);
void load_cut_fp(FILE *);
void save_cut_fp(FILE *, int);
void load_grd(char *);
void load_grd_fp(FILE *);
void save_grd(char *);
void save_grd_fp(FILE *);
void load_pal_fp(FILE *);
void load_old_pal(char *);
void save_pal_fp(FILE *fp);
void load_map(HWND, char *);
void save_map(char *);
void load_cnt(char *);
void load_cnt_fp(FILE *);
void save_cnt(char *);
void save_cnt_fp(FILE *);
void save_cnt_as_txt(char *);

static void GetIndexChrs(char *, char *, char *);
void find_list(char *);
void find_list_prev(char *);
void insert_list(char *);
void nuke_list();

void load_fig(char *);
void save_fig(char *);
void write_bar_attrib(PRINT_EL *, FILE *);
void write_print_image_attrib(PRINT_EL *, FILE *);
void read_print_bar_attrib(PRINT_EL *, int, FILE *);
void read_print_image_attrib(PRINT_EL *, int, FILE *);
void write_print_text(PRINT_EL *, FILE *);
void read_print_text(PRINT_EL *, int, FILE *);
void write_print_line(PRINT_EL *, FILE *);
void read_print_line(PRINT_EL *, int, FILE *);

void force_ext(char *fname,int file_type)
{
    char *slash;
    char *dot;
    
    slash=strrchr(fname,'\\');
    if (slash==NULL) slash=fname;
    dot=strrchr(slash,'.');
    
    if (dot==NULL) switch (file_type)
    {
        case FTYPE_CUT:
            strcat(fname,".cut");
            break;
        case FTYPE_STM:
        case FTYPE_STM_UNCOMPRESSED:
            strcat(fname,".stm");
            break;
        case FTYPE_STP:
            strcat(fname,".stp");
            break;
        case FTYPE_PS:
            strcat(fname,".ps");
            break;
        case FTYPE_MAS:
            strcat(fname,".mas");
            break;
        case FTYPE_SPC:
            strcat(fname,".spc");
            break;
        case FTYPE_PAL:
            strcat(fname,".pal");
            break;
        case FTYPE_GIF:
            strcat(fname,".gif");
            break;
        case FTYPE_DEP:
            strcat(fname,".dep");
            break;
        case FTYPE_GRD:
            strcat(fname,".grd");
            break;
        case FTYPE_CNT:
            strcat(fname,".cnt");
            break;
        case FTYPE_GEN2D_EXPORT:
            strcat(fname,".exp");
            break;
        case FTYPE_FIG:
            strcat(fname,".fig");
	    break;
	case FTYPE_MAP:
	    strcat(fname, ".vmp");
            break;
	case FTYPE_HOP_DATA:
	    strcat(fname, ".hop");
            break;
    }
}    


int str2int(char *str)
{
    int count;              
    str[0]=tolower(str[0]);
    str[1]=tolower(str[1]);
    
    if (str[0]<='9' && str[1] <='9')
    {
        sscanf(str,"%d",&count);
        return(count);
    }
    count=100+((int)str[0]-(int)'a')*36;
    if (str[1]>'9')
    {
        count+=str[1]-'a'+10;
    }
    else
    {
        count+=str[1]-'0';
    }
/*
sprintf(string,"%s %d",str,count);
mprintf(string);
*/
    return(count);    
}

void int2str(char *str,int count)
{
    if (count<10)
    {
        str[0]='0';
        str[1]='0'+count;
    }
    else if (count<100)
    {
        str[0]='0'+count/10;
        str[1]='0'+count%10;
    }
    else
    {
        str[0]='a'+(count-100)/36;
        if ((count-100)%36>9)
        {
            str[1]='a'+((count-100)%36-10);
        }
        else str[1]='0'+((count-100)%36);
    }
/*        
    else if (count<360)
    {
        str[0]='0'+(count-100)/26;
        str[1]='a'+(count-100)%26;
    }
    else if (count<620)
    {
        str[0]='a'+(count-360)/10;
        str[1]='0'+(count-360)%10;
    }
    else
    {
        str[0]='a'+(count-620)/26;
        str[1]='a'+(count-620)%26;
    }
*/
}

void init_file(char *dir,char *filename,char *ext,int *count)
{
    time_t now;
    struct tm *now_struct;
//    struct find_t fileinfo;
	HANDLE current_file = NULL;
	WIN32_FIND_DATA lpffd;
    char path[CURRENT_DIR_MAX];
    int done=0;
    int year;


    time(&now);
    now_struct=localtime(&now);

    int2str(filename,now_struct->tm_mon+1);
    int2str(filename+2,now_struct->tm_mday);

    // the year is the number of years since 1900. we only want last two digits.
    year = now_struct->tm_year;
    if(year > 99) year = year % 100;
    int2str(filename+4,year);

/*
    int2str(filename+4,now_struct->tm_year);
*/

/*        
        sprintf(string,"%d",now_struct->tm_year);
        MessageBox(GetFocus(),string,"Year",MB_OKCANCEL);
*/
    while(!done)
    {
        int2str(filename+6,*count);
        filename[8]='\0';
        strcat(filename,".");
        strcat(filename,ext);
        strcpy(path,dir);
        strcat(path,"\\");
        strcat(path,filename);
//        if (!_dos_findfirst(path,_A_NORMAL, &fileinfo))
		current_file = FindFirstFile(path, &lpffd);
		if(current_file != 0xFFFFFFFF) {
			FindClose(current_file);
			(*count)++;
		}
		else done=1;
//        _dos_findclose(&fileinfo);
    }
}

void inc_file(char *filename,int *count)
{
    if (valid_filename(filename)) 
    {
        (*count)++;
        int2str(filename+6,*count);
    }
}

void change_dir(char *fname, int file_type)
{
    char *temp;
    
    temp=strrchr(fname,'\\');
    if (temp!=NULL)
    {
        *temp='\0';
        switch (file_type)
        {
            case FTYPE_CUT:
                strcpy(current_dir_cut,fname);
                break;
            case FTYPE_STM:
            case FTYPE_STM_UNCOMPRESSED:
                strcpy(current_dir_stm,fname);
                break;
            case FTYPE_STP:
                strcpy(current_dir_stp,fname);
                break;
            case FTYPE_PS:
                strcpy(current_dir_ps,fname);
                break;
            case FTYPE_MAS:
                strcpy(current_dir_mas,fname);
                break;
            case FTYPE_SPC:
                strcpy(current_dir_spc,fname);
                break;
            case FTYPE_PAL:
                strcpy(current_dir_pal,fname);
                break;
            case FTYPE_GIF:
                strcpy(current_dir_gif,fname);
                break;
            case FTYPE_DEP:
                strcpy(current_dir_dep,fname);
                break;
            case FTYPE_GRD:
                strcpy(current_dir_grd,fname);
                break;
            case FTYPE_CNT:
                strcpy(current_dir_cnt,fname);
                break;
            case FTYPE_GEN2D_EXPORT:
                strcpy(current_dir_exp,fname);
                break;
            case FTYPE_FIG:
                strcpy(current_dir_fig,fname);
                break;
        }
        *temp='\\';
    }
}
 
BOOL file_open(HWND hWnd,HANDLE hInstance,int file_type,int allow_all,char *this_file)
{
  OPENFILENAME ofn;
  char szDirName[256];
  char szFile[256], szFileTitle[256];
  int cbString;
  char chReplace;
  char szFilter[256];

  int i;

  szFile[0] = '\0';
//sprintf(string,"%d",file_type);
//mprintf(string);
    if (!this_file) this_file=current_file_stm;
  switch(file_type)
  {
    case FTYPE_CUT:
      strcpy(szDirName,current_dir_cut);
      strcpy(szFile,this_file);
      cbString=LoadString(hInstance,DATA_CUT_FILTERSTRING,szFilter,sizeof(szFilter));
      break;
    case FTYPE_STM:
      strcpy(szDirName,current_dir_stm);
      strcpy(szFile,this_file);
      cbString=LoadString(hInstance,DATA_STM_FILTERSTRING,szFilter,sizeof(szFilter));
      break;
    case FTYPE_STP:
      strcpy(szDirName,current_dir_stp);
      strcpy(szFile,this_file);
      cbString=LoadString(hInstance,DATA_STP_FILTERSTRING,szFilter,sizeof(szFilter));
      break;
    case FTYPE_DEP:
      strcpy(szDirName,current_dir_dep);
      strcpy(szFile,this_file);
      cbString=LoadString(hInstance,DATA_DEP_FILTERSTRING,szFilter,sizeof(szFilter));
      break;
    case FTYPE_GRD:
      strcpy(szDirName,current_dir_grd);
      strcpy(szFile,this_file);
      cbString=LoadString(hInstance,GRD_FILTERSTRING,szFilter,sizeof(szFilter));
      break;
    case FTYPE_CNT:
      strcpy(szDirName,current_dir_cnt);
      strcpy(szFile,this_file);
      cbString=LoadString(hInstance,CNT_FILTERSTRING,szFilter,sizeof(szFilter));
      break;
    case FTYPE_GEN2D_EXPORT:
      strcpy(szDirName,current_dir_exp);
      strcpy(szFile,this_file);
      cbString=LoadString(hInstance,EXP_FILTERSTRING,szFilter,sizeof(szFilter));
      break;
    case FTYPE_FIG:
      strcpy(szDirName,current_dir_fig);
      strcpy(szFile,this_file);
      cbString=LoadString(hInstance,FIG_FILTERSTRING,szFilter,sizeof(szFilter));
//mprintf("loading fig");      
      break;
    case FTYPE_PAL:
      strcpy(szDirName,current_dir_pal);
      strcpy(szFile,this_file);
      cbString=LoadString(hInstance,PAL_FILTERSTRING,szFilter,sizeof(szFilter));
      break;
    case FTYPE_MAS:
      strcpy(szDirName,current_dir_mas);
      strcpy(szFile,this_file);
      cbString=LoadString(hInstance,DATA_MAS_FILTERSTRING,szFilter,sizeof(szFilter));
      break;
    case FTYPE_SPC:
      strcpy(szDirName,current_dir_spc);
      strcpy(szFile,this_file);
      cbString=LoadString(hInstance,DATA_SPC_FILTERSTRING,szFilter,sizeof(szFilter));
      break;
    case FTYPE_MAP:
      strcpy(szDirName,current_dir_map);
      strcpy(szFile,this_file);
      cbString=LoadString(hInstance,MAP_FILTERSTRING,szFilter,sizeof(szFilter));
/*
    case FTYPE_GIF:
      cbString=LoadString(hInstance,GIF_FILTERSTRING,szFilter,sizeof(szFilter));
      break;
*/
  }
  chReplace = szFilter[cbString-1];
  for(i=0;szFilter[i]!='\0';i++)
    if(szFilter[i]==chReplace)
      szFilter[i] = '\0';
  memset(&ofn,0,sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hWnd;
  ofn.lpstrFilter = szFilter;
  ofn.nFilterIndex = 1;
  ofn.lpstrFile = szFile;
  ofn.nMaxFile = sizeof(szFile);
/*
   ofn.lpstrFile = (char *) malloc(sizeof(char)*256);
  ofn.nMaxFile = sizeof(char)*256;
*/
   ofn.lpstrFileTitle = szFileTitle;
  ofn.nMaxFileTitle = sizeof(szFileTitle);
  ofn.lpstrInitialDir = szDirName;
  ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;
//mprintf(szFile);  
  if(GetOpenFileName(&ofn)) {
    char tmpfile[500];
//      MessageBox(hWnd,szFile,"Test Title",MB_OKCANCEL|MB_ICONEXCLAMATION); 
    strcpy(tmpfile,szFile);
    force_ext(tmpfile,file_type);
    switch(file_type)
    {
        case FTYPE_STM:
        case FTYPE_STP:
        case FTYPE_CUT:
        case FTYPE_DEP:
        case FTYPE_MAS:
        case FTYPE_SPC:
            smart_load(tmpfile,file_type,allow_all);
            break;
        case FTYPE_GRD:
            load_grd(tmpfile);
            change_dir(tmpfile,file_type);
            break;
        case FTYPE_CNT:
            load_cnt(tmpfile);
            change_dir(tmpfile,file_type);
            break;
        case FTYPE_FIG:
            load_fig(tmpfile);
            change_dir(tmpfile,file_type);
            break;
        case FTYPE_PAL:
            load_pal(tmpfile);
            change_dir(tmpfile,file_type);
            break;
	case FTYPE_MAP:
	    load_map(hWnd, tmpfile);
    }
    return(TRUE);
  }
  else
    return(FALSE);
}
int guess_file_type(char *filename)
{
    int real_file_type;
    char *ext;
    
    real_file_type=FTYPE_UNKNOWN;
    
    ext=strrchr(filename,'.');
    if (ext==NULL) return FTYPE_UNKNOWN;
    ext+=1;
    strupr(ext);    
    if (!strcmp(ext,"STM"))
    {
        real_file_type=FTYPE_STM;
    }
    if (!strcmp(ext,"STP"))
    {
        real_file_type=FTYPE_STP;
    }
    if (!strcmp(ext,"CUT"))
    {
        real_file_type=FTYPE_CUT;
    }
    else if (!strcmp(ext,"DEP"))
    {
        real_file_type=FTYPE_DEP;
    }
    else if (!strcmp(ext,"SPC"))
    {
        real_file_type=FTYPE_SPC;
    }
    else if (!strcmp(ext,"MAS"))
    {
        real_file_type=FTYPE_MAS;
    }
    return real_file_type;
}

void smart_load(char *filename,int file_type,int allow_all)
{
    int real_file_type;
    
    
    real_file_type=guess_file_type(filename);
    if (real_file_type==FTYPE_UNKNOWN) return;
    if (real_file_type!=file_type && !allow_all) return;
    if (real_file_type!=file_type || (*glob_data)->type!=DATATYPE_3D)
    {
        free_data(glob_data);
        switch(real_file_type)
        {
            case FTYPE_STM:
                alloc_data(glob_data,DATATYPE_3D,DATA_MAX,GEN2D_NONE,GEN2D_NONE,1);
                break;
            case FTYPE_STP:
                alloc_data(glob_data,DATATYPE_3D_PARAMETERS,0,GEN2D_NONE,GEN2D_NONE,1);
                break;
            case FTYPE_CUT:
                alloc_data(glob_data,DATATYPE_GEN2D,1,GEN2D_NONE,GEN2D_NONE,0);
                break;
            case FTYPE_DEP:                                                     
                alloc_data(glob_data,DATATYPE_2D,1,GEN2D_NONE,GEN2D_NONE,0);
                break;
            case FTYPE_SPC:
                //alloc_data(glob_data,DATATYPE_SPEC_I,1,GEN2D_NONE,GEN2D_NONE,0);  // old way
				alloc_data(glob_data,DATATYPE_GEN2D,1,GEN2D_NONE,GEN2D_NONE,0);
                break;
            case FTYPE_MAS:
                alloc_data(glob_data,DATATYPE_MASS,SP_NUM_CH,GEN2D_NONE,GEN2D_NONE,0);
                break;
        }
    }
    else
    {
        destroy_logpal(&((*glob_data)->pal.logpal));
        (*glob_data)->pal.type=PALTYPE_MAPPED;
        
    }
    switch(real_file_type)
    {
        case FTYPE_CUT:
            load_cut(filename);
            break;
        case FTYPE_STM:
            load_image(filename);
            break;
        case FTYPE_STP:
            load_image(filename);
            break;
        case FTYPE_DEP:
            load_dep(filename);
            break;
        case FTYPE_MAS:
            load_mas(filename);
            break;
        case FTYPE_SPC:
            load_mas(filename);
            break;
    }
    strcpy(current_file,filename);
    change_dir(filename,real_file_type);
}    


int file_save_as(HWND hWnd,HANDLE hInstance,int file_type)
{
// pushkin
  OPENFILENAME ofn;
  char szDirName[256];
  char szFile[256], szFileTitle[256];
  int cbString;
  char chReplace;
  char szFilter[256];
  char *temp;
  int i;
  int result=0;

  szFile[0] = '\0';
  switch(file_type)
  {
    case FTYPE_STM:
    case FTYPE_STM_UNCOMPRESSED:
      strcpy(szDirName,current_dir_stm);
      strcpy(szFile,current_file_stm);
      cbString=LoadString(hInstance,IDS_FILTERSTRING,szFilter,sizeof(szFilter));
      break;
    case FTYPE_STP:
      strcpy(szDirName,current_dir_stp);
      strcpy(szFile,current_file_stp);
      cbString=LoadString(hInstance,STP_FILTERSTRING,szFilter,sizeof(szFilter));
      break;
    case FTYPE_PS:
      strcpy(szDirName,current_dir_ps);
      strcpy(szFile,current_file_ps);
      cbString=LoadString(hInstance,PS_FILTERSTRING,szFilter,sizeof(szFilter));
      break;
    case FTYPE_CUT:
      strcpy(szDirName,current_dir_cut);
      strcpy(szFile,current_file_cut);
      cbString=LoadString(hInstance,DATA_CUT_FILTERSTRING,szFilter,sizeof(szFilter));
      break;
    case FTYPE_DEP:
      strcpy(szDirName,current_dir_dep);
      strcpy(szFile,current_file_dep);
      cbString=LoadString(hInstance,DEP_FILTERSTRING,szFilter,sizeof(szFilter));
      break;
    case FTYPE_GRD:
      strcpy(szDirName,current_dir_grd);
      strcpy(szFile,current_file_grd);
      cbString=LoadString(hInstance,GRD_FILTERSTRING,szFilter,sizeof(szFilter));
      break;
    case FTYPE_MAP:
      strcpy(szDirName,current_dir_map);
      strcpy(szFile,current_file_map);
      cbString=LoadString(hInstance,MAP_FILTERSTRING,szFilter,sizeof(szFilter));
      break;
    case FTYPE_CNT:
      strcpy(szDirName,current_dir_cnt);
      strcpy(szFile,current_file_cnt);
      cbString=LoadString(hInstance,CNT_FILTERSTRING,szFilter,sizeof(szFilter));
      break;
    case FTYPE_CNT_AS_TXT:
      strcpy(szDirName,current_dir_exp);
      strcpy(szFile,current_file_cnt);
      cbString=LoadString(hInstance,CXP_FILTERSTRING,szFilter,sizeof(szFilter));
      break;
    case FTYPE_GEN2D_EXPORT:
      strcpy(szDirName,current_dir_exp);
      strcpy(szFile,current_file_exp);
      cbString=LoadString(hInstance,EXP_FILTERSTRING,szFilter,sizeof(szFilter));
      break;
    case FTYPE_HOP_DATA:
      strcpy(szDirName,current_dir_hop);
      strcpy(szFile,current_file_hop);
      cbString=LoadString(hInstance,HOP_FILTERSTRING,szFilter,sizeof(szFilter));
      break;
    case FTYPE_FIG:
      strcpy(szDirName,current_dir_fig);
      strcpy(szFile,current_file_fig);
      cbString=LoadString(hInstance,FIG_FILTERSTRING,szFilter,sizeof(szFilter));
      break;
    case FTYPE_PAL:
      strcpy(szDirName,current_dir_pal);
      strcpy(szFile,current_file_pal);
      cbString=LoadString(hInstance,PAL_FILTERSTRING,szFilter,sizeof(szFilter));
      break;
    case FTYPE_MAS:
      strcpy(szDirName,current_dir_mas);
      strcpy(szFile,current_file_mas);
      cbString=LoadString(hInstance,MAS_FILTERSTRING,szFilter,sizeof(szFilter));
      break;
    case FTYPE_SPC:
      strcpy(szDirName,current_dir_spc);
      strcpy(szFile,current_file_spc);
      cbString=LoadString(hInstance,SPC_FILTERSTRING,szFilter,sizeof(szFilter));
      break;
    case FTYPE_3D2TEXT:
      strcpy( szDirName, current_dir_exp);
      strcpy( szFile, current_file_exp);
      cbString = LoadString( hInstance, EXP_FILTERSTRING, szFilter, sizeof(szFilter));
      break;
    case FTYPE_GIF:
      strcpy(szDirName,current_dir_gif);
      strcpy(szFile,current_file_gif);
      cbString=LoadString(hInstance,GIF_FILTERSTRING,szFilter,sizeof(szFilter));
      break;
  }
   chReplace = szFilter[cbString-1];
  for(i=0;szFilter[i]!='\0';i++)
    if(szFilter[i]==chReplace)
      szFilter[i] = '\0';
  memset(&ofn,0,sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hWnd;
  ofn.lpstrFilter = szFilter;
  ofn.nFilterIndex = 1;
  ofn.lpstrFile = szFile;
  ofn.nMaxFile = sizeof(szFile);
  ofn.lpstrFileTitle = szFileTitle;
  ofn.nMaxFileTitle = sizeof(szFileTitle);
  ofn.lpstrInitialDir = szDirName;
  ofn.Flags = OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY|OFN_NOREADONLYRETURN;
  if (GetSaveFileName(&ofn)) {
    char tmpfile[500];
    strcpy(tmpfile,szFile);
    force_ext(tmpfile,file_type);
    result=1;
    switch(file_type)
    {
        case FTYPE_STM:
            save_image(tmpfile,1);
            temp=strrchr(tmpfile,'\\');
            if (temp==NULL) temp=tmpfile;
            else temp+=1;
            strcpy(current_file_stm,temp);
            if (valid_filename(current_file_stm)) 
                file_stm_count=str2int(current_file_stm+6);
            (*glob_data)->saved = TRUE;
            break;
        case FTYPE_STP:
            save_image(tmpfile,0);
            (*glob_data)->saved = TRUE;
            break;
        case FTYPE_PS:
            print_file(tmpfile,hWnd);
            temp=strrchr(tmpfile,'\\');
            if (temp==NULL) temp=tmpfile;
            else temp+=1;
            strcpy(current_file_ps,temp);
            break;
        case FTYPE_CUT:
            save_cut(tmpfile,1);
            temp=strrchr(tmpfile,'\\');
            if (temp==NULL) temp=tmpfile;
            else temp+=1;
            strcpy(current_file_cut,temp);
            if (valid_filename(current_file_cut)) 
                file_cut_count=str2int(current_file_cut+6);
            break;
        case FTYPE_STM_UNCOMPRESSED:
            save_image(tmpfile,0);
            temp=strrchr(tmpfile,'\\');
            if (temp==NULL) temp=tmpfile;
            else temp+=1;
            strcpy(current_file_stm,temp);
            if (valid_filename(current_file_stm)) 
                file_stm_count=str2int(current_file_stm+6);
            (*glob_data)->saved = TRUE;
            break;
        case FTYPE_DEP:
            save_dep(tmpfile);
            temp=strrchr(tmpfile,'\\');
            if (temp==NULL) temp=tmpfile;
            else temp+=1;
            strcpy(current_file_dep,temp);
            if (valid_filename(current_file_dep)) 
                file_dep_count=str2int(current_file_dep+6);
            break;
        case FTYPE_GRD:
            save_grd(tmpfile);
            temp=strrchr(tmpfile,'\\');
            if (temp==NULL) temp=tmpfile;
            else temp+=1;
            strcpy(current_file_grd,temp);
            break;
        case FTYPE_MAP:
            save_map(tmpfile);
            temp=strrchr(tmpfile,'\\');
            if (temp==NULL) temp=tmpfile;
            else temp+=1;
            strcpy(current_file_map,temp);
            break;
        case FTYPE_CNT:
            save_cnt(tmpfile);
            temp=strrchr(tmpfile,'\\');
            if (temp==NULL) temp=tmpfile;
            else temp+=1;
            strcpy(current_file_cnt,temp);
            break;
        case FTYPE_CNT_AS_TXT:
            save_cnt_as_txt(tmpfile);
            temp=strrchr(tmpfile,'\\');
            if (temp==NULL) temp=tmpfile;
            else temp+=1;
            strcpy(current_file_cnt,temp);
            break;
        case FTYPE_GEN2D_EXPORT:
            save_exp(tmpfile);
            temp=strrchr(tmpfile,'\\');
            if (temp==NULL) temp=tmpfile;
            else temp+=1;
            strcpy(current_file_exp,temp);
            break;
        case FTYPE_HOP_DATA:
            save_exp(tmpfile);
            temp=strrchr(tmpfile,'\\');
            if (temp==NULL) temp=tmpfile;
            else temp+=1;
            strcpy(current_file_hop,temp);
            break;
        case FTYPE_FIG:
            save_fig(tmpfile);
            temp=strrchr(tmpfile,'\\');
            if (temp==NULL) temp=tmpfile;
            else temp+=1;
            strcpy(current_file_fig,temp);
            break;
        case FTYPE_PAL:
            save_pal(tmpfile);
            break;
        case FTYPE_SPC:
            save_mas(tmpfile,1);
            temp=strrchr(tmpfile,'\\');
            if (temp==NULL) temp=tmpfile;
            else temp+=1;
            strcpy(current_file_spc,temp);
            if (valid_filename(current_file_spc)) 
                file_spc_count=str2int(current_file_spc+6);
            break;
        case FTYPE_MAS:
            save_mas(tmpfile,1);
            temp=strrchr(tmpfile,'\\');
            if (temp==NULL) temp=tmpfile;
            else temp+=1;
            strcpy(current_file_mas,temp);
            if (valid_filename(current_file_mas)) 
                file_mas_count=str2int(current_file_mas+6);
            break;
        case FTYPE_3D2TEXT:
	    save_3d_as_text(tmpfile);
	    break;
        case FTYPE_GIF:
            save_gif(tmpfile);
            break;
    }
    change_dir(tmpfile,file_type);
   }
   return(result);
}

int load_image_old(char *filename)
{
  FILE *fp;
  char *name;

  (*glob_data)->saved=1;

  fp = fopen(filename,"rb");
  if (fp==NULL) return 0;
  strcpy((*glob_data)->full_filename,filename);  
  name=strrchr(filename,'\\');
  if (name==NULL) strcpy((*glob_data)->filename,filename);
  else strcpy((*glob_data)->filename,name+1);
  *(strrchr((*glob_data)->filename,'.'))='\0';
  (*glob_data)->type=DATATYPE_3D;
  fread(&((*glob_data)->comment.size),sizeof((*glob_data)->comment.size),1,fp);
  fread((*glob_data)->comment.ptr,sizeof(char),(size_t)(*glob_data)->comment.size,fp);
  (*glob_data)->comment.ptr[(*glob_data)->comment.size]='\0';
  fread(&((*glob_data)->size),sizeof((*glob_data)->size),1,fp);
  fread(&((*glob_data)->step),sizeof((*glob_data)->step),1,fp);
  fread(&((*glob_data)->x),sizeof((*glob_data)->x),1,fp);
  fread(&((*glob_data)->y),sizeof((*glob_data)->y),1,fp);
  fread(&((*glob_data)->z),sizeof((*glob_data)->z),1,fp);
  fread(&((*glob_data)->x_gain),sizeof((*glob_data)->x_gain),1,fp);
  fread(&((*glob_data)->y_gain),sizeof((*glob_data)->y_gain),1,fp);
  fread(&((*glob_data)->z_gain),sizeof((*glob_data)->z_gain),1,fp);
  fread(&((*glob_data)->z_freq),sizeof((*glob_data)->z_freq),1,fp);
  fread((*glob_data)->ptr,sizeof(float),(size_t)((*glob_data)->size*(*glob_data)->size),fp);
  (*glob_data)->version=1;
/* version 2 files */
  if (fread(&((*glob_data)->x_range),sizeof((*glob_data)->x_range),1,fp)==1)

  {
      fread(&((*glob_data)->y_range),sizeof((*glob_data)->y_range),1,fp);
      fread(&((*glob_data)->sample_type.size),sizeof((*glob_data)->sample_type.size),1,fp);
      fread((*glob_data)->sample_type.ptr,sizeof(char),(size_t)(*glob_data)->sample_type.size,fp);
      (*glob_data)->sample_type.ptr[(*glob_data)->sample_type.size]='\0';
      (*glob_data)->version=2;
/* version 3 files */
      if (fread(&(*glob_data)->x_offset_range,sizeof((*glob_data)->x_offset_range),1,fp)==1)  
      {
          fread(&(*glob_data)->y_offset_range,sizeof((*glob_data)->y_offset_range),1,fp);  
          fread(&(*glob_data)->bias,sizeof((*glob_data)->bias),1,fp);  
          fread(&(*glob_data)->bias_range,sizeof((*glob_data)->bias_range),1,fp);  
          fread(&(*glob_data)->scan_dir,sizeof((*glob_data)->scan_dir),1,fp);  
          fread(&(*glob_data)->i_setpoint,sizeof((*glob_data)->i_setpoint),1,fp);  
          fread(&(*glob_data)->i_set_range,sizeof((*glob_data)->i_set_range),1,fp);  
          fread(&(*glob_data)->dosed_langmuir,sizeof((*glob_data)->dosed_langmuir),1,fp);  
          fread(&(*glob_data)->amp_gain,sizeof((*glob_data)->amp_gain),1,fp);  
          fread(&(*glob_data)->step_delay,sizeof((*glob_data)->step_delay),1,fp);  
          fread(&(*glob_data)->inter_step_delay,sizeof((*glob_data)->inter_step_delay),1,fp);  
          fread(&(*glob_data)->digital_feedback,sizeof((*glob_data)->digital_feedback),1,fp);  
          fread(&(*glob_data)->tip_spacing,sizeof((*glob_data)->tip_spacing),1,fp);  
          fread(&(*glob_data)->inter_line_delay,sizeof((*glob_data)->inter_line_delay),1,fp);  
          fread(&(*glob_data)->scan_num,sizeof((*glob_data)->scan_num),1,fp);  
          fread(&(*glob_data)->scan_feedback,sizeof((*glob_data)->scan_feedback),1,fp);  
          fread(&(*glob_data)->read_ch,sizeof((*glob_data)->read_ch),1,fp);  
          fread(&(*glob_data)->crash_protection,sizeof((*glob_data)->crash_protection),1,fp);  
          fread(&(*glob_data)->overshoot,sizeof((*glob_data)->overshoot),1,fp);  
          fread(&(*glob_data)->overshoot_percent,sizeof((*glob_data)->overshoot_percent),1,fp);  
          fread(&(*glob_data)->overshoot_wait1,sizeof((*glob_data)->overshoot_wait1),1,fp);  
          fread(&(*glob_data)->overshoot_wait2,sizeof((*glob_data)->overshoot_wait2),1,fp);  
          fread(&((*glob_data)->dosed_type.size),sizeof((*glob_data)->dosed_type.size),1,fp);
          fread((*glob_data)->dosed_type.ptr,sizeof(char),(size_t)(*glob_data)->dosed_type.size,fp);
          (*glob_data)->version=3;
/* version 4 files */          
          if (fread(&((*glob_data)->temperature),sizeof((*glob_data)->temperature),1,fp)==1)
          {
            fread((*glob_data)->dep_filename,sizeof((*glob_data)->dep_filename[0])*FILE_NAME_SIZE,1,fp);
            (*glob_data)->version=4;
          }
      }
  }
  if ((*glob_data)->version<2)
  {
    (*glob_data)->sample_type.size=0;    
    (*glob_data)->x_range=2;
    (*glob_data)->y_range=2;
  }
  if ((*glob_data)->version<3)
  {
      (*glob_data)->x_offset_range=2;
      (*glob_data)->y_offset_range=2;
      (*glob_data)->bias=2200;
      (*glob_data)->bias_range=2;
      (*glob_data)->scan_dir=0;
      (*glob_data)->i_setpoint=2048;
      (*glob_data)->i_set_range=2;
      (*glob_data)->amp_gain=8;
      (*glob_data)->dosed_langmuir=0;
      (*glob_data)->dosed_type.size=0;
      (*glob_data)->step_delay=0;
      (*glob_data)->inter_step_delay=0;
      (*glob_data)->digital_feedback=0;
      (*glob_data)->tip_spacing=0.5;
      (*glob_data)->inter_step_delay=0;
      (*glob_data)->scan_num=1;
      (*glob_data)->scan_feedback=1;
      (*glob_data)->read_ch=zi_ch;
      (*glob_data)->crash_protection=0;
      (*glob_data)->overshoot=0;
      (*glob_data)->overshoot_percent=0;
      (*glob_data)->overshoot_wait1=0;
      (*glob_data)->overshoot_wait2=0;
  }
  if ((*glob_data)->version<4)
  {
    (*glob_data)->temperature=0;
    (*glob_data)->dep_filename[0]='\0';
  }

  (*glob_data)->read_feedback=(*glob_data)->scan_feedback;
  (*glob_data)->scan_dither0=0;
  (*glob_data)->scan_dither1=0;
  (*glob_data)->dither_wait=0;
  (*glob_data)->dither0=0;
  (*glob_data)->dither1=0;
  (*glob_data)->read_feedback=(*glob_data)->scan_feedback;
  (*glob_data)->scan_dither0=0;
  (*glob_data)->scan_dither1=0;
  (*glob_data)->dither_wait=0;
  (*glob_data)->dither0=0;
  (*glob_data)->dither1=0;
  (*glob_data)->type=DATATYPE_3D;
  (*glob_data)->digital_feedback_max=DIGITAL_FEEDBACK_MAX;
  (*glob_data)->digital_feedback_reread=DIGITAL_FEEDBACK_REREAD;
  (*glob_data)->z_limit_percent=5.0;
  (*glob_data)->current_read_seq=0;
  (*glob_data)->step_delay_fixed=1;
  if((*glob_data)->scan_dir>=SCAN_X_FIRST)
  {
    (*glob_data)->x_first=1;
    (*glob_data)->scan_dir-=SCAN_X_FIRST;
  }
  else
  {
    (*glob_data)->x_first=0;
  }
#ifdef OLD_READ_SEQUENCE
  if((*glob_data)->read_seq_num!=1)
  {
    free_data_seq(glob_data);
    alloc_data_seq(glob_data,1);
    (*glob_data)->read_seq_num=1;
  }
  (*glob_data)->read_seq[0].feedback=(*glob_data)->read_feedback;
  (*glob_data)->read_seq[0].wait1=0;
  (*glob_data)->read_seq[0].dither0=(*glob_data)->dither0;
  (*glob_data)->read_seq[0].wait2=0;
  (*glob_data)->read_seq[0].dither1=(*glob_data)->dither1;
  (*glob_data)->read_seq[0].wait3=0;
  (*glob_data)->read_seq[0].do_ramp_bias=0;
  (*glob_data)->read_seq[0].ramp_value=0;
  (*glob_data)->read_seq[0].wait4=0;
  (*glob_data)->read_seq[0].record=1;
  (*glob_data)->read_seq[0].read_ch=(*glob_data)->read_ch;
  (*glob_data)->read_seq[0].ramp_ch=sample_bias_ch;
  (*glob_data)->read_seq[0].num_samples=(*glob_data)->scan_num;
#endif
  (*glob_data)->total_steps = 1;
  (*glob_data)->sequence[0].type = READ_TYPE;
  (*glob_data)->sequence[0].state = 1;
  (*glob_data)->sequence[0].wait = 1;
  (*glob_data)->sequence[0].out_ch = 0;
  (*glob_data)->sequence[0].step_bias = 0;
  (*glob_data)->sequence[0].in_ch  = zi_ch;
  (*glob_data)->sequence[0].num_reads = 5;
  strcpy((*glob_data)->sequence[0].step_type_string,"Read");

  fclose(fp);

  *((*glob_data)->sample_type.ptr+(*glob_data)->sample_type.size) = '\0';
  *((*glob_data)->dosed_type.ptr+(*glob_data)->dosed_type.size) = '\0';
  *((*glob_data)->comment.ptr+(*glob_data)->comment.size) = '\0';
  (*glob_data)->valid=1;
  unequalize((*glob_data));
  
    return 1;
}

int load_image(char *filename)
{
  FILE *fp;
  char *name;
  int magic;
  int result=0;
    
    strcpy((*glob_data)->full_filename,filename);  
    name=strrchr(filename,'\\');
    if (name==NULL) strcpy((*glob_data)->filename,filename);
    else strcpy((*glob_data)->filename,name+1);
    *(strrchr((*glob_data)->filename,'.'))='\0';
    fp = fopen(filename,"rb");
    if (fp==NULL) return 0;
    fread(&magic,sizeof(magic),1,fp);
    if (magic<FILE_MAGIC_MIN)
    {
        fclose(fp);
        return(load_image_old(filename));
    }
    fclose(fp);
    fp = fopen(filename,"rb");
    result=load_image_fp(fp);
    fclose(fp);
    return result;
}

static void sequence_to_scan(int version)
{
  // converts a valid read sequence in (*glob_data) to a
  // series of read steps

  int steps = 0; // number of read steps
  int j;
  int cur_feedback = (*glob_data)->scan_feedback;
  int cur_dither0 = (*glob_data)->scan_dither0;
  int cur_dither1 = (*glob_data)->scan_dither1;
  
  for(j = 0;(j < (*glob_data)->read_seq_num) && (steps < MAX_READ_STEPS);j++)
  {
    // convert each read sequence to the appropriate read steps
	if((*glob_data)->read_seq[j].feedback != cur_feedback)
	{
      (*glob_data)->sequence[steps].type = FEEDBACK_TYPE;
      strcpy((*glob_data)->sequence[steps].step_type_string,"Feedback");
      (*glob_data)->sequence[steps].state = (*glob_data)->read_seq[j].feedback;
      cur_feedback = (*glob_data)->read_seq[j].feedback;
	  steps++;
	  if(steps == MAX_READ_STEPS) break;
	}
	if((*glob_data)->read_seq[j].wait1 > 0)
	{
      (*glob_data)->sequence[steps].type = WAIT_TYPE;
      strcpy((*glob_data)->sequence[steps].step_type_string,"Wait");
	  (*glob_data)->sequence[steps].wait = (*glob_data)->read_seq[j].wait1;
	  steps++;
	  if(steps == MAX_READ_STEPS) break;
	}
	if((*glob_data)->read_seq[j].dither0 != cur_dither0)
	{
      (*glob_data)->sequence[steps].type = DITHER0_TYPE;
      strcpy((*glob_data)->sequence[steps].step_type_string,"Dither Ch 0");
      (*glob_data)->sequence[steps].state = (*glob_data)->read_seq[j].dither0;
      cur_dither0 = (*glob_data)->read_seq[j].dither0;
	  steps++;
	  if(steps == MAX_READ_STEPS) break;
	}
	if((*glob_data)->read_seq[j].wait2 > 0)
	{
      (*glob_data)->sequence[steps].type = WAIT_TYPE;
      strcpy((*glob_data)->sequence[steps].step_type_string,"Wait");
	  (*glob_data)->sequence[steps].wait = (*glob_data)->read_seq[j].wait2;
	  steps++;
	  if(steps == MAX_READ_STEPS) break;
	}
	if((*glob_data)->read_seq[j].dither1 != cur_dither1)
	{
      (*glob_data)->sequence[steps].type = DITHER1_TYPE;
      strcpy((*glob_data)->sequence[steps].step_type_string,"Dither Ch 1");
      (*glob_data)->sequence[steps].state = (*glob_data)->read_seq[j].dither1;
      cur_dither1 = (*glob_data)->read_seq[j].dither1;
	  steps++;
	  if(steps == MAX_READ_STEPS) break;
	}
	if((*glob_data)->read_seq[j].wait3 > 0)
	{
      (*glob_data)->sequence[steps].type = WAIT_TYPE;
      strcpy((*glob_data)->sequence[steps].step_type_string,"Wait");
	  (*glob_data)->sequence[steps].wait = (*glob_data)->read_seq[j].wait3;
	  steps++;
	  if(steps == MAX_READ_STEPS) break;
	}

    if(version > 7)
	{
      if((*glob_data)->read_seq[j].do_ramp_bias)
	  {
        (*glob_data)->sequence[steps].type = STEP_OUTPUT_TYPE;
        strcpy((*glob_data)->sequence[steps].step_type_string,"Step Output Ch");
		(*glob_data)->sequence[steps].step_bias = (*glob_data)->read_seq[j].ramp_value;
        if(version > 8)
		{
		  (*glob_data)->sequence[steps].out_ch = (*glob_data)->read_seq[j].ramp_ch;
		}
		else (*glob_data)->sequence[steps].out_ch = sample_bias_ch;
	    steps++;
	    if(steps == MAX_READ_STEPS) break;
	  }
	  if((*glob_data)->read_seq[j].wait4 > 0)
	  {
        (*glob_data)->sequence[steps].type = WAIT_TYPE;
        strcpy((*glob_data)->sequence[steps].step_type_string,"Wait");
	    (*glob_data)->sequence[steps].wait = (*glob_data)->read_seq[j].wait4;
	    steps++;
	    if(steps == MAX_READ_STEPS) break;
	  }
    }

    if((*glob_data)->read_seq[j].record)
    {
      (*glob_data)->sequence[steps].type = READ_TYPE;
      strcpy((*glob_data)->sequence[steps].step_type_string,"Read");
      (*glob_data)->sequence[steps].in_ch = (*glob_data)->read_seq[j].read_ch;
      (*glob_data)->sequence[steps].num_reads = (*glob_data)->read_seq[j].num_samples;
	  steps++;
	  if(steps == MAX_READ_STEPS) break;
	}

  }
  if(steps == 0)
  {
    steps = 1;
	(*glob_data)->sequence[0].type = READ_TYPE;
	(*glob_data)->sequence[0].state = 1;
	(*glob_data)->sequence[0].wait = 1;
	(*glob_data)->sequence[0].out_ch = 0;
	(*glob_data)->sequence[0].step_bias = 0;
	(*glob_data)->sequence[0].in_ch  = zi_ch;
	(*glob_data)->sequence[0].num_reads = 5;
    strcpy((*glob_data)->sequence[0].step_type_string,"Read");
  }

  (*glob_data)->total_steps = steps;
}

int load_image_fp(FILE *fp)
{
  int magic,version,i;
  Byte *compr,*uncompr;
  uLong comprLen,uncomprLen;
  int err;
  int compressed = 1;

  (*glob_data)->saved = 1;
    
  fread(&magic,sizeof(magic),1,fp);
  fread(&version,sizeof(version),1,fp);

  (*glob_data)->version = version;
//  (*glob_data)->type=DATATYPE_3D;
  fread(&((*glob_data)->comment.size),sizeof((*glob_data)->comment.size),1,fp);
  fread((*glob_data)->comment.ptr,sizeof(char),(size_t)(*glob_data)->comment.size,fp);
  (*glob_data)->comment.ptr[(*glob_data)->comment.size] = '\0';
  fread(&((*glob_data)->size),sizeof((*glob_data)->size),1,fp);
  fread(&((*glob_data)->step),sizeof((*glob_data)->step),1,fp);
  fread(&((*glob_data)->x),sizeof((*glob_data)->x),1,fp);
  fread(&((*glob_data)->y),sizeof((*glob_data)->y),1,fp);
  fread(&((*glob_data)->z),sizeof((*glob_data)->z),1,fp);
  fread(&((*glob_data)->x_gain),sizeof((*glob_data)->x_gain),1,fp);
  fread(&((*glob_data)->y_gain),sizeof((*glob_data)->y_gain),1,fp);
  fread(&((*glob_data)->z_gain),sizeof((*glob_data)->z_gain),1,fp);
  fread(&((*glob_data)->z_freq),sizeof((*glob_data)->z_freq),1,fp);
  fread(&compressed,sizeof(int),1,fp);
  fread(&comprLen,sizeof(uLong),1,fp);
  if(comprLen && (*glob_data)->type == DATATYPE_3D_PARAMETERS)
  {
    mprintf("eek. wrong file!");
    return 0;
  }
  if(comprLen)
  {
    compr = (Byte *)malloc(comprLen);
    fread(compr,sizeof(Byte),comprLen,fp);
    uncomprLen = (*glob_data)->size * (*glob_data)->size * sizeof(float);
    if(compressed)
    {
      uncompr = (Byte *)((*glob_data)->ptr);
      err = uncompress(uncompr, &uncomprLen, compr, comprLen);
      if(err != Z_OK)
      {
        mprintf("Data corrupted. Start praying.");
      }
    }
    else
    {
      memcpy((*glob_data)->ptr,compr,comprLen);
    }
    free(compr);
  }
/* version 2 files */    
  fread(&((*glob_data)->x_range),sizeof((*glob_data)->x_range),1,fp);
  fread(&((*glob_data)->y_range),sizeof((*glob_data)->y_range),1,fp);
  fread(&((*glob_data)->sample_type.size),sizeof((*glob_data)->sample_type.size),1,fp);
  fread((*glob_data)->sample_type.ptr,sizeof(char),(size_t)(*glob_data)->sample_type.size,fp);
  (*glob_data)->sample_type.ptr[(*glob_data)->sample_type.size] = '\0';
/* version 3 files */
  fread(&(*glob_data)->x_offset_range,sizeof((*glob_data)->x_offset_range),1,fp);
  fread(&(*glob_data)->y_offset_range,sizeof((*glob_data)->y_offset_range),1,fp);  
  fread(&(*glob_data)->bias,sizeof((*glob_data)->bias),1,fp);  
  fread(&(*glob_data)->bias_range,sizeof((*glob_data)->bias_range),1,fp);  
  fread(&(*glob_data)->scan_dir,sizeof((*glob_data)->scan_dir),1,fp);  
  fread(&(*glob_data)->i_setpoint,sizeof((*glob_data)->i_setpoint),1,fp);  
  fread(&(*glob_data)->i_set_range,sizeof((*glob_data)->i_set_range),1,fp);  
  fread(&(*glob_data)->dosed_langmuir,sizeof((*glob_data)->dosed_langmuir),1,fp);  
  fread(&(*glob_data)->amp_gain,sizeof((*glob_data)->amp_gain),1,fp);  
  fread(&(*glob_data)->step_delay,sizeof((*glob_data)->step_delay),1,fp);  
  fread(&(*glob_data)->inter_step_delay,sizeof((*glob_data)->inter_step_delay),1,fp);  
  fread(&(*glob_data)->digital_feedback,sizeof((*glob_data)->digital_feedback),1,fp);  
  fread(&(*glob_data)->tip_spacing,sizeof((*glob_data)->tip_spacing),1,fp);  
  fread(&(*glob_data)->inter_line_delay,sizeof((*glob_data)->inter_line_delay),1,fp);  
  fread(&(*glob_data)->scan_num,sizeof((*glob_data)->scan_num),1,fp);  
  fread(&(*glob_data)->scan_feedback,sizeof((*glob_data)->scan_feedback),1,fp);  
  if((*glob_data)->scan_feedback) (*glob_data)->scan_feedback = 1;
  fread(&(*glob_data)->read_ch,sizeof((*glob_data)->read_ch),1,fp);  
  fread(&(*glob_data)->crash_protection,sizeof((*glob_data)->crash_protection),1,fp);  
  fread(&(*glob_data)->overshoot,sizeof((*glob_data)->overshoot),1,fp);  
  fread(&(*glob_data)->overshoot_percent,sizeof((*glob_data)->overshoot_percent),1,fp);  
  fread(&(*glob_data)->overshoot_wait1,sizeof((*glob_data)->overshoot_wait1),1,fp);  
  fread(&(*glob_data)->overshoot_wait2,sizeof((*glob_data)->overshoot_wait2),1,fp);  
  fread(&((*glob_data)->dosed_type.size),sizeof((*glob_data)->dosed_type.size),1,fp);
  fread((*glob_data)->dosed_type.ptr,sizeof(char),(size_t)(*glob_data)->dosed_type.size,fp);
/* version 4 files */          
  fread(&((*glob_data)->temperature),sizeof((*glob_data)->temperature),1,fp);
  fread((*glob_data)->dep_filename,sizeof((*glob_data)->dep_filename[0]) * FILE_NAME_SIZE,1,fp);
/* version 5 files */
  fread(&((*glob_data)->version),sizeof((*glob_data)->version),1,fp);
/* erf kludge to fix some badly saved files */    
  if(version == 5 && (*glob_data)->version == 3) (*glob_data)->version = 5;
/* version 6 files */
  if(version > 5)
  {
    fread(&((*glob_data)->read_feedback),sizeof((*glob_data)->read_feedback),1,fp);
    if((*glob_data)->read_feedback) (*glob_data)->read_feedback = 1;
    fread(&((*glob_data)->scan_dither0),sizeof((*glob_data)->scan_dither0),1,fp);
    fread(&((*glob_data)->scan_dither1),sizeof((*glob_data)->scan_dither1),1,fp);
    fread(&((*glob_data)->dither_wait),sizeof((*glob_data)->dither_wait),1,fp);
    fread(&((*glob_data)->dither0),sizeof((*glob_data)->dither0),1,fp);
    fread(&((*glob_data)->dither1),sizeof((*glob_data)->dither1),1,fp);
  }
  else
  {
    (*glob_data)->read_feedback = (*glob_data)->scan_feedback;
    (*glob_data)->scan_dither0 = 0;
    (*glob_data)->scan_dither1 = 0;
    (*glob_data)->dither_wait = 0;
    (*glob_data)->dither0 = 0;
    (*glob_data)->dither1 = 0;
  }
/* version 7 files */
  if(version > 6)
  {
    fread(&((*glob_data)->type),sizeof((*glob_data)->type),1,fp);
    fread(&((*glob_data)->digital_feedback_max),
          sizeof((*glob_data)->digital_feedback_max),1,fp);
    fread(&((*glob_data)->digital_feedback_reread),
          sizeof((*glob_data)->digital_feedback_reread),1,fp);
    fread1((*glob_data)->z_limit_percent);
    fread1((*glob_data)->x_first);
    fread1((*glob_data)->current_read_seq);
    fread1((*glob_data)->step_delay_fixed);
	if(version < 10)  // file was saved with old read sequences
	{
	  free_data_seq(glob_data);
      fread(&((*glob_data)->read_seq_num),sizeof((*glob_data)->read_seq_num),1,fp);
      alloc_data_seq(glob_data,(*glob_data)->read_seq_num);
      for(i = 0;i < (*glob_data)->read_seq_num;i++)
	  {
        fread1((*glob_data)->read_seq[i].feedback);
        fread1((*glob_data)->read_seq[i].wait1);
        fread1((*glob_data)->read_seq[i].dither0);
        fread1((*glob_data)->read_seq[i].wait2);
        fread1((*glob_data)->read_seq[i].dither1);
        fread1((*glob_data)->read_seq[i].wait3);
        fread1((*glob_data)->read_seq[i].record);
        fread1((*glob_data)->read_seq[i].read_ch);
        fread1((*glob_data)->read_seq[i].num_samples);

/* version 8 files */
	    if(version > 7)
		{
		  fread1((*glob_data)->read_seq[i].do_ramp_bias);
		  fread1((*glob_data)->read_seq[i].ramp_value);
		  fread1((*glob_data)->read_seq[i].wait4);

/* version 9 files */
          if(version > 8) fread1((*glob_data)->read_seq[i].ramp_ch);
		  else (*glob_data)->read_seq[i].ramp_ch = sample_bias_ch;
		}

	  } 
	  sequence_to_scan(version);
	}
	else  // file was saved with new read step sequences
	{
/* version 10 files */
	  fread(&((*glob_data)->total_steps),sizeof((*glob_data)->total_steps),1,fp);
	  for(i = 0;i < (*glob_data)->total_steps;i++)
	  {
		fread1((*glob_data)->sequence[i].type);
		fread1((*glob_data)->sequence[i].state);
		fread1((*glob_data)->sequence[i].wait);
		fread1((*glob_data)->sequence[i].out_ch);
		fread1((*glob_data)->sequence[i].step_bias);
		fread1((*glob_data)->sequence[i].in_ch);
		fread1((*glob_data)->sequence[i].num_reads);
        fread((*glob_data)->sequence[i].step_type_string,sizeof(char),STEP_TYPE_STR_MAX,fp);
	  }
	}
  }

  else  // version < 6. fill in fields with default values.
  {
    (*glob_data)->type = DATATYPE_3D;
    (*glob_data)->digital_feedback_max = DIGITAL_FEEDBACK_MAX;
    (*glob_data)->digital_feedback_reread = DIGITAL_FEEDBACK_REREAD;
    (*glob_data)->z_limit_percent = 5.0;
    (*glob_data)->current_read_seq = 0;
    (*glob_data)->step_delay_fixed = 1;
    if((*glob_data)->scan_dir >= SCAN_X_FIRST)
	{
      (*glob_data)->x_first = 1;
      (*glob_data)->scan_dir -= SCAN_X_FIRST;
	}
    else
	{
      (*glob_data)->x_first = 0;
	}
#ifdef OLD
    if((*glob_data)->read_seq_num != 1)
	{
      free_data_seq(glob_data);
      alloc_data_seq(glob_data,1);
      (*glob_data)->read_seq_num = 1;
	}
    (*glob_data)->read_seq[0].feedback = (*glob_data)->read_feedback;
    (*glob_data)->read_seq[0].wait1 = 0;
    (*glob_data)->read_seq[0].dither0 = (*glob_data)->dither0;
    (*glob_data)->read_seq[0].wait2 = 0;
    (*glob_data)->read_seq[0].dither1 = (*glob_data)->dither1;
    (*glob_data)->read_seq[0].wait3 = 0;
    (*glob_data)->read_seq[0].do_ramp_bias = 0;
    (*glob_data)->read_seq[0].ramp_value = 0;
    (*glob_data)->read_seq[0].wait4 = 0;
    (*glob_data)->read_seq[0].record = 1;
    (*glob_data)->read_seq[0].read_ch = (*glob_data)->read_ch;
    (*glob_data)->read_seq[0].num_samples = (*glob_data)->scan_num;
	(*glob_data)->read_seq[i].ramp_ch = sample_bias_ch;
#endif
	if((*glob_data)->total_steps != 1)
	  (*glob_data)->total_steps = 1;
	(*glob_data)->sequence[0].type = READ_TYPE;
	(*glob_data)->sequence[0].state = 1;
	(*glob_data)->sequence[0].wait = 1;
	(*glob_data)->sequence[0].out_ch = 0;
	(*glob_data)->sequence[0].step_bias = 0;
	(*glob_data)->sequence[0].in_ch  = zi_ch;
	(*glob_data)->sequence[0].num_reads = 5;
    strcpy((*glob_data)->sequence[0].step_type_string,"Read");
  } 
        
  if((*glob_data)->type == DATATYPE_3D_PARAMETERS)
  {
    if((*glob_data)->ptr != NULL) 
    {
      free((*glob_data)->ptr);
      (*glob_data)->ptr = NULL;
    }
    (*glob_data)->size = 0;
  }
    
            
  *((*glob_data)->sample_type.ptr + (*glob_data)->sample_type.size) = '\0';
  *((*glob_data)->dosed_type.ptr + (*glob_data)->dosed_type.size) = '\0';
  *((*glob_data)->comment.ptr + (*glob_data)->comment.size) = '\0';
  (*glob_data)->valid = 1;
  unequalize((*glob_data));
    
  return 1;
}

void load_cut(char *filename)
{
  FILE *fp;
  char *name;
    
    strcpy((*glob_data)->full_filename,filename);  
    name=strrchr(filename,'\\');
    if (name==NULL) strcpy((*glob_data)->filename,filename);
    else strcpy((*glob_data)->filename,name+1);
    *(strrchr((*glob_data)->filename,'.'))='\0';
    fp = fopen(filename,"rb");
    if (fp==NULL) mprintf("couldn't load CUT file");
    load_cut_fp(fp);
    fclose(fp);
//mprintf("loaded");
}

void load_cut_fp(FILE *fp)
{
  int magic,version;    

  (*glob_data)->saved=1;
    
    fread(&magic,sizeof(magic),1,fp);
    fread(&version,sizeof(version),1,fp);
    fread(&(*glob_data)->type,sizeof((*glob_data)->type),1,fp);
    fread(&(*glob_data)->type2d,sizeof((*glob_data)->type2d),1,fp);
    
    load_gen2d_fp(fp);    

    fread(&((*glob_data)->comment.size),sizeof((*glob_data)->comment.size),1,fp);
    fread((*glob_data)->comment.ptr,sizeof(char),(size_t)(*glob_data)->comment.size,fp);
    (*glob_data)->comment.ptr[(*glob_data)->comment.size]='\0';
    fread(&((*glob_data)->step),sizeof((*glob_data)->step),1,fp);
    fread(&((*glob_data)->x),sizeof((*glob_data)->x),1,fp);
    fread(&((*glob_data)->y),sizeof((*glob_data)->y),1,fp);
    fread(&((*glob_data)->z),sizeof((*glob_data)->z),1,fp);
    fread(&((*glob_data)->x_gain),sizeof((*glob_data)->x_gain),1,fp);
    fread(&((*glob_data)->y_gain),sizeof((*glob_data)->y_gain),1,fp);
    fread(&((*glob_data)->z_gain),sizeof((*glob_data)->z_gain),1,fp);
    fread(&((*glob_data)->z_freq),sizeof((*glob_data)->z_freq),1,fp);
    fread(&((*glob_data)->x_range),sizeof((*glob_data)->x_range),1,fp);
    fread(&((*glob_data)->y_range),sizeof((*glob_data)->y_range),1,fp);
    fread(&((*glob_data)->sample_type.size),sizeof((*glob_data)->sample_type.size),1,fp);
    fread((*glob_data)->sample_type.ptr,sizeof(char),(size_t)(*glob_data)->sample_type.size,fp);
    (*glob_data)->sample_type.ptr[(*glob_data)->sample_type.size]='\0';
    fread(&(*glob_data)->x_offset_range,sizeof((*glob_data)->x_offset_range),1,fp);
    fread(&(*glob_data)->y_offset_range,sizeof((*glob_data)->y_offset_range),1,fp);  
    fread(&(*glob_data)->bias,sizeof((*glob_data)->bias),1,fp);  
    fread(&(*glob_data)->bias_range,sizeof((*glob_data)->bias_range),1,fp);  
    fread(&(*glob_data)->scan_dir,sizeof((*glob_data)->scan_dir),1,fp);  
    fread(&(*glob_data)->i_setpoint,sizeof((*glob_data)->i_setpoint),1,fp);  
    fread(&(*glob_data)->i_set_range,sizeof((*glob_data)->i_set_range),1,fp);  
    fread(&(*glob_data)->dosed_langmuir,sizeof((*glob_data)->dosed_langmuir),1,fp);  
    fread(&(*glob_data)->amp_gain,sizeof((*glob_data)->amp_gain),1,fp);  
    fread(&(*glob_data)->step_delay,sizeof((*glob_data)->step_delay),1,fp);  
    fread(&(*glob_data)->inter_step_delay,sizeof((*glob_data)->inter_step_delay),1,fp);  
    fread(&(*glob_data)->digital_feedback,sizeof((*glob_data)->digital_feedback),1,fp);  
    fread(&(*glob_data)->tip_spacing,sizeof((*glob_data)->tip_spacing),1,fp);  
    fread(&(*glob_data)->inter_line_delay,sizeof((*glob_data)->inter_line_delay),1,fp);  
    fread(&(*glob_data)->scan_num,sizeof((*glob_data)->scan_num),1,fp);  
    fread(&(*glob_data)->scan_feedback,sizeof((*glob_data)->scan_feedback),1,fp);  
    fread(&(*glob_data)->read_ch,sizeof((*glob_data)->read_ch),1,fp);  
    fread(&(*glob_data)->crash_protection,sizeof((*glob_data)->crash_protection),1,fp);  
    fread(&(*glob_data)->overshoot,sizeof((*glob_data)->overshoot),1,fp);  
    fread(&(*glob_data)->overshoot_percent,sizeof((*glob_data)->overshoot_percent),1,fp);  
    fread(&(*glob_data)->overshoot_wait1,sizeof((*glob_data)->overshoot_wait1),1,fp);  
    fread(&(*glob_data)->overshoot_wait2,sizeof((*glob_data)->overshoot_wait2),1,fp);  
    fread(&((*glob_data)->dosed_type.size),sizeof((*glob_data)->dosed_type.size),1,fp);
    fread((*glob_data)->dosed_type.ptr,sizeof(char),(size_t)(*glob_data)->dosed_type.size,fp);
    
    fread(&((*glob_data)->temperature),sizeof((*glob_data)->temperature),1,fp);
    fread((*glob_data)->dep_filename,sizeof((*glob_data)->dep_filename[0])*FILE_NAME_SIZE,1,fp);
    
    fread(&((*glob_data)->version),sizeof((*glob_data)->version),1,fp);
        
    fread(&((*glob_data)->read_feedback),sizeof((*glob_data)->read_feedback),1,fp);
    fread(&((*glob_data)->scan_dither0),sizeof((*glob_data)->scan_dither0),1,fp);
    fread(&((*glob_data)->scan_dither1),sizeof((*glob_data)->scan_dither1),1,fp);
    fread(&((*glob_data)->dither_wait),sizeof((*glob_data)->dither_wait),1,fp);
    fread(&((*glob_data)->dither0),sizeof((*glob_data)->dither0),1,fp);
    fread(&((*glob_data)->dither1),sizeof((*glob_data)->dither1),1,fp);
    fread(&((*glob_data)->min_x),sizeof((*glob_data)->min_x),1,fp);
    fread(&((*glob_data)->max_x),sizeof((*glob_data)->max_x),1,fp);
    fread(&((*glob_data)->cut_x1),sizeof((*glob_data)->cut_x1),1,fp);
    fread(&((*glob_data)->cut_x2),sizeof((*glob_data)->cut_x2),1,fp);
    fread(&((*glob_data)->cut_y1),sizeof((*glob_data)->cut_y1),1,fp);
    fread(&((*glob_data)->cut_y2),sizeof((*glob_data)->cut_y2),1,fp);
            
//mprintf("all read");
    *((*glob_data)->sample_type.ptr+(*glob_data)->sample_type.size) = '\0';
    *((*glob_data)->dosed_type.ptr+(*glob_data)->dosed_type.size) = '\0';
    *((*glob_data)->comment.ptr+(*glob_data)->comment.size) = '\0';
    (*glob_data)->valid=1;
}

void save_image(char *filename,int compressit)
{
  FILE *fp;
  char *name;
  int i;
  char string[FILE_STRING_SIZE];
//  int version, magic;
//  uLong comprLen;
  int compressed=1;

  strcpy((*glob_data)->full_filename,filename);  
  name=strrchr(filename,'\\');
  if (name==NULL) strcpy((*glob_data)->filename,filename);
  else strcpy((*glob_data)->filename,name+1);
  *(strrchr((*glob_data)->filename,'.'))='\0';
  fp = fopen(filename,"wb");
  if (fp==NULL) mprintf("Couldn't open STM file to save!");
  save_image_fp(fp,compressit);
//mprintf("all done");
  fclose(fp);
  
  if (!compressit)
  {
    for(i=0;i<FILE_STRING_SIZE;i++) string[i]='\0';
    
    if (strrchr(filename,'.')==NULL)
    {
      strcpy(string,filename);
    }
    else
      strncpy(string,filename,strlen(filename)-strlen(strrchr(filename,'.')));
    strcat(string,".dx");
    fp = fopen(string,"w");
    if (fp==NULL) mprintf("Couldn't open DX file to write");
    fprintf(fp,"file=%s\n",strlwr(strrchr(filename,'\\')+1));
    fprintf(fp,"grid=%dx%d\n",(*glob_data)->size,(*glob_data)->size);
    fprintf(fp,"format=lsb binary\n");
    fprintf(fp,"type=float\n");
    fprintf(fp,"majority=column\n");
    fprintf(fp,"header=bytes %d\n",
                    sizeof((*glob_data)->comment.size)+
                   sizeof(char)*(*glob_data)->comment.size+
                   sizeof((*glob_data)->size)+
                   sizeof((*glob_data)->step)+
                   sizeof((*glob_data)->x)+
                   sizeof((*glob_data)->y)+
                   sizeof((*glob_data)->z)+
                   sizeof((*glob_data)->x_gain)+
                   sizeof((*glob_data)->y_gain)+
                   sizeof((*glob_data)->z_gain)+
                   sizeof((*glob_data)->z_freq)+
                   sizeof(int/*version*/)+
                   sizeof(int/*magic*/)+sizeof(compressed)+sizeof(uLong/*comprLen*/));
    fclose(fp);
  }
}

void save_image_top(FILE *fp)
{
	/*
	char newln = 10;	//new line character
	char my_byte;
	int j;
	char header[800];
	char temp[100];
	Byte *compr=NULL;
	int len;

	strcpy(header,"WSxM file copyright Nanotec Electronica");
	strcat(header,newln);
	
	strcat(header,"SxM Image file");
	strcat(header,newln);
	
	strcat(header,"Image header size: 653");
	strcat(header,newln);
	strcat(header,newln);

	strcat(header,"[Control]");
	strcat(header,newln);
	strcat(header,newln);

//    sprintf(temp,"Set Point: %f nA",i_set);
//	strcat(header,temp);
//	strcat(newln);
	
	strcat(header,"    Set Point: 3.110000 nA");
	strcat(header,newln);

	strcat(header,"    Topography Bias: -4000.000000 mV");
	strcat(header,newln);

	strcat(header,"    X Amplitude: 50.000000 Å");
	strcat(header,newln);

	strcat(header,"    X Offset: 4.000000 Å");
	strcat(header,newln);

	strcat(header,"    X-Frequency: 4.000000 Hz");
	strcat(header,newln);

	strcat(header,"    Y Amplitude: 50.000000 Å");
	strcat(header,newln);

	strcat(header,"    Y Offset: -4.000000 Å");
	strcat(header,newln);

	strcat(header,"    Z Gain: 5.000000");
	strcat(header,newln);
	strcat(header,newln);

	strcat(header,"[General Info]");
	strcat(header,newln);
	strcat(header,newln);

	strcat(header,"    Acquisition channel: Topo");
	strcat(header,newln);

	strcat(header,"    Head type: STM");
	strcat(header,newln);

	strcat(header,"    Number of columns: ");
	//(!!!) insetr the number of lines here
	strcat(header,newln);


	strcat(header,"    Number of rows: ");
	//(!!!) insetr the number of lines here
	strcat(header,newln);

	strcat(header,"    Z Amplitude: 1.498352 Å");
	strcat(header,newln);
	strcat(header,newln);

	strcat(header,"[Head Settings]");
	strcat(header,newln);
	strcat(header,newln);

	strcat(header,"    X Calibration: 50.000000 Å/V");
	strcat(header,newln);

	strcat(header,"    Z Calibration: 10.000000 Å/V");
	strcat(header,newln);
	strcat(header,newln);

	strcat(header,"[Miscellaneous]");
	strcat(header,newln);
	strcat(header,newln);

	strcat(header,"    Comments: ");
	strcat(header,newln);

	strcat(header,"    Version: 1.0 (April 2000)");
	strcat(header,newln);
	strcat(header,newln);

	strcat(header,"[Header end]");
	strcat(header,newln);

	fwrite(&header,sizeof(char),653,fp);

	// (!!!) write the actual data here
	len= (*glob_data)->size*(*glob_data)->size*sizeof(float);
	compr=(Byte *)((*glob_data)->ptr);
	if (len) fwrite(compr,sizeof(Byte),len,fp);
    free(compr);
    */
}

void save_image_fp(FILE *fp,int compressit)
{
  int i,wrote;
  int version, magic;
  Byte *compr=NULL;
  uLong comprLen;
  int err,len;
  int compressed=1;


  wait_cursor();
  version=STM_DATA_VERSION;
  magic=FILE_MAGIC_STM;
  fwrite(&magic,sizeof(magic),1,fp);
  fwrite(&version,sizeof(version),1,fp);
  fwrite(&(*glob_data)->comment.size,sizeof((*glob_data)->comment.size),1,fp);
  fwrite((*glob_data)->comment.ptr,sizeof(char),(size_t)(*glob_data)->comment.size,fp);
  fwrite(&(*glob_data)->size,sizeof((*glob_data)->size),1,fp);
  fwrite(&(*glob_data)->step,sizeof((*glob_data)->step),1,fp);
  fwrite(&(*glob_data)->x,sizeof((*glob_data)->x),1,fp);
  fwrite(&(*glob_data)->y,sizeof((*glob_data)->y),1,fp);
  fwrite(&(*glob_data)->z,sizeof((*glob_data)->z),1,fp);
  fwrite(&(*glob_data)->x_gain,sizeof((*glob_data)->x_gain),1,fp);
  fwrite(&(*glob_data)->y_gain,sizeof((*glob_data)->y_gain),1,fp);
  fwrite(&(*glob_data)->z_gain,sizeof((*glob_data)->z_gain),1,fp);
  fwrite(&(*glob_data)->z_freq,sizeof((*glob_data)->z_freq),1,fp);
  if ((*glob_data)->type==DATATYPE_3D_PARAMETERS)
  {
    len=0;
    compressit=0;
    comprLen=0;
  }
  else
  {
      len= (*glob_data)->size*(*glob_data)->size*sizeof(float);

	//compressit = 0; //this line is to make sure that the files aren't compressed

      if (compressit)
      {
        comprLen=len+DATA_MAX_COMP_ADD;
        compr=(Byte *)malloc(comprLen);
        err = compress(compr, &comprLen, (const Bytef*)((*glob_data)->ptr), len);
        if (err !=Z_OK)
        {
          mprintf("Had trouble compressing... will save uncompressed");
          compressed=0;
          free(compr);
          compr=(Byte *)((*glob_data)->ptr);
          comprLen=len;
        }
      }
      else
      {
        compressed=0;
        compr=(Byte *)((*glob_data)->ptr);
        comprLen=len;
      }
  }
  fwrite(&compressed,sizeof(int),1,fp);
  fwrite(&comprLen,sizeof(uLong),1,fp);
  if (comprLen) fwrite(compr,sizeof(Byte),comprLen,fp);
  if (compressed) free(compr);    
  

/* version 2 files */
  fwrite(&(*glob_data)->x_range,sizeof((*glob_data)->x_range),1,fp);
  fwrite(&(*glob_data)->y_range,sizeof((*glob_data)->y_range),1,fp);
  fwrite(&((*glob_data)->sample_type.size),sizeof((*glob_data)->sample_type.size),1,fp);
  fwrite((*glob_data)->sample_type.ptr,sizeof(char),(size_t)(*glob_data)->sample_type.size,fp);
/*version 3 files */
  fwrite(&(*glob_data)->x_offset_range,sizeof((*glob_data)->x_offset_range),1,fp);  
  fwrite(&(*glob_data)->y_offset_range,sizeof((*glob_data)->y_offset_range),1,fp);  
  fwrite(&(*glob_data)->bias,sizeof((*glob_data)->bias),1,fp);  
  fwrite(&(*glob_data)->bias_range,sizeof((*glob_data)->bias_range),1,fp);  
  fwrite(&(*glob_data)->scan_dir,sizeof((*glob_data)->scan_dir),1,fp);  
  fwrite(&(*glob_data)->i_setpoint,sizeof((*glob_data)->i_setpoint),1,fp);  
  fwrite(&(*glob_data)->i_set_range,sizeof((*glob_data)->i_set_range),1,fp);  
  fwrite(&(*glob_data)->dosed_langmuir,sizeof((*glob_data)->dosed_langmuir),1,fp);  
  fwrite(&(*glob_data)->amp_gain,sizeof((*glob_data)->amp_gain),1,fp);  
  fwrite(&(*glob_data)->step_delay,sizeof((*glob_data)->step_delay),1,fp);  
  fwrite(&(*glob_data)->inter_step_delay,sizeof((*glob_data)->inter_step_delay),1,fp);  
  fwrite(&(*glob_data)->digital_feedback,sizeof((*glob_data)->digital_feedback),1,fp);  
  fwrite(&(*glob_data)->tip_spacing,sizeof((*glob_data)->tip_spacing),1,fp);  
  fwrite(&(*glob_data)->inter_line_delay,sizeof((*glob_data)->inter_line_delay),1,fp);  
  fwrite(&(*glob_data)->scan_num,sizeof((*glob_data)->scan_num),1,fp);  
  fwrite(&(*glob_data)->scan_feedback,sizeof((*glob_data)->scan_feedback),1,fp);  
  fwrite(&(*glob_data)->read_ch,sizeof((*glob_data)->read_ch),1,fp);  
  fwrite(&(*glob_data)->crash_protection,sizeof((*glob_data)->crash_protection),1,fp);  
  fwrite(&(*glob_data)->overshoot,sizeof((*glob_data)->overshoot),1,fp);  
  fwrite(&(*glob_data)->overshoot_percent,sizeof((*glob_data)->overshoot_percent),1,fp);  
  fwrite(&(*glob_data)->overshoot_wait1,sizeof((*glob_data)->overshoot_wait1),1,fp);  
  fwrite(&(*glob_data)->overshoot_wait2,sizeof((*glob_data)->overshoot_wait2),1,fp);  
  fwrite(&((*glob_data)->dosed_type.size),sizeof((*glob_data)->dosed_type.size),1,fp);
  fwrite((*glob_data)->dosed_type.ptr,sizeof(char),(size_t)(*glob_data)->dosed_type.size,fp);
/* version 4 files */
  fwrite(&((*glob_data)->temperature),sizeof((*glob_data)->temperature),1,fp);
  fwrite(&((*glob_data)->dep_filename),sizeof((*glob_data)->dep_filename[0])*FILE_NAME_SIZE,1,fp);
/* version 5 files */  
  fwrite(&((*glob_data)->version),sizeof((*glob_data)->version),1,fp);
/* version 6 files */
  fwrite(&((*glob_data)->read_feedback),sizeof((*glob_data)->read_feedback),1,fp);
  fwrite(&((*glob_data)->scan_dither0),sizeof((*glob_data)->scan_dither0),1,fp);
  fwrite(&((*glob_data)->scan_dither1),sizeof((*glob_data)->scan_dither1),1,fp);
  fwrite(&((*glob_data)->dither_wait),sizeof((*glob_data)->dither_wait),1,fp);
  fwrite(&((*glob_data)->dither0),sizeof((*glob_data)->dither0),1,fp);
  fwrite(&((*glob_data)->dither1),sizeof((*glob_data)->dither1),1,fp);

//mprintf("got to 7");

/* version 7 file */
  fwrite(&((*glob_data)->type),sizeof((*glob_data)->type),1,fp);
  fwrite(&((*glob_data)->digital_feedback_max),
    sizeof((*glob_data)->digital_feedback_max),1,fp);
  fwrite(&((*glob_data)->digital_feedback_reread),
    sizeof((*glob_data)->digital_feedback_reread),1,fp);
  fwrite1((*glob_data)->z_limit_percent);
  fwrite1((*glob_data)->x_first);
  fwrite1((*glob_data)->current_read_seq);
  fwrite1((*glob_data)->step_delay_fixed);
#ifdef OLD
  fwrite(&((*glob_data)->read_seq_num),sizeof((*glob_data)->read_seq_num),1,fp);
  for(i=0;i<(*glob_data)->read_seq_num;i++)
  {
      fwrite1((*glob_data)->read_seq[i].feedback);
      fwrite1((*glob_data)->read_seq[i].wait1);
      fwrite1((*glob_data)->read_seq[i].dither0);
      fwrite1((*glob_data)->read_seq[i].wait2);
      fwrite1((*glob_data)->read_seq[i].dither1);
      fwrite1((*glob_data)->read_seq[i].wait3);
      fwrite1((*glob_data)->read_seq[i].record);
      fwrite1((*glob_data)->read_seq[i].read_ch);
      wrote=fwrite1((*glob_data)->read_seq[i].num_samples);
/* version 8 files */
      if(version > 7) {
        fwrite1((*glob_data)->read_seq[i].do_ramp_bias);
        fwrite1((*glob_data)->read_seq[i].ramp_value);
        fwrite1((*glob_data)->read_seq[i].wait4);
      }
	  if(version > 8) fwrite1((*glob_data)->read_seq[i].ramp_ch);
  }
#endif

/* version 9 files */
  fwrite1((*glob_data)->total_steps);
  for(i = 0;i < (*glob_data)->total_steps;i++)
  {
    fwrite1((*glob_data)->sequence[i].type);
    fwrite1((*glob_data)->sequence[i].state);
    fwrite1((*glob_data)->sequence[i].wait);
    fwrite1((*glob_data)->sequence[i].out_ch);
    fwrite1((*glob_data)->sequence[i].step_bias);
    fwrite1((*glob_data)->sequence[i].in_ch);
    fwrite1((*glob_data)->sequence[i].num_reads);
    wrote = fwrite(&((*glob_data)->sequence[i].step_type_string),
		           sizeof(char),STEP_TYPE_STR_MAX,fp);
   }

//mprintf("all done");  
  arrow_cursor();
  if (wrote<1)
    mprintf("Disk is full. Image not saved properly!\n Free disk space and resave\nwith the same filename.");
}

void save_cut(char *filename,int compressit)
{
  FILE *fp;
  char *name;

  strcpy((*glob_data)->full_filename,filename);  
  name=strrchr(filename,'\\');
  if (name==NULL) strcpy((*glob_data)->filename,filename);
  else strcpy((*glob_data)->filename,name+1);
  *(strrchr((*glob_data)->filename,'.'))='\0';
  fp = fopen(filename,"wb");
  if (fp==NULL) mprintf("Couldn't open CUT file to write");
  save_cut_fp(fp,compressit);
  fclose(fp);
}

static void save_gen2d_fp(FILE *fp,int compressit)
{
  Byte *compr;
  uLong comprLen;
  int err,len;
  int compressed=1;
    fwrite(&((*glob_data)->x_type),sizeof((*glob_data)->x_type),1,fp);
    fwrite(&((*glob_data)->y_type),sizeof((*glob_data)->y_type),1,fp);
    fwrite(&((*glob_data)->size),sizeof((*glob_data)->size),1,fp);
    switch ((*glob_data)->y_type)
    {
        case GEN2D_FLOAT:
            len= (*glob_data)->size*sizeof(float);
            if (compressit)
            {
              comprLen=len+DATA_MAX_COMP_ADD;
              compr=(Byte *)malloc(comprLen);
              err = compress(compr, &comprLen, (const Bytef*)((*glob_data)->yf), len);
              if (err !=Z_OK)
              {
                mprintf("Had trouble compressing... will save uncompressed");
                compressed=0;
                free(compr);
                compr=(Byte *)((*glob_data)->yf);
                comprLen=len;
              }
            }
            else
            {
              compressed=0;
              compr=(Byte *)((*glob_data)->yf);
              comprLen=len;
            }
            fwrite(&compressed,sizeof(int),1,fp);
            fwrite(&comprLen,sizeof(uLong),1,fp);
            fwrite(compr,sizeof(Byte),comprLen,fp);
            if (compressed) free(compr);    
            break;
    }
}    

void save_cut_fp(FILE *fp,int compressit)
{
  int i;
  int version, magic;

  wait_cursor();
  version=CUT_DATA_VERSION;
  magic=FILE_MAGIC_CUT;
  fwrite(&magic,sizeof(magic),1,fp);
  fwrite(&version,sizeof(version),1,fp);
  
  fwrite(&(*glob_data)->type,sizeof((*glob_data)->type),1,fp);
  fwrite(&(*glob_data)->type2d,sizeof((*glob_data)->type2d),1,fp);
  save_gen2d_fp(fp,compressit);  

  fwrite(&(*glob_data)->comment.size,sizeof((*glob_data)->comment.size),1,fp);
  fwrite((*glob_data)->comment.ptr,sizeof(char),(size_t)(*glob_data)->comment.size,fp);
  fwrite(&(*glob_data)->step,sizeof((*glob_data)->step),1,fp);
  fwrite(&(*glob_data)->x,sizeof((*glob_data)->x),1,fp);
  fwrite(&(*glob_data)->y,sizeof((*glob_data)->y),1,fp);
  fwrite(&(*glob_data)->z,sizeof((*glob_data)->z),1,fp);
  fwrite(&(*glob_data)->x_gain,sizeof((*glob_data)->x_gain),1,fp);
  fwrite(&(*glob_data)->y_gain,sizeof((*glob_data)->y_gain),1,fp);
  fwrite(&(*glob_data)->z_gain,sizeof((*glob_data)->z_gain),1,fp);
  fwrite(&(*glob_data)->z_freq,sizeof((*glob_data)->z_freq),1,fp);
  

  fwrite(&(*glob_data)->x_range,sizeof((*glob_data)->x_range),1,fp);
  fwrite(&(*glob_data)->y_range,sizeof((*glob_data)->y_range),1,fp);
  fwrite(&((*glob_data)->sample_type.size),sizeof((*glob_data)->sample_type.size),1,fp);
  fwrite((*glob_data)->sample_type.ptr,sizeof(char),(size_t)(*glob_data)->sample_type.size,fp);
  
  fwrite(&(*glob_data)->x_offset_range,sizeof((*glob_data)->x_offset_range),1,fp);  
  fwrite(&(*glob_data)->y_offset_range,sizeof((*glob_data)->y_offset_range),1,fp);  
  fwrite(&(*glob_data)->bias,sizeof((*glob_data)->bias),1,fp);  
  fwrite(&(*glob_data)->bias_range,sizeof((*glob_data)->bias_range),1,fp);  
  fwrite(&(*glob_data)->scan_dir,sizeof((*glob_data)->scan_dir),1,fp);  
  fwrite(&(*glob_data)->i_setpoint,sizeof((*glob_data)->i_setpoint),1,fp);  
  fwrite(&(*glob_data)->i_set_range,sizeof((*glob_data)->i_set_range),1,fp);  
  fwrite(&(*glob_data)->dosed_langmuir,sizeof((*glob_data)->dosed_langmuir),1,fp);  
  fwrite(&(*glob_data)->amp_gain,sizeof((*glob_data)->amp_gain),1,fp);  
  fwrite(&(*glob_data)->step_delay,sizeof((*glob_data)->step_delay),1,fp);  
  fwrite(&(*glob_data)->inter_step_delay,sizeof((*glob_data)->inter_step_delay),1,fp);  
  fwrite(&(*glob_data)->digital_feedback,sizeof((*glob_data)->digital_feedback),1,fp);  
  fwrite(&(*glob_data)->tip_spacing,sizeof((*glob_data)->tip_spacing),1,fp);  
  fwrite(&(*glob_data)->inter_line_delay,sizeof((*glob_data)->inter_line_delay),1,fp);  
  fwrite(&(*glob_data)->scan_num,sizeof((*glob_data)->scan_num),1,fp);  
  fwrite(&(*glob_data)->scan_feedback,sizeof((*glob_data)->scan_feedback),1,fp);  
  fwrite(&(*glob_data)->read_ch,sizeof((*glob_data)->read_ch),1,fp);  
  fwrite(&(*glob_data)->crash_protection,sizeof((*glob_data)->crash_protection),1,fp);  
  fwrite(&(*glob_data)->overshoot,sizeof((*glob_data)->overshoot),1,fp);  
  fwrite(&(*glob_data)->overshoot_percent,sizeof((*glob_data)->overshoot_percent),1,fp);  
  fwrite(&(*glob_data)->overshoot_wait1,sizeof((*glob_data)->overshoot_wait1),1,fp);  
  fwrite(&(*glob_data)->overshoot_wait2,sizeof((*glob_data)->overshoot_wait2),1,fp);  
  fwrite(&((*glob_data)->dosed_type.size),sizeof((*glob_data)->dosed_type.size),1,fp);
  fwrite((*glob_data)->dosed_type.ptr,sizeof(char),(size_t)(*glob_data)->dosed_type.size,fp);
  
  fwrite(&((*glob_data)->temperature),sizeof((*glob_data)->temperature),1,fp);
  fwrite(&((*glob_data)->dep_filename),sizeof((*glob_data)->dep_filename[0])*FILE_NAME_SIZE,1,fp);
  
  fwrite(&((*glob_data)->version),sizeof((*glob_data)->version),1,fp);
  
  fwrite(&((*glob_data)->read_feedback),sizeof((*glob_data)->read_feedback),1,fp);
  fwrite(&((*glob_data)->scan_dither0),sizeof((*glob_data)->scan_dither0),1,fp);
  fwrite(&((*glob_data)->scan_dither1),sizeof((*glob_data)->scan_dither1),1,fp);
  fwrite(&((*glob_data)->dither_wait),sizeof((*glob_data)->dither_wait),1,fp);
  fwrite(&((*glob_data)->dither0),sizeof((*glob_data)->dither0),1,fp);
  fwrite(&((*glob_data)->dither1),sizeof((*glob_data)->dither1),1,fp);
  fwrite(&((*glob_data)->min_x),sizeof((*glob_data)->min_x),1,fp);
  fwrite(&((*glob_data)->max_x),sizeof((*glob_data)->max_x),1,fp);
  fwrite(&((*glob_data)->cut_x1),sizeof((*glob_data)->cut_x1),1,fp);
  fwrite(&((*glob_data)->cut_x2),sizeof((*glob_data)->cut_x2),1,fp);
  fwrite(&((*glob_data)->cut_y1),sizeof((*glob_data)->cut_y1),1,fp);
  i=fwrite(&((*glob_data)->cut_y2),sizeof((*glob_data)->cut_y2),1,fp);
  
  arrow_cursor();
  if (i<1)
    mprintf("Disk is full. Data not saved properly!\n Free disk space and resave\nwith the same filename.");
}

void save_dep(char *filename)
{
  FILE *fp;
  char *name;

  strcpy((*glob_data)->full_filename,filename);  
  name=strrchr(filename,'\\');
  if (name==NULL) strcpy((*glob_data)->filename,filename);
  else strcpy((*glob_data)->filename,name+1);
  *(strrchr((*glob_data)->filename,'.'))='\0';
  strcpy((*glob_data)->full_filename,filename);  
  fp = fopen(filename,"wb");
  if (fp==NULL) mprintf("Couldn't open DEP file to write");
  fwrite(&(*glob_data)->version,sizeof((*glob_data)->version),1,fp);
  fwrite(&(*glob_data)->comment.size,sizeof((*glob_data)->comment.size),1,fp);
  fwrite((*glob_data)->comment.ptr,sizeof(char),(size_t)(*glob_data)->comment.size,fp);
  fwrite(&((*glob_data)->sample_type.size),sizeof((*glob_data)->sample_type.size),1,fp);
  fwrite((*glob_data)->sample_type.ptr,sizeof(char),(size_t)(*glob_data)->sample_type.size,fp);
  fwrite(&((*glob_data)->dosed_type.size),sizeof((*glob_data)->dosed_type.size),1,fp);
  fwrite((*glob_data)->dosed_type.ptr,sizeof(char),(size_t)(*glob_data)->dosed_type.size,fp);
  fwrite(&(*glob_data)->size,sizeof((*glob_data)->size),1,fp);
  fwrite((*glob_data)->time2d,sizeof(*((*glob_data)->time2d)),(*glob_data)->size,fp);  
  fwrite((*glob_data)->data2d,sizeof(*((*glob_data)->data2d)),(*glob_data)->size,fp);  
  fwrite(&(*glob_data)->bias,sizeof((*glob_data)->bias),1,fp);  
  fwrite(&(*glob_data)->bias_range,sizeof((*glob_data)->bias_range),1,fp);  
  fwrite(&(*glob_data)->scan_dir,sizeof((*glob_data)->scan_dir),1,fp);  
  fwrite(&(*glob_data)->i_setpoint,sizeof((*glob_data)->i_setpoint),1,fp);  
  fwrite(&(*glob_data)->i_set_range,sizeof((*glob_data)->i_set_range),1,fp);  
  fwrite(&(*glob_data)->dosed_langmuir,sizeof((*glob_data)->dosed_langmuir),1,fp);  
  fwrite(&(*glob_data)->amp_gain,sizeof((*glob_data)->amp_gain),1,fp);  
  fwrite(&(*glob_data)->z_gain,sizeof((*glob_data)->z_gain),1,fp);
  fwrite(&(*glob_data)->dosed_langmuir,sizeof((*glob_data)->dosed_langmuir),1,fp);  
  fwrite(&(*glob_data)->temperature,sizeof((*glob_data)->temperature),1,fp);  
  fwrite(&(*glob_data)->move_time,sizeof((*glob_data)->move_time),1,fp);  
  fwrite(&(*glob_data)->write_time,sizeof((*glob_data)->write_time),1,fp);  
  fwrite(&(*glob_data)->move_bias,sizeof((*glob_data)->move_bias),1,fp);  
  fwrite(&(*glob_data)->write_bias,sizeof((*glob_data)->write_bias),1,fp);  
  fwrite(&(*glob_data)->move_i_setpoint,sizeof((*glob_data)->move_i_setpoint),1,fp);  
  fwrite(&(*glob_data)->write_i_setpoint,sizeof((*glob_data)->write_i_setpoint),1,fp);  
  fwrite(&(*glob_data)->pulse_duration,sizeof((*glob_data)->pulse_duration),1,fp);  
  fwrite(&(*glob_data)->bias_speed,sizeof((*glob_data)->bias_speed),1,fp);  
  fwrite(&(*glob_data)->current_speed,sizeof((*glob_data)->current_speed),1,fp);  
  fwrite(&(*glob_data)->z_offset_speed,sizeof((*glob_data)->z_offset_speed),1,fp);  
  fwrite(&(*glob_data)->measure_before,sizeof((*glob_data)->measure_before),1,fp);  
  fwrite(&(*glob_data)->scan_feedback,sizeof((*glob_data)->scan_feedback),1,fp);  
  fwrite(&(*glob_data)->delta_z_offset,sizeof((*glob_data)->delta_z_offset),1,fp);  
  fwrite(&(*glob_data)->z_offset_move,sizeof((*glob_data)->z_offset_move),1,fp);  
  fwrite(&(*glob_data)->z_offset_crash_protect,
    sizeof((*glob_data)->z_offset_crash_protect),1,fp);  
  fwrite(&(*glob_data)->measure_i_once,sizeof((*glob_data)->measure_i_once),1,fp);  
  fwrite(&(*glob_data)->measure_i_wait,sizeof((*glob_data)->measure_i_wait),1,fp);  
  fwrite(&(*glob_data)->feedback_wait,sizeof((*glob_data)->feedback_wait),1,fp);  
  fwrite(&(*glob_data)->measure_i,sizeof((*glob_data)->measure_i),1,fp);  
  fwrite(&(*glob_data)->measure_z,sizeof((*glob_data)->measure_z),1,fp);  
  fwrite(&(*glob_data)->bias_speed_step,sizeof((*glob_data)->bias_speed_step),1,fp);  
/* version 4 */
  fwrite(&(*glob_data)->scan_num,sizeof((*glob_data)->scan_num),1,fp);  
  fwrite(&(*glob_data)->dep_pulse_ignore_initial,sizeof((*glob_data)->dep_pulse_ignore_initial),1,fp);  
  fwrite(&(*glob_data)->dep_pulse_change_time,sizeof((*glob_data)->dep_pulse_change_time),1,fp);  
  fwrite(&(*glob_data)->dep_pulse_avg_stop_pts,sizeof((*glob_data)->dep_pulse_avg_stop_pts),1,fp);  
  fwrite(&(*glob_data)->dep_pulse_after_time,sizeof((*glob_data)->dep_pulse_after_time),1,fp);  
  fwrite(&(*glob_data)->dep_pulse_percent_change,sizeof((*glob_data)->dep_pulse_percent_change),1,fp);  
  fwrite(&(*glob_data)->dep_pulse_duration_mode,sizeof((*glob_data)->dep_pulse_duration_mode),1,fp);  
/* version 5 */
  fwrite(&(*glob_data)->dep_pulse_z_after,
    sizeof((*glob_data)->dep_pulse_z_after),1,fp);  
  fwrite(&(*glob_data)->dep_pulse_z_offset_overshoot,
    sizeof((*glob_data)->dep_pulse_z_offset_overshoot),1,fp);  
  fwrite(&(*glob_data)->dep_pulse_z_offset_overshoot_percentage,
    sizeof((*glob_data)->dep_pulse_z_offset_overshoot_percentage),1,fp);  
  fwrite(&(*glob_data)->dep_pulse_feedback_after, 
    sizeof((*glob_data)->dep_pulse_feedback_after),1,fp);  
  fclose(fp);
}

int load_dep(char *filename)
{
  FILE *fp;
  char *name;

  (*glob_data)->saved=1;

  strcpy((*glob_data)->full_filename,filename);  
  name=strrchr(filename,'\\');
  if (name==NULL) strcpy((*glob_data)->filename,filename);
  else strcpy((*glob_data)->filename,name+1);
  *(strrchr((*glob_data)->filename,'.'))='\0';

  fp = fopen(filename,"rb");
  if (fp==NULL)
  {
    return 0;
  }
  (*glob_data)->type=DATATYPE_2D;
  (*glob_data)->valid=1;
  fread(&(*glob_data)->version,sizeof((*glob_data)->version),1,fp);
  fread(&(*glob_data)->comment.size,sizeof((*glob_data)->comment.size),1,fp);
  fread((*glob_data)->comment.ptr,sizeof(char),(size_t)(*glob_data)->comment.size,fp);
  (*glob_data)->comment.ptr[(*glob_data)->comment.size]='\0';
  fread(&((*glob_data)->sample_type.size),sizeof((*glob_data)->sample_type.size),1,fp);
  fread((*glob_data)->sample_type.ptr,sizeof(char),(size_t)(*glob_data)->sample_type.size,fp);
  (*glob_data)->sample_type.ptr[(*glob_data)->sample_type.size]='\0';
  fread(&((*glob_data)->dosed_type.size),sizeof((*glob_data)->dosed_type.size),1,fp);
  fread((*glob_data)->dosed_type.ptr,sizeof(char),(size_t)(*glob_data)->dosed_type.size,fp);
  (*glob_data)->dosed_type.ptr[(*glob_data)->dosed_type.size]='\0';
  fread(&(*glob_data)->size,sizeof((*glob_data)->size),1,fp);
  free((*glob_data)->time2d);
  free((*glob_data)->data2d);
  (*glob_data)->time2d=(float *) malloc((*glob_data)->size*sizeof(float));
  (*glob_data)->data2d=(unsigned short *) malloc((*glob_data)->size*sizeof(unsigned short));
  fread((*glob_data)->time2d,sizeof(*((*glob_data)->time2d)),(*glob_data)->size,fp);  
  fread((*glob_data)->data2d,sizeof(*((*glob_data)->data2d)),(*glob_data)->size,fp);  
  fread(&(*glob_data)->bias,sizeof((*glob_data)->bias),1,fp);  
  fread(&(*glob_data)->bias_range,sizeof((*glob_data)->bias_range),1,fp);  
  fread(&(*glob_data)->scan_dir,sizeof((*glob_data)->scan_dir),1,fp);  
  fread(&(*glob_data)->i_setpoint,sizeof((*glob_data)->i_setpoint),1,fp);  
  fread(&(*glob_data)->i_set_range,sizeof((*glob_data)->i_set_range),1,fp);  
  fread(&(*glob_data)->dosed_langmuir,sizeof((*glob_data)->dosed_langmuir),1,fp);  
  fread(&(*glob_data)->amp_gain,sizeof((*glob_data)->amp_gain),1,fp);  
  fread(&(*glob_data)->z_gain,sizeof((*glob_data)->z_gain),1,fp);
  fread(&(*glob_data)->dosed_langmuir,sizeof((*glob_data)->dosed_langmuir),1,fp);  
  fread(&(*glob_data)->temperature,sizeof((*glob_data)->temperature),1,fp);  
  fread(&(*glob_data)->move_time,sizeof((*glob_data)->move_time),1,fp);  
  fread(&(*glob_data)->write_time,sizeof((*glob_data)->write_time),1,fp);  
  fread(&(*glob_data)->move_bias,sizeof((*glob_data)->move_bias),1,fp);  
  fread(&(*glob_data)->write_bias,sizeof((*glob_data)->write_bias),1,fp);  
  fread(&(*glob_data)->move_i_setpoint,sizeof((*glob_data)->move_i_setpoint),1,fp);  
  fread(&(*glob_data)->write_i_setpoint,sizeof((*glob_data)->write_i_setpoint),1,fp);  
  fread(&(*glob_data)->pulse_duration,sizeof((*glob_data)->pulse_duration),1,fp);  
  fread(&(*glob_data)->bias_speed,sizeof((*glob_data)->bias_speed),1,fp);  
  fread(&(*glob_data)->current_speed,sizeof((*glob_data)->current_speed),1,fp);  
  fread(&(*glob_data)->z_offset_speed,sizeof((*glob_data)->z_offset_speed),1,fp);  
  fread(&(*glob_data)->measure_before,sizeof((*glob_data)->measure_before),1,fp);  
  fread(&(*glob_data)->scan_feedback,sizeof((*glob_data)->scan_feedback),1,fp);  
  fread(&(*glob_data)->delta_z_offset,sizeof((*glob_data)->delta_z_offset),1,fp);  
  if ((*glob_data)->version>1)
  {
      fread(&(*glob_data)->z_offset_move,sizeof((*glob_data)->z_offset_move),1,fp);  
      fread(&(*glob_data)->z_offset_crash_protect,
        sizeof((*glob_data)->z_offset_crash_protect),1,fp);  
      fread(&(*glob_data)->measure_i_once,sizeof((*glob_data)->measure_i_once),1,fp);  
      fread(&(*glob_data)->measure_i_wait,sizeof((*glob_data)->measure_i_wait),1,fp);  
      fread(&(*glob_data)->feedback_wait,sizeof((*glob_data)->feedback_wait),1,fp);  
      if ((*glob_data)->version>2)
      {
          fread(&(*glob_data)->measure_i,sizeof((*glob_data)->measure_i),1,fp);  
          fread(&(*glob_data)->measure_z,sizeof((*glob_data)->measure_z),1,fp);  
          fread(&(*glob_data)->bias_speed_step,sizeof((*glob_data)->bias_speed_step),1,fp);  
      }
      if ((*glob_data)->version>3)
      {
          fread(&(*glob_data)->scan_num,sizeof((*glob_data)->scan_num),1,fp);  
          fread(&(*glob_data)->dep_pulse_ignore_initial,sizeof((*glob_data)->dep_pulse_ignore_initial),1,fp);  
          fread(&(*glob_data)->dep_pulse_change_time,sizeof((*glob_data)->dep_pulse_change_time),1,fp);  
          fread(&(*glob_data)->dep_pulse_avg_stop_pts,sizeof((*glob_data)->dep_pulse_avg_stop_pts),1,fp);  
          fread(&(*glob_data)->dep_pulse_after_time,sizeof((*glob_data)->dep_pulse_after_time),1,fp);  
          fread(&(*glob_data)->dep_pulse_percent_change,sizeof((*glob_data)->dep_pulse_percent_change),1,fp);  
          fread(&(*glob_data)->dep_pulse_duration_mode,sizeof((*glob_data)->dep_pulse_duration_mode),1,fp);  
      }
      if ((*glob_data)->version>4)
      {
          fread(&(*glob_data)->dep_pulse_z_after,
            sizeof((*glob_data)->dep_pulse_z_after),1,fp);  
          fread(&(*glob_data)->dep_pulse_z_offset_overshoot,
            sizeof((*glob_data)->dep_pulse_z_offset_overshoot),1,fp);  
          fread(&(*glob_data)->dep_pulse_z_offset_overshoot_percentage,
            sizeof((*glob_data)->dep_pulse_z_offset_overshoot_percentage),1,fp);  
          fread(&(*glob_data)->dep_pulse_feedback_after, 
            sizeof((*glob_data)->dep_pulse_feedback_after),1,fp);  
      }
                        
        
  }
  if ((*glob_data)->version<2)
  {
    (*glob_data)->z_offset_move=0;
    (*glob_data)->z_offset_crash_protect=0;
    (*glob_data)->measure_i_once=0;
    (*glob_data)->measure_i_wait=0;
    (*glob_data)->feedback_wait=0;
    (*glob_data)->measure_i=(*glob_data)->measure_z=0;
    (*glob_data)->bias_speed_step=0;
  }
  if ((*glob_data)->version<3)
  {
      (*glob_data)->measure_i=(*glob_data)->measure_z=0;
      (*glob_data)->bias_speed_step=0;
  }
  if ((*glob_data)->version<4)
  {
      (*glob_data)->scan_num=1;
      (*glob_data)->dep_pulse_ignore_initial=0;
      (*glob_data)->dep_pulse_change_time=1000;
      (*glob_data)->dep_pulse_avg_stop_pts=10;
      (*glob_data)->dep_pulse_after_time=100;
      (*glob_data)->dep_pulse_percent_change=10;
      (*glob_data)->dep_pulse_duration_mode=DEP_PULSE_DURATION_FIXED;
  }
  if ((*glob_data)->version<5)
  {
    (*glob_data)->dep_pulse_z_after=0;
    (*glob_data)->dep_pulse_z_offset_overshoot=0;
    (*glob_data)->dep_pulse_z_offset_overshoot_percentage= 0;
    (*glob_data)->dep_pulse_feedback_after=0 ;
  }
    

  fclose(fp);
  return(1);
}

void load_init(char *fname)
{
  FILE *fp;
  int scan_num; /* for compatibility only */

  fp = fopen(fname,"rb");
  if (fp==NULL)
  {
    mprintf("Couldn't open INIT file");
  }
  fread(&ch,sizeof(ch),1,fp);
  fread(&ch_data,sizeof(ch_data),1,fp);
  fread(dac_data,sizeof(unsigned int),16,fp);
  fread(&dac_ranges,sizeof(dac_ranges),1,fp);
  fread(&out_range,sizeof(out_range),1,fp);
  fread(&input_ch,sizeof(input_ch),1,fp);
  fread(&out1,sizeof(out1),1,fp);
  fread(&coarse_z,sizeof(coarse_z),1,fp);
  fread(&tip_accel,sizeof(tip_accel),1,fp);
  fread(&num_steps,sizeof(num_steps),1,fp);
  fread(&coarse_x,sizeof(coarse_x),1,fp);
  fread(&coarse_y,sizeof(coarse_y),1,fp);
  fread(&wave,sizeof(wave),1,fp);
  fread(&scan_x,sizeof(scan_x),1,fp);
  fread(&scan_y,sizeof(scan_y),1,fp);
  fread(&scan_z,sizeof(scan_z),1,fp);
  fread(&scan_step,sizeof(scan_step),1,fp);
  fread(&scan_size,sizeof(scan_size),1,fp);
  fread(&scan_num,sizeof(scan_num),1,fp);
  fread(&scan_x_gain,sizeof(scan_x_gain),1,fp);
  fread(&scan_y_gain,sizeof(scan_y_gain),1,fp);
  fread(&scan_z_gain,sizeof(scan_z_gain),1,fp);
  fread(&scan_scale,sizeof(scan_scale),1,fp);
  fread(&scan_freq,sizeof(scan_freq),1,fp);
  fread(&image_size,sizeof(image_size),1,fp);
  fread(&image_x_offset,sizeof(image_x_offset),1,fp);
  fread(&image_y_offset,sizeof(image_y_offset),1,fp);
  fread(&tip_gain,sizeof(tip_gain),1,fp);
  fread(&i_setpoint,sizeof(i_setpoint),1,fp);
  fread(&sample_bias,sizeof(sample_bias),1,fp);
  fread(&tip_number,sizeof(tip_number),1,fp);
  fread(&tip_delay,sizeof(tip_delay),1,fp);
  fread(&tip_x_step,sizeof(tip_x_step),1,fp);
  fread(&tip_zo_step,sizeof(tip_zo_step),1,fp);
  fread(&z_offset_ch,sizeof(z_offset_ch),1,fp);
  fread(&bit16,sizeof(bit16),1,fp);


  fclose(fp);
}

void save_init(char *fname)
{
  FILE *fp;
  int scan_num; /* for compatibility only */

  fp = fopen(fname,"wb");
  if (fp==NULL) mprintf("Couldn't open INIT file to write");
  fwrite(&ch,sizeof(ch),1,fp);
  fwrite(&ch_data,sizeof(ch_data),1,fp);
  fwrite(dac_data,sizeof(unsigned int),16,fp);
  fwrite(&dac_ranges,sizeof(dac_ranges),1,fp);
  fwrite(&out_range,sizeof(out_range),1,fp);
  fwrite(&input_ch,sizeof(input_ch),1,fp);
  fwrite(&out1,sizeof(out1),1,fp);
  fwrite(&coarse_z,sizeof(coarse_z),1,fp);
  fwrite(&tip_accel,sizeof(tip_accel),1,fp);
  fwrite(&num_steps,sizeof(num_steps),1,fp);
  fwrite(&coarse_x,sizeof(coarse_x),1,fp);
  fwrite(&coarse_y,sizeof(coarse_y),1,fp);
  fwrite(&wave,sizeof(wave),1,fp);
  fwrite(&scan_x,sizeof(scan_x),1,fp);
  fwrite(&scan_y,sizeof(scan_y),1,fp);
  fwrite(&scan_z,sizeof(scan_z),1,fp);
  fwrite(&scan_step,sizeof(scan_step),1,fp);
  fwrite(&scan_size,sizeof(scan_size),1,fp);
  fwrite(&scan_num,sizeof(scan_num),1,fp);
  fwrite(&scan_x_gain,sizeof(scan_x_gain),1,fp);
  fwrite(&scan_y_gain,sizeof(scan_y_gain),1,fp);
  fwrite(&scan_z_gain,sizeof(scan_z_gain),1,fp);
  fwrite(&scan_scale,sizeof(scan_scale),1,fp);
  fwrite(&scan_freq,sizeof(scan_freq),1,fp);
  fwrite(&image_size,sizeof(image_size),1,fp);
  fwrite(&image_x_offset,sizeof(image_x_offset),1,fp);
  fwrite(&image_y_offset,sizeof(image_y_offset),1,fp);
  fwrite(&tip_gain,sizeof(tip_gain),1,fp);
  fwrite(&i_setpoint,sizeof(i_setpoint),1,fp);
  fwrite(&sample_bias,sizeof(sample_bias),1,fp);
  fwrite(&tip_number,sizeof(tip_number),1,fp);
  fwrite(&tip_delay,sizeof(tip_delay),1,fp);
  fwrite(&tip_x_step,sizeof(tip_x_step),1,fp);
  fwrite(&tip_zo_step,sizeof(tip_zo_step),1,fp);
  fwrite(&z_offset_ch, sizeof(z_offset_ch),1,fp);
  fwrite(&bit16,sizeof(bit16),1,fp);
  fclose(fp);
}

void moh_find_next(char *this_file)
{
	// Finds the next file after this_file, i.e. the file that 
	//		1) is of the same type as this_file
	//		2) is in the same directory as this_file
	//		3) follows this_file
	// Upon exit, this_file will be set to the path + filename of the next file or '\0'
    char path[CURRENT_DIR_MAX];
    char *fname;
    char *temp;
    int result = TRUE;
    int file_type;
	HANDLE current_file = NULL;
	WIN32_FIND_DATA lpffd;
//    struct find_t fileinfo;	OLD_COMPILER

//    MessageBox(GetFocus(),current_dir_stm,"file",MB_OKCANCEL);

    if(*this_file)
    {
        strupr(this_file);   
        temp = strrchr(this_file, '\\');
        file_type = guess_file_type(this_file);
    } else temp = NULL;

    if(temp == NULL) 
    {
        strcpy(path, current_dir_stm);
        strcat(path, "\\");
        strcat(path, this_file);
        strcpy(this_file, path);
        strcpy(path, current_dir_stm);
        strcat(path, "\\*.stm");
        temp=strrchr(this_file, '\\');
        fname = temp + 1;	// set fname to the filename within this_file
    }
    else
    {
        *temp='\0';
        
        strcpy(path, this_file);
        switch(file_type)
        {
            case FTYPE_CUT:
                strcat(path, "\\*.cut");
                break;
            case FTYPE_STM:
                strcat(path, "\\*.stm");
                break;
            case FTYPE_STP:
                strcat(path, "\\*.stp");
                break;
            case FTYPE_DEP:
                strcat(path, "\\*.dep");
                break;     
            case FTYPE_MAS:
                strcat(path, "\\*.mas");
                break;
            case FTYPE_SPC:
                strcat(path, "\\*.spc");
                break;
        }
        *temp = '\\';
        fname = temp + 1;	// set fname to the filename within this_file
    }

//    MessageBox(GetFocus(), path,"path",MB_OKCANCEL);
//    if(_dos_findfirst(path, _A_NORMAL, &fileinfo)) 		OLD_COMPILER
	current_file = FindFirstFile(path, &lpffd);
	if(current_file == NULL)
    {
        *this_file = '\0';
        return;
    }
    else 
    {
        while(result == TRUE)
        {
			// create a list of all files of the appropriate type that are in this dir
//            insert_list(fileinfo.name);	OLD_COMPILER
//            result = _dos_findnext(&fileinfo);	OLD_COMPILER
		    insert_list(lpffd.cFileName);
			result = FindNextFile(current_file, &lpffd); // function returns TRUE if it succeeds
        }
    }
    
    find_list(fname); // NOTE that fname points to the filename within this_file
    if(*fname == '\0')
    {
        *this_file = '\0';
    }
    
//    file_list=NULL;
    nuke_list();
//    _dos_findclose(&fileinfo);	OLD_COMPILER
	FindClose(current_file);
}

void find_list(char *fname)
{
    FILE_LIST *pos;
    int done=0;
    
    if (*fname=='\0')
    {
        strcpy(fname,file_list->name);
        return;
    }
    else
    {
        pos=file_list;
        while(pos!=NULL && !done)
        {
//            if (!strcmp(fname,pos->name)) KAFKA
            if (!stricmp(fname,pos->name))
            {
                done=1;
                if (pos->next==NULL)
                {
                    *fname='\0';
                }
                else
                {
                    strcpy(fname,(pos->next)->name);
                }
            }
            pos=pos->next;
        }
    }
}

void moh_find_prev(char *this_file)
{
 	// Finds the previous file before this_file, i.e. the file that 
	//		1) is of the same type as this_file
	//		2) is in the same directory as this_file
	//		3) preceeds this_file
	// Upon exit, this_file will be set to the path + filename of the previous file or '\0'
    char path[CURRENT_DIR_MAX];
    char *fname;
    char *temp;
    int result = TRUE;
    int file_type;
	HANDLE current_file = NULL;
	WIN32_FIND_DATA lpffd;
//    struct find_t fileinfo;	OLD_COMPILER
	
//    MessageBox(GetFocus(),current_dir_stm,"file",MB_OKCANCEL);

    if(*this_file)
    {
        strupr(this_file);   
        temp = strrchr(this_file, '\\');
        file_type = guess_file_type(this_file);
    } else return;
    if(temp == NULL) 
    {
        strcpy(path, current_dir_stm);
        strcat(path, "\\");
        strcat(path, this_file);
        strcpy(this_file, path);
        strcpy(path, current_dir_stm);
        strcat(path, "\\*.stm");
        temp=strrchr(this_file, '\\');
        fname = temp + 1;	// set fname to the filename within this_file
        
    }
    else
    {
        *temp='\0';
        
        strcpy(path, this_file);
        switch(file_type)
        {
            case FTYPE_CUT:
                strcat(path, "\\*.cut");
                break;
            case FTYPE_STM:
                strcat(path, "\\*.stm");
                break;
            case FTYPE_STP:
                strcat(path, "\\*.stp");
                break;
            case FTYPE_DEP:
                strcat(path, "\\*.dep");
                break;     
            case FTYPE_MAS:
                strcat(path, "\\*.mas");
                break;
            case FTYPE_SPC:
                strcat(path, "\\*.spc");
                break;
        }
        *temp = '\\';
        fname = temp + 1;	// set fname to the filename within this_file
    }

//    MessageBox(GetFocus(),path,"path",MB_OKCANCEL);

//    if (_dos_findfirst(path,_A_NORMAL, &fileinfo))	OLD_COMPILER
	current_file = FindFirstFile(path, &lpffd);
	if(current_file == NULL)
    {
        *this_file='\0';
        return;
    }
    else 
    {
        while(result == TRUE)
        {
			// create a list of all files of the appropriate type that are in this dir
//            insert_list(fileinfo.name);
//            result = _dos_findnext(&fileinfo);
			  insert_list(lpffd.cFileName);
			  result = FindNextFile(current_file, &lpffd); // returns TRUE if successful
        }
    }
    if(*fname == '\0')
    {
        *this_file = '\0';
        return;
    }
    find_list_prev(fname); // NOTE that fname points to the filename within this_file
    if (*fname == '\0')
    {
        *this_file = '\0';
    }
    
//        file_list=NULL;
    nuke_list();
//    _dos_findclose(&fileinfo);	OLD_COMPILER
	FindClose(current_file);
}

void find_list_prev(char *fname)
{
    FILE_LIST *pos,*ppos;
    int done=0;
    
    if (*fname=='\0')
    {
        strcpy(fname,file_list->name);
        return;
    }
    else
    {
        pos=file_list;
        while(pos!=NULL && !done)
        {
//            if (!strcmp(fname,pos->name)) KAFKA
            if (!stricmp(fname,pos->name))
            {
                done=1;
                if (pos==file_list)
                {
                    *fname='\0';
                }
                else
                {
                    strcpy(fname,ppos->name);
                }
            }
            ppos=pos;
            pos=pos->next;
        }
    }
}

void insert_list(char *fname)
{
    
    FILE_LIST *pos, *temp, *oldpos;
    int done = 0;
        
    if(file_list == NULL)
    {
		// first item in new list ?
        file_list = (FILE_LIST *)malloc(sizeof(FILE_LIST));
        strcpy(file_list->name, fname);
        file_list->next = NULL;

#ifdef DEBUG
        temp_list[0]=file_list;
        sprintf(string,"%d first\n",temp_list[total_files]);
        MessageBox(GetFocus(),string,"test",MB_OKCANCEL);
        total_files=1;
#endif

    }
    else
    {
		// not first item in new list, so insert
//        if(strcmp(fname, file_list->name) < 0) KAFKA
        if(stricmp(fname, file_list->name) < 0)
        {
			// fname is less than the first (lowest) name on the list
            temp = file_list;
            file_list = (FILE_LIST *)malloc(sizeof(FILE_LIST));
            strcpy(file_list->name, fname);
            file_list->next = temp;

#ifdef DEBUG
        temp_list[total_files] = file_list;
        sprintf(string, "%d before first\n", temp_list[total_files]);
        MessageBox(GetFocus(), string, "test", MB_OKCANCEL);
        total_files++;
#endif

        }
        else
        {
            
            pos = file_list->next;
            oldpos = file_list;
            while(pos != NULL && !done)
            {
				// increment pos through the list
//                if(strcmp(fname, pos->name) < 0) KAFKA
                if(stricmp(fname, pos->name) < 0)
                {
					// insert fname if fname < pos->name
                    temp = pos;
                    pos = (FILE_LIST *)malloc(sizeof(FILE_LIST));
                    strcpy(pos->name, fname);
                    pos->next = temp;
                    oldpos->next = pos;
                    done = 1;

#ifdef DEBUG
					temp_list[total_files] = pos;
					sprintf(string, "%d inserted\n", temp_list[total_files]);
					MessageBox(GetFocus(), string, "test", MB_OKCANCEL);
					total_files++;
#endif

                }
                else 
                {
					// otherwise, on to next item
                    oldpos = pos;
                    pos = pos->next;
                }
            } // end of while loop

            if(!done)
            {
				// fname is greater than every name on the list
                oldpos->next = (FILE_LIST *)malloc(sizeof(FILE_LIST));
                strcpy((oldpos->next)->name, fname);
                (oldpos->next)->next = NULL;
#ifdef DEBUG
                temp_list[total_files] = oldpos->next;
                sprintf(string, "%d end %s\n", temp_list[total_files], fname);
                MessageBox(GetFocus(), string, "test", MB_OKCANCEL);
                total_files++;
#endif

            } // end of if(!done)
        }
    }
}    
                
void nuke_list()
{
    FILE_LIST *temp;
    
    while(file_list!=NULL)
    {
        temp=file_list;
        file_list=file_list->next;
        free(temp);
    }
#ifdef DEBUG
    MessageBox(GetFocus(),"done","test",MB_OKCANCEL);
    
    while(total_files)
    {
        
        total_files--;
    sprintf(string,"%d\n",temp_list[total_files]);
    MessageBox(GetFocus(),string,"test",MB_OKCANCEL);
        free((void *) temp_list[total_files]);
    }
    file_list=NULL;
#endif
}

int eatspace(FILE *fp)
{
    int result;
    while(isspace(result=getc(fp)));
    return(result);
}

void init_dirs()
{
    FILE *f;
    
    // set initial_working_dir from current working directory (cwd)
    rel_path_return = (char *) malloc(sizeof(char)*CURRENT_DIR_MAX);
    initial_working_dir = (char *) malloc(sizeof(char)*CURRENT_DIR_MAX);
    if (getcwd(initial_working_dir,CURRENT_DIR_MAX)==NULL)
    {
        mprintf("Could not get cwd!");
    }
    strcat(initial_working_dir,"\\");    
    if (rel_path_return== NULL || initial_working_dir==NULL) exit(1);
    
    // read in all initial/current directory variables from ini file
    f=fopen(INITIAL_DIR_INI,"rb");
    if (f==NULL) mprintf("Couldn't open INITDIR");
/* 1 */
    fgets(current_dir_stm,CURRENT_DIR_MAX,f);
    current_dir_stm[strlen(current_dir_stm)-2]='\0';
    strcpy(initial_dir_stm,current_dir_stm);
/* 2 */
    fgets(current_dir_pal,CURRENT_DIR_MAX,f);
    current_dir_pal[strlen(current_dir_pal)-2]='\0';
    strcpy(initial_dir_pal,current_dir_pal);
/* 3 */
    fgets(current_dir_mas,CURRENT_DIR_MAX,f);
    current_dir_mas[strlen(current_dir_mas)-2]='\0';
    strcpy(initial_dir_mas,current_dir_mas);
/* 4 */
    fgets(current_dir_gif,CURRENT_DIR_MAX,f);
    current_dir_gif[strlen(current_dir_gif)-2]='\0';
    strcpy(initial_dir_gif,current_dir_gif);
/* 5 */
    fgets(current_dir_spc,CURRENT_DIR_MAX,f);
    current_dir_spc[strlen(current_dir_spc)-2]='\0';
    strcpy(initial_dir_spc,current_dir_spc);
/* 6 */
    fgets(current_dir_dep,CURRENT_DIR_MAX,f);
    current_dir_dep[strlen(current_dir_dep)-2]='\0';
    strcpy(initial_dir_dep,current_dir_dep);
/* 7 */
    fgets(current_dir_grd,CURRENT_DIR_MAX,f);
    current_dir_grd[strlen(current_dir_grd)-2]='\0';
    strcpy(initial_dir_grd,current_dir_grd);
/* 8 */
    fgets(current_dir_fig,CURRENT_DIR_MAX,f);
    current_dir_fig[strlen(current_dir_fig)-2]='\0';
    strcpy(initial_dir_fig,current_dir_fig);
/* 9 */
    fgets(current_dir_cut,CURRENT_DIR_MAX,f);
    current_dir_cut[strlen(current_dir_cut)-2]='\0';
    strcpy(initial_dir_cut,current_dir_cut);
/* 10 */
    fgets(current_dir_cnt,CURRENT_DIR_MAX,f);
    current_dir_cnt[strlen(current_dir_cnt)-2]='\0';
    strcpy(initial_dir_cnt,current_dir_cnt);
/* 11 */
    fgets(current_dir_exp,CURRENT_DIR_MAX,f);
    current_dir_exp[strlen(current_dir_exp)-2]='\0';
    strcpy(initial_dir_exp,current_dir_exp);
/* 12 */
    fgets(initial_dir_trc,CURRENT_DIR_MAX,f);
    initial_dir_trc[strlen(initial_dir_trc)-2]='\0';
/* 13 */
    fgets(initial_dir_stp,CURRENT_DIR_MAX,f);
    initial_dir_stp[strlen(initial_dir_stp)-2]='\0';
    strcpy(current_dir_stp,initial_dir_stp);
/* 14 */
    fgets(initial_dir_ps,CURRENT_DIR_MAX,f);
    initial_dir_ps[strlen(initial_dir_ps)-2]='\0';
    strcpy(current_dir_ps,initial_dir_ps);
/* 15 */
    fgets(initial_dir_hop,CURRENT_DIR_MAX,f);
    initial_dir_hop[strlen(initial_dir_hop)-2]='\0';
    strcpy(current_dir_hop,initial_dir_hop);
/* 16 */
    fgets(initial_dir_map,CURRENT_DIR_MAX,f);
    initial_dir_map[strlen(initial_dir_map)-2]='\0';
    strcpy(current_dir_map,initial_dir_map);
    fclose(f);
}
    
void save_grd(char *filename)
{
  FILE *fp;
  fp = fopen(filename,"wb");
  if (fp==NULL) mprintf("Couldn't open GRD file to write");
  save_grd_fp(fp);
  fclose(fp);
}

void save_grd_fp(FILE *fp)
{
  int i;

  fwrite(&grd_data->version,sizeof(grd_data->version),1,fp);
  i=GRID_MAX_LINES;
  fwrite(&i,sizeof(int),1,fp);
  fwrite(&(grd_data->clip),sizeof(grd_data->clip),1,fp);
  fwrite(&(grd_data->clip_type),sizeof(grd_data->clip_type),1,fp);
  fwrite(&(grd_data->clip_y_less),sizeof(grd_data->clip_y_less),1,fp);
  fwrite(&(grd_data->clip_num_pts),sizeof(grd_data->clip_num_pts),1,fp);
    for(i=0;i<grd_data->clip_num_pts;i++)
    {
      fwrite(&(grd_data->x[i]),sizeof(grd_data->x[i]),1,fp);
      fwrite(&(grd_data->y[i]),sizeof(grd_data->y[i]),1,fp);
    }
        
  fwrite(&(grd_data->scan_filename[0]),sizeof(grd_data->scan_filename[0]),FILE_NAME_SIZE,fp);
  for(i=0;i<GRID_MAX_LINES;i++)
  {
    fwrite(&(grd_data->line[i].theta),sizeof(grd_data->line[i].theta),1,fp);
    fwrite(&(grd_data->line[i].r),sizeof(grd_data->line[i].r),1,fp);
    fwrite(&(grd_data->line[i].dist),sizeof(grd_data->line[i].dist),1,fp);
    fwrite(&(grd_data->line[i].hide),sizeof(grd_data->line[i].hide),1,fp);
    fwrite(&(grd_data->line[i].lock),sizeof(grd_data->line[i].lock),1,fp);
  }
}    

void load_grd(char *filename)
{
  FILE *fp;
  fp = fopen(filename,"rb");
  if (fp==NULL) mprintf("Couldn't open GRD file");
  load_grd_fp(fp);
  fclose(fp);
}

void load_grd_fp(FILE *fp)
{
  int i,maxlines;

  fread(&grd_data->version,sizeof(grd_data->version),1,fp);
  
  fread(&maxlines,sizeof(int),1,fp);
  if (grd_data->version>1)
  {
      fread(&(grd_data->clip),sizeof(grd_data->clip),1,fp);
      fread(&(grd_data->clip_type),sizeof(grd_data->clip_type),1,fp);
      fread(&(grd_data->clip_y_less),sizeof(grd_data->clip_type),1,fp);
      if (grd_data->version>2)
      {
        fread(&(grd_data->clip_num_pts),sizeof(grd_data->clip_num_pts),1,fp);
      } else grd_data->clip_num_pts=2;
        for(i=0;i<grd_data->clip_num_pts;i++)
        {
          fread(&(grd_data->x[i]),sizeof(grd_data->x[i]),1,fp);
          fread(&(grd_data->y[i]),sizeof(grd_data->y[i]),1,fp);
        }
  }
  else
  {
    grd_data->version=3;
    grd_data->clip=grd_data->clip_y_less=0;
    grd_data->clip_type=GRID_CLIP_TYPE_LINE;
    grd_data->clip_num_pts=2;
  } 
  fread(&(grd_data->scan_filename[0]),sizeof(grd_data->scan_filename[0]),FILE_NAME_SIZE,fp);
/*
sprintf(string,"%d %s ",grd_data->version,grd_data->scan_filename);
mprintf(string);  
*/
  for(i=0;i<maxlines;i++)
  {
    fread(&(grd_data->line[i].theta),sizeof(grd_data->line[i].theta),1,fp);
    fread(&(grd_data->line[i].r),sizeof(grd_data->line[i].r),1,fp);
    fread(&(grd_data->line[i].dist),sizeof(grd_data->line[i].dist),1,fp);
    fread(&(grd_data->line[i].hide),sizeof(grd_data->line[i].hide),1,fp);
    fread(&(grd_data->line[i].lock),sizeof(grd_data->line[i].lock),1,fp);
  }
}    

void save_fig(char *filename)
{
    FILE *fp;
    int file_type=FTYPE_FIG;
    int version=PRINT_DATA_VERSION;
    int i;
    
    PRINT_EL *this_el;
    
    if (print_list!=NULL)
    {
        fp=fopen(filename,"wb");
        if (fp==NULL) mprintf("Couldn't open FIG file to write");
        fwrite(&file_type,sizeof(file_type),1,fp);
        fwrite(&version,sizeof(version),1,fp);
        fwrite(&output_color,sizeof(output_color),1,fp);
        fwrite(&device_res,sizeof(device_res),1,fp);
        this_el=print_list;
        do
        {
            if (print_save_links_as_added && this_el->type==PRINT_TYPE_IMAGE_LINK)
            {
                i=PRINT_TYPE_IMAGE;
                fwrite(&i,sizeof(int),1,fp);

            }
            else fwrite(&this_el->type,sizeof(this_el->type),1,fp);
            switch(this_el->type)
            {
                case PRINT_TYPE_IMAGE_LINK:
                    if (!print_save_links_as_added)
                    {
                        write_print_image_attrib(this_el,fp);
                        break;
                    }
                case PRINT_TYPE_IMAGE:
                    write_print_image_attrib(this_el,fp);
                    glob_data=&(this_el->print_image);
                    save_image_fp(fp,1);
                    save_pal_fp(fp);
                    grd_data=this_el->print_grid;
                    save_grd_fp(fp);
                    glob_count_data=this_el->count_data;
                    save_cnt_fp(fp);
                    break;
                case PRINT_TYPE_LINE:
                    write_print_line(this_el,fp);
                    break;
                case PRINT_TYPE_TEXT:
                    write_print_text(this_el,fp);
                    break;
		case PRINT_TYPE_BAR:
		    write_bar_attrib(this_el,fp);
                    glob_data=&(this_el->print_image);
                    save_image_fp(fp,1);
                    save_pal_fp(fp);
		    break;
            }
            this_el=this_el->next;
        } while(this_el!=print_list);
    fclose(fp);
    }
}

void write_bar_attrib(PRINT_EL *this_el,FILE *fp)
{
    fwrite(&this_el->x1,sizeof(this_el->x1),1,fp);
    fwrite(&this_el->y1,sizeof(this_el->y1),1,fp);
    fwrite(&this_el->sizex,sizeof(this_el->sizex),1,fp);
    fwrite(&this_el->sizey,sizeof(this_el->sizey),1,fp);
//    fwrite(&(this_el->bar_pal.type),sizeof(this_el->bar_pal.type),1,fp);
}

void write_print_image_attrib(PRINT_EL *this_el,FILE *fp)
{
    fwrite(&this_el->x1,sizeof(this_el->x1),1,fp);
    fwrite(&this_el->y1,sizeof(this_el->y1),1,fp);
    fwrite(&this_el->link,sizeof(this_el->link),1,fp);
    fwrite(&this_el->sizex,sizeof(this_el->sizex),1,fp);
    fwrite(&this_el->sizey,sizeof(this_el->sizey),1,fp);
    fwrite(&this_el->color,sizeof(this_el->color),1,fp);
    fwrite(&this_el->gamma,sizeof(this_el->gamma),1,fp);
/*version 4*/	
    fwrite( &this_el->image_resolution_type, sizeof(this_el->image_resolution_type),1,fp);

/*
    fwrite(&this_el->,sizeof(this_el->),1,fp);
    fwrite(&this_el->,sizeof(this_el->),1,fp);
    fwrite(&this_el->,sizeof(this_el->),1,fp);
    fwrite(&this_el->,sizeof(this_el->),1,fp);
    fwrite(&this_el->,sizeof(this_el->),1,fp);
    fwrite(&this_el->,sizeof(this_el->),1,fp);
    fwrite(&this_el->,sizeof(this_el->),1,fp);
    fwrite(&this_el->,sizeof(this_el->),1,fp);
    fwrite(&this_el->,sizeof(this_el->),1,fp);
    fwrite(&this_el->,sizeof(this_el->),1,fp);
*/
}    

void load_fig(char *filename)
{
    FILE *fp;
    int file_type=FTYPE_FIG;
    int version,type;
    int bitmap_factor=1;
    float min_z,max_z;
    
    PRINT_EL *this_el;
    
    if (print_list==NULL)
    {
        fp=fopen(filename,"rb");
        if (fp==NULL) mprintf("Couldn't open FIG file");
        fread(&file_type,sizeof(file_type),1,fp);
        if (file_type!=FTYPE_FIG)
        {
            mprintf("Wrong file type!");
            return;
        }    
        fread(&version,sizeof(version),1,fp);
        if (version>1)
        {
            fread(&output_color,sizeof(output_color),1,fp);
            fread(&device_res,sizeof(device_res),1,fp);
        }
        file_type=0;
        do
        {

            if (fread(&type,sizeof(type),1,fp)!=1)
            {

                fclose(fp);
                return;
            }
            file_type++;

            this_el=new_print_el(&print_list,type);
            switch(this_el->type)
            {
                case PRINT_TYPE_IMAGE_LINK:
                    read_print_image_attrib(this_el,version,fp);
                    break;
                case PRINT_TYPE_IMAGE:
                    read_print_image_attrib(this_el,version,fp);
//                    load_Data!
                    this_el->print_image=NULL;
                    alloc_data(&(this_el->print_image),DATATYPE_3D,DATA_MAX,GEN2D_NONE,GEN2D_NONE,1);
                    glob_data=&(this_el->print_image);
                    load_image_fp(fp);
    
                    find_min_max(this_el->print_image,&min_z,&max_z);
                    load_pal_fp(fp);
                    this_el->print_bitmap_size=(int *)malloc(sizeof(int));
                    switch (this_el->print_image->pal.type)
                    {
                        case PALTYPE_LOGICAL:
                            bitmap_factor=3;
                            break;
                        case PALTYPE_MAPPED:
                            bitmap_factor=1;
                            break;
                    }
                    this_el->print_bitmap=NULL;
                    calc_bitmap_gen(this_el->print_image,
                        &(this_el->print_bitmap),this_el->print_bitmap_size);
                    this_el->print_grid=(GRID *) malloc(sizeof(GRID));
                    grd_data=this_el->print_grid;
                    load_grd_fp(fp);
                    this_el->count_data=(COUNT_DATA *) malloc(sizeof(COUNT_DATA));
                    init_count_data(this_el->count_data);
                    if (version>2)
                    {
                        glob_count_data=this_el->count_data;
                        load_cnt_fp(fp);
                    }
                    break;
                case PRINT_TYPE_LINE:
                    read_print_line(this_el,version,fp);
                    break;
                case PRINT_TYPE_TEXT:
                    read_print_text(this_el,version,fp);
                    break;
		case PRINT_TYPE_BAR:
		    read_print_bar_attrib(this_el,version,fp);
                    alloc_data(&(this_el->print_image),DATATYPE_3D,DATA_MAX,GEN2D_NONE,GEN2D_NONE,1);
                    glob_data=&(this_el->print_image);
                    load_image_fp(fp);
		    load_pal_fp(fp);
                    this_el->bar_pal.type = this_el->print_image->pal.type;
		    calc_print_pal_bitmap(this_el);
		    break;
            }
        } while(1);
    fclose(fp);
    }
}

void read_print_bar_attrib(PRINT_EL *this_el,int version,FILE *fp)
{
    fread(&this_el->x1,sizeof(this_el->x1),1,fp);
    fread(&this_el->y1,sizeof(this_el->y1),1,fp);
    fread(&this_el->sizex,sizeof(this_el->sizex),1,fp);
    fread(&this_el->sizey,sizeof(this_el->sizey),1,fp);
//    fread(&(this_el->bar_pal.type),sizeof(this_el->bar_pal.type),1,fp);
}

void read_print_image_attrib(PRINT_EL *this_el,int version,FILE *fp)
{
    fread(&this_el->x1,sizeof(this_el->x1),1,fp);
    fread(&this_el->y1,sizeof(this_el->y1),1,fp);
    fread(&this_el->link,sizeof(this_el->link),1,fp);
    fread(&this_el->sizex,sizeof(this_el->sizex),1,fp);
    fread(&this_el->sizey,sizeof(this_el->sizey),1,fp);
    fread(&this_el->color,sizeof(this_el->color),1,fp);
    fread(&this_el->gamma,sizeof(this_el->gamma),1,fp);

    if( version > 3) {
	fread( &this_el->image_resolution_type, sizeof(this_el->image_resolution_type),1,fp);
    }
    else {
	this_el->image_resolution_type = PRINT_IMAGE_RES_SMOOTH;
    }

/*
    fread(&this_el->,sizeof(this_el->),1,fp);
    fread(&this_el->,sizeof(this_el->),1,fp);
    fread(&this_el->,sizeof(this_el->),1,fp);
    fread(&this_el->,sizeof(this_el->),1,fp);
    fread(&this_el->,sizeof(this_el->),1,fp);
    fread(&this_el->,sizeof(this_el->),1,fp);
    fread(&this_el->,sizeof(this_el->),1,fp);
    fread(&this_el->,sizeof(this_el->),1,fp);
    fread(&this_el->,sizeof(this_el->),1,fp);
    fread(&this_el->,sizeof(this_el->),1,fp);
*/
}    

void write_print_text(PRINT_EL *this_el,FILE *fp)
{
    int text_length;
    
    text_length=strlen(this_el->text);
    fwrite(&this_el->x1,sizeof(this_el->x1),1,fp);
    fwrite(&this_el->y1,sizeof(this_el->y1),1,fp);
    fwrite(&this_el->color,sizeof(this_el->color),1,fp);
    fwrite(&text_length,sizeof(text_length),1,fp);
    fwrite(this_el->text,sizeof(char),text_length,fp);
    fwrite(&this_el->text_just,sizeof(this_el->text_just),1,fp);
    fwrite(&this_el->print_font,sizeof(this_el->print_font),1,fp);
    fwrite(&this_el->font_size,sizeof(this_el->font_size),1,fp);
    fwrite(&this_el->angle,sizeof(this_el->angle),1,fp);

}

void read_print_text(PRINT_EL *this_el,int version,FILE *fp)
{
    int text_length;
    
    fread(&this_el->x1,sizeof(this_el->x1),1,fp);
    fread(&this_el->y1,sizeof(this_el->y1),1,fp);
    fread(&this_el->color,sizeof(this_el->color),1,fp);
    fread(&text_length,sizeof(text_length),1,fp);
    fread(this_el->text,sizeof(char),text_length,fp);
    this_el->text[text_length]='\0';
    fread(&this_el->text_just,sizeof(this_el->text_just),1,fp);
    fread(&this_el->print_font,sizeof(this_el->print_font),1,fp);
    fread(&this_el->font_size,sizeof(this_el->font_size),1,fp);
    fread(&this_el->angle,sizeof(this_el->angle),1,fp);

}

void write_print_line(PRINT_EL *this_el,FILE *fp)
{
    fwrite(&this_el->x1,sizeof(this_el->x1),1,fp);
    fwrite(&this_el->y1,sizeof(this_el->y1),1,fp);
    fwrite(&this_el->x2,sizeof(this_el->x2),1,fp);
    fwrite(&this_el->y2,sizeof(this_el->y2),1,fp);
    fwrite(&this_el->color,sizeof(this_el->color),1,fp);
    fwrite(&this_el->arrow_width,sizeof(this_el->arrow_width),1,fp);
    fwrite(&this_el->arrow_head,sizeof(this_el->arrow_head),1,fp);
    fwrite(&this_el->arrow_head_type,sizeof(this_el->arrow_head_type),1,fp);

}

void read_print_line(PRINT_EL *this_el,int version,FILE *fp)
{
    fread(&this_el->x1,sizeof(this_el->x1),1,fp);
    fread(&this_el->y1,sizeof(this_el->y1),1,fp);
    fread(&this_el->x2,sizeof(this_el->x2),1,fp);
    fread(&this_el->y2,sizeof(this_el->y2),1,fp);
    fread(&this_el->color,sizeof(this_el->color),1,fp);
    fread(&this_el->arrow_width,sizeof(this_el->arrow_width),1,fp);
    fread(&this_el->arrow_head,sizeof(this_el->arrow_head),1,fp);
    if (version>1)
    {
        fread(&this_el->arrow_head_type,sizeof(this_el->arrow_head_type),1,fp);
    }
    else
    {
        this_el->arrow_head_type=PRINT_ARROW_HEAD_DEF;
    }
    
        

}

void save_pal(char *fname)
{
    FILE *fp;
    fp = fopen(fname,"wb");
    if (fp == NULL) mprintf("Couldn't open PAL file to write");
    save_pal_fp(fp);
    fclose(fp);
}

void save_pal_fp(FILE *fp)
{
    int i;
    int version, magic;
    LOGPAL_EL *this;
  
    version=PAL_DATA_VERSION;
    magic=FILE_MAGIC_PAL;
    fwrite(&magic,sizeof(magic),1,fp);
    fwrite(&version,sizeof(version),1,fp);
    
    fwrite(&(*glob_data)->pal.type,sizeof((*glob_data)->pal.type),1,fp);
    switch((*glob_data)->pal.type)
    {
        case PALTYPE_MAPPED:
            for(i=0;i<LOGPALSIZE;i++)
            {
                fwrite(&(*glob_data)->pal.dacbox[i*3],
                    sizeof((*glob_data)->pal.dacbox[0]),1,fp);
                fwrite(&(*glob_data)->pal.dacbox[i*3+1],
                    sizeof((*glob_data)->pal.dacbox[0]),1,fp);
                fwrite(&(*glob_data)->pal.dacbox[i*3+2],
                    sizeof((*glob_data)->pal.dacbox[0]),1,fp);
            }
            break;
        case PALTYPE_LOGICAL:
            this=(*glob_data)->pal.logpal;
            do
            {
                fwrite(&(this->color.r),sizeof(this->color.r),1,fp);
                fwrite(&(this->color.g),sizeof(this->color.g),1,fp);
                fwrite(&(this->color.b),sizeof(this->color.b),1,fp);
                fwrite(&(this->index),sizeof(this->index),1,fp);
                fwrite(&(this->equalized),sizeof(this->equalized),1,fp);
                fwrite(&(this->gamma),sizeof(this->gamma),1,fp);
                this=this->next;
            } while(this!=(*glob_data)->pal.logpal);                    
            i=-1;
            fwrite(&(i),sizeof(i),1,fp);
            break;
    }
}

void load_pal(char *fname)
{
    FILE *fp;
    int magic;
    
    fp = fopen(fname,"rb");
    if (fp==NULL) mprintf("Couldn't open PAL file");
    fread(&magic,sizeof(magic),1,fp);
    if (magic!=FILE_MAGIC_PAL)
    {
        fclose(fp);
        load_old_pal(fname);
        return;
    }
    fclose(fp);
    fp = fopen(fname,"rb");
    if (fp==NULL) mprintf("Couldn't open PAL file");
    load_pal_fp(fp);
    fclose(fp);
}

void load_map( HWND hWnd, char *fname)
{
    FILE *fp;
    int w;
    int error_flag = 0;

    fp = fopen(fname,"rb");
    if (fp==NULL) {
	MessageBox( hWnd, "NULL file pointer", "Debug", MB_OK);
	return;
    }

    if( fscanf( fp, "%d", &sp_scan_count) == EOF) {
	sp_scan_count = 0;
	error_flag = 1;
    }
    else if( sp_scan_count > SP_MAX_MAP_SIZE) {
	sp_scan_count = SP_MAX_MAP_SIZE;
	error_flag = 1;
    }

    for( w=0; w<sp_scan_count; w++) {
	if( fscanf( fp, "%d", &(sp_offsets_map[w].x)) == EOF) {
	    error_flag = 1;
	    sp_scan_count = w;
	    break;
	}
	if( fscanf( fp, "%d", &(sp_offsets_map[w].y)) == EOF) {
	    error_flag = 1;
	    sp_scan_count = w;
	    break;
	}
    }

    if( error_flag) MessageBox( hWnd, "Map file format error", "Error", MB_OK);

    fclose(fp);
    
}


void save_map( char *fname)
{
    FILE *fp;
    int w;

    fp = fopen(fname,"wb");
    if (fp==NULL) mprintf( "fp is NULL");

    fprintf( fp, "%d\n", sp_scan_count);

    for( w=0; w<sp_scan_count; w++) {
	fprintf( fp, "%d ", sp_offsets_map[w].x );
	fprintf( fp, "%d\n", sp_offsets_map[w].y );
    }

    fclose(fp);
    
}

void load_pal_fp(FILE *fp)
{
    int i;
    int version, magic;
    LOGPAL_EL *this;
  
    version=PAL_DATA_VERSION;
    fread(&magic,sizeof(magic),1,fp);
    fread(&version,sizeof(version),1,fp);
    if ((*glob_data)->pal.type==PALTYPE_LOGICAL)
        destroy_logpal(&((*glob_data)->pal.logpal));
        
    
    fread(&(*glob_data)->pal.type,sizeof((*glob_data)->pal.type),1,fp);
    switch((*glob_data)->pal.type)
    {
        case PALTYPE_MAPPED:
            for(i=0;i<LOGPALSIZE;i++)
            {
                fread(&(*glob_data)->pal.dacbox[i*3],
                    sizeof((*glob_data)->pal.dacbox[0]),1,fp);
                fread(&(*glob_data)->pal.dacbox[i*3+1],
                    sizeof((*glob_data)->pal.dacbox[0]),1,fp);
                fread(&(*glob_data)->pal.dacbox[i*3+2],
                    sizeof((*glob_data)->pal.dacbox[0]),1,fp);
            }
            break;
        case PALTYPE_LOGICAL:
            do
            {
                fread(&(i),sizeof(i),1,fp);
                if (i>=0)
                {
                    this=new_logpal_el(
                        &((*glob_data)->pal.logpal),0);
                    this->color.r=i;
                    fread(&(this->color.g),sizeof(this->color.g),1,fp);
                    fread(&(this->color.b),sizeof(this->color.b),1,fp);
                    fread(&(this->index),sizeof(this->index),1,fp);
                    fread(&(this->equalized),sizeof(this->equalized),1,fp);
                    fread(&(this->gamma),sizeof(this->gamma),1,fp);
                }
            } while(i>=0);
            sort_logpal_els(&((*glob_data)->pal.logpal));
            this=(*glob_data)->pal.logpal;
            do
            {
                if (this->equalized && this->index<this->next->index)
                {
                    equalize(*glob_data,
                        INDEX2Z(this->index,*glob_data),
                        INDEX2Z(this->next->index,*glob_data),
                        this->fhist);
                }
                this=this->next;
                
            } while(this!=(*glob_data)->pal.logpal);
            
            break;
    }
}
    
void load_old_pal(char *fname)
{
    
    FILE *fp;
    int i;
    unsigned char *dacbox;

    dacbox=(*glob_data)->pal.dacbox;    
    if ((*glob_data)->pal.type==PALTYPE_LOGICAL)
        destroy_logpal(&((*glob_data)->pal.logpal));
    (*glob_data)->pal.type=PALTYPE_MAPPED;
    fp=fopen(fname,"rb");
    if (fp==NULL) mprintf("Couldn't open PAL file");
    
    for(i=0;i<LOGPALSIZE;i++)
        fscanf(fp,"%d %d %d", &dacbox[i*3+0],&dacbox[i*3+1],&dacbox[i*3+2]);
    fclose(fp);
    spindac(0,1,dacbox);
}

int valid_filename(char *name)
{
    int i;
    if ( strlen(name)<12) return 0;
    for (i=0;i<6;i++) if (!isdigit(name[i])) return 0;
    return 1;
}

static void load_gen2d_fp(FILE *fp)
{
  Byte *compr,*uncompr;
  uLong comprLen,uncomprLen;
  int err;
  int compressed=1;
    
    fread(&((*glob_data)->x_type),sizeof((*glob_data)->x_type),1,fp);
    fread(&((*glob_data)->y_type),sizeof((*glob_data)->y_type),1,fp);
    fread(&((*glob_data)->size),sizeof((*glob_data)->size),1,fp);
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
    switch((*glob_data)->x_type)
    {
        case GEN2D_FLOAT:
            
            (*glob_data)->xf = (float *) malloc((*glob_data)->size*sizeof(float));
            break;
        case GEN2D_USHORT:
            (*glob_data)->xd = (unsigned short *) 
                malloc((*glob_data)->size*sizeof(unsigned short));
            break;
    }
    switch((*glob_data)->y_type)
    {
        case GEN2D_FLOAT:
            (*glob_data)->yf = (float *) malloc((*glob_data)->size*sizeof(float));
            fread(&compressed,sizeof(int),1,fp);
            fread(&comprLen,sizeof(uLong),1,fp);
            compr=(Byte *)malloc(comprLen);
            fread(compr,sizeof(Byte),comprLen,fp);
            uncomprLen= (*glob_data)->size*sizeof(float);
            if (compressed)
            {
                uncompr=(Byte *)((*glob_data)->yf);
                err = uncompress(uncompr, &uncomprLen, compr, comprLen);
                if (err != Z_OK)
                {
                    mprintf("Data corrupted. Start praying.");
                }
            }
            else
            {
                memcpy((*glob_data)->yf,compr,comprLen);
            }
            free(compr);
            break;
        case GEN2D_USHORT:
            (*glob_data)->yd = (unsigned short *) 
                malloc((*glob_data)->size*sizeof(unsigned short));
            break;
    }
}    

int load_mas(char *fname)
{
    FILE *fp;
    int version;
    int magic;
    int i,*nan;
    
    char *name;
  
  (*glob_data)->saved=1;
    
    strcpy((*glob_data)->full_filename,fname);  
    name=strrchr(fname,'\\');
    if (name==NULL) strcpy((*glob_data)->filename,fname);
    else strcpy((*glob_data)->filename,name+1);
    *(strrchr((*glob_data)->filename,'.'))='\0';
    fp=fopen(fname,"rb");
  if (fp==NULL)
  {
    return 0;
  }
    fread(&magic,sizeof(version),1,fp);
    if (magic<FILE_MAGIC_MIN)
    {
        fclose(fp);
        load_mas_old(fname);
        return 1;
    }
    fread(&version,sizeof(version),1,fp);

    load_gen2d_fp(fp);    
    
    fread(&((*glob_data)->start),sizeof((*glob_data)->start),1,fp);
    fread(&((*glob_data)->end),sizeof((*glob_data)->end),1,fp);
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
    if (version>5)
    {
        
        fread(&((*glob_data)->type2d),sizeof((*glob_data)->type2d),1,fp);
        fread(&((*glob_data)->bias_range),sizeof((*glob_data)->bias_range),1,fp);
        fread(&((*glob_data)->i_set_range),sizeof((*glob_data)->i_set_range),1,fp);
        fread(&((*glob_data)->amp_gain),sizeof((*glob_data)->amp_gain),1,fp);
        fread(&((*glob_data)->measure_i),sizeof((*glob_data)->measure_i),1,fp);
        fread(&((*glob_data)->measure_z),sizeof((*glob_data)->measure_z),1,fp);
        fread(&((*glob_data)->measure_2),sizeof((*glob_data)->measure_2),1,fp);
        fread(&((*glob_data)->measure_3),sizeof((*glob_data)->measure_3),1,fp);
        fread(&((*glob_data)->track_offset_x),sizeof((*glob_data)->track_offset_x),1,fp);
        fread(&((*glob_data)->track_offset_y),sizeof((*glob_data)->track_offset_y),1,fp);
        fread(&((*glob_data)->track_max_bits),sizeof((*glob_data)->track_max_bits),1,fp);
        fread(&((*glob_data)->track_sample_every),sizeof((*glob_data)->track_sample_every),1,fp);
        fread(&((*glob_data)->tracking_mode),sizeof((*glob_data)->tracking_mode),1,fp);
        fread(&((*glob_data)->track_auto_auto),sizeof((*glob_data)->track_auto_auto),1,fp);
        fread(&((*glob_data)->track_iterations),sizeof((*glob_data)->track_iterations),1,fp);
        fread(&((*glob_data)->track_every),sizeof((*glob_data)->track_every),1,fp);
        fread(&((*glob_data)->z_offset_bias),sizeof((*glob_data)->z_offset_bias),1,fp);
        fread(&((*glob_data)->num_passes),sizeof((*glob_data)->num_passes),1,fp);
        fread(&((*glob_data)->scan_dir),sizeof((*glob_data)->scan_dir),1,fp);
        fread(&((*glob_data)->scan_num),sizeof((*glob_data)->scan_num),1,fp);
        fread(&((*glob_data)->crash_protection),sizeof((*glob_data)->crash_protection),1,fp);
        fread(&((*glob_data)->dither0),sizeof((*glob_data)->dither0),1,fp);
        fread(&((*glob_data)->dither1),sizeof((*glob_data)->dither1),1,fp);
        fread(&((*glob_data)->step),sizeof((*glob_data)->step),1,fp);
        fread(&((*glob_data)->scan_feedback),sizeof((*glob_data)->scan_feedback),1,fp);
    }        
    if (version>6)
    {
        fread(&((*glob_data)->move_time),sizeof((*glob_data)->move_time),1,fp);
        fread(&((*glob_data)->dither_wait),sizeof((*glob_data)->dither_wait),1,fp);
    }
    else
    {
        (*glob_data)->move_time=(*glob_data)->speed;
        (*glob_data)->dither_wait=0;
    }
    if (version>7)
    {
        fread(&((*glob_data)->feedback_wait),sizeof((*glob_data)->feedback_wait),1,fp);
        fread(&((*glob_data)->feedback_every),sizeof((*glob_data)->feedback_every),1,fp);
    }
    else
    {
        (*glob_data)->feedback_wait=0;
        (*glob_data)->feedback_every=0;
    }
    if (version>8)
    {
        fread(&((*glob_data)->track_avg_data_pts),sizeof((*glob_data)->track_avg_data_pts),1,fp);
    }
    else
    {
        (*glob_data)->track_avg_data_pts=200;
    }
    if (version>9)
    {
        fread(&((*glob_data)->sp_mode),sizeof((*glob_data)->sp_mode),1,fp);
        for(i=0;i<16;i++)
            fread(&((*glob_data)->pre_dac_data[i]),
                sizeof((*glob_data)->pre_dac_data[i]),1,fp);
        fread(&((*glob_data)->x),sizeof((*glob_data)->x),1,fp);
        fread(&((*glob_data)->y),sizeof((*glob_data)->y),1,fp);
        fread((*glob_data)->dep_filename,sizeof((*glob_data)->dep_filename[0])*FILE_NAME_SIZE,1,fp);
        fread(&((*glob_data)->track_step_delay),sizeof((*glob_data)->track_step_delay),1,fp);
        fread(&((*glob_data)->output_ch),sizeof((*glob_data)->output_ch),1,fp);
        fread(&((*glob_data)->lockin_sen1),sizeof((*glob_data)->lockin_sen1),1,fp);
        fread(&((*glob_data)->lockin_sen2),sizeof((*glob_data)->lockin_sen2),1,fp);
        fread(&((*glob_data)->lockin_osc),sizeof((*glob_data)->lockin_osc),1,fp);
        fread(&((*glob_data)->lockin_w),sizeof((*glob_data)->lockin_w),1,fp);
        fread(&((*glob_data)->lockin_tc),sizeof((*glob_data)->lockin_tc),1,fp);
        fread(&((*glob_data)->lockin_n1),sizeof((*glob_data)->lockin_n1),1,fp);
        fread(&((*glob_data)->lockin_pha1),sizeof((*glob_data)->lockin_pha1),1,fp);
        fread(&((*glob_data)->lockin_pha2),sizeof((*glob_data)->lockin_pha2),1,fp);
#ifdef OLD
        if (version==10) 
        {
            ((*glob_data)->lockin_sen1)=0;
            ((*glob_data)->lockin_sen2)=0;
            ((*glob_data)->lockin_osc)=0;
            ((*glob_data)->lockin_w)=0;
            ((*glob_data)->lockin_tc)=0;
            ((*glob_data)->lockin_n1)=0;
            ((*glob_data)->lockin_pha1)=0;
            ((*glob_data)->lockin_pha2)=0;
			// why?
        }
#endif    
    }
    else
    {
        (*glob_data)->sp_mode=SP_MODE_ABSOLUTE;
        for (i=0;i<16;i++) (*glob_data)->pre_dac_data[i]=0;
        (*glob_data)->x=0;
        (*glob_data)->y=0; 
        (*glob_data)->dep_filename[0]='\0';
        (*glob_data)->track_step_delay=1000;
        (*glob_data)->output_ch=sample_bias_ch;
        (*glob_data)->lockin_sen1 = 0.0;
        (*glob_data)->lockin_sen2 = 0.0;
        (*glob_data)->lockin_osc = 0.0;
        (*glob_data)->lockin_w = 0.0;
        (*glob_data)->lockin_tc = 0.0;
        (*glob_data)->lockin_n1 = 0.0;
        (*glob_data)->lockin_pha1 = 0.0;
        (*glob_data)->lockin_pha2 = 0.0;
    }

    if(version > 10)
	{
	    fread(&((*glob_data)->track_at_min),sizeof((*glob_data)->track_at_min),1,fp);
	    fread(&((*glob_data)->feedback_at_min),sizeof((*glob_data)->feedback_at_min),1,fp);
    }
    if(version > 11)
	{
	    fread(&((*glob_data)->track_plane_fit_a),sizeof((*glob_data)->track_plane_fit_a),1,fp);
	    fread(&((*glob_data)->track_plane_fit_b),sizeof((*glob_data)->track_plane_fit_b),1,fp);
    }
    if(version > 12)
	{
	    fread(&((*glob_data)->offset_hold),sizeof((*glob_data)->offset_hold),1,fp);
    }
	// a very small number of files saved during development only have lockin_in_use field
	// but not offset hold and are labeled as version 13
	if(version > 13)
	{
	    fread(&((*glob_data)->lockin_in_use),sizeof((*glob_data)->lockin_in_use),1,fp);
	}
	else
	{
        (*glob_data)->lockin_in_use = LOCKIN_NOT_USED;
	}
    if(version > 14)
	{
	    fread(&((*glob_data)->bit16),sizeof((*glob_data)->bit16),1,fp);
	}
	else
	{
        (*glob_data)->bit16 = 0;
	}

	if(version > 15)
	{
	    fread(&((*glob_data)->ramp_time),sizeof((*glob_data)->ramp_time),1,fp);
	}
	else
	{
        (*glob_data)->ramp_time = (*glob_data)->move_time;
	}


    switch((*glob_data)->sp_mode)
    {
        case SP_MODE_ABSOLUTE:
            (*glob_data)->min_x=(*glob_data)->start;
            (*glob_data)->max_x=(*glob_data)->start+
                            ((*glob_data)->size-1)*(*glob_data)->step;
            break;
        case SP_MODE_RELATIVE:
            (*glob_data)->min_x=(*glob_data)->start+
                (*glob_data)->pre_dac_data[(*glob_data)->output_ch];
            (*glob_data)->max_x=(*glob_data)->start+
                        ((*glob_data)->size-1)*(*glob_data)->step+
                        (*glob_data)->pre_dac_data[(*glob_data)->output_ch];
            break;
    }
    (*glob_data)->valid=1;
    fclose(fp);
    if ((*glob_data)->version==9 && (*glob_data)->y_type==GEN2D_FLOAT) 
    /* fix for NaN's */
    { 
        for(i=1;i<(*glob_data)->size-1;i++)
        {
            nan=(int *)((*glob_data)->yf+i);
            if (*nan==0x7FC45B82 || *nan==0x7FC5F46C)
            {
                *((*glob_data)->yf+i)=(*((*glob_data)->yf+i-1)+
                    *((*glob_data)->yf+i+1))/2;
            }
        }
    }
    return 1;    
}

static void save_3d_as_text( char *fname)
{
    FILE *fp;
    int row = 0;
    int col = 0;
    unsigned int dimension = (*glob_data)->size;

    fp=fopen( fname, "wb");
    if ( fp==NULL ) mprintf( "Couldn't open EXP file to write");

    // print out the data row by row (same way as it is stored in pointer)
    for( row = 0; row < dimension; row++) {
	for( col = 0; col < dimension; col++ ) {
	    fprintf( fp, "%f ", (*glob_data)->ptr[ dimension*row + col ]);
	}
	fprintf( fp, "\n");
    }

    fclose( fp);

}

void save_mas(char *fname,int compressit)
{
    FILE *fp;
    int magic=FILE_MAGIC_SPC;
    int version = SP_DATA_VERSION;
    int i;
    
    char *name;
  
    strcpy((*glob_data)->full_filename,fname);  
    name=strrchr(fname,'\\');
    if (name==NULL) strcpy((*glob_data)->filename,fname);
    else strcpy((*glob_data)->filename,name+1);
    *(strrchr((*glob_data)->filename,'.'))='\0';
    fp=fopen(fname,"wb");
    if (fp==NULL) mprintf("Couldn't open MAS file to write");
    fwrite(&magic,sizeof(magic),1,fp);
    fwrite(&version,sizeof(version),1,fp);
    save_gen2d_fp(fp,compressit);
    
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
    fwrite(&((*glob_data)->sample_type.size),sizeof((*glob_data)->sample_type.size),1,fp);
    fwrite((*glob_data)->sample_type.ptr,sizeof(char),(size_t)(*glob_data)->sample_type.size,fp);
    fwrite(&(*glob_data)->dosed_langmuir,sizeof((*glob_data)->dosed_langmuir),1,fp);  
    fwrite(&((*glob_data)->dosed_type.size),sizeof((*glob_data)->dosed_type.size),1,fp);
    fwrite((*glob_data)->dosed_type.ptr,sizeof(char),(size_t)(*glob_data)->dosed_type.size,fp);
    fwrite(&((*glob_data)->bias),sizeof((*glob_data)->bias),1,fp);
    fwrite(&((*glob_data)->dz_offset),sizeof((*glob_data)->dz_offset),1,fp);
    fwrite(&((*glob_data)->i_setpoint),sizeof((*glob_data)->i_setpoint),1,fp);
    fwrite(&((*glob_data)->samples),sizeof((*glob_data)->samples),1,fp);
    fwrite(&((*glob_data)->type),sizeof((*glob_data)->type),1,fp);
/* version 5 */
    fwrite(&((*glob_data)->delta_z_offset),sizeof((*glob_data)->delta_z_offset),1,fp);
    fwrite(&((*glob_data)->temperature),sizeof((*glob_data)->temperature),1,fp);
    fwrite(&((*glob_data)->version),sizeof((*glob_data)->version),1,fp);
/* version 6 */
    fwrite(&((*glob_data)->type2d),sizeof((*glob_data)->type2d),1,fp);
    fwrite(&((*glob_data)->bias_range),sizeof((*glob_data)->bias_range),1,fp);
    fwrite(&((*glob_data)->i_set_range),sizeof((*glob_data)->i_set_range),1,fp);
    fwrite(&((*glob_data)->amp_gain),sizeof((*glob_data)->amp_gain),1,fp);
    fwrite(&((*glob_data)->measure_i),sizeof((*glob_data)->measure_i),1,fp);
    fwrite(&((*glob_data)->measure_z),sizeof((*glob_data)->measure_z),1,fp);
    fwrite(&((*glob_data)->measure_2),sizeof((*glob_data)->measure_2),1,fp);
    fwrite(&((*glob_data)->measure_3),sizeof((*glob_data)->measure_3),1,fp);
    fwrite(&((*glob_data)->track_offset_x),sizeof((*glob_data)->track_offset_x),1,fp);
    fwrite(&((*glob_data)->track_offset_y),sizeof((*glob_data)->track_offset_y),1,fp);
    fwrite(&((*glob_data)->track_max_bits),sizeof((*glob_data)->track_max_bits),1,fp);
    fwrite(&((*glob_data)->track_sample_every),sizeof((*glob_data)->track_sample_every),1,fp);
    fwrite(&((*glob_data)->tracking_mode),sizeof((*glob_data)->tracking_mode),1,fp);
    fwrite(&((*glob_data)->track_auto_auto),sizeof((*glob_data)->track_auto_auto),1,fp);
    fwrite(&((*glob_data)->track_iterations),sizeof((*glob_data)->track_iterations),1,fp);
    fwrite(&((*glob_data)->track_every),sizeof((*glob_data)->track_every),1,fp);
    fwrite(&((*glob_data)->z_offset_bias),sizeof((*glob_data)->z_offset_bias),1,fp);
    fwrite(&((*glob_data)->num_passes),sizeof((*glob_data)->num_passes),1,fp);
    fwrite(&((*glob_data)->scan_dir),sizeof((*glob_data)->scan_dir),1,fp);
    fwrite(&((*glob_data)->scan_num),sizeof((*glob_data)->scan_num),1,fp);
    fwrite(&((*glob_data)->crash_protection),sizeof((*glob_data)->crash_protection),1,fp);
    fwrite(&((*glob_data)->dither0),sizeof((*glob_data)->dither0),1,fp);
    fwrite(&((*glob_data)->dither1),sizeof((*glob_data)->dither1),1,fp);
    fwrite(&((*glob_data)->step),sizeof((*glob_data)->step),1,fp);
    fwrite(&((*glob_data)->scan_feedback),sizeof((*glob_data)->scan_feedback),1,fp);
/* version 7 */
    fwrite(&((*glob_data)->move_time),sizeof((*glob_data)->move_time),1,fp);
    fwrite(&((*glob_data)->dither_wait),sizeof((*glob_data)->dither_wait),1,fp);
/* version 8 */
    fwrite(&((*glob_data)->feedback_wait),sizeof((*glob_data)->feedback_wait),1,fp);
    fwrite(&((*glob_data)->feedback_every),sizeof((*glob_data)->feedback_every),1,fp);
/* version 9 */
    fwrite(&((*glob_data)->track_avg_data_pts),sizeof((*glob_data)->track_avg_data_pts),1,fp);

/* version 10 */
    fwrite(&((*glob_data)->sp_mode),sizeof((*glob_data)->sp_mode),1,fp);
    for(i=0;i<16;i++)
        fwrite(&((*glob_data)->pre_dac_data[i]),
            sizeof((*glob_data)->pre_dac_data[i]),1,fp);
    fwrite(&((*glob_data)->x),sizeof((*glob_data)->x),1,fp);
    fwrite(&((*glob_data)->y),sizeof((*glob_data)->y),1,fp);
    fwrite(&((*glob_data)->dep_filename),sizeof((*glob_data)->dep_filename[0])*FILE_NAME_SIZE,1,fp);
    fwrite(&((*glob_data)->track_step_delay),sizeof((*glob_data)->track_step_delay),1,fp);
    fwrite(&((*glob_data)->output_ch),sizeof((*glob_data)->output_ch),1,fp);
    fwrite(&((*glob_data)->lockin_sen1),sizeof((*glob_data)->lockin_sen1),1,fp);
    fwrite(&((*glob_data)->lockin_sen2),sizeof((*glob_data)->lockin_sen2),1,fp);
    fwrite(&((*glob_data)->lockin_osc),sizeof((*glob_data)->lockin_osc),1,fp);
    fwrite(&((*glob_data)->lockin_w),sizeof((*glob_data)->lockin_w),1,fp);
    fwrite(&((*glob_data)->lockin_tc),sizeof((*glob_data)->lockin_tc),1,fp);
    fwrite(&((*glob_data)->lockin_n1),sizeof((*glob_data)->lockin_n1),1,fp);
    fwrite(&((*glob_data)->lockin_pha1),sizeof((*glob_data)->lockin_pha1),1,fp);
    fwrite(&((*glob_data)->lockin_pha2),sizeof((*glob_data)->lockin_pha2),1,fp);
    
/* version 11 */
    fwrite(&((*glob_data)->track_at_min),sizeof((*glob_data)->track_at_min),1,fp);
    fwrite(&((*glob_data)->feedback_at_min),sizeof((*glob_data)->feedback_at_min),1,fp);
    
/* version 12 */
    fwrite(&((*glob_data)->track_plane_fit_a),sizeof((*glob_data)->track_plane_fit_a),1,fp);
    fwrite(&((*glob_data)->track_plane_fit_b),sizeof((*glob_data)->track_plane_fit_b),1,fp);

/* version 13 */
	fwrite(&((*glob_data)->offset_hold),sizeof((*glob_data)->offset_hold),1,fp);

/* version 14 */
	fwrite(&((*glob_data)->lockin_in_use),sizeof((*glob_data)->lockin_in_use),1,fp);

/* version 15 */
	fwrite(&((*glob_data)->bit16),sizeof((*glob_data)->bit16),1,fp);
/* version	16*/
	fwrite(&((*glob_data)->ramp_time),sizeof((*glob_data)->ramp_time),1,fp);

	fclose(fp);
     
}

void save_cnt_as_txt(char *filename)
{
    FILE *fp;
    char index_char_1, index_char_2;
    COUNT_EL *this_el;
    int j;
  
    fp = fopen(filename, "wb");
    if(fp == NULL) {
       mprintf("Couldn't open CXP file to write");
       return;
    }

    // header
    GetIndexChrs(filename, &index_char_1, &index_char_2);
    fprintf(fp, "x%c%c y%c%c col%c%c\n", index_char_1, index_char_2,
		index_char_1, index_char_2, index_char_1, index_char_2);
 
    for(j=0; j < COUNT_COLORS; j++)
    {
        this_el = glob_count_data->list[j];
        if(this_el)
        {
            do
            {
                fprintf(fp, "%d %d %d\n", this_el->x, this_el->y, j);
                this_el=this_el->next;
            }while(this_el != glob_count_data->list[j]);
        }
    }
    fclose(fp);
}

void save_cnt(char *filename)
{
    FILE *fp;
  
    fp=fopen(filename,"wb");
    if (fp==NULL) mprintf("Couldn't open CNT file to write");
    save_cnt_fp(fp);    
    fclose(fp);
}

void save_cnt_fp(FILE *fp)
{
    int magic=FILE_MAGIC_CNT;
    int version=COUNT_DATA_VERSION;
    int i,j;
    COUNT_EL *this_el;
    
    fwrite(&magic,sizeof(magic),1,fp);
    fwrite(&version,sizeof(version),1,fp);
    fwrite(&(glob_count_data->comment.size),sizeof(glob_count_data->comment.size),1,fp);
    fwrite(glob_count_data->comment.ptr,sizeof(glob_count_data->comment.ptr[0]),(size_t) glob_count_data->comment.size,fp);
    fwrite(&(glob_count_data->filename),sizeof(glob_count_data->filename[0])*FILE_NAME_SIZE,1,fp);
    fwrite(&(glob_count_data->version),sizeof(glob_count_data->version),1,fp);
    fwrite(&(glob_count_data->size),sizeof(glob_count_data->size),1,fp);
    for(j=0;j<COUNT_COLORS;j++)
    {
        this_el=glob_count_data->list[j];
        if (this_el)
        {
            do
            {
                fwrite(&(this_el->x),sizeof(this_el->x),1,fp);
                fwrite(&(this_el->y),sizeof(this_el->y),1,fp);
                this_el=this_el->next;
            }while(this_el!=glob_count_data->list[j]);
        }
        i=-1;
        fwrite(&(i),sizeof(i),1,fp);
        fwrite(&(i),sizeof(i),1,fp);
    }
    for(j=0;j<COUNT_COLORS;j++)
    {
        fwrite(&(glob_count_data->color[j].r),sizeof(glob_count_data->color[j].r),1,fp);
        fwrite(&(glob_count_data->color[j].g),sizeof(glob_count_data->color[j].g),1,fp);
        fwrite(&(glob_count_data->color[j].b),sizeof(glob_count_data->color[j].b),1,fp);
    }
    fwrite(&(glob_count_data->hide),sizeof(glob_count_data->hide),1,fp);
}

void load_cnt(char *filename)
{
    FILE *fp;
  
    fp=fopen(filename,"rb");
    if (fp==NULL) mprintf("Couldn't open CNT file");
    load_cnt_fp(fp);

    fclose(fp);
}

void load_cnt_fp(FILE *fp)
{
    int magic;
    int version;
    int x,y;
    int i,colors;
    
    destroy_count(glob_count_data);
    fread(&magic,sizeof(magic),1,fp);
    fread(&version,sizeof(version),1,fp);
    fread(&(glob_count_data->comment.size),sizeof(glob_count_data->comment.size),1,fp);
    fread(glob_count_data->comment.ptr,sizeof(glob_count_data->comment.ptr[0]),(size_t) glob_count_data->comment.size,fp);
    glob_count_data->comment.ptr[glob_count_data->comment.size]='\0';
    fread(&(glob_count_data->filename),sizeof(glob_count_data->filename[0])*FILE_NAME_SIZE,1,fp);
    fread(&(glob_count_data->version),sizeof(glob_count_data->version),1,fp);
    fread(&(glob_count_data->size),sizeof(glob_count_data->size),1,fp);

    if (version<2) colors=2;
    else colors=4;
    for(i=0;i<colors;i++)
    {    
        do
        {
            fread(&(x),sizeof(x),1,fp);
            fread(&(y),sizeof(y),1,fp);
            if (x!=-1) 
            {
                new_count_el(&(glob_count_data->list[i]),x,y);
                glob_count_data->total[i]++;
            }
        } while (x!=-1);
    }
    if (version>2)
    {
        for(i=0;i<colors;i++)
        {
            fread(&(glob_count_data->color[i].r),sizeof(glob_count_data->color[i].r),1,fp);
            fread(&(glob_count_data->color[i].g),sizeof(glob_count_data->color[i].g),1,fp);
            fread(&(glob_count_data->color[i].b),sizeof(glob_count_data->color[i].b),1,fp);
        }
        fread(&(glob_count_data->hide),sizeof(glob_count_data->hide),1,fp);
    }
    else
    {
        glob_count_data->color[0].r=255;
        glob_count_data->color[0].g=0;
        glob_count_data->color[0].b=0;
        glob_count_data->color[1].r=0;
        glob_count_data->color[1].g=255;
        glob_count_data->color[1].b=0;
        glob_count_data->color[2].r=0;
        glob_count_data->color[2].g=0;
        glob_count_data->color[2].b=255;
        glob_count_data->color[3].r=255;
        glob_count_data->color[3].g=0;
        glob_count_data->color[3].b=255;
        
        glob_count_data->hide=0;
    }
}

static void GetIndexChrs( char *filename, char *index1, char *index2)
{
    int m = 0;

    while( filename[m] != '\0') m++;
    while( filename[m] != '.') m--;
    m-=1;
    *index2 = filename[m--];
    *index1 = filename[m];
}

void save_exp(char *filename)
{
    FILE *fp;
    int i;
    float max_v;
    char index1, index2;
    

    fp=fopen(filename,"wb");
    if (fp==NULL) mprintf("Couldn't open EXP file to write");
    switch((*glob_data)->type)
    {
        case DATATYPE_GEN2D:
            switch((*glob_data)->type2d)
            {
                case TYPE2D_SPEC_I:
                case TYPE2D_SPEC_Z:
                case TYPE2D_SPEC_2:
                case TYPE2D_SPEC_3:
                case TYPE2D_MASSPEC:
					GetIndexChrs( filename, &index1, &index2);
					fprintf( fp, "V%c%c S%c%c\n", index1, index2, index1, index2);
                    for(i=0;i<(*glob_data)->size;i++)
                    {
                        fprintf(fp,"%f %f\n",
                            dtov(index2d((*glob_data),i),
                            (*glob_data)->v_range),in_dtov(*((*glob_data)->yf+i)));
                    }
                    break;
                case TYPE2D_CUT:
                    for(i=0;i<(*glob_data)->size;i++)
                    {
                        max_v=dtov_len((*glob_data)->step*(*glob_data)->max_x,
                            (*glob_data)->x_range)*(*glob_data)->x_gain;
                        fprintf(fp,"%f %f\n",
                            ((float)i)*max_v/(float)(*glob_data)->size,
                            in_dtov(*((*glob_data)->yf+i)));
                    }
                    break;
            }
            break;
    	case DATATYPE_HOP:
		    for(i=0;i<(*glob_data)->size;i++) {
#if 0
		      fprintf( fp, "%f %f %d\n", *(time_hop+i), *(hop_timer_array+i), (*glob_data)->xd[i]);
#else
		      fprintf( fp, "%f %d %d\n", *(time_hop+i),
				(*glob_data)->xd[i], (*glob_data)->yd[i]);
#endif
		    }
		    break;
 		case DATATYPE_2D:
		    GetIndexChrs( filename, &index1, &index2);
		    fprintf( fp, "X%c%c Y%c%c\n", index1, index2, index1, index2);
            for(i=0;i<(*glob_data)->size;i++)
            {
                fprintf(fp,"%f %f\n",*((*glob_data)->time2d+i),
                in_dtov(*((*glob_data)->data2d+i)));
            }
			break;

    }
    
    fclose(fp);
}

char *rel_path(char *end)
{
    strcpy(rel_path_return,initial_working_dir);
    strcat(rel_path_return,end);
    return(rel_path_return);
}

