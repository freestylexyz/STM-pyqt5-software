#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <conio.h>
#include <math.h>
#include <time.h>
#include <sys\timeb.h>
#include <dos.h>
//#include <i86.h>
#include <string.h>
#include "dio.h"
#include "highres.h"
#include "stm.h"
#include "data.h"
#include "scan.h"
#include "common.h"
#include "hop.h"
#include <intrin.h>


extern float hop_timer;
float one_input_time,input_switch_time,dio_out_time;
float dep_read_clock_time;
extern unsigned int sample_bias;
unsigned int input_ch;          /* Current input channel for A/D start conversion */
unsigned int out1;			/* Bit 0 of cfg1 register (out1=LSB of output ch #) */
int clock_used=0;
unsigned char clock_hibyte,clock_lobyte;
unsigned int clock_clock;
char dio_string[20];

extern unsigned int bit16;
extern unsigned int out_range;		/* Output range (+/- 5 or 10 volts) */
extern unsigned int dac_data[];		/* Current output on dac channels[0-11] */
extern int feedback,z_range;
extern char string[];
#ifdef OLD
int digital_feedback_max=DIGITAL_FEEDBACK_MAX;
int digital_feedback_reread=DIGITAL_FEEDBACK_REREAD;
#endif

int z_offset_ch = 1;

int dio_dither_status(int ch)
{
    int bit;
    switch(ch)
    {
        case 0:
            bit=DIO_CH_0_DITHER_BIT;
            break;
        case 1:
            bit=DIO_CH_1_DITHER_BIT;
            break;
        default:
            mprintf("BUG: only channels 0 and 1 can be dithered");
            break;
    }
    return((dac_data[AD_ch]&(1<<bit))>0);
}

void dio_dither(int ch,int status)
{
    int bit;
    if (status!=0 && status!=1) mprintf("BUG: calling dither with >1");
    switch(ch)
    {
        case 0:
            bit=DIO_CH_0_DITHER_BIT;
            break;
        case 1:
            bit=DIO_CH_1_DITHER_BIT;
            break;
        default:
            mprintf("BUG: only channels 0 and 1 can be dithered");
            break;
    }
    dac_data[AD_ch]&=(~(1<<bit));
    dac_data[AD_ch]|=(status<<bit);
    dio_out(AD_ch,dac_data[AD_ch]);
}    

void dio_set_registers()
{
  int i;

  for(i=0;i<56;i++)
    outp(rtsishft,0x00);
  outp(rtsistrb,0x00);
  outpw(cfg1,0x0000);
  outpw(cfg2,0x0000);
  outpw(cfg3,0x0000);
  outpw(cfg4,0x0001);
  outpw(cfg1,0x0100);           /* Local reset for group 1 */
  outpw(cfg1,0x0000);
  outpw(cfg1,CFG1_CONST);           /* Port A,B handshake enable */
  outpw(cfg2,0x0100);           /* Local reset for group 2 */
  outpw(cfg2,0x0000);
  outpw(cfg2,0x0600);           /* Port C,D handshake enable */
  outpw(cfg3,0x0000);
  outpw(cfg3,0x2400);           /* Port A,B=Write ports(output); C,D=Read ports(input) */
  outpw(dmaclr1,0x0000);
  outpw(dmaclr2,0x0000);
  outpw(cntintclr,0x0000);
  outp(cntrcmd,0xB4);           /* Counter 3: 16-bit rate generator */
}    
void dio_init()
{
  int i;
  
  dio_set_registers();
  
  for(i=0;i<16;i++)         /* Initialize output values */
    dio_out(i,dac_data[i]);

}

int get_range(int ch) /* assumes range channel treats channels sequentially */
{
	if (ch==10 && bit16) return 1;
		else {
			if (dac_data[range_ch] & (1 << ch)) return 1; /* +- 5 V */
			else return 2; /* +- 10V*/
			}
}

void set_range(int ch,int range) /* assumes range_ch treats channels sequentially */
{
    dac_data[range_ch] &= ~(1 <<ch); /* clear range bit */
    /* now set it if we want +- 5V */
    if (range==1) dac_data[range_ch] |= 1<<ch;
    dio_out(range_ch,dac_data[range_ch]);
    
}

void dio_feedback(int on)
{
  if(!on) {
    dac_data[feedback_ch] &= ~feedback_bit;     /* Zero feedback bit */
    dio_out(feedback_ch,dac_data[feedback_ch]);
  }
  else {
    dac_data[feedback_ch] |= feedback_bit;      /* Set feedback bit */
    dio_out(feedback_ch,dac_data[feedback_ch]);
  }
  feedback=on;
}
    


#ifdef SEAN
void dio_out(unsigned int ch,unsigned int n)
{
  outpw(cfg1,0x0000+out1);		/* Resets output channel for another output */
  outpw(cfg1,CFG1_CONST+out1);		/* without altering bit 0 of cfg1 */
  outpw(porta,n+(ch << 12));		/* Output data(bits 0-11) and channel(bits 12-15) */
}
#endif
	/* this procedure allocates memory for, calculates and stores the waveform to be used */

unsigned int *dio_blk_setup(unsigned int ch,unsigned int wave,unsigned int n,float wave_range,int dir)
{
  unsigned int *data;
  unsigned int i;
  double v;

  data = (unsigned int *) malloc(n*sizeof(unsigned int));
  if(data == NULL) {
    printf("Malloc Failed\n");
    exit(0);
  }
  for(i=0;i<n;i++) {
    switch(wave) {
    case paracut:			/* Paracut */
      v = dir*(2*wave_range*pow((double) i/(n-6),2.0)-wave_range);
      break;
    case sawtooth:			/* Sawtooth */
      v = dir*((double) i*2*wave_range/(n-6)-wave_range);
      break;
    default:
      v = 0;
      break;
    }
    if(i==n-5)
      *(data+i) = ZERO-ZERO+(unsigned int) (zo_ch << 12); /* z motion: change 0 to offset desired */
    else if((i==n-4) || (i==n-3) || (i==n-2))
      *(data+i) = vtod(-dir*wave_range,z_range)+(unsigned int) (ch << 12); /* Drop z */
    else if(i==n-1)
      *(data+i) = ZERO+(unsigned int) (zo_ch << 12);	/* restores z */
    else
      *(data+i) = vtod(v,get_range(ch))+(unsigned int) (ch << 12);    /* continue to generate waveform */
  }
  return(data);
}

