/*  Simple "run-time" for the "Small" Abstract Machine, using a
 *  Garbage Collector to manage local data.
 *
 *  Copyright (c) ITB CompuPhase, 2004-2005
 *
 *  This file may be freely used. No warranties of any kind.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>     /* for memset() (on some compilers) */
#if defined __WIN32__ || defined _WIN32 || defined WIN32
  #include <malloc.h>
#endif
#include "osdefs.h"     /* for _MAX_PATH */
#include "amx.h"
#include "amxgc.h"
#include "bstrlib.h"    /* Better String library from Paul Hsieh */


/* These initialization and clean-up functions are part of two "extension
 * modules" (libraries with native functions) that this run-time uses.
 */
extern int amx_ConsoleInit(AMX *amx);
extern int amx_ConsoleCleanup(AMX *amx);
extern int amx_CoreInit(AMX *amx);
extern int amx_CoredCleanp(AMX *amx);


static
void garbagecollect(AMX amx[], int number)
{
  int exp, usage, n;

  /* see whether it may be a good time to increase the table size */
  gc_tablestat(&exp, &usage);
  if (usage > 50) {
    if (gc_settable(exp+1, GC_AUTOGROW) != GC_ERR_NONE)
      fprintf(stderr, "Warning, memory low\n");
  } /* if */

  /* scan all abstract machines */
  for (n = 0; n < number; n++)
    gc_scan(&amx[n]);

  /* clean up unused references */
  gc_clean();
}

/* srun_Monitor()
 * We use the debug hook to drive the garbage collector.
 * Although this is a simple and portable interface, it is not a terribly
 * good idea to run the garbage collector from the debug hook, because:
 *  o  The debug hook can only monitor a single abstract machine, whereas
 *     you are likely to have multiple concurrent abstract machines in real
 *     projects.
 *  o  To call the garbage collector at a regular interval, monitoring the
 *     DBG_LINE opcode is the best option. However, this debug code will never
 *     be sent when the script was compiled without debug information.
 *  o  The debug hook does not consider system load, whereas you would want the
 *     garbage collection to take place especially when the system is not busy.
 *  o  The debug hook carries some overhead (though just a little).
 */
int AMXAPI srun_Monitor(AMX *amx)
{
  static int linecount;

  switch (amx->dbgcode) {
  case DBG_INIT:
    return AMX_ERR_NONE;

  case DBG_LINE:
    if (--linecount > 0)
      return AMX_ERR_NONE;
    linecount = 100;
    /* drop through */
  case DBG_RETURN:
    garbagecollect(amx, 1);
    return AMX_ERR_NONE;

  default:
    return AMX_ERR_DEBUG;
  } /* switch */
}

/* aux_LoadProgram()
 * Load a compiled Small script into memory and initialize the abstract machine.
 * This function is extracted out of AMXAUX.C.
 */
int AMXAPI aux_LoadProgram(AMX *amx, char *filename, void *memblock,
                           int AMXAPI (*amx_Debug)(AMX*))
{
  FILE *fp;
  AMX_HEADER hdr;
  int result, didalloc;

  /* open the file, read and check the header */
  if ((fp = fopen(filename, "rb")) == NULL)
    return AMX_ERR_NOTFOUND;
  fread(&hdr, sizeof hdr, 1, fp);
  amx_Align16(&hdr.magic);
  amx_Align32((uint32_t *)&hdr.size);
  amx_Align32((uint32_t *)&hdr.stp);
  if (hdr.magic != AMX_MAGIC) {
    fclose(fp);
    return AMX_ERR_FORMAT;
  } /* if */

  /* allocate the memblock if it is NULL */
  didalloc = 0;
  if (memblock == NULL) {
    if ((memblock = malloc(hdr.stp)) == NULL) {
      fclose(fp);
      return AMX_ERR_MEMORY;
    } /* if */
    didalloc = 1;
    /* after amx_Init(), amx->base points to the memory block */
  } /* if */

  /* read in the file */
  rewind(fp);
  fread(memblock, 1, (size_t)hdr.size, fp);
  fclose(fp);

  /* initialize the abstract machine */
  memset(amx, 0, sizeof *amx);
  amx_SetDebugHook(amx, amx_Debug);     /* set up the debug hook */
  result = amx_Init(amx, memblock);

  /* free the memory block on error, if it was allocated here */
  if (result != AMX_ERR_NONE && didalloc) {
    free(memblock);
    amx->base = NULL;                   /* avoid a double free */
  } /* if */

  return result;
}

