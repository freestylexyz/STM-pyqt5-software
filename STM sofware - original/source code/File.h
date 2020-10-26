#include "common.h"
#define CURRENT_DIR_MAX 400
#define CURRENT_FILENAME_MAX 300
#define FTYPE_STM 0
#define FTYPE_PAL 1
#define FTYPE_MAS 2
#define FTYPE_TP  3
#define FTYPE_GIF 4
#define FTYPE_SPC 5
#define FTYPE_DEP 6
#define FTYPE_GRD 7
#define FTYPE_FIG 8
#define FTYPE_DATA 9
#define FTYPE_UNKNOWN 10
#define FTYPE_STM_UNCOMPRESSED 11
#define FTYPE_CUT 12
#define FTYPE_CNT 13
#define FTYPE_GEN2D_EXPORT 14
#define FTYPE_STP 15
#define FTYPE_PS 16
#define FTYPE_EPS 17
#define FTYPE_HOP_DATA 18
#define FTYPE_3D2TEXT 19
#define FTYPE_MAP 20
#define FTYPE_CNT_AS_TXT 21

#define FILE_MAGIC_MIN 0x1000
#define FILE_MAGIC_STM 0x1001
#define FILE_MAGIC_PAL 0x1002
#define FILE_MAGIC_SPC 0x1003
#define FILE_MAGIC_CUT 0x1004
#define FILE_MAGIC_CNT 0x1005

#define SCAN_INI rel_path("scan")
#define DEP_INI rel_path("dep")
#define SP_INI rel_path("sp")

#define POSTSCRIPT_PREAMBLE rel_path("stmpre.ps")
#define STM_INI rel_path("stm.ini")
#define MASSPEC_INI rel_path("masspec.ini")
#define SCAN_SETTINGS rel_path("scanset.tmp")
#define SCAN_IMAGE rel_path("scanimg.tmp")
#define DTRANS_L_INI rel_path("dtransl.ini")
#define DTRANS_S_INI rel_path("dtranss.ini")
#define SAMPLES_INI rel_path("samples.ini")
#define DOSED_INI rel_path("dosed.ini")
#define INITIAL_DIR_INI rel_path("initdir.ini")
/*
#define INITIAL_DIR_STM rel_path("data"
#define INITIAL_DIR_PAL rel_path("data\\pal"
#define INITIAL_DIR_MAS rel_path("data\\mas"
#define INITIAL_DIR_GIF rel_path("data\\gif"
#define INITIAL_DIR_SPC rel_path("data\\spc"
*/

#define SAMPLE_BUF_MAX 500
#define FILE_STRING_SIZE 500

BOOL file_open(HWND,HANDLE,int,int,char *);
int file_save_as(HWND,HANDLE,int);
int load_image(char *);
void save_image(char *,int);
void load_cut(char *);
void save_cut(char *,int);
void load_init(char *);
void save_init(char *);
int load_dep(char *);
void save_dep(char *);
void moh_find_next(char *);
void moh_find_prev(char *);
int eatspace(FILE *);
void init_file(char*,char*,char*,int*);
void init_dirs();
void inc_file(char*,int*);
void smart_load(char *,int,int);
int guess_file_type(char *);
int load_mas(char *);
void save_mas(char *,int);
char *rel_path(char *);
void save_exp(char *);
void force_ext(char *,int);
int valid_filename(char *);

void save_image_top(FILE);

typedef struct FILE_LISTDEF
{
        char name[FILE_NAME_SIZE];
        struct FILE_LISTDEF *next;
} FILE_LIST;

        