void dio_blk_free(unsigned int *data)	/* frees memory */
{
  free(data);
}

void dio_blk_out(unsigned int n,unsigned int *data)
{
  unsigned int i;

  __MACHINE(void __cdecl _disable());				/* Disable interrupts within one period of waveform */
  for(i=0;i<n;i++) {
    outpw(cfg1,0x0000+out1);		/* Reset output */
    outpw(cfg1,CFG1_CONST+out1);
    outpw(porta,*(data+i));		/* Output ch,data */
  }
  __MACHINE(void __cdecl _enable());				/* Enable interrupts */
}

unsigned int *para_move_setup(unsigned int step,unsigned int size,unsigned int offset)
{
  unsigned int *para;
  unsigned int *ptr;
  int n,i,j;

  n = para_size(step);			/* Size of parabolic increase and decrease */
  para = (unsigned int *) malloc(n*sizeof(unsigned int));
  for(i=0;i<n;i++)
    *(para+i) = 0;
  j = step;
  while(j) {				/* Generate increasing, then decreasing parabolas */
    if(n%2) {
      *(para+n/2) += 1;
      if(!--j)
        break;
    }
    for(i=n/2-1;i>=0;i--) {
      if(n%2 && i==n/2)
        i--;
      if(*(para+n-1-i)<2*i+1) {
        *(para+n-1-i) += 1;
	if(!--j)
	  break;
      }
      if(*(para+i)<2*i+1) {
        *(para+i) += 1;
	if(!--j)
	  break;
      }
    }
  }
  ptr = (unsigned int *) malloc((size*n+1)*sizeof(unsigned int));
  *ptr = offset;			/* Add initial offset */
  for(j=0;j<size;j++)			/* Make 'size' number of copies of parabola */
    for(i=0;i<n;i++)
      *(ptr+j*n+i+1) = *(ptr+j*n+i)+*(para+i);
  free(para);
  return(ptr);
}

unsigned int para_size(unsigned int n) /* Returns the minimum size to generate a parabola */
{
  int i=n;
  int j=0;

  if(n==1)
    return(1);
  while(i>0) {
    j++;
    i -= 2*(2*j-1);
  }
  if(abs(i)>=2*j-1)
    j = 2*j-1;
  else
    j = 2*j;
  return((unsigned int)j);
}

void para_free(unsigned int *ptr)
{
  free(ptr);
}

unsigned int *stair_move_setup(unsigned int step,unsigned int size,unsigned int offset)
{
//  unsigned int *stair;
  unsigned int *ptr;
  int n,i,j;

  n = stair_size(step);          /* Size of parabolic increase and decrease */
#ifdef SEAN
  stair = (unsigned int *) malloc(n*sizeof(unsigned int));
  for(i=0;i<n;i++)
    *(stair+i) = 0;
  j = step;
  while(j) {				/* Generate increasing, then decreasing parabolas */
    if(n%2) {
      *(stair+n/2) += 1;
      if(!--j)
        break;
    }
    for(i=n/2-1;i>=0;i--) {
      if(n%2 && i==n/2)
        i--;
      if(*(stair+n-1-i)<2*i+1) {
        *(stair+n-1-i) += 1;
	if(!--j)
	  break;
      }
      if(*(stair+i)<2*i+1) {
        *(stair+i) += 1;
	if(!--j)
	  break;
      }
    }
  }
#endif
  ptr = (unsigned int *) malloc((size*n+1)*sizeof(unsigned int));
  *ptr = offset;			/* Add initial offset */
  for(j=0;j<size;j++)			/* Make 'size' number of copies of parabola */
    for(i=0;i<n;i++)
      *(ptr+j*n+i+1) = *(ptr+j*n+i)+1;
  return(ptr);
}

unsigned int stair_size(unsigned int n) /* Returns the minimum size to generate a parabola */
{
/*
  int i=n;
  int j=0;

  if(n==1)
    return(1);
  while(i>0) {
    j++;
    i -= 2*(2*j-1);
  }
  if(abs(i)>=2*j-1)
    j = 2*j-1;
  else
    j = 2*j;
  return((unsigned int)j);

*/
    return(n);
}

void stair_free(unsigned int *ptr)
{
  free(ptr);
}

