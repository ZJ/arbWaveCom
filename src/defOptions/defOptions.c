#include <getopt.h>
#include "defOptions_int.h"
#include "defOptions.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "templateContents.h"

#define  ANY_ALL_TEXT "If any of -s, -e, -n, and -p are supplied, they all must be supplied.\n"

#ifdef _WIN32
#define SUFFIX_EXE ".exe"
#else
#define SUFFIX_EXE ""
#endif

const char helpText[] = "Usage:  genAWGpattern" SUFFIX_EXE " [options]\n\n        genAWGpattern" SUFFIX_EXE " [options] -s <freq> -e <freq> -n <count>\n		                    -p <period> [-r|-a <amplitude>]\n\nOptions:\n  -h | --help           Displays this information\n  -t | --template       Output blank template to " TEMPLATE_FILENAME "\n\n  -d | --debug          Output debug information.\n  -q | --quiet          Suppress normal output.  Does not suppress debug output\n\n  -i | --input-file     Path to an input file\n  -f | --clock-freq     MHz. Sets the target sample clock on the AWG\n\nCommand Line Pulse Specification:\n  WARNING: " ANY_ALL_TEXT "  -s | --start-freq     MHz. Lowest frequency in pulse\n  -e | --end-freq       MHz. High frequency in pulse\n  -n | --number-freq    Total number of pulse teeth\n  -p | --tooth-period   ns. Time per tooth\n  -r | --random-amp     Randomly select amplitude for each tooth from [0.1, 1.0]\n  -a | --fixed-amp      Same amplitude for every tooth (range 0 to 1)\n\n\nGenerates a file whose content is suitable for streaming directly over a serial\nconnection to an AWG2040 to program it with a specified frequency pulse-train.\nPulse parameters are read in from '" INPUT_FILENAME "' (Unless overridden by the '-i'\noption), OR are specified using the 'Command Line Pulse' set of options.\n\nIf both '-i' and any of the 'Command Line Pulse' set are supplied, the last one\nwill be honored.  Likewise the last of any repeated options are honored.\n";

int parseOptions(int argc, char * argv[], progOptions_type * options) {
	int currentOption = -1;
	int errCount = 0;
	while (1) {
		// DELICIOUS OPTIONS TABLE
		static struct option long_options[] = {
			{"fixed-amp", required_argument, 0, 'a'},
			{"debug", no_argument, 0, 'd'},
			{"end-freq", required_argument, 0, 'e'},
			{"clock-freq", required_argument, 0, 'f'},
			{"help", no_argument, 0, 'h'},
			{"input-file", required_argument, 0, 'i'},
			{"number-freq", required_argument, 0, 'n'},
			{"tooth-period", required_argument, 0, 'p'},
			{"quiet", no_argument, 0, 'q'},
			{"random-amp", no_argument, 0, 'r'},
			{"start-freq", required_argument, 0, 's'},
			{"template", no_argument, 0, 't'},
			{0,0,0,0} // Mark the end of the options list
		};
		// END OPTIONS TABLE
		int longOptIdx = 0;
		currentOption = getopt_long(argc, argv, "a:de:f:hi:n:p:qrs:t", long_options, &longOptIdx);
		
		if ( -1 == currentOption ) break; // -1 is out of options
		
		switch (currentOption) {
			case 0:
				break;
			case 'a':
				options->amplitude = strtod(optarg, NULL);
				options->flags |= ( OPT_FROMCMD_MASK | OPT_AMPSET_MASK );
				options->flags &= ~OPT_RANDAMP_MASK;
				break;
			case 'd':
				g_opt_debug = 1;
				break;
			case 'e':
				options->stop_f = strtod(optarg, NULL);
				options->flags |= ( OPT_FROMCMD_MASK | OPT_STOPSET_MASK );
				break;
			case 'f':
				options->clock_freq = strtod(optarg, NULL);
				break;
			case 'h':
				options->flags |= OPT_HELPREQ_MASK;
				break;
			case 'i':
				options->flags &= ~OPT_FROMCMD_MASK;
				if ( NULL != options->inputPath ) free(options->inputPath);
				options->inputPath = malloc(strlen(optarg)+1);
				if ( NULL == options->inputPath ) return OPT_RET_ERR;
				strcpy(options->inputPath, optarg);
				break;
			case 'n':
				options->num_f = strtol(optarg, NULL,0);
				options->flags |= ( OPT_FROMCMD_MASK | OPT_NUMSET_MASK );
				break;
			case 'p':
				options->tooth_period = strtod(optarg, NULL);
				options->flags |= ( OPT_FROMCMD_MASK | OPT_PERIODSET_MASK );
				break;
			case 'q':
				g_opt_quiet = 1;
				break;
			case 'r':
				options->flags |= ( OPT_FROMCMD_MASK | OPT_AMPSET_MASK | OPT_RANDAMP_MASK );
				break;
			case 's':
				options->start_f = strtod(optarg, NULL);
				options->flags |= ( OPT_FROMCMD_MASK | OPT_STARTSET_MASK );
				break;
			case 't':
				options->flags |= OPT_TEMPLATE_MASK;
				break;
			case '?':
				// getopt_long prints an error message
				errCount++;
				break;
			default:
				// shouldn't be possible to get here
				return OPT_RET_ERR;
				break;
		}
		
		if ( g_opt_debug ) printf("Found \"%c\" (or equiv.) with argument %s\n",  currentOption, optarg);
		
	}
	
	if ( g_opt_debug )  printOptions(options, "options");
	
	if ( options->flags & OPT_HELPREQ_MASK ) {
		printf("%s\n", helpText);
		return OPT_RET_EXIT;
	}
	
	if ( errCount > 0 ) {
		printf("\nImproper command format detected.\n\n");
		printf("%s\n", helpText);
		return OPT_RET_ERR;		
	}
	
	if ( options->flags & OPT_TEMPLATE_MASK ) {
		FILE * templateFile = NULL;
		templateFile = fopen(TEMPLATE_FILENAME,"a");
		if ( NULL != templateFile ) {
			fprintf(templateFile, "%s", templateStr);
			if (ferror(templateFile) ) {
				perror("Writing template file");
				fclose(templateFile);
				return OPT_RET_ERR;
			}
			fclose(templateFile);
			if ( !g_opt_quiet ) printf("Template file written to " TEMPLATE_FILENAME ".\n");
		} else {
			perror("Writing template file");
			return OPT_RET_ERR;
		}
		return OPT_RET_EXIT;
	}
	
	if ( (options->flags & OPT_FROMCMD_MASK) && !((options->flags & OPT_ALLSET_MASK) == OPT_ALLSET_MASK) ) {
		printf("ERROR:\t" ANY_ALL_TEXT);
		return OPT_RET_ERR;		
	}
	
	return OPT_RET_OK;
}

