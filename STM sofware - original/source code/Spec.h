#define SP_UPDATE_TIME  10000
#define SP_CREEP_TIME	1000000

#define SP_NUM_DEFAULTS 4

#define SP_FORWARD_DIR  0x4
#define SP_BACKWARD_DIR 0x2
#define SP_AVG_BOTH_DIR 0x1
#define SP_BACKWARD_DATA 0x8

#define SP_DATASETS     8

#define SP_Z_OFFSET_MIN (-500)
#define SP_Z_OFFSET_MAX 500
#define SP_Z_OFFSET_DELT 1
#define SP_Z_OFFSET_DDELT 50


//#define SP_DATA_VERSION 11
// inherited from Moh

//#define SP_DATA_VERSION 12
// add plane fit during tracking

//#define SP_DATA_VERSION 13
// added offset with feedback off

//#define SP_DATA_VERSION 14
// add lockin settings functionality

#define SP_DATA_VERSION 16
// add bit16

#define SP_NUM_SAMPLES_MIN 1
#define SP_NUM_SAMPLES_MAX 1000
#define SP_NUM_SAMPLES_DELT 1
#define SP_NUM_SAMPLES_DDELT 10
#define SP_NUM_PASSES_MIN 1
#define SP_NUM_PASSES_MAX 1000
#define SP_NUM_PASSES_DELT 1
#define SP_NUM_PASSES_DDELT 10

#define SP_TRIGGER_CH_MIN 1//Mod. 2 Shaowei define channel range
#define SP_TRIGGER_CH_MAX 11//Mod. 2 Shaowei define channel range
#define SP_TRIGGER_CH_DELT 1//Mod. 2 Shaowei define channel delta

#define SP_WEIGHTS 10

#define SP_X_MIN 5
#define SP_X_MAX 261
#define SP_Y_MIN 5
#define SP_Y_MAX 261
#define SP_Y_MIN_2 264
#define SP_Y_MAX_2 520
#define SP_V_PER_AMU     0.05
#define SP_Z_OFFSET_BIAS_MIN       0
#define SP_Z_OFFSET_BIAS_MAX       4095
#define SP_MIN_MIN       0
#define SP_MIN_MAX       4095
#define SP_MAX_MIN       0
#define SP_MAX_MAX       4095
#define SP_SPEED_MIN     10
#define SP_SPEED_MAX     10000000
#define SP_SPEED_DELT    10
#define SP_SPEED_DDELT   100
#define SP_PTR_MIN       0
#define SP_PTR_MAX       4095
#define SP_SCAN_DELT     1
#define SP_SCAN_DDELT    5
#define SP_VERT_MAX      0xFFFF
#define SP_VERT_MIN      0
#define SP_VERT_DELT     0xFFFF/200
#define SP_VERT_DDELT    0xFFFF/20
#define SP_SAMPLES_MAX   20
#define SP_SAMPLES_MIN   1
#define SP_SAMPLES_DELT  1
#define SP_SAMPLES_DDELT 5
#define NOFILE_STR "none"

#define SP_UPPER_IMAGE 0
#define SP_LOWER_IMAGE 2 // this is 2 so that it can be passed to repaint_rect and still make sense

typedef struct offset_pt_tag {
	int x;
	int y;
} OFFSET_PT;

void repaint_spec_graphics(HDC ,HPEN ,datadef *,
    int ,int ,int ,int ,int ,int , int , int );

