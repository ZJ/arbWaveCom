#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "genBinary.h"

freqList_ptr genFreqList(double start_f, double stop_f, unsigned int freqCount) {
	freqList_ptr	newList		= NULL;
	double *		listPtr		= NULL;
	double			freqSpacing	= 0.0;
	unsigned int	i = 0;
	
	if ( freqCount <= 1 ) return NULL; // 0 or 1 frequencies is silly
	
	if ( NULL == (newList = malloc(sizeof(freqList_type)) ) ) return NULL;
	
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

unsigned char * genPointList(freqList_ptr freqList, double duration, double pointInterval, unsigned long * finalCount) {
	unsigned int *	pointCounts = NULL;
	int 			numShifts = 0;
	int				i = 0;
	unsigned long	totalPoints = 0;
	double *		freqHead = freqList->freqList;
	unsigned char *	pointVals = NULL;
	unsigned char * fillPos	  = NULL;
	double 			lastFlip	= 1.0;
	pointCounts = malloc(sizeof(unsigned int)*freqList->freqCount);
	if ( NULL == pointCounts ) return NULL;
	printf("Alloc pointCounts\n");
	printf("Dur: %e, int %e\n", duration, pointInterval);
	
	for ( i=0; i<freqList->freqCount; i++ ) {
		*(pointCounts + i) = pointsToHalfCycle(duration, pointInterval, *(freqHead + i));
		totalPoints += *(pointCounts + i);
	}
	printf("Filled pointCounts\n");
	
	pointVals = malloc(sizeof(unsigned char)*totalPoints);
	if ( NULL == pointVals ) {
		free(pointCounts);
		return NULL;
	}
	fillPos = pointVals;
	printf("Alloc pointVals\n");
	
	for ( i=0; i<freqList->freqCount; i++ ) {
		/*
		printf("*(freqList->freqList + i) = %f\n",*(freqList->freqList + i));
		printf("*(freqList->ampList + i) = %f\n",*(freqList->ampList + i));
		printf("*(pointCounts + i) = %d\n",*(pointCounts + i));
		*/
		fillPos = genWavePts(*(freqList->freqList + i), *(freqList->ampList + i) * lastFlip, *(pointCounts + i), pointInterval, fillPos);
		lastFlip = *(fillPos - 1) < AWG_ZERO_VAL ? 1.0 : -1.0;
	}
	printf("Total points after cont. check: %d\n", totalPoints);
	printf("last flip: %f\n",lastFlip);
	if ( lastFlip > 0.0 ) {
		pointVals = realloc(pointVals, sizeof(unsigned char)*totalPoints*2);
		if ( NULL == pointVals ) return NULL;
		
		for (i=0; i<totalPoints; i++) *(pointVals + totalPoints + i) = *(pointVals + i) * -1.0;
		totalPoints *=2;
	}
	
	printf("Total points after cont. check: %d\n", totalPoints);
	//printf("Mod 32: %d\n",totalPoints%32);
	
	if ((totalPoints%32) != 0 ) {
		numShifts = 1;
		while ( (totalPoints << numShifts)&0x1F )
		{
			printf("Mod 32: %d\n", (totalPoints << numShifts)%32);
			numShifts++;
		}
		printf("Mod 32: %d\n", (totalPoints << numShifts)%32);
		printf("Shift count: %d, to %d\n", numShifts, totalPoints << numShifts);
		
		pointVals = realloc(pointVals, sizeof(unsigned char)*totalPoints*numShifts);
		if ( NULL == pointVals ) return NULL;
		
		for (i=0; i<numShifts; i++) {
			printf("Copy level %d\n",i);
			memcpy(pointVals + (1<<i)*totalPoints, pointVals, sizeof(unsigned char)*(1<<i)*totalPoints);
		}
	}
	
	
	*finalCount = (1<<numShifts)*totalPoints;
	printf("final count %d\n",*finalCount);
	return pointVals;
}

unsigned char * genWavePts(double freq, double amp, unsigned int numPts, double pointInterval, unsigned char * startPtr) {
	unsigned int i = 0;
	//printf("Made it inside the function, what is even????????");
	for ( i=0; i < numPts; i++ ) {
		double point = amp * sin(freq * ((double) i) * pointInterval * TWO_PI) + ((double) AWG_ZERO_VAL);
		printf("%f, %f -> %d\n",amp * sin(freq * ((double) i) * pointInterval * TWO_PI), point, (unsigned char) round(point));
		*(startPtr + i) = round(point);
	}
	return (startPtr + numPts);
}
