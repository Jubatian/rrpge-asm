/**
**  \file
**  \brief     RRPGE simple assembler main file
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEv1 (version 1 of the RRPGE License):
**             see LICENSE.GPLv3 and LICENSE.RRPGEv1 in the project root.
**  \date      2014.03.24
**
**
** Short usage summary:
** rrpgeasm [input.asm]
**
** If there is no input file, it will attempt to compile "main.asm" on the
** current path. The output file name is always "app.rpa".
*/



#include "types.h"
#include "typec.h"
#include "compst.h"
#include "firead.h"
#include "pass1.h"
#include "pass2.h"
#include "pass3.h"



/* Compiler output data, for receiving data from pass1 */
static compout_t compout;


int main(int argc, char** argv)
{
 uint8     s[80];
 uint8     serrn[80];
 FILE*     fp;
 FILE*     of;
 compst_t* cst = compst_getobj();
 auint     i;
 auint     n;
 uint8     c;


 /* Welcome message (should be here) */

 printf("\nRRPGE Assembler\n");

 /* Initialize */

 compst_init(cst);

 /* Open destination file */

 of = fopen("app.rpa", "wb");
 if (of == NULL){ goto fault_ofo; }

 /* Open source file */

 if (argc > 1){
  if (firead_open((uint8 const*)(   argv[1]), cst, &fp)){ goto fault_oth; }
 }else{
  if (firead_open((uint8 const*)("main.asm"), cst, &fp)){ goto fault_oth; }
 }

 /* Pass1 */

 printf("Compilation pass1\n");
 memset(&compout, 0U, sizeof(compout));
 i = pass1_run(fp, &compout, cst);
 firead_close(fp);
 if (i){ goto fault_oth; }

 /* Pass2 */

 printf("Compilation pass2\n");
 if (pass2_run(&(compout.cons[0]), &(compout.code[0]))){ goto fault_oth; }

 /* Check generated application type & write destination */

 printf("Preparing application binary\n");
 if ((compout.conu[0xBC0U >> 5] & 0x001FU) != 0x001FU){ goto fault_una; }
 n = (((compout.cons[0xBC2U] >> 8) - 1U) & 0xFU) + 1U;
 for (i = (n << 7); i < (16U << 7); i++){
  if (compout.codu[i] != 0U){ goto fault_cex; }
 }
 for (i = 0U; i < 0x1000U; i++){
  c = (uint8)(compout.cons[i] >> 8);
  if (fwrite(&c, 1U, 1U, of) != 1U){ goto fault_wrt; }
  c = (uint8)(compout.cons[i] & 0xFFU);
  if (fwrite(&c, 1U, 1U, of) != 1U){ goto fault_wrt; }
 }
 for (i = 0U; i < (n * 0x1000U); i++){
  c = (uint8)(compout.code[i] >> 8);
  if (fwrite(&c, 1U, 1U, of) != 1U){ goto fault_wrt; }
  c = (uint8)(compout.code[i] & 0xFFU);
  if (fwrite(&c, 1U, 1U, of) != 1U){ goto fault_wrt; }
 }

 /* Pass3 */

 printf("Compilation pass3\n");
 i = ((auint)(compout.cons[0xBC2U] & 0xFFU) << 16) +
     ((auint)(compout.cons[0xBC3U] & 0xFFU)); /* Page count may trip over a 32bit system, let it pass for now */
 if (pass3_run(of, &(compout.conu[0]), i)){ goto fault_oth; }

 /* Done, try to close file and be happy */

 printf("Compilation complete\n");
 if (fclose(of)){ goto fault_ofc; }

 return 0U;


fault_wrt:

 strerror_r(errno, (char*)(&serrn[0]), 80U);
 serrn[79] = 0U;
 fclose(fp);
 snprintf((char*)(&s[0]), 80U, "Failed to write target binary: %s", (char const*)(&serrn[0]));
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 1U;

fault_cex:

 fclose(of);
 snprintf((char*)(&s[0]), 80U, "Allocated code area exceed (check 0xBC2)");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 1U;

fault_una:

 fclose(of);
 snprintf((char*)(&s[0]), 80U, "App. definition area unpopulated (0xBC0 - 0xBC4)");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 1U;

fault_ofc:

 strerror_r(errno, (char*)(&serrn[0]), 80U);
 serrn[79] = 0U;
 snprintf((char*)(&s[0]), 80U, "Failed to close \'app.rpa\': %s", (char const*)(&serrn[0]));
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 1U;

fault_ofo:

 strerror_r(errno, (char*)(&serrn[0]), 80U);
 serrn[79] = 0U;
 snprintf((char*)(&s[0]), 80U, "Failed to open \'app.rpa\': %s", (char const*)(&serrn[0]));
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 1U;

fault_oth:

 fclose(of);
 return 1U;

}