unsigned int *tip_setup(unsigned int xn,unsigned int zon,unsigned int xstep,unsigned int zostep,int dir)
{
  int i,j;
  unsigned int *data;

  data = (unsigned int *) malloc((2*xn+2*zon+10)*sizeof(unsigned int));

  j = 0;
  *(data+j++) = ZERO+(unsigned int) (zo_ch << 12);

  if (dir == -1) {
      for(i=0;i<(0.586*xn);i++)
        *(data+j++) = (unsigned int) (ZERO)+(unsigned int) (x_ch << 12);
      for(i=0;i<(1.414*xn);i++)
        *(data+j++) = (unsigned int) (ZERO+dir*(pow(i,2)*xstep/2/pow(xn,2))+0.5)
                                 +(unsigned int) (x_ch << 12);
    
#ifdef OLD
    for(i=0;i<xn;i++)
      *(data+j++) = (unsigned int) (ZERO+dir*(pow(i,2)*xstep/2/pow(xn,2))+0.5)
                                   +(unsigned int) (x_ch << 12);
    for(i=xn;i>=0;i--)
      *(data+j++) = (unsigned int) (ZERO+dir*(xstep-pow(i,2)*xstep/2/pow(xn,2))+0.5)
                                   +(unsigned int) (x_ch << 12);
#endif
  }
  else {
  for(i=0;i<(0.586*xn);i++)
    *(data+j++) = (unsigned int) (ZERO)+(unsigned int) (x_ch << 12);
  for(i=0;i<(1.414*xn);i++)
    *(data+j++) = (unsigned int) (ZERO+dir*(pow(i,2)*xstep/2/pow(xn,2))+0.5)
                                 +(unsigned int) (x_ch << 12);
  }
  
  for(i=0;i<6;i++)
    *(data+j++) = ZERO+zostep+(unsigned int) (zo_ch << 12);
  *(data+j++) = ZERO+(unsigned int) (x_ch << 12);

  for(i=0;i<zon;i++)
    *(data+j++) = (unsigned int) (ZERO+zostep-pow(i,2)*zostep/2/pow(zon,2)+0.5)
                                 +(unsigned int) (zo_ch << 12);
  if(zon)
    for(i=zon;i>=0;i--)
      *(data+j++) = (unsigned int) (ZERO+pow(i,2)*zostep/2/pow(zon,2)+0.5)
                                   +(unsigned int) (zo_ch << 12);
  else
    *(data+j++) = (unsigned int) ZERO+(unsigned int) (zo_ch << 12);
return(data);
}

unsigned int *tip_zo_setup(unsigned int n)
{
  int i,j;
  unsigned int *data;

  data = (unsigned int *) malloc(2*n*sizeof(unsigned int));

  j = 0;
  for(i=1;i<n;i++)
    *(data+j++) = (unsigned int) (MAX-(pow(i,2)*(MAX-ZERO)/2/pow(n,2))+0.5)+(unsigned int) (zo_ch << 12);
  for(i=n;i>=0;i--)
    *(data+j++) = (unsigned int) ZERO+(pow(i,2)*(MAX-ZERO)/2/pow(n,2)+0.5)+(unsigned int) (zo_ch << 12);
  return(data);
}

void tip_free(unsigned int *data)
{
  free(data);
}

void dio_in_ch(unsigned int ch)
{
  outpw(cfg1,ch&1);			/* Set out1 = bit 0 of input channel number */
  outpw(cfg2,ch>>1);			/* Set out2 = bit 1 of input channel number */
  input_ch = 1<<(ch*2+1);		/* Set A/D start convert bit */
  out1 = ch&1;				/* Update current out1 */
}

void dio_in_data(unsigned int *data)
{
  dio_out(AD_ch,input_ch|dac_data[AD_ch]);      /* Start A/D conversion (bit => 1) */
  dio_out(AD_ch,AD_strobe|dac_data[AD_ch]);     /* Pulse (bit => 0) */
  adc_delay();				/* Wait for conversion (approx. 15 microsec) */
  *data = inpw(portc);			/* Read input */
}

void dio_in_data_hop(unsigned int *data)
{
  dio_out(AD_ch,input_ch|dac_data[AD_ch]);      /* Start A/D conversion (bit => 1) */
  dio_out(AD_ch,AD_strobe|dac_data[AD_ch]);     /* Pulse (bit => 0) */
    /* Wait for conversion (approx. 15 microsec) */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
  *data = inpw(portc);			/* Read input */
}

double dio_read(unsigned int n)		/* Returns average of n reads of input */
{
  unsigned int i;
  double data = 0;

  for(i=0;i<n;i++) {			/* Do n times */
    dio_out(AD_ch,input_ch|dac_data[AD_ch]);        /* Start A/D conversion (bit => 1) */
    dio_out(AD_ch,AD_strobe|dac_data[AD_ch]);       /* Pulse (bit => 0) */
    adc_delay();			/* Wait for conversion (approx. 15 microsec) */
    data += (double) inpw(portc);	/* Accumulate input */
  }
  data /= n;				/* Average */
  return(data);
}

double dio_read_hop(unsigned int n)		/* Returns average of n reads of input */
{
  unsigned int i;
  double data = 0;

  for(i=0;i<n;i++) {			/* Do n times */
    dio_out(AD_ch,input_ch|dac_data[AD_ch]);        /* Start A/D conversion (bit => 1) */
    dio_out(AD_ch,AD_strobe|dac_data[AD_ch]);       /* Pulse (bit => 0) */
    /* Wait for conversion (approx. 15 microsec) */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
                outp(cntrcmd,0x74); /* delay!! */
    data += (double) inpw(portc);	/* Accumulate input */
  }
  data /= n;				/* Average */
  return(data);
}

double dtov(unsigned int data,int range) /* range=1 is +-5 V, range=2 is +=10 V*/
{
	return((double) data*range*10/0x1000-range*5);
}

double dtov16(unsigned int data)
{
	return((double) data*10/0x10000-5);
}

double dtov_bias(unsigned int data,int range)
{
	if (bit16) return dtov16(data);
	else return dtov(data, range);
}

double dtov_len(int data, int range)
{
    return((double) data*range*10/0x1000);
}

double dtov_len16(int data)
{
    return((double) data*10/0x10000);
}

unsigned int vtod(double v,int range)
{
  v = (v+range*5)*0x1000/(range*10)+0.5;
  return((unsigned int) v);
}

unsigned int vtod16(double v)
{
  v = max(-5,min(v,5));
  v = (v+5)*0x10000/(10)+0.5;
  return((unsigned int) v);
}

unsigned int vtod_bias(double v, int range)
{
	if (bit16) return vtod16(v);
	else return vtod(v,range);
}

