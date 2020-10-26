/*

________________________________________________________________

        bandpassf
        $Id: bandpass.c,v 1.37 1997/05/27 13:23:05 svein Exp $
        Copyright 1990, Blab, UiO
        Image processing lab, Department of Informatics
        University of Oslo
        E-mail: blab@ifi.uio.no
________________________________________________________________
  
*/



#include <stdlib.h>
#include <math.h>
//#include "fht\ideal.h"
#include "ideal.h"
#ifdef OLD
#include <xite/includes.h>
#include <xite/biff.h>
#include XITE_STDIO_H
#include XITE_STRING_H
#include <xite/fft.h>
#include <xite/message.h>
#include <xite/readarg.h>
#include <xite/arithmetic.h>
#include <xite/ideal.h>
#include "ideal_L.h"
#endif

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

int scale(float *in,float *out, float factor, float blah, int size)
{
    int i,j;
    
    for(i=0;i<size;i++) for(j=0;j<size;j++)
    {
        out[i*size+j] = in[i*size+j]*factor;
    }
    return(0);
}

int bandpass(float *band, double low_cut_frequency, double high_cut_frequency, int filter_size, window_type win_type, int size)
{
  int stat;
  float *b2;
  int i,j;

  stat = lowpass(band, low_cut_frequency, filter_size, win_type, size);
//  if (stat != 0) return(Error(stat, "%s\n", e_s[stat]));

    b2 = (float *) malloc(sizeof(float)*size*size);

  stat = lowpass(b2, high_cut_frequency, filter_size, win_type,size);
//  if (stat != 0) return(Error(stat, "%s\n", e_s[stat]));

  for(i=0;i<size;i++) for(j=0;j<size;j++)
  {                 
    band[i*size+j] = b2[i*size+j]-band[i*size+j];
  }

    free(b2);  

  return(0);

} /* bandpass() */


#ifdef OLD
int bandpassf (IBAND band, double low_cut_frequency, double high_cut_frequency, int filter_size, window_type win_type)
{
  int stat;

  if (Ipixtyp(band) != Icomplex_typ)
    return(Error(E_PIXTYP, "%d\n", e_s[E_PIXTYP]));

  stat = bandpass(band, low_cut_frequency, high_cut_frequency, filter_size,
		  win_type);
  if (stat != 0) return(Error(stat, "%s\n", e_s[stat]));

  if (fft2d(band, band, 0, 1.0))
    return(Error(E_FFT, "%s\n", e_s[E_FFT]));

  return(0);
}
#endif

/*C*

________________________________________________________________

        bandstopf
        $Id: bandstop.c,v 1.39 1997/05/27 13:23:07 svein Exp $
        Copyright 1991, Blab, UiO
        Image processing lab, Department of Informatics
        University of Oslo
        E-mail: blab@ifi.uio.no
________________________________________________________________
  
*/

int bandstop(float *band, double low_cut_frequency, double high_cut_frequency, int *filter_size, window_type win_type, int size)
{
  int stat;
  long xsize, ysize;

  xsize = size;
  ysize = size;

  if (((*filter_size) % 2 == 0) && (*filter_size) < xsize &&
      (*filter_size) < ysize) {
    (*filter_size)++;
//    Warning(1, "Filter-size must be odd, increased by one.\n");
  } else if ((*filter_size) % 2 == 0) {
    (*filter_size)--;
//    Warning(1, "Filter-size must be odd, decreased by one.\n");
  }

  stat = bandpass(band, low_cut_frequency, high_cut_frequency,
          (*filter_size), win_type, size);
//  if (stat != 0) return(Error(stat, "%s\n", e_s[stat]));

  /* Change sign */

  stat = scale(band, band, -1.0, 0.0,size);
//  if (stat) return(Error(E_PIXTYP, "%s\n", e_s[E_PIXTYP]));

  /* Change to bandstop filter. */

  stat = 0;
  band[0]++;
  
//  if (stat != 0) return(Error(stat, "%s\n", e_s[E_PIXTYP]));

  return(0);

} /* bandstop() */



#ifdef OLD
int bandstopf(IBAND band, double low_cut_frequency, double high_cut_frequency, int *filter_size, window_type win_type)
{
   int stat;

   if (Ipixtyp(band) != Icomplex_typ)
     return(Error(E_PIXTYP, "%d\n", e_s[E_PIXTYP]));

   stat = bandstop(band, low_cut_frequency, high_cut_frequency,
		   filter_size, win_type);
   if (stat != 0) return(Error(stat, "%s\n", e_s[stat]));

   if (fft2d(band, band, 0, 1.0))
     return(Error(E_FFT, "%s\n", e_s[E_FFT]));

   return(0);
}


/*C*

________________________________________________________________

        butterworth
        $Id: butterworth.c,v 1.8 1997/06/03 13:14:41 svein Exp $
        Copyright 1991, Blab, UiO
        Image processing lab, Department of Informatics
        University of Oslo
        E-mail: blab@ifi.uio.no
________________________________________________________________
  
*/

#ifndef XITE_ABS
# define XITE_ABS(i) ((i) >= 0 ? (i) : (-(i)))
#endif


