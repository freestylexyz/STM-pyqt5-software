#ifndef _DATA_H
#define _DATA_H
#include "pal.h"

// #define MW_TRACK_PLANE

#define COUNT_COLORS 4

#define FILE_NAME_SIZE 256
#define FULL_FILE_NAME_SIZE 2048
#define DATA_MAX 512
#define DATA_MAX_COMP_ADD 2700
#define DTOI(D,MIN,MAX) (int) floor((double) (D-(double) MIN)*32768.0/((double) MAX-MIN+1))
#define ITOD(I,MIN,MAX) ((double) I*(double) (MAX-MIN+1)/32768.0)+(double) MIN
#define COMMENTMAXSIZE 5000
#define SAMPLETYPESIZE 500
#define LOGPALUSED 236
#define DATA_HIST_MAX LOGPALUSED*10
#define M_NUM_CH        2048
#define SP_NUM_CH       4096
#define GRID_MAX_POLY            100
#define GRID_CLIP_TYPE_LINE     0
#define GRID_CLIP_TYPE_POLY     1
#define GRID_CLIP_TYPE_CIRCLE   2

#define DATATYPE_SPEC_I         0
#define DATATYPE_SPEC_Z         1
#define DATATYPE_MASS           2
#define DATATYPE_SPEC_DIDV      3
#define DATATYPE_3D             4
#define DATATYPE_2D             5
#define DATATYPE_GEN2D          6
#define DATATYPE_3D_PARAMETERS  7
#define DATATYPE_HOP			8

#define GEN2D_FLOAT         0
#define GEN2D_USHORT        1
#define GEN2D_NONE          2

#define TYPE2D_PULSE_I          0
#define TYPE2D_PULSE_Z          1
#define TYPE2D_SPEC_I           2
#define TYPE2D_SPEC_Z           3
#define TYPE2D_MASSPEC          4
#define TYPE2D_CUT              5
#define TYPE2D_PULSE_DEF        6
#define TYPE2D_SPEC_2           7
#define TYPE2D_SPEC_3           8
#define TYPE2D_HOP		9

#define GRID_MAX_LINES 3
#define PRINT_TYPE_IMAGE        1
#define PRINT_TYPE_LINE         2
#define PRINT_TYPE_TEXT         3
#define PRINT_TYPE_EPS          4
#define PRINT_TYPE_IMAGE_LINK   5
#define PRINT_TYPE_ANY_IMAGE    6
#define PRINT_TYPE_PAL          7
#define PRINT_TYPE_PAL_LINK     8
#define PRINT_TYPE_BAR		9

#define LINEAR_MAX      255

// #define STM_DATA_VERSION 7
// inherited from Moh
//#define STM_DATA_VERSION 8
// added ramp_bias and wait_4 to read sequence
//#define STM_DATA_VERSION 9
// added ramp_ch to read sequence

#define STM_DATA_VERSION 10
// changed read sequences to read steps for better compatibility w/DSP

#define CUT_DATA_VERSION 1


#define LOGPAL_NUM_INDICES 256

#define PALTYPE_MAPPED 1
#define PALTYPE_LOGICAL 2

#define MAX_READ_STEPS	64 // maximum number of steps in read sequence


enum step_type {
	WAIT_TYPE = 0,
	READ_TYPE = 1,
	STEP_OUTPUT_TYPE = 2,
	SERIAL_OUT_TYPE = 3,
	DITHER0_TYPE = 4,
	//FEEDBACK_TYPE = 8,
	DITHER1_TYPE = 12
};

#define FEEDBACK_TYPE 8

#define STEP_TYPE_STR_MAX 100

typedef struct read_stepdef {
	// replaces old READ_SEQ

	int type;
    int state;			// ON or OFF
    int wait;			// delay in microsecs
    int out_ch;
	unsigned int step_bias; // value to output in bits
    int in_ch;
    int num_reads;		// how many times the input is read
	char step_type_string[STEP_TYPE_STR_MAX];
	char serial_out_char;

} read_step;


typedef struct read_seqdef {
// parameters for reading data during a "3d" scan
// need at least one for each image during a scan

    int feedback;
    int wait1;
    int dither0;
    int wait2;
    int dither1;
    int wait3;
    int record;
    int read_ch;
    int num_samples;

	/* version 8 */
	int do_ramp_bias;
	unsigned int ramp_value;
	int wait4;

	/* version 9 */
	int ramp_ch;
} READ_SEQ;

struct commentdef {
  unsigned int	size;
  char		*ptr;
};

typedef struct colordef {
        int r;
        int g;
        int b;
} STM_COLOR;