char *dtob(unsigned int data)
{
  unsigned int i;

  if(data&1)
    strcpy(dio_string,"1");
  else
    strcpy(dio_string,"0");
  for(i=1;i<12;i++) {
    data >>= 1;
    if(data&1)
      strcat(dio_string,"1");
    else
      strcat(dio_string,"0");
  }
  return(strrev(dio_string));
}

unsigned int btod(char *str)
{
  unsigned int i;
  unsigned int data = 0;

  for(i=0;i<strlen(str);i++) {
    if(str[i]=='1')
      data++;
    data <<= 1;
  }
  data >>= 1;
  return(data);
}

double in_dtov(float data)
{
// Converts input data (16 bits) to voltage.
// data is type float to accomodate input that is read multiple times & averaged
  data++;
  return((double) data*in_range*2/0x10000-in_range);
}

int in_vtod(double data)
{
  return((int) ((data+(double)in_range)*(double)0x10000/2.0/(double)in_range-1.0));
}
  

char *in_dtob(unsigned int data)
{
  unsigned int i;

  if(data&1)
    strcpy(dio_string,"1");
  else
    strcpy(dio_string,"0");
  for(i=1;i<16;i++) {
    if(i==4 || i==8 || i==12)
      strcat(dio_string," ");		/* Add space between every four binary digits */
    data >>= 1;
    if(data&1)
      strcat(dio_string,"1");
    else
      strcat(dio_string,"0");
  }
  return(strrev(dio_string));
}

void mode(int m)
{
  if(m!=fine_mode) {
    dac_data[zo_ch] = ZERO;
    dio_out(zo_ch,dac_data[zo_ch]);
  }
  switch(m) {
  case coarse_mode:
    dac_data[mode_ch] |= coarse_bit;	/* Set coarse bit */
    break;
  case fine_mode:
    dac_data[mode_ch] &= ~fine_bit;	/* Zero fine bit */
    break;
  case rotate_mode:
    dac_data[mode_ch] &= ~rotate_bit;	/* Zero rotate bit */
    break;
  case translate_mode:
    dac_data[mode_ch] |= translate_bit;	/* Set translate bit */
    break;
  }
  dio_out(mode_ch,dac_data[mode_ch]);
}

void tip_offset(unsigned int data)
{
  dio_out(zo_ch,data);
  dac_data[zo_ch] = data;
}

void move_to(unsigned int ch,unsigned int datai,unsigned int dataf)
{

    
  if(datai<dataf)
    while(++datai<=dataf)       /* Increment output from datai to dataf */
    {
      dio_out(ch,datai);
      dio_start_clock(1000);
      dio_wait_clock();
    }
  else if(datai>dataf)
    while(--datai>=dataf)       /* Decrement output from datai to dataf */
    {
      dio_out(ch,datai);
      dio_start_clock(1000);
      dio_wait_clock();
      if (datai==0) goto END_MOVE_TO;
    }
  else dio_out(ch,dataf); /* just to be sure */
END_MOVE_TO:
    datai=0;
}

void move_to_speed(unsigned int ch,unsigned int datai,unsigned int dataf,
    int time /* in micro seconds*/,int digital,int Imin,int Imax,
    int digital_feedback_max,int digital_feedback_reread)
{    
    if (dataf>MAX)
    {
        sprintf(string,"Tried to move to %d on channel %d\nI'll just move to %d",
            dataf,ch,MAX);
        MessageBox(GetFocus(),string,"Bug Alert!",MB_OKCANCEL);
        dataf=MAX;
    }
          
  
  if(datai<dataf)
    while(++datai<=dataf)       /* Increment output from datai to dataf */
    {
      dio_out(ch,datai);
      dio_start_clock(time);
      dio_wait_clock();
      if (digital)
      {
        dio_digital_feedback(Imin,Imax,digital_feedback_max,
            digital_feedback_reread);  

      }
       
    }
  else if(datai>dataf)
    while(--datai>=dataf)       /* Decrement output from datai to dataf */
    {
      dio_out(ch,datai);
      dio_start_clock(time);
      dio_wait_clock();
      if (digital)
      {
        dio_digital_feedback(Imin,Imax,digital_feedback_max,
            digital_feedback_reread);
      }
      if (datai==0) goto END_MOVE_TO_SPEED;
    }
  else dio_out(ch,dataf); /* just to be sure */
END_MOVE_TO_SPEED:
    datai=0;

}
#define SGN(a) (((a)>0) ? 1 : -1)

