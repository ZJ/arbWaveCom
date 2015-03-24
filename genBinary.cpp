#include "genBinary.hpp"

freqList_ptr genFreqList(double start_f, double stop_f, unsigned int freqCount) {
	freqList_ptr	newList		= NULL;
	double *		listPtr		= NULL;
	double			freqSpacing	= 0.0;
	unsigned int	i = 0;
	
	if ( freqCount <= 1 ) return NULL; // 0 or 1 frequencies is silly
	
	if ( NULL == (newList = malloc(sizeof(freqList)) ) ) return NULL;
	
	newList->freqCount	= freqCount;
	listPtr				= malloc(sizeof(double)*freqCount);
	
	if ( NULL == listPtr ) {
		free(newList);
		return NULL;
	}
	
	newList->freqList = listPtr;
	
	freqSpacing = (stop_f - start_f)/((double) (freqCount - 1));
	listPtr[0] = start_f;
	listPtr[freqCount - 1] = stop_f;
	
	for (i = 1; i < freqCount-1; i++) listPtr[i] = start_f + i*freqSpacing;
	
	return newList;
}

unsigned int pointsToHalfCycle(double targetDuration, double pointInterval, double frequency) {
	double cycles = round(targetDuration * frequency * 2.0)/2.0;
	return (unsigned int) floor(cycles / frequency / pointInterval);
}

unsigned char * genPointList(freqList_ptr freqList, double duration, double pointInterval) {
	unsigned int *	pointCounts = NULL;
	int				i = 0;
	unsigned long	totalPoints = 0;
	double *		freqHead = freqList->freqList;
	unsigned char *	pointVals = NULL;
	unsigned char * fillPos	  = NULL;
	double 			lastFlip	= 1.0;
	pointCounts = malloc(sizeof(unsigned int)*freqList->freqCount);
	if ( NULL == pointCounts ) return NULL;
	
	for ( i=0; i<freqList->freqCount; i++ ) {
		pointCounts[i] = pointsToHalfCycle(duration, pointInterval, freqHead[i]);
		totalPoints += pointCounts[i];
	}
	
	pointVals = malloc(sizeof(unsigned char)*totalPoints);
	if ( NULL == pointVals ) {
		free(pointCounts);
		return NULL;
	}
	fillPos = pointVals;
	
	for ( i=0; i<freqList->freqCount; i++ ) {
		fillPos = genWavePts((freqList->freqList)[i], (freqList->ampList)[i] * lastFlip, pointCounts[i], fillPos);
		lastFlip = *(fillPos - 1) < AWG_ZERO_VAL ? 1.0 : -1.0;
	}
	
	return pointVals;
}

genWavePts(double freq, double amp, unsigned int numPts, unsigned char * startPtr) {
	
}
