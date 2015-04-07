#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "genBinary.h"
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include "../defOptions/defOptions.h"

freqList_ptr blankFreqList() {
	freqList_ptr newList = malloc(sizeof(freqList_type));
	if ( NULL == newList ) return NULL;
	
	newList->freqCount	= 0;
	newList->actualSize	= 0;
	newList->freqList	= NULL;
	newList->ampList	= NULL;
	newList->durList	= NULL;
	
	return newList;
}

void freeFreqList(freqList_ptr toFree) {
	if ( NULL == toFree ) return;
	if ( NULL != toFree->freqList ) free(toFree->freqList);
	if ( NULL != toFree->ampList  ) free(toFree->ampList);
	if ( NULL != toFree->durList  ) free(toFree->durList);
	free(toFree);
	return;
}	

int allocSubLists(freqList_ptr toSet, unsigned int nFreqs) {
	toSet->freqList = malloc(sizeof(double)*nFreqs);
	if ( NULL == toSet->freqList ) return -1;

	toSet->ampList = malloc(sizeof(double)*nFreqs);
	if ( NULL == toSet->ampList ) return -1;

	toSet->durList = malloc(sizeof(double)*nFreqs);
	if ( NULL == toSet->durList ) return -1;
	
	toSet->freqCount	= nFreqs;
	toSet->actualSize	= nFreqs;
	return 0;
}

int setFreqList(freqList_ptr toSet, const double start_f, const double stop_f) {
	double *			listPtr 	= toSet->freqList;
	double				freqSpacing	= 0.0;
	const unsigned int	nFreqs		= toSet->freqCount;
	unsigned int		i			= 0;
	
	freqSpacing = (stop_f - start_f)/((double) (nFreqs - 1));
	
	for (i = 1; i < nFreqs-1; i++) listPtr[i] = start_f + i*freqSpacing;

	listPtr[0] = start_f;
	listPtr[nFreqs - 1] = stop_f;
	
	return 0;
}

int setFixedAmp(freqList_ptr toSet, const double amplitude) {
	double *			listPtr 	= toSet->ampList;
	const unsigned int	nFreqs		= toSet->freqCount;
	unsigned int		i			= 0;
	
	for (i = 0; i < nFreqs; i++) listPtr[i] = amplitude;

	return 0;
}

int setRandAmp(freqList_ptr toSet) {
	double *			listPtr 	= toSet->ampList;
	const unsigned int	nFreqs		= toSet->freqCount;
	unsigned int		i			= 0;
	
	srand((unsigned) time(NULL));
	for (i = 0; i < nFreqs; i++) listPtr[i] = 114.0*((double) rand())/(127.0*(double) RAND_MAX)+13.0/127.0;

	return 0;
}

int setFixedDur(freqList_ptr toSet, const double duration) {
	double *			listPtr 	= toSet->durList;
	const unsigned int	nFreqs		= toSet->freqCount;
	unsigned int		i			= 0;
	
	for (i = 0; i < nFreqs; i++) listPtr[i] = duration;

	return 0;
}

unsigned int pointsToHalfCycle(double targetDuration, double pointInterval, double frequency) {
	double cycles = round(targetDuration * frequency * 2.0 * 0.001)/2.0;
	return (unsigned int) floor(1000.0*cycles / frequency / pointInterval);
}

unsigned int * pointCounts(const freqList_ptr freqList, const double pointInterval) {
	int					i = 0;
	unsigned int		totalSets	= freqList->freqCount;
	const double *		freqTable	= freqList->freqList;
	const double *		durTable	= freqList->durList;
	unsigned int *		pointCounts	= NULL;
	
	if ( NULL == freqList ) return NULL;
	
	pointCounts = malloc(((size_t) totalSets)*sizeof(unsigned int));
	if ( NULL == pointCounts ) {
		perror("pointCounts allocation");
		return NULL;
	}
	
	for ( i=0; i<totalSets; i++ ) {
		*(pointCounts + i) = pointsToHalfCycle(*(durTable + i), pointInterval, *(freqTable + i));
	}
	
	return pointCounts;
}