#ifdef OLD
void move_to_speed2(unsigned int chx,unsigned int x1,unsigned int x2,
            unsigned int chy,unsigned int y1,unsigned int y2,
            int time /* in micro seconds*/,int digital,int Imin,int Imax)
{
    unsigned int I;
    int d,x,y,ax,ay,sx,sy,dx,dy;
    
    dx = x2-x1;  ax = abs(dx)<<1;  sx = SGN(dx);
    dy = y2-y1;  ay = abs(dy)<<1;  sy = SGN(dy);

    x = x1;
    y = y1;
    if (ax>ay) {		/* x dominant */
	d = ay-(ax>>1);
	for (;;) {
/*
        (*dotproc)(x, y);
*/
        if (x==x2) return;
	    if (d>=0) {
        move_to_speed(chy,y,y+sy,time,digital,Imin,Imax);
        y += sy;
		d -= ax;
	    }
        move_to_speed(chx,x,x+sx,time,digital,Imin,Imax);
        x += sx;
	    d += ay;
	}
    }
    else {			/* y dominant */
	d = ax-(ay>>1);
	for (;;) {
/*
        (*dotproc)(x, y);
*/
        if (y==y2) return;
	    if (d>=0) {
        move_to_speed(chx,x,x+sx,time,digital,Imin,Imax);
        x += sx;
		d -= ay;
	    }
        move_to_speed(chy,y,y+sy,time,digital,Imin,Imax);
        y += sy;
	    d += ax;
	}
    }
    
    
}
#endif
#ifdef OLD
void move2_to_protect2(unsigned int chx,unsigned int x1,unsigned int x2,
            unsigned int chy,unsigned int y1,unsigned int y2,
    int time /* in micro seconds*/, int crash_protection,float limit_percent,
    int digital,int Imin,int Imax,int digital_abort,int force_it,
    unsigned int *final_x,unsigned int *final_y)
{
    unsigned int I;
    int d,x,y,ax,ay,sx,sy,dx,dy;
    
    dx = x2-x1;  ax = abs(dx)<<1;  sx = SGN(dx);
    dy = y2-y1;  ay = abs(dy)<<1;  sy = SGN(dy);

    x = x1;
    y = y1;
    if (ax>ay) {		/* x dominant */
	d = ay-(ax>>1);
	for (;;) {
/*
        (*dotproc)(x, y);
*/
        if (x==x2) return;
	    if (d>=0) {
        *final_y=move_to_protect2(chy,y,y+sy,time,crash_protection,limit_percent,digital,Imin,Imax,digital_abort,force_it);
        if (*final_y!=y+sy) return;
        y += sy;
		d -= ax;
	    }
        *final_x=move_to_protect2(chx,x,x+sx,time,crash_protection,limit_percent,digital,Imin,Imax,digital_abort,force_it);
        if (*final_x!=x+sx) return;
        x += sx;
	    d += ay;
	}
    }
    else {			/* y dominant */
	d = ax-(ay>>1);
	for (;;) {
/*
        (*dotproc)(x, y);
*/
        if (y==y2) return;
	    if (d>=0) {
        *final_x=move_to_protect2(chx,x,x+sx,time,crash_protection,limit_percent,digital,Imin,Imax,digital_abort,force_it);
        if (*final_x!=x+sx) return;
        x += sx;
		d -= ay;
	    }
        *final_y=move_to_protect2(chy,y,y+sy,time,crash_protection,limit_percent,digital,Imin,Imax,digital_abort,force_it);
        if (*final_y!=y+sy) return;
        y += sy;
	    d += ax;
	}
    }
    
}
#endif
void move_to_timed(unsigned int ch,unsigned int datai,unsigned int dataf,
    int time /* in micro seconds*/)
{

    if (dataf>MAX)
    {
        sprintf(string,"Tried to move to %d on channel %d\nI'll just move to %d",
            dataf,ch,MAX);
        MessageBox(GetFocus(),string,"Bug Alert!",MB_OKCANCEL);
        dataf=MAX;
    }
    if (datai>MAX)
    {
        sprintf(string,"Tried to move from %d on channel %d\nI'll just move from %d",
            datai,ch,MAX);
        MessageBox(GetFocus(),string,"Bug Alert!",MB_OKCANCEL);
        datai=MAX;
    }
  if(datai<dataf)
    while(++datai<=dataf)       /* Increment output from datai to dataf */
    {
      dio_out(ch,datai);
      dio_start_clock(time);
      dio_wait_clock();
    }
  else if(datai>dataf)
    while(--datai>=dataf)       /* Decrement output from datai to dataf */
    {
      dio_out(ch,datai);
      dio_start_clock(time);
      dio_wait_clock();
      if (datai==0) goto END_MOVE_TO_TIMED;
    }
  else dio_out(ch,dataf); /* just to be sure */
END_MOVE_TO_TIMED:
    datai=0;

}
unsigned int move_to_protect(unsigned int ch,unsigned int datai,unsigned int dataf,
    int time /* in micro seconds*/, int crash_protection,float limit_percent)
{

    unsigned int z;
    unsigned int high_limit,low_limit;
    
    if (dataf>MAX)
    {
        sprintf(string,"Tried to move to %d on channel %d\nI'll just move to %d",
            dataf,ch,MAX);
        MessageBox(GetFocus(),string,"Alert!",MB_OKCANCEL);
        dataf=MAX;
    }
    if (datai>MAX)
    {
        sprintf(string,"Tried to move from %d on channel %d\nI'll just move from %d",
            datai,ch,MAX);
        MessageBox(GetFocus(),string,"Alert!",MB_OKCANCEL);
        datai=MAX;
    }

    high_limit=IN_MAX-(unsigned int) ((float)IN_MAX*limit_percent/100);
    low_limit=(unsigned int)((float)IN_MAX*limit_percent/100);
    dio_in_ch(zi_ch);
    if(datai<dataf)
    {
        while(++datai<=dataf)       /* Increment output from datai to dataf */
        {
            dio_out(ch,datai);
            dio_start_clock(time);
            dio_wait_clock();
            if (crash_protection!=CRASH_DO_NOTHING)
            {
                dio_in_data(&z);
                if (z>high_limit || z<low_limit)
                {
                    if (crash_protection==CRASH_STOP)
                    {
                        goto END_MOVE_TO_PROTECT;
                    }
                    else
                    {
                        if (!auto_z_above(IN_ZERO)) 
                        {
                            goto END_MOVE_TO_PROTECT;
                        }
                    }
                }
            }
        }
        datai--;
    }
    
    else if(datai>dataf)
    {
        while(--datai>=dataf)       /* Decrement output from datai to dataf */
        {
            dio_out(ch,datai);
            dio_start_clock(time);
            dio_wait_clock();
            if (crash_protection!=CRASH_DO_NOTHING)
            {
                dio_in_data(&z);
                if (z>high_limit || z<low_limit)
                {
                    if (crash_protection==CRASH_STOP)
                    {
                        goto END_MOVE_TO_PROTECT;
                    }
                    else
                    {
                        if (!auto_z_above(IN_ZERO)) 
                        {
                            goto END_MOVE_TO_PROTECT;
                        }
                    }
                }       
            }
            if (datai==0) goto END_MOVE_TO_PROTECT;
        }
        datai++;
    }
    else dio_out(ch,dataf); /* just to be sure */
END_MOVE_TO_PROTECT:    
    return(datai);
}