/* aux_FreeProgram()
 * Clean up a program and free memory.
 * This function is extracted out of AMXAUX.C.
 */
int AMXAPI aux_FreeProgram(AMX *amx)
{
  if (amx->base!=NULL) {
    amx_Cleanup(amx);
    free(amx->base);
    memset(amx,0,sizeof(AMX));
  } /* if */
  return AMX_ERR_NONE;
}

/* aux_StrError()
 * Convert an error code to a "readable" string.
 * This function is extracted out of AMXAUX.C.
 */
char * AMXAPI aux_StrError(int errnum)
{
static char *messages[] = {
      /* AMX_ERR_NONE      */ "(none)",
      /* AMX_ERR_EXIT      */ "Forced exit",
      /* AMX_ERR_ASSERT    */ "Assertion failed",
      /* AMX_ERR_STACKERR  */ "Stack/heap collision (insufficient stack size)",
      /* AMX_ERR_BOUNDS    */ "Array index out of bounds",
      /* AMX_ERR_MEMACCESS */ "Invalid memory access",
      /* AMX_ERR_INVINSTR  */ "Invalid instruction",
      /* AMX_ERR_STACKLOW  */ "Stack underflow",
      /* AMX_ERR_HEAPLOW   */ "Heap underflow",
      /* AMX_ERR_CALLBACK  */ "No (valid) native function callback",
      /* AMX_ERR_NATIVE    */ "Native function failed",
      /* AMX_ERR_DIVIDE    */ "Divide by zero",
      /* AMX_ERR_SLEEP     */ "(sleep mode)",
      /* 13 */                "(reserved)",
      /* 14 */                "(reserved)",
      /* 15 */                "(reserved)",
      /* AMX_ERR_MEMORY    */ "Out of memory",
      /* AMX_ERR_FORMAT    */ "Invalid/unsupported P-code file format",
      /* AMX_ERR_VERSION   */ "File is for a newer version of the AMX",
      /* AMX_ERR_NOTFOUND  */ "File or function is not found",
      /* AMX_ERR_INDEX     */ "Invalid index parameter (bad entry point)",
      /* AMX_ERR_DEBUG     */ "Debugger cannot run",
      /* AMX_ERR_INIT      */ "AMX not initialized (or doubly initialized)",
      /* AMX_ERR_USERDATA  */ "Unable to set user data field (table full)",
      /* AMX_ERR_INIT_JIT  */ "Cannot initialize the JIT",
      /* AMX_ERR_PARAMS    */ "Parameter error",
    };
  if (errnum < 0 || errnum >= sizeof messages / sizeof messages[0])
    return "(unknown)";
  return messages[errnum];
}

void ExitOnError(AMX *amx, int error)
{
  if (error != AMX_ERR_NONE) {
    printf("Run time error %d: \"%s\" on line %ld\n",
           error, aux_StrError(error), (long)amx->curline);
    exit(1);
  } /* if */
}

void PrintUsage(char *program)
{
  printf("Usage: %s <filename>\n", program);
  exit(1);
}


#define VERIFY(exp)     do if (!(exp)) abort(); while(0)

/* String:string(const source[] = "") */
static cell AMX_NATIVE_CALL n_bstring(AMX *amx,cell *params)
{
  cell hstr = 0;
  char *cstr;

  amx_StrParam(amx, params[1], cstr);
  if (cstr != NULL)
    hstr = (cell)cstr2bstr(cstr);
  VERIFY( gc_mark(hstr) );
  return hstr;
}

/* String:strdup(String:source) */
static cell AMX_NATIVE_CALL n_bstrdup(AMX *amx,cell *params)
{
  cell hstr = (cell)bstrcpy((const bstring)params[1]);
  VERIFY( gc_mark(hstr) );
  return hstr;
}

/* String:strcat(String:target, String:source) */
static cell AMX_NATIVE_CALL n_bstrcat(AMX *amx,cell *params)
{
  cell hstr = params[1];
  bconcat((bstring)hstr, (const bstring)params[2]);
  return hstr;
}

