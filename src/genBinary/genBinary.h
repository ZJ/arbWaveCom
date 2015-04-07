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
 #define AWG_ZERO_VAL 127
 
/*	See (equip. man. 4B-11)
 *	Max clock rate is 1.024 GHz (= 1024 MHz)
 *	Min clock rate is 1 kHz     (= 0.001 MHz)
 */

 
 
#define	PI		3.141592653589793
#define	TWO_PI	6.283185307179586
#define DEFAULT_FREQ_LIST_SIZE 8
#define GEN_BINARY_EPARSE  -1
#define GEN_BINARY_ERESIZE -2

 typedef struct freqList {
	unsigned int	freqCount;
	unsigned int	actualSize;
	double *		freqList;
	double *		ampList;
	double *		durList;
 } freqList_type;
 typedef freqList_type * freqList_ptr;
 
/*!	\brief 
 *
 */

int allocSubLists(freqList_ptr toSet, unsigned int nFreqs);
int setFreqList(freqList_ptr toSet, const double start_f, const double stop_f);
int setFixedAmp(freqList_ptr toSet, const double amplitude);
int setFixedDur(freqList_ptr toSet, const double duration);
int setRandAmp(freqList_ptr toSet);
unsigned int pointsToHalfCycle(double targetDuration, double pointInterval, double frequency);
unsigned char * genPointList(const freqList_ptr freqList, const unsigned int * pointCounts, const double pointInterval, unsigned long * finalCount);
unsigned char * genWavePts(double freq, double amp, unsigned int numPts, double pointInterval, unsigned char * startPtr);
int writeToFile(const char * rootName, const unsigned char * ptsList, const unsigned long numPtrs);
ssize_t myGetLine(char ** bufferPtr, size_t * bufferSize, FILE * fp);
void freeFreqList(freqList_ptr toFree);
int resizeFreqList(unsigned int newSize, freqList_ptr * toResize);
freqList_ptr readSpecFile(const char * inPath);
int parseLine(char * lineBuf, freqList_ptr destList);
freqList_ptr blankFreqList();
unsigned int * pointCounts(const freqList_ptr freqList, const double pointInterval);
int writeSummaryFile(const char * rootName, const freqList_ptr freqList, const unsigned int * pointCounts, const double pointInterval);

#endif
