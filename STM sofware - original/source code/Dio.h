#include <conio.h>
#define NO_DIO_CARD
#define DAC16

#define DIO_INPUT_CHANNELS      4
#define DIO_OUTPUT_CHANNELS     16

#define DIGITAL_FEEDBACK_REREAD 1
#define DIGITAL_FEEDBACK_REREAD_MIN 1
#define DIGITAL_FEEDBACK_REREAD_MAX 50
#define DIGITAL_FEEDBACK_MAX    1000
#define DIGITAL_FEEDBACK_MAX_MAX  100000
#define DIGITAL_FEEDBACK_MAX_MIN  3

#define CFG1_CONST 0x3E00

#define DIO_CH_0_DITHER_BIT 11
#define	DIO_CH_10_DITHER_BIT 10
#define DIO_CH_1_DITHER_BIT 9
#define DIO_CH_11_DITHER_BIT 8

#define ADC_DELAY       15

#ifdef OLD
#define INPUT_SWITCH_TIME 2 /* 23.4 ms for 10000 */
#define ONE_INPUT_TIME  21  /* 212 ms for 10000 dio_in_data */
#define DIO_OUT_TIME  6 /* microseconds it takes to do dio_out !! CPU Dep.*/
#endif
#define INPUT_SWITCH_TIME  input_switch_time
#define ONE_INPUT_TIME  one_input_time
#define DIO_OUT_TIME  dio_out_time
#define CLOCK_DEAD_TIME  9

#ifdef OLD
#define CLOCK_DEAD_TIME  8 /* microseconds of dead time !! CPU dependent!*/
        /* 8 is good on Triton I chipsets */
#endif
#define CLOCK_CALIB     0x7FF8 /* microseconds that clock measures for 0xFFF0 */
#define SEC_PER_OUTPUT          3.6e-6  /* Needs to be calibrated */

#define ANGS_PER_BIT            0.32195 /* Needs to be calibrated */
#define METERS_PER_XY_VOLT      2e-8    /* 100 Angs/volt */
#define METERS_PER_Z_VOLT       3e-9    /* 30 Angs/volt */
#define G                       9.8     /* Meters/sec^2 */

#define RETRACT_TIME    1       /* Seconds to wait to retract */
#define UNRETRACT_TIME  6       /* Seconds to wait to unretract */

#define in_range        10      /* +/- 10V input range */
#define IN_MAX          65535   /* 16-bit input ADC */
#define IN_ZERO         32768
#define IN_MIN          0
#define MAX             4095    /* 12-bit output DAC */
#define ZERO            2048
#define ZERO16			0x7FFF
#define MIN             0

                /* defining the I/O card's register addresses */

#define base            0x240           /* Base I/O address */
                /* configuration and status register group */
#define cfg1            base+0x00
#define cfg2            base+0x02
#define cfg3            base+0x04
#define cfg4            base+0x14
#define stat            base+0x00
#define cntintclr       base+0x0A
#define dmaclr1         base+0x0C
#define dmaclr2         base+0x0E
                /* digital I/O register group */
#define porta           base+0x06
#define portb           base+0x07
#define portc           base+0x08
#define portd           base+0x09
                /* Real Time System Integration register group */
#define rtsishft        base+0x10
#define rtsistrb        base+0x12
                /* counter register group */
#define cntr1           base+0x18
#define cntr2           base+0x1A
#define cntr3           base+0x1C
#define cntrcmd         base+0x1E

        /******************************************************/
        /* See dio.ch for digital ch information for ch 12-15 */
        /******************************************************/

#define range_ch        12
#define range_five      0xFFF   /* Sets all channels to +/-5V */
#define range_ten       0x000   /* Sets all channels to +/-10V */

#define AD_ch           13
#define AD_strobe       0       /* Strobes ADC start conversion */

#define mode_ch 14
#define coarse_bit      0x0040          /* bit 6 */
#define fine_bit        0x0040          /* bit 6 */
#define rotate_bit      0x0080          /* bit 7 */
#define translate_bit   0x0080          /* bit 7 */
#define coarse_mode     1
#define fine_mode       2
#define rotate_mode     3
#define translate_mode  4

#define feedback_ch     14              /* bit 2  */
#define feedback_bit    0x0004

#define hold_ch 14
#define hold_bit        0x0004          /* bit 2 */

#define retract_ch 14
#define retract_bit     0x0008          /* bit 3 */

                /*input channel controlling the STM signal gain */