typedef struct logpal_eldef
{
        STM_COLOR color;
        int index;
        int equalized;
        float gamma;
        float fhist[LOGPALUSED+1];
        struct logpal_eldef *next;
        struct logpal_eldef *prev;
} LOGPAL_EL;

typedef struct genpaldef
{
        int type;
        int version;
        unsigned char *dacbox;
        LOGPAL_EL *logpal;
} GENPAL;

typedef struct count_eldef
{
        int x;
        int y;
        struct count_eldef *next;
        struct count_eldef *prev;
} COUNT_EL;

typedef struct count_datadef
{
        char filename[FILE_NAME_SIZE];
        COUNT_EL *list[COUNT_COLORS];
        struct commentdef     comment;
        int total[COUNT_COLORS];
        int hide;
        int version;
        int size;
        STM_COLOR color[COUNT_COLORS];
} COUNT_DATA;
        


typedef struct tagdatadef 
{
  struct commentdef	comment;
  unsigned int	size;
  unsigned int	step;
  unsigned int	x;
  unsigned int	y;
  unsigned int	z;
  float		x_gain;
  float		y_gain;
  float		z_gain;
  int           x_range;
  int           y_range;
  unsigned int	z_freq;
  float 	*ptr;
  float         max_z;
  float         min_z;
  float         clip_max;
  float         clip_min;
  int           valid;
  unsigned int  bias;
  int           scan_dir;
  struct commentdef sample_type;
  struct commentdef dosed_type;
  float dosed_langmuir;
  unsigned int i_setpoint;
  int i_set_range;
  int bias_range;
  unsigned int amp_gain;
  int x_offset_range;
  int y_offset_range;
  int step_delay;
  int inter_step_delay;
  int digital_feedback;
  float tip_spacing;
  int inter_line_delay;
  int scan_num;
  int scan_feedback;
  int read_feedback;
  int read_ch;
  int crash_protection;
  int overshoot;
  float overshoot_percent;
  int overshoot_wait1;
  int overshoot_wait2;
  int version;
  unsigned char hist[DATA_HIST_MAX];
  int equalized;
  float fhist[LOGPALUSED+1];
  char filename[FILE_NAME_SIZE];
  char dep_filename[FILE_NAME_SIZE];
  char full_filename[FULL_FILE_NAME_SIZE];
  float *time2d;
  unsigned short *data2d;
  float *xf;
  float *yf;
  unsigned short *xd;
  unsigned short *yd;
  int x_type;
  int y_type;
  float min_x,max_x;
  float cut_x1,cut_x2,cut_y1,cut_y2;
  int type;
  float temperature;
  int type2d;

  GENPAL pal;

  // the following are no longer used
  // retained for compatibility with pre-DSP files
  READ_SEQ *read_seq; // variable size, depending on number of READ_SEQ
  int read_seq_num;

  int digital_feedback_max;
  int digital_feedback_reread;
  float z_limit_percent;
  int x_first;
  int current_read_seq;
    
/* following are for pulse data */

  int move_time;
  int ramp_time;
  int write_time;
  int move_bias;
  int write_bias;
  int move_i_setpoint;
  int write_i_setpoint;  
  int pulse_duration; /* in micro seconds */
  float bias_speed;
  float current_speed;
  float z_offset_speed;
  int z_offset_move;
  int z_offset_crash_protect;
  int measure_i_once;
  int measure_i_wait;
  int feedback_wait;
  int measure_before;
  int delta_z_offset;
  int measure_i;
  int measure_z;
  int measure_2;
  int measure_3;
  int bias_speed_step;
  int dep_pulse_ignore_initial;
  int dep_pulse_change_time;
  int dep_pulse_avg_stop_pts;
  int dep_pulse_after_time;
  int dep_pulse_duration_mode;
  int dep_pulse_feedback_after;
  int dep_pulse_z_after;
  int dep_pulse_z_offset_overshoot;
  int dep_pulse_z_offset_overshoot_percentage;
  float dep_pulse_percent_change;
  int dither0;
  int dither1;
  int dither_wait;
  int scan_dither0;
  int scan_dither1;
  int track_offset_x;
  int track_offset_y;
  int track_max_bits;
  int track_sample_every;
  int tracking_mode;
  int track_auto_auto;
  int track_iterations;
  int track_avg_data_pts;
  int track_every;
  int track_step_delay;
  int feedback_every;
  int z_offset_bias;
  int num_passes;      
  int sp_mode;
  int step_delay_fixed;

/* the following are for masspec and spectroscopy data */

  int start; /* voltage (in bits) we started measuring from */
  int end; /* voltage (in bits) we ended the measurement */
  int skip; /* voltage range skipped (in bits). total skipped is
                  twice this */
  int v_range; /* 1 for +-5V, 2 for +-10V */
  float speed; /* in AMU/s or V/s*/
  float calib_int;
  float calib_slope;
  int dz_offset; /* delta z-offset for spec. from equil.*/
  int samples; /* number of samples */
  int scale; /* mass spec scale. e.g. 11 means 10^(-11) Torr 
               or pre-amp scale. e.g. 8 means 10^8   */
  int saved; 
  unsigned int pre_dac_data[16];
  int output_ch;
  
  float lockin_sen1;
  float lockin_sen2;
  float lockin_osc;
  float lockin_w;
  float lockin_tc;
  float lockin_n1;
  float lockin_pha1;
  float lockin_pha2;
  int lockin_in_use;

  int track_at_min;
  int feedback_at_min;

  float track_plane_fit_a;
  float track_plane_fit_b;

  int offset_hold;

  int total_steps;
  read_step sequence[MAX_READ_STEPS];

  unsigned int bit16;

  //add by Shaowei for spc settings
  int spc_trigger_on;
  int spc_trigger_ch;
  int spc_trigger_hi;
  int spc_trigger_lo;
  //end Shaowei

} datadef;                 

