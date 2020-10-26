#define FREQ_PER_BIT 266135

#define coarse_step_min         25
#define coarse_step_max         2047
#define coarse_step_delt        1
#define coarse_step_ddelt       100
#define coarse_period_min       25
#define coarse_period_max       3000
#define coarse_period_delt      1
#define coarse_period_ddelt     100
#define coarse_num_steps_min    1
#define coarse_num_steps_max    30000
#define coarse_num_steps_delt   1
#define coarse_num_steps_ddelt  1000
#define RIGHT   1
#define LEFT    -1

void repaint_step(HWND,unsigned int);
void repaint_period(HWND,unsigned int);
void repaint_position(HWND,double,double);
void repaint_num(HWND,unsigned int,unsigned int,unsigned int);
void coarse_window_enable(HWND,BOOL);
void coarse_go(HWND,unsigned int,unsigned int,unsigned int,unsigned int,int);
char *GetDistance(double);
char *GetTime(double);
void pre_coarse();
void post_coarse();
