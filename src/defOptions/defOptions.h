#ifndef DEFOPTIONS_H
#define DEFOPTIONS_H

#include <inttypes.h>

#ifndef DEFOPTIONS_INT_H
extern int g_opt_debug;
extern int g_opt_quiet;
#endif

#define	OPT_RET_OK		 0
#define OPT_RET_ERR		-1
#define OPT_RET_EXIT	-2

// No argument option bit masks
#define OPT_TEMPLATE_MASK	(1u << 0)
#define OPT_RANDAMP_MASK	(1u << 1)
#define OPT_HELPREQ_MASK	(1u << 2)
// Are we setting input from command line bit mask
#define OPT_FROMCMD_MASK	(1u << 15)
// Track if we've set all parameters bit masks
#define OPT_STARTSET_MASK	(1u << 8)
#define OPT_STOPSET_MASK	(1u << 9)
#define OPT_AMPSET_MASK		(1u << 10)
#define OPT_PERIODSET_MASK	(1u << 11)
#define OPT_NUMSET_MASK		(1u << 12)
#define OPT_ALLSET_MASK		( OPT_STARTSET_MASK | OPT_STOPSET_MASK | OPT_AMPSET_MASK | OPT_PERIODSET_MASK | OPT_NUMSET_MASK )

typedef struct progOptions {
	uint32_t		flags;
	double			amplitude;
	double			start_f;
	double			stop_f;
	unsigned int	num_f;
	double			sample_period;
	double			tooth_period;
	char *			inputPath;
} progOptions_type;
#define OPT_INIT_VAL {0, 0.0, 0.0, 0.0, 0, 1000.0/1024.0, 0.0, NULL}

int parseOptions(int argc, char * argv[], progOptions_type * options);

#endif