unsigned int move_to_protect_hop(unsigned int ch,unsigned int datai,unsigned int dataf,
    int time /* in micro seconds*/, int crash_protection,float limit_percent)
{
	// crash protection must be CRASH_STOP or CRASH_DO_NOTHING

    unsigned int z;
    unsigned int high_limit,low_limit;
    
    if (dataf>MAX)
    {
        sprintf(string,"Tried to move to %d on channel %d\nI'll just move to %d",
            dataf,ch,MAX);
        MessageBox(GetFocus(),string,"Alert!",MB_OKCANCEL);
        dataf=MAX;
    }
    if (datai>MAX)
    {
        sprintf(string,"Tried to move from %d on channel %d\nI'll just move from %d",
            datai,ch,MAX);
        MessageBox(GetFocus(),string,"Alert!",MB_OKCANCEL);
        datai=MAX;
    }

    high_limit=IN_MAX-(unsigned int) ((float)IN_MAX*limit_percent/100);
    low_limit=(unsigned int)((float)IN_MAX*limit_percent/100);
    dio_in_ch(zi_ch);
    if(datai<dataf)
    {
        while(++datai<=dataf)       /* Increment output from datai to dataf */
        {
            dio_out(ch,datai);
#ifdef OLD_HOP_TIMING
            dio_start_clock(time);
            dio_wait_clock();
#else
	    hop_delay_for( time);
#endif
            if (crash_protection!=CRASH_DO_NOTHING)
            {
                dio_in_data_hop(&z);
                if (z>high_limit || z<low_limit)
                {
                    if (crash_protection==CRASH_STOP)
                    {
                        goto END_MOVE_TO_PROTECT_HOP;
                    }
                }
            }
        }
        datai--;
    }
    	
    else if(datai>dataf)
    {
        while(--datai>=dataf)       /* Decrement output from datai to dataf */
        {
            dio_out(ch,datai);
#ifdef OLD_HOP_TIMING
            dio_start_clock(time);
            dio_wait_clock();
#else
	    hop_delay_for( time);
#endif
            if (crash_protection!=CRASH_DO_NOTHING)
            {
                dio_in_data_hop(&z);
                if (z>high_limit || z<low_limit)
                {
                    if (crash_protection==CRASH_STOP)
                    {
                        goto END_MOVE_TO_PROTECT_HOP;
                    }
                }       
            }
            if (datai==0) goto END_MOVE_TO_PROTECT_HOP;
        }
        datai++;
    }
    else dio_out(ch,dataf); /* just to be sure */
END_MOVE_TO_PROTECT_HOP:    
    return(datai);
}

unsigned int move_to_protect2(unsigned int ch,unsigned int datai,
    unsigned int dataf,
    int time /* in micro seconds*/, int crash_protection,float limit_percent,
    int digital,int Imin,int Imax,int digital_abort,int force_it,
    int digital_feedback_max,int digital_feedback_reread)
{

    unsigned int z;
    unsigned int high_limit,low_limit;
/*
        sprintf(string,"Tried to move to %d on channel %d",
            dataf,ch);
        MessageBox(GetFocus(),string,"Bug Alert!",MB_OKCANCEL);
*/
    if (dataf>MAX)
    {
        sprintf(string,"Tried to move to %d on channel %d\nI'll just move to %d",
            dataf,ch,MAX);
        MessageBox(GetFocus(),string,"Bug Alert!",MB_OKCANCEL);
        dataf=MAX;
    }
    if (datai>MAX)
    {
        sprintf(string,"Tried to move from %d on channel %d\nI'll just move from %d",
            datai,ch,MAX);
        MessageBox(GetFocus(),string,"Bug Alert!",MB_OKCANCEL);
        datai=MAX;
    }
    high_limit=IN_MAX-(unsigned int) ((float)IN_MAX*limit_percent/100);
    low_limit=(unsigned int)((float)IN_MAX*limit_percent/100);
//    dio_in_ch(zi_ch);
    if(datai<dataf)
    {
        while(++datai<=dataf)       /* Increment output from datai to dataf */
        {
            dio_out(ch,datai);
            dio_start_clock(time);
            dio_wait_clock();
            if (digital)
            {
                if (!dio_digital_feedback(Imin,Imax,digital_feedback_max,
                    digital_feedback_reread))
                    if (digital_abort && !force_it)
                    {
                        goto END_MOVE_TO_PROTECT2;
                    }
            }
            if (crash_protection!=CRASH_DO_NOTHING)
            {
                dio_in_ch(zi_ch);
                dio_in_data(&z);
                if (z>high_limit || z<low_limit)
                {
                    if (crash_protection==CRASH_STOP && !force_it)
                    {
                        goto END_MOVE_TO_PROTECT2;
                    }
                    else
                    {
                        if (!auto_z_above(IN_ZERO))
                            if (!force_it) 
                            {
                                goto END_MOVE_TO_PROTECT2;
                            }
                    }
                }
            }
        }
        datai--;
    }
    
    else if(datai>dataf)
    {
        while(--datai>=dataf)       /* Decrement output from datai to dataf */
        {
            dio_out(ch,datai);
            dio_start_clock(time);
            dio_wait_clock();
            if (digital)
            {
                if (!dio_digital_feedback(Imin,Imax,digital_feedback_max,
                    digital_feedback_reread))
                    if (digital_abort && !force_it)
                    {
                        goto END_MOVE_TO_PROTECT2;
                    }
            }
            if (crash_protection!=CRASH_DO_NOTHING && !force_it)
            {
                dio_in_ch(zi_ch);
                dio_in_data(&z);
                if (z>high_limit || z<low_limit)
                {
                    if (crash_protection==CRASH_STOP)
                    {
                        goto END_MOVE_TO_PROTECT2;
                    }
                    else
                    {
                        if (!auto_z_above(IN_ZERO))
                            if (!force_it) 
                            {
                                goto END_MOVE_TO_PROTECT2;
                            }
                    }
                }       
            }
            if (datai==0) goto END_MOVE_TO_PROTECT2;
        }
        datai++;
    }
    else dio_out(ch,datai); /* just to be sure */
END_MOVE_TO_PROTECT2:
    return(datai);
}

