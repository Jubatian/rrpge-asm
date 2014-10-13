/**
**  \file
**  \brief     Section & Data management
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEv2 (version 2 of the RRPGE License):
**             see LICENSE.GPLv3 and LICENSE.RRPGEv2 in the project root.
**  \date      2014.10.13
**
**  Manages the sections and their data during the compilation. Currently
**  singleton, but desinged so it is possible to extend later.
*/


#ifndef SECTION_H
#define SECTION_H


#include "types.h"


/* Section object structure */
typedef struct section_s section_t;


/* Sections supported */
#define SECT_CODE 0U
#define SECT_DATA 1U
#define SECT_HEAD 2U
#define SECT_DESC 3U
#define SECT_ZERO 4U
#define SECT_FILE 5U

/* The FILE section is special: it does not accept operations; it is meant
** to be used for adding data to the application binary. */

/* The ZERO section is special: it has no data, however it accepts operations
** normally. */


/* Error codes. OVR: Overlap. OVF: Overflow (out of range). */
#define SECT_ERR_OK   0U
#define SECT_ERR_OVR  1U
#define SECT_ERR_OVF  2U


/* Get built-in singleton object handle. */
section_t* section_getobj(void);


/* Initializes or resets a section object. */
void  section_init(section_t* hnd);


/* Sets section to perform operations on. */
void  section_setsect(section_t* hnd, auint sect);


/* Gets currently selected section. */
auint section_getsect(section_t* hnd);


/* Sets offset within section, word granularity. */
void  section_setoffw(section_t* hnd, auint off);


/* Pushes word data to the section. If necessary, aligns offset to word
** boundary first. Returns error code on failure (overlap or out of section's
** allowed size). */
auint section_pushw(section_t* hnd, auint data);


/* Pushes byte data to the section. Returns error code on failure (overlap or
** out of section's allowed size). */
auint section_pushb(section_t* hnd, auint data);


/* Changes an unit of word data at a given offset. This is meant to be used by
** second pass to substitue values which could not be resolved earlier. Will
** only have effect in areas already occupied. OR combines. */
void  section_setw(section_t* hnd, auint off, auint data);


/* Changes an unit of byte data at a given (byte) offset. This is meant to be
** used by second pass to substitue values which could not be resolved
** earlier. Will only have effect in areas already occupied. OR combines. */
void  section_setb(section_t* hnd, auint off, auint data);


/* Set section base offset. */
void  section_setbase(section_t* hnd, auint base);


/* Get aligned word address for an offset within the section (that is, base
** added to it). */
auint section_getaddr(section_t* hnd, auint off);


/* Get section's size in words. This is calculated from the last occupied word
** of the section. */
auint section_getsize(section_t* hnd);


/* Get section data pointer for reading, combining into an application binary.
** The size of the memory area can be retrieved by section_getsize(), */
uint16 const* section_getdata(section_t* hnd);


#endif