/* strlen(String:source) */
static cell AMX_NATIVE_CALL n_bstrlen(AMX *amx,cell *params)
{
  return blength((const bstring)params[1]);
}

/* String:strmid(String:Source, start = 0, length = cellmax) */
static cell AMX_NATIVE_CALL n_bstrmid(AMX *amx,cell *params)
{
  cell hstr = bmidstr((const bstring)params[1], (int)params[2], (int)params[3]);
  VERIFY( gc_mark(hstr) );
  return hstr;
}

/* strtoarray(dest[], size = sizeof dest, String:source, bool:packed = false) */
static cell AMX_NATIVE_CALL n_bstrtoarray(AMX *amx,cell *params)
{
  char *cstr = bstr2cstr((const bstring)params[3], '#');
  int length = strlen(cstr) + 1;
  cell *cellptr;

  if (params[4])
    length *= sizeof(cell);
  if (params[2] >= length) {
    amx_GetAddr(amx, params[1], &cellptr);
    amx_SetString(cellptr, cstr, params[4], 0);
  } /* if */
  free(cstr);
  return 0;
}

AMX_NATIVE_INFO bstring_Natives[] = {
  { "bstring",       n_bstring },
  { "bstrdup",       n_bstrdup },
  { "bstrcat",       n_bstrcat },
  { "bstrlen",       n_bstrlen },
  { "bstrmid",       n_bstrmid },
  { "bstrtoarray",   n_bstrtoarray },
  { NULL, NULL }        /* terminator */
};


int main(int argc,char *argv[])
{
  AMX amx;
  cell ret = 0;
  int err;

  if (argc != 2)
    PrintUsage(argv[0]);        /* function "usage" aborts the program */

  /* Load the program and initialize the abstract machine. */
  err = aux_LoadProgram(&amx, argv[1], NULL, srun_Monitor);
  if (err != AMX_ERR_NONE) {
    /* try adding an extension */
    char filename[_MAX_PATH];
    strcpy(filename, argv[1]);
    strcat(filename, ".amx");
    err = aux_LoadProgram(&amx, filename, NULL, srun_Monitor);
    if (err != AMX_ERR_NONE)
      PrintUsage(argv[0]);
  } /* if */

  /* Initialize two core extension modules (more extension modules may be
   * loaded & initialized automatically as DLLs or shared libraries.
   */
  amx_ConsoleInit(&amx);
  amx_CoreInit(&amx);
  /* register functions for the bstring library */
  err = amx_Register(&amx, bstring_Natives, -1);
  ExitOnError(&amx, err);

  /* Initialize the garbage collector, start with a small table. */
  gc_setcallback((GC_FREE)bdestroy);
  err = gc_settable(7, GC_AUTOGROW);
  ExitOnError(&amx, err);

  /* Run the compiled script and time it. The "sleep" instruction causes the
   * abstract machine to return in a "restartable" state (it restarts from
   * the point that it stopped. This enables for a kind of light-weight
   * cooperative multi-tasking. As native functions (or the debug hook) can
   * also force an abstract machine to "sleep", you can also use it to create
   * "latent functions": functions that allow the host application to continue
   * processing, and/or other abstract machines to run, while they wait for
   * some resource.
   * In this example, there are no other abstract machines (there is just one)
   * and this "host" program has nothing else to do than run the abstract
   * machine. So if it detects a "sleep" it just restarts the abstract machine
   * immediately.
   */
  err = amx_Exec(&amx, &ret, AMX_EXEC_MAIN, 0);
  while (err == AMX_ERR_SLEEP)
    err = amx_Exec(&amx, &ret, AMX_EXEC_CONT, 0);
  ExitOnError(&amx, err);

  /* Free the compiled script and resources. This also unloads and DLLs or
   * shared libraries that were registered automatically by amx_Init().
   */
  aux_FreeProgram(&amx);

  /* Free the garbarge collector data and tables. */
  gc_settable(0);

  /* Print the return code of the compiled script (often not very useful). */
  if (ret!=0)
    printf("\nReturn value: %ld\n", (long)ret);

  #if defined AMX_TERMINAL
    /* This is likely a graphical terminal, which should not be closed
     * automatically
     */
    {
      extern int amx_termctl(int,int);
      while (amx_termctl(4,0))
        /* nothing */;
    }
  #endif

  return 0;
}