#define gain_ch 14
#define gain_bits       0x0F33          /* bits 0,1,4,5,8,9,10,11 */
#define x_gain_bits     0x0300          /* bits 9=x1,   8=x0 */
#define y_gain_bits     0x0C00          /* bits 11=y0, 10=y1 */
#define z_gain_bits     0x0003          /* bits 1=z1,   0=z0 */
#define z2_gain_bits    0x0030          /* bits 5=z3,   4=z2 */
#define X_TENTH_GAIN    0x0000          /* 00 */
#define X_ONE_GAIN      0x0100          /* 01 */
#define X_TEN_GAIN      0x0300          /* 11 */
#define Y_TENTH_GAIN    0x0000          /* 00 */
#define Y_ONE_GAIN      0x0800          /* 01 */
#define Y_TEN_GAIN      0x0C00          /* 11 */
#define Z_TENTH_GAIN    0x0003          /* 11 */
#define Z_ONE_GAIN      0x0002          /* 10 */
#define Z_TEN_GAIN      0x0000          /* 00 */
#define Z2_TENTH_GAIN   0x0000          /* 00 */
#define Z2_ONE_GAIN     0x0020          /* 10 */
#define Z2_TEN_GAIN     0x0030          /* 11 */

        /* input channels */
#define zi_ch           1  /* z input with feedback on */
#define i_in_ch         0  /* i input with feedback off */
#define tip_ch          0  /* tip channel during approach */        

                
                /* output channels controlling x,y,z position and tip current */
#define UNUSED_CH       1

#define x_ch            2
#define x_offset_ch     5
#define y_ch            3
#define y_offset_ch     6
#define zo_ch           4
#define i_setpoint_ch   8
#define sample_bias_ch  0
#define test_ch         9 /* for testing purposes */
#define highres_bias_ch 10		//move this to the same place as sample_bias_ch

extern int z_offset_ch;

#ifdef OLD
#define z_offset_ch     1
#endif

#define paracut         0
#define sawtooth        1

void dio_init();
void dio_set_registers();
#ifdef SEAN
void dio_out(unsigned int,unsigned int);
#endif
                /* setting up waveform generation */
unsigned int *dio_blk_setup(unsigned int,unsigned int,unsigned int,float,int);
void dio_blk_free(unsigned int *);
void dio_blk_out(unsigned int,unsigned int *);    /*generating waveform*/

void dio_in_ch(unsigned int);       /*setting input channel*/
void dio_in_data(unsigned int *);   /*read in one piece of data*/
double dio_read(unsigned int);      /*multiple reads with average of data*/

		/* special modifications to work with hop_timer */
double dio_read_hop( unsigned int);      /*multiple reads with average of data*/
void dio_in_data_hop(unsigned int *);   /*read in one piece of data*/
void hop_delay_for( float time);

                /* conversion of volts to bits or vice versa */
double dtov(unsigned int,int);
double dtov16(unsigned int); 
double dtov_bias(unsigned int,int);       
double dtov_len(int,int);        
double dtov_len16(int);
unsigned int vtod(double,int);
unsigned int vtod16(double);
unsigned int vtod_bias(double,int);
char *dtob(unsigned int);
double in_dtov(float);
int in_vtod(double);
char *in_dtob(unsigned int);
unsigned int btod(char *);

void mode(int);    /* Coarse/fine,translate/rotate */
void move_to(unsigned int,unsigned int,unsigned int);   /* Changing voltage one bit at a time */
void move_to_speed(unsigned int,unsigned int,unsigned int,int,int,int,int,
        int,int);   /* Changing voltage one bit at a time */
#ifdef OLD
void move_to_speed2(unsigned int,unsigned int,unsigned int,
                    unsigned int,unsigned int, unsigned int,
                    int,int,int,int);   /* Changing voltage one bit at a time */
#endif
void move_to_timed(unsigned int,unsigned int,unsigned int,int); 
unsigned int move_to_protect(unsigned int,unsigned int,unsigned int,int,int,float); 
unsigned int move_to_protect_hop(unsigned int,unsigned int,unsigned int,int,int,float); 
unsigned int move_to_protect2(unsigned int,unsigned int,unsigned int,int,int,
        float,int,int,int,int,int,int,int); 
#ifdef OLD
void move2_to_protect2(unsigned int,unsigned int,unsigned int,
        unsigned int,unsigned int,unsigned int,int,int,
        float,int,int,int,int,int,unsigned int *,unsigned int *); 
#endif
void move_to2(unsigned int,unsigned int,unsigned int,unsigned int);   /* Changing voltage one bit at a time */
void set_gain(unsigned int,unsigned int,unsigned int,unsigned int);     /* Setting x,y,z,z2 gains */
void set_x_gain(unsigned int);
void set_y_gain(unsigned int);
void set_z_gain(unsigned int);
void set_z2_gain(unsigned int);

void hold(int);         /* Hold on/off */
void retract(int);      /* Retract on/off */
void adc_delay();       /* ADC timing delay, appox 15 microsec */

                /* Creates parabola for x,y scan motion */
unsigned int *para_move_setup(unsigned int,unsigned int,unsigned int);

unsigned int para_size(unsigned int);   /* Digital outputs per parabola */
void para_free(unsigned int *);
unsigned int *stair_move_setup(unsigned int,unsigned int,unsigned int);

unsigned int stair_size(unsigned int);
void stair_free(unsigned int *);

