/**
**  \file
**  \brief     Value write-out logic
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEv1 (version 1 of the RRPGE License):
**             see LICENSE.GPLv3 and LICENSE.RRPGEv1 in the project root.
**  \date      2014.03.23
*/


#ifndef VALWR_H
#define VALWR_H


#include "types.h"
#include "fault.h"
#include "compst.h"


/* Value usage: 16 bit constant for dw. Truncates silently. */
#define VALWR_C16  0
/* Value usage: 8 bit constant at low, for db. Truncates silently. */
#define VALWR_C8L  1
/* Value usage: 8 bit constant at high, for db. Truncates silently. */
#define VALWR_C8H  2
/* Value usage: 4 bit immediate in addressing mode */
#define VALWR_A4   3
/* Value usage: 16 bit immediate in addressing mode. Truncates silently. */
#define VALWR_A16  4
/* Value usage: 4 bit immediate in bit select opcode (BTS, BTC, XBS, XBC) */
#define VALWR_B4   5
/* Value usage: 12 bit page immediate used in JML and JFL */
#define VALWR_J12  6
/* Value usage: 16 bit page immediate used in JMA and JFA */
#define VALWR_J16  7
/* Value usage: 10 bit relative used in JMR */
#define VALWR_R10  8

/* Where "truncates silently" is noted, out of range values are simply bit
** masked. Other places if such is necessary warnings are raised. In the case
** of jumps, errors are raised if the target is out of the possible range of
** the jump. */

/* Usage is in the application header */
#define VALWR_APPH 0x10000U



/* Attempts to write out value at a given offset. On 'off' use VALWR_APPH for
** writing by application header logic. In 'use' supply the form by which to
** write out. When using the application header the offset is truncated to the
** low 12 bits, otherwise the low 16 bits. Returns nonzero (TRUE) if some
** severe failure arises where compilation shouldn't continue. */
auint valwr_write(uint16* dst, uint32 val, auint off, auint use, fault_off_t const* fof);


/* Like valwr_writeoff(), but uses the current compile state for offset &
** outputting fault messages. Works with word offsets (will pad to word
** boundary when use), but otherwise it does not alter the offset. */
auint valwr_writeat(uint16* dst, uint32 val, auint use, compst_t* cof);


/* Like valwr_writeoff(), but uses the current compile state for outputting
** fault messages, but not the offset. This way an earlier offset from the
** compile state may be supplied if necessary. */
auint valwr_writeatoff(uint16* dst, uint32 val, auint off, auint use, compst_t* cof);


#endif
