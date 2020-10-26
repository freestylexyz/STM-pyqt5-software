//#define MW_BUG_SEARCH
//#define HOP_BOGUS_INIT 1
//#define OLD_HOP_TIMING 1
//#define HOP_TIMER_CALIBRATION 1

#define HOP_MAX_ITERATIONS 5000
#define HOP_PADDING 10
#define HOP_NUM_NEIGHBORS 8
#define HOP_LIMIT_PERCENT 10

#define ZERO_PIXEL 128 /* if fine coordinates are zero, pixels should be center */

#define HOP_EMPTY_STR ""
#define HOP_PRESS_ESC_STR "Press ESC to stop tracking"

#define HOP_HOLE 0
#define HOP_BUMP 1

#define HOP_DISPLAY_LEFT 10
#define HOP_DISPLAY_TOP 10
#define HOP_DISPLAY_RIGHT 266
#define HOP_DISPLAY_BOTTOM 266
#define HOP_DISPLAY_WIDTH 256
#define HOP_DISPLAY_HEIGHT 256

#define HOP_LIMIT_TIME_LEFT 100

extern float hop_timer, hop_card_clock, hop_last_clock;
extern unsigned int hop_lobyte, hop_hibyte;

#define RESET_CARD_CLOCK() { \
        if (clock_used) exit(1); \
        clock_used=1; \
	/* hop_timer += CLOCK_DEAD_TIME; */ \
	hop_card_clock = hop_last_clock = 0.0; \
        outpw(cntintclr,0x0000); \
        /* set up counter 3 for 16 bit rate generation */ \
	outp(cntrcmd,0xB4); \
	/* set up initial value for counter (which is CLOCK_CALIB microseconds) */ \
        outp(cntr3,0xF0);  \
        outp(cntr3,0xFF); \
        outpw(cfg3,0x2440);\
        }

#define READ_HOP_TIMER() { \
	outp(cntrcmd,0x84); /*latch cntr3 */ \
        hop_lobyte=inp(cntr3); \
	hop_hibyte = inp(cntr3)*256+hop_lobyte; \
	hop_last_clock = hop_card_clock; \
        hop_card_clock = (float)CLOCK_CALIB - (float)hop_hibyte * 0.5; \
	if( hop_card_clock < hop_last_clock) { \
	    hop_timer += (float)CLOCK_CALIB - hop_last_clock; \
	    hop_last_clock = 0; \
	} \
	hop_timer += hop_card_clock - hop_last_clock; \
      }
