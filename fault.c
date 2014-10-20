/**
**  \file
**  \brief     Fault message handling
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEvt (temporary version of the RRPGE
**             License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
**             root.
**  \date      2014.10.20
*/


#include "fault.h"
#include "strpr.h"



/* Prints out a failure message, sev is the severity, dsc is the reason of
** failure, off is it's offset. */
void fault_print(auint sev, uint8 const* dsc, fault_off_t const* off)
{
 if      (sev == FAULT_NOTE){ printf("Note ..: "); }
 else if (sev == FAULT_WARN){ printf("Warning: "); }
 else                       { printf("Error .: "); }

 printf("%s\n", (char const*)(dsc));

 printf("File ..: %s\n", (char const*)(off->fil));
 printf("At ....: Line %d, Character %d\n", off->lin, off->chr);
}



/* Prints out a failure message for the current offset. */
void fault_printat(auint sev, uint8 const* dsc, compst_t* hnd)
{
 fault_off_t cur;
 cur.fil = compst_getfile(hnd);
 cur.lin = compst_getline(hnd);
 cur.chr = compst_getcoff(hnd);
 fault_print(sev, dsc, &cur);
}


/* Prints out source independent fault (use if it is not possible to
** determine the source of the problem, but don't prefer it) */
void fault_printgen(auint sev, uint8 const* dsc)
{
 fault_off_t cur;
 cur.fil = (uint8 const*)("<no file>");
 cur.lin = 0U;
 cur.chr = 0U;
 fault_print(sev, dsc, &cur);
}


/* Deep copies a fault offset. It also takes in the target file name string's
** location where it will save the file name. */
void fault_fofcopy(fault_off_t* dst, fault_off_t const* src, uint8* fil)
{
 strpr_copy(fil, src->fil, FILE_MAX);
 dst->fil = fil;
 dst->lin = src->lin;
 dst->chr = src->chr;
}



/* Retrieves a fault offset from the current location */
void fault_fofget(fault_off_t* dst, compst_t* src, uint8* fil)
{
 fault_off_t cur;
 cur.fil = compst_getfile(src);
 cur.lin = compst_getline(src);
 cur.chr = compst_getcoff(src);
 fault_fofcopy(dst, &cur, fil);
}
