#include <getopt.h>
#include "defOptions_int.h"
#include "defOptions.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"

const char helpText[] = "Usage:  genAWGpattern[.exe] [options]\n\n        genAWGpattern[.exe] [options] -s <freq> -e <freq> -n <count>\n		                    -p <period> [-r|-a <amplitude>]\n\nOptions:\n  -h | --help           Displays this information\n  -t | --template       Output blank template to \"demoFile.txt\"\n\n  -d | --debug          Output debug information.\n  -q | --quiet          Suppress normal output.  Does not suppress debug output\n\n  -i | --input-file     Path to an input file\n  -f | --clock-freq     Sets the target sample clock on the AWG\n\nCommand Line Pulse Specification:\n  WARNING: If any of -s, -e, -n, and -p are supplied, they all must be supplied.\n  -s | --start-freq     Lowest frequency in pulse [MHz]\n  -e | --end-freq       High frequency in pulse [MHz]\n  -n | --number-freq    Total number of pulse teeth\n  -p | --tooth-period   Time per tooth [ns]\n  -r | --random-amp     Randomly select amplitude for each tooth\n  -a | --fixed-amp      Same amplitude for every tooth (range 0 to 1)\n\n\nGenerates a file whose content is suitable for streaming directly over a serial\nconnection to an AWG2040 to program it with a specified frequency pulse-train.\nPulse parameters are read in from 'demoFile.txt' (Unless overridden by the '-i'\noption), OR are specified using the 'Command Line Pulse' set of options.\n\nIf both '-i' and any of the 'Command Line Pulse' set are supplied, the last one\nwill be honored.  Likewise the last of any repeated options are honored.\n";

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
			{"random-amp", required_argument, 0, 'r'},
			{"start-freq", required_argument, 0, 's'},
			{"template", no_argument, 0, 't'},
			{0,0,0,0} // Mark the end of the options list
		};
		// END OPTIONS TABLE
		int longOptIdx = 0;
		currentOption = getopt_long(argc, argv, "a:de:f:hi:n:p:qr:s:t", long_options, &longOptIdx);
		
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
				options->sample_period = 1000.0/strtod(optarg, NULL);
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
				options->num_f = //Pull string into number;
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
		
	}
	
	if ( errCount > 0 ) {
		printf("%s\n", helpText);
		return OPT_RET_EXIT;		
	}
	
	if ( options->flags & OPT_HELPREQ_MASK ) {
		printf("%s\n", helpText);
		return OPT_RET_EXIT;
	}
	
	if ( options->flags & OPT_TEMPLATE_MASK ) {
		//fprintf("%s", templateText);
		printf("%s\n", "This will be eventually saving to a file.");
		return OPT_RET_EXIT;
	}
	
	
	
	return OPT_RET_OK;
}
