
/*
  This header-file is produced automatically (at least in part) by
  makeHeader of BLAB, Ifi, UiO.
*/


/*

________________________________________________________________

        fht.h
        $Id: fht.h,v 1.5 1997/03/24 09:31:52 svein Exp $
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

#ifndef _FHT_H_
#define _FHT_H_

extern int fhtInitTables  ( float *sinTab, float *cosTab, int *perm, int size );
extern void fht1d  ( float *data, float *sinTab, float *cosTab, float *work, int *perm, int size, int power, int forw );
extern int fht2d  ( float *inband, float *outband, int forward , int size);
#ifdef OLD
extern int fhtPhase  ( IR_BAND inband, IR_BAND outband );
extern int fhtPower  ( IR_BAND inband, IR_BAND outband );
extern int ft2ht  ( IC_BAND inband, IR_BAND outband );
extern int ht2ft  ( IR_BAND inband, IC_BAND outband );
#endif

#endif /* _FHT_H_ */
