/**
**  \file
**  \brief     Source file reader
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEv1 (version 1 of the RRPGE License):
**             see LICENSE.GPLv3 and LICENSE.RRPGEv1 in the project root.
**  \date      2014.03.23
*/


#include "firead.h"
#include "fault.h"


/* Opens source file for reading, and sets up the compile state to point at
** the start of this file, with the 0th line read in. If the file can not be
** opened, it outputs a fault accordingly. Returns 0 (FALSE) if the open was
** succesful, nonzero (TRUE) if for some error it failed. Populates the passed
** FILE pointer with the file handle, reading is at the end of the first line
** (so subsequent firead_read() calls may work with it). fp is set NULL if the
** open fails. */
auint firead_open(uint8 const* fnam, compst_t* hnd, FILE** fp)
{
 uint8  s[80];
 uint8  serrn[80];

 *fp = fopen((char const*)(fnam), "r");
 if (*fp == NULL){ goto fault_fil; }

 compst_setfile(hnd, fnam);
 compst_setline(hnd, 0U);
 compst_setcoff(hnd, 0U);

 return firead_read(hnd, *fp);

fault_fil:

 strerror_r(errno, (char*)(&serrn[0]), 80U);
 serrn[79] = 0U;
 snprintf((char*)(&s[0]), 80U, "Failed to open %s: %s", (char const*)(fnam), (char const*)(&serrn[0]));
 fault_printat(FAULT_FAIL, &s[0], hnd);
 return 1U;
}



/* Reads next line into the compile state from the given file, from current
** position. May produce fault, returns nonzero (TRUE) if so, 0 (FALSE)
** otherwise. Note that reaching or reading past the end of file is not
** considered a fault, empty lines are produced from this point. */
auint firead_read(compst_t* hnd, FILE* fp)
{
 uint8  s[80];
 uint8  serrn[80];
 uint8  t[LINE_MAX];
 uint8  c;
 auint  i = 0U;

 /* Start next line in compile state */

 compst_setline(hnd, compst_getline(hnd) + 1U);
 compst_setcoff(hnd, 0U);

 /* Read one line from the file */

 while (1){
  if (fread(&c, 1U, 1U, fp) == 0U){
   if (feof(fp)){ break; }
   goto fault_red;
  }
  if (c == (uint8)('\n')){ break; } /* Readched end of line */
  if (i < LINE_MAX){ t[i] = c; }
  i++;
 }

 /* Test for too long line (also terminating string as needed) */

 if (i >= LINE_MAX){
  snprintf((char*)(&s[0]), 80U, "Source line too long");
  fault_printat(FAULT_NOTE, &s[0], hnd);
  t[LINE_MAX - 1U] = 0U;
 }else{
  t[i] = 0U;
 }

 /* Submit new line and return */

 compst_setsstr(hnd, &(t[0]));
 return 0U;

fault_red:

 strerror_r(errno, (char*)(&serrn[0]), 80U);
 serrn[79] = 0U;
 snprintf((char*)(&s[0]), 80U, "Failed to read source: %s", (char const*)(&serrn[0]));
 fault_printat(FAULT_FAIL, &s[0], hnd);
 return 1U;
}



/* Checks end of file. Returns nonzero (TRUE) if so. Just a wrapper for an
** feof() call, but taking care for not skipping the last line. */
auint firead_iseof(compst_t* hnd, FILE* fp)
{
 uint8 const* src = compst_getsstr(hnd);
 if (src[0] != 0U){ return 0U; } /* Don't skip the last line! */
 if (feof(fp)){ return 1U; }
 return 0U;
}



/* Closes a file. Just a wrapper for an fclose() call. */
void  firead_close(FILE* fp)
{
 fclose(fp);
}
