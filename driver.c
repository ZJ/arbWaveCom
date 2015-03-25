#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "genBinary.h"

int main (int arc, char * argv[]) {
	freqList_ptr freqList = NULL;
	unsigned char * pointList = NULL;
	double * ampList = NULL;
	int i = 0;
	unsigned long patternLength = 0;
	char lenStr[20]= "";
	
	freqList = genFreqList(100e6, 200e6, 13);
	if ( NULL == freqList ) return -1;
	printf("Made freqList\n");
	
	ampList = malloc(sizeof(double)*freqList->freqCount);
	if ( NULL == ampList ) return -1;
	freqList->ampList = ampList;
	printf("Made ampList\n");
	
	printf("freqList->freqCount = %d\n", freqList->freqCount);
	printf("*(freqList->freqList) = %f\n", *(freqList->freqList));
	
	for ( i=0; i<(freqList->freqCount); i++ ) {
		*(ampList + i) = 100.0;
	}
	printf("Init ampList to %f\n", *ampList);
	
	pointList = genPointList(freqList, 5.0e-8, 1.0e-9, &patternLength);
	if ( NULL == pointList ) return -1;
	printf("Done with points\n");
	
	printf("%d\n",patternLength);
	sprintf(lenStr,"%d", patternLength);
	printf("#%d%s\n", strlen(lenStr),lenStr);
	
	free(pointList);
	free(ampList);
	free(freqList);
	
	return 0;
}
