#include "common.h"

#define Z_SAFETY 0

#define DITHER_WAIT_MIN 0
#define DITHER_WAIT_MAX 10000000

#define PRESCAN_CONSEQ_DELT 1
#define PRESCAN_CONSEQ_DDELT 5

#define PRESCAN_OVER_PERCENT_MIN 0
#define PRESCAN_OVER_PERCENT_MAX 200
#define PRESCAN_OVER_WAIT1_MIN 0
#define PRESCAN_OVER_WAIT1_MAX 1000
#define PRESCAN_OVER_WAIT2_MIN 0
#define PRESCAN_OVER_WAIT2_MAX 1000

#define LISTBOX_STR_MAX 100
#define RS_TYPES_STR_MAX 30

#define PRESCAN_UNKNOWN_SAMPLE  "None"
#define PRESCAN_UNKNOWN_DOSED  "None"

typedef struct SAMPLELISTDEF
{
	char *name[LISTBOXMAX];
    LISTBOX listbox;
} SAMPLELIST;

unsigned int calc_i_setpoint(double,int,unsigned int);
void read_sample_list(SAMPLELIST *,char *,char *,char *); 
void free_sample_list(SAMPLELIST *);
int isdir(char *);