void move_to2(unsigned int ch,unsigned int datai,unsigned int dataf,unsigned int steps)
{
    int i;

  if(datai<dataf)
    while(datai<=dataf)       /* Increment output from datai to dataf */
    {
    for(i=0;i<steps;i++)
    {
      dio_out(ch,datai);
      datai++;
    }
      dio_start_clock(1000); /* wait roughly 1 ms, remove later */
      dio_wait_clock();
    }
  else if(datai>dataf)
    while(datai>=dataf)       /* Decrement output from datai to dataf */
    {
    for(i=0;i<steps;i++)
    {
      dio_out(ch,datai);
      datai--;
    }
      dio_start_clock(1000); /* wait roughly 1 ms, remove later */
      dio_wait_clock();
    }
dio_out(ch,dataf);    
}

void set_gain(unsigned int x_gain,unsigned int y_gain,unsigned int z_gain,unsigned int z2_gain)
{

  dac_data[gain_ch] &= ~gain_bits;  /* Mask out gain bits */
  dac_data[gain_ch] += x_gain+y_gain+z_gain+z2_gain;	/* Set gains */
  
  dio_out(gain_ch,dac_data[gain_ch]);
}
void set_x_gain(unsigned int x_gain)
{

  dac_data[gain_ch] &= ~x_gain_bits;  /* Mask out gain bits */
  dac_data[gain_ch] += x_gain;    /* Set gains */
  
  dio_out(gain_ch,dac_data[gain_ch]);
}
void set_y_gain(unsigned int y_gain)
{

  dac_data[gain_ch] &= ~y_gain_bits;  /* Mask out gain bits */
  dac_data[gain_ch] += y_gain;    /* Set gains */
  
  dio_out(gain_ch,dac_data[gain_ch]);
}
void set_z_gain(unsigned int z_gain)
{

  dac_data[gain_ch] &= ~z_gain_bits;  /* Mask out gain bits */
  dac_data[gain_ch] += z_gain;    /* Set gains */
  
  dio_out(gain_ch,dac_data[gain_ch]);
}
void set_z2_gain(unsigned int z2_gain)
{

  dac_data[gain_ch] &= ~z2_gain_bits;  /* Mask out gain bits */
  dac_data[gain_ch] += z2_gain;    /* Set gains */
  
  dio_out(gain_ch,dac_data[gain_ch]);
}

void hold(int hold_on)
{
  if(hold_on) {
    dac_data[hold_ch] &= ~hold_bit;     /* Zero hold bit */
    dio_out(hold_ch,dac_data[hold_ch]);
  }
  else {
    dac_data[hold_ch] |= hold_bit;      /* Set hold bit */
    dio_out(hold_ch,dac_data[hold_ch]);
  }
}

void retract(int retract_on)
{
  if(retract_on) {
    dac_data[retract_ch] |= retract_bit;	/* Set retract bit */
    dio_out(retract_ch,dac_data[retract_ch]);
    dio_start_clock(RETRACT_TIME*1000000);            /* Wait (defined int dio.h) */
    dio_wait_clock();
  }
  else {
    dac_data[retract_ch] &= ~retract_bit;	/* Zero retract bit */
    dio_out(retract_ch,dac_data[retract_ch]);
    dio_start_clock(UNRETRACT_TIME*1000000);          /* Wait (defined in dio.h) */
    dio_wait_clock();
  }
}

void adc_delay()		/* Should give approx. 15 microsec */
{
//  _disable();           /* Disable interrupts */
  dio_start_clock(ADC_DELAY);
  dio_wait_clock();
//  _enable();            /* Enable interrupts */
}

void tip_current(unsigned int data)
{
  dio_out(i_setpoint_ch,data);
  dac_data[i_setpoint_ch] = data;
}

void bias(unsigned int data)
{
  if(bit16 == 0)
  {
    dio_out(sample_bias_ch,data);
    dac_data[sample_bias_ch] = data;
  }
  else hires_bias(data);
}

