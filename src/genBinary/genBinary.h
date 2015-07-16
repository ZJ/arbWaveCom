/*! @file genBinary.h
 * @brief Handles generating AWG command string from user-supplied options.
 *
 * Device documentation is hosted at [exodus.poly.edu/~kurt/](http://exodus.poly.edu/~kurt/):
 * - [User Manual (pdf)](http://exodus.poly.edu/~kurt/manuals/manuals/Tektronix/TEK%20AWG%202040%20User.pdf)
 * - [Programmer's Manual (pdf)](http://exodus.poly.edu/~kurt/manuals/manuals/Tektronix/TEK%20AWG%202000%20Series%20Programmer%20Version%202.pdf)
 */


#ifndef GENBINARY_H
#define GENBINARY_H

/*! @page AWGInterfaceFormat AWG Data/Communications format
 *  @brief How data is communicated to and from the AWG
 *  @tableofcontents
 *
 * @section DeviceDocs Device Documentation
 * Device documentation is hosted at [exodus.poly.edu/~kurt/](http://exodus.poly.edu/~kurt/):
 * - [User Manual (pdf)](http://exodus.poly.edu/~kurt/manuals/manuals/Tektronix/TEK%20AWG%202040%20User.pdf)
 * - [Programmer's Manual (pdf)](http://exodus.poly.edu/~kurt/manuals/manuals/Tektronix/TEK%20AWG%202000%20Series%20Programmer%20Version%202.pdf)
 *
 * @section FormatASCIINumbers Arbitrary precision numbers as ASCII
 * To send numbers of arbitrary lengths of decimal digits, the AWG uses the format:
 *
 * \#\<Number of decimal digits in the number\>\<The number itself\>
 *
 * All digits are sent as ASCII, not binary.
 * Numbers longer than 9 digits are not representable, nor should they be needed.
 * @subsection FormatASCIIRepExamples Examples
 * Number | AWG Representation
 * ------ | ------------------
 * 1 | #11
 * 31 | #231
 * 193 | #3193
 * 14253697 | #714253697
 *
 * @section FormatMarker Marker Data Format
 *
 * @ref DeviceDocs "See the Programmer's Manual", page 2-112
 *
 * The command invoked to send marker data to the AWG is @c MARKER:DATA
 *
 * It is formatted as @code MARKER:DATA <Number of points in ASCII Number Format><That many bytes of marker data, formatted as below>@endcode
 * @subsection FormatMarkerPoint Individual Marker Point Format
 * Points are represented as binary bytes, with the LSB corresponding to Marker 2 and the next lowest bit Marker 1.
 * For clarity:
 * Marker 1 | Marker 2 | Marker Value
 * -------- | -------- | ------------
 * L | L | 0 (00)
 * L | H | 1 (01)
 * H | L | 2 (10)
 * H | H | 3 (11)
 * 
 * @subsection MarkerExamples Examples
 * Marker Pattern | Command ([] means send binary representation)
 * -------------- | -------
 * 6 points, Marker 1 toggling every point, Marker 2 every other point | MARKER:DATA #16[0][2][1][3][0][2]
 * 10 points, Marker 1 high on only first point only, Marker 2 only on the sixth | MARKER:DATA #210[2][0][0][0][0][1][0][0][0][0]
 *
 * @section FormatCurve Curve (Waveform) Data Format
 * See @ref DeviceDocs "the manuals":
 * - Equipment Manual, page 4A-15 (PDF page 149)
 * - Programmer's Manual, page 2-59 (PDF page 85) and following
 * 
 * The command used to transfer data to the AWG is @c CURVe
 * Here we assume the data width has already been set to 1 (byte).
 * In that case, the command format as
 * @code CURVe <Number of points in ASCII Number Format><That many bytes output data, formatted as below>@endcode
 * @subsection CurvePoints Output Point Format
 * The generator has 8-bits of output resolution, and to have the ability to output 0 it has slightly asymmetric output ranges.
 * They also chose a data format that has output values linearly increasing with binary values.  As a result, you get:
 * - [0, 254] maps onto [-1, 1]
 * - 127 onto 0
 * - 255 is 1.0079
 *
 * @subsection CurveExamples Examples
 * Curve Pattern | Command ([] means send binary representation)
 * -------------- | -------
 * 6 points, 50% duty cycle, 2 point period, ranging from 0 to 1 | CURVe #16[127][255][127][255][127][255]
 * 12 points, Triangle wave from [-1,1] | CURVe #212[127][169][213][255][213][169][127][85][42][0][42][85]
 * @note Actual waveforms are restricted to have lengths that are multiples of 32, but for simplicity these are shorter.
 */

#define AWG_ZERO_VAL 127 //!< The integer value that corresponds to a zero-volt output on the AWG

/*	See (equip. man. 4B-11)
 *	Max clock rate is 1.024 GHz (= 1024 MHz)
 *	Min clock rate is 1 kHz     (= 0.001 MHz)
 */

#define	PI		3.141592653589793 //!< Pi to double precision
#define	TWO_PI	6.283185307179586 //!< Twice pi to double precision
#define DEFAULT_FREQ_LIST_SIZE 8  //!< Default frequency list size, tradeoff between minimum memory footprint and overhead for expansion if too small.

/*!
 * @defgroup GenBinaryRetCodes genBinary subsystem return codes
 * @brief Return codes used internally to indicate kinds of failures.
 * @{
 */
