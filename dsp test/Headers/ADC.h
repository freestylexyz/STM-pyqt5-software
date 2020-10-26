/*
 * ADC.h
 *
 *  Created on: Sep 8, 2020
 *      Author: yaoji
 */

#ifndef HEADERS_ADC_H_
#define HEADERS_ADC_H_

#include "F28x_Project.h"     // DSP28x Headerfile
#include "PinInit.h"
#include "SPI.h"
#include "Serial.h"

//
// Command Register
//
#define ADC_NOP        0x0000
#define ADC_STDBY      0x8200
#define ADC_PD         0x8300
#define ADC_RST        0x8500
#define ADC_AUTORST    0xA000
#define ADC_CH0        0xC000
#define ADC_CH1        0xC400
#define ADC_CH2        0xC800
#define ADC_CH3        0xCC00
#define ADC_CH4        0xD000
#define ADC_CH5        0xD400
#define ADC_CH6        0xD800
#define ADC_CH7        0xDC00
#define ADC_AUX        0xE000

//
// Program Register
//
#define ADC_AUTOSEQ      (0x01 << 1)
#define ADC_CHPD         (0x02 << 1)
#define ADC_FS           (0x03 << 1)
#define ADC_RANGE0       (0x05 << 1)
#define ADC_RANGE1       (0x06 << 1)
#define ADC_RANGE2       (0x07 << 1)
#define ADC_RANGE3       (0x08 << 1)
#define ADC_RANGE4       (0x09 << 1)
#define ADC_RANGE5       (0x0A << 1)
#define ADC_RANGE6       (0x0B << 1)
#define ADC_RANGE7       (0x0C << 1)

//
// Input Range selection
//
#define ADC_B20         0x00
#define ADC_B10         0x01
#define ADC_B5          0x02
#define ADC_U10         0x05
#define ADC_U5          0x06

//
// Default value for program register
//
#define ADC_AUTOSEQ_DEFAULT     0xFF
#define ADC_CHPD_DEFAULT        0x00

//
// Global variables declaration
//
extern char adcrange[8];

//
// Function declaration
//
void adc_RST_H();
void adc_PD_H();
Uint16 adc_command(Uint16);
Uint16 adc_CNV(Uint16);
Uint16 adc_CNV_N(Uint16, Uint16);
void adc_DSP();
void adc_N_DSP();
void adc_W_DSP();
void adc_R_DSP();
void adcInit();


#endif /* HEADERS_ADC_H_ */

//
// End of file
//
