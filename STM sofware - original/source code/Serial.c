#include <windows.h>
#include <winerror.h>
#include <math.h>
#include <stdio.h>
#include "common.h"
#include "dio.h"
#include "highres.h"
#include "serial.h"
#include "tip.h"
#include "stm.h"
#include "data.h"
#include "scan.h"

extern unsigned int dac_data[];		/* Current output on dac channels[0-15] */
extern datadef *scan_defaults_data[];
extern int scan_current_default;
extern unsigned int bit16;
extern int feedback;
extern unsigned int scan_size;
extern unsigned int scan_step;
extern unsigned int scan_z;

extern char string[];

HANDLE hCom1;
DCB dcbCom1 = {0};

static int steps_to_words(datadef *);

int open_serial()
{
// Open and initialize the serial communications port.
	
	hCom1 = CreateFile("COM1",GENERIC_READ | GENERIC_WRITE,0,0,OPEN_EXISTING,
						/*FILE_FLAG_OVERLAPPED*/ 0,0);
	if(hCom1 == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}

}

void close_serial()
{
	if(hCom1) CloseHandle(hCom1);
}

void serial_settings()
{
	COMMTIMEOUTS timeouts;
	
	// set up serial timeouts
	GetCommTimeouts(hCom1,&timeouts);
    timeouts.ReadIntervalTimeout = 0;	// 1000 ms between chars 
    timeouts.ReadTotalTimeoutMultiplier = 0; 
    timeouts.ReadTotalTimeoutConstant = 1000; 
    timeouts.WriteTotalTimeoutMultiplier = 0; 
//    timeouts.WriteTotalTimeoutConstant stays at default
	SetCommTimeouts(hCom1,&timeouts);

	// set up serial settings
	if(GetCommState(hCom1,&dcbCom1))
	{
		dcbCom1.BaudRate = 38400;
		dcbCom1.ByteSize = 8;
		dcbCom1.fParity = FALSE;
		dcbCom1.Parity = NOPARITY;
		// NOTE: no flow control by default
		SetCommState(hCom1,&dcbCom1);
	}
	else
	{
		// handle any possible errors
	}

}

int write_serial(char *write_buffer,int bytes_to_write)
{
// Write the first bytes_to_write characters in the write_buffer
// to hCom1. Returns 1 if all bytes were written. Otherwise 0.
	int bytes_written = 0;
	int err = NO_ERROR;

	if(!WriteFile(hCom1,write_buffer,bytes_to_write,&bytes_written,NULL))
	{
		err = GetLastError(); // check error code
		return 0;
	}
	else if(bytes_written != bytes_to_write)
	{
		return 0;
	}

	return 1;
}

int read_serial(char *read_buffer,int bytes_to_read,int *unread_bytes)
{
	int bytes_read;
	int err = NO_ERROR;

	if(!ReadFile(hCom1,read_buffer,bytes_to_read,&bytes_read,NULL))
	{
		err = GetLastError();
		*unread_bytes = -1;
		return 0;
	}
	else if(bytes_read != bytes_to_read)
	{
		*unread_bytes = bytes_to_read - bytes_read;
		return 0;
	}

	*unread_bytes = 0;
	return 1;
}

unsigned int read_2_serial() //reads two bytes from the serial port
{
	unsigned char data_buff[2] = {0,0};
	int bytes_unread = 0;
	int i = 0;
	
	//while(!read_serial(data_buff,INPUT_DATA_SIZE,&bytes_unread) && (i < 20)) i++;  
	while(!read_serial(data_buff,INPUT_DATA_SIZE,&bytes_unread)) i++;	//modified by SW
	
	return (data_buff[0] + (data_buff[1] << 8));
}

unsigned int read_3_serial()
{
	unsigned char data_buff[3] = {0,0,0};
	int bytes_unread = 0;
	int i = 0;
	while(!read_serial(data_buff,3,&bytes_unread) && (i < 20)) i++;
	return (data_buff[0] + (data_buff[1] <<8) + (data_buff[2] << 16));
}


int read_dsp_version(char *version_string)
{
	// Tries to send "v" (version) command to dsp and receive firmware version info
	// If all goes well, stuff version info into version_string and return 1.
	// If there is no reply, return 0.

	int bytes_unread = 0;

#ifdef SERIAL_TEST
	if(write_serial(DSPCMD_VERSION,1))
	{
		strcpy(version_string,"TEST VERSION");
		return 1;
	}
	else return 0;
#else

	if(write_serial(DSPCMD_VERSION,1))
	{
		if(read_serial(version_string,VERSION_SIZE,&bytes_unread))
		{
			return 1;
		}
	}

	return 0;

#endif
}

void dio_init_serial()
{
  int i = 0;

  // Initialize output values
  // Reverse the order for compatibility with 16 bit DAC's
  for(i= 15;i >= 0;i--)         
    dio_out_serial(i,(dac_data[i] & 0xFFF));

}

void dio_out_serial(unsigned int channel,unsigned int data)
{
	char ch_data_buff[3];
	
	if(data > MAX) exit(1);

	ch_data_buff[0] = DSPCMD_OUTPUT;
	ch_data_buff[1] = (char)((channel << 4) + (data >> 8));
	ch_data_buff[2] = (char)(data & 0x0FF);		//mask out upper 4 data bits

	write_serial(ch_data_buff,OUTPUT_SIZE + 1);
}

void dio_in_serial(unsigned int channel,unsigned int *data)
{
	char ch_buff[2];
	unsigned char data_buff[2] = {0,0};
	int bytes_unread = 0;
//	int temp0 = 0;
//	int temp1 = 0;

	if(channel > 3) return;
	
	ch_buff[0] = DSPCMD_INPUT;
	ch_buff[1] = (char)(channel);

	if(write_serial(ch_buff,INPUT_CH_SIZE + 1))
	  read_serial(data_buff,INPUT_DATA_SIZE,&bytes_unread);

//	temp1 = abs((int)data_buff[1]);
//	temp0 = abs((int)data_buff[0]) << 8;
//	*data = temp1 + temp0;	

	//The intuitive conversion doesn't work for data_buff[n] >= 0x80
	// unless data_buff is unsigned char
	*data = data_buff[1] + (data_buff[0] << 8);
}

void set_range_serial(int ch,int range) /* assumes range_ch treats channels sequentially */
{
    dac_data[range_ch] &= ~(1 << ch); /* clear range bit */
    /* now set it if we want +- 5V */
    if(range == 1) dac_data[range_ch] |= 1 << ch;

//    dio_out(range_ch,dac_data[range_ch]);
    dio_out_serial(range_ch,dac_data[range_ch]);    
}

void tip_current_serial(unsigned int data)
{
//  dio_out(i_setpoint_ch,data);
  dio_out_serial(i_setpoint_ch,data);
  dac_data[i_setpoint_ch] = data;
}

void bias_serial(unsigned int data)
{
  if(bit16 == 0)
  {
//    dio_out(sample_bias_ch,data);
    dio_out_serial(sample_bias_ch,data);
    dac_data[sample_bias_ch] = data;
  }
  else highres_bias_serial(data);
}

void highres_bias_serial(unsigned int data)
{ 
	char out_buff[HIGHRESBIAS_SIZE + 1];
	out_buff[0] = DSPCMD_HIGHRESBIAS;
	out_buff[1] = (char)((data & 0xFF00) >> 8); //MS bits
	out_buff[2] = (char)(data & 0x0FF);			//LS bits
	out_buff[3] = (char)(highres_bias_ch);		//ch10
	if(!write_serial(out_buff,HIGHRESBIAS_SIZE + 1))
		MessageBox(GetFocus(),"Comm Error during output to ch10!","Comm Error",MB_OK);
	dac_data[highres_bias_ch] = data & 0xFFF;
	dac_data[extra_ch] = (dac_data[extra_ch] & 0x0F0)+ ((data & 0xF000)/0x10); //preserving ch 11 extra bits
																			   // and updating ch10 extra bits
}