int butterworthf(IBAND band, double cut_frequency, int filter_order)
{
  int x, y, xsize, ysize, xhalf, yhalf, feven, xeven, yeven, low, filter_size;
  int xhalf2, yhalf2, status = 0;
  double max_rad, *distx, *disty, dist, fn, PPI, sum = 0.0;
  IPIXTYP pt;

  sum         = 0.0;
  xsize       = Ixsize (band);
  ysize       = Iysize (band);
  filter_size = xsize;

  pt      = Ipixtyp(band);
  PPI     = M_PI*M_PI;
  max_rad = (float) (filter_size - 1) / (float) 2.0;
  feven   = 1 - filter_size % 2;
  xeven   = 1 - xsize % 2;
  yeven   = 1 - ysize % 2;
  low     = feven ? 1 : 2;
  xhalf   = (xsize + 1) / 2; /* Odd ysize: Includes center row. */
  yhalf   = (ysize + 1) / 2; /* Odd ysize: Includes center row. */
  xhalf2  = xhalf - feven * (!xeven);
  yhalf2  = yhalf - feven * (!yeven);
  cut_frequency = cut_frequency * max_rad;

  if (xsize != ysize) {
    disty = (double *) malloc(sizeof(double) * (yhalf + xhalf));
    distx = &disty[yhalf];
  } else {
    disty = (double *) malloc(sizeof(double) * yhalf);
    distx = disty;
  }

  for (y = 1; y <= yhalf; y++) 
    disty[y-1] = XITE_ABS(y - 1) + (double) feven / 2.0;

  if (xsize != ysize) {
    for (x = 1; x <= xhalf; x++)
      distx[x-1] = XITE_ABS(x - 1) + (double) feven / 2.0;
  }

  switch ((int) pt) {

  case Ireal_typ: {
    IR_BAND bnd = (IR_BAND) band;

    /* Basic quadrant (upper left). */

    for (y = 1; y <= yhalf; y++) 
      for (x = 1; x <= xhalf; x++) {
	dist  = sqrt ((double) ((distx[x-1] * distx[x-1]) +
				(disty[y-1] * disty[y-1])));

	if (cut_frequency == 0.0) fn = 0.0;
	else if (dist <= max_rad) {
	  fn = 1 / (1 + pow((double) dist / cut_frequency,
			    (double) 2.0 * filter_order));
	} else fn = 0.0;

	bnd[y][x] = (float) fn;
	sum = sum + (float) fn;
      }

    /* Lower left. */

    for (y = low; y <= yhalf2; y++)
      for (x = 1; x <= xhalf; x++) {
	  bnd[ysize + low - y][x] = (float) bnd[y][x];
	  sum = sum + (float) bnd[y][x];
	}

    /* Upper right. */

    for (y = 1; y <= yhalf; y++)
      for (x = low; x <= xhalf2; x++) {
	bnd[y][xsize + low - x] = (float) bnd[y][x];
	sum = sum + (float) bnd[y][x];
      }

    /* Lower right. */

    for (y = low; y <= yhalf2; y++)
      for (x = low; x <= xhalf2; x++) {
	bnd[ysize + low - y][xsize + low - x] = 
	  (float) bnd[ysize + low - y][x];
	sum = sum + (float) bnd[ysize + low - y][x];
      }

    /* Initialize zero-samples in cross-center of band. */

    if ((!feven && yeven) || (feven && !yeven))
      for (x = 1; x <= xsize; x++)
	bnd[yhalf2 + 1][x] = 0.0;
    if ((!feven && xeven) || (feven && !xeven))
      for (y = 1; y <= ysize; y++)
	bnd[y][xhalf2 + 1] = 0.0;

  }
    break;

  case Idouble_typ: {
    ID_BAND bnd = (ID_BAND) band;

    /* Basic quadrant (upper left). */

    for (y = 1; y <= yhalf; y++) 
      for (x = 1; x <= xhalf; x++) {
        dist  = sqrt ((double) ((distx[x-1] * distx[x-1]) +
				(disty[y-1] * disty[y-1])));

	if (cut_frequency == 0.0) fn = 0.0;
	else if (dist <= max_rad) {
	  fn = 1 / (1 + pow((double) dist / cut_frequency,
			    (double) 2.0 * filter_order));
	} else fn = 0.0;

        bnd[y][x] = fn;
        sum = sum + fn;
      }

    /* Lower left. */

    for (y = low; y <= yhalf2; y++)
      for (x = 1; x <= xhalf; x++) {
          bnd[ysize + low - y][x] = bnd[y][x];
          sum = sum + bnd[y][x];
        }

    /* Upper right. */

    for (y = 1; y <= yhalf; y++)
      for (x = low; x <= xhalf2; x++) {
        bnd[y][xsize + low - x] = bnd[y][x];
        sum = sum + bnd[y][x];
      }

    /* Lower right. */

    for (y = low; y <= yhalf2; y++)
      for (x = low; x <= xhalf2; x++) {
        bnd[ysize + low - y][xsize + low - x] = 
          bnd[ysize + low - y][x];
        sum = sum + bnd[ysize + low - y][x];
      }

    /* Initialize zero-samples in cross-center of band. */

    if ((!feven && yeven) || (feven && !yeven))
      for (x = 1; x <= xsize; x++)
        bnd[yhalf2 + 1][x] = 0.0;
    if ((!feven && xeven) || (feven && !xeven))
      for (y = 1; y <= ysize; y++)
        bnd[y][xhalf2 + 1] = 0.0;

  }
    break;

  case Icomplex_typ: {
    IC_BAND bnd = (IC_BAND) band;

    /* Basic quadrant (upper left). */

    for (y = 1; y <= yhalf; y++) 
      for (x = 1; x <= xhalf; x++) {
        dist  = sqrt ((double) ((distx[x-1] * distx[x-1]) +
				(disty[y-1] * disty[y-1])));

	if (cut_frequency == 0.0) fn = 0.0;
	else if (dist <= max_rad) {
	  fn = 1 / (1 + pow((double) dist / cut_frequency,
			    (double) 2.0 * filter_order));
	} else fn = 0.0;

        bnd[y][x].re = (float) fn;
        bnd[y][x].im = (float) 0.0;
        sum = sum + (float) fn;
      }

    /* Lower left. */

    for (y = low; y <= yhalf2; y++)
      for (x = 1; x <= xhalf; x++) {
          bnd[ysize + low - y][x].re = (float) bnd[y][x].re;
          bnd[ysize + low - y][x].im = 0.0;
          sum = sum + (float) bnd[y][x].re;
        }

    /* Upper right. */

    for (y = 1; y <= yhalf; y++)
      for (x = low; x <= xhalf2; x++) {
        bnd[y][xsize + low - x].re = (float) bnd[y][x].re;
        bnd[y][xsize + low - x].im = 0.0;
        sum = sum + (float) bnd[y][x].re;
      }

    /* Lower right. */

    for (y = low; y <= yhalf2; y++)
      for (x = low; x <= xhalf2; x++) {
        bnd[ysize + low - y][xsize + low - x].re = 
          (float) bnd[ysize + low - y][x].re;
        bnd[ysize + low - y][xsize + low - x].im = 0.0;
        sum = sum + (float) bnd[ysize + low - y][x].re;
      }

    /* Initialize zero-samples in cross-center of band. */

    if ((!feven && yeven) || (feven && !yeven))
      for (x = 1; x <= xsize; x++) {
        bnd[yhalf2 + 1][x].re = 0.0;
        bnd[yhalf2 + 1][x].im = 0.0;
      }
    if ((!feven && xeven) || (feven && !xeven))
      for (y = 1; y <= ysize; y++) {
        bnd[y][xhalf2 + 1].re = 0.0;
        bnd[y][xhalf2 + 1].im = 0.0;
      }

  }
    break;

  case Id_complex_typ: {
    IDC_BAND bnd = (IDC_BAND) band;

    /* Basic quadrant (upper left). */

    for (y = 1; y <= yhalf; y++) 
      for (x = 1; x <= xhalf; x++) {
        dist  = sqrt ((double) ((distx[x-1] * distx[x-1]) +
				(disty[y-1] * disty[y-1])));

	if (cut_frequency == 0.0) fn = 0.0;
	else if (dist <= max_rad) {
	  fn = 1 / (1 + pow((double) dist / cut_frequency,
			    (double) 2.0 * filter_order));
	} else fn = 0.0;

        bnd[y][x].re = fn;
        bnd[y][x].im = 0.0;
        sum = sum + fn;
      }

    /* Lower left. */

    for (y = low; y <= yhalf2; y++)
      for (x = 1; x <= xhalf; x++) {
          bnd[ysize + low - y][x].re = bnd[y][x].re;
          bnd[ysize + low - y][x].im = 0.0;
          sum = sum + bnd[y][x].re;
        }

    /* Upper right. */

    for (y = 1; y <= yhalf; y++)
      for (x = low; x <= xhalf2; x++) {
        bnd[y][xsize + low - x].re = bnd[y][x].re;
        bnd[y][xsize + low - x].im = 0.0;
        sum = sum + bnd[y][x].re;
      }

    /* Lower right. */

    for (y = low; y <= yhalf2; y++)
      for (x = low; x <= xhalf2; x++) {
        bnd[ysize + low - y][xsize + low - x].re = 
          bnd[ysize + low - y][x].re;
        bnd[ysize + low - y][xsize + low - x].im = 0.0;
        sum = sum + bnd[ysize + low - y][x].re;
      }

    /* Initialize zero-samples in cross-center of band. */

    if ((!feven && yeven) || (feven && !yeven))
      for (x = 1; x <= xsize; x++) {
        bnd[yhalf2 + 1][x].re = 0.0;
        bnd[yhalf2 + 1][x].im = 0.0;
      }
    if ((!feven && xeven) || (feven && !xeven))
      for (y = 1; y <= ysize; y++) {
        bnd[y][xhalf2 + 1].re = 0.0;
        bnd[y][xhalf2 + 1].im = 0.0;
      }

  }
    break;

  default: status = 1;
    break;
    /* break; */

  } /* switch */

  free((char *) disty); disty = NULL;

  if (status) return(Error(E_PIXTYP, "%s\n", e_s[E_PIXTYP]));
  else return(0);

} /* butterworthf() */


/*C*

________________________________________________________________

        exponential
        $Id: exponential.c,v 1.6 1997/05/27 13:23:12 svein Exp $
        Copyright 1991, Blab, UiO
        Image processing lab, Department of Informatics
        University of Oslo
        E-mail: blab@ifi.uio.no
________________________________________________________________
  
 

*/

