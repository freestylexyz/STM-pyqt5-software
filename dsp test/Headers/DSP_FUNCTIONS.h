/*
 * DSP_FUNCTIONS.h
 *
 *  Created on: Jun 2, 2016
 *      Author: Ho Group
 *
 */

#ifndef DSP_FUNCTIONS_H_
#define DSP_FUNCTIONS_H_

#include <stdlib.h>

#include "F28x_Project.h"     // DSP28x header file
#include "PinInit.h"          // Pin initial header file
#include "Serial.h"           // Serial communication header file
#include "20bitDAC.h"         // 20-bit DAC header file
#include "DAC.h"              // DAC header file
#include "ADC.h"              // ADC header file
#include "Digital.h"          // Digital header file

#define DAC_W_DSP       0xD0
#define DAC_R_DSP       0xD1
#define DAC_RANGE       0xD2
#define BIT20_W_DSP     0x20
#define BIT20_R_DSP     0x21
#define ADC_DSP         0xA0
#define ADC_N_DSP       0xA1
#define ADC_W_DSP       0xA2
#define ADC_R_DSP       0xA3
#define DIGITALO_DSP    0xD3
#define GAIN_DSP        0xD4
#define DIGITALSCAN_DSP 0xD5

#define XIN             0x10
#define XOFFSET         0x11
#define ZOFFSET_FINE    0x12
#define ZOFFSET         0x13
#define ISET_OFFSET     0x14
#define ISET            0x15
#define ZOUTER          0x1A
#define BIAS_OFFSET     0x1C
#define BIAS            0x1D
#define YOFFSET         0x1E
#define YIN             0x1F

#define ZOUT            0xD800
#define PREAMP          0xDC00

#endif /* DSP_FUNCTIONS_H_ */
