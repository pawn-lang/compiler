/*  Command-line shell for the "Small" Abstract Machine.
 *
 *  Copyright (c) ITB CompuPhase, 2001-2005
 *
 *  This file may be freely used. No warranties of any kind.
 */
#include <stdio.h>
#include <stdlib.h>     /* for exit() */
#include <string.h>     /* for memset() (on some compilers) */
#include "amx.h"

size_t srun_ProgramSize(char *filename)
{
  FILE *fp;
  AMX_HEADER hdr;

  if ((fp=fopen(filename,"rb")) == NULL)
    return 0;
  fread(&hdr, sizeof hdr, 1, fp);
  fclose(fp);

  amx_Align16(&hdr.magic);
  amx_Align32((unsigned long*)&hdr.stp);
  return (hdr.magic==AMX_MAGIC) ? (size_t)hdr.stp : 0;
}

int srun_LoadProgram(AMX *amx, char *filename, void *memblock)
{
  FILE *fp;
  AMX_HEADER hdr;

  if ((fp = fopen(filename, "rb")) == NULL )
    return AMX_ERR_NOTFOUND;
  fread(&hdr, sizeof hdr, 1, fp);
  amx_Align32((unsigned long *)&hdr.size);
  rewind(fp);
  fread(memblock, 1, (size_t)hdr.size, fp);
  fclose(fp);

  memset(amx, 0, sizeof *amx);
  return amx_Init(amx, memblock);
}

void ErrorExit(AMX *amx, int errorcode)
{
  printf("Run time error %d: \"%s\" on line %ld\n",
         errorcode, amx_StrError(errorcode),
         (amx != NULL) ? amx->curline : 0);
  exit(1);
}

void PrintUsage(char *program)
{
  printf("Usage: %s <filename>\n", program);
  exit(1);
}

int main(int argc,char *argv[])
{
  extern int amx_ConsoleInit(AMX *amx);
  extern int amx_ConsoleCleanup(AMX *amx);
  extern int amx_CoreInit(AMX *amx);
  extern int amx_CoredCleanp(AMX *amx);

  size_t memsize;
  void *program;
  AMX amx;
  int index, err;
  cell amx_addr, *phys_addr;
  char output[128];

  if (argc != 4)
    PrintUsage(argv[0]);
  if ((memsize = srun_ProgramSize(argv[1])) == 0)
    PrintUsage(argv[0]);

  program = malloc(memsize);
  if (program == NULL)
    ErrorExit(NULL, AMX_ERR_MEMORY);

  err = srun_LoadProgram(&amx, argv[1], program);
  if (err)
    ErrorExit(&amx, err);

  amx_ConsoleInit(amx);
  err = amx_CoreInit(amx);
  if (err)
    ErrorExit(&amx, err);

  err = amx_FindPublic(&amx, argv[2], &index);
  if (err)
    ErrorExit(&amx, err);

  err = amx_Allot(&amx, strlen(argv[3]) + 1, &amx_addr, &phys_addr);
  if (err)
    ErrorExit(&amx, err);
  amx_SetString(phys_addr, argv[3], 0);

  err = amx_Exec(&amx, NULL, index, 1, amx_addr);
  if (err)
    ErrorExit(&amx, err);

  amx_GetString(output, phys_addr);
  amx_Release(&amx, amx_addr);
  printf("%s returns %s\n", argv[1], output);

  amx_ConsoleCleanup(&amx);
  amx_CoreCleanup(&amx);
  amx_Cleanup(&amx);
  free(program);
  return 0;
}