int exponentialf(IBAND band, double cut_frequency, int filter_order)
{
  int x, y, xsize, ysize, xhalf, yhalf, feven, xeven, yeven, low, filter_size;
  int xhalf2, yhalf2;
  double max_rad, distx, disty, dist, fn, PPI, sum = 0.0;
  IPIXTYP pt;

  sum         = 0.0;
  xsize       = Ixsize (band);
  ysize       = Iysize (band);
  filter_size = xsize;

  pt      = Ipixtyp(band);
  PPI     = M_PI*M_PI;
  max_rad = (float) (filter_size - 1) / (float) 2.0;
  feven   = 1 - filter_size % 2;
  xeven   = 1 - xsize % 2;
  yeven   = 1 - ysize % 2;
  low     = feven ? 1 : 2;
  xhalf   = (xsize + 1) / 2; /* Odd ysize: Includes center row. */
  yhalf   = (ysize + 1) / 2; /* Odd ysize: Includes center row. */
  xhalf2  = xhalf - feven * (!xeven);
  yhalf2  = yhalf - feven * (!yeven);
  cut_frequency = cut_frequency * max_rad;

  switch ((int) pt) {

  case Ireal_typ: {
    IR_BAND bnd = (IR_BAND) band;

    /* Basic quadrant (upper left). */

    for (y = 1; y <= yhalf; y++) 
      for (x = 1; x <= xhalf; x++) {
	distx = abs(x - 1) + (double) feven / 2;
	disty = abs(y - 1) + (double) feven / 2;
	dist  = sqrt ((double) ((distx * distx) + (disty * disty)));

	if (cut_frequency == 0.0) fn = 0.0;
	else if (dist <= max_rad) {
	  fn = exp(-pow((double) dist / cut_frequency,
			(double) filter_order));
	} else fn = 0.0;

	bnd[y][x] = (float) fn;
	sum = sum + (float) fn;
      }

    /* Lower left. */

    for (y = low; y <= yhalf2; y++)
      for (x = 1; x <= xhalf; x++) {
	  bnd[ysize + low - y][x] = (float) bnd[y][x];
	  sum = sum + (float) bnd[y][x];
	}

    /* Upper right. */

    for (y = 1; y <= yhalf; y++)
      for (x = low; x <= xhalf2; x++) {
	bnd[y][xsize + low - x] = (float) bnd[y][x];
	sum = sum + (float) bnd[y][x];
      }

    /* Lower right. */

    for (y = low; y <= yhalf2; y++)
      for (x = low; x <= xhalf2; x++) {
	bnd[ysize + low - y][xsize + low - x] = 
	  (float) bnd[ysize + low - y][x];
	sum = sum + (float) bnd[ysize + low - y][x];
      }

    /* Initialize zero-samples in cross-center of band. */

    if ((!feven && yeven) || (feven && !yeven))
      for (x = 1; x <= xsize; x++)
	bnd[yhalf2 + 1][x] = 0.0;
    if ((!feven && xeven) || (feven && !xeven))
      for (y = 1; y <= ysize; y++)
	bnd[y][xhalf2 + 1] = 0.0;

  }
    break;

  case Idouble_typ: {
    ID_BAND bnd = (ID_BAND) band;

    /* Basic quadrant (upper left). */

    for (y = 1; y <= yhalf; y++) 
      for (x = 1; x <= xhalf; x++) {
        distx = abs(x - 1) + (double) feven / 2;
        disty = abs(y - 1) + (double) feven / 2;
        dist  = sqrt ((double) ((distx * distx) + (disty * disty)));

	if (cut_frequency == 0.0) fn = 0.0;
	else if (dist <= max_rad) {
	  fn = exp(-pow((double) dist / cut_frequency,
			(double) filter_order));
	} else fn = 0.0;

        bnd[y][x] = fn;
        sum = sum + fn;
      }

    /* Lower left. */

    for (y = low; y <= yhalf2; y++)
      for (x = 1; x <= xhalf; x++) {
          bnd[ysize + low - y][x] = bnd[y][x];
          sum = sum + bnd[y][x];
        }

    /* Upper right. */

    for (y = 1; y <= yhalf; y++)
      for (x = low; x <= xhalf2; x++) {
        bnd[y][xsize + low - x] = bnd[y][x];
        sum = sum + bnd[y][x];
      }

    /* Lower right. */

    for (y = low; y <= yhalf2; y++)
      for (x = low; x <= xhalf2; x++) {
        bnd[ysize + low - y][xsize + low - x] = 
          bnd[ysize + low - y][x];
        sum = sum + bnd[ysize + low - y][x];
      }

    /* Initialize zero-samples in cross-center of band. */

    if ((!feven && yeven) || (feven && !yeven))
      for (x = 1; x <= xsize; x++)
        bnd[yhalf2 + 1][x] = 0.0;
    if ((!feven && xeven) || (feven && !xeven))
      for (y = 1; y <= ysize; y++)
        bnd[y][xhalf2 + 1] = 0.0;

  }
    break;

  case Icomplex_typ: {
    IC_BAND bnd = (IC_BAND) band;

    /* Basic quadrant (upper left). */

    for (y = 1; y <= yhalf; y++) 
      for (x = 1; x <= xhalf; x++) {
        distx = abs(x - 1) + (double) feven / 2;
        disty = abs(y - 1) + (double) feven / 2;
        dist  = sqrt ((double) ((distx * distx) + (disty * disty)));

	if (cut_frequency == 0.0) fn = 0.0;
	else if (dist <= max_rad) {
	  fn = exp(-pow((double) dist / cut_frequency,
			(double) filter_order));
	} else fn = 0.0;

        bnd[y][x].re = (float) fn;
        bnd[y][x].im = (float) 0.0;
        sum = sum + (float) fn;
      }

    /* Lower left. */

    for (y = low; y <= yhalf2; y++)
      for (x = 1; x <= xhalf; x++) {
          bnd[ysize + low - y][x].re = (float) bnd[y][x].re;
          bnd[ysize + low - y][x].im = 0.0;
          sum = sum + (float) bnd[y][x].re;
        }

    /* Upper right. */

    for (y = 1; y <= yhalf; y++)
      for (x = low; x <= xhalf2; x++) {
        bnd[y][xsize + low - x].re = (float) bnd[y][x].re;
        bnd[y][xsize + low - x].im = 0.0;
        sum = sum + (float) bnd[y][x].re;
      }

    /* Lower right. */

    for (y = low; y <= yhalf2; y++)
      for (x = low; x <= xhalf2; x++) {
        bnd[ysize + low - y][xsize + low - x].re = 
          (float) bnd[ysize + low - y][x].re;
        bnd[ysize + low - y][xsize + low - x].im = 0.0;
        sum = sum + (float) bnd[ysize + low - y][x].re;
      }

    /* Initialize zero-samples in cross-center of band. */

    if ((!feven && yeven) || (feven && !yeven))
      for (x = 1; x <= xsize; x++) {
        bnd[yhalf2 + 1][x].re = 0.0;
        bnd[yhalf2 + 1][x].im = 0.0;
      }
    if ((!feven && xeven) || (feven && !xeven))
      for (y = 1; y <= ysize; y++) {
        bnd[y][xhalf2 + 1].re = 0.0;
        bnd[y][xhalf2 + 1].im = 0.0;
      }

  }
    break;

  case Id_complex_typ: {
    IDC_BAND bnd = (IDC_BAND) band;

    /* Basic quadrant (upper left). */

    for (y = 1; y <= yhalf; y++) 
      for (x = 1; x <= xhalf; x++) {
        distx = abs(x - 1) + (double) feven / 2;
        disty = abs(y - 1) + (double) feven / 2;
        dist  = sqrt ((double) ((distx * distx) + (disty * disty)));

	if (cut_frequency == 0.0) fn = 0.0;
	else if (dist <= max_rad) {
	  fn = exp(-pow((double) dist / cut_frequency,
			(double) filter_order));
	} else fn = 0.0;

        bnd[y][x].re = fn;
        bnd[y][x].im = 0.0;
        sum = sum + fn;
      }

    /* Lower left. */

    for (y = low; y <= yhalf2; y++)
      for (x = 1; x <= xhalf; x++) {
          bnd[ysize + low - y][x].re = bnd[y][x].re;
          bnd[ysize + low - y][x].im = 0.0;
          sum = sum + bnd[y][x].re;
        }

    /* Upper right. */

    for (y = 1; y <= yhalf; y++)
      for (x = low; x <= xhalf2; x++) {
        bnd[y][xsize + low - x].re = bnd[y][x].re;
        bnd[y][xsize + low - x].im = 0.0;
        sum = sum + bnd[y][x].re;
      }

    /* Lower right. */

    for (y = low; y <= yhalf2; y++)
      for (x = low; x <= xhalf2; x++) {
        bnd[ysize + low - y][xsize + low - x].re = 
          bnd[ysize + low - y][x].re;
        bnd[ysize + low - y][xsize + low - x].im = 0.0;
        sum = sum + bnd[ysize + low - y][x].re;
      }

    /* Initialize zero-samples in cross-center of band. */

    if ((!feven && yeven) || (feven && !yeven))
      for (x = 1; x <= xsize; x++) {
        bnd[yhalf2 + 1][x].re = 0.0;
        bnd[yhalf2 + 1][x].im = 0.0;
      }
    if ((!feven && xeven) || (feven && !xeven))
      for (y = 1; y <= ysize; y++) {
        bnd[y][xhalf2 + 1].re = 0.0;
        bnd[y][xhalf2 + 1].im = 0.0;
      }

  }
    break;

  default: return(Error(E_PIXTYP, "%s\n", e_s[E_PIXTYP]));
    /* break; */

  } /* switch */

  return(0);

} /* exponentialf() */

/*C*

________________________________________________________________

        highpassf
        $Id: highpass.c,v 1.37 1997/05/27 13:23:09 svein Exp $
        Copyright 1991, Blab, UiO
        Image processing lab, Department of Informatics
        University of Oslo
        E-mail: blab@ifi.uio.no
________________________________________________________________
  
*/
#endif

