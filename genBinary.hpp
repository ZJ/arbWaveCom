#ifndef GENBINARY_H
#define GENBINARY_H

// 0 to 254 is -1.00 to 1.00 V, 255 is 1.0079
// http://exodus.poly.edu/~kurt/manuals/manuals/Tektronix/TEK%20AWG%202040%20User.pdf
// http://exodus.poly.edu/~kurt/manuals/manuals/Tektronix/TEK%20AWG%202000%20Series%20Programmer%20Version%202.pdf

/* Marker data (prog. man. 2-112)
   Via MARKER:DATA
   format:
   <ASCII #><x (number of digits in yyy)><number of points N = yyy> N*<marker data>
   marker data format:
   0'b000000<Marker 1 state><Marker 2 state>
 */

/*	Standard data: (equip. man. 4A-15 PDF pg 149)
 *	Via CURVe
 *	<ASCII #><x (number of digits in yyy)><number of points N = yyy> N*<binary point>
 *	<point> format:
 *	[0,254] <-> [-1.00 to 1.00], 255 <-> 1.0079
 *	Note this makes value 127 <-> 0
 */
 
/*	See (equip. man. 4B-11)
 *	Max clock rate is 1.024 GHz (= 1024 MHz)
 *	Min clock rate is 1 kHz     (= 0.001 MHz)
 */

/*!	\brief 
 *
 */

#endif