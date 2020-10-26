// looted on 11/01/01

#include <conio.h>

#define hires_bias_ch 10
#define extra_ch 15
#define upper4_mask 0xF00
#define extrabit_mask 0xF000
#define regular_mask 0xFFF
#define MAX16 0xFFFF


void hires_bias(unsigned int);
void ramp_hires_bias(unsigned int,int,int,int);
