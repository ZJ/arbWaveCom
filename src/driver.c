#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "genBinary\genBinary.h"
#include "defOptions\defOptions.h"

int main (int argc, char * argv[]) {
	freqList_ptr	parsedList	= NULL;
	unsigned int *	countList	= NULL;
	unsigned char *	pointsList	= NULL;
	int				checkStatus	= 0;
	unsigned long	finalCount = 0;
	const char baseName[] = "testFile";
	const char tempPath[] = "demoFile.txt";
	
	progOptions_type	myOptions = OPT_INIT_VAL;
	
	
	checkStatus = parseOptions(argc, argv, &myOptions);
	switch (checkStatus) {
		case OPT_RET_OK:
			// Do nothing
			break;
		case OPT_RET_EXIT:
			return 0;
			break;
		case OPT_RET_ERR:
			return -1;
			break;
		default:
			//HOW!?
			return -2;
			break;
	}
	
	// This is the earliest you're allowed to print anything to stdout
	// Because before now we might've been ignoring a --quiet options if we did.
	
	// stderr is OK, because I said so.
	
	if ( !(OPT_FROMCMD_MASK & myOptions.flags) ) {
		const char * loadPath = (NULL == myOptions.inputPath) ? tempPath : myOptions.inputPath;
		if ( !g_opt_quiet ) printf("Attempting to load frequency list from file: \"%s\".\n", loadPath);
		parsedList = readSpecFile(loadPath);
		if ( NULL == parsedList ) {
			fprintf(stderr, "Problem parsing file at \"%s\".\n", loadPath);
			return -1;
		}
	} else {
		// Loading from frigging cmdline
		parsedList = blankFreqList();
		if ( NULL == parsedList ) {
			fprintf(stderr, "Problem allocating frequency list\n");
			return -1;
		}
		if ( allocSubLists(parsedList, myOptions.num_f) ) {
			fprintf(stderr, "Problem allocating frequency list\n");
			return -1;
		}
		setFreqList(parsedList, myOptions.start_f, myOptions.stop_f);
		setFixedDur(parsedList, myOptions.tooth_period);
		if ( OPT_RANDAMP_MASK & myOptions.flags ) {
			// Random amplitudes
			setRandAmp(parsedList);
		} else {
			setFixedAmp(parsedList, myOptions.amplitude);
		}
	}
	
	countList = pointCounts(parsedList, myOptions.sample_period);
	if ( NULL == countList ) {
		fprintf(stderr, "Problem counting points.\n");
		return -1;
	}
		
	checkStatus = writeSummaryFile(baseName, parsedList, countList, myOptions.sample_period);
	if ( checkStatus ) {
		fprintf(stderr, "Problem writing summary file.\n");
		return -1;
	}
	
	pointsList = genPointList(parsedList, countList, myOptions.sample_period, &finalCount);
	if ( NULL == pointsList ) {
		fprintf(stderr, "Problem generating points.\n");
		return -1;
	}
	
	_fmode = _O_BINARY; // Turn off line ending conversion.
	checkStatus = writeToFile(baseName, pointsList, finalCount);
	if ( checkStatus ) {
		fprintf(stderr, "Problem writing points file.\n");
		return -1;
	}
	
	return 0;
}
