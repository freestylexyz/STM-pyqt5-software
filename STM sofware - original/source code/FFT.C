/****************************************************************************/
/*  2D FFT subroutine ------                                                */
/*                                                                          */
/*  INPUT :                                                                 */
/*     rl : pointer to the real part of the 2D data (float)                 */
/*     im : pointer to the imaginary part of the 2D data (float)            */
/*     ln : log2 of the array size for FFT                                  */
/*     dir : 1 for FFT and -1 for inverse FFT                               */
/*                                                                          */
/*  OUTPUT :                                                                */
/*     FFT or the inverse FFT of the input data                             */ 
/*                                                                          */
/*  Note :                                                                  */
/*     1. The DC component is at (0, 0) for the FFT of an input data        */
/*     2. The maximal image size allowed in this version is M=256           */
/*                                                                          */
/****************************************************************************/
// #define DEBUG

#ifdef DEBUG
#include <windows.h>
#endif

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "fft.h"

#define M 512
#define PI 3.14159265

void fft_2d( float *rl, int ln, int dir )

{
float   ar[M], ai[M], *im,temp;
int	i, j, m;

	m = 1 << ln;
//    MessageBox(GetFocus(),"1","test",MB_OKCANCEL);

    
    im=(float *) malloc(sizeof(float)*m*m);

#ifdef DEBUG
    if (im==NULL)     
    {
        MessageBox(GetFocus(),"No free mem!","test",MB_OKCANCEL);
        exit(1);
    }
#endif

    if (dir==1)
    {
        for(i=0;i<m;i++)
            for (j=0;j<m;j++) *(im+j*m+i)=0;
    }
    else
    {
        for(i=0;i<m/2;i++)
        {
            memcpy(im+(m/2+i)*m+m/2,rl+i*m,sizeof(float)*m/2);
            memcpy(im+i*m,rl+(i+m/2)*m+m/2,sizeof(float)*m/2);
            memcpy(im+(i+m/2)*m,rl+i*m+m/2,sizeof(float)*m/2);
            memcpy(im+i*m+m/2,rl+(i+m/2)*m,sizeof(float)*m/2);
        }
        memcpy(rl,im,sizeof(float)*m*m);
/*
        for(i=0;i<m;i++)
            for(j=1;j<i+(i%2);j++)
            {
                *(im+(m-j)*m+i)=-*(rl+((m-j))*m+i);
                *(rl+(m-j)*m+i)=*(rl+j*m+(m-i));
            }
        *(im+(m-1)*m+m-1)=-*(im+m+1);
        for(i=0;i<m;i++)
            for(j=1;j<i+(i%2);j++)
            {
                *(im+j*m+m-i)=*(im+(m-j)*m+i);
            }
        for(i=m/2+1;i<m;i++)
        {
            *(im+i)=*(rl+i);
            *(im+m-i)=-*(rl+i);
            *(im+i*m)=*(rl+i*m);
            *(im+(m-i)*m)=-*(rl+i*m);
            *(rl+i)=*(rl+(m-i));
            *(rl+i*m)=*(rl+(m-i)*m);
        }
        
        *(im)=*(im+m/2)=*(im+(m/2)*m)=*(im+(m/2)*m+m/2)=0;
*/
        for(i=0;i<m;i++)
            for(j=0;j<m;j++)
            {
                *(rl+j*m+i)=(*(im+((m-j)%m)*m+(m-i)%m)+*(im+j*m+i))/2;
            }
        for(i=0;i<m;i++)
            for(j=0;j<m;j++)
            {
                *(im+j*m+i)=*(im+j*m+i)-*(rl+j*m+i);
            }
            
    }
    
//    MessageBox(GetFocus(),"2","test",MB_OKCANCEL);

	for( i=0; i<m; i++ )  {
	   for( j=0; j<m; j++ )  {
	      ar[j] = *(rl + i*m + j);
	      ai[j] = *(im + i*m + j);
	   }

//    MessageBox(GetFocus(),"3","test",MB_OKCANCEL);
       fft_1d( ar, ai, ln, dir );
//    MessageBox(GetFocus(),"4","test",MB_OKCANCEL);

	   for( j=0; j<m; j++ )  {
	      *(rl + i*m + j ) = ar[j];
	      *(im + i*m + j ) = ai[j];
	   }
	}
//    MessageBox(GetFocus(),"5","test",MB_OKCANCEL);

	for( i=0; i<m; i++ )  {
	   for( j=0; j<m; j++ )  {
	      ar[j] = *(rl + j*m + i);
	      ai[j] = *(im + j*m + i);
	   }	

	   fft_1d( ar, ai, ln, dir );

	   for( j=0; j<m; j++ )  {
	      *(rl + j*m + i ) = ar[j];
	      *(im + j*m + i ) = ai[j];
	   }
 	}
//    MessageBox(GetFocus(),"6","test",MB_OKCANCEL);
    if (dir==1)
    {                  
/*
        temp=*(rl+m+m-1);
        for(i=0;i<m;i++)
            for(j=1;j<i+(i%2);j++)
            {
                *(rl+(m-j)*m+i)=-*(im+(m-j)*m+i);
            }
        *(rl+m+m-1)=temp;
        for(i=m/2+1;i<m;i++)
        {
            *(rl+i)=*(im+i);
            *(rl+i*m)=*(im+i*m);
        }
*/
/*
        for(i=0;i<m;i++)
            for(j=0;j<m;j++) *(rl+j*m+i)=*(rl+j*m+i)**(rl+j*m+i)+
                            *(im+j*m+i)**(im+j*m+i);
*/
        for(i=0;i<m;i++)
            for(j=0;j<m;j++) *(rl+j*m+i)+=*(im+j*m+i);
//        *rl=0; /* zero the DC component */
        for(i=0;i<m/2;i++)
        {
            memcpy(im+(m/2+i)*m+m/2,rl+i*m,sizeof(float)*m/2);
            memcpy(im+i*m,rl+(i+m/2)*m+m/2,sizeof(float)*m/2);
            memcpy(im+(i+m/2)*m,rl+i*m+m/2,sizeof(float)*m/2);
            memcpy(im+i*m+m/2,rl+(i+m/2)*m,sizeof(float)*m/2);
        }
        memcpy(rl,im,sizeof(float)*m*m);
/*
        for(i=0;i<m/2;i++)
        {
            memcpy(rl+(m/2+i)*m+m/2,im+i*m,sizeof(float)*m/2);
            memcpy(rl+i*m,im+(i+m/2)*m+m/2,sizeof(float)*m/2);
            memcpy(rl+(i+m/2)*m,im+i*m+m/2,sizeof(float)*m/2);
            memcpy(rl+i*m+m/2,im+(i+m/2)*m,sizeof(float)*m/2);
        }
        memcpy(rl,im,sizeof(float)*m*m);
*/
    }
    free(im);
//    MessageBox(GetFocus(),"7","test",MB_OKCANCEL);
}


