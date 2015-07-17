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
 * 14253697 | #814253697
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
 * @return A pointer to the newly allocated freqList
 * @return NULL pointer on failure.
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
 * @return 0 on success
 * @return -1 on failure.
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

/*!	@brief Sets the amplitude of every pulse.
 *
 * Number of pulses is pulled from toSet's #freqList::freqCount member.
 *
 * @param[inout] toSet Pointer to the freqList to change.
 * @param[in] amplitude The value of the amplitude to use.
 * @return 0, always.
 */
int setFixedAmp(freqList_ptr toSet, const double amplitude);

/*!	@brief Sets every pulse to a random amplitude.
 *
 * Number of pulses is pulled from toSet's #freqList::freqCount member.
 *
 * Amplitudes are chosen on the interval [0.1, 1.0] (or [13, 127]).
 * Uses rand() and calls srand( time(NULL)) for random number generation.
 *
 * @param[inout] toSet Pointer to the freqList to change.
 * @return 0, always
 */
int setRandAmp(freqList_ptr toSet);

/*!	@brief Sets the duration of every pulse to the same value.
 *
 * Number of pulses is pulled from toSet's #freqList::freqCount member.
 *
 * @param[inout] toSet Pointer to the freqList to change.
 * @param[in] duration Duration of each pulse, in ns.
 * @return 0, always
 */
int setFixedDur(freqList_ptr toSet, const double duration);

/*!	@brief Find the number of points that finishes a half cycle closest to a target duration.
 *
 * This allows us to always change frequencies at a zero crossing.
 * This makes phase continuity is easy to enforce, even with varying amplitudes.
 * Additionally, the last point will always be before the zero crossing,
 * so the first point of the next pulse will never result in discontinuity.
 *
 * @param[in] targetDuration How long we'd like the pulse to last, in ns.
 * @param[in] pointInterval The duration of an individual output sample, in ns.
 * @param[in] frequency The frequency for this pulse, in MHz.
 * @return The number of output samples to complete a half-cycle nearest the targetDuration.
 */
unsigned int pointsToHalfCycle(double targetDuration, double pointInterval, double frequency);

/*!	@brief Allocates an array holding the number of samples for every pulse.
 *
 * Utilizes information in freqList to allocate an array, the fills it
 * via calls to pointsToHalfCycle().
 *
 * @param[in] freqList A pointer to the the #freqList describing the pulse train.
 * @param[in] pointInterval The output sample period being used, in ns.
 * @return A pointer to the array of points on success
 * @return NULL on failure.
 */
unsigned int * pointCounts(const freqList_ptr freqList, const double pointInterval);

/*!	@brief Generates the full waveform, including continuity and length checks.
 *
 * Takes the pulse description and other key parameters and generates all the output samples.
 * Because we require continuity of the function and its derivative, we check for the final
 * slope, and duplicate and flip the entire waveform if necessary.
 *
 * Finally, the AWG itself requires waveforms to have a multiple of 32 samples.
 * If this didn't already happen, we duplicate the entire waveform as many times as needed
 * to meet this condition.
 *
 * @warning No check on total length fitting in memory is performed.  However unlikely, if
 * you exceed the total number of points allowed, I don't know what the AWG will do.
 *
 * @param[in] freqList A pointer to the the freqList describing the pulse train.
 * @param[in] pointCounts An array holding the length of each pulse in output samples.
 * @param[in] pointInterval The output sample period, in ns.
 * @param[inout] finalCount Pointer to memory to hold the total number of points in the final waveform.
 * @return Pointer to the array holding all of the output waveform's points
 */
unsigned char * genPointList(const freqList_ptr freqList, const unsigned int * pointCounts, const double pointInterval, unsigned long * finalCount);

/*!	@brief Generate the output samples for an individual pulse.
 *
 * Fills the next numPts unsigned chars starting at startPtr with
 * a sin wave with the passed parameters.
 *
 * Uses #AWG_ZERO_VAL to set the offset of the waveform from zero.
 *
 * @warning No bounds checking for the array is performed internally,
 * because the function doesn't have access to the information it needs to do that.
 *
 * @param[in] freq The frequency of the pulse, in MHz
 * @param[in] amp The amplitude of the pulse, should be in the range [-127.0, 127.0]
 * @param[in] numPts The number of samples to output
 * @param[in] pointInterval The output sample period, in ns.
 * @param[in] startPtr The first location to put a point in.
 * @return A pointer to the position in the array \e after the last one it filled.
 */