void printOptions(const progOptions_type * toPrint, const char * idStr) {
	char optionsStr[] = "options";
	
	const char * optName = ( NULL != idStr ) ? idStr : optionsStr;
	
	printf("\nPrinting current state of %s\n:", optName);
	printf("\t%s.flags:          %08x\n", optName, toPrint->flags);
	printBitSetting(toPrint->flags, OPT_HELPREQ_MASK, "Help Request");
	printBitSetting(toPrint->flags, OPT_TEMPLATE_MASK, "Print Template");
	printBitSetting(toPrint->flags, OPT_FROMCMD_MASK, "From Command");
	printBitSetting(toPrint->flags, OPT_STARTSET_MASK, "Start Frequency Set");
	printBitSetting(toPrint->flags, OPT_STOPSET_MASK, "Stop Frequency Set");
	printBitSetting(toPrint->flags, OPT_NUMSET_MASK, "Number of Frequencies Set");
	printBitSetting(toPrint->flags, OPT_AMPSET_MASK, "Amplitude Set");
	printBitSetting(toPrint->flags, OPT_RANDAMP_MASK, "Random Amplitude");
	printBitSetting(toPrint->flags, OPT_PERIODSET_MASK, "Period Set");
	printf("\t%s.amplitude:      %g\n", optName, toPrint->amplitude);
	printf("\t%s.start_f:        %g\n", optName, toPrint->start_f);
	printf("\t%s.stop_f:         %g\n", optName, toPrint->stop_f);
	printf("\t%s.num_f:          %d\n", optName, toPrint->num_f);
	printf("\t%s.clock_freq:     %g\n", optName, toPrint->clock_freq);
	printf("\t%s.tooth_period:   %g\n", optName, toPrint->tooth_period);
	if ( NULL == toPrint->inputPath ) {
			printf("\t%s.inputPath:      NULL\n", optName);
	} else {
			printf("\t%s.inputPath:      %s\n", optName, toPrint->inputPath);
	}
	return;
}

void printBitSetting(uint32_t flags, unsigned int mask, const char * title) {
	int bitCount = 0;
	if ( !mask ) {
		printf("\t\tMask is zero.\n");
		return;
	}
	
	while( !((mask>>bitCount) & 0x01uL) ) bitCount++;
	
	if ( NULL == title ) {
		printf("\t\tBit %d is %s.\n", bitCount, (mask & flags) ? "set" : "unset");
	} else {
		printf("\t\t\"%s\" (bit %d) is %s.\n", title, bitCount, (mask & flags) ? "set" : "unset");
	}
	return;
}