int highpass(float *band, double cut_frequency, int *filter_size, window_type win_type, int size)
{
  int stat;
  long xsize, ysize;
  double sum;

  xsize = size;
  ysize = size;

  if (((*filter_size) % 2 == 0) && (*filter_size) < xsize &&
      (*filter_size) < ysize) {
    (*filter_size)++;
//    Warning(1, "Filter-size must be odd, increased by one.\n");
  } else if ((*filter_size) % 2 == 0) {
    (*filter_size)--;
//    Warning(1, "Filter-size must be odd, decreased by one.\n");
  }

  stat = lowpassu(band, cut_frequency, (*filter_size), win_type, &sum, size);
//  if (stat != 0) return(Error(stat, "%s\n", e_s[stat]));

  /* Change sign and normalize to get sum of coefficients (DC-value) of
   * lowpass-filter equal to -1.0.
   */

  if (sum != 0.0) {
    stat = scale(band, band, -(1 / sum), 0.0, size);
//    if (stat) return(Error(E_PIXTYP, "%s\n", e_s[E_PIXTYP]));
  }

  /* Transform to highpass filter. */

  stat = 0;
  band[0]++;
  
//  if (stat != 0) return(Error(stat, "%s\n", e_s[E_PIXTYP]));

  return(0);

} /* highpass() */

#ifdef OLD
int highpassf(IBAND band, double cut_frequency, int *filter_size, window_type win_type)
{
   int stat;

   if (Ipixtyp(band) != Icomplex_typ)
     return(Error(E_PIXTYP, "%d\n", e_s[E_PIXTYP]));

   stat = highpass(band, cut_frequency, filter_size, win_type);
   if (stat != 0) return(Error(stat, "%s\n", e_s[stat]));

   if (fft2d(band, band, 0, 1.0))
      return(Error(E_FFT, "%s\n", e_s[E_FFT]));

   return(0);
}


/*C*

________________________________________________________________

        lowpass
        $Id: lowpass.c,v 1.53 1997/09/04 10:00:25 svein Exp $
        Copyright 1991, Blab, UiO
        Image processing lab, Department of Informatics
        University of Oslo
        E-mail: blab@ifi.uio.no
________________________________________________________________
  
*/


/*F:lowpass*

________________________________________________________________

		lowpass, highpass, bandpass, bandstop, lowpassu
________________________________________________________________

Name:		lowpass, highpass, bandpass, bandstop, lowpassu - "Ideal"
                filters truncated with window function in the spatial domain

Syntax:		| #include <xite/ideal.h>
                | 
		| int lowpass (IBAND band, double cut_frequency,
		|    int filter_size, window_type win_type);
		|
		| int highpass (IBAND band, double cut_frequency,
		|    int *filter_size, window_type win_type);
		|
		| int bandpass (IBAND band, double low_cut_frequency,
		|    double high_cut_frequency, int filter_size,
		|    window_type win_type);
		|
		| int bandstop (IBAND band, double low_cut_frequency,
		|    double high_cut_frequency, int *filter_size,
		|    window_type win_type);
		|
		| int lowpassu (IBAND band, double cut_frequency,
		|    int filter_size, window_type win_type, double *sum);
		|
		
Description:    'lowpass' generates a lowpass filter in the spatial domain.
                The filter is made from an ideal lowpass-filter which is
		truncated with a window function in the spatial domain.

                'filter_size' specifies filter diameter. 'cut_frequency' is
		relative to the Nyquist frequency, i.e. a 'cut_frequency'
		of 1.0 means half the sampling frequency. 'win_type'
		specifies window function. Legal values for 'win_type'
		are the constants
		&BARTLETT
		Circularly symmetric Bartlett window
		&HAMMING
		Circularly symmetric Hamming window
		&HANNING
		Circularly symmetric Hanning (von Hann) window
		&RECTANGLE
		Circularly symmetric rectangular (box, boxcar) window function
		&TRIANGLE
		Circularly symmetric triangular window function

		-&The circular symmetry is achieved using Huang''s method.

		'lowpass' is normalized so that the sum of the filter
		coefficients equals 1.0. 'lowpassu' is not normalized. For
		'lowpassu', 'sum' is returned as the sum of the filter
		coefficients.

		For odd 'filter_size', the symmetry-center of the filter is
		located in pixel (1,1) in 'band'. For even 'filter_size',
		the symmetry-center of the filter is located at the upper-left
		corner of pixel (1,1), i.e. the symmetry-center does not
		fall into the center of a pixel.

		For even 'filter_size' and even horizontal band-size, the
		resulting 'filter_size' may become smaller than you''ld
		expect, because of the location of the symmetry-center of the
		filter, as explained above.

		'highpass', 'bandpass' and 'bandstop' behave correspondingly.
		'highpass' is implemented as delta(n1, n2) - 'lowpass', with
		sum of coefficients equal to 0.0. 'bandpass' is implemented as
		the difference between two lowpass-filters. 'bandstop' is
		implemented as delta(n1, n2) - 'bandpass'.

		For 'highpass' and 'bandstop', 'filter_size' should be odd.
		If it is even, a warning is issued and 'filter_size' will be
		made odd. For these functions, 'filter_size' is a reference.

Restrictions:   'band' must be one of the pixeltypes real, double, complex or
                double complex.

		'filter_size' must not exceed horizontal or vertical size of
		'band'.

		For 'highpass' and 'bandstop', 'filter_size' must be odd.
		If it is even, it will be increased by one (or decreased by
		one if this exceeds the size of 'band').

Return value:   | 0 : Ok
		| 1 : Bad pixeltype
                | 2 : Unknown window type
		| 3 : Filtersize larger than bandsize

See also:       bartlett(3), window(3), lowpassf(3), lowpass(1)

Reference:      &Jae S. Lim
                "Two-dimensional Signal And Image Processing"
                Prentice-Hall, 1990

Author:		Trond Muri Sørensen. Major revision by Svein Bøe.

Doc:            Svein Bøe
________________________________________________________________

*/

#endif

int lowpassu(float *band, double cut_frequency, int filter_size, window_type win_type, double *sum, int size)
{
  int x, y, xsize, ysize, xhalf, yhalf, feven, xeven, yeven, low;
  int xhalf2, yhalf2;
  double max_rad, distx, disty, dist, fn, PPI;
  double (*window_func)(double n, int size);

  switch (win_type) {
    case BARTLETT:  window_func = bartlett;
      break;
    case HAMMING:   window_func = hamming;
      break;
    case HANNING:   window_func = hanning;
      break;
    case RECTANGLE: window_func = rectangle;
      break;
    case TRIANGLE:  window_func = triangle;
      break;
//    default: return(Error(E_WINDOW, "%s\n", e_s[E_WINDOW]));
  }

  *sum    = 0.0;
  xsize   = size;
  ysize   = size;

/*
  if (filter_size > xsize || filter_size > ysize)
    return(Error(E_FILTERSIZE, "%s\n", e_s[E_FILTERSIZE]));
*/

  PPI     = M_PI*M_PI;
  max_rad = (float) (filter_size - 1) / (float) 2.0;
  feven   = 1 - filter_size % 2;
  xeven   = 1 - xsize % 2;
  yeven   = 1 - ysize % 2;
  low     = feven ? 1 : 2;
  xhalf   = (xsize + 1) / 2; /* Odd ysize: Includes center row. */
  yhalf   = (ysize + 1) / 2; /* Odd ysize: Includes center row. */
  xhalf2  = xhalf - feven * (!xeven);
  yhalf2  = yhalf - feven * (!yeven);


    /* Basic quadrant (upper left). */

    for (y = 1; y <= yhalf; y++) 
      for (x = 1; x <= xhalf; x++) {
	distx = abs(x - 1) + (double) feven / 2;
	disty = abs(y - 1) + (double) feven / 2;
	dist  = sqrt ((double) ((distx * distx) + (disty * disty)));

	if (dist == 0.0)
	  fn = cut_frequency * cut_frequency * PPI / (4.0 * M_PI);
	else if (dist <= max_rad) {
	  fn = ((cut_frequency / ((double) 2.0 * dist)) *
		j1 ((double) (cut_frequency * M_PI * dist)));
	  fn *= window_func(dist + max_rad, filter_size);
	} else fn = 0.0;

    band[(y-1)*size+x-1] = (float) fn;
	*sum = *sum + (float) fn;
      }

    /* Lower left. */

    for (y = low; y <= yhalf2; y++)
      for (x = 1; x <= xhalf; x++) {
      band[(ysize + low - y-1)*size+x-1] = (float) band[(y-1)*size+x-1];
      *sum = *sum + (float) band[(y-1)*size+x-1];
	}

    /* Upper right. */

    for (y = 1; y <= yhalf; y++)
      for (x = low; x <= xhalf2; x++) {
    band[(y-1)*size+xsize + low - x-1] = (float) band[(y-1)*size+x-1];
    *sum = *sum + (float) band[(y-1)*size+x-1];
      }

    /* Lower right. */

    for (y = low; y <= yhalf2; y++)
      for (x = low; x <= xhalf2; x++) {
    band[(ysize + low - y-1)*size+xsize + low - x-1] = 
      (float) band[(ysize + low - y-1)*size+x-1];
    *sum = *sum + (float) band[(ysize + low - y-1)*size+x-1];
      }

    /* Initialize zero-samples in cross-center of band. */

    if ((!feven && yeven) || (feven && !yeven))
      for (x = 1; x <= xsize; x++)
    band[yhalf2*size+x-1] = 0.0;
    if ((!feven && xeven) || (feven && !xeven))
      for (y = 1; y <= ysize; y++)
    band[(y-1)*size+xhalf2] = 0.0;

  return(0);

} /* lowpassu() */

