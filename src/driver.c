#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "genBinary\genBinary.h"

int main (int arc, char * argv[]) {
	freqList_ptr	parsedList	= NULL;
	unsigned int *	countList	= NULL;
	unsigned char *	pointsList	= NULL;
	int				checkStatus	= 0;
	const double	interval = 1000.0/1024.0;
	unsigned long	finalCount = 0;
	const char baseName[] = "testFile";
	const char tempPath[] = "demoFile.txt";
	
	_fmode = _O_BINARY;
	
	parsedList = readSpecFile(tempPath);
	if ( NULL == parsedList ) {
		fprintf(stderr, "Problem parsing file at \"%s\".\n", tempPath);
		return -1;
	}
	
	countList = pointCounts(parsedList, interval);
	if ( NULL == countList ) {
		fprintf(stderr, "Problem counting points.\n");
		return -1;
	}
		
	checkStatus = writeSummaryFile(baseName, parsedList, countList, interval);
	if ( checkStatus ) {
		fprintf(stderr, "Problem writing summary file.\n");
		return -1;
	}
	
	pointsList = genPointList(parsedList, countList, interval, &finalCount);
	if ( NULL == pointsList ) {
		fprintf(stderr, "Problem generating points.\n");
		return -1;
	}
	
	checkStatus = writeToFile(baseName, pointsList, finalCount);
	if ( checkStatus ) {
		fprintf(stderr, "Problem writing points file.\n");
		return -1;
	}
	
	return 0;
}
