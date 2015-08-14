
/*! @file templateContents.h
 * @brief Content of the template file printed with the -t or --template flags
 *
 * Contained here in its entirety to allow easy editing in the future.
 */

 //! The full template file contents.
 /*!
  * The template file will be written to disk when the full program is called with the -t or --template flags.
  * This gives an example of how to specify the output pulses, as well as a description of how it will actually
  * work internally.
  */
const char          templateStr[] = "# Lines starting with '#' are comments\n\
# All other lines should be in the following format\n\
# freq [MHz], duration [ns], amplitude [relative, [0,1] ]\n\
# durations are a goal, not a guarantee, will be rounded to nearest 1/2 cycle of freq (including 0!)\n\
# amplitudes relative scales, where 1 is full-scale.\n\
# Output is only 8-bit, so effective amplitude resolution is 1/127 ~ 0.008\n\
#\n\
# Example line of 111 MHz for 30ns, with 3/4 full scale amplitude\n\
# 100, 30, 0.75\n\
";