#define GEN_BINARY_EPARSE  -1 //!< An error occurred while parsing a value
#define GEN_BINARY_ERESIZE -2 //!< An error occurred while resizing an array
/*! @} */

/*! @brief Holds all the information needed to describe a train of frequency pulses.
 *
 *  Stores pointers to arrays containing the frequencies, amplitudes, and durations of each pulse.
 *  The utilized and actual sizes of the arrays are actually stored.
 *  The arrays are in matched order, i.e. the first pulse represented by the 0th element of each array.
 */
typedef struct freqList {
	unsigned int	freqCount;	//!< The number of frequency pulses actually used in freqList, ampList, and durList.
	unsigned int	actualSize;	//!< The total number of spaces for values in the arrays freqList, ampList, and durList.
	double *		freqList;	//!< Array of frequency values, in MHz.
	double *		ampList;	//!< Array of relative amplitude values, on interval [0,1]
	double *		durList;	//!< Array of pulse durations, in ns.
} freqList_type;
typedef freqList_type * freqList_ptr;  //!< Pointer to a #freqList

/*!	@brief Allocates an empty freqList
 *
 * Default values are 0 or NULL, as appropriate.
 *
 * @return A pointer to the newly allocated freqList, otherwise a NULL pointer on failure.
 */
freqList_ptr blankFreqList();
 
/*!	@brief Frees all memory associated with the referenced freqList
 *
 * @note Cannot marker the pointer as NULL, the caller must do this.
 *
 * @param[in] toFree A pointer to the freqList to be freed.
 */
void freeFreqList(freqList_ptr toFree);

/*!	@brief Allocates new arrays of fixed size for the referenced freqList
 *
 * Allocates all three freqList sub-arrays, and sets both freqCount and actualSize to that length.
 *
 * @warning Does not check if there are already arrays referenced, so check the pointers first.
 * If they are non-null, calling this function is a memory leak.
 *
 * @param[inout] toSet Pointer to the freqList whose arrays are being allocated.
 * @param[in] nFreqs The length of the arrays to allocate.
 * @return 0 on success, -1 on failure.
 */
int allocSubLists(freqList_ptr toSet, unsigned int nFreqs);

/*!	@brief Sets the #freqList::freqList to frequencies evenly spaced over an interval
 *
 * Uses the freqCount memeber of the referenced freqList for the number of frequencies,
 * and determines the bounds from the passed parameters.
 *
 * @param[inout] toSet Pointer to the freqList to change.
 * @param[in] start_f The first frequency in the pulse train
 * @param[in] stop_f The final frequency in the pulse train.
 * @return 0, always.
 */
int setFreqList(freqList_ptr toSet, const double start_f, const double stop_f);

/*!	@brief 
 *
 * <details>
 *
 * @param[inout] toSet <descriptions>
 * @return 0, always.
 */
int setFixedAmp(freqList_ptr toSet, const double amplitude);

/*!	@brief 
 *
 * <details>
 *
 * @param[inout] <name> <descriptions>
 * @return <description>
 */
int setRandAmp(freqList_ptr toSet);

/*!	@brief 
 *
 * <details>
 *
 * @param[inout] <name> <descriptions>
 * @return <description>
 */
int setFixedDur(freqList_ptr toSet, const double duration);

/*!	@brief 
 *
 * <details>
 *
 * @param[inout] <name> <descriptions>
 * @return <description>
 */
unsigned int pointsToHalfCycle(double targetDuration, double pointInterval, double frequency);

/*!	@brief 
 *
 * <details>
 *
 * @param[inout] <name> <descriptions>
 * @return <description>
 */
unsigned int * pointCounts(const freqList_ptr freqList, const double pointInterval);

/*!	@brief 
 *
 * <details>
 *
 * @param[inout] <name> <descriptions>
 * @return <description>
 */
unsigned char * genPointList(const freqList_ptr freqList, const unsigned int * pointCounts, const double pointInterval, unsigned long * finalCount);

/*!	@brief 
 *
 * <details>
 *
 * @param[inout] <name> <descriptions>
 * @return <description>
 */
unsigned char * genWavePts(double freq, double amp, unsigned int numPts, double pointInterval, unsigned char * startPtr);

/*!	@brief 
 *
 * <details>
 *
 * @param[inout] <name> <descriptions>
 * @return <description>
 */
ssize_t myGetLine(char ** bufferPtr, size_t * bufferSize, FILE * fp);

/*!	@brief 
 *
 * <details>
 *
 * @param[inout] <name> <descriptions>
 * @return <description>
 */
int resizeFreqList(unsigned int newSize, freqList_ptr * toResize);

/*!	@brief 
 *
 * <details>
 *
 * @param[inout] <name> <descriptions>
 * @return <description>
 */
freqList_ptr readSpecFile(const char * inPath);

/*!	@brief 
 *
 * <details>
 *
 * @param[inout] <name> <descriptions>
 * @return <description>
 */
int parseLine(char * lineBuf, freqList_ptr destList);

/*!	@brief 
 *
 * <details>
 *
 * @param[inout] <name> <descriptions>
 * @return <description>
 */
int writeToFile(const char * rootName, const unsigned char * ptsList, const unsigned long numPtrs, const double clockFreq);

/*!	@brief 
 *
 * <details>
 *
 * @param[inout] <name> <descriptions>
 * @return <description>
 */
int writeSummaryFile(const char * rootName, const freqList_ptr freqList, const unsigned int * pointCounts, const double clock_freq);

#endif
