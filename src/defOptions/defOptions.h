
/*! @file defOptions.h
 * @brief Functions and parameters for dealing with command-line options.
 *
 * Contains the functions used to handle parsing of input flags, printing out
 * debug information about set options, and masks used to decode flags.
 */

#ifndef DEFOPTIONS_H
#define DEFOPTIONS_H

#include <inttypes.h>
#include "filenames.h"

#ifndef DEFOPTIONS_INT_H
extern int          g_opt_debug;	//!< Whether -d/--debug has been set. 0 is unset, 1 is set.
extern int          g_opt_quiet;	//!< Whether -q/--quiet has been set. 0 is unset, 1 is set.
extern const char   helpText[];	//!< The help text to display for user-requested help.
#endif

/*!
 * @defgroup OptRetCodes Option subsystem return codes
 * @brief Return codes used internally to indicate how to respond to parsing options.
 * @{
 */

#define	OPT_RET_OK		 0	//!< Indicates successful option parsing.  Continue program operation.
#define OPT_RET_ERR		-1	//!< Indicates an error occurred while parseing options.  Exit with non-zero code.
#define OPT_RET_EXIT	-2   //!< Indicates program should exit normally without further activity.

/*! @} */

/*!
 * @defgroup OptMasks Option encoding bitmasks
 * @brief Bit masks for encoding the flags for the various options that can be set on the command line.
 *
 * Options these flags refer to are stored in #progOptions::flags
 * @{
 */

// No argument option bit masks
#define OPT_TEMPLATE_MASK	(1u << 0)	//!< Flag for requested template file output. 0 is unset, 1 is set.
#define OPT_RANDAMP_MASK	(1u << 1)	//!< Flag for using random amplitudes for output. 0 is unset, 1 is set.
#define OPT_HELPREQ_MASK	(1u << 2)	//!< Flag for user-requested help. 0 is unset, 1 is set.
// Are we setting input from command line bit mask
#define OPT_FROMCMD_MASK	(1u << 15)	//!< Flag indicating user input frequency specification via command-line options. 0 is unset, 1 is set.
// Track if we've set all parameters bit masks
#define OPT_STARTSET_MASK	(1u << 8)	//!< Flag indicating user specified a start frequency, found in #progOptions::start_f. 0 is unset, 1 is set.
#define OPT_STOPSET_MASK	(1u << 9)	//!< Flag indicating user specified a stop frequency, found in #progOptions::stop_f. 0 is unset, 1 is set.
#define OPT_AMPSET_MASK		(1u << 10)	//!< Flag indicating user specified an amplitude. If #OPT_RANDAMP_MASK is not also set, found in #progOptions::amplitude. 0 is unset, 1 is set.
#define OPT_PERIODSET_MASK	(1u << 11)	//!< Flag indicating pulse duration is set, found in #progOptions::tooth_period. 0 is unset, 1 is set.
#define OPT_NUMSET_MASK		(1u << 12)	//!< Flag indicating the number of teeth is set, found in #progOptions::num_f. 0 is unset, 1 is set.
#define OPT_ALLSET_MASK		( OPT_STARTSET_MASK | OPT_STOPSET_MASK | OPT_AMPSET_MASK | OPT_PERIODSET_MASK | OPT_NUMSET_MASK )	//!< Pre-combined set of flags for checking if all needed command line options are set.

/*! @} */

/*! @brief Structure to hold values for command-line options.
 *
 * Expected initialization found in #OPT_INIT_VAL
 * @sa helpText
 * @sa progOptions
 * 
 */
typedef struct progOptions {
    uint32_t            flags;	//!< Bit flags used to indicate on-off states for various options.  See \ref OptMasks.
    double              amplitude;	//!< Amplitude chosen for combs with constant output-amplitude. In range [0, 1] (fraction of max).
    double              start_f;	//!< Start frequency, sets the lowest frequency to be output in the series of pulses. In MHz.
    double              stop_f;	//!< Stop frequency, sets the highest frequency to be output in the series of pulses. In MHz.
    unsigned int        num_f;	//!< The number of individual pulse to generate.
    double              clock_freq;	//!< The sample output frequency. In MHz.
    double              tooth_period;	//!< The length of each pulse. In ns.
    char               *inputPath;	//!< C-string for a command-line specified frequency specification file path.
} progOptions_type;

#define OPT_INIT_VAL {0, 0.0, 0.0, 0.0, 0, 1024.0, 0.0, NULL}	//!< Initialization data for a #progOptions instantiation.

/*! @brief Takes command-line arguments and parses them
 *	
 *  Uses getopt_long() to process long & short flags passed in to the command line into appropriate settings.
 *  The flags and their values are described in #helpText
 *
 *  If --debug is set, it will begin printing out options as they are set,
 *  and will print out the full options array via #printOptions() when processing is complete.
 *
 *  @param[in] argc argc as passed in to main()
 *  @param[in] argv argv as passed in to main()
 *  @param[out] options Pointer to the structure containing the set options from the command line input.
 *  Must be appropriately allocated before calling this function.
 *  @return One of the values defined in @ref OptRetCodes.
 */
int                 parseOptions(
    int argc,
    char *argv[],
    progOptions_type * options
);

/*! @brief Takes a progOptions structure and prints a formatted version to stdout.
 *	
 *  The formatted output includes the values of any numeric option,
 *  as well as the decoded contents of the #progOptions::flags field via calls to #printBitSetting().
 *
 *  @param[in] toPrint The options whose setting will be printed
 *  @param[in] idStr A pointer to a string containing the name toPrint will be referred to on screen.
 *  If a Null pointer is passed, the display will default to "options"
 */
void                printOptions(
    const progOptions_type * toPrint,
    const char *idStr
);

/*! @brief Takes a bitfield and mask and prints the selected bit's state to stdout.
 *	
 *  Prints out the state as 'set' or 'unset', along with an optional title explaining what
 *  has been decoded.
 *
 *  @warning Will happily accept a mask with multiple bits set,
 *  but will report only the state of the highest set bit.
 *
 *  @param[in] flags A bitfield with an encoded value
 *  @param[in] mask The bitmask for the value to decode
 *  @param[in] title The name to print as a label for the decoded value.
 *  If a Null pointer is passed it will list only the bit number.
 */
void                printBitSetting(
    uint32_t flags,
    unsigned int mask,
    const char *title
);

#endif
