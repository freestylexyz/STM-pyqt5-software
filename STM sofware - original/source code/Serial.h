//Debug option for testing with serial connection to another PC
//#define SERIAL_TEST

#define DSP_CLOCK_PERIOD 11.625744	/*dsp clock period in nsec*/
#define MAX_SERIAL_DELAY 200000		// 200,000 microseconds

#define AUTOPLUS 0
#define AUTOMINUS 1

// STM Control Command Set
// ***********************
#define DSP_DATA_SIZE 2048		//max number of data "slots" in dsp memory

#define DSPCMD_VERSION "v"
#define VERSION_SIZE 12			//number of chars in reply: VVVVDDMMYYYY

#define DSPCMD_OUTPUT 'o'
#define	OUTPUT_SIZE	2			//number of additional chars to output (16 bits)

#define DSPCMD_INPUT 'i'
#define	INPUT_CH_SIZE 1			//have to send one char (8 bits), but only 2 LS matter
#define INPUT_DATA_SIZE 2		//read back two chars (16 bits) of data

#define DSPCMD_DELAY 'd'
#define DELAY_SIZE 3			//max delay can be 0xFFFFFF cycles (about 0.2 sec)

#define DSPCMD_RAMP	'r'
#define RAMP_SIZE 8

#define DSPCMD_RAMPREAD	'R'
#define	RAMP_READ_SIZE 9

#define DSPCMD_SQUAREWAVE 'q'
#define SQUARE_SIZE 7			// 2 bytes each for high, low biases + 3 bytes for delay

#define DSPCMD_TIPSETUP 't'
#define TIPSETUP_SIZE 15		// 2 bytes each for xparab,xzero,zparab sizes (in cyles) +
								// 3 bytes each for x,z multipliers + 3 bytes for delay factor

#define DSPCMD_GIANTSTEP 'g'
#define GIANT_SIZE 2			// 2 bytes: 15 bits for num steps,
								// 1 bit for direction (bit high = up, bit low = down)
/////////////////////////////////////02132012////////////////////////////////////////////
#define DSPCMD_TRANSLATE_SAMPLE 'a'
#define TRANSLATE_SAMPLE_SIZE 2	// 2 bytes: 15 bits for num steps,
								// 1 bit for direction 
/////////////////////////////////////02132012////////////////////////////////////////////
#define DSPCMD_TIPAPPROACH 'T'
#define APPROACH_SIZE 7			// 1 byte each for # giant & baby step size
								// + 2 bytes for min tunneling current (in bits)
								// + 3 bytes for baby parabola multiplier

#define DSPCMD_AUTOMINUS '-'
#define DSPCMD_AUTOPLUS '+'
#define AUTO_SIZE 3				// 2 bytes for current z offset, 1 byte for ch

#define DSPCMD_SCANSETUP 's'
#define SCANSETUP_SIZE 16		// 1 byte crash protection, 2 bytes protection limit
								// 2 bytes current z offset, 1 byte z offset ch
								// 2 scan size, 2 step size, 3 interstep delay,
								// 3 read delay

#define DSPCMD_SCANLINE 'S'
#define SCANLINE_SIZE 1			// 1 byte for output ch, direction, read on/off, newline

#define SCANLINE_FORWARD 0x40
#define SCANLINE_BACKWARD 0x00
#define SCANLINE_READON 0x80
#define SCANLINE_READOFF 0x00
#define SCANLINE_FIRSTLINE 0x20
#define SCANLINE_OTHERLINE 0x00
#define SCANLINE_NOERR 0
#define SCANLINE_ERROR 1
#define SCANLINE_COMMERR 2
#define SCANLINE_CRASH 3

#define DSPCMD_RSSETUP 'Q'
#define RSSETUP_SIZE 128*3

#define DSPCMD_TRACKSETUP 'k'
#define TRACKSETUP_SIZE 13

#define DSPCMD_TRACK 'K'
#define TRACK_SIZE 4

#define DSPCMD_TRACK_CONT 'c'	//continues tracking from the current position