unsigned char * genPointList(const freqList_ptr freqList, const unsigned int * pointCounts, const double pointInterval, unsigned long * finalCount) {
	int 			numShifts	= 0;
	int				i			= 0;
	unsigned long	totalPoints	= 0;
	const double *	freqTable	= freqList->freqList;
	const double *	ampTable	= freqList->ampList;
	unsigned char *	pointVals	= NULL;
	unsigned char * fillPos		= NULL;
	double 			lastFlip	= 1.0;

	for ( i=0; i<freqList->freqCount; i++ ) {
		totalPoints += *(pointCounts + i);
	}
	if ( g_opt_debug ) printf("Filled pointCounts\n");
	
	pointVals = malloc(sizeof(unsigned char)*totalPoints);
	if ( NULL == pointVals ) return NULL;

	fillPos = pointVals;
	if ( g_opt_debug ) printf("Alloc pointVals\n");
	
	for ( i=0; i<freqList->freqCount; i++ ) {
		/*
		printf("*(freqList->freqList + i) = %f\n",*(freqList->freqList + i));
		printf("*(freqList->ampList + i) = %f\n",*(freqList->ampList + i));
		printf("*(pointCounts + i) = %d\n",*(pointCounts + i));
		*/
		fillPos = genWavePts(*(freqTable + i), *(ampTable + i) * lastFlip * 127.0, *(pointCounts + i), pointInterval, fillPos);
		lastFlip = *(fillPos - 1) < AWG_ZERO_VAL ? 1.0 : -1.0;
	}
	if ( g_opt_debug ) printf("Total points after cont. check: %lu\n", totalPoints);
	if ( g_opt_debug ) printf("last flip: %f\n", lastFlip);
	if ( lastFlip < 0.0 ) {
		pointVals = realloc(pointVals, sizeof(unsigned char)*totalPoints*2);
		if ( NULL == pointVals ) return NULL;
		
		for (i=0; i<totalPoints; i++) *(pointVals + totalPoints + i) = *(pointVals + i) * -1.0;
		totalPoints *=2;
	}
	
	if ( g_opt_debug ) printf("Total points after cont. check: %lu\n", totalPoints);
	//printf("Mod 32: %d\n",totalPoints%32);
	
	if ((totalPoints%32) != 0 ) {
		numShifts = 1;
		while ( (totalPoints << numShifts)&0x1F )
		{
			if ( g_opt_debug ) printf("Mod 32: %lu\n", (totalPoints << numShifts)%32);
			numShifts++;
		}
		if ( g_opt_debug ) printf("Mod 32: %lu\n", (totalPoints << numShifts)%32);
		if ( g_opt_debug ) printf("Shift count: %d, to %lu\n", numShifts, totalPoints << numShifts);
		
		pointVals = realloc(pointVals, sizeof(unsigned char)*totalPoints*(1<<numShifts));
		if ( NULL == pointVals ) return NULL;
		
		for (i=0; i<numShifts; i++) {
			if ( g_opt_debug ) printf("Copy level %d\n",i);
			memcpy(pointVals + (1<<i)*totalPoints, pointVals, sizeof(unsigned char)*(1<<i)*totalPoints);
		}
	}
	
	
	*finalCount = (1<<numShifts)*totalPoints;
	if ( !g_opt_quiet ) printf("Final point count %lu\n",*finalCount);
	return pointVals;
}

unsigned char * genWavePts(double freq, double amp, unsigned int numPts, double pointInterval, unsigned char * startPtr) {
	unsigned int i = 0;
	//printf("Made it inside the function, what is even????????");
	for ( i=0; i < numPts; i++ ) {
		double point = amp * sin(freq * ((double) i) * pointInterval * TWO_PI * 0.001) + ((double) AWG_ZERO_VAL);
		//printf("%f, %f -> %d\n",amp * sin(freq * ((double) i) * pointInterval * TWO_PI), point, (unsigned char) round(point));
		*(startPtr + i) = round(point);
	}
	return (startPtr + numPts);
}

