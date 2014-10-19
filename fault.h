/**
**  \file
**  \brief     Fault message handling
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEvt (temporary version of the RRPGE
**             License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
**             root.
**  \date      2014.10.19
*/


#ifndef FAULT_H
#define FAULT_H


#include "types.h"
#include "compst.h"



/* Structure for giving the offset of failure */
typedef struct{
 uint8 const* fil;   /* File in which the failure was detected */
 auint        lin;   /* Line on which the failure was detected */
 auint        chr;   /* Character offset of failure within line */
}fault_off_t;


/* Severity definitions */
#define FAULT_NOTE 0U
#define FAULT_WARN 1U
#define FAULT_FAIL 2U


/* Prints out a failure message, sev is the severity, dsc is the reason of
** failure, off is it's offset. */
void fault_print(auint sev, uint8 const* dsc, fault_off_t const* off);


/* Prints out a failure message for the current offset. */
void fault_printat(auint sev, uint8 const* dsc, compst_t* hnd);


/* Deep copies a fault offset. It also takes in the target file name string's
** location where it will save the file name. */
void fault_fofcopy(fault_off_t* dst, fault_off_t const* src, uint8* fil);


/* Retrieves a fault offset from the current location */
void fault_fofget(fault_off_t* dst, compst_t* src, uint8* fil);


#endif