int lowpass(float *band, double cut_frequency, int filter_size, window_type win_type,int size)
{
  int stat;
  double sum;

  stat = lowpassu(band, cut_frequency, filter_size, win_type, &sum, size);
//  if (stat != 0) return(Error(stat, "%s\n", e_s[stat]));

  /* Normalize to get sum of coefficients (DC-value) equal to 1.0. */

  if (sum != 0.0) {
    int x,y;
    
    for(x=0;x<size;x++) for(y=0;y<size;y++)
    {
        band[y*size+x] /= sum;
    }
  }

  return(0);

} /* lowpass() */

#ifdef OLD
/*F:lowpassf*

________________________________________________________________

lowpassf, highpassf, bandpassf, bandstopf, lowpassIdealf, butterworthf, exponentialf, parseFilterTypeName, fftMakeLp, fftMakeHp, fftMakeBp, fftMakeBs
________________________________________________________________

Name:		lowpassf, highpassf, bandpassf, bandstopf, lowpassIdealf,
                butterworthf, exponentialf, parseFilterTypeName, fftMakeLp,
		fftMakeHp, fftMakeBp, fftMakeBs - make a filter in the
		Fourier domain

Syntax:         | #include <xite/ideal.h>
		|
                | int lowpassf( IBAND band,
                |    double cut_frequency, int filter_size,
		|    window_type win_type );
		|
                | int highassf( IBAND band,
                |    double cut_frequency, int *filter_size,
		|    window_type win_type );
		|
                | int bandpassf( IBAND band,
                |    double low_cut_frequency, double high_cut_frequency,
		|    int filter_size, window_type win_type );
		|
                | int bandstopf( IBAND band,
                |    double low_cut_frequency, double high_cut_frequency,
		|    int *filter_size, window_type win_type );
		|
                | int lowpassIdealf( IBAND band,
                |    double cut_frequency );
		|
		| int butterworthf( IBAND band, double cut_frequency,
		|    int filter_order );
		|
		| int exponentialf( IBAND band, double cut_frequency,
		|    int filter_order );
		|
		| filter_type parseFilterTypeName( char *name );
		|
		| #include <xite/fft.h>
		|
		| int fftMakeLp( IC_BAND band,
                |    double cut_frequency, int filter_size );
		|
		| int fftMakeHp( IC_BAND band,
                |    double cut_frequency, int filter_size );
		|
		| int fftMakeBp( IC_BAND band,
                |    double low_cut_frequency,
                |    double high_cut_frequency, int filter_size );
		|
		| int fftMakeBs( IC_BAND band,
                |    double low_cut_frequency,
                |    double high_cut_frequency, int filter_size );
		|
Description:    'lowpassf' makes a lowpass-filter in the Fourier domain. The
                filter used is an ideal lowpass-filter truncated with a
                window in the spatial domain. 'lowpassf' calls 'lowpass' to
		generate the filter in the spatial domain, then calls 'fft2d'
		to get the Fourier transform. The scaling in the fft is 1.0,
		although the standard/default scaling in 'fft2d' forward
		transform is 1/(xsize*ysize). The choice of scaling will give
		a DC (frequency (0,0)) value of 1.0.

                'filter_size' specifies filter diameter. 'cut_frequency' is
		relative to the Nyquist frequency, i.e. a 'cut_frequency'
		of 1.0 means half the sampling frequency. 'win_type'
		specifies the type of window function. Legal value are
		the constants
		&RECTANGLE
		Circularly symmetric rectangular (box, boxcar) window function
		&BARTLETT
		Circularly symmetric Bartlett window
		&TRIANGLE
		Circularly symmetric triangular window function
		&HAMMING
		Circularly symmetric Hamming window
		&HANNING
		Circularly symmetric Hanning (von Hann) window

		-&The circular symmetry is achieved using Huang''s method.

		'highpassf', 'bandpassf' and 'bandstopf' all call their
		spatial-domain counterparts and 'fft2d' with scaling equal to
		1.0.

		'lowpassIdealf' makes an ideal lowpass-filter in the Fourier
		domain. Unlike 'lowpassf', 'lowpassIdealf' is not made from
		a window-truncated spatial-domain filter. It is the box-shaped
		ideal frequency response which corresponds to a non-realizable
		spatial-domain filter.

		'butterworthf' makes a Butterworth filter in the Fourier
		domain.

		'exponentialf' makes an exponential filter in the Fourier
		domain.

		'parseFilterTypeName' returns a symbolic integer value for
		various forms of filter-type 'name'. Legal forms of 'name' are

		| Full name    Short name  Number
		| lowpass         lp       LOWPASS
		| highpass        hp       HIGHPASS
		| bandpass        bp       BANDPASS
		| bandstop        bs       BANDSTOP
		| lowpass_ideal   ilp      LOWPASS_IDEAL
		| butterworth_lp  blp      BUTTER_LP
		| exponential_lp  elp      EXP_LP

		'fftMakeLp' was a function similar (but not identical) to
		'lowpassf' in previous XITE versions. It is retained for
		compatibility, but now as a macro call to 'lowpassf' with
		'win_type' equal to HAMMING. The old 'fftMakeLp' gave
		results slightly different from the results of 'lowpassf'.
		This is due to differences in the implementation of the window
		function. The main difference is that 'filter_size' will be
		reduced by two in 'lowpass', compared to 'fftMakeLp'.

Restrictions:   Horizontal and vertical size of 'band' must be power of
                two.

		'filter_size' may not be smaller than horizontal or vertical
		size of 'band'.

		For 'highpassf' and 'bandstopf', 'filter_size' should be
		odd. If it is even, it will be made odd and a warning is
		issued. For these two functions, 'filter_size' is a reference.

		'band' must be of pixeltype complex.

Return value:   | 0 - OK
                | 1 - Bad pixel type
		| 2 - Unknown window type
                | 3 - Filter_size larger than bandsize
		| 4 - Error in fft2d

See also:       bartlett(3), window(3), lowpass(3), lowpass(1), fft2d(3),
                phase(3), absValue(3), power(3), real(3), imag(3)

Reference:      &Jae S. Lim
                "Two-dimensional Signal And Image Processing"
                Prentice-Hall, 1990

Author:		Trond Muri Sørensen
________________________________________________________________

*/

int lowpassf (IBAND band, double cut_frequency, int filter_size, window_type win_type)
{
   int stat;

   if (Ipixtyp(band) != Icomplex_typ)
     return(Error(E_PIXTYP, "%d\n", e_s[E_PIXTYP]));

   stat = lowpass(band, cut_frequency, filter_size, win_type);
   if (stat != 0) return(Error(stat, "%s\n", e_s[stat]));

   if (fft2d (band, band, 0, 1.0))
      return(Error(E_FFT, "%s\n", e_s[E_FFT]));

   return(0);

} /* lowpassf() */