ssize_t myGetLine(char ** bufferPtr, size_t * bufferSize, FILE * fp) {
	#define DEFAULT_BUFF_INC_SIZE 128
	ssize_t	readCount = 0;
	int		thisChar = '\0';
	
	// Check for null pointers, these are always errors
	if ( NULL == bufferPtr  ) {
		fprintf(stderr, "In myGetLine: bufferPtr is NULL.\n");
		return -1;
	}
	
	if ( NULL == bufferSize ) {
		fprintf(stderr, "In myGetLine: bufferSize is NULL.\n");
		return -1;
	}
	
	if ( NULL == fp ) {
		fprintf(stderr, "In myGetLine: fp is NULL.\n");
		return -1;
	}
	
	// Allocate a new buffer if NULL ptr and 0 size, otherwise error
	if ( NULL == *bufferPtr ) {
		if ( 0 != *bufferSize ) {
			fprintf(stderr, "In myGetLine: *bufferPtr is NULL with nonzero *bufferSize.\n");
			return -1;
		}
		*bufferPtr = malloc(DEFAULT_BUFF_INC_SIZE);
		if ( NULL == *bufferPtr ) {
			perror("In myGetLine");
			return -1;
		}
		*bufferSize = DEFAULT_BUFF_INC_SIZE;
	}
	
	// If the bufferSize is <= 0 now, that don't make no sense.
	if ( *bufferSize <= 0 ) return -1;
	
	// If the stream is in the error state before we touch it, what are you even doing?
	if ( ferror(fp) ) {
		fprintf(stderr, "In myGetLine: fp already in error state.\n");
		return -1;
	}
	
	// Initial checks are done. Finally.  Time to load in the line.
	thisChar = fgetc(fp);
	
	// If just EOF, just return -1.
	if ( feof(fp) ) return -1;
	
	while ( !feof(fp) ) {
		// Remember to leave space for null termination
		if ( (readCount + 2) >= *bufferSize ) {
			char * oldBufferPtr  = *bufferPtr;
			*bufferPtr = realloc(*bufferPtr, (*bufferSize) + DEFAULT_BUFF_INC_SIZE);
			if ( NULL == *bufferPtr ) {
				perror("In myGetLine:");
				*bufferPtr  = oldBufferPtr;
				// Buffer size isn't changed automatically
				return -1;
			}
			*bufferSize = (*bufferSize) + DEFAULT_BUFF_INC_SIZE;
		}
		
		*((*bufferPtr) + readCount) = (char) thisChar;
		readCount++;
		
		if ( thisChar == '\n' ) {
			break;
		}
		
		thisChar = fgetc(fp);
		if ( ferror(fp) ) {
			perror("In myGetLine, while reading");
			return -1;
		}
	}
	
	// Promise null-terminated string, and readCount doesn't include that.
	*((*bufferPtr) + readCount) = '\0';
	return readCount;
		
	#undef DEFAULT_BUFF_INC_SIZE
}

int resizeFreqList(unsigned int newSize, freqList_ptr * toResize) {
	freqList_ptr thisOne = *toResize;
	double * tempPtr = NULL;
	int		 errsv	 = 0;
	
	if ( NULL == toResize ) return -1;
	if ( NULL == *toResize ) return -1;
	
	tempPtr = realloc(thisOne->freqList, sizeof(double)*newSize);
	if ( NULL == tempPtr ) {
		errsv = errno;
		freeFreqList(thisOne);
		*toResize = NULL;
		errno = errsv;
		return -1;
	}
	thisOne->freqList = tempPtr;
	
	tempPtr = realloc(thisOne->ampList, sizeof(double)*newSize);
	if ( NULL == tempPtr ) {
		errsv = errno;
		freeFreqList(thisOne);
		*toResize = NULL;
		errno = errsv;
		return -1;
	}
	thisOne->ampList = tempPtr;

	tempPtr = realloc(thisOne->durList, sizeof(double)*newSize);
	if ( NULL == tempPtr ) {
		errsv = errno;
		freeFreqList(thisOne);
		*toResize = NULL;
		errno = errsv;
		return -1;
	}
	thisOne->durList = tempPtr;
	
	thisOne->actualSize = newSize;
	return 0;
}