void dio_dither_serial(int ch,int status)
{
    int bit;

    if(status != 0 && status != 1) mprintf("BUG: calling dither with >1");
    switch(ch)
    {
        case 0:
            if (!bit16) bit = DIO_CH_0_DITHER_BIT;
			else bit = DIO_CH_10_DITHER_BIT;
            break;
        case 1:
            if (!bit16) bit = DIO_CH_1_DITHER_BIT;
			else bit = DIO_CH_11_DITHER_BIT;
            break;
        default:
            mprintf("BUG: only channels 0 and 1 can be dithered");
            break;
    }
    dac_data[AD_ch] &= (~(1<<bit));
    dac_data[AD_ch] |= (status<<bit);
//    dio_out(AD_ch,dac_data[AD_ch]);
    dio_out_serial(AD_ch,dac_data[AD_ch]);
}    

void set_gain_serial(unsigned int x_gain,unsigned int y_gain,
					 unsigned int z_gain,unsigned int z2_gain)
{

  dac_data[gain_ch] &= ~gain_bits;  /* Mask out gain bits */
  dac_data[gain_ch] += x_gain + y_gain + z_gain + z2_gain;	/* Set gains */
  
//  dio_out(gain_ch,dac_data[gain_ch]);
  dio_out_serial(gain_ch,dac_data[gain_ch]);
}

void set_x_gain_serial(unsigned int x_gain)
{

  dac_data[gain_ch] &= ~x_gain_bits;  /* Mask out gain bits */
  dac_data[gain_ch] += x_gain;    /* Set gains */
  
  //dio_out(gain_ch,dac_data[gain_ch]);
  dio_out_serial(gain_ch,dac_data[gain_ch]);
}

void set_y_gain_serial(unsigned int y_gain)
{

  dac_data[gain_ch] &= ~y_gain_bits;  /* Mask out gain bits */
  dac_data[gain_ch] += y_gain;    /* Set gains */
  
  //dio_out(gain_ch,dac_data[gain_ch]);
  dio_out_serial(gain_ch,dac_data[gain_ch]);
}

void set_z_gain_serial(unsigned int z_gain)
{

  dac_data[gain_ch] &= ~z_gain_bits;  /* Mask out gain bits */
  dac_data[gain_ch] += z_gain;    /* Set gains */
  
  //dio_out(gain_ch,dac_data[gain_ch]);
  dio_out_serial(gain_ch,dac_data[gain_ch]);
}

void set_z2_gain_serial(unsigned int z2_gain)
{

  dac_data[gain_ch] &= ~z2_gain_bits;  /* Mask out gain bits */
  dac_data[gain_ch] += z2_gain;    /* Set gains */
  
  //dio_out(gain_ch,dac_data[gain_ch]);
  dio_out_serial(gain_ch,dac_data[gain_ch]);
}

void retract_serial(int retract_on)
{
  int i;

  if(retract_on) {
    dac_data[retract_ch] |= retract_bit;	/* Set retract bit */
//    dio_out(retract_ch,dac_data[retract_ch]);
    dio_out_serial(retract_ch,dac_data[retract_ch]);

//    dio_start_clock(RETRACT_TIME*1000000);
//    dio_wait_clock();
	for(i = 0;i < 5 * RETRACT_TIME;i++)	// RETRACT_TIME is in secs.
		serial_soft_delay(MAX_SERIAL_DELAY);    // call soft delay 5x w/0xFFFFFF = about 1 sec
  }
  else {
    dac_data[retract_ch] &= ~retract_bit;	/* Zero retract bit */
//    dio_out(retract_ch,dac_data[retract_ch]);
    dio_out_serial(retract_ch,dac_data[retract_ch]);

//    dio_start_clock(UNRETRACT_TIME*1000000);
//    dio_wait_clock();
	for(i = 0;i < 5 * UNRETRACT_TIME;i++)	// UNRETRACT_TIME is in secs
		serial_soft_delay(MAX_SERIAL_DELAY);		// call soft delay 5x w/0xFFFFFF cycles = about 1 sec
  }
}

void dio_feedback_serial(int on)
{
  if(!on) {
    dac_data[feedback_ch] &= ~feedback_bit;     /* Zero feedback bit */
    //dio_out(feedback_ch,dac_data[feedback_ch]);
    dio_out_serial(feedback_ch,dac_data[feedback_ch]);
  }
  else {
    dac_data[feedback_ch] |= feedback_bit;      /* Set feedback bit */
    //dio_out(feedback_ch,dac_data[feedback_ch]);
    dio_out_serial(feedback_ch,dac_data[feedback_ch]);
  }
  feedback = on;
}

