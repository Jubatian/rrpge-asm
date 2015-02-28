/**
**  \file
**  \brief     Value write-out logic
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2015, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEvt (temporary version of the RRPGE
**             License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
**             root.
**  \date      2015.02.28
*/


#ifndef VALWR_H
#define VALWR_H


#include "types.h"
#include "fault.h"
#include "compst.h"
#include "section.h"


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
/* Value usage: 6 bit immediate in JSV operand */
#define VALWR_S6   6
/* Value usage: 16 bit relative used in JMR or JFR. */
#define VALWR_R16  7
/* Value usage: 10 bit relative used in JMS */
#define VALWR_R10  8

/* Where "truncates silently" is noted, out of range values are simply bit
** masked. Other places if such is necessary warnings are raised. */


/* Attempts to write out value at a given offset in the current section. In
** 'use' supply the form by which to write out. Returns nonzero (TRUE) if some
** severe failure arises where compilation shouldn't continue. Before writing
** the value, the area should be occupied by normal section data pushes. */
auint valwr_write(section_t* dst, uint32 val, auint off, auint use, fault_off_t const* fof);


/* Like valwr_write(), but uses the current compile state for outputting
** fault messages. */
auint valwr_writecs(section_t* dst, uint32 val, auint off, auint use, compst_t* cof);


#endif