freqList_ptr readSpecFile(const char * inPath) {
	// file io set
	FILE * 			specFile	= NULL;
	char *			lineBuf		= NULL;
	size_t			lineBufSize	= 0;
	ssize_t			lineLen		= -1;
	unsigned long	lineNum		= 0;
	int				storeFerror = 0;
	// frequency list set
	freqList_ptr 	listPtr		= NULL;
	
	// Initial malloc for list.
	listPtr = blankFreqList();
	if ( NULL == listPtr ) return NULL;
	if ( resizeFreqList(DEFAULT_FREQ_LIST_SIZE, &listPtr) )  return NULL;
	
	// Open the file for reading
	specFile = fopen(inPath, "r");
	if ( (NULL == specFile) || ferror(specFile) ) {
		int errsv = errno;
		freeFreqList(listPtr);
		errno = errsv;
		return NULL;
	}
	
	// Process line-by-line
	// lineBuf allocated here, remember we've got to call free()
	lineLen = myGetLine(&lineBuf, &lineBufSize, specFile);
	while ( lineLen >= 1) {
		int parseResult;
		lineNum++;
		if ( (parseResult = parseLine(lineBuf, listPtr)) ) {
			if ( GEN_BINARY_ERESIZE == parseResult ) {
					fclose(specFile);
					free(lineBuf);
					freeFreqList(listPtr);
					return NULL;
			} else if ( GEN_BINARY_EPARSE == parseResult ) {
				fprintf(stderr, "Error parsing file at line %lu, ignoring line:\n  > %s\n", lineNum, lineBuf);
			}
		}
		// Get Next Line
		lineLen = myGetLine(&lineBuf, &lineBufSize, specFile);
	}
	
	// Because we've got stuff to do either way first
	storeFerror = (ferror(specFile) && !feof(specFile));

	// Close the file, clean up our buffers
	fclose(specFile);
	free(lineBuf);
	lineBuf = NULL;
	
	if ( !g_opt_quiet ) printf("Processed %lu lines, found %d frequencies.\n", lineNum, listPtr->freqCount);
	malloc(128);
	if ( 0 == listPtr->freqCount ) {
		fprintf(stderr, "However, we need at least one entry.\n");
		freeFreqList(listPtr);
		return NULL;
	}
	
	// Handle error case (anything other than reading the whole file)
	if ( storeFerror ) {
		fprintf(stderr, "However, an file read error occurred at line %lu.\n", lineNum);
		freeFreqList(listPtr);
		return NULL;
	}
	
	// Shrink lists to appropriate sizes (nonzero is error)
	if ( resizeFreqList(listPtr->freqCount, &listPtr) )  return NULL;
	return listPtr;
}