filter_type parseFilterTypeName(char *name)
{
  filter_type val;

  if (! strcasecmp(name, "lp"            )) return(LOWPASS);
  if (! strcasecmp(name, "lowpass"       )) return(LOWPASS);
  if (! strcasecmp(name, "hp"            )) return(HIGHPASS);
  if (! strcasecmp(name, "highpass"      )) return(HIGHPASS);
  if (! strcasecmp(name, "bp"            )) return(BANDPASS);
  if (! strcasecmp(name, "bandpass"      )) return(BANDPASS);
  if (! strcasecmp(name, "bs"            )) return(BANDSTOP);
  if (! strcasecmp(name, "bandstop"      )) return(BANDSTOP);
  if (! strcasecmp(name, "ilp"           )) return(LOWPASS_IDEAL);
  if (! strcasecmp(name, "lowpass_ideal" )) return(LOWPASS_IDEAL);
  if (! strcasecmp(name, "blp"           )) return(BUTTER_LP);
  if (! strcasecmp(name, "butterworth_lp")) return(BUTTER_LP);
  if (! strcasecmp(name, "elp"           )) return(EXP_LP);
  if (! strcasecmp(name, "exponential_lp")) return(EXP_LP);

  if ((val = atoi(name)) >= LOWPASS && val <= EXP_LP) return(val);

  return(Error(-1, "Unknown filter type.\n"));
}


/*P:lowpass*

________________________________________________________________

lowpass, highpass, bandpass, bandstop, butterworth, exponential, lowpassIdeal, fftMakeLp, fftMakeHp, fftMakeBp, fftMakeBs
________________________________________________________________

Name:		lowpass, highpass, bandpass, bandstop, butterworth,
                exponential, lowpassIdeal, fftMakeLp, fftMakeHp, fftMakeBp,
		fftMakeBs - make an "ideal", truncated,	lowpass filter in
		the spatial or Fourier domain

Syntax:		| lowpass [<option>...] <outimage>
                | highpass [<option>...] <outimage>
                | bandpass [<option>...] <outimage>
                | bandstop [<option>...] <outimage>
		| butterworth [<option>...] <outimage>
		| exponential [<option>...] <outimage>
		| lowpassIdeal [<option>...] <outimage>

Description:    'lowpass' makes a lowpass-filter in the spatial or Fourier
                domain. The filter used is an ideal lowpass filter
		truncated with a circularly symmetric window function
		in the image domain.

		'highpass', 'bandpass' and 'bandstop' behave correspondingly.

		The circular symmetry is achieved using Huang''s method.

		'butterworth' makes a Butterworth lowpass filter in the
		spatial or Fourier domain.

		'exponential' makes an exponential lowpass filter in the
		spatial or Fourier domain.

		'lowpassIdeal' makes an "ideal" lowpass filter in the spatial
		or Fourier domain. This filter is the box-shaped ideal
		frequency response which corresponds to a non-realizable
		spatial-domain filter. The spatial domain result (using
		inverse fft on the frequency-domain result), is an aliased
		version of the non-realizable spatial-domain filter.

		'fftMakeLp' was a program similar (but not identical) to
		'lowpass' in previous XITE versions. It is retained for
		compatibility, but now as a script which invokes 'lowpass'.
		The old 'fftMakeLp' gave results slightly different from the
		results of 'lowpass'. This is due to differences in the
		implementation of the window function. The main difference is
		that 'filter_size' will be reduced by two in 'lowpass',
		compared to the old 'fftMakeLp'. So, the new script version of
		'fftMakeLp' increases 'filter_size' by two before invoking
		'lowpass' with Hamming window. Likewise for 'fftMakeHp',
		'fftMakeBp' and 'fftMakeBs'.
  
Options:	Not all options are accepted by all the programs.

                &-c cut_frequency
		Only for 'lowpass', 'highpass', 'butterworth', 'exponential'
		and 'lowpassIdeal'.

		Specified relative to the Nyquist frequency, i.e. a
		'cut_frequency' of 1.0 equals half the sampling frequency.
		Default: 0.2.

                &-f filter_size, -f filter_order
		For 'lowpass', 'highpass', 'bandpass' and 'bandstop' this
		option gives the diameter of the filter. Default: 41.
		For 'butterworth' and 'exponential', it is the filter order.
		Default: 5. This option can not be used with 'lowpassIdeal'.

                &-h high_cut_frequency
		Only for 'bandpass' and 'bandstop'.

		Specified relative to the Nyquist frequency, i.e. a
		'high_cut_frequency' of 1.0 equals half the sampling frequency.
		Default: 0.7.

                &-l low_cut_frequency
		Only for 'bandpass' and 'bandstop'.

		Specified relative to the Nyquist frequency, i.e. a
		'low_cut_frequency' of 1.0 equals half the sampling frequency.
		Default: 0.2.

		&-pt pixtyp
		Only when result is in the spatial domain.

		Pixeltype of filter.

		  &&"r" or 8
		  Real
		  &&"c" or 9
		  Complex
		  &&"d" or 10
		  Double
		  &&"dc" or 11
		  Double complex

		-&&Default: 8 (real) for spatial-domain result, 9 (complex)
		for Fourier-domain result.

                &-r
		Only for 'highpass' and 'bandpass' when result is in the
		Fourier domain.

                Restore image mean value (without scaling the rest of the
                filter correspondingly).

                &-s
		Return filter in spatial domain, not in Fourier domain.
		Default: Fourier domain.
		
		&-t image_title
		Image title. Default: Lowpass filter.

		&-w win_type
		This option can only be used with 'lowpass', 'highpass',
		'bandpass' and 'bandstop'.

		  &&"r" or 0
		  Circularly symmetric rectangular (box, boxcar) window
		  function
		  &&"b" or 1
		  Circularly symmetric Bartlett (triangular) window
		  &&"hm" or 2
		  Circularly symmetric Hamming window
		  &&"hn" or 3
		  Circularly symmetric Hanning (von Hann) window
		  &&"t" or 4
		  Circularly symmetric triangular window function

		-&&Default: 2.

		&-x xsize
		Horizontal size of the filter image. Default 512.

		&-y ysize
		Vertical size of the filter image. Default 512.

Restrictions:   For a result in the Fourier domain, 'xsize' and 'ysize' must
                be a power of two and 'pixtyp' must be complex.

Return value:	| 0 - OK
                | 1 - Bad pixel type
		| 2 - Unknown window type
		| 3 - Filtersize larger than bandsize
		| 4 - Error in fft2d
	        | 5 - Malloc failed
                | 6 - Bad number of arguments

See also:       bartlett(3), window(1), lowpass(3), lowpassf(3), fft2d(1),
                fftMagPhase(1)

Reference:      &Jae S. Lim
                "Two-dimensional Signal And Image Processing"
                Prentice-Hall, 1990

Author:		Trond Muri Sørensen and Svein Bøe
________________________________________________________________

*/


/*C*

________________________________________________________________

        lowpassIdeal
        $Id: lowpassIdeal.c,v 1.6 1997/05/27 13:23:14 svein Exp $
        Copyright 1991, Blab, UiO
        Image processing lab, Department of Informatics
        University of Oslo
        E-mail: blab@ifi.uio.no
________________________________________________________________
  
*/

