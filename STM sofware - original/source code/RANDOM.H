/* This random number generator was ftp'd from wuarchive
    /mirrors/msdos/c/random-c.zip */


/*
C This random number generator originally appeared in "Toward a Universal 
C Random Number Generator" by George Marsaglia and Arif Zaman. 
C Florida State University Report: FSU-SCRI-87-50 (1987)
C 
C It was later modified by F. James and published in "A Review of Pseudo-
C random Number Generators" 
C 
C THIS IS THE BEST KNOWN RANDOM NUMBER GENERATOR AVAILABLE.
C       (However, a newly discovered technique can yield 
C         a period of 10^600. But that is still in the development stage.)
C
C It passes ALL of the tests for random number generators and has a period 
C   of 2^144, is completely portable (gives bit identical results on all 
C   machines with at least 24-bit mantissas in the doubleing point 
C   representation). 
C 
C The algorithm is a combination of a Fibonacci sequence (with lags of 97
C   and 33, and operation "subtraction plus one, modulo one") and an 
C   "arithmetic sequence" (using subtraction).
C======================================================================== 
This C language version was written by Jim Butler, and was based on a
FORTRAN program posted by David LaSalle of Florida State University.
*/


#ifndef RANDOM_H
#define RANDOM_H
#undef random

extern void rmarin(int,int);
extern void ranmar(double*,int);
extern double mohrand();

#endif