#ifdef OLD
typedef struct tagmassdata {
        int version; /* for revision changes */
        int type;
        float data[SP_NUM_CH]; /* actual data */
        int start; /* voltage (in bits) we started measuring from */
        int end; /* voltage (in bits) we ended the measurement */
        int skip; /* voltage range skipped (in bits). total skipped is
                        twice this */
        int v_range; /* 1 for +-5V, 2 for +-10V */
        float speed; /* in AMU/s or V/s*/
        float calib_int;
        float calib_slope;
        unsigned int bias; /* bias for spectr. */
        int dz_offset; /* delta z-offset for spec. from equil.*/
        unsigned int i_setpoint; /* i-setpoint for spec. */
        int samples; /* number of samples */
        int scale; /* mass spec scale. e.g. 11 means 10^(-11) Torr 
                     or pre-amp scale. e.g. 8 means 10^8   */
        struct commentdef     comment;
        
} massdata;        
#endif

typedef struct tagheaterdata {
        int size; /* # of data points */
        unsigned int *data; /* temperature data */
        float init_ramp_speed; /* V/s */
        float target_current; /* mA */
        float hold_time; /* s */
        int check_time; /* ms */
        float final_ramp_speed; /* V/s */
        struct commentdef     comment;
        
} heaterdata;        


typedef struct taggridline
{
        float theta;
        float r;
        float dist;
        int hide;
        int lock;
} GRID_LINE;

typedef struct taggrid
{
        GRID_LINE line[GRID_MAX_LINES];
        int version;
        int clip;
        int clip_y_less;
        int clip_type;
        int clip_num_pts;
        int x[GRID_MAX_POLY];
        int y[GRID_MAX_POLY];
        char scan_filename[FILE_NAME_SIZE];
        
} GRID;

typedef struct print_eldef {
        int type;
        int version;
        float x1;
        float y1;
        float x2;
        float y2;
        int link;
        char filename[FILE_NAME_SIZE];
        datadef *print_image;
        unsigned char *print_bitmap;
        int *print_bitmap_size;
        float sizex;
        float sizey;
        char logpal_name[FILE_NAME_SIZE];
        char grid_name[FILE_NAME_SIZE];
        GRID *print_grid;
        STM_COLOR color;
        char text[COMMENTMAXSIZE];
        int text_just;
        struct print_eldef *next;
        struct print_eldef *prev;
        int selected;
        int maybe_selected;
        LOGFONT print_font;
        float font_size;
        float angle;
        float arrow_width;
        float arrow_head;
        int arrow_head_type;
        float gamma;
        COUNT_DATA *count_data;
	GENPAL bar_pal;
	int image_resolution_type;			// PRINT_IMAGE_RES_RAW or PRINT_IMAGE_RES_SMOOTH
} PRINT_EL;


void alloc_data(datadef **,int,int,int,int,int);
void alloc_data_ptrs(datadef **,int,int,int,int,int);
void alloc_data_seq(datadef **,int);
void free_data(datadef **);
void free_data_ptrs(datadef **);
void free_data_seq(datadef **);
void unequalize(datadef *);
void equalize(datadef *,float ,float ,float *);
void repaint_pal(HWND, int , int ,unsigned char *,datadef *);
#endif