int lowpassIdealf (IBAND band, double cut_frequency)
{
  int x, y, xsize, ysize, xhalf, yhalf, feven, xeven, yeven, low;
  int filter_size, xhalf2, yhalf2;
  double max_rad, distx, disty, dist, sum = 0.0, fn = 0.0;
  IPIXTYP pt;

  xsize       = Ixsize(band);
  ysize       = Iysize(band);
  pt          = Ipixtyp(band);

  filter_size = cut_frequency * xsize;
  feven       = 1 - filter_size % 2;
  xeven       = 1 - xsize % 2;
  yeven       = 1 - ysize % 2;
  low         = feven ? 1 : 2;

  max_rad     = (float) (filter_size - 1) / (float) 2.0;
  xhalf       = (xsize + 1) / 2; /* Odd ysize: Includes center row. */
  yhalf       = (ysize + 1) / 2; /* Odd ysize: Includes center row. */
  xhalf2      = xhalf - feven * (!xeven);
  yhalf2      = yhalf - feven * (!yeven);

  switch((int) pt) {
    
  case Ireal_typ: {
    IR_BAND bnd = (IR_BAND) band;
    
    /* Basic quadrant (upper left). */
    
    for (y = 1; y <= yhalf; y++)
      for (x = 1; x <= xhalf; x++) {
        distx = abs(x - 1) + (double) feven / 2;
        disty = abs(y - 1) + (double) feven / 2;
        dist  = sqrt ((double) ((distx * distx) + (disty * disty)));
	
        if (dist <= max_rad) fn = 1.0;
        else fn = 0.0;
	
        bnd[y][x] = (float) fn;
        sum += bnd[y][x];
      }
    
    /* Lower left. */
    
    for (y = low; y <= yhalf2; y++)
      for (x = 1; x <= xhalf; x++) {
	bnd[ysize + low - y][x] = bnd[y][x];
	sum = sum + bnd[y][x];
      }
    
    /* Upper right. */
    
    for (y = 1; y <= yhalf; y++)
      for (x = low; x <= xhalf2; x++) {
        bnd[y][xsize + low - x] = bnd[y][x];
        sum = sum + bnd[y][x];
      }
    
    /* Lower right. */
    
    for (y = low; y <= yhalf2; y++)
      for (x = low; x <= xhalf2; x++) {
        bnd[ysize + low - y][xsize + low - x] = bnd[ysize + low - y][x];
        sum = sum + bnd[ysize + low - y][x];
      }
    
    /* Initialize zero-samples in cross-center of band. */
    
    if ((!feven && yeven) || (feven && !yeven))
      for (x = 1; x <= xsize; x++) {
        bnd[yhalf2 + 1][x] = 0.0;
      }
    if ((!feven && xeven) || (feven && !xeven))
      for (y = 1; y <= ysize; y++) {
        bnd[y][xhalf2 + 1] = 0.0;
      }
  }
    break;

  case Idouble_typ: {
    ID_BAND bnd = (ID_BAND) band;
    
    /* Basic quadrant (upper left). */
    
    for (y = 1; y <= yhalf; y++)
      for (x = 1; x <= xhalf; x++) {
        distx = abs(x - 1) + (double) feven / 2;
        disty = abs(y - 1) + (double) feven / 2;
        dist  = sqrt ((double) ((distx * distx) + (disty * disty)));
	
        if (dist <= max_rad) fn = 1.0;
        else fn = 0.0;
	
        bnd[y][x] = fn;
        sum += bnd[y][x];
      }
    
    /* Lower left. */
    
    for (y = low; y <= yhalf2; y++)
      for (x = 1; x <= xhalf; x++) {
	bnd[ysize + low - y][x] = bnd[y][x];
	sum = sum + bnd[y][x];
      }
    
    /* Upper right. */
    
    for (y = 1; y <= yhalf; y++)
      for (x = low; x <= xhalf2; x++) {
        bnd[y][xsize + low - x] = bnd[y][x];
        sum = sum + bnd[y][x];
      }
    
    /* Lower right. */
    
    for (y = low; y <= yhalf2; y++)
      for (x = low; x <= xhalf2; x++) {
        bnd[ysize + low - y][xsize + low - x] = bnd[ysize + low - y][x];
        sum = sum + bnd[ysize + low - y][x];
      }
    
    /* Initialize zero-samples in cross-center of band. */
    
    if ((!feven && yeven) || (feven && !yeven))
      for (x = 1; x <= xsize; x++) {
        bnd[yhalf2 + 1][x] = 0.0;
      }
    if ((!feven && xeven) || (feven && !xeven))
      for (y = 1; y <= ysize; y++) {
        bnd[y][xhalf2 + 1] = 0.0;
      }
  }
    break;

  case Icomplex_typ: {
    IC_BAND bnd = (IC_BAND) band;
    
    /* Basic quadrant (upper left). */
    
    for (y = 1; y <= yhalf; y++)
      for (x = 1; x <= xhalf; x++) {
        distx = abs(x - 1) + (double) feven / 2;
        disty = abs(y - 1) + (double) feven / 2;
        dist  = sqrt ((double) ((distx * distx) + (disty * disty)));
	
        if (dist <= max_rad) fn = 1.0;
        else fn = 0.0;
	
        bnd[y][x].re = (float) fn;
        bnd[y][x].im = 0.0;
        sum += bnd[y][x].re;
      }
    
    /* Lower left. */
    
    for (y = low; y <= yhalf2; y++)
      for (x = 1; x <= xhalf; x++) {
	bnd[ysize + low - y][x].re = bnd[y][x].re;
	bnd[ysize + low - y][x].im = 0.0;
	sum = sum + bnd[y][x].re;
      }
    
    /* Upper right. */
    
    for (y = 1; y <= yhalf; y++)
      for (x = low; x <= xhalf2; x++) {
        bnd[y][xsize + low - x].re = bnd[y][x].re;
        bnd[y][xsize + low - x].im = 0.0;
        sum = sum + bnd[y][x].re;
      }
    
    /* Lower right. */
    
    for (y = low; y <= yhalf2; y++)
      for (x = low; x <= xhalf2; x++) {
        bnd[ysize + low - y][xsize + low - x].re = bnd[ysize + low - y][x].re;
        bnd[ysize + low - y][xsize + low - x].im = 0.0;
        sum = sum + bnd[ysize + low - y][x].re;
      }
    
    /* Initialize zero-samples in cross-center of band. */
    
    if ((!feven && yeven) || (feven && !yeven))
      for (x = 1; x <= xsize; x++) {
        bnd[yhalf2 + 1][x].re = 0.0;
        bnd[yhalf2 + 1][x].im = 0.0;
      }
    if ((!feven && xeven) || (feven && !xeven))
      for (y = 1; y <= ysize; y++) {
        bnd[y][xhalf2 + 1].re = 0.0;
        bnd[y][xhalf2 + 1].im = 0.0;
      }
  }
    break;
    
  case Id_complex_typ: {
    IDC_BAND bnd = (IDC_BAND) band;
    
    /* Basic quadrant (upper left). */
    
    for (y = 1; y <= yhalf; y++)
      for (x = 1; x <= xhalf; x++) {
        distx = abs(x - 1) + (double) feven / 2;
        disty = abs(y - 1) + (double) feven / 2;
        dist  = sqrt ((double) ((distx * distx) + (disty * disty)));
	
        if (dist <= max_rad) fn = 1.0;
        else fn = 0.0;
	
        bnd[y][x].re = fn;
        bnd[y][x].im = 0.0;
        sum += bnd[y][x].re;
      }
    
    /* Lower left. */
    
    for (y = low; y <= yhalf2; y++)
      for (x = 1; x <= xhalf; x++) {
	bnd[ysize + low - y][x].re = bnd[y][x].re;
	bnd[ysize + low - y][x].im = 0.0;
	sum = sum + bnd[y][x].re;
      }
    
    /* Upper right. */
    
    for (y = 1; y <= yhalf; y++)
      for (x = low; x <= xhalf2; x++) {
        bnd[y][xsize + low - x].re = bnd[y][x].re;
        bnd[y][xsize + low - x].im = 0.0;
        sum = sum + bnd[y][x].re;
      }
    
    /* Lower right. */
    
    for (y = low; y <= yhalf2; y++)
      for (x = low; x <= xhalf2; x++) {
        bnd[ysize + low - y][xsize + low - x].re = bnd[ysize + low - y][x].re;
        bnd[ysize + low - y][xsize + low - x].im = 0.0;
        sum = sum + bnd[ysize + low - y][x].re;
      }
    
    /* Initialize zero-samples in cross-center of band. */

    if ((!feven && yeven) || (feven && !yeven))
      for (x = 1; x <= xsize; x++) {
        bnd[yhalf2 + 1][x].re = 0.0;
        bnd[yhalf2 + 1][x].im = 0.0;
      }
    if ((!feven && xeven) || (feven && !xeven))
      for (y = 1; y <= ysize; y++) {
        bnd[y][xhalf2 + 1].re = 0.0;
        bnd[y][xhalf2 + 1].im = 0.0;
      }
  }
    break;

  default: return(Error(E_PIXTYP, "%s\n", e_s[E_PIXTYP]));
    /* break; */
  } /* switch */

  return(0);

} /* lowpassIdealf() */


/*C*

________________________________________________________________

        window
        $Id: window.c,v 1.21 1997/05/27 13:23:15 svein Exp $
        Copyright 1991, Blab, UiO
        Image processing lab, Department of Informatics
        University of Oslo
        E-mail: blab@ifi.uio.no
________________________________________________________________
  
*/