#define DSPCMD_HIGHRESBIAS 'H'	//sends a 16bit output to ch10
#define HIGHRESBIAS_SIZE 3

#define DSPCMD_SPECSETUP 'y'
#define SPECSETUP_SIZE 11		// 2 bytes for lower value, 2 bytes for upper value,
								// 1 byte for step size, 3 bytes for move delay, 3 bytes
								// for read delay

#define DSPSCMD_SPEC 'Y'
#define SPEC_SIZE 1				// 1 byte for output ch and flags (fwd/bkwd)

#define DSPCMD_DEPSETUP		'p'
#define DEPSETUP_SIZE		34

#define DSPCMD_DEP			'P'

#define DSPCMD_HOPSETUP		'j'
#define HOPSETUP_SIZE		20

#define DSPCMD_HOP			'J'

#define DSPCMD_LMSETUP		'l'
#define LMSETUP_SIZE		21  // long ch start(2 bytes), long ch end (2 bytes),
								// short ch start (2), z start (2), read every (1),
								// long/short ratio (3), long/z ratio (3)
								// move delay (3), read delay (3)


#define DSPCMD_LATMAN		'L'
#define LATMAN_SIZE			2	// 1 for long ch, 1 for short ch

#define DSPCMD_STOP			'!'

#define DSPCMD_UPDATE		'#'

// ***********************

int write_serial(char *,int);
int read_serial(char *,int,int *);
unsigned int read_2_serial();
unsigned int read_3_serial();
int open_serial();
void close_serial();
void serial_settings();

void cont(); //sends 'c' to the DSP 

void dio_out_serial(unsigned int,unsigned int);
void dio_in_serial(unsigned int,unsigned int *);
int read_dsp_version(char *);
void serial_soft_delay(int);
unsigned int ramp_serial(unsigned int,unsigned int,unsigned int,int,int);
void ramp_read_serial(unsigned int,unsigned int,unsigned int,unsigned int,int,int *,int);

unsigned int ramp_bias16_serial(unsigned int,unsigned int,int);

void stop_bias_square();
void start_bias_square(unsigned int,unsigned int,int);

void tip_steps_setup(float,float,unsigned int,unsigned int,unsigned int,int);
void tip_steps(unsigned int,int);

int tip_steps_update(unsigned int);
void tip_steps_terminate();
void approach_serial(int,int,unsigned int,float);
unsigned int tip_approach_update(int *);

void track_setup(int,int,int,float,float); //(average,sample_every,delay,plane_a,plane_b)
void track_start(int,int,int);
void track_again();
void track_terminate();

void auto_serial(int);

void readseq_setup(datadef *);
void scan_setup(int,float,int,int);
int scan_line(unsigned int,int,int,int,float *,unsigned int *);

void spec_setup(unsigned int,unsigned int,int,int,int);
void spec_start(int,int,int);
char spec_continue();
void spec_finish(float *,int);

void dep_setup(int,int,int,int,int,int,int,int,int,int,int,int,double,int,int);
void dep_start();

void hop_setup(int,int,int,int,int,float,float);
void hop_start();
int hop(int,int,int *,int *);

void latman_setup(unsigned int,unsigned int,unsigned int,unsigned int,
				  int,float,float,int,int);
void latman(int,int,int,int,int,unsigned int *,unsigned int *,int *,float *,int,int);

void retract_serial(int);
void mode_serial(int);
void dio_feedback_serial(int);
void dio_init_serial();
void set_range_serial(int,int);
void tip_current_serial(unsigned int);
void bias_serial(unsigned int);
void highres_bias_serial(unsigned int);
void set_gain_serial(unsigned int,unsigned int,unsigned int,unsigned int);
void set_z2_gain_serial(unsigned int);
void set_z_gain_serial(unsigned int);
void set_y_gain_serial(unsigned int);
void set_x_gain_serial(unsigned int);
void dio_dither_serial(int,int);
void move_to_serial(unsigned int,unsigned int,unsigned int);
unsigned int move_to_protect_serial(unsigned int,unsigned int,unsigned int,int,int,float);