unsigned char * genWavePts(double freq, double amp, unsigned int numPts, double pointInterval, unsigned char * startPtr);

/*!	@brief A custom, getLine implementation
 *
 * See [GNU Getline Documentation](http://www.gnu.org/software/libc/manual/html_node/Line-Input.html)
 *
 * Reads a line from the specified file into the provided buffer.
 * Will automatically resize the buffer for lines that will not fit.
 *
 * @param[inout] bufferPtr Pointer to the buffer containing the line that was read
 * @param[inout] bufferSize Total size, in bytes, of the buffer at bufferPtr.
 * @param[inout] fp An open file pointer to the file we're reading from.
 * @return The number of characters we read into the buffer, including the newline (if present), but not the terminating NULL character.
 * @return -1 on error, including EOF.
 */
ssize_t myGetLine(char ** bufferPtr, size_t * bufferSize, FILE * fp);

/*!	@brief Resizes all sublists of the pointed-to freqList to the specified length.
 *
 * Specifically freqList has array members %freqList, ampList, and durList.
 *
 * Possible reasons for returning an error value:
 * - Passing a NULL pointer, or a pointer to a NULL pointer
 * - realloc() call failed for any of the sub-lists.
 *
 * @param[in] newSize New length for all of the sub-lists
 * @param[inout] toResize Points to a freqList_ptr of the freqList for resizing.
 * @return 0 on success
 * @return -1 on error
 */
int resizeFreqList(unsigned int newSize, freqList_ptr * toResize);

/*!	@brief Parse the file at the passed path for a pulse train specification
 *
 * Reads the file line by line via myGetLine() and parses the contents via parseLine(),
 * attempting to build the pulse train specification.
 *
 * @param[in] inPath A string containing the path to the file to read the spec's from.
 * @return A pointer to the freqList from parsing the file
 * @return NULL on failures:
 * - Unable to (re)allocate buffers
 * - No pulses specified in the file
 * - Error reading the file
 */
freqList_ptr readSpecFile(const char * inPath);

/*!	@brief Takes text specifying a frequency pulse and adds it to the end of a freqList
 *
 * The format is described in #templateStr in templateContents.h
 *
 * @param[inout] lineBuf The buffer containing the line to be parsed
 * @param[inout] destList The freqList we'll add any specified new pulse.
 * @return 0 on success
 * @return #GEN_BINARY_ERESIZE if we failed to make room for the new pulse.
 * @return #GEN_BINARY_EPARSE if we couldn't parse the line (likely because it was malformed)
 */
int parseLine(char * lineBuf, freqList_ptr destList);

/*!	@brief Writes byte stream suitable for transmission to the AWG to a file.
 *
 * File will be output as "\<rootName\>_points".
 * E.g. a rootName of "test" would result in a file "test_points"
 *
 * See writeSummaryFile() for human-readable description.
 *
 * Commands generated to save the waveform on the AWG as "GPIB.WFM",
 * then set up the correct format for the transfer, send the points,
 * set the correct clock frequency, and ask for confirmation information back.
 *
 * @param[in] rootName The base of the filename we're saving to.
 * @param[in] ptsList Pointer to the array of output samples, already stored in AWG format
 * @param[in] numPtrs Total number of points in the output waveform
 * @param[in] clockFreq The output sample frequency
 * @return 0 on success
 * @return -1 on failure
 */
int writeToFile(const char * rootName, const unsigned char * ptsList, const unsigned long numPtrs, const double clockFreq);

/*!	@brief Writes a human-readable text file describing the contents of the generated points file.
 *
 * File will be output as "\<rootName\>_desc.txt"
 * E.g. a rootName of "test" would result in a file "test_desc.txt"
 *
 * The file contains the frequency, amplitude, duration, and number of samples for each pulse, in order.
 * It also lists the output sample frequency (and period) used.
 *
 * See writeToFile() for actual contents of points file.
 *
 * @param[in] rootName The base of the filename we're saving to.
 * @param[in] freqList freqList describing the generated pulse train.
 * @param[in] pointCounts Total number of points for each pulse in the output waveform
 * @param[in] clock_freq The output sample frequency
 * @return 0 on success
 * @return -1 on failure
 */
int writeSummaryFile(const char * rootName, const freqList_ptr freqList, const unsigned int * pointCounts, const double clock_freq);

#endif