void ramp_bias(unsigned int data,int time,int skip,int bits)
{
    unsigned int datai=dac_data[sample_bias_ch];

	if(bit16 == 0)
	{
		if (data>MAX) return;
		time-=DIO_OUT_TIME;
	  if (data>ZERO && dac_data[sample_bias_ch]<ZERO)
	  {
		if (datai<ZERO-skip)
		{
			while(datai<ZERO-skip)
			{
				datai+=bits;
				if (datai>ZERO-skip) datai=ZERO-skip;
				dio_out(sample_bias_ch,datai);
				dio_start_clock(time);
				dio_wait_clock();
			}
		}
		dio_out(sample_bias_ch,ZERO+skip);
		datai=ZERO+skip;
		dio_start_clock(time);
		dio_wait_clock();
		while(datai<data)
		{
			datai+=bits;
			if (datai>data) datai=data;
			dio_out(sample_bias_ch,datai);
			dio_start_clock(time);
			dio_wait_clock();
		}
	  }
	  else if (data<ZERO && dac_data[sample_bias_ch]>ZERO)
	  {
		if (datai>ZERO+skip)
		{
			while(datai>ZERO+skip)
			{
				datai-=bits;
				if (datai<ZERO+skip) datai=ZERO+skip;
				dio_out(sample_bias_ch,datai);
				dio_start_clock(time);
				dio_wait_clock();
			}
		}
		dio_out(sample_bias_ch,ZERO-skip);
		datai=ZERO-skip;
		dio_start_clock(time);
		dio_wait_clock();
		while(datai>data)
		{
			if (datai>bits) datai-=bits;
			else datai=0;
			if (datai<data) datai=data;
			dio_out(sample_bias_ch,datai);
			dio_start_clock(time);
			dio_wait_clock();
		}
	  }
	  else
	  {
		while(datai<data)  
		{
			datai+=bits;
			if (datai>data) datai=data;
			dio_out(sample_bias_ch,datai);
			dio_start_clock(time);
			dio_wait_clock();
		}
		while(datai>data)
		{
			if (datai>bits) datai-=bits;
			else datai=0;
			if (datai<data) datai=data;
			dio_out(sample_bias_ch,datai);
			dio_start_clock(time);
			dio_wait_clock();
		}
	  }  
	  dac_data[sample_bias_ch] = data;
	}
	else ramp_hires_bias(data,time,skip,bits);
}

void ramp_ch(unsigned int ch,unsigned int data,int time,int skip,int bits)
{
    unsigned int datai=dac_data[ch];
    if (data>MAX) return;
    time-=DIO_OUT_TIME;
  if (data>ZERO && dac_data[ch]<ZERO)
  {
    if (datai<ZERO-skip)
    {
        while(datai<ZERO-skip)
        {
            datai+=bits;
            if (datai>ZERO-skip) datai=ZERO-skip;
            dio_out(ch,datai);
            dio_start_clock(time);
            dio_wait_clock();
        }
    }
    dio_out(ch,ZERO+skip);
    datai=ZERO+skip;
    dio_start_clock(time);
    dio_wait_clock();
    while(datai<data)
    {
        datai+=bits;
        if (datai>data) datai=data;
        dio_out(ch,datai);
        dio_start_clock(time);
        dio_wait_clock();
    }
  }
  else if (data<ZERO && dac_data[ch]>ZERO)
  {
    if (datai>ZERO+skip)
    {
        while(datai>ZERO+skip)
        {
            datai-=bits;
            if (datai<ZERO+skip) datai=ZERO+skip;
            dio_out(ch,datai);
            dio_start_clock(time);
            dio_wait_clock();
        }
    }
    dio_out(ch,ZERO-skip);
    datai=ZERO-skip;
    dio_start_clock(time);
    dio_wait_clock();
    while(datai>data)
    {
        if (datai>bits) datai-=bits;
        else datai=0;
        if (datai<data) datai=data;
        dio_out(ch,datai);
        dio_start_clock(time);
        dio_wait_clock();
    }
  }
  else
  {
    while(datai<data)  
    {
        datai+=bits;
        if (datai>data) datai=data;
        dio_out(ch,datai);
        dio_start_clock(time);
        dio_wait_clock();
    }
    while(datai>data)
    {
        if (datai>bits) datai-=bits;
        else datai=0;
        if (datai<data) datai=data;
        dio_out(ch,datai);
        dio_start_clock(time);
        dio_wait_clock();
    }
  }  
  dac_data[ch] = data;
}
int dio_digital_feedback(int Imin,int Imax,int digital_feedback_max,
    int digital_feedback_reread)
{
    unsigned int I;
    int count=0,done=0,i;
    
    dio_in_ch(i_in_ch);
    dio_in_data(&I);
    if (dac_data[sample_bias_ch]<ZERO) I=0xFFFF-I;
                        
    while(!done)
    {
        while((I<Imin || I>Imax) && count<digital_feedback_max) 
        {
            count++;
            dio_in_data(&I);
            if (dac_data[sample_bias_ch]<ZERO) I=0xFFFF-I;
        }
        if (count<digital_feedback_max)
        {
            i=0;
            while((i<(digital_feedback_reread-1)) && (I>=Imin && I<=Imax))
            {
                i++;
                dio_in_data(&I);
                if (dac_data[sample_bias_ch]<ZERO) I=0xFFFF-I;
            }
            if (i==digital_feedback_reread-1) done=1;   
        }
        else done=1;
    }
    return (count<digital_feedback_max);
    
}
#ifdef OLD
#ifdef DEBUG
int 
#else
void
#endif
mdelay(int milli)
{
    static struct timeb start_time, end_time;
#ifdef DEBUG
    int result=0;
#endif
    
    ftime(&start_time);
    ftime(&end_time);
    while((end_time.millitm-start_time.millitm+(end_time.time-
        start_time.time)*1000)<milli) 
    {
        ftime(&end_time);
#ifdef DEBUG
        result++;
#endif
    }
#ifdef DEBUG
    return(result);
#endif
}
#endif

unsigned int flipped_bias(unsigned int bias)
{
#ifdef DAC16
	if (bit16) return (ZERO16 -(int) ((int) bias-(int) ZERO16)-1);
	else
    return (ZERO -(int) ((int) bias-(int) ZERO)-1);
#else
	return (ZERO -(int) ((int) bias-(int) ZERO)-1);
#endif
}

void hop_delay_for( float time)
{
    float start_time;

    READ_HOP_TIMER();
    start_time = hop_timer;
    while( !( (hop_timer - start_time) >= time)) {
	READ_HOP_TIMER();
    }
}

/*
void delay_calibrate( int n) {
	int i;
	for( i = 1; i < (n+1); i++) outp(cntrcmd,0x74); // delay!!
}
*/
