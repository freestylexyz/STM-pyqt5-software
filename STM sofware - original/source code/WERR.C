// ----------------------- data.h

typedef struct tagdatadef {
  unsigned int	size;
  float 	*ptr;
} datadef;

// ----------------------------- end of data.h

// ----------------------------- start of werr.c

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
// #include "data.h"
#define SMOOTH_SMALL 0

datadef *gendata[4]={NULL,NULL,NULL,NULL};
float cur_smooth_s[9];
/* note that these arrays are initialized in a different section of code
    (omitted because it was not causing the problem) */

void smooth(HWND hDlg,int num)
{
    float *sr=gendata[num]->ptr;
    float *ds=gendata[3]->ptr;
    int size=gendata[num]->size;
    float *sa;
    int i,j;
    float sum;
    
    if (IsDlgButtonChecked(hDlg,SMOOTH_SMALL))
    {
        /* bounding columns */
        sa=cur_smooth_s;
        sum=(sa[4]+sa[7]+sa[6]+sa[3]+sa[0]+sa[1]);
        for(i=1;i<size-1;i++) /*right col */
        {
//            ds[(size-1)*size+i]=sr[(size-1)*size+i]*sa[4];
//            ds[(size-1)*size+i]+=sr[(size-1)*size+i-1]*sa[7];
//            ds[(size-1)*size+i]+=sr[(size-2)*size+i-1]*sa[6];
//            ds[(size-1)*size+i]+=sr[(size-2)*size+i]*sa[3];
//            ds[(size-1)*size+i]+=sr[(size-2)*size+i+1]*sa[0];
            ds[(size-1)*size+i]+=sr[(size-1)*size)+i+1]*sa[1];
            ds[(size-1)*size+i]/=sum;
        }
    }
}

// -------------------- end of werr.c
/* console output:
C:\STM\NEW>wcc386p -zW werr
WATCOM C 386 Optimizing Compiler  Version 9.01e
Copyright by WATCOM Systems Inc. 1984, 1992.  All rights reserved.
WATCOM is a trademark of WATCOM Systems Inc.
werr.c: 57 lines, included 7006, 0 warnings, 0 errors
werr.c(46): Error! E1119: Internal compiler error 3
Code size: 210

*/