int parseLine(char * lineBuf, freqList_ptr destList) {
	char *				endConv		= NULL;
	const unsigned int  curSize		= destList->actualSize;
	const unsigned int	curCount	= destList->freqCount;
	double *			freqBase	= destList->freqList;
	double *			durBase		= destList->durList;
	double *			ampBase		= destList->ampList;

	// Consume leading whitespace
	while ( isspace(*lineBuf) ) lineBuf++;
	
	// Check if comment, ignore line if it is
	if ( '#' == *lineBuf ) return 0;
	
	// Check if expansion is necessary
	if ( curSize < (curCount + 1) ) {
		if ( resizeFreqList((curSize + DEFAULT_FREQ_LIST_SIZE), &destList) ) {
			return GEN_BINARY_ERESIZE;
		}
		freqBase	= destList->freqList;
		durBase		= destList->durList;
		ampBase		= destList->ampList;
	}
	
	// Read in the new values
	errno = 0;
	*(freqBase + curCount) = strtod(lineBuf, &endConv);
	if ( errno ) return GEN_BINARY_EPARSE;
	lineBuf = endConv;
	
	while ( isspace(*lineBuf) ) lineBuf++;
	if ( ',' != *(lineBuf++) ) return GEN_BINARY_EPARSE;
	
	errno = 0;
	*(durBase + curCount) = strtod(lineBuf, &endConv);
	if ( errno ) return GEN_BINARY_EPARSE;
	lineBuf = endConv;
	
	while ( isspace(*lineBuf) ) lineBuf++;
	if ( ',' != *(lineBuf++) ) return GEN_BINARY_EPARSE;
	
	errno = 0;
	*(ampBase + curCount) = strtod(lineBuf, &endConv);
	if ( errno ) return GEN_BINARY_EPARSE;
	lineBuf = endConv;

	while ( isspace(*lineBuf) ) lineBuf++;
	if ( '\0' != *(lineBuf++) ) return GEN_BINARY_EPARSE;
	
	if ( !g_opt_quiet ) printf("Amp %f, Freq %f, Dur %f\n", *(ampBase + curCount), *(freqBase + curCount), *(durBase + curCount));
	
	destList->freqCount = curCount + 1;
	return 0;
}

int writeToFile(const char * rootName, const unsigned char * ptsList, const unsigned long numPtrs) {
	FILE * pointsFile = NULL;
	char *			fileName		= NULL;
	size_t			fileNameLen;
	const char		fileNameSuf[]	= "_points";
	unsigned int	numLen			= 0;
	unsigned long	numCpy;

	fileNameLen = strlen(rootName) + strlen(fileNameSuf);
	
	fileName = malloc(fileNameLen + 1);
	if ( NULL == fileName ) return -1;
	
	strcpy(fileName, rootName);
	strcat(fileName, fileNameSuf);

	for(numCpy = numPtrs; numCpy != 0; numCpy/=10) numLen++;
	
	pointsFile = fopen(fileName, "w");
	free(fileName);
	if ( NULL == pointsFile ) return -1;
	fprintf(pointsFile, "DATA:DESTINATION \"GPIB.WFM\"\n");
	fprintf(pointsFile, "DATA:WIDTH 1\n");
	fprintf(pointsFile, "CURVE ");
	fprintf(pointsFile, "#%d%lu", numLen, numPtrs);
	fwrite(ptsList, sizeof(unsigned char), numPtrs, pointsFile);
	fprintf(pointsFile, "\n");
	fprintf(pointsFile, "WFMP?\n");
	if ( ferror(pointsFile) ) {
		fclose(pointsFile);
		return -1;
	}
	fclose(pointsFile);
	
	return 0;
}

int writeSummaryFile(const char * rootName, const freqList_ptr freqList, const unsigned int * pointCounts, const double pointInterval) {
	FILE *			sumFile			= NULL;
	char *			fileName		= NULL;
	size_t			fileNameLen;
	const char		fileNameSuf[]	= "_desc.txt";
	int				i;
	const unsigned int entries		= freqList->freqCount;
	const double *	freqTable		= freqList->freqList;
	const double *	ampTable		= freqList->ampList;
	
	fileNameLen = strlen(rootName) + strlen(fileNameSuf);
	
	fileName = malloc(fileNameLen + 1);
	if ( NULL == fileName ) return -1;
	
	strcpy(fileName, rootName);
	strcat(fileName, fileNameSuf);
	
	sumFile = fopen(fileName, "w");
	if( NULL == sumFile ) {
		free(fileName);
		return -1;
	}
	
	fprintf(sumFile, "Frequency pattern summary for %s:\n", fileName);
	for( i=0; i<entries; i++ ) {
		fprintf(sumFile, "\t%f amplitude %f MHz for %f ns.\n", *(ampTable + i), *(freqTable + i), ((double)(*(pointCounts + i)))*pointInterval);
	}
	fclose(sumFile);
	
	free(fileName);
	
	return 0;
}