void mode_serial(int m)
{

  if((m != fine_mode) && (m != coarse_mode) && (m != rotate_mode) && (m != translate_mode))
  {
	mprintf("Unacceptable Mode!");
	return;
  }

  if((m == fine_mode) || (m == coarse_mode))
  {
	dac_data[zo_ch] = ZERO;
	//dio_out(zo_ch,dac_data[zo_ch]);
	dio_out_serial(zo_ch,dac_data[zo_ch]);
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
  //dio_out(mode_ch,dac_data[mode_ch]);
  dio_out_serial(mode_ch,dac_data[mode_ch]);
}

void move_to_serial(unsigned int ch,unsigned int datai,unsigned int dataf)
{
  unsigned int actual_final = datai;

  if(datai != dataf)
    actual_final = ramp_serial(ch,datai,dataf,1000,0); // without crash protect,
													   // actual_final will always = dataf
  else dio_out_serial(ch,datai); // just to be sure
  dac_data[ch] = actual_final;
}

unsigned int move_to_protect_serial(unsigned int ch,unsigned int datai,unsigned int dataf,
    int time/* in micro seconds*/,int crash_protection,float limit_percent)
{
  unsigned int actual_final = datai;

  if(dataf > MAX)
  {
    sprintf(string,"Tried to move to %d on channel %d\nI'll just move to %d",
			dataf,ch,MAX);
    MessageBox(GetFocus(),string,"Alert!",MB_OKCANCEL);
    dataf = MAX;
  }
  if(datai > MAX)
  {
    sprintf(string,"Tried to move from %d on channel %d\nI'll just move from %d",
            datai,ch,MAX);
    MessageBox(GetFocus(),string,"Alert!",MB_OKCANCEL);
    datai = MAX;
  }

  if(datai != dataf)
  {
	scan_setup(crash_protection,limit_percent,time,time);
	actual_final = ramp_serial(ch,datai,dataf,time,1);
	return(actual_final);
  }
  else
  {
	dio_out_serial(ch,datai); // just to be sure
    return(datai);
  }
}

unsigned int convert_serial_delay(int delay /*in usec*/)
{
	unsigned int i;

	i = (unsigned int)ceil((double)delay * 1000.0 / DSP_CLOCK_PERIOD /*in nsec*/);
	if(i > 0xFFFFFF) i = 0xFFFFFF;
	if(i == 0) i = 1;
	return i;
}

void serial_soft_delay(int delay)
{
// delay should be given in microsecs. The delay is "soft" in that the total
// delay time will be greater than delay. In addition to delay microsecs, the
// total delay time will also include serial communication time and instruction
// execution time. Maximum delay is 0xFFFFFF * 11.6 ns = about 200 millisecs. 
// Serial communications & other overhead adds about 2.5 ms per execution.

	char ch_buff[4];
	unsigned int conv_delay = convert_serial_delay(delay);
	int i = 0;
	int bytes_unread = 0;

	if(conv_delay > 0xFFFFFF)
		conv_delay = 0xFFFFFF;

	ch_buff[0] = DSPCMD_DELAY;
	ch_buff[1] = (char)(conv_delay / 0x10000);
	ch_buff[2] = (char)((conv_delay & 0xFFFF) / 0x100);
	ch_buff[3] = (char)((conv_delay & 0xFF));

	if(write_serial(ch_buff,DELAY_SIZE + 1))
	{
//		while(!read_serial(ch_buff,1,&bytes_unread)) i++;
		if(!read_serial(ch_buff,1,&bytes_unread))
		{
			MessageBox(GetFocus(),"Comm Error during delay!","Comm Error",MB_OK);
		}
		else if(ch_buff[0] != DSPCMD_DELAY)
			MessageBox(GetFocus(),"Comm Error during delay!","Comm Error",MB_OK);
	}
	else MessageBox(GetFocus(),"Comm Error during delay!","Comm Error",MB_OK);
}

void ramp_read_serial(unsigned int in_ch,unsigned int out_ch,
					  unsigned int start,unsigned int final,
					  int delay,int *read_buffer,int do_protection)
{
	// note: do not call with do protection set to true unless dsp has been updated
	// with current z offset and z offset channel
	char out_buff[RAMP_READ_SIZE + 1];
	unsigned char size_buff[2] = {0,0};
	unsigned char data_buff[(MAX + 1) * 3];
	unsigned char read_buff[4] = {0,0,0,0};
	int data_size = 0;
	unsigned int conv_delay = convert_serial_delay(delay);
	int all_read = 0;
	int i = 0,l = 0;
	int j,k;
	int bytes_unread = 0;
	int dummy_bytes = 0;
	int read_result = 0;
	char protect_bit;
	unsigned int new_z_off,new_ch_data;

	if(do_protection) protect_bit = (char)0x80;
	else protect_bit = (char)0x00;

	out_buff[0] = DSPCMD_RAMPREAD;
	out_buff[1] = (char)in_ch;
	out_buff[2] = (char)out_ch + protect_bit;
	out_buff[3] = (char)(start >> 8);
	out_buff[4] = (char)(start & 0xFF);
	out_buff[5] = (char)(final >> 8);
	out_buff[6] = (char)(final & 0xFF);
	out_buff[7] = (char)(conv_delay / 0x10000);
	out_buff[8] = (char)((conv_delay & 0xFFFF) / 0x100);
	out_buff[9] = (char)((conv_delay & 0xFF));

	if(write_serial(out_buff,RAMP_READ_SIZE + 1))
	{
		out_buff[0] = 0x00;
		while(!all_read) {
			while(!read_serial(out_buff,1,&dummy_bytes)) i++;
			if(out_buff[0] == DSPCMD_RAMPREAD)
			{
				// get number of data
				while(!read_serial(size_buff,2,&dummy_bytes) && (i < 20)) i++;
				data_size = size_buff[1] + (size_buff[0] << 8); 
				j = i;

				// read data
				read_result = read_serial(&(data_buff[l]),data_size * 3,&bytes_unread);
				l += (data_size * 3 - bytes_unread);
				while((!read_result || (bytes_unread != 0)) && (i < 20))
				{
					data_size = bytes_unread;
					read_result = read_serial(&(data_buff[l]),data_size,&bytes_unread);
					l += (data_size - bytes_unread);
					i++;
				}
				k = i;

				if(j == 50 || k == 50)
				{
					all_read = 1;
					// Shaowei Aug 29 2013 Tip retract when error
		            retract_serial(TRUE);
					MessageBox(GetFocus(),"Comm Error during ramp and read! Possible underflow.","Comm Error",MB_OK);
				}

			}
			else if(out_buff[0] == DSPCMD_RAMP)
			{
				all_read = 1;
			}
			else
			{
				MessageBox(GetFocus(),"Comm Error during ramp and read!","Comm Error",MB_OK);
				all_read = 1;
			}
		}
		l = l / 3;
		for(i = 0;i < l;i++)
		{
			read_buffer[i] = data_buff[i * 3] +
							(data_buff[i * 3 + 1] << 8) +
							(data_buff[i * 3 + 2] << 16);	// zero
		}
		if(do_protection)
		{
		  read_serial(read_buff,4,&bytes_unread);
		  new_ch_data = read_buff[3] << 8;
		  new_ch_data += read_buff[2];
		  dac_data[in_ch] = new_ch_data;
		  new_z_off = read_buff[1] << 8;
		  new_z_off += read_buff[0];
		  dac_data[z_offset_ch] = new_z_off;
		}
		else
		{
		  if(start < final) dac_data[out_ch] = start;
		  else dac_data[out_ch] = start;
		}
	}
	else MessageBox(GetFocus(),"Comm Error during ramp and read!","Comm Error",MB_OK);
}

//pushkin
unsigned int ramp_bias16_serial(unsigned int start,unsigned int final,int delay)
{
	char out_buff[RAMP_SIZE + 1];
	unsigned int conv_delay = convert_serial_delay(delay);
	unsigned char read_buff[4];
	int i = 0;
	int bytes_unread = 0;
	unsigned int actual_final = start;

	out_buff[0] = DSPCMD_RAMP;
	out_buff[1] = (char)(0x00A);
	out_buff[2] = (char)((start & 0xFF00) >> 8);
	out_buff[3] = (char)(start & 0xFF);
	out_buff[4] = (char)((final & 0xFF00) >> 8);
	out_buff[5] = (char)(final & 0xFF);
	out_buff[6] = (char)(conv_delay / 0x10000);
	out_buff[7] = (char)((conv_delay & 0xFFFF) / 0x100);
	out_buff[8] = (char)((conv_delay & 0xFF));

	if(write_serial(out_buff,RAMP_SIZE + 1))
	{
		read_buff[0] = 0x00;
		while(!read_serial(read_buff,1,&bytes_unread)) i++;
		if(read_buff[0] != DSPCMD_RAMP)
			MessageBox(GetFocus(),"Comm Error during ramp!","Comm Error",MB_OK);
		else {
			actual_final = final;
			dac_data[highres_bias_ch] = final & 0xFFF;
			dac_data[extra_ch] = (dac_data[extra_ch] & 0x0F0)+ ((final & 0xF000)/0x10);
		}
	}
	else MessageBox(GetFocus(),"Comm Error during ramp!","Comm Error",MB_OK);
	return(actual_final);

}

unsigned int ramp_serial(unsigned int ch,unsigned int start,unsigned int final,int delay,
				 int do_protection)
{
	// note: do not call with do protection set to true unless dsp has been updated
	// with current z offset and z offset channel
	char out_buff[RAMP_SIZE + 1];
	unsigned char read_buff[4];
	unsigned int conv_delay = convert_serial_delay(delay);
	int i = 0;
	int bytes_unread = 0;
	char protect_bit;
	unsigned int new_z_off,new_ch_data;
	unsigned int actual_final = start;

	if(do_protection) protect_bit = (char)0x80;
	else protect_bit = (char)0x00;

	out_buff[0] = DSPCMD_RAMP;
	out_buff[1] = (char)ch + protect_bit;
	out_buff[2] = (char)(start >> 8);
	out_buff[3] = (char)(start & 0xFF);
	out_buff[4] = (char)(final >> 8);
	out_buff[5] = (char)(final & 0xFF);
	out_buff[6] = (char)(conv_delay / 0x10000);
	out_buff[7] = (char)((conv_delay & 0xFFFF) / 0x100);
	out_buff[8] = (char)((conv_delay & 0xFF));

	if(write_serial(out_buff,RAMP_SIZE + 1))
	{
		read_buff[0] = 0x00;
		while(!read_serial(read_buff,1,&bytes_unread)) i++;
		if(read_buff[0] != DSPCMD_RAMP)
			MessageBox(GetFocus(),"Comm Error during ramp!","Comm Error",MB_OK);
		else
		{
		  if(do_protection){
			read_serial(read_buff,4,&bytes_unread);
			new_ch_data = read_buff[3] << 8;
			new_ch_data += read_buff[2];
			actual_final = new_ch_data;
			new_z_off = read_buff[1] << 8;
			new_z_off += read_buff[0];
			dac_data[z_offset_ch] = new_z_off;
		  }
		  else
			actual_final = final;
		}
	}
	else MessageBox(GetFocus(),"Comm Error during ramp!","Comm Error",MB_OK);

    return(actual_final);
}

void tip_steps_setup(float z_multiplier,float x_multiplier,unsigned int x_zero_cycles,
					 unsigned int x_parab_size,unsigned int z_parab_size,int delay)
{
	// here "z_" prefix means z outer

	char out_buff[TIPSETUP_SIZE + 1];
	unsigned int conv_delay = convert_serial_delay(delay);

	out_buff[0] = DSPCMD_TIPSETUP;

	// convert to 24 bit format where bit 23 is 2^5 and bit 0 is 2^(-18)
	out_buff[1] = (char)(z_multiplier * (4.0));
	z_multiplier -= (float)(out_buff[1] * (0.25));
	out_buff[2] = (char)(z_multiplier / (0.0009765625)); // (2^(-10))
	z_multiplier -= (float)(out_buff[2] * (0.0009765625));
	out_buff[3] = (char)(z_multiplier / (0.000003814697265625)); // (2^(-18))

	out_buff[4] = (char)((z_parab_size & 0xFF00) >> 8); 
	out_buff[5] = (char)(z_parab_size & 0xFF);

	out_buff[6] = (char)(x_multiplier * (4.0));
	x_multiplier -= (float)(out_buff[6] * (0.25));
	out_buff[7] = (char)(x_multiplier / (0.0009765625));
	x_multiplier -= (float)(out_buff[7] * (0.0009765625));
	out_buff[8] = (char)(x_multiplier / (0.000003814697265625));

	out_buff[9] = (char)((x_parab_size & 0xFF00) >> 8); 
	out_buff[10] = (char)(x_parab_size & 0xFF);
	out_buff[11] = (char)((x_zero_cycles & 0xFF00) >> 8); 
	out_buff[12] = (char)(x_zero_cycles & 0xFF);

	out_buff[13] = (char)((conv_delay & 0xFF0000) >> 16);
	out_buff[14] = (char)((conv_delay & 0xFF00) >> 8);
	out_buff[15] = (char)(conv_delay & 0xFF);

	if(!write_serial(out_buff,TIPSETUP_SIZE + 1))
		MessageBox(GetFocus(),"Comm Error during tip setup!","Comm Error",MB_OK);
}

void tip_steps(unsigned int numsteps,int dir)
{
	// dir == 1, up
	// dir == 0, down

	char out_buff[GIANT_SIZE + 1];

	if(dir == -1) dir = 0; // dir now a switch; used to be a multiplier

	out_buff[0] = DSPCMD_GIANTSTEP;
	out_buff[1] = (char)(((numsteps & 0x7F00) >> 8) + (dir << 7));
	out_buff[2] = (char)(numsteps & 0xFF);
		
	if(!write_serial(out_buff,GIANT_SIZE + 1))
		MessageBox(GetFocus(),"Comm Error during giant steps!","Comm Error",MB_OK);
}

int tip_steps_update(unsigned int expected_steps)
{
	unsigned int max_steps = 3;
	char read_buff[3];
	int bytes_read = 0;
	int bytes_unread = 0;
	int i = 0;

	if(expected_steps < max_steps) max_steps = expected_steps;
	read_buff[0] = 0x00;
	while(!read_serial(read_buff,max_steps,&bytes_unread)) i++;
	if(read_buff[0] != DSPCMD_GIANTSTEP)
	{
		MessageBox(GetFocus(),"Comm Error during tip steps!","Comm Error",MB_OK);
		return(-1);
	}
	else
	{
		bytes_read = max_steps - bytes_unread;
		return(bytes_read);
	}

}

void tip_steps_terminate()
{
	char out_buff[1];

	out_buff[0] = DSPCMD_STOP;
	if(!write_serial(out_buff,1))
		MessageBox(GetFocus(),"Comm Error during tip steps!","Comm Error",MB_OK);

}

void approach_serial(int number_giant,int size_baby,unsigned int tunnel_target,
						 float baby_multiplier)
{
	char out_buff[APPROACH_SIZE + 1];

	out_buff[0] = DSPCMD_TIPAPPROACH;
	out_buff[1] = (char)number_giant;
	out_buff[2] = (char)size_baby;

	out_buff[3] = (char)((tunnel_target & 0xFF00) >> 8); 
	out_buff[4] = (char)(tunnel_target & 0xFF);

	out_buff[5] = (char)(baby_multiplier * (4.0));
	baby_multiplier -= (float)(out_buff[5] * (0.25));
	out_buff[6] = (char)(baby_multiplier / (0.0009765625));
	baby_multiplier -= (float)(out_buff[6] * (0.0009765625));
	out_buff[7] = (char)(baby_multiplier / (0.000003814697265625));

	if(!write_serial(out_buff,APPROACH_SIZE + 1))
		MessageBox(GetFocus(),"Comm Error during tip approach!","Comm Error",MB_OK);
}

unsigned int tip_approach_update(int *tunneled)
{
	// return value is false trigger value, if any exists, when tunneled is false
	// return value is final zouter vale (in bits) when tunneled is true

	unsigned int bytes_per_pass = 3;
	char read_buff[3];
	int bytes_read = 0;
	int bytes_unread = 0;
	int i = 0;
	unsigned int return_val = 0;
	
	*tunneled = 0;
	read_buff[0] = 0x00;
	while(!read_serial(read_buff,bytes_per_pass,&bytes_unread)) i++;
	if(read_buff[0] != DSPCMD_TIPAPPROACH)
	{
		if(read_buff[0] == DSPCMD_STOP)
		{
			*tunneled = 1;
			return_val = read_buff[1] + (read_buff[2] << 8);
			return return_val;
		}
		else
		{
			MessageBox(GetFocus(),"Comm Error during tip approach!","Comm Error",MB_OK);
			return 0;
		}
	}
	else
	{
		if(read_buff[1] != 0 || read_buff[2] != 0)
			return_val = read_buff[1] + (read_buff[2] << 8);
		return return_val;
	}
}

void start_bias_square(unsigned int high_volts,unsigned int low_volts,int delay)
{
	// confusing: high_volts,low_volts actually in bits
	char out_buff[SQUARE_SIZE + 1];
	unsigned int conv_delay = convert_serial_delay(delay);

	out_buff[0] = DSPCMD_SQUAREWAVE;
	out_buff[1] = (char)(conv_delay / 0x10000);
	out_buff[2] = (char)((conv_delay & 0xFFFF) / 0x100);
	out_buff[3] = (char)((conv_delay & 0xFF));
	out_buff[4] = (char)(high_volts >> 8);
	out_buff[5] = (char)(high_volts & 0xFF);
	out_buff[6] = (char)(low_volts >> 8);
	out_buff[7] = (char)(low_volts & 0xFF);

	if(!write_serial(out_buff,SQUARE_SIZE + 1))
		MessageBox(GetFocus(),"Comm Error during square wave!","Comm Error",MB_OK);

	dac_data[sample_bias_ch] = high_volts; // bias will always end on high volts
}

void stop_bias_square()
{
	char out_buff;

	out_buff = DSPCMD_SQUAREWAVE;
	if(!write_serial(&out_buff,1))
		MessageBox(GetFocus(),"Comm Error during square wave!","Comm Error",MB_OK);
}

void auto_serial(int dir)
{
	char out_buff[AUTO_SIZE + 1];
	unsigned char read_buff[2];
	int i = 0;
	int bytes_unread;
	unsigned int new_z_off;

	switch(dir)
	{
	  case AUTOPLUS:
	    out_buff[0] = DSPCMD_AUTOPLUS;
		break;
	  case AUTOMINUS:
	    out_buff[0] = DSPCMD_AUTOMINUS;
	 	break;
	}

	out_buff[1] = (char)((dac_data[z_offset_ch] & 0xF00) >> 8);
	out_buff[2] = (char)(dac_data[z_offset_ch] & 0x0FF);
	out_buff[3] = (char)z_offset_ch;


	if(write_serial(out_buff,AUTO_SIZE + 1))
	{
	  while(!read_serial(read_buff,2,&bytes_unread)) i++;
	  new_z_off = read_buff[1] << 8;
	  new_z_off += read_buff[0];
	  dac_data[z_offset_ch] = new_z_off;
	}
	else
	{
	  MessageBox(GetFocus(),"Comm Error during auto minus/plus!","Comm Error",MB_OK);
	}
}

static int steps_to_words(datadef *the_data)
{
  int k;
  unsigned char count = 0;

  for(k = 0;k < the_data->total_steps;k++)
  {
    if(the_data->sequence[k].type == READ_TYPE || the_data->sequence[k].type == WAIT_TYPE)
	{
	  count+=2;
	}
	else
	{
      count++;
	}
  }

  return(count);
}

void readseq_setup(datadef *the_data)
{
  int k;
  int buff_index = 0;
  float z_multiplier;
  char out_buff[RSSETUP_SIZE + 3];
  unsigned int word;
  unsigned int conv_delay;
  int num_words = steps_to_words(the_data);
  int average_times;
  int wait_divisor = 1;
  
  out_buff[buff_index++] = DSPCMD_RSSETUP;
  out_buff[buff_index++] = (char)(num_words); /*number of words >= to the number to steps*/
  out_buff[buff_index++] = (char)(the_data->total_steps); //number of steps in the read sequence

  for(k = 0;k < the_data->total_steps;k++)
  {
    word = (the_data->sequence[k].type << 20);
	switch(the_data->sequence[k].type)
	{
	  case WAIT_TYPE:
        conv_delay = convert_serial_delay(the_data->sequence[k].wait);
		while(conv_delay >= 0xFFFFFF && wait_divisor <= 0x0FFFFF)
		{
		  wait_divisor++;
		  conv_delay = convert_serial_delay(the_data->sequence[k].wait / wait_divisor);
		}
        word += wait_divisor;
	    break;
	  case DITHER0_TYPE:
	  case DITHER1_TYPE:
	  case FEEDBACK_TYPE:
        word += the_data->sequence[k].state;
		break;
	  case STEP_OUTPUT_TYPE:
        word += (the_data->sequence[k].out_ch << 16);
		word += (the_data->sequence[k].step_bias);
		break;
	  case READ_TYPE:
        word += (the_data->sequence[k].in_ch << 18);
		average_times = (min(the_data->sequence[k].num_reads,0x3FFFF));
		word += average_times;
	    // convert to 24 bit format where bit 23 is 2^5 and bit 0 is 2^(-18)
	    z_multiplier = (float)(1.0 / average_times);
    	break;
	  case SERIAL_OUT_TYPE:
        word += the_data->sequence[k].serial_out_char;
		break;
	}
    out_buff[buff_index++] = (char)(word / 0x10000);
	out_buff[buff_index++] = (char)((word & 0xFFFF) / 0x100);
	out_buff[buff_index++] = (char)(word & 0xFF);

    if(the_data->sequence[k].type == READ_TYPE)
	{
 	// convert to 24 bit format where bit 23 is 2^5 and bit 0 is 2^(-18)
      out_buff[buff_index] = (char)(z_multiplier * (4.0));
	  z_multiplier -= (float)(out_buff[buff_index] * (0.25));
	  buff_index++;
	  out_buff[buff_index] = (char)(z_multiplier / (0.0009765625)); // (2^(-10))
	  z_multiplier -= (float)(out_buff[buff_index] * (0.0009765625));
	  buff_index++;
	  out_buff[buff_index++] = (char)(z_multiplier / (0.000003814697265625)); // (2^(-18))
	}

    if(the_data->sequence[k].type == WAIT_TYPE)
	{
      out_buff[buff_index++] = (char)(conv_delay / 0x10000);
	  out_buff[buff_index++] = (char)((conv_delay & 0xFFFF) / 0x100);
	  out_buff[buff_index++] = (char)(conv_delay & 0xFF);
	}
  }

  if(!write_serial(out_buff,3 * num_words + 3))
		MessageBox(GetFocus(),"Comm Error during scan setup!","Comm Error",MB_OK);
}
				
void scan_setup(int protection_type,float limit_percent,int interdelay,int readdelay)
{
	char out_buff[SCANSETUP_SIZE + 1];
	char protection;
	unsigned int limit_bits = (unsigned int)(IN_MAX * limit_percent / 100);
	unsigned int conv_delay_inter = convert_serial_delay(interdelay);
	//unsigned int conv_delay_read = convert_serial_delay(readdelay); SW
	unsigned int conv_delay_read = convert_serial_delay(readdelay/10);

	out_buff[0] = DSPCMD_SCANSETUP;
	switch(protection_type)
	{
	  case CRASH_STOP:
	    protection = (char)0x01;
		break;
	  case CRASH_DO_NOTHING:
	    protection = (char)0x00;
		break;
	  case CRASH_AUTO_Z:
	    protection = (char)0x02;
		break;
	  case CRASH_MINMAX_Z:
	    protection = (char)0x03;
		break;
	  default:
	    protection = (char)0x00;
		break;
	}
    out_buff[1] = protection;
	out_buff[2] = (char)((limit_bits & 0xFF00) >> 8);
	out_buff[3] = (char)(limit_bits & 0x0FF);
	out_buff[4] = (char)((dac_data[z_offset_ch] & 0xF00) >> 8);
	out_buff[5] = (char)(dac_data[z_offset_ch] & 0x0FF);
	out_buff[6] = (char)z_offset_ch;
	out_buff[7] = (char)((scan_size & 0xF00) >> 8);
	out_buff[8] = (char)(scan_size & 0x0FF);
	out_buff[9] = (char)((scan_step & 0xF00) >> 8);
	out_buff[10] = (char)(scan_step & 0x0FF);
	out_buff[11] = (char)(conv_delay_inter / 0x10000);
	out_buff[12] = (char)((conv_delay_inter & 0xFFFF) / 0x100);
	out_buff[13] = (char)((conv_delay_inter & 0xFF));

	out_buff[14] = (char)(conv_delay_read / 0x10000);
	out_buff[15] = (char)((conv_delay_read & 0xFFFF) / 0x100);
	out_buff[16] = (char)((conv_delay_read & 0xFF));

	if(!write_serial(out_buff,SCANSETUP_SIZE + 1))
		MessageBox(GetFocus(),"Comm Error during scan setup!","Comm Error",MB_OK);
}

int scan_line(unsigned int out_ch,int do_read,int dir,
			  int do_newline,float *input_buffer,unsigned int *last_out)
{
  char out_buff[SCANLINE_SIZE + 1];
  unsigned char size_buff[2] = {0,0};
  unsigned char data_buff[(MAX + 1) * 3];
  unsigned char read_buff[4] = {0,0,0,0};
  char spliced = 0;
  int all_read = 0;
  int dummy_bytes = 0;
  int data_size = 0;
  int read_result = 0;
  int bytes_unread = 0;
  int return_val = SCANLINE_NOERR;
  int i = 0,l = 0;
  int j,k;
  unsigned int new_z_off;
  unsigned int new_ch_data;
  
  if(out_ch > 0xB) return(SCANLINE_ERROR);

  out_buff[0] = DSPCMD_SCANLINE;
  spliced += (char)(out_ch + do_read + dir + do_newline);
  out_buff[1] = spliced;

  if(write_serial(out_buff,SCANLINE_SIZE + 1))
  {
	out_buff[0] = 0x00;
	while(!all_read) {
	  while(!read_serial(out_buff,1,&dummy_bytes)) i++;
	  if(out_buff[0] == DSPCMD_RAMPREAD)
	  {
		// get number of data
		while(!read_serial(size_buff,2,&dummy_bytes) && (i < 20)) i++;
		data_size = size_buff[1] + (size_buff[0] << 8); 
		j = i;

		// read data
		read_result = read_serial(&(data_buff[l]),data_size * 3,&bytes_unread);
		l += (data_size * 3 - bytes_unread);
		while((!read_result || (bytes_unread != 0)) && (i < 20))
		{
		  data_size = bytes_unread;
		  read_result = read_serial(&(data_buff[l]),data_size,&bytes_unread);
		  l += (data_size - bytes_unread);
		  i++;
		}
		k = i;

		if(j == 50 || k == 50)
		{
		  all_read = 1;
		  return_val = SCANLINE_COMMERR;
		  // Shaowei Aug 29 2013 Tip retract when error
		  retract_serial(TRUE);
		  MessageBox(GetFocus(),"Comm Error during ramp and read! Possible underflow.","Comm Error",MB_OK);
		}

	  }
	  else if(out_buff[0] == DSPCMD_SCANLINE)
	  {
	    all_read = 1;
	  }
	  else if(out_buff[0] == DSPCMD_STOP)
	  {
		all_read = 1;
		return_val = SCANLINE_CRASH;
	  }
	  else
	  {
	    MessageBox(GetFocus(),"Comm Error during ramp and read!","Comm Error",MB_OK);
		all_read = 1;
		return_val = 0;
	  }
	}
	l = l / 3;
	for(i = 0;i < l;i++)
	{
	  input_buffer[i] = (float)((data_buff[i * 3] +
					   (data_buff[i * 3 + 1] << 8) +
					   (data_buff[i * 3 + 2] << 16)) / 16.0);	// zero
	}
	if(SD->crash_protection != CRASH_DO_NOTHING)
	{
      read_serial(read_buff,4,&bytes_unread);
	  new_ch_data = read_buff[3] << 8;
	  new_ch_data += read_buff[2];
      *last_out = new_ch_data;
      new_z_off = read_buff[1] << 8;
	  new_z_off += read_buff[0];
	  dac_data[z_offset_ch] = new_z_off;
	  scan_z = new_z_off;
	}
  }
  else
  {
	MessageBox(GetFocus(),"Comm Error during scan line!","Comm Error",MB_OK);
	return_val = SCANLINE_COMMERR;
  }

  return(return_val);

}

void track_setup(int track_avg,int inter_step_delay,int step_delay,
				 float plane_a,float plane_b)
{
	unsigned int sample_every = convert_serial_delay(inter_step_delay);
	unsigned int move_delay = convert_serial_delay(step_delay);
	char out_buff[TRACKSETUP_SIZE + 1];
	out_buff[0] = DSPCMD_TRACKSETUP;

	if(track_avg>256) track_avg=256;
	out_buff[1]=(char)(track_avg & 0x00FF);

	if(sample_every > 0xFFFFFF)
		sample_every = 0xFFFFFF;

	out_buff[2] = (char)(sample_every / 0x10000);
	out_buff[3] = (char)((sample_every & 0xFFFF) / 0x100);
	out_buff[4] = (char)((sample_every & 0xFF));

	
	if(move_delay > 0xFFFFFF)
		move_delay = 0xFFFFFF;

	out_buff[5] = (char)(move_delay / 0x10000);
	out_buff[6] = (char)((move_delay & 0xFFFF) / 0x100);
	out_buff[7] = (char)((move_delay & 0xFF));

	out_buff[8] = (char)(plane_a * (4.0));
	plane_a -= (float)(out_buff[8] * (0.25));
	out_buff[9] = (char)(plane_a / (0.0009765625));
	plane_a -= (float)(out_buff[9] * (0.0009765625));
	out_buff[10]= (char)(plane_a / (0.000003814697265625));

	out_buff[11] = (char)(plane_b * (4.0));
	plane_b -= (float)(out_buff[11] * (0.25));
	out_buff[12] = (char)(plane_b / (0.0009765625));
	plane_b -= (float)(out_buff[12] * (0.0009765625));
	out_buff[13]= (char)(plane_b / (0.000003814697265625));

	if(!write_serial(out_buff,TRACKSETUP_SIZE + 1))
		MessageBox(GetFocus(),"Comm Error during track setup!","Comm Error",MB_OK);
}

void track_start(int x_start,int y_start, int tracking_mode)
{
	char out_buff[TRACK_SIZE + 1];
	unsigned char read_buff[2];
	int i = 0;
	int bytes_unread;
	unsigned int new_x,new_y;

	out_buff[0] = DSPCMD_TRACK;
	out_buff[1] = (char)((x_start & 0xFF00) >> 8);
	out_buff[2] = (char)(x_start & 0x0FF);
	if (tracking_mode == TRACK_MIN) 
		out_buff[1]=(char)(out_buff[1] | 0x80);

	out_buff[3] = (char)((y_start & 0xFF00) >> 8);
	out_buff[4] = (char)(y_start & 0x0FF);

	
	if(write_serial(out_buff,TRACK_SIZE + 1))
	{
	  while(!read_serial(read_buff,4,&bytes_unread)) i++;
	  new_x = read_buff[1] << 8;
	  new_x += read_buff[0];
	  dac_data[x_ch] = new_x;
	  new_y = read_buff[3] << 8;
	  new_y += read_buff[2];
	  dac_data[y_ch] = new_y;
	}
	else
	{
	  MessageBox(GetFocus(),"Comm Error during tracking!","Comm Error",MB_OK);
	}

}

void track_again()
{
	char out_buff;
	unsigned char read_buff[2];
	int i = 0;
	int bytes_unread;
	unsigned int new_x,new_y;

	out_buff = DSPCMD_TRACK_CONT;
	
	if(write_serial(&out_buff,1))
	{
	  while(!read_serial(read_buff,4,&bytes_unread)) i++;
	  /*{
		  i++;
		  if(i>10)
		  {
			i = bytes_unread;
			break;
		  }
	  } */
	  new_x = read_buff[1] << 8;
	  new_x += read_buff[0];
	  dac_data[x_ch] = new_x;
	  new_y = read_buff[3] << 8;
	  new_y += read_buff[2];
	  dac_data[y_ch] = new_y;
	}
	else
	{
	  MessageBox(GetFocus(),"Comm Error during tracking!","Comm Error",MB_OK);
	}
}

void track_terminate()
{
	char out_buff;

	out_buff = DSPCMD_STOP;
	if(!write_serial(&out_buff,1))
		MessageBox(GetFocus(),"Comm Error during tracking!","Comm Error",MB_OK);
}

void spec_setup(unsigned int lower,unsigned int higher,int step_size,
				int move_delay,int read_delay)
{
  char out_buff[SPECSETUP_SIZE + 1];
  unsigned int serial_move_delay = convert_serial_delay(move_delay);
  //unsigned int serial_read_delay = convert_serial_delay(read_delay); SW
  unsigned int serial_read_delay = convert_serial_delay(read_delay/10);

  out_buff[0] = DSPCMD_SPECSETUP;
  out_buff[1] = (char)((lower & 0xFF00) >> 8);
  out_buff[2] = (char)(lower & 0x0FF);
  out_buff[3] = (char)((higher & 0xFF00) >> 8);
  out_buff[4] = (char)(higher & 0x0FF);

  if(step_size > 255) step_size = 255;
  out_buff[5] = (char) step_size;

  out_buff[6] = (char)(serial_move_delay / 0x10000);
  out_buff[7] = (char)((serial_move_delay & 0xFFFF) / 0x100);
  out_buff[8] = (char)((serial_move_delay & 0xFF));
  out_buff[9] = (char)(serial_read_delay / 0x10000);
  out_buff[10] = (char)((serial_read_delay & 0xFFFF) / 0x100);
  out_buff[11] = (char)((serial_read_delay & 0xFF));

  if(!write_serial(out_buff,SPECSETUP_SIZE + 1))
    MessageBox(GetFocus(),"Comm Error during spec setup!","Comm Error",MB_OK);
}

void spec_start(int do_forward,int do_backward,int spec_ch)
{
  char out_buff[SPEC_SIZE + 1];
  char start_word = (char)spec_ch;

  out_buff[0] = DSPSCMD_SPEC;
  if(do_forward) start_word += (char)0x80;   // set flag bit for read forward
  if(do_backward) start_word += (char)0x40;  // set flag bit for read backward
  if(bit16 && ((spec_ch == 10) || (spec_ch == 11)))
    start_word += (char)0x20;  // set flag bit for hires output
  out_buff[1] = start_word;
  if(!write_serial(out_buff,SPEC_SIZE + 1))
    MessageBox(GetFocus(),"Comm Error during spec start!","Comm Error",MB_OK);

}

char spec_continue()
{
	// return value is false trigger value, if any exists, when tunneled is false
	// return value is final zouter vale (in bits) when tunneled is true

	unsigned int bytes_per_pass = 1;
	char read_buff;
	int bytes_read = 0;
	int bytes_unread = 0;
	int i = 0;
	unsigned int return_val = 0;
	
	while(!read_serial(&read_buff,bytes_per_pass,&bytes_unread)) i++;
	return read_buff;
}

void spec_finish(float *data_buffer,int just_read)
{
  int all_read = 0;
  int dummy_bytes = 0;
  int read_result = 0;
  int bytes_unread = 0;
  int i = 0,l = 0;
  int data_size = 0;
  int j,k;
  unsigned char size_buff[2] = {0,0};
  unsigned char serial_buff[(MAX + 1) * 3];

  while(!all_read) {
    if(!just_read)
	{
	  while(!read_serial(serial_buff,1,&dummy_bytes)) i++;
	}
	else
	{
	  serial_buff[0] = DSPCMD_RAMPREAD;
	}
	if(serial_buff[0] == DSPCMD_RAMPREAD)
	{
		// get number of data
		while(!read_serial(size_buff,2,&dummy_bytes) && (i < 20)) i++;
		data_size = size_buff[1] + (size_buff[0] << 8); 
		j = i;

		// read data                      
		read_result = read_serial(&(serial_buff[l]),data_size * 3,&bytes_unread);
		l += (data_size * 3 - bytes_unread);

		while((!read_result || (bytes_unread != 0)) && (i < 20))
		{
		  data_size = bytes_unread;
		  read_result = read_serial(&(serial_buff[l]),data_size,&bytes_unread);
		  l += (data_size - bytes_unread);
		  i++;
		}
		k = i;
        if(bytes_unread == 0) all_read = 1;

		if(j == 50 || k == 50)
		{
		  all_read = 1;
		  // Shaowei Aug 29 2013 Tip retract when error
		  retract_serial(TRUE);
		  MessageBox(GetFocus(),"Comm Error during ramp and read! Possible underflow.","Comm Error",MB_OK);
		}
	}
	else
	{
	    MessageBox(GetFocus(),"Comm Error during ramp and read!","Comm Error",MB_OK);
		all_read = 1;
	}
  }

  l = l / 3;
  for(i = 0;i < l;i++)
  {
	data_buffer[i] = (float)((serial_buff[i * 3] +
					   (serial_buff[i * 3 + 1] << 8) +
					   (serial_buff[i * 3 + 2] << 16)) / 16.0);	// zero
  }
}

void latman_setup(unsigned int long_start,unsigned int long_end,
				  unsigned int short_start,unsigned int zoff_start,
				  int read_every,float ls_ratio,float z_ratio,
				  int move_delay,int read_delay)
{
  char out_buff[LMSETUP_SIZE + 1];
  unsigned int lm_move_delay = convert_serial_delay(move_delay);
  //unsigned int lm_read_delay = convert_serial_delay(read_delay); SW
  unsigned int lm_read_delay = convert_serial_delay(read_delay/10);

  out_buff[0] = DSPCMD_LMSETUP;
  out_buff[1] = (char)((long_start & 0xFF00) >> 8);
  out_buff[2] = (char)(long_start & 0x0FF);
  out_buff[3] = (char)((long_end & 0xFF00) >> 8);
  out_buff[4] = (char)(long_end & 0x0FF);
  out_buff[5] = (char)((short_start & 0xFF00) >> 8);
  out_buff[6] = (char)(short_start & 0x0FF);
  out_buff[7] = (char)((zoff_start & 0xFF00) >> 8);
  out_buff[8] = (char)(zoff_start & 0x0FF);

  if(read_every > 255) read_every = 255;
  out_buff[9] = (char)read_every;

  // convert ratios to 24 bit format where bit 23 is 2^11 and bit 0 is 2^(-12)
  out_buff[10] = (char)(ls_ratio / (16.0)); //2^4
  ls_ratio -= (float)(out_buff[10] * (16.0));
  out_buff[11] = (char)(ls_ratio / (0.0625)); // (2^(-4))
  ls_ratio -= (float)(out_buff[11] * (0.0625));
  out_buff[12] = (char)(ls_ratio / (0.000244140625)); // (2^(-12))
  out_buff[13] = (char)(z_ratio / (16.0)); //2^4
  z_ratio -= (float)(out_buff[13] * (16.0));
  out_buff[14] = (char)(z_ratio / (0.0625)); // (2^(-4))
  z_ratio -= (float)(out_buff[14] * (0.0625));
  out_buff[15] = (char)(z_ratio / (0.000244140625)); // (2^(-12))

  out_buff[16] = (char)(lm_move_delay / 0x10000);
  out_buff[17] = (char)((lm_move_delay & 0xFFFF) / 0x100);
  out_buff[18] = (char)((lm_move_delay & 0xFF));
  out_buff[19] = (char)(lm_read_delay / 0x10000);
  out_buff[20] = (char)((lm_read_delay & 0xFFFF) / 0x100);
  out_buff[21] = (char)((lm_read_delay & 0xFF));

  if(!write_serial(out_buff,LMSETUP_SIZE + 1))
		MessageBox(GetFocus(),"Comm Error during latman setup!","Comm Error",MB_OK);
}

void latman(int long_ch,int short_ch,int do_read,int long_forward,int short_forward,
			unsigned int *long_final,unsigned int *short_final,int *data_amount,
			float *the_buffer,int do_const_height,int z_forward)
{
  char out_buff[LATMAN_SIZE + 1];
  char flags_word = (char)short_ch;
  int all_read = 0;
  int dummy_bytes = 0;
  int data_size;
  int bytes_unread = 0;
  int read_result = 0;
  int i = 0,l = 0;
  int j,k;
  unsigned char size_buff[2] = {0,0};
  unsigned char data_buff[(MAX + 1) * 3];
  unsigned char final_buff[4] = {0,0,0,0};
  unsigned int new_z_off;

  *data_amount = 0; // amount of data retrieved

  out_buff[0] = DSPCMD_LATMAN;
  if(do_const_height) flags_word += (char)0x80;   // set flag bit for const height mode
  if(z_forward) flags_word += (char)0x40;  // set flag bit for increasing z off
  out_buff[1] = flags_word;

  flags_word = (char)long_ch;
  if(do_read) flags_word += (char)0x80;   // set flag bit for reading
  if(long_forward) flags_word += (char)0x40;  // set flag bit for increasing long ch
  if(short_forward) flags_word += (char)0x20;  // set flag bit for increasing long ch
  out_buff[2] = flags_word;

  if(!write_serial(out_buff,LATMAN_SIZE + 1))
  {
    MessageBox(GetFocus(),"Comm Error during lat man!","Comm Error",MB_OK);
  }
  else
  {
	out_buff[0] = 0x00;
	while(!all_read)
	{
	  while(!read_serial(out_buff,1,&dummy_bytes)) i++;
	  if(out_buff[0] == DSPCMD_RAMPREAD)
	  {
		// get number of data
		while(!read_serial(size_buff,2,&dummy_bytes) && (i < 20)) i++;
		data_size = size_buff[1] + (size_buff[0] << 8);
		*data_amount += data_size;
		j = i;

		// read data
		read_result = read_serial(&(data_buff[l]),data_size * 3,&bytes_unread);
		l += (data_size * 3 - bytes_unread);
		while((!read_result || (bytes_unread != 0)) && (i < 20))
		{
		  data_size = bytes_unread;
		  read_result = read_serial(&(data_buff[l]),data_size,&bytes_unread);
		  l += (data_size - bytes_unread);
		  i++;
		}
		k = i;

		if(j == 50 || k == 50)
		{
		  all_read = 1;
		  // Shaowei Aug 29 2013 Tip retract when error
		  retract_serial(TRUE);
		  MessageBox(GetFocus(),"Comm Error during lat man! Possible underflow.","Comm Error",MB_OK);
		}

	  }
	  else if(out_buff[0] == DSPCMD_LATMAN)
	  {
	    all_read = 1;

        if(do_const_height)
		{
          while(!read_serial(final_buff,2,&dummy_bytes) && (i < 20)) i++;
		  new_z_off = final_buff[1] << 8;
		  new_z_off += final_buff[0];
		  dac_data[z_offset_ch] = new_z_off;
	      scan_z = new_z_off;
		}
		
		while(!read_serial(final_buff,4,&dummy_bytes) && (i < 20)) i++;
	    *long_final = final_buff[1] << 8;
	    *long_final += final_buff[0];

	    *short_final = final_buff[3] << 8;
	    *short_final += final_buff[2];
	  }
	  else
	  {
	    MessageBox(GetFocus(),"Comm Error during lat man! Unknown character.","Comm Error",MB_OK);
		all_read = 1;
	  }

	}
    l = l / 3;
    for(i = 0;i < l;i++)
	{
	  the_buffer[i] = (float)((data_buff[i * 3] +
					   (data_buff[i * 3 + 1] << 8) +
					   (data_buff[i * 3 + 2] << 16)) / 16.0);	// zero
	}
  }
}

void dep_setup(int bias_m, int bias_w, int i_m, int i_w, int z_offset, int wait, int num_pts,
			   int delay, int aver_pts, int ignore_init, int chng_t, int after_t,
			   double chng_ratio, int aver_every, int flags)
{
	char out_buff[DEPSETUP_SIZE + 1];
	unsigned int serial_delay;
	float aver_every_inv = (1.0/aver_every);

	out_buff[0] = DSPCMD_DEPSETUP;

	out_buff[1] = (char)((bias_m & 0xFF00) >> 8);
	out_buff[2] = (char)(bias_m & 0x0FF);		//not used
	
	out_buff[3] = (char)((bias_w & 0xFF00) >> 8);
	out_buff[4] = (char)(bias_w & 0x0FF);

	out_buff[5] = (char)((i_m & 0xFF00) >> 8);
	out_buff[6] = (char)(i_m & 0x0FF);			//not used

	out_buff[7] = (char)((i_w & 0xFF00) >> 8);
	out_buff[8] = (char)(i_w & 0x0FF);

	out_buff[9] = (char)((z_offset & 0xFF00) >> 8);
	out_buff[10] = (char)(z_offset & 0x0FF);

	serial_delay = convert_serial_delay(wait);
	out_buff[11] = (char)(serial_delay / 0x10000);
	out_buff[12] = (char)((serial_delay & 0xFFFF) / 0x100);
	out_buff[13] = (char)((serial_delay & 0xFF));

	out_buff[14] = (char)((num_pts & 0xFF00) >> 8);
	out_buff[15] = (char)(num_pts & 0x0FF);

	serial_delay = convert_serial_delay(delay);
	out_buff[16] = (char)(serial_delay / 0x10000);
	out_buff[17] = (char)((serial_delay & 0xFFFF) / 0x100);
	out_buff[18] = (char)((serial_delay & 0xFF));

	out_buff[19] = (char)((aver_pts & 0xFF00) >> 8);
	out_buff[20] = (char)(aver_pts & 0x0FF);		//not used

	out_buff[21] = (char)((ignore_init & 0xFF00) >> 8);
	out_buff[22] = (char)(ignore_init & 0x0FF);		//not used

	out_buff[23] = (char)((chng_t & 0xFF00) >> 8);
	out_buff[24] = (char)(chng_t & 0x0FF);			//not used
	out_buff[25] = (char)((after_t & 0xFF00) >> 8);
	out_buff[26] = (char)(after_t & 0x0FF);			//not used

	out_buff[27] = (char)((((int)(chng_ratio*32)) & 0xFF00) >> 8);
	out_buff[28] = (char)(((int)(chng_ratio*32)) & 0x0FF);		//not used

	out_buff[29] = (char)((aver_every & 0xFF00) >> 8);
	out_buff[30] = (char)(aver_every & 0x0FF);

	//out_buff[31] = (char)(aver_every_inv * (4.0));
	//aver_every_inv -= (float)(out_buff[31] * (0.25));
	//out_buff[32] = (char)(aver_every_inv / (0.0009765625));
	//aver_every_inv -= (float)(out_buff[32] * (0.0009765625));
	//out_buff[33]= (char)(aver_every_inv / (0.000003814697265625));

	// modified by SW to match SW's modified DSP code.
	out_buff[31] = (char)(aver_every_inv * (64.0));
	aver_every_inv -= (float)(out_buff[31] * (0.015625));
	out_buff[32] = (char)(aver_every_inv / (0.00006103515625));
	aver_every_inv -= (float)(out_buff[32] * (0.00006103515625));
	out_buff[33]= (char)(aver_every_inv / (0.0000002384185791015625));

	out_buff[34] = (char)flags;

	if(!write_serial(out_buff,DEPSETUP_SIZE + 1))
		MessageBox(GetFocus(),"Comm Error during deposition setup!","Comm Error",MB_OK);

}

void dep_start()
{
	char out_buff;

	out_buff = DSPCMD_DEP;
	if(!write_serial(&out_buff,1))
	MessageBox(GetFocus(),"Comm Error during deposition!","Comm Error",MB_OK);
	
}

void hop_setup(int avg, int step_delay, int inter_step_delay, int circ_delay, int radius, 
			   float plane_a, float plane_b)
{
	char out_buff[HOPSETUP_SIZE + 1];
	unsigned int serial_delay;
	int r_sqrt;

	out_buff[0] = DSPCMD_HOPSETUP;

	out_buff[1] = avg;

	serial_delay = convert_serial_delay(step_delay);
	out_buff[2] = (char)(serial_delay / 0x10000);
	out_buff[3] = (char)((serial_delay & 0xFFFF) / 0x100);
	out_buff[4] = (char)((serial_delay & 0xFF));

	serial_delay = convert_serial_delay(inter_step_delay);
	out_buff[5] = (char)(serial_delay / 0x10000);
	out_buff[6] = (char)((serial_delay & 0xFFFF) / 0x100);
	out_buff[7] = (char)((serial_delay & 0xFF));

	serial_delay = convert_serial_delay(circ_delay);
	out_buff[8] = (char)(serial_delay / 0x10000);
	out_buff[9] = (char)((serial_delay & 0xFFFF) / 0x100);
	out_buff[10] = (char)((serial_delay & 0xFF));

	out_buff[11] = (char)((radius & 0xFF00) >> 8);
	out_buff[12] = (char)(radius & 0x0FF);

	r_sqrt = (int)(sqrt(radius));
	out_buff[13] = (char)((r_sqrt & 0xFF00) >> 8);
	out_buff[14] = (char)(r_sqrt & 0x0FF);

	out_buff[15] = (char)(plane_a * (4.0));
	plane_a -= (float)(out_buff[15] * (0.25));
	out_buff[16] = (char)(plane_a / (0.0009765625));
	plane_a -= (float)(out_buff[16] * (0.0009765625));
	out_buff[17]= (char)(plane_a / (0.000003814697265625));

	out_buff[18] = (char)(plane_b * (4.0));
	plane_b -= (float)(out_buff[18] * (0.25));
	out_buff[19] = (char)(plane_b / (0.0009765625));
	plane_b -= (float)(out_buff[19] * (0.0009765625));
	out_buff[20]= (char)(plane_b / (0.000003814697265625));

	if(!write_serial(out_buff,HOPSETUP_SIZE + 1))
		MessageBox(GetFocus(),"Comm Error during hop setup!","Comm Error",MB_OK);
}

void hop_start()	//starts the hop
{
	char out_buff;

	out_buff = DSPCMD_HOP;
	if(!write_serial(&out_buff,1))
		MessageBox(GetFocus(),"Comm Error during hop tracking!","Comm Error",MB_OK);
}

int hop(int x, int y, int *x_slope, int *y_slope)	//sends the starting coordinates to the dsp returns the timer reading
{
	char out_buff[3];
	int timer;
	
	out_buff[0] = (char)((x & 0xFF00) >> 8);
	out_buff[1] = (char)(x & 0x0FF);

	out_buff[2] = (char)((y & 0xFF00) >> 8);
	out_buff[3] = (char)(y & 0x0FF);

	if(!write_serial(out_buff,3))
		MessageBox(GetFocus(),"Comm Error during hop!","Comm Error",MB_OK);

	*x_slope = read_3_serial();
	*y_slope = read_3_serial();
	timer = read_3_serial();

	return timer;
}

void cont()		//sends 'c' to the DSP
{
	char out_buff;

	out_buff = DSPCMD_TRACK_CONT;
	if(!write_serial(&out_buff,1))
		MessageBox(GetFocus(),"Was not able to continue!","Comm Error",MB_OK);

}
