// looted on: 11/01/01

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <conio.h>
#include <math.h>
#include <time.h>
#include <sys\timeb.h>
#include <dos.h>
#include <string.h>
#include "dio.h"
#include "stm.h"
#include "data.h"
#include "scan.h"
#include "common.h"
#include "hop.h"
#include "highres.h"
#include "clock.h"

extern unsigned int out1;
extern unsigned int dac_data[];

void hires_bias(unsigned int data)
{
#ifdef DAC16
  if ((dac_data[hires_bias_ch] & extrabit_mask)==(data & extrabit_mask)){
      dio_out(hires_bias_ch,data & regular_mask);
  }
  else{
//       dio_out(extra_ch, >>(>>(>>(>>(data & extrabit_mask)))));
       dio_out(extra_ch, (data & extrabit_mask)>>4);//Fix this
       dio_out(hires_bias_ch, data & regular_mask);
  }
  dac_data[hires_bias_ch] = data;
#endif
}

void ramp_hires_bias(unsigned int data,int time,int skip,int bits)
{
#ifdef DAC16
    unsigned int datai=dac_data[hires_bias_ch];
    if (data>MAX16) return;
    time-=DIO_OUT_TIME;
  if (data>ZERO16 && dac_data[hires_bias_ch]<ZERO16)
  {
    if (datai<ZERO16-skip)
    {
        while(datai<ZERO16-skip)
        {
            datai+=bits;
            if (datai>ZERO16-skip) datai=ZERO16-skip;
            //dio_out(sample_bias_ch,datai);
            hires_bias(datai);
            dio_start_clock(time);
            dio_wait_clock();
        }
    }
    //dio_out(sample_bias_ch,ZERO16+skip);
    hires_bias(ZERO16+skip);
    datai=ZERO16+skip;
    dio_start_clock(time);
    dio_wait_clock();
    while(datai<data)
    {
        datai+=bits;
        if (datai>data) datai=data;
        //dio_out(sample_bias_ch,datai);
        hires_bias(datai);
        dio_start_clock(time);
        dio_wait_clock();
    }
  }
  else if (data<ZERO16 && dac_data[sample_bias_ch]>ZERO16)
  {
    if (datai>ZERO16+skip)
    {
        while(datai>ZERO16+skip)
        {
            datai-=bits;
            if (datai<ZERO16+skip) datai=ZERO16+skip;
            //dio_out(sample_bias_ch,datai);
            hires_bias(datai);
            dio_start_clock(time);
            dio_wait_clock();
        }
    }
    //dio_out(sample_bias_ch,ZERO-skip);
    hires_bias(ZERO16-skip);
    datai=ZERO16-skip;
    dio_start_clock(time);
    dio_wait_clock();
    while(datai>data)
    {
        if (datai>bits) datai-=bits;
        else datai=0;
        if (datai<data) datai=data;
        //dio_out(sample_bias_ch,datai);
        hires_bias(datai);
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
        //dio_out(sample_bias_ch,datai);
        hires_bias(datai);
        dio_start_clock(time);
        dio_wait_clock();
    }
    while(datai>data)
    {
        if (datai>bits) datai-=bits;
        else datai=0;
        if (datai<data) datai=data;
        //dio_out(sample_bias_ch,datai);
        hires_bias(datai);
        dio_start_clock(time);
        dio_wait_clock();
    }
  }  
  //dac_data[hires_bias_ch] = data;
#endif
}
