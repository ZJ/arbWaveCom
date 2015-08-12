
/*! @file defOptions_int.h
 * @brief Sets up global flags for debug and quiet options
 */

#ifndef DEFOPTIONS_INT_H
#define DEFOPTIONS_INT_H

#include "filenames.h"

int                 g_opt_debug = 0;	//!< Whether -d/--debug has been set. 0 is unset, 1 is set.
int                 g_opt_quiet = 0;	//!< Whether -q/--quiet has been set. 0 is unset, 1 is set.

/*!
 * @defgroup HelpText User-requested help message
 * @brief Sets up the text displayed when invoked with -h/--help 
 * @{
 */

 /*! @brief Pick the correct suffix for referring to the execuatable in the help text
  *
  * On Windows builds, this will evaluate to '.exe' on Linux/Unix it will evaluate to the empty string.
  * For documentation purposes the _WIN32 version will be shown.
  */
#ifdef _WIN32
#define SUFFIX_EXE ".exe"
#else
#define SUFFIX_EXE ""
#endif

#define  ANY_ALL_TEXT "If any of -s, -e, -n, and -p are supplied, they all must be supplied.\n"	//!< Text for warning when not supplying all of the required options.

/*! @brief The text displayed when invoked with -h/--help
 *
 * The full text of the help message is printed here, including whatever platform-dependent changes need to be made.
 * The text also depends on #SUFFIX_EXE, #TEMPLATE_FILENAME, #ANY_ALL_TEXT, and #INPUT_FILENAME.
 * 
 * @showinitializer
 */
const char          helpText[] = "Usage:  genAWGpattern" SUFFIX_EXE " [options]\n\
\n\
        genAWGpattern" SUFFIX_EXE " [options] -s <freq> -e <freq> -n <count>\n\
		-p <period> [-r|-a <amplitude>]\n\
\n\
Options:\n\
  -h | --help           Displays this information\n\
  -t | --template       Output blank template to " TEMPLATE_FILENAME "\n\
\n\
  -d | --debug          Output debug information.\n\
  -q | --quiet          Suppress normal output.  Does not suppress debug output\n\
\n\
  -i | --input-file     Path to an input file\n\
  -f | --clock-freq     MHz. Sets the target sample clock on the AWG\n\
\n\
Command Line Pulse Specification:\n\
  WARNING: " ANY_ALL_TEXT "\
  -s | --start-freq     MHz. Lowest frequency in pulse\n\
  -e | --end-freq       MHz. High frequency in pulse\n\
  -n | --number-freq    Total number of pulse teeth\n\
  -p | --tooth-period   ns. Time per tooth\n\
  -r | --random-amp     Randomly select amplitude for each tooth from [0.1, 1.0]\n\
  -a | --fixed-amp      Same amplitude for every tooth (range 0 to 1)\n\
\n\
\n\
Generates a file whose content is suitable for streaming directly over a serial\n\
connection to an AWG2040 to program it with a specified frequency pulse-train.\n\
Pulse parameters are read in from '" INPUT_FILENAME "' (Unless overridden by the '-i'\n\
option), OR are specified using the 'Command Line Pulse' set of options.\n\
\n\
If both '-i' and any of the 'Command Line Pulse' set are supplied, the last one\n\
will be honored.  Likewise the last of any repeated options are honored.\n";

/*! @} */


#endif
