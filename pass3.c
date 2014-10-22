/**
**  \file
**  \brief     Third pass logic of the assembler.
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEvt (temporary version of the RRPGE
**             License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
**             root.
**  \date      2014.10.22
*/


#include "pass3.h"



/* Executes the third pass. This combines the application components prepared
** in pass 2 with the FILE section binary data blocks into a new application
** binary file. Returns nonzero on failure. */
auint pass3_run(FILE* obi, symtab_t* stb, bindata_t* bdt)
{
 uint8  s[80];
 uint8  e[80];
 section_t* sec = symtab_getsectob(stb);
 auint  i;
 auint  j;
 auint  ssi;
 uint16 const* d;
 uint8  c[2];

 /* Write out sections in order. The FILE section will have zero size here, so
 ** no problem including it. */

 for (i = 0U; i < SECT_CNT; i++){
  section_setsect(sec, i);
  d   = section_getdata(sec);
  ssi = section_getsize(sec);
  for (j = 0U; j < ssi; j++){
   c[0] = d[j] >> 8;
   c[1] = d[j] & 0xFFU;
   if (fwrite(&c[0], 1U, 2U, obi) != 2U){ goto fault_wrt; }
  }
 }

 /* Write out binary data */

 bindata_out(bdt, obi);

 /* Done */

 return 0U;

fault_wrt:

 strerror_r(errno, (char*)(&e[0]), 80U);
 e[79] = 0U;
 snprintf((char*)(&s[0]), 80U, "Failed to write target binary: %s", (char const*)(&e[0]));
 fault_printgen(FAULT_FAIL, &s[0]);
 return 1U;

}