unsigned int *tip_setup(unsigned int,unsigned int,unsigned int,unsigned int,int);
unsigned int *tip_zo_setup(unsigned int);
void tip_free(unsigned int *);
void tip_current(unsigned int);
void bias(unsigned int);
void ramp_bias(unsigned int,int,int,int);
void ramp_ch(unsigned int,unsigned int,int,int,int);
int get_range(int);
void set_range(int,int);
void dio_feedback(int);
int dio_digital_feedback(int,int,int,int);
unsigned int flipped_bias(unsigned int);
void dio_dither(int,int);
int dio_dither_status(int);
void dio_start_clock(int); /* argument is in microseconds */
void dio_wait_clock(); /* each call from dio_start_clock must have
                                a corresponding dio_clock_wait; no nesting! */
#ifndef NO_DIO_CARD
#define dio_out(CH,VAL) { \
        outpw(cfg1,0x0000+out1); \
        outpw(cfg1,CFG1_CONST+out1); \
        if ((VAL) > MAX)\
        { \
                outpw(porta,(unsigned int) (CH+(UNUSED_CH << 12)));\
                exit(1); \
        } \
        outpw(porta,(unsigned int) ((VAL)+(CH << 12))); }

#define dio_start_clock(TIME) { \
        if (clock_used) exit(1); \
        clock_used=1; \
        if (TIME>CLOCK_CALIB+CLOCK_DEAD_TIME) \
        { \
                outpw(cntintclr,0x0000); \
                outp(cntrcmd,0xB4); \
                outp(cntr3,0xF0);  \
                outp(cntr3,0xFF); \
                outpw(cfg3,0x2440);\
                clock_clock=TIME-CLOCK_CALIB-CLOCK_DEAD_TIME; \
        } \
        else \
        { \
                if (TIME<=CLOCK_DEAD_TIME) clock_clock=1; \
                else clock_clock=(TIME-CLOCK_DEAD_TIME) <<1;  \
                outpw(cntintclr,0x0000); \
                clock_hibyte=(unsigned char)(clock_clock >> 8); \
                clock_lobyte=(unsigned char)(clock_clock & 0x000000FF); \
                outp(cntrcmd,0xB4); \
                outp(cntr3,clock_lobyte);  \
                outp(cntr3,clock_hibyte); \
                outpw(cfg3,0x2440);\
                clock_clock=0; \
        } \
        }
      
#define dio_wait_clock() {  \
        if (!clock_clock)\
        { \
                while(!(inpw(stat)&0x4000) ) ; \
        } \
        else \
        { \
                while(!(inpw(stat)&0x4000) ) ; \
                while(clock_clock>CLOCK_CALIB) \
                { \
                        outpw(cntintclr,0x0000); \
                        outp(cntrcmd,0xB4); \
                        outp(cntr3,0xF0);  \
                        outp(cntr3,0xFF); \
                        outpw(cfg3,0x2440);\
                        clock_clock-=CLOCK_CALIB; \
                        while(!(inpw(stat)&0x4000) ) ; \
                } \
                if (clock_clock) \
                { \
                        clock_clock=clock_clock <<1;  \
                        outpw(cntintclr,0x0000); \
                        clock_hibyte=(unsigned char)(clock_clock >> 8); \
                        clock_lobyte=(unsigned char)(clock_clock & 0x000000FF); \
                        outp(cntrcmd,0xB4); \
                        outp(cntr3,clock_lobyte);  \
                        outp(cntr3,clock_hibyte); \
                        outpw(cfg3,0x2440);\
                        while(!(inpw(stat)&0x4000) ) ; \
                } \
		} \
        outpw(cfg3,0x2400); \
        outpw(cntintclr,0x0000); \
        clock_used=0; \
}
#else
#define dio_out(CH,VAL) {}
#define dio_start_clock(TIME) {}
#define dio_wait_clock() {}
#define outp(A,B) {}
#define outpw(A,B) {}
#define inp(A) 0
#define inpw(A) 0
#endif


        
#ifdef OLD
#define dio_start_clock(TIME) { \
        if (clock_used) exit(1); \
        clock_used=1; \
        if (TIME>CLOCK_CALIB) mdelay((TIME-CLOCK_CALIB)/1000), \
                clock_clock=(TIME-(int)((TIME-CLOCK_CALIB)/1000)*1000) << 1; \
        else clock_clock=TIME <<1;  \
        outpw(cntintclr,0x0000); \
        clock_hibyte=(unsigned char)(clock_clock >> 8); \
        clock_lobyte=(unsigned char)(clock_clock & 0x000000FF); \
        outp(cntrcmd,0xB4); \
        outp(cntr3,clock_lobyte);  \
        outp(cntr3,clock_hibyte); \
        outpw(cfg3,0x2440);\
        }
      
#define dio_wait_clock() {  \
        while(!(inpw(stat)&0x4000) ) ; \
        outpw(cfg3,0x2400); \
        outpw(cntintclr,0x0000); \
        clock_used=0;\
        }
#endif

//void delay_calibrate( int n);

