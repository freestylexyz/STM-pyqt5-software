#define SCAN_CURRENT_DATA_DELT  1
#define SCAN_CURRENT_DATA_DDELT 2

#define SD scan_defaults_data[scan_current_default]
#define SDR scan_defaults_data[scan_current_default]->\
        read_seq[scan_defaults_data[scan_current_default]->current_read_seq]

#define SCAN_NUM_DEFAULTS   4

#define Z_CRASHED 2

#define OFFSET_GAIN     10

#define DEFAULT_Z_OFFSET_MOVE 3

#define DEFAULT_Z_OFFSET_TIME 10000

#define OVERSHOOT_FAST 0x1
#define OVERSHOOT_SLOW 0x2

#define SCAN_OFFSET_FUDGE 10
#define SCAN_WARN_TIME 20000000

#define BIAS_RAMP_TIME  500
#define BIAS_RAMP_SKIP  400

#define CRASH_TOLERANCE_MIN     3
#define CRASH_TOLERANCE_MAX     20
#define DRAW_TIME_PER_LINE      1200

#define FTOD_SCALE 1000
#define TIP_SPACING_MAX 1000
#define TIP_SPACING_MIN 100
#define TIP_SPACING_DELT 10
#define TIP_SPACING_DDELT 100

#define ABS_SCALE 819200

//#define A_PER_V 200
//#define Z_A_PER_V 30
#define A_PER_V a_per_v
#define Z_A_PER_V z_a_per_v

#define INTER_STEP_DELT 100
#define INTER_STEP_DDELT 1000
#define INTER_STEP_MIN_DELAY 1
#define INTER_STEP_MAX_DELAY 10000000

#define INTER_LINE_DELT 1
#define INTER_LINE_DDELT 10
#define INTER_LINE_MIN_DELAY 0
#define INTER_LINE_MAX_DELAY 400

#define FORWARD_DIR 0
#define BACKWARD_DIR 1
#define BOTH_DIR1 2
#define BOTH_DIR2 3
#define BOTH_DIR2_POLAR 4

#define PAL_IM_HEIGHT 28
#define PAL_IM_WIDTH 524
#define PAL_X_POS 37
#define PAL_Y_POS 599

//#define IMAGESIZE 524
#define IMAGESIZE 512
#define IMAGE_X_POS 37
#define IMAGE_Y_POS 37
#define PALETTESIZE 256
#define SCANSIZE 4096
#define DATASIZE 65536
#define GRAYSCALESIZE 150
#define GRAYSCALEOFFSET 53
#define COLOR 0.1

#define COLOR1BEGIN 0
#define COLOR1END 59
#define COLOR2BEGIN 80
#define COLOR2END 139
#define COLOR3BEGIN 170
#define COLOR3END 240

#define COLOR1MIN 50
#define COLOR1MAX 255
#define COLOR2MIN 50
#define COLOR2MAX 255
#define COLOR3MIN 50
#define COLOR3MAX 255

#define scan_x_min 0
#define scan_x_max 4095
#define scan_x_delt 1
#define scan_x_ddelt 50
#define scan_y_min 0
#define scan_y_max 4095
#define scan_y_delt 1
#define scan_y_ddelt 50
#define scan_z_min 0
#define scan_z_max 4095
#define scan_z_delt 1
#define scan_z_ddelt 50
#define scan_step_min 1
#define scan_step_max 255
#define scan_step_delt 1
#define scan_step_ddelt 10
#define scan_size_min 1
#define scan_size_max 512
#define scan_size_delt 1
#define scan_size_ddelt 10
#define scan_num_min 1
#define scan_num_max 10000
#define scan_num_delt 1
#define scan_num_ddelt 10

#define tip_current_min 0
#define tip_current_max 4095
#define tip_current_delt 1
#define tip_current_ddelt 50

#define sample_bias_min 0

#ifdef DAC16
#define sample_bias_max (bit16 == 0 ? 0xFFF:0xFFFF)
#else
#define sample_bias_max 0xFFF
#endif

#define sample_bias_delt 1
#define sample_bias_ddelt 50

int x_first_scan;

void repaint_scan_x(HWND);
void repaint_scan_y(HWND);
void repaint_scan_z(HWND);
void repaint_scan_step(HWND);
void repaint_scan_size(HWND);
void repaint_scan_num(HWND);
void repaint_scan_freq(HWND);
void repaint_scan_bitmap(HWND);
void repaint_sample_bias(HWND);
void repaint_tip_current(HWND);
void UpdateBitmap(HWND,int,int,int,int);
int pre_scan(HWND);
void post_scan();
void scan(HWND);
float calc_time(int);
void calc_gains(unsigned int *,unsigned int *,unsigned int *,unsigned int *);
int auto_z_above(int);
int auto_z_below(int);
int find_size(int);
float calc_total_time();
void scan_status(HWND,char *,float);
float calc_z_gain(unsigned int);
int num_data();
float calc_overshoot_slow_time();
float calc_initial_time();
float calc_overshoot_fast_time();
void set_scan_defaults(datadef *);

