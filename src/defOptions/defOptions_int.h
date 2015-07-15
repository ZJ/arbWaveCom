/*! @file defOptions_int.h
 * @brief Sets up global flags for debug and quiet options
 */

#ifndef DEFOPTIONS_INT_H
#define DEFOPTIONS_INT_H

int g_opt_debug = 0; //!< Whether -d/--debug has been set. 0 is unset, 1 is set.
int g_opt_quiet = 0; //!< Whether -q/--quiet has been set. 0 is unset, 1 is set.

#endif