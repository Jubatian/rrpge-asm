/**
**  \file
**  \brief     Second pass logic of the assembler
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEvt (temporary version of the RRPGE
**             License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
**             root.
**  \date      2014.10.22
*/


#include "pass2.h"



/* Head autofill contents */
static const uint16 pass2_h_adat[32] = {
 ((auint)( 'R') << 8) + (auint)( 'P'),
 ((auint)( 'A') << 8) + (auint)('\n'),
 ((auint)('\n') << 8) + (auint)( 'A'),
 ((auint)( 'p') << 8) + (auint)( 'p'),
 ((auint)( 'A') << 8) + (auint)( 'u'),
 ((auint)( 't') << 8) + (auint)( 'h'),
 ((auint)( ':') << 8) + (auint)( ' '),
 ((auint)('\n') << 8) + (auint)( 'A'),
 ((auint)( 'p') << 8) + (auint)( 'p'),
 ((auint)( 'N') << 8) + (auint)( 'a'),
 ((auint)( 'm') << 8) + (auint)( 'e'),
 ((auint)( ':') << 8) + (auint)( ' '),
 ((auint)('\n') << 8) + (auint)( 'V'),
 ((auint)( 'e') << 8) + (auint)( 'r'),
 ((auint)( 's') << 8) + (auint)( 'i'),
 ((auint)( 'o') << 8) + (auint)( 'n'),
 ((auint)( ':') << 8) + (auint)( ' '),
 ((auint)('\n') << 8) + (auint)( 'E'),
 ((auint)( 'n') << 8) + (auint)( 'g'),
 ((auint)( 'S') << 8) + (auint)( 'p'),
 ((auint)( 'e') << 8) + (auint)( 'c'),
 ((auint)( ':') << 8) + (auint)( ' '),
 ((auint)('\n') << 8) + (auint)( 'D'),
 ((auint)( 'e') << 8) + (auint)( 's'),
 ((auint)( 'c') << 8) + (auint)( 'O'),
 ((auint)( 'f') << 8) + (auint)( 'f'),
 ((auint)( ':') << 8) + (auint)( ' '),
 ((auint)('\n') << 8) + (auint)( 'L'),
 ((auint)( 'i') << 8) + (auint)( 'c'),
 ((auint)( 'e') << 8) + (auint)( 'n'),
 ((auint)( 's') << 8) + (auint)( 'e'),
 ((auint)( ':') << 8) + (auint)( ' ')};

/* Head autofill offsets */
static const uint8 pass2_h_aoff[32] = {
 0x00U, 0x01U,                      /* RPA\n */
 0x02U, 0x03U, 0x04U, 0x05U, 0x06U, /* \nAppAuth: */
 0x0FU, 0x10U, 0x11U, 0x12U, 0x13U, /* \nAppName: */
 0x25U, 0x26U, 0x27U, 0x28U, 0x29U, /* \nVersion: */
 0x2FU, 0x30U, 0x31U, 0x32U, 0x33U, /* \nEngSpec: */
 0x39U, 0x3AU, 0x3BU, 0x3CU, 0x3DU, /* \nDescOff: */
 0x40U, 0x41U, 0x42U, 0x43U, 0x44U  /* \nLicense: */
};



/* Retrieves ASCII uppercase hex digit of a number */
static auint pass2_gethex(auint num, auint dig)
{
 num >>= (dig << 2);
 num  &= 0xFU;
 if (num < 10U){ return (num + (auint)('0')); }
 else          { return (num + (auint)('A') - 10U); }
}



/* Executes the second pass. Finalizes the symbol table by adding section
** bases and resolving it. Autofills the head and desc sections where
** necessary to give the appropriate application binary structure. Returns
** nonzero if failed, fault code printed. */
auint pass2_run(symtab_t* stb)
{
 uint8         s[80];
 section_t*    sec = symtab_getsectob(stb);
 auint         ssi[SECT_CNT]; /* Section sizes */
 auint         sbs[SECT_CNT]; /* Section bases */
 auint         i;
 auint         t;

 /* Autofill head and desc sections where necessary */

 /* Head elements (offset of desc remains unfilled for now) */

 section_setsect(sec, SECT_HEAD);
 for (i = 0U; i < 32U; i++){
  section_fsetw(sec, pass2_h_aoff[i], pass2_h_adat[i]);
 }
 for (i = 0U; i < 64U; i++){
  section_strpad(sec, i);
 }
 if (section_getsize(sec) == 0x45U){ /* Nothing after "License: ", add some acceptable (no license) fill */
  section_setoffw(sec, 0x45U);
  if (section_pushw(sec, ((auint)('\n') << 8) + 0x00U) != 0U){ goto fault_sec; }
 }

 /* Desc elements: Set up external stack, no input controllers, and the
 ** simplest requirement flags if the descriptor has not got these. */

 section_setsect(sec, SECT_DESC);
 i = section_getsize(sec);
 if (i < 0x09U){ section_fsetw(sec, 0x08U, 0x0000U); } /* Separate 32KWords stack */
 if (i < 0x0BU){ section_fsetw(sec, 0x0AU, 0x0000U); } /* No input controllers */
 if (i < 0x0CU){ section_fsetw(sec, 0x0BU, 0xCC00U); } /* Only has important A/V, multi-streaming */

 /* Calculate section sizes */

 for (i = 0U; i < SECT_CNT; i++){
  section_setsect(sec, i);
  ssi[i] = section_getsize(sec);
 }

 /* FILE section's size has to be calculated separately... (Will be here) */

 /* Calculate section base offsets & submit their symbols. */

 sbs[SECT_CODE] = 0U;
 sbs[SECT_DATA] = 0x40U;
 sbs[SECT_HEAD] = 0U;
 sbs[SECT_DESC] = 0U;
 sbs[SECT_ZERO] = 0x40U + ssi[SECT_DATA];
 sbs[SECT_FILE] = ssi[SECT_HEAD] + ssi[SECT_DESC] + ssi[SECT_CODE] + ssi[SECT_DATA];

 for (i = 0U; i < SECT_CNT; i++){
  section_setsect(sec, i);
  section_setbase(sec, sbs[i]);
  t = symtab_addsymdef(stb, SYMTAB_CMD_MOV, sbs[i], NULL, 0U, NULL);
  if (t == 0U){ goto fault_oth; }
  t = symtab_bind(stb, section_getsbstr(i), t);
  if (t != 0U){ goto fault_oth; }
 }

 /* Check section size constraints */

 if ((ssi[SECT_DATA] + ssi[SECT_ZERO]) > SECT_MAXRAM){ goto fault_mxr; }
 if ((ssi[SECT_HEAD] + ssi[SECT_DESC]) > 0x10000U){    goto fault_hea; }

 /* Fill in Application Descriptor elements defining the positions of sections
 ** within the binary. */

 section_setsect(sec, SECT_HEAD);
 t = ssi[SECT_HEAD]; /* Position of Application Descriptor */
 section_fsetw(sec, 0x3EU, (pass2_gethex(t, 3) << 8) + pass2_gethex(t, 2));
 section_fsetw(sec, 0x3FU, (pass2_gethex(t, 1) << 8) + pass2_gethex(t, 0));

 section_setsect(sec, SECT_DESC);
 t = sbs[SECT_FILE] + ssi[SECT_FILE]; /* Total size of the file */
 section_fsetw(sec, 0x00U, t >> 16);
 section_fsetw(sec, 0x01U, t & 0xFFFFU);
 t = ssi[SECT_HEAD] + ssi[SECT_DESC]; /* Word offset of code */
 section_fsetw(sec, 0x02U, t >> 16);
 section_fsetw(sec, 0x03U, t & 0xFFFFU);
 t = ssi[SECT_HEAD] + ssi[SECT_DESC] + ssi[SECT_CODE]; /* Word offset of data */
 section_fsetw(sec, 0x04U, t >> 16);
 section_fsetw(sec, 0x05U, t & 0xFFFFU);
 t = ssi[SECT_CODE];                  /* Count of code words */
 section_fsetw(sec, 0x06U, t & 0xFFFFU); /* (0: 64KWords) */
 t = ssi[SECT_DATA];                  /* Count of data words */
 section_fsetw(sec, 0x07U, t & 0xFFFFU);

 /* Resolve symbols */

 if (symtab_resolve(stb)){ goto fault_oth; }

 /* Done */

 return 0U;

fault_hea:

 snprintf((char*)(&s[0]), 80U, "Application Header too large (%04X words)", ssi[SECT_HEAD]);
 fault_printgen(FAULT_FAIL, &s[0]);
 return 1U;

fault_mxr:

 snprintf((char*)(&s[0]), 80U, "CPU RAM limit (%04X words) overran", SECT_MAXRAM);
 fault_printgen(FAULT_FAIL, &s[0]);
 return 1U;

fault_sec:

 snprintf((char*)(&s[0]), 80U, "Unable to autofill header");
 fault_printgen(FAULT_FAIL, &s[0]);
 return 1U;

fault_oth:

 return 1U;
}