void fft_1d( float br[], float bi[], int ln, int dir )

{
float	ur, ui, wr, wi, tr,ti;
int	n, n1, n2, i, j, k, kp, l, le, le1;

	n = 1 << ln;
	n2 = n / 2;
	n1 = n - 1;
	j = 1;

	if( dir == -1 )
	   for( i=0; i<n; i++ )
	      *(bi + i) = -*(bi + i);
	
	for( i=1; i<=n1; i++ )  {
	   if( i < j )  {
	      tr = *(br + j - 1);              ti = *(bi + j - 1);
	      *(br + j - 1) = *(br + i - 1);   *(bi + j - 1) = *(bi + i - 1);
	      *(br + i - 1) = tr;	       *(bi + i - 1) = ti;
	   }
	   k = n2;
	   while( k < j )  {
	      j -= k;
	      k /= 2;
	   }
	   j += k;
	}

	for( l=1; l<=ln; l++ )  {
	   le = 1 << l;
	   le1 = le >> 1;
	   ur = 1.0;	ui = 0.0;
	   wr = cos( PI/le1 );
	   wi = - sin( PI/le1 );

	   for( j=0; j<le1; j++ )  { float  urr, uii;
	      for( k=j; k<n; k+=le )  {
                 kp = k + le1;
	         tr = *(br + kp)*ur - *(bi + kp)*ui;
	         ti = *(br + kp)*ui + *(bi + kp)*ur;
	         *(br + kp) = *(br + k) - tr;
	         *(bi + kp) = *(bi + k) - ti;
	   	 *(br + k ) += tr;
	         *(bi + k ) += ti;
	      }
	      urr = ur*wr - ui*wi;
	      uii = ur*wi + ui*wr;
	      ur = urr;  ui = uii;
	   }
	}

	if( dir == 1)
	   for( i=0; i<n; i++ )  {
	      *(br + i) /= n;
              *(bi + i) /= n;
	   }
	else
	   for( i=0; i<n; i++ ) 
	      *(bi + i) = -*(bi + i);
}
