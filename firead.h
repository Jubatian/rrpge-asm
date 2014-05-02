/**
**  \file
**  \brief     Source file reader
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEv2 (version 2 of the RRPGE License):
**             see LICENSE.GPLv3 and LICENSE.RRPGEv2 in the project root.
**  \date      2014.05.02
*/


#ifndef FIREAD_H
#define FIREAD_H


#include "types.h"
#include "compst.h"


/* Opens source file for reading, and sets up the compile state to point at
** the start of this file, with the 0th line read in. If the file can not be
** opened, it outputs a fault accordingly. Returns 0 (FALSE) if the open was
** succesful, nonzero (TRUE) if for some error it failed. Populates the passed
** FILE pointer with the file handle, reading is at the end of the first line
** (so subsequent firead_read() calls may work with it). fp is set NULL if the
** open fails. */
auint firead_open(uint8 const* fnam, compst_t* hnd, FILE** fp);


/* Reads next line into the compile state from the given file, from current
** position. May produce fault, returns nonzero (TRUE) if so, 0 (FALSE)
** otherwise. Note that reaching or reading past the end of file is not
** considered a fault, empty lines are produced from this point. */
auint firead_read(compst_t* hnd, FILE* fp);


/* Checks end of file. Returns nonzero (TRUE) if so. Just a wrapper for an
** feof() call, but taking care for not skipping the last line. */
auint firead_iseof(compst_t* hnd, FILE* fp);


/* Closes a file. Just a wrapper for an fclose() call. */
void  firead_close(FILE* fp);


#endif
