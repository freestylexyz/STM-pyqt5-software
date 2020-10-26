
/*

________________________________________________________________

        ideal.h
        $Id: ideal.h,v 1.10 1997/03/24 09:32:02 svein Exp $
        Copyright 1990, Blab, UiO
        Image processing lab, Department of Informatics
        University of Oslo
        E-mail: blab@ifi.uio.no
________________________________________________________________
  
  Permission to use, copy, modify and distribute this software and its
  documentation for any purpose and without fee is hereby granted, 
  provided that this copyright notice appear in all copies and that 
  both that copyright notice and this permission notice appear in supporting
  documentation and that the name of B-lab, Department of Informatics or
  University of Oslo not be used in advertising or publicity pertaining 
  to distribution of the software without specific, written prior permission.

  B-LAB DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL B-LAB
  BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
  OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
  CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 

*/

#ifndef _IDEAL_H_
#define _IDEAL_H_

/* Window functions. */

typedef enum {
  RECTANGLE,
  BARTLETT,
  TRIANGLE,
  HAMMING,
  HANNING
} window_type;

typedef enum {
  LOWPASS,
  HIGHPASS,
  BANDPASS,
  BANDSTOP,
  LOWPASS_IDEAL,
  BUTTER_LP,
  EXP_LP
} filter_type;


extern double hamming  ( double n, int size );
extern double hanning  ( double n, int size );
extern double rectangle  ( double n, int size );
extern double bartlett  ( double n, int size );
extern double triangle  ( double n, int size );
extern window_type parseWindowName ( char *name );
extern int window  ( float * band, int size, window_type win_type );
extern int lowpass  ( float * cb, double cut_frequency, int filter_size, window_type win_type , int size);
extern int lowpassu  ( float * cb, double cut_frequency, int filter_size, window_type win_type, double *sum , int size);
extern int highpass  (float * cb, double cut_frequency, int *filter_size, window_type win_type, int size );
extern int bandpass  ( float * cb, double low_cut_frequency, double high_cut_frequency, int filter_size, window_type win_type, int size );
extern int bandpassu  ( float * cb, double low_cut_frequency, double high_cut_frequency, int filter_size, window_type win_type, double *sum, int size );
extern int bandstop  ( float * cb, double low_cut_frequency, double high_cut_frequency, int *filter_size, window_type win_type, int size );


#ifdef OLD
extern int bandstopf  (float * out_band, double low_cut_frequency, double high_cut_frequency, int *filter_size, window_type win_type );
extern int lowpassf  ( float * out_band, double cut_frequency, int filter_size, window_type win_type );
extern int highpassf  ( float * out_band, double cut_frequency, int *filter_size, window_type win_type );
extern int bandpassf  ( float * out_band, double low_cut_frequency, double high_cut_frequency, int filter_size, window_type win_type );
extern int lowpassIdealf  ( float * out_band, double cut_frequency );
extern int butterworthf ( float * band, double cut_frequency, int filter_order );
extern int exponentialf ( float * band, double cut_frequency, int filter_order );
extern filter_type parseFilterTypeName ( char *name );
#endif

#endif /* _IDEAL_H_ */
