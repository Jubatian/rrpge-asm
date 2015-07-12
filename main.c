/**
**  \file
**  \brief     RRPGE simple assembler main file
**  \author    Sandor Zsuga (Jubatian)
**  \license   2013 - 2015, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEvt (temporary version of the RRPGE
**             License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
**             root.
**  \date      2015.07.12
**
**
** Short usage summary:
** rrpgeasm [input.asm]
**
** If there is no input file, it will attempt to compile "main.asm" on the
** current path. The output file name is always "app.rpa".
*/



#include "types.h"
#include "symtab.h"
#include "bindata.h"
#include "firead.h"
#include "pass1.h"
#include "pass2.h"
#include "pass3.h"
#include "version.h"


/* Application name string */
static char const* main_appname = "RRPGE Assembler. Version: " ASSEMBLER_VERSION;

/* Other elements */
static char const* main_appauth = "By: Sandor Zsuga (Jubatian)\n";
static char const* main_copyrig = "License: 2013 - 2015, GNU GPLv3 (version 3 of the GNU General Public\nLicense) extended as RRPGEvt (temporary version of the RRPGE License):\nsee LICENSE.GPLv3 and LICENSE.RRPGEvt in the project root.\n";



int main(int argc, char** argv)
{
 uint8      s[80];
 uint8      e[80];
 FILE*      fp;
 FILE*      of;
 compst_t*  cst = compst_getobj();
 section_t* sec = section_getobj();
 symtab_t*  stb = symtab_getobj();
 bindata_t* bdt = bindata_getobj();
 auint      t;


 /* Welcome message */

 printf("\n");
 printf("%s", main_appname);
 printf("\n\n");
 printf("%s", main_appauth);
 printf("%s", main_copyrig);
 printf("\n");

 /* Initialize */

 compst_init(cst);
 section_init(sec);
 symtab_init(stb, sec, cst);
 bindata_init(bdt);

 /* Open source file */

 if (argc > 1){
  if (firead_open((uint8 const*)(   argv[1]), cst, &fp)){ goto fault_oth; }
 }else{
  if (firead_open((uint8 const*)("main.asm"), cst, &fp)){ goto fault_oth; }
 }

 /* Pass1 */

 printf("Compilation pass1\n");
 t = pass1_run(fp, stb, bdt);
 firead_close(fp);
 if (t){ goto fault_oth; }

 /* Pass2 */

 printf("Compilation pass2\n");
 if (pass2_run(stb)){ goto fault_oth; }

 /* Pass3 */

 printf("Compilation pass3\n");
 of = fopen("app.rpa", "wb"); /* Open destination file */
 if (of == NULL){ goto fault_ofo; }
 if (pass3_run(of, stb, bdt)){ fclose(of); goto fault_oth; }

 /* Done, try to close file and be happy */

 printf("Compilation complete\n");
 if (fclose(of)){ goto fault_ofc; }

 return 0U;

fault_ofc:

 strerror_r(errno, (char*)(&e[0]), 80U);
 e[79] = 0U;
 snprintf((char*)(&s[0]), 80U, "Failed to close \'app.rpa\': %s", (char const*)(&e[0]));
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 1U;

fault_ofo:

 strerror_r(errno, (char*)(&e[0]), 80U);
 e[79] = 0U;
 snprintf((char*)(&s[0]), 80U, "Failed to open \'app.rpa\': %s", (char const*)(&e[0]));
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 1U;

fault_oth:

 return 1U;
}
