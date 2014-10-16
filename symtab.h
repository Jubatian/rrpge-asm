/**
**  \file
**  \brief     Symbol table
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEv2 (version 2 of the RRPGE License):
**             see LICENSE.GPLv3 and LICENSE.RRPGEv2 in the project root.
**  \date      2014.10.16
**
**  Manages the symbol table: creates entries, and for pass2, resolves those.
**  Currently singleton, but designed so it is possible to extend later.
**
**  Symbols in the source are identified by strings: the symbol table supports
**  the resolution of these indexed by the strings. Internally however the
**  table uses ID values to index symbols, which is used for implicit symbol
**  definitions (such as parts of an expression).
**
**  The component is built so it may work with arbitrary (fixed) sizes,
**  however for the singleton, these sizes are fixed and are controlled by the
**  size definitions.
**
**  Section base offsets are meant to be applied using special symbols added
**  before resolution. The suggested names for these: '$.code', '$.data',
**  '$.head', '$.desc', '$.zero' and '$.file'.
*/


#ifndef SYMTAB_H
#define SYMTAB_H


#include "types.h"
#include "section.h"
#include "compst.h"
#include "valwr.h"


/* Symbol table data structure */
typedef struct symtab_s symtab_t;


/* Maximal number of symbol definitions. */
#define SYMTAB_DEF_SIZE 32768U
/* Maximal number of symbol usage entries. */
#define SYMTAB_USE_SIZE 16384U
/* String size for collecting symbol names. */
#define SYMTAB_STR_SIZE (24U * SYMTAB_USE_SIZE)

/* Symbol definition command: Source 0 is name (s0n) for other symbol flag. */
#define SYMTAB_CMD_S0N  0x4000U
/* Symbol definition command: Source 0 is ID (s0v) for other symbol flag. */
#define SYMTAB_CMD_S0I  0x1000U
/* Symbol definition command: Source 1 is name (s1n) for other symbol flag. */
#define SYMTAB_CMD_S1N  0x8000U
/* Symbol definition command: Source 1 is ID (s1v) for other symbol flag. */
#define SYMTAB_CMD_S1V  0x2000U

/* Symbol definition command: Use only first source. */
#define SYMTAB_CMD_MOV  0x00U
/* Symbol definition command: Add sources. */
#define SYMTAB_CMD_ADD  0x01U
/* Symbol definition command: Subtract: Sr0 - Sr1. */
#define SYMTAB_CMD_SUB  0x02U
/* Symbol definition command: Multiply sources. */
#define SYMTAB_CMD_MUL  0x03U
/* Symbol definition command: Divide: Sr0 / Sr1. */
#define SYMTAB_CMD_DIV  0x04U
/* Symbol definition command: Modulo: Sr0 % Sr1. */
#define SYMTAB_CMD_MOD  0x05U
/* Symbol definition command: AND: Sr0 & Sr1. */
#define SYMTAB_CMD_AND  0x06U
/* Symbol definition command: OR: Sr0 | Sr1. */
#define SYMTAB_CMD_OR   0x07U
/* Symbol definition command: XOR: Sr0 ^ Sr1. */
#define SYMTAB_CMD_XOR  0x08U
/* Symbol definition command: Right shift: Sr0 >> Sr1. */
#define SYMTAB_CMD_SHR  0x09U
/* Symbol definition command: Left shift: Sr0 << Sr1. */
#define SYMTAB_CMD_SHL  0x0AU



/* Get built-in singleton object handle. */
symtab_t* symtab_getobj(void);


/* Initialize or resets a symbol table object (size is unchanged). The given
** section and compile state object is bound to the symbol table. */
void  symtab_init(symtab_t* hnd, section_t* sec, compst_t* cst);


/* Add new symbol definition to the table. Prints fault if the symbol
** definition can not be added. Returns ID of the definition (nonzero), or
** zero (!) if it was not possible to add. Parameters: 'cmd': The command to
** use for generating the value of the symbol, 's0v', 's0n': Source 0, 's1v',
** 's1n': Source 1. The name is used only if 'cmd' requests so (then a symbol
** usage entry is generated). */
auint symtab_addsymdef(symtab_t* hnd, auint cmd,
                       auint s0v, uint8 const* s0n,
                       auint s1v, uint8 const* s1n);


/* Gets a symbol definition by symbol name. If the symbol definition does not
** exists, it creates a "dangling" definition referring the given name, and
** returns that. Returns ID of the definition (nonzero), or zero (!) if it is
** not possible to do this (fault code printed). */
auint symtab_getsymdef(symtab_t* hnd, uint8 const* nam);


/* Add symbol name string binding to a symbol definition. Prints fault if it
** can not be done. Returns nonzero on failure (symbol name space exhausted
** or redefinition). The name terminates with a white character, and does not
** need to be preserved after addition. */
auint symtab_bind(symtab_t* hnd, uint8 const* nam, auint id);


/* Add symbol usage. The 'off' parameter supplies the offset within the
** section where the symbol will have to be resolved. The 'use' parameter
** gives the usage as defined in valwr.h. A symbol definition has to be passed
** to this function, however it is possible to submit usage for a not-yet
** defined symbol by creating a "dangling" definition (with a MOV command,
** symbol name source) to bind to. Returns nonzero and prints fault if it is
** not possible to add this. */
auint symtab_use(symtab_t* hnd, auint def, auint off, auint use);


/* Resolves the symbol table into the bound section. Prints fault and returns
** nonzero if it is not possible to resolve. */
auint symtab_resolve(symtab_t* hnd);


#endif