/*F:bartlett*

________________________________________________________________

		bartlett, hamming, hanning, rectangle, triangle
________________________________________________________________

Name:		bartlett, hamming, hanning, rectangle, triangle,
                parseWindowName - circularly symmetric window
		functions for filters and parsing of window function name

Syntax:         | #include <xite/ideal.h>
                |
		| double bartlett (double n, int size);
		| double hamming (double n, int size);
		| double hanning (double n, int size);
		| double rectangle (double n, int size);
		| double triangle (double n, int size);
		| window_type parseWindowName(char *name);

Description:    The 'bartlett', 'hamming', 'hanning' (von Hann),
                'rectangle' (box, boxcar) and 'triangle' functions return
		function values	for the various windows, at a certain
		distance 'n'. 'size' is the diameter of the window
		function.

		Window functions are typically used to multiply with a filter
		in the spatial domain.

		These functions should only be called when
		| 0 <= n < size
		Outside this interval, the functions should be considered
		equal to zero.

		'rectangle' is given by
		| w(n)=1.0

		'bartlett' is given by
		|      2n/(size-1),     n<=(size-1)/2
		| w(n)= 
		|      2-2n/(size-1),   n>(size-1)/2

		'triangle' is given by
		|      2(n+1)/(size+1),   n<=(size-1)/2
		| w(n)=
		|      2-2(n+1)/(size+1), n>(size-1)/2
		'triangle' is almost the same as 'bartlett', but 'triangle'
		does not include zeros at the end-points.

		'hamming' is given by
		| w(n)=0.54-0.46*cos(2 Pi n/(size-1))

		'hanning' is given by
		| w(n)=0.5-0.5*cos(2 Pi (n+1)/(size+1))
		'hanning' is sometimes defined slightly different. This
		version does not include zeros at the end-points.

		'parseWindowName' returns a symbolic integer value for various
		forms of window 'name'. Legal forms of 'name' are

		| Full name  Short name  Number
		| -----------------------------
		| rectangle      r         RECTANGLE
		| bartlett       b         BARTLETT
		| triangle       t         TRIANGLE
		| hamming        hm        HAMMING
		| hanning        hn        HANNING
   
Return value:   Function value.

See also:       window(3), lowpass(3), lowpassf(3)

Author:		Trond Muri Sørensen, Svein Bøe
_______________________________________________________________

*/

window_type parseWindowName(char *name)
{
  window_type val;

  if (! strcasecmp(name, "r"        )) return(RECTANGLE);
  if (! strcasecmp(name, "rectangle")) return(RECTANGLE);
  if (! strcasecmp(name, "b"        )) return(BARTLETT);
  if (! strcasecmp(name, "bartlett" )) return(BARTLETT);
  if (! strcasecmp(name, "t"        )) return(TRIANGLE);
  if (! strcasecmp(name, "triangle" )) return(TRIANGLE);
  if (! strcasecmp(name, "hm"       )) return(HAMMING);
  if (! strcasecmp(name, "hamming"  )) return(HAMMING);
  if (! strcasecmp(name, "hn"       )) return(HANNING);
  if (! strcasecmp(name, "hanning"  )) return(HANNING);

  if ((val = atoi(name)) >= RECTANGLE && val <= HANNING) return(val);

  return(Error(-1, "Unknown window name.\n"));
}
#endif

double bartlett(double n, int size)
{
   double val, v;

   v = 2.0 * n / (double) (size - 1);
   val = (n <= (double) (size - 1) / 2) ? v : (-v + 2);

   return (val);
}

double triangle(double n, int size)
{
   double val, v;

   v = 2.0 * (n + 1) / (double) (size + 1);
   val = (n <= (double) (size - 1) / 2) ? v : (-v + 2);

   return (val);
}

double hamming(double n, int size)
{
   double  val, pl;

   pl = (double) 2.0 * M_PI * (double) n;
   val = (0.54 - 0.46 * cos (pl / (double) (size - 1)));

   return (val);
}


double hanning(double n, int size)
{
   double  val, pl;

   pl = (double) 2.0 * M_PI * (double) (n + 1);
   val = (0.5 - 0.5 * cos (pl / (double) (size + 1)));

   return (val);
}

double rectangle(double n, int size)
{
   return (1.0);
}



/*F:window*

________________________________________________________________

		window
________________________________________________________________

Name:		window - Insert a window-function in a BIFF band.

Syntax:		| #include <xite/ideal.h>
		| 
		| int window(IBAND band, int size,
		|    window_type win_type);
		| 
Description:	Produce a circularly symmetric window of diameter 'size'
                in 'band'. Legal values for 'win_type' are
                &BARTLETT
                Circularly symmetric Bartlett (triangular) window
                &HAMMING
                Circularly symmetric Hamming window
                &HANNING
                Circularly symmetric Hanning (von Hann) window
                &RECTANGLE
                Circularly symmetric rectangular (box, boxcar) window function
                &TRIANGLE
                Circularly symmetric triangular window function

		The window is returned with its center in the center of
		'band'. This means that the actual center will be located
		between two pixels in the horizontal direction when the
		horizontal band size is even. When the vertical band size
		is even, the center will be located between two pixels in
		the vertical direction. When both horizontal and vertical
		band size is even, the window center will be located between
		four pixels. With both horizontal and vertical band size
		odd, the window center will be located exactly inside the
		center pixel of the band.

		To be applied as a multiplicative window to an image-domain
		filter, make sure that also the filter-function is centered
		around the band	center, or if the filter-function is centered
		around a band corner, the window must be shifted 
		correspondingly before it can be applied. With center on
		a band corner, the resulting filter may be zero phase (if the
		filter function itself obeys the necessary symmetry
		relations).

Return value:   | 0 : Ok
                | 1 : Unknown window type

See also:	bartlett(3), window(1), lowpass(3), lowpassf(3)

Author:		Svein Bøe, Blab, Ifi, UiO

Id: 		$Id: window.c,v 1.21 1997/05/27 13:23:15 svein Exp $
________________________________________________________________

*/


int window(float *band, int size, window_type win_type)
{
  long xsize, ysize, x, y, xhalf, yhalf;
  double radx, rady, rad, max_rad, xcent, ycent;

   double (*window_func)(double n, int size);

  xsize = size;
  ysize = size;
  xcent = ((double) xsize + 1) / 2;
  ycent = ((double) ysize + 1) / 2;
  xhalf = (long) xcent;
  yhalf = (long) ycent;

  max_rad = (double) (size - 1) / (float) 2.0;

  switch (win_type) {
    case BARTLETT:  window_func = bartlett;
      break;
    case HAMMING:   window_func = hamming;
      break;
    case HANNING:   window_func = hanning;
      break;
    case RECTANGLE: window_func = rectangle;
      break;
    case TRIANGLE:  window_func = triangle;
      break;
    default: return(1);
  }


    /* The top left fourth (the fourth closest to (1,1) in the image). */
    for (y = 1; y <= yhalf; y++)
      for (x = 1; x <= xhalf; x++) {
	radx = fabs(xcent - x);
	rady = fabs(ycent - y);
	rad  = sqrt ((double) ((radx * radx) + (rady * rady)));
	
	if (rad <= max_rad) {
      band[(y-1)*size+x-1] = window_func(max_rad - rad, size);
    } else band[(y-1)*size+x-1] = 0.0;
      }

    /* The bottom left fourth. */
    for (y = 1; y <= yhalf; y++)
      for (x = 1; x <= xhalf; x++) {
    band[(ysize - y)*size+x-1] = band[(y-1)*size+x-1];
      }

    /* The top right and bottom right fourths. */
    for (y = 1; y <= yhalf; y++)
      for (x = 1; x <= xhalf; x++) {
    band[(y-1)*size+xsize - x] = band[(y-1)*size+x-1];
    band[(ysize - y)*size+xsize - x] = band[(ysize - y)*size+x-1];
      }
  return(0);
}


/*P:window*

________________________________________________________________

		window
________________________________________________________________

Name:		window - Make a BIFF image containing a window function

Syntax:		window [<option>...] <outimage>

Description:	Make a BIFF image containing a circularly symmetric window
                function.

		The window is returned with its center in the center of
		the image. To be applied to an image-domain filter, make sure
		that also the filter-function is centered around the image
		center. If the filter-function is centered around an image
		corner, the window must be shifted before it can be applied.

Options:	&-w type
                Specifies the window type. Legal values are
		&&"r" or 0
		Circularly symmetric rectangular window.
		&&"b" or 1
		Circularly symmetric Bartlett window.
		&&"hm" or 2
		Circularly symmetric Hamming window.
		&&"hn" or 3
		Circularly symmetric Hanning window.
		&&"t" or 4
		Circularly symmetric triangular window.
		-&&Default: Hamming window.
		&-x xsize
		Horizontal size of output image. Default: 512.
		&-y ysize
		Vertical size of output image. Default: 512.
		&-s window_size
		Diameter of window function. Default: xsize/2.
		&-t title
		Title of image. Default: "Window".

See also:	window(3), bartlett(3), lowpass(1)

Author:		Svein Bøe, Blab, Ifi, UiO

Id: 		$Id: window.c,v 1.21 1997/05/27 13:23:15 svein Exp $
________________________________________________________________

*/
