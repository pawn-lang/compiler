/*  Pawn compiler
 *
 *  Function and variable definition and declaration, statement parser.
 *
 *  Copyright (c) ITB CompuPhase, 1997-2006
 *
 *  This software is provided "as-is", without any express or implied warranty.
 *  In no event will the authors be held liable for any damages arising from
 *  the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *  1.  The origin of this software must not be misrepresented; you must not
 *      claim that you wrote the original software. If you use this software in
 *      a product, an acknowledgment in the product documentation would be
 *      appreciated but is not required.
 *  2.  Altered source versions must be plainly marked as such, and must not be
 *      misrepresented as being the original software.
 *  3.  This notice may not be removed or altered from any source distribution.
 *
 *  Version: $Id: sc1.c 3664 2006-11-08 12:09:25Z thiadmer $
 */

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined	__WIN32__ || defined _WIN32 || defined __MSDOS__
  #include <conio.h>
  #include <io.h>
#endif

#if defined LINUX || defined __FreeBSD__ || defined __OpenBSD__
  #include <sclinux.h>
  #include <binreloc.h> /* from BinReloc, see www.autopackage.org */
#endif

#if defined LINUX || defined __APPLE__
  #include <unistd.h>
#endif

#if defined FORTIFY
  #include <alloc/fortify.h>
#endif

#if defined __BORLANDC__ || defined __WATCOMC__
  #include <dos.h>
  static unsigned total_drives; /* dummy variable */
  #define dos_setdrive(i)       _dos_setdrive(i,&total_drives)
#elif defined _MSC_VER && defined _WIN32
  #include <direct.h>           /* for _chdrive() */
  #define dos_setdrive(i)       _chdrive(i)
  #define stricmp  _stricmp
  #define chdir    _chdir
  #define access   _access
  #define snprintf _snprintf
#endif
#if defined __BORLANDC__
  #include <dir.h>              /* for chdir() */
#elif defined __WATCOMC__
  #include <direct.h>           /* for chdir() */
#endif
#if defined __WIN32__ || defined _WIN32 || defined _Windows
  #include <windows.h>
#endif

#include "lstring.h"
#include "sc.h"
#include "version.h"

static void resetglobals(void);
static void initglobals(void);
static char *get_extension(char *filename);
static void setopt(int argc,char **argv,char *oname,char *ename,char *pname,
                   char *codepage);
static void setconfig(char *root);
static void setcaption(void);
static void about(void);
static void invalid_option(const char *opt);
static void usage(void);
static void setconstants(void);
static void setstringconstants(void);
static void parse(void);
static void dumplits(void);
static void dumpzero(int count);
static void declfuncvar(int fpublic,int fstatic,int fstock,int fconst);
static void declglb(char *firstname,int firsttag,int fpublic,int fstatic,
                    int fstock,int fconst);
static int declloc(int fstatic);
static void decl_const(int vclass);
static void decl_enum(int vclass,int fstatic);
static cell needsub(int *tag,constvalue_root **enumroot);
static void initials(int ident,int tag,cell *size,int dim[],int numdim,
                     constvalue_root *enumroot,int *explicit_init);
static cell initarray(int ident,int tag,int dim[],int numdim,int cur,
                      int startlit,int counteddim[],constvalue_root *lastdim,
                      constvalue_root *enumroot,int *errorfound);
static cell initvector(int ident,int tag,cell size,int startlit,int fillzero,
                       constvalue_root *enumroot,int *errorfound);
static cell init(int ident,int *tag,int *errorfound);
static int getstates(const char *funcname);
static void attachstatelist(symbol *sym, int state_id);
static void funcstub(int fnative);
static int newfunc(char *firstname,int firsttag,int fpublic,int fstatic,int stock);
static int declargs(symbol *sym,int chkshadow);
static void doarg(char *name,int ident,int offset,int tags[],int numtags,
                  int fpublic,int fconst,int written,int chkshadow,arginfo *arg);
static void make_report(symbol *root,FILE *log,char *sourcefile);
static void reduce_referrers(symbol *root);
static long max_stacksize(symbol *root,int *recursion);
static int testsymbols(symbol *root,int level,int testlabs,int testconst);
static void scanloopvariables(symstate **loopvars,int dowhile);
static void testloopvariables(symstate *loopvars,int dowhile,int line);
static void destructsymbols(symbol *root,int level);
static constvalue *find_constval_byval(constvalue_root *table,cell val);
static symbol *fetchlab(char *name);
static void statement(int *lastindent,int allow_decl);
static void compound(int stmt_sameline,int starttok);
static int test(int label,int parens,int invert);
static int doexpr(int comma,int chkeffect,int allowarray,int mark_endexpr,
                  int *tag,symbol **symptr,int chkfuncresult,cell *val);
static void doassert(void);
static void doexit(void);
static int doif(void);
static int dowhile(void);
static int dodo(void);
static int dofor(void);
static int doswitch(void);
static void docase(int isdefault);
static int dogoto(void);
static void dolabel(void);
static void emit_invalid_token(int expected_token,int found_token);
static regid emit_findreg(char *opname);
static int emit_getlval(int *identptr,emit_outval *p,int *islocal,
                        regid reg,int allow_char,int allow_const,
                        int store_pri,int store_alt,int *ispushed);
static int emit_getrval(int *identptr,cell *val);
static int emit_param_any_internal(emit_outval *p,int expected_tok,
                                   int allow_nonint,int allow_expr);
static void emit_param_any(emit_outval *p);
static void emit_param_integer(emit_outval *p);
static void emit_param_index(emit_outval *p,int isrange,
                             const cell *valid_values,int numvalues);
static void emit_param_nonneg(emit_outval *p);
static void emit_param_shift(emit_outval *p);
static void emit_param_data(emit_outval *p);
static void emit_param_local(emit_outval *p,int allow_ref);
static void emit_param_label(emit_outval *p);
static void emit_param_function(emit_outval *p,int isnative);
static void emit_noop(char *name);
static void emit_parm0(char *name);
static void emit_parm1_any(char *name);
static void emit_parm1_integer(char *name);
static void emit_parm1_nonneg(char *name);
static void emit_parm1_shift(char *name);
static void emit_parm1_data(char *name);
static void emit_parm1_local(char *name);
static void emit_parm1_local_noref(char *name);
static void emit_parm1_label(char *name);
static void emit_do_casetbl(char *name);
static void emit_do_case(char *name);
static void emit_do_lodb_strb(char *name);
static void emit_do_align(char *name);
static void emit_do_call(char *name);
static void emit_do_sysreq_c(char *name);
static void emit_do_sysreq_n(char *name);
static void emit_do_const(char *name);
static void emit_do_const_s(char *name);
static void emit_do_load_both(char *name);
static void emit_do_load_s_both(char *name);
static void emit_do_pushn_c(char *name);
static void emit_do_pushn(char *name);
static void emit_do_pushn_s_adr(char *name);
static void emit_do_load_u_pri_alt(char *name);
static void emit_do_stor_u_pri_alt(char *name);
static void emit_do_addr_u_pri_alt(char *name);
static void emit_do_push_u(char *name);
static void emit_do_push_u_adr(char *name);
static void emit_do_zero_u(char *name);
static void emit_do_inc_dec_u(char *name);
static int emit_findopcode(const char *instr);
static int isterminal(int tok);
static void doreturn(void);
static void dobreak(void);
static void docont(void);
static void dosleep(void);
static void dostate(void);
static void addwhile(int *ptr);
static void delwhile(void);
static int *readwhile(void);
static char *parsestringparam(int onlycheck,int *bck_litidx);
static void dopragma(void);
static void pragma_apply(symbol *sym);

typedef void (*OPCODE_PROC)(char *name);
typedef struct {
  char *name;
  OPCODE_PROC func;
} EMIT_OPCODE;

enum {
  TEST_PLAIN,           /* no parentheses */
  TEST_THEN,            /* '(' <expr> ')' or <expr> 'then' */
  TEST_DO,              /* '(' <expr> ')' or <expr> 'do' */
  TEST_OPT,             /* '(' <expr> ')' or <expr> */
};
static int lastst     = 0;      /* last executed statement type */
static int endlessloop= 0;      /* nesting level of endless loop */
static int rettype    = 0;      /* the type that a "return" expression should have */
static int skipinput  = 0;      /* number of lines to skip from the first input file */
static int optproccall = TRUE;  /* support "procedure call" */
static int verbosity  = 1;      /* verbosity level, 0=quiet, 1=normal, 2=verbose */
static int sc_reparse = 0;      /* needs 3th parse because of changed prototypes? */
static int sc_parsenum = 0;     /* number of the extra parses */
static int wq[wqTABSZ];         /* "while queue", internal stack for nested loops */
static int *wqptr;              /* pointer to next entry */
static time_t now;              /* current timestamp, for built-in constants "__time" and "__timestamp" */
static char reportname[_MAX_PATH];/* report file name */
#if !defined SC_LIGHT
  static char sc_rootpath[_MAX_PATH];
  static char *sc_documentation=NULL;/* main documentation */
#endif
#if defined	__WIN32__ || defined _WIN32 || defined _Windows
  static HWND hwndFinish = 0;
#endif

/*  "main" of the compiler
 */
#if defined __cplusplus
  extern "C"
#endif
int pc_compile(int argc, char *argv[])
{
  int entry,i,jmpcode;
  int retcode;
  char incfname[_MAX_PATH];
  char codepage[MAXCODEPAGE+1];
  FILE *binf;
  void *inpfmark;
  int lcl_packstr,lcl_needsemicolon,lcl_tabsize;
  #if !defined SC_LIGHT
    int hdrsize=0;
  #endif
  char *ptr;
  char *tname=NULL;

  /* set global variables to their initial value */
  binf=NULL;
  initglobals();
  errorset(sRESET,0);
  errorset(sEXPRRELEASE,0);
  lexinit();

  /* make sure that we clean up on a fatal error; do this before the first
   * call to error(). */
  if ((jmpcode=setjmp(errbuf))!=0)
    goto cleanup;

  /* allocate memory for fixed tables */
  inpfname=(char*)malloc(_MAX_PATH);
  if (inpfname==NULL)
    error(103);         /* insufficient memory */
  litq=(cell*)malloc(litmax*sizeof(cell));
  if (litq==NULL)
    error(103);         /* insufficient memory */

  /* inptfname may be used in error(), fill it with zeros */
  memset(inpfname,0,_MAX_PATH);

  setopt(argc,argv,outfname,errfname,incfname,codepage);
  strcpy(binfname,outfname);
  ptr=get_extension(binfname);
  if (ptr!=NULL && stricmp(ptr,".asm")==0)
    set_extension(binfname,".amx",TRUE);
  else
    set_extension(binfname,".amx",FALSE);
  /* set output names that depend on the input name */
  if (sc_listing)
    set_extension(outfname,".lst",TRUE);
  else
    set_extension(outfname,".asm",TRUE);
  if (!strempty(errfname))
    remove(errfname);   /* delete file on startup */
  else if (verbosity>0)
    setcaption();
  setconfig(argv[0]);   /* the path to the include and codepage files */
  sc_ctrlchar_org=sc_ctrlchar;
  lcl_packstr=sc_packstr;
  lcl_needsemicolon=sc_needsemicolon;
  lcl_tabsize=sc_tabsize;
  #if !defined NO_CODEPAGE
    if (!cp_set(codepage))      /* set codepage */
      error(108);               /* codepage mapping file not found */
  #endif
  /* optionally create a temporary input file that is a collection of all
   * input files
   */
  assert(get_sourcefile(0)!=NULL);  /* there must be at least one source file */
  if (get_sourcefile(1)!=NULL) {
    /* there are at least two or more source files */
    char *sname;
    FILE *ftmp,*fsrc;
    int fidx;
    ftmp=pc_createtmpsrc(&tname);
    for (fidx=0; (sname=get_sourcefile(fidx))!=NULL; fidx++) {
      unsigned char tstring[128];
      fsrc=(FILE*)pc_opensrc(sname);
      if (fsrc==NULL) {
        strcpy(inpfname,sname); /* avoid invalid filename */
        error(100,sname);
      } /* if */
      pc_writesrc(ftmp,(unsigned char*)"#file \"");
      pc_writesrc(ftmp,(unsigned char*)sname);
      pc_writesrc(ftmp,(unsigned char*)"\"\n");
      while (pc_readsrc(fsrc,tstring,arraysize(tstring))!=NULL) {
        pc_writesrc(ftmp,tstring);
      } /* while */
      pc_writesrc(ftmp,(unsigned char*)"\n");
      pc_closesrc(fsrc);
    } /* for */
    pc_closesrc(ftmp);
    strcpy(inpfname,tname);
  } else {
    strcpy(inpfname,get_sourcefile(0));
  } /* if */
  inpf_org=(FILE*)pc_opensrc(inpfname);
  if (inpf_org==NULL)
    error(100,inpfname);
  freading=TRUE;
  outf=(FILE*)pc_openasm(outfname); /* first write to assembler file (may be temporary) */
  if (outf==NULL)
    error(101,outfname);
  /* immediately open the binary file, for other programs to check */
  if (sc_asmfile || sc_listing) {
    binf=NULL;
  } else {
    binf=(FILE*)pc_openbin(binfname);
    if (binf==NULL)
      error(101,binfname);
  } /* if */
  setconstants();               /* set predefined constants and tagnames */
  for (i=0; i<skipinput; i++)   /* skip lines in the input file */
    if (pc_readsrc(inpf_org,pline,sLINEMAX)!=NULL)
      fline++;                  /* keep line number up to date */
  skipinput=fline;
  sc_status=statFIRST;
  /* do the first pass through the file (or possibly two or more "first passes") */
  sc_parsenum=0;
  inpfmark=pc_getpossrc(inpf_org);
  do {
    /* reset "defined" flag of all functions and global variables */
    reduce_referrers(&glbtab);
    delete_symbols(&glbtab,0,TRUE,FALSE);
    delete_heaplisttable();
    #if !defined NO_DEFINE
      delete_substtable();
    #endif
    resetglobals();
    sc_ctrlchar=sc_ctrlchar_org;
    sc_packstr=lcl_packstr;
    sc_needsemicolon=lcl_needsemicolon;
    sc_tabsize=lcl_tabsize;
    errorset(sRESET,0);
    /* reset the source file */
    inpf=inpf_org;
    freading=TRUE;
    pc_resetsrc(inpf,inpfmark); /* reset file position */
    fline=skipinput;            /* reset line number */
    sc_reparse=FALSE;           /* assume no extra passes */
    sc_status=statFIRST;        /* resetglobals() resets it to IDLE */
    setstringconstants();
    setfileconst(inpfname);
    if (!strempty(incfname)) {
      if (strcmp(incfname,sDEF_PREFIX)==0) {
        plungefile(incfname,FALSE,TRUE);    /* parse "default.inc" */
      } else {
        if (!plungequalifiedfile(incfname)) /* parse "prefix" include file */
          error(100,incfname);          /* cannot read from ... (fatal error) */
      } /* if */
    } /* if */
    warnstack_init();
    preprocess();                       /* fetch first line */
    parse();                            /* process all input */
    warnstack_cleanup();
    sc_parsenum++;
  } while (sc_reparse);

  /* second (or third) pass */
  if (sc_listing)
    sc_status=statSECOND;
  else
    sc_status=statWRITE;                  /* set, to enable warnings */
  state_conflict(&glbtab);

  /* write a report, if requested */
  #if !defined SC_LIGHT
    if (sc_makereport) {
      FILE *frep=stdout;
      if (!strempty(reportname))
        frep=fopen(reportname,"wb");    /* avoid translation of \n to \r\n in DOS/Windows */
      if (frep!=NULL) {
        make_report(&glbtab,frep,get_sourcefile(0));
        if (!strempty(reportname))
          fclose(frep);
      } /* if */
      if (sc_documentation!=NULL) {
        free(sc_documentation);
        sc_documentation=NULL;
      } /* if */
    } /* if */
  #endif

  sc_ctrlchar=sc_ctrlchar_org;
  sc_packstr=lcl_packstr;
  sc_needsemicolon=lcl_needsemicolon;
  sc_tabsize=lcl_tabsize;

  /*if (sc_listing)
    goto cleanup;*/
  /* write starting options (from the command line or the configuration file) */
  if (sc_listing) {
    char string[150];
    sprintf(string,"#pragma ctrlchar 0x%02x\n"
                   "#pragma pack %s\n"
                   "#pragma semicolon %s\n"
                   "#pragma tabsize %d\n",
            sc_ctrlchar,
            sc_packstr ? "true" : "false",
            sc_needsemicolon ? "true" : "false",
            sc_tabsize);
    pc_writeasm(outf,string);
  } /* if */
  setfiledirect(inpfname);

  /* ??? for re-parsing the listing file instead of the original source
   * file (and doing preprocessing twice):
   * - close input file, close listing file
   * - re-open listing file for reading (inpf)
   * - open assembler file (outf)
   */

  /* reset "defined" flag of all functions and global variables */
  reduce_referrers(&glbtab);
  delete_symbols(&glbtab,0,TRUE,FALSE);
  #if !defined NO_DEFINE
    delete_substtable();
  #endif
  resetglobals();
  errorset(sRESET,0);
  /* reset the source file */
  inpf=inpf_org;
  freading=TRUE;
  pc_resetsrc(inpf,inpfmark);   /* reset file position */
  fline=skipinput;              /* reset line number */
  lexinit();                    /* clear internal flags of lex() */
  if (sc_listing)
    sc_status=statSECOND;
  else
    sc_status=statWRITE;          /* allow to write --this variable was reset by resetglobals() */
  writeleader(&glbtab);
  setstringconstants();
  setfileconst(inpfname);
  insert_dbgfile(inpfname);
  if (!strempty(incfname)) {
    if (strcmp(incfname,sDEF_PREFIX)==0)
      plungefile(incfname,FALSE,TRUE);  /* parse "default.inc" (again) */
    else
      plungequalifiedfile(incfname);    /* parse implicit include file (again) */
  } /* if */
  warnstack_init();
  preprocess();                         /* fetch first line */
  parse();                              /* process all input */
  warnstack_cleanup();
  if (sc_listing)
    goto cleanup;
  /* inpf is already closed when readline() attempts to pop of a file */
  writetrailer();                       /* write remaining stuff */

  entry=testsymbols(&glbtab,0,TRUE,FALSE);  /* test for unused or undefined
                                             * functions and variables */
  if (!entry)
    error(13);                  /* no entry point (no public functions) */

cleanup:
  if (inpf!=NULL) {             /* main source file is not closed, do it now */
    pc_closesrc(inpf);
    inpf=NULL;
  }
  /* write the binary file (the file is already open) */
  if (!(sc_asmfile || sc_listing) && errnum==0 && jmpcode==0) {
    assert(binf!=NULL);
    pc_resetasm(outf);          /* flush and loop back, for reading */
    #if !defined SC_LIGHT
      hdrsize=
    #endif
    assemble(binf,outf);        /* assembler file is now input */
  } /* if */
  if (outf!=NULL) {
    pc_closeasm(outf,!(sc_asmfile || sc_listing));
    outf=NULL;
  } /* if */
  if (binf!=NULL) {
    pc_closebin(binf,errnum!=0);
    binf=NULL;
  } /* if */

  #if !defined SC_LIGHT
    if (errnum==0 && strempty(errfname)) {
      int recursion;
      long stacksize=max_stacksize(&glbtab,&recursion);
      int flag_exceed=FALSE;
      if (pc_amxlimit>0) {
        long totalsize=hdrsize+code_idx;
        if (pc_amxram==0)
          totalsize+=(glb_declared+pc_stksize)*sizeof(cell);
        if (totalsize>=pc_amxlimit)
          flag_exceed=TRUE;
      } /* if */
      if (pc_amxram>0 && (glb_declared+pc_stksize)*sizeof(cell)>=(unsigned long)pc_amxram)
        flag_exceed=TRUE;
      if ((sc_debug & sSYMBOLIC)!=0 || verbosity>=2 || stacksize+32>=(long)pc_stksize || flag_exceed) {
        pc_printf("Header size:       %8ld bytes\n", (long)hdrsize);
        pc_printf("Code size:         %8ld bytes\n", (long)code_idx);
        pc_printf("Data size:         %8ld bytes\n", (long)glb_declared*sizeof(cell));
        pc_printf("Stack/heap size:   %8ld bytes; ", (long)pc_stksize*sizeof(cell));
        pc_printf("estimated max. usage");
        if (recursion)
          pc_printf(": unknown, due to recursion\n");
        else if ((pc_memflags & suSLEEP_INSTR)!=0)
          pc_printf(": unknown, due to the \"sleep\" instruction\n");
        else
          pc_printf("=%ld cells (%ld bytes)\n",stacksize,stacksize*sizeof(cell));
        pc_printf("Total requirements:%8ld bytes\n", (long)hdrsize+(long)code_idx+(long)glb_declared*sizeof(cell)+(long)pc_stksize*sizeof(cell));
      } /* if */
      if (flag_exceed)
        error(106,pc_amxlimit+pc_amxram); /* this causes a jump back to label "cleanup" */
    } /* if */
  #endif

  if (get_sourcefile(1)!=NULL && tname!=NULL) {
    remove(tname);         /* the "input file" was in fact a temporary file */
    free(tname);
  } /* if */
  free(inpfname);
  free(litq);
  stgbuffer_cleanup();
  clearstk();
  assert(jmpcode!=0 || loctab.next==NULL);/* on normal flow, local symbols
                                           * should already have been deleted */
  delete_symbols(&loctab,0,TRUE,TRUE);    /* delete local variables if not yet
                                           * done (i.e. on a fatal error) */
  delete_symbols(&glbtab,0,TRUE,TRUE);
  line_sym=NULL;
  free(pc_deprecate);
  pc_deprecate=NULL;
  free(pc_recstr);
  pc_recstr=NULL;
  hashtable_term(&symbol_cache_ht);
  delete_consttable(&tagname_tab);
  delete_consttable(&libname_tab);
  delete_consttable(&sc_automaton_tab);
  delete_consttable(&sc_state_tab);
  state_deletetable();
  delete_aliastable();
  delete_pathtable();
  delete_sourcefiletable();
  delete_dbgstringtable();
  #if !defined NO_DEFINE
    delete_substtable();
  #endif
  #if !defined SC_LIGHT
    delete_docstringtable();
    free(sc_documentation);
  #endif
  delete_autolisttable();
  delete_heaplisttable();
  if (errnum!=0) {
    if (strempty(errfname))
      pc_printf("\n%d Error%s.\n",errnum,(errnum>1) ? "s" : "");
    retcode=1;
  } else if (warnnum!=0){
    if (strempty(errfname))
      pc_printf("\n%d Warning%s.\n",warnnum,(warnnum>1) ? "s" : "");
    retcode=0;          /* use "0", so that MAKE and similar tools continue */
  } else {
    retcode=jmpcode;
    if (retcode==0 && verbosity>=2)
      pc_printf("\nDone.\n");
  } /* if */
  #if defined	__WIN32__ || defined _WIN32 || defined _Windows
    if (IsWindow(hwndFinish))
      PostMessage(hwndFinish,RegisterWindowMessage("PawnNotify"),retcode,0L);
  #endif
  #if defined FORTIFY
    Fortify_ListAllMemory();
  #endif
  return retcode;
}

#if defined __cplusplus
  extern "C"
#endif
int pc_addconstant(char *name,cell value,int tag)
{
  errorset(sFORCESET,0);        /* make sure error engine is silenced */
  sc_status=statIDLE;
  add_constant(name,value,sGLOBAL,tag);
  return 1;
}

#if defined __cplusplus
  extern "C"
#endif
int pc_addtag(char *name)
{
  cell val;
  constvalue *ptr;
  int last,tag;

  if (name==NULL) {
    /* no tagname was given, check for one */
    if (lex(&val,&name)!=tLABEL) {
      lexpush();
      return 0;         /* untagged */
    } /* if */
  } /* if */

  assert(strchr(name,':')==NULL); /* colon should already have been stripped */
  last=0;
  ptr=tagname_tab.first;
  while (ptr!=NULL) {
    tag=(int)(ptr->value & TAGMASK);
    if (strcmp(name,ptr->name)==0)
      return tag;       /* tagname is known, return its sequence number */
    tag &= (int)~FIXEDTAG;
    if (tag>last)
      last=tag;
    ptr=ptr->next;
  } /* while */

  /* tagname currently unknown, add it */
  tag=last+1;           /* guaranteed not to exist already */
  if (isupper(*name))
    tag |= (int)FIXEDTAG;
  append_constval(&tagname_tab,name,(cell)tag,0);
  return tag;
}

static void resetglobals(void)
{
  /* reset the subset of global variables that is modified by the first pass */
  curfunc=NULL;         /* pointer to current function */
  lastst=0;             /* last executed statement type */
  pc_nestlevel=0;       /* number of active (open) compound statements */
  rettype=0;            /* the type that a "return" expression should have */
  litidx=0;             /* index to literal table */
  stgidx=0;             /* index to the staging buffer */
  sc_labnum=0;          /* top value of (internal) labels */
  staging=FALSE;        /* true if staging output */
  declared=0;           /* number of local cells declared */
  glb_declared=0;       /* number of global cells declared */
  code_idx=0;           /* number of bytes with generated code */
  ntv_funcid=0;         /* incremental number of native function */
  curseg=0;             /* 1 if currently parsing CODE, 2 if parsing DATA */
  freading=FALSE;       /* no input file ready yet */
  fline=0;              /* the line number in the current file */
  fnumber=0;            /* the file number in the file table (debugging) */
  fcurrent=0;           /* current file being processed (debugging) */
  sc_intest=FALSE;      /* true if inside a test */
  pc_sideeffect=FALSE;  /* true if an expression causes a side-effect */
  pc_ovlassignment=FALSE;/* true if an expression contains an overloaded assignment */
  stmtindent=0;         /* current indent of the statement */
  indent_nowarn=FALSE;  /* do not skip warning "217 loose indentation" */
  sc_allowtags=TRUE;    /* allow/detect tagnames */
  sc_status=statIDLE;
  sc_allowproccall=FALSE;
  pc_addlibtable=TRUE;  /* by default, add a "library table" to the output file */
  sc_alignnext=FALSE;
  pc_docexpr=FALSE;
  free(pc_deprecate);
  pc_deprecate=NULL;
  sc_curstates=0;
  pc_memflags=0;
  pc_naked=FALSE;
  pc_retexpr=FALSE;
  pc_attributes=0;
  pc_loopcond=0;
  emit_flags=0;
  emit_stgbuf_idx=-1;
}

static void initglobals(void)
{
  resetglobals();

  sc_asmfile=FALSE;      /* do not create .ASM file */
  sc_listing=FALSE;      /* do not create .LST file */
  skipinput=0;           /* number of lines to skip from the first input file */
  sc_ctrlchar=CTRL_CHAR; /* the escape character */
  litmax=sDEF_LITMAX;    /* current size of the literal table */
  litgrow=sDEF_LITMAX;   /* amount to increase the literal table by */
  errnum=0;              /* number of errors */
  warnnum=0;             /* number of warnings */
  optproccall=TRUE;      /* support "procedure call" */
  verbosity=1;           /* verbosity level, no copyright banner */
  sc_debug=sCHKBOUNDS;   /* by default: bounds checking+assertions */
  pc_optimize=sOPTIMIZE_NOMACRO;
  sc_packstr=FALSE;      /* strings are unpacked by default */
  #if AMX_COMPACTMARGIN > 2
    sc_compress=TRUE;    /* compress output bytecodes */
  #else
    sc_compress=FALSE;
  #endif
  sc_needsemicolon=FALSE;   /* semicolon required to terminate expressions? */
  sc_dataalign=sizeof(cell);
  pc_stksize=sDEF_AMXSTACK; /* default stack size */
  pc_amxlimit=0;         /* no limit on size of the abstract machine */
  pc_amxram=0;           /* no limit on data size of the abstract machine */
  sc_tabsize=8;          /* assume a TAB is 8 spaces */
  sc_rationaltag=0;      /* assume no support for rational numbers */
  rational_digits=0;     /* number of fractional digits */

  outfname[0]='\0';      /* output file name */
  errfname[0]='\0';      /* error file name */
  inpf=NULL;             /* file read from */
  inpfname=NULL;         /* pointer to name of the file currently read from */
  outf=NULL;             /* file written to */
  litq=NULL;             /* the literal queue */
  glbtab.next=NULL;      /* clear global variables/constants table */
  loctab.next=NULL;      /*   "   local      "    /    "       "   */
  hashtable_init(&symbol_cache_ht, sizeof(symbol *),(16384/3*2),NULL); /* 16384 slots */
  tagname_tab.first=tagname_tab.last=NULL; /* tagname table */
  libname_tab.first=libname_tab.last=NULL; /* library table (#pragma library "..." syntax) */

  pline[0]='\0';         /* the line read from the input file */
  lptr=NULL;             /* points to the current position in "pline" */
  curlibrary=NULL;       /* current library */
  inpf_org=NULL;         /* main source file */

  wqptr=wq;              /* initialize while queue pointer */
  reportname[0]='\0';    /* report file name */

#if !defined SC_LIGHT
  sc_documentation=NULL;
  sc_makereport=FALSE;   /* do not generate a cross-reference report */
#endif
}

static char *get_extension(char *filename)
{
  char *ptr;

  assert(filename!=NULL);
  ptr=strrchr(filename,'.');
  if (ptr!=NULL) {
    /* ignore extension on a directory or at the start of the filename */
    if (strchr(ptr,DIRSEP_CHAR)!=NULL || ptr==filename || *(ptr-1)==DIRSEP_CHAR)
      ptr=NULL;
  } /* if */
  return ptr;
}

/* set_extension
 * Set the default extension, or force an extension. To erase the
 * extension of a filename, set "extension" to an empty string.
 */
SC_FUNC void set_extension(char *filename,char *extension,int force)
{
  char *ptr;

  assert(extension!=NULL && (*extension=='\0' || *extension=='.'));
  assert(filename!=NULL);
  ptr=get_extension(filename);
  if (force && ptr!=NULL)
    *ptr='\0';          /* set zero terminator at the position of the period */
  if (force || ptr==NULL)
    strcat(filename,extension);
}

static const char *option_value(const char *optptr)
{
  return (*(optptr+1)=='=' || *(optptr+1)==':') ? optptr+2 : optptr+1;
}

static int toggle_option(const char *optptr, int option)
{
  switch (*option_value(optptr)) {
  case '\0':
    option=TRUE;
    break;
  case '-':
    option=FALSE;
    break;
  case '+':
    option=TRUE;
    break;
  default:
    invalid_option(optptr);
  } /* switch */
  return option;
}

/* Parsing command line options is indirectly recursive: parseoptions()
 * calls parserespf() to handle options in a a response file and
 * parserespf() calls parseoptions() at its turn after having created
 * an "option list" from the contents of the file.
 */
static void parserespf(char *filename,char *oname,char *ename,char *pname,
                       char *codepage);

static void parseoptions(int argc,char **argv,char *oname,char *ename,char *pname,
                         char *codepage)
{
  char str[_MAX_PATH],*name;
  const char *ptr;
  int arg,i,isoption;

  for (arg=1; arg<argc; arg++) {
    #if DIRSEP_CHAR=='/'
      isoption= argv[arg][0]=='-';
    #else
      isoption= argv[arg][0]=='/' || argv[arg][0]=='-';
    #endif
    if (isoption) {
      ptr=&argv[arg][1];
      switch (*ptr) {
      case 'A':
        i=atoi(option_value(ptr));
        if ((i % sizeof(cell))==0)
          sc_dataalign=i;
        else
          invalid_option(ptr);
        break;
      case 'a':
        if (*(ptr+1)!='\0')
          invalid_option(ptr);
        sc_asmfile=TRUE;        /* skip last pass of making binary file */
        if (verbosity>1)
          verbosity=1;
        break;
      case 'C':
        #if AMX_COMPACTMARGIN > 2
          sc_compress=toggle_option(ptr,sc_compress);
        #else
          invalid_option(ptr);
        #endif
        break;
      case 'c':
        strlcpy(codepage,option_value(ptr),MAXCODEPAGE);  /* set name of codepage */
        break;
      case 'D':                 /* set active directory */
        ptr=option_value(ptr);
#if defined dos_setdrive
        if (ptr[1]==':')
          dos_setdrive(toupper(*ptr)-'A'+1);    /* set active drive */
#endif
        if (chdir(ptr)==-1)
          ; /* silently ignore chdir() errors */
        break;
      case 'd': {
        int debug;
        switch (*option_value(ptr)) {
        case '0':
          sc_debug=0;
          break;
        case '1':
          sc_debug=sCHKBOUNDS;  /* assertions and bounds checking */
          break;
        case '2':
          sc_debug=sCHKBOUNDS | sSYMBOLIC;  /* also symbolic info */
          break;
        case '3':
          sc_debug=sCHKBOUNDS | sSYMBOLIC;
          pc_optimize=sOPTIMIZE_NONE;
          /* also avoid peephole optimization */
          break;
        default:
          invalid_option(ptr);
        } /* switch */
        debug=0;
        if ((sc_debug & (sCHKBOUNDS | sSYMBOLIC))==(sCHKBOUNDS | sSYMBOLIC))
          debug=2;
        else if ((sc_debug & sCHKBOUNDS)==sCHKBOUNDS)
          debug=1;
        add_builtin_constant("debug",debug,sGLOBAL,0);
        break;
      } /* case */
      case 'e':
        if (ename)
          strlcpy(ename,option_value(ptr),_MAX_PATH); /* set name of error file */
        break;
#if defined	__WIN32__ || defined _WIN32 || defined _Windows
      case 'H':
        #if defined __64BIT__
          hwndFinish=(HWND)atoll(option_value(ptr));
        #else
          hwndFinish=(HWND)atoi(option_value(ptr));
        #endif
        if (!IsWindow(hwndFinish))
          hwndFinish=(HWND)0;
        break;
#endif
      case 'i':
        strlcpy(str,option_value(ptr),arraysize(str));  /* set name of include directory */
        i=strlen(str);
        if (i>0) {
          if (str[i-1]!=DIRSEP_CHAR) {
            str[i]=DIRSEP_CHAR;
            str[i+1]='\0';
          } /* if */
          insert_path(str);
        } /* if */
        break;
      case 'l':
        if (*(ptr+1)!='\0')
          invalid_option(ptr);
        sc_listing=TRUE;        /* skip second pass & code generation */
        break;
      case 'o':
        if (oname)
          strlcpy(oname,option_value(ptr),_MAX_PATH); /* set name of (binary) output file */
        break;
      case 'O':
        pc_optimize=*option_value(ptr) - '0';
        if (pc_optimize<sOPTIMIZE_NONE || pc_optimize>=sOPTIMIZE_NUMBER)
          invalid_option(ptr);
        break;
      case 'p':
        if (pname)
          strlcpy(pname,option_value(ptr),_MAX_PATH); /* set name of implicit include file */
        break;
      case 'R':
        pc_recursion=toggle_option(ptr,pc_recursion);
        break;
#if !defined SC_LIGHT
      case 'r':
        strlcpy(reportname,option_value(ptr),_MAX_PATH); /* set name of report file */
        sc_makereport=TRUE;
        if (!strempty(reportname)) {
          set_extension(reportname,".xml",FALSE);
        } else if ((name=get_sourcefile(0))!=NULL) {
          assert(strempty(reportname));
          assert(strlen(name)<_MAX_PATH);
          if ((ptr=strrchr(name,DIRSEP_CHAR))!=NULL)
            ptr++;          /* strip path */
          else
            ptr=name;
          assert(strlen(ptr)<_MAX_PATH);
          strcpy(reportname,ptr);
          set_extension(reportname,".xml",TRUE);
        } /* if */
        break;
#endif
      case 'S':
        i=atoi(option_value(ptr));
        if (i>32)
          pc_stksize=(cell)i;   /* stack size has minimum size */
        else
          invalid_option(ptr);
        break;
      case 's':
        skipinput=atoi(option_value(ptr));
        break;
      case 't':
        i=atoi(option_value(ptr));
        if (i>0)
          sc_tabsize=i;
        else
          invalid_option(ptr);
        break;
      case 'v':
        verbosity= isdigit(*option_value(ptr)) ? atoi(option_value(ptr)) : 2;
        if (sc_asmfile && verbosity>1)
          verbosity=1;
        break;
      case 'E':
        switch (*option_value(ptr)) {
        case '+':
          pc_seterrorwarnings(1);
          break;
        case '-':
          pc_seterrorwarnings(0);
          break;
        default:
          pc_seterrorwarnings(2);
          break;
        }
        break;
      case 'w':
        i=(int)strtol(option_value(ptr),(char **)&ptr,10);
        if (*ptr=='-')
          pc_enablewarning(i,warnDISABLE);
        else if (*ptr=='+')
          pc_enablewarning(i,warnENABLE);
        else if (*ptr=='\0')
          pc_enablewarning(i,warnTOGGLE);
        break;
      case 'X':
        if (*(ptr+1)=='D') {
          i=atoi(option_value(ptr+1));
          if (i>64)
            pc_amxram=(cell)i;  /* abstract machine data/stack has minimum size */
          else
            invalid_option(ptr);
        } else {
          i=atoi(option_value(ptr));
          if (i>64)
            pc_amxlimit=(cell)i;/* abstract machine has minimum size */
          else
            invalid_option(ptr);
        } /* if */
        break;
      case 'Z': {
        symbol *sym;
        pc_compat=toggle_option(ptr,pc_compat);
        sym=findconst("__compat",NULL);
        if (sym!=NULL) {
          assert(sym!=NULL);
          sym->addr=pc_compat;
        } /* if */
        break;
      } /* case */
      case '\\':                /* use \ instead for escape characters */
        sc_ctrlchar='\\';
        break;
      case '^':                 /* use ^ instead for escape characters */
        sc_ctrlchar='^';
        break;
      case ';':
        sc_needsemicolon=toggle_option(ptr,sc_needsemicolon);
        break;
      case '(':
        optproccall=!toggle_option(ptr,!optproccall);
        break;
      default:                  /* wrong option */
        invalid_option(ptr);
      } /* switch */
    } else if (argv[arg][0]=='@') {
      #if !defined SC_LIGHT
        parserespf(&argv[arg][1],oname,ename,pname,codepage);
      #endif
    } else if ((ptr=strchr(argv[arg],'='))!=NULL) {
      i=(int)(ptr-argv[arg]);
      if (i>sNAMEMAX) {
        i=sNAMEMAX;
        error(200,argv[arg],sNAMEMAX);  /* symbol too long, truncated to sNAMEMAX chars */
      } /* if */
      strlcpy(str,argv[arg],i+1);       /* str holds symbol name */
      i=atoi(ptr+1);
      add_builtin_constant(str,i,sGLOBAL,0);
    } else if (oname) {
      strlcpy(str,argv[arg],arraysize(str)-2); /* -2 because default extension is ".p" */
      set_extension(str,".p",FALSE);
      insert_sourcefile(str);
      /* The output name is the first input name with a different extension,
       * but it is stored in a different directory
       */
      if (strempty(oname)) {
        if ((ptr=strrchr(str,DIRSEP_CHAR))!=NULL)
          ptr++;          /* strip path */
        else
          ptr=str;
        assert(strlen(ptr)<_MAX_PATH);
        strcpy(oname,ptr);
      } /* if */
      set_extension(oname,".asm",TRUE);
#if !defined SC_LIGHT
      if (sc_makereport && strempty(reportname)) {
        if ((ptr=strrchr(str,DIRSEP_CHAR))!=NULL)
          ptr++;          /* strip path */
        else
          ptr=str;
        assert(strlen(ptr)<_MAX_PATH);
        strcpy(reportname,ptr);
        set_extension(reportname,".xml",TRUE);
      } /* if */
#endif
    } /* if */
  } /* for */
}

void parsesingleoption(char *argv)
{
  /* argv[0] is the program, which we don't need here */
  char *args[2] = { 0, argv };
  char codepage[MAXCODEPAGE+1] = { 0 };
  codepage[0] = '\0';
  parseoptions(2, args, NULL, NULL, NULL, codepage);
  /* need explicit support for codepages */
  if (codepage[0] && !cp_set(codepage))
    error(108);         /* codepage mapping file not found */
}

#if !defined SC_LIGHT
static void parserespf(char *filename,char *oname,char *ename,char *pname,
                       char *codepage)
{
#define MAX_OPTIONS     100
  FILE *fp;
  char *string, *ptr, **argv;
  int argc;
  long size;

  if ((fp=fopen(filename,"rb"))==NULL)
    error(100,filename);        /* error reading input file */
  /* load the complete file into memory */
  fseek(fp,0L,SEEK_END);
  size=ftell(fp);
  fseek(fp,0L,SEEK_SET);
  assert(size<INT_MAX);
  if ((string=(char *)malloc((int)size+1))==NULL)
    error(103);                 /* insufficient memory */
  /* fill with zeros; in MS-DOS, fread() may collapse CR/LF pairs to
   * a single '\n', so the string size may be smaller than the file
   * size. */
  memset(string,0,(int)size+1);
  if (fread(string,1,(int)size,fp)<(size_t)size)
    error(100,filename);        /* error reading input file */
  fclose(fp);
  /* allocate table for option pointers */
  if ((argv=(char **)malloc(MAX_OPTIONS*sizeof(char*)))==NULL)
    error(103);                 /* insufficient memory */
  /* fill the options table */
  ptr=strtok(string," \t\r\n");
  for (argc=1; argc<MAX_OPTIONS && ptr!=NULL; argc++) {
    /* note: the routine skips argv[0], for compatibility with main() */
    argv[argc]=ptr;
    ptr=strtok(NULL," \t\r\n");
  } /* for */
  if (ptr!=NULL)
    error(102,"option table");   /* table overflow */
  /* parse the option table */
  parseoptions(argc,argv,oname,ename,pname,codepage);
  /* free allocated memory */
  free(argv);
  free(string);
}
#endif

static void setopt(int argc,char **argv,char *oname,char *ename,char *pname,
                   char *codepage)
{
  delete_sourcefiletable(); /* make sure it is empty */
  *oname='\0';
  *ename='\0';
  *pname='\0';
  *codepage='\0';
  strcpy(pname,sDEF_PREFIX);

  #if 0 /* needed to test with BoundsChecker for DOS (it does not pass
         * through arguments) */
    insert_sourcefile("test.p");
    strcpy(oname,"test.asm");
  #endif

  #if !defined SC_LIGHT
    /* first parse a "config" file with default options */
    if (argv[0]!=NULL) {
      char cfgfile[_MAX_PATH];
      char *ext;
      strcpy(cfgfile,argv[0]);
      if ((ext=strrchr(cfgfile,DIRSEP_CHAR))!=NULL) {
        *(ext+1)='\0';          /* strip the program filename */
        strcat(cfgfile,"pawn.cfg");
      } else {
        strcpy(cfgfile,"pawn.cfg");
      } /* if */
      if (access(cfgfile,4)==0)
        parserespf(cfgfile,oname,ename,pname,codepage);
    } /* if */
  #endif
  parseoptions(argc,argv,oname,ename,pname,codepage);
  if (get_sourcefile(0)==NULL)
    about();
}

#if defined __BORLANDC__ || defined __WATCOMC__
  #pragma argsused
#endif
static void setconfig(char *root)
{
  #if defined macintosh
    insert_path(":include:");
  #else
    char path[_MAX_PATH];
    char *ptr,*base;
    int len;

    /* add the default "include" directory */
    #if defined __WIN32__ || defined _WIN32
      GetModuleFileName(NULL,path,_MAX_PATH);
    #elif defined LINUX || defined __FreeBSD__ || defined __OpenBSD__
      /* see www.autopackage.org for the BinReloc module */
      br_init_lib(NULL);
      ptr=br_find_exe("/opt/Pawn/bin/pawncc");
      strlcpy(path,ptr,arraysize(path));
      free(ptr);
    #else
      if (root!=NULL)
        strlcpy(path,root,arraysize(path)); /* path + filename (hopefully) */
    #endif
    #if defined __MSDOS__
      /* strip the options (appended to the path + filename) */
      if ((ptr=strpbrk(path," \t/"))!=NULL)
        *ptr='\0';
    #endif
    /* terminate just behind last \ or : */
    if ((ptr=strrchr(path,DIRSEP_CHAR))!=NULL || (ptr=strchr(path,':'))!=NULL) {
      /* If there is no "\" or ":", the string probably does not contain the
       * path; so we just don't add it to the list in that case
       */
      *(ptr+1)='\0';
      base=ptr;
      strcat(path,"include");
      len=strlen(path);
      path[len]=DIRSEP_CHAR;
      path[len+1]='\0';
      /* see if it exists */
      if (access(path,0)!=0 && *base==DIRSEP_CHAR) {
        /* There is no "include" directory below the directory where the compiler
         * is found. This typically means that the compiler is in a "bin" sub-directory
         * and the "include" is below the *parent*. So find the parent...
         */
        *base='\0';
        if ((ptr=strrchr(path,DIRSEP_CHAR))!=NULL) {
          *(ptr+1)='\0';
          strcat(path,"include");
          len=strlen(path);
          path[len]=DIRSEP_CHAR;
          path[len+1]='\0';
        } else {
          *base=DIRSEP_CHAR;
        } /* if */
      } /* if */
      insert_path(path);
      /* same for the codepage root */
      #if !defined NO_CODEPAGE
        if (ptr!=NULL)
          *ptr='\0';
        if (!cp_path(path,"codepage"))
          error(109,path);        /* codepage path */
      #endif
      /* also copy the root path (for the XML documentation) */
      #if !defined SC_LIGHT
        if (ptr!=NULL)
          *ptr='\0';
        strcpy(sc_rootpath,path);
      #endif
    } /* if */
  #endif /* macintosh */
}

static void setcaption(void)
{
  pc_printf("Pawn compiler " VERSION_STR "\t \t \tCopyright (c) 1997-2006, ITB CompuPhase\n\n");
}

static void about(void)
{
  usage();
  longjmp(errbuf,3);        /* user abort */
}

static void invalid_option(const char *opt)
{
  usage();
  pc_printf("\nInvalid or unsupported option: -%s\n",opt);
  longjmp(errbuf,3);        /* user abort */
}

static void usage(void)
{
  if (strempty(errfname)) {
    setcaption();
    pc_printf("Usage:   pawncc <filename> [filename...] [options]\n\n");
    pc_printf("Options:\n");
    pc_printf("         -A<num>  alignment in bytes of the data segment and the stack\n");
    pc_printf("         -a       output assembler code\n");
#if AMX_COMPACTMARGIN > 2
    pc_printf("         -C[+/-]  compact encoding for output file (default=%c)\n", sc_compress ? '+' : '-');
#endif
    pc_printf("         -c<name> codepage name or number; e.g. 1252 for Windows Latin-1\n");
    pc_printf("         -Dpath   active directory path\n");
    pc_printf("         -d<num>  debugging level (default=-d%d)\n",sc_debug);
    pc_printf("             0    no symbolic information, no run-time checks\n");
    pc_printf("             1    run-time checks, no symbolic information\n");
    pc_printf("             2    full debug information and dynamic checking\n");
    pc_printf("             3    same as -d2, but implies -O0\n");
    pc_printf("         -e<name> set name of error file (quiet compile)\n");
#if defined	__WIN32__ || defined _WIN32 || defined _Windows
    pc_printf("         -H<hwnd> window handle to send a notification message on finish\n");
#endif
    pc_printf("         -i<name> path for include files\n");
    pc_printf("         -l       create list file (preprocess only)\n");
    pc_printf("         -o<name> set base name of (P-code) output file\n");
    pc_printf("         -O<num>  optimization level (default=-O%d)\n",pc_optimize);
    pc_printf("             0    no optimization\n");
    pc_printf("             1    JIT-compatible optimizations only\n");
    pc_printf("             2    full optimizations\n");
    pc_printf("         -p<name> set name of \"prefix\" file\n");
    pc_printf("         -R[+/-]  add detailed recursion report with call chains (default=%c)\n",pc_recursion ? '+' : '-');
#if !defined SC_LIGHT
    pc_printf("         -r[name] write cross reference report to console or to specified file\n");
#endif
    pc_printf("         -S<num>  stack/heap size in cells (default=%d)\n",(int)pc_stksize);
    pc_printf("         -s<num>  skip lines from the input file\n");
    pc_printf("         -t<num>  TAB indent size (in character positions, default=%d)\n",sc_tabsize);
    pc_printf("         -v<num>  verbosity level; 0=quiet, 1=normal, 2=verbose (default=%d)\n",verbosity);
    pc_printf("         -w<num>  disable a specific warning by its number\n");
    pc_printf("         -X<num>  abstract machine size limit in bytes\n");
    pc_printf("         -XD<num> abstract machine data/stack size limit in bytes\n");
    pc_printf("         -Z[+/-]  run in compatibility mode (default=%c)\n",pc_compat ? '+' : '-');
    pc_printf("         -E[+/-]  turn warnings in to errors\n");
    pc_printf("         -\\       use '\\' for escape characters\n");
    pc_printf("         -^       use '^' for escape characters\n");
    pc_printf("         -;[+/-]  require a semicolon to end each statement (default=%c)\n", sc_needsemicolon ? '+' : '-');
    pc_printf("         -([+/-]  require parentheses for function invocation (default=%c)\n", optproccall ? '-' : '+');
    pc_printf("         sym=val  define constant \"sym\" with value \"val\"\n");
    pc_printf("         sym=     define constant \"sym\" with value 0\n");
#if defined	__WIN32__ || defined _WIN32 || defined _Windows || defined __MSDOS__
    pc_printf("\nOptions may start with a dash or a slash; the options \"-d0\" and \"/d0\" are\n");
    pc_printf("equivalent.\n");
#endif
    pc_printf("\nOptions with a value may optionally separate the value from the option letter\n");
    pc_printf("with a colon (\":\") or an equal sign (\"=\"). That is, the options \"-d0\", \"-d=0\"\n");
    pc_printf("and \"-d:0\" are all equivalent.\n");
  } /* if */
}

static void setconstants(void)
{
  int debug;
  time_t loctime;
  struct tm loctm,utctm;

  assert(sc_status==statIDLE);
  append_constval(&tagname_tab,"_",0,0);/* "untagged" */
  append_constval(&tagname_tab,"bool",BOOLTAG,0);

  add_builtin_constant("true",1,sGLOBAL,BOOLTAG);/* boolean flags */
  add_builtin_constant("false",0,sGLOBAL,BOOLTAG);
  add_builtin_constant("EOS",0,sGLOBAL,0);      /* End Of String, or '\0' */
  #if PAWN_CELL_SIZE==16
  add_builtin_constant("cellbits",16,sGLOBAL,0);
    #if defined _I16_MAX
      add_builtin_constant("cellmax",_I16_MAX,sGLOBAL,0);
      add_builtin_constant("cellmin",_I16_MIN,sGLOBAL,0);
    #else
      add_builtin_constant("cellmax",SHRT_MAX,sGLOBAL,0);
      add_builtin_constant("cellmin",SHRT_MIN,sGLOBAL,0);
    #endif
  #elif PAWN_CELL_SIZE==32
    add_builtin_constant("cellbits",32,sGLOBAL,0);
    #if defined _I32_MAX
      add_builtin_constant("cellmax",_I32_MAX,sGLOBAL,0);
      add_builtin_constant("cellmin",_I32_MIN,sGLOBAL,0);
    #else
      add_builtin_constant("cellmax",INT_MAX,sGLOBAL,0);
      add_builtin_constant("cellmin",INT_MIN,sGLOBAL,0);
    #endif
  #elif PAWN_CELL_SIZE==64
    #if !defined _I64_MIN
      #define _I64_MIN  (-9223372036854775807ULL - 1)
      #define _I64_MAX    9223372036854775807ULL
    #endif
    add_builtin_constant("cellbits",64,sGLOBAL,0);
    add_builtin_constant("cellmax",_I64_MAX,sGLOBAL,0);
    add_builtin_constant("cellmin",_I64_MIN,sGLOBAL,0);
  #else
    #error Unsupported cell size
  #endif
  add_builtin_constant("charbits",sCHARBITS,sGLOBAL,0);
  add_builtin_constant("charmin",0,sGLOBAL,0);
  add_builtin_constant("charmax",~((ucell)-1 << sCHARBITS) - 1,sGLOBAL,0);
  add_builtin_constant("ucharmax",(1 << (sizeof(cell)-1)*8)-1,sGLOBAL,0);

  add_builtin_constant("__Pawn",VERSION_INT,sGLOBAL,0);
  add_builtin_constant("__PawnBuild",VERSION_BUILD,sGLOBAL,0);
  line_sym=add_builtin_constant("__line",0,sGLOBAL,0);
  add_builtin_constant("__compat",pc_compat,sGLOBAL,0);

  now=time(NULL);
  loctm=*localtime(&now);
  utctm=*gmtime(&now);
  loctime=now+(loctm.tm_sec-utctm.tm_sec)+(loctm.tm_min-utctm.tm_min)*60
          +(loctm.tm_hour-utctm.tm_hour)*60*60+(loctm.tm_mday-utctm.tm_mday)*60*60*24;
  add_builtin_constant("__timestamp",(cell)loctime,sGLOBAL,0);

  debug=0;
  if ((sc_debug & (sCHKBOUNDS | sSYMBOLIC))==(sCHKBOUNDS | sSYMBOLIC))
    debug=2;
  else if ((sc_debug & sCHKBOUNDS)==sCHKBOUNDS)
    debug=1;
  add_builtin_constant("debug",debug,sGLOBAL,0);

  append_constval(&sc_automaton_tab,"",0,0);    /* anonymous automaton */
}

static void setstringconstants(void)
{
  char timebuf[arraysize("11:22:33")];
  char datebuf[arraysize("10 Jan 2017")];

  assert(sc_status!=statIDLE);
  add_builtin_string_constant("__file","",sGLOBAL);

  strftime(timebuf,arraysize(timebuf),"%H:%M:%S",localtime(&now));
  add_builtin_string_constant("__time",timebuf,sGLOBAL);
  strftime(datebuf,arraysize(datebuf),"%d %b %Y",localtime(&now));
  add_builtin_string_constant("__date",datebuf,sGLOBAL);
}

static int getclassspec(int initialtok,int *fpublic,int *fstatic,int *fstock,int *fconst)
{
  int tok,err;
  cell val;
  char *str;

  assert(fconst!=NULL);
  assert(fstock!=NULL);
  assert(fstatic!=NULL);
  assert(fpublic!=NULL);
  *fconst=FALSE;
  *fstock=FALSE;
  *fstatic=FALSE;
  *fpublic=FALSE;
  switch (initialtok) {
  case tCONST:
    *fconst=TRUE;
    break;
  case tSTOCK:
    *fstock=TRUE;
    break;
  case tSTATIC:
    *fstatic=TRUE;
    break;
  case tPUBLIC:
    *fpublic=TRUE;
    break;
  } /* switch */

  err=0;
  do {
    tok=lex(&val,&str);  /* read in (new) token */
    switch (tok) {
    case tCONST:
      if (*fconst)
        err=42;          /* invalid combination of class specifiers */
      *fconst=TRUE;
      break;
    case tSTOCK:
      if (*fstock)
        err=42;          /* invalid combination of class specifiers */
      *fstock=TRUE;
      break;
    case tSTATIC:
      if (*fstatic)
        err=42;          /* invalid combination of class specifiers */
      *fstatic=TRUE;
      break;
    case tPUBLIC:
      if (*fpublic)
        err=42;          /* invalid combination of class specifiers */
      *fpublic=TRUE;
      break;
    default:
      lexpush();
      tok=0;             /* force break out of loop */
    } /* switch */
  } while (tok && err==0);

  /* extra checks */
  if (*fstatic && *fpublic) {
    err=42;              /* invalid combination of class specifiers */
    *fstatic=*fpublic=FALSE;
  } /* if */

  if (err)
    error(err);
  return err==0;
}

/*  parse       - process all input text
 *
 *  At this level, only static declarations and function definitions are legal.
 */
static void parse(void)
{
  int tok,fconst,fstock,fstatic,fpublic;
  cell val;
  char *str;

  while (freading){
    /* first try whether a declaration possibly is native or public */
    tok=lex(&val,&str);  /* read in (new) token */
    switch (tok) {
    case 0:
      /* ignore zero's */
      break;
    case t__EMIT:
      begcseg();
      emit_flags |= efGLOBAL;
      lex(&val,&str);
      emit_parse_line();
      needtoken(';');
      emit_flags &= ~efGLOBAL;
      break;
    case tNEW:
      if (getclassspec(tok,&fpublic,&fstatic,&fstock,&fconst))
        declglb(NULL,0,fpublic,fstatic,fstock,fconst);
      break;
    case tSTATIC:
      if (matchtoken(tENUM)) {
        decl_enum(sGLOBAL,TRUE);
      } else {
        /* This can be a static function or a static global variable; we know
         * which of the two as soon as we have parsed up to the point where an
         * opening parenthesis of a function would be expected. To back out after
         * deciding it was a declaration of a static variable after all, we have
         * to store the symbol name and tag.
         */
        if (getclassspec(tok,&fpublic,&fstatic,&fstock,&fconst)) {
          assert(!fpublic);
          declfuncvar(fpublic,fstatic,fstock,fconst);
        } /* if */
      } /* if */
      break;
    case tCONST:
      decl_const(sGLOBAL);
      break;
    case tENUM:
      decl_enum(sGLOBAL,matchtoken(tSTATIC));
      break;
    case tPUBLIC:
      /* This can be a public function or a public variable; see the comment
       * above (for static functions/variables) for details.
       */
      if (getclassspec(tok,&fpublic,&fstatic,&fstock,&fconst)) {
        assert(!fstatic);
        declfuncvar(fpublic,fstatic,fstock,fconst);
      } /* if */
      break;
    case tSTOCK:
      /* This can be a stock function or a stock *global*) variable; see the
       * comment above (for static functions/variables) for details.
       */
      if (getclassspec(tok,&fpublic,&fstatic,&fstock,&fconst)) {
        assert(fstock);
        declfuncvar(fpublic,fstatic,fstock,fconst);
      } /* if */
      break;
    case t__PRAGMA:
    case tLABEL:
    case tSYMBOL:
    case tOPERATOR:
      lexpush();
      if (!newfunc(NULL,-1,FALSE,FALSE,FALSE)) {
        error(10);              /* illegal function or declaration */
        lexclr(TRUE);           /* drop the rest of the line */
        litidx=0;               /* drop the literal queue too */
      } /* if */
      break;
    case tNATIVE:
      funcstub(TRUE);           /* create a dummy function */
      break;
    case tFORWARD:
      funcstub(FALSE);
      break;
    case t__STATIC_ASSERT:
    case t__STATIC_CHECK: {
      int use_warning=(tok==t__STATIC_CHECK);
      do_static_check(use_warning);
      needtoken(';');
      break;
    } /* case */
    case '}':
      error(54);                /* unmatched closing brace */
      break;
    case '{':
      error(55);                /* start of function body without function header */
      break;
    default:
      if (freading) {
        error(10);              /* illegal function or declaration */
        lexclr(TRUE);           /* drop the rest of the line */
        litidx=0;               /* drop any literal arrays (strings) */
      } /* if */
    } /* switch */
  } /* while */
}

/*  dumplits
 *
 *  Dump the literal pool (strings etc.)
 *
 *  Global references: litidx (referred to only)
 */
static void dumplits(void)
{
  int i,j;
  static const int row_len=16;

  if (sc_status==statSKIP)
    return;

  i=0;
  while (i<litidx) {
    /* should be in the data segment */
    assert(curseg==2);
    j=i+1;
    while (j<litidx && litq[j]==litq[i])
      j++;
    if (j-i>=row_len-1) {
      int count=j-i;
      defcompactstorage();
      outval(litq[i],FALSE);
      stgwrite(" ");
      outval(count,TRUE);
      i+=count;
    } else {
      defstorage();
      j=row_len;       /* 16 values per line */
      while (j && i<litidx){
        outval(litq[i],FALSE);
        stgwrite(" ");
        i++;
        j--;
        if (j==0 || i>=litidx)
          stgwrite("\n");         /* force a newline after 10 dumps */
        /* Note: stgwrite() buffers a line until it is complete. It recognizes
         * the end of line as a sequence of "\n\0", so something like "\n\t"
         * so should not be passed to stgwrite().
         */
      } /* while */
    } /* if */
  } /* while */
}

/*  dumpzero
 *
 *  Dump zero's for default initial values
 */
static void dumpzero(int count)
{
  if (sc_status==statSKIP || count<=0)
    return;
  assert(curseg==2);
  defcompactstorage();
  outval(0, FALSE);
  stgwrite(" ");
  outval(count, TRUE);
}

static void aligndata(int numbytes)
{
  assert(numbytes % sizeof(cell) == 0);   /* alignment must be a multiple of
                                           * the cell size */
  assert(numbytes!=0);

  if ((((glb_declared+litidx)*sizeof(cell)) % numbytes)!=0) {
    while ((((glb_declared+litidx)*sizeof(cell)) % numbytes)!=0)
      litadd(0);
  } /* if */

}

#if !defined SC_LIGHT
/* sc_attachdocumentation()
 * appends documentation comments to the passed-in symbol, or to a global
 * string if "sym" is NULL.
 */
void sc_attachdocumentation(symbol *sym)
{
  int line;
  size_t length;
  char *str,*doc;

  if (!sc_makereport || sc_status!=statFIRST || sc_parsenum>0) {
    /* just clear the entire table */
    delete_docstringtable();
    return;
  } /* if */
  /* in the case of state functions, multiple documentation sections may
   * appear; we should concatenate these
   * (with forward declarations, this is also already the case, so the assertion
   * below is invalid)
   */
  // assert(sym==NULL || sym->documentation==NULL || sym->states!=NULL);

  /* first check the size */
  length=0;
  for (line=0; (str=get_docstring(line))!=NULL && *str!=sDOCSEP; line++) {
    if (str[0]!='\0') {
      if (length>0)
        length++;   /* count 1 extra for a separating space */
      length+=strlen(str);
    }
  } /* for */
  if (length>0) {
    if (sym==NULL && sc_documentation!=NULL) {
      length += strlen(sc_documentation) + 1 + 4; /* plus 4 for "<p/>" */
      assert(length > strlen(sc_documentation));
    } else if (sym!=NULL && sym->documentation!=NULL) {
      length+=strlen(sym->documentation) + 1 + 4;/* plus 4 for "<p/>" */
      assert(length > strlen(sym->documentation));
    } /* if */

    /* allocate memory for the documentation */
    doc=(char*)malloc((length+1)*sizeof(char));
    if (doc!=NULL) {
      /* initialize string or concatenate */
      if (sym==NULL && sc_documentation!=NULL) {
        strcpy(doc,sc_documentation);
        strcat(doc,"<p/>");
        free(sc_documentation);
        sc_documentation=NULL;
      } else if (sym!=NULL && sym->documentation!=NULL) {
        strcpy(doc,sym->documentation);
        strcat(doc,"<p/>");
        free(sym->documentation);
        sym->documentation=NULL;
      } else {
        doc[0]='\0';
      } /* if */
      /* collect all documentation */
      while ((str=get_docstring(0))!=NULL && *str!=sDOCSEP) {
        if (str[0]!='\0') {
          if (doc[0]!='\0')
            strcat(doc," ");
          strcat(doc,str);
        }
        delete_docstring(0);
      } /* while */
      if (str!=NULL) {
        /* also delete the separator */
        assert(*str==sDOCSEP);
        delete_docstring(0);
      } /* if */
      if (sym==NULL) {
        assert(sc_documentation==NULL);
        sc_documentation=doc;
      } else {
        assert(sym->documentation==NULL);
        sym->documentation=doc;
      } /* if */
    } /* if */
  } else {
    /* delete an empty separator, if present */
    if ((str=get_docstring(0))!=NULL && *str==sDOCSEP)
      delete_docstring(0);
  } /* if */
}

static void insert_docstring_separator(void)
{
  char sep[2]={sDOCSEP,'\0'};
  insert_docstring(sep);
}
#else
  #define sc_attachdocumentation(s)      (void)(s)
  #define insert_docstring_separator()
#endif

static void declfuncvar(int fpublic,int fstatic,int fstock,int fconst)
{
  char name[sNAMEMAX+11];
  int tok,tag;
  char *str;
  cell val;
  int invalidfunc;

  tag=pc_addtag(NULL);
  tok=lex(&val,&str);
  /* if we arrived here, this may not be a declaration of a native function
   * or variable
   */
  if (tok==tNATIVE) {
    error(42);          /* invalid combination of class specifiers */
    return;
  } /* if */

  if (tok==t__PRAGMA) {
    dopragma();
    tok=lex(&val,&str);
  } /* if */

  if (tok!=tSYMBOL && tok!=tOPERATOR) {
    lexpush();
    needtoken(tSYMBOL);
    lexclr(TRUE);       /* drop the rest of the line */
    litidx=0;           /* drop the literal queue too */
    return;
  } /* if */
  if (tok==tOPERATOR) {
    lexpush();          /* push "operator" keyword back (for later analysis) */
    if (!newfunc(NULL,tag,fpublic,fstatic,fstock)) {
      error(10);        /* illegal function or declaration */
      lexclr(TRUE);     /* drop the rest of the line */
      litidx=0;         /* drop the literal queue too */
    } /* if */
  } else {
    /* so tok is tSYMBOL */
    assert(strlen(str)<=sNAMEMAX);
    strcpy(name,str);
    /* only variables can be "const" or both "public" and "stock" */
    invalidfunc= fconst || (fpublic && fstock);
    if (invalidfunc || !newfunc(name,tag,fpublic,fstatic,fstock)) {
      /* if not a function, try a global variable */
      declglb(name,tag,fpublic,fstatic,fstock,fconst);
    } /* if */
  } /* if */
}

/*  declglb     - declare global symbols
 *
 *  Declare a static (global) variable. Global variables are stored in
 *  the DATA segment.
 *
 *  global references: glb_declared     (altered)
 */
static void declglb(char *firstname,int firsttag,int fpublic,int fstatic,int fstock,int fconst)
{
  int ident,tag,ispublic;
  int idxtag[sDIMEN_MAX];
  char name[sNAMEMAX+1];
  cell val,size,cidx;
  ucell address;
  int glb_incr;
  char *str;
  int dim[sDIMEN_MAX];
  int numdim;
  int explicit_init;
  short filenum;
  symbol *sym;
  constvalue_root *enumroot=NULL;
  #if !defined NDEBUG
    cell glbdecl=0;
  #endif

  assert(!fpublic || !fstatic);         /* may not both be set */
  insert_docstring_separator();         /* see comment in newfunc() */
  filenum=fcurrent;                     /* save file number at the start of the declaration */
  do {
    size=1;                             /* single size (no array) */
    numdim=0;                           /* no dimensions */
    ident=iVARIABLE;
    if (firstname!=NULL) {
      assert(strlen(firstname)<=sNAMEMAX);
      strcpy(name,firstname);           /* save symbol name */
      tag=firsttag;
      firstname=NULL;
    } else {
      tag=pc_addtag(NULL);
      if (matchtoken(t__PRAGMA))
        dopragma();
      if (lex(&val,&str)!=tSYMBOL)      /* read in (new) token */
        error_suggest(20,str,NULL,estSYMBOL,esfFUNCTION);   /* invalid symbol name */
      assert(strlen(str)<=sNAMEMAX);
      strcpy(name,str);                 /* save symbol name */
    } /* if */
    ispublic=fpublic;
    if (name[0]==PUBLIC_CHAR) {
      ispublic=TRUE;                    /* implicitly public variable */
      assert(!fstatic);
    } /* if */
    while (matchtoken('[')) {
      ident=iARRAY;
      if (numdim == sDIMEN_MAX) {
        error(53);                      /* exceeding maximum number of dimensions */
        return;
      } /* if */
      size=needsub(&idxtag[numdim],&enumroot);  /* get size; size==0 for "var[]" */
      #if INT_MAX < LONG_MAX
        if (size > INT_MAX)
          error(105);                   /* overflow, exceeding capacity */
      #endif
      if (ispublic)
        error(56,name);                 /* arrays cannot be public */
      dim[numdim++]=(int)size;
    } /* while */
    assert(sc_curstates==0);
    sc_curstates=getstates(name);
    if (sc_curstates<0) {
      error(85,name);           /* empty state list on declaration */
      sc_curstates=0;
    } else if (sc_curstates>0 && ispublic) {
      error(88,name);           /* public variables may not have states */
      sc_curstates=0;
    } /* if */
    sym=findconst(name,NULL);
    if (sym==NULL) {
      sym=findglb(name,sSTATEVAR);
      /* if a global variable without states is found and this declaration has
       * states, the declaration is okay
       */
      if (sym!=NULL && sym->states==NULL && sc_curstates>0)
        sym=NULL;               /* set to NULL, we found the global variable */
      if (sc_curstates>0 && findglb(name,sGLOBAL)!=NULL)
        error(233,name);        /* state variable shadows a global variable */
    } /* if */
    /* we have either:
     * a) not found a matching variable (or rejected it, because it was a shadow)
     * b) found a global variable and we were looking for that global variable
     * c) found a state variable in the automaton that we were looking for
     */
    assert(sym==NULL
           || (sym->states==NULL && sc_curstates==0)
           || (sym->states!=NULL && sym->states->first!=NULL && sym->states->first->index==sc_curstates));
    /* a state variable may only have a single id in its list (so either this
     * variable has no states, or it has a single list)
     */
    assert(sym==NULL || sym->states==NULL || sym->states->first->next==NULL);
    /* it is okay for the (global) variable to exist, as long as it belongs to
     * a different automaton
     */
    if (sym!=NULL && (sym->usage & uDEFINE)!=0)
      error(21,name);                   /* symbol already defined */
    /* if this variable is never used (which can be detected only in the
     * second stage), shut off code generation
     */
    cidx=0;             /* only to avoid a compiler warning */
    if (sc_status==statWRITE && sym!=NULL && (sym->usage & (uREAD | uWRITTEN | uPUBLIC))==0) {
      sc_status=statSKIP;
      cidx=code_idx;
      #if !defined NDEBUG
        glbdecl=glb_declared;
      #endif
    } /* if */
    begdseg();          /* real (initialized) data in data segment */
    assert(litidx==0);  /* literal queue should be empty */
    if (sc_alignnext) {
      litidx=0;
      aligndata(sc_dataalign);
      dumplits();       /* dump the literal queue */
      sc_alignnext=FALSE;
      litidx=0;         /* global initial data is dumped, so restart at zero */
    } /* if */
    assert(litidx==0);  /* literal queue should be empty (again) */
    initials(ident,tag,&size,dim,numdim,enumroot,&explicit_init);/* stores values in the literal queue */
    assert(size>=litidx);
    if (numdim==1)
      dim[0]=(int)size;
    /* before dumping the initial values (or zeros) check whether this variable
     * overlaps another
     */
    if (sc_curstates>0) {
      unsigned char *map;

      if (litidx!=0)
        error(89,name); /* state variables may not be initialized */
      /* find an appropriate address for the state variable */
      /* assume that it cannot be found */
      address=sizeof(cell)*glb_declared;
      glb_incr=(int)size;
      /* use a memory map in which every cell occupies one bit */
      if (glb_declared>0 && (map=(unsigned char*)malloc((glb_declared+7)/8))!=NULL) {
        int fsa=state_getfsa(sc_curstates);
        symbol *sweep;
        cell sweepsize,addr;
        memset(map,0,(glb_declared+7)/8);
        assert(fsa>=0);
        /* fill in all variables belonging to this automaton */
        for (sweep=glbtab.next; sweep!=NULL; sweep=sweep->next) {
          if (sweep->parent!=NULL || sweep->states==NULL || sweep==sym)
            continue;   /* hierarchical type, or no states, or same as this variable */
          if (sweep->ident!=iVARIABLE && sweep->ident!=iARRAY)
            continue;   /* a function or a constant */
          if ((sweep->usage & uDEFINE)==0)
            continue;   /* undefined variable, ignore */
          if (fsa!=state_getfsa(sweep->states->first->index))
            continue;   /* wrong automaton */
          /* when arrived here, this is a global variable, with states and
           * belonging to the same automaton as the variable we are declaring
           */
          sweepsize=(sweep->ident==iVARIABLE) ? 1 : array_totalsize(sweep);
          assert(sweep->addr % sizeof(cell) == 0);
          addr=sweep->addr/sizeof(cell);
          /* mark this address range */
          while (sweepsize-->0) {
            map[addr/8] |= (unsigned char)(1 << (addr % 8));
            addr++;
          } /* while */
        } /* for */
        /* go over it again, clearing any ranges that have conflicts */
        for (sweep=glbtab.next; sweep!=NULL; sweep=sweep->next) {
          if (sweep->parent!=NULL || sweep->states==NULL || sweep==sym)
            continue;   /* hierarchical type, or no states, or same as this variable */
          if (sweep->ident!=iVARIABLE && sweep->ident!=iARRAY)
            continue;   /* a function or a constant */
          if ((sweep->usage & uDEFINE)==0)
            continue;   /* undefined variable, ignore */
          if (fsa!=state_getfsa(sweep->states->first->index))
            continue;   /* wrong automaton */
          /* when arrived here, this is a global variable, with states and
           * belonging to the same automaton as the variable we are declaring
           */
          /* if the lists of states of the existing variable and the new
           * variable have a non-empty intersection, this is not a suitable
           * overlap point -> wipe the address range
           */
          if (state_conflict_id(sc_curstates,sweep->states->first->index)) {
            sweepsize=(sweep->ident==iVARIABLE) ? 1 : array_totalsize(sweep);
            assert(sweep->addr % sizeof(cell) == 0);
            addr=sweep->addr/sizeof(cell);
            /* mark this address range */
            while (sweepsize-->0) {
              map[addr/8] &= (unsigned char)(~(1 << (addr % 8)));
              addr++;
            } /* while */
          } /* if */
        } /* for */
        /* now walk through the map and find a starting point that is big enough */
        sweepsize=0;
        for (addr=0; addr<glb_declared; addr++) {
          if ((map[addr/8] & (1 << (addr % 8)))==0)
            continue;
          for (sweepsize=addr+1; sweepsize<glb_declared; sweepsize++) {
            if ((map[sweepsize/8] & (1 << (sweepsize % 8)))==0)
              break;    /* zero bit found, skip this range */
            if (sweepsize-addr>=size)
              break;    /* fitting range found, no need to search further */
          } /* for */
          if (sweepsize-addr>=size)
            break;      /* fitting range found, no need to search further */
          addr=sweepsize;
        } /* for */
        free(map);
        if (sweepsize-addr>=size) {
          address=sizeof(cell)*addr;    /* fitting range found, set it */
          glb_incr=0;
        } /* if */
      } /* if */
    } else {
      address=sizeof(cell)*glb_declared;
      glb_incr=(int)size;
    } /* if */
    if (address==sizeof(cell)*glb_declared) {
      dumplits();       /* dump the literal queue */
      dumpzero((int)size-litidx);
    } /* if */
    litidx=0;
    if (sym==NULL) {    /* define only if not yet defined */
      sym=addvariable(name,address,ident,sGLOBAL,tag,dim,numdim,idxtag,0);
      if (sc_curstates>0)
        attachstatelist(sym,sc_curstates);
    } else {            /* if declared but not yet defined, adjust the variable's address */
      assert(sym->states==NULL && sc_curstates==0
             || sym->states->first!=NULL && sym->states->first->index==sc_curstates && sym->states->first->next==NULL);
      sym->addr=address;
      sym->codeaddr=code_idx;
      sym->usage|=uDEFINE;
    } /* if */
    assert(sym!=NULL);
    sc_curstates=0;
    if (ispublic)
      sym->usage|=uPUBLIC;
    if (fconst) {
      symbol *cur=sym;
      do {
        cur->usage|=uCONST;
      } while ((cur=cur->child)!=NULL);
    } /* if */
    if (fstock)
      sym->usage|=uSTOCK;
    if (fstatic)
      sym->fnumber=filenum;
    if (explicit_init)
      markinitialized(sym,TRUE);
    sc_attachdocumentation(sym);/* attach any documentation to the variable */
    if (sc_status==statSKIP) {
      sc_status=statWRITE;
      code_idx=cidx;
      assert(glb_declared==glbdecl);
    } else {
      glb_declared+=glb_incr;   /* add total number of cells (if added to the end) */
    } /* if */
    if (matchtoken(t__PRAGMA))
      dopragma();
    pragma_apply(sym);
  } while (matchtoken(',')); /* enddo */   /* more? */
  needtoken(tTERM);    /* if not comma, must be semicolon */
}

/*  declloc     - declare local symbols
 *
 *  Declare local (automatic) variables. Since these variables are relative
 *  to the STACK, there is no switch to the DATA segment. These variables
 *  cannot be initialized either.
 *
 *  global references: declared   (altered)
 *                     funcstatus (referred to only)
 */
static int declloc(int fstatic)
{
  int ident,tag;
  int idxtag[sDIMEN_MAX];
  char name[sNAMEMAX+1];
  symbol *sym;
  constvalue_root *enumroot=NULL;
  cell val,size;
  char *str;
  value lval = {0};
  int cur_lit=0;
  int dim[sDIMEN_MAX];
  int numdim;
  int fconst;
  int staging_start;
  int explicit_init;    /* is the variable explicitly initialized? */
  int suppress_w240=FALSE;

  fconst=matchtoken(tCONST);
  do {
    ident=iVARIABLE;
    size=1;
    numdim=0;                           /* no dimensions */
    if (matchtoken(t__PRAGMA))
      dopragma();
    tag=pc_addtag(NULL);
    if (!needtoken(tSYMBOL)) {
      lexclr(TRUE);                     /* drop the rest of the line... */
      return 0;                         /* ...and quit */
    } /* if */
    tokeninfo(&val,&str);
    assert(strlen(str)<=sNAMEMAX);
    strcpy(name,str);                   /* save symbol name */
    if (name[0]==PUBLIC_CHAR)
      error(56,name);                   /* local variables cannot be public */
    /* Note: block locals may be named identical to locals at higher
     * compound blocks (as with standard C); so we must check (and add)
     * the "nesting level" of local variables to verify the
     * multi-definition of symbols.
     */
    if ((sym=findloc(name))!=NULL && sym->compound==pc_nestlevel)
      error(21,name);                   /* symbol already defined */
    /* Although valid, a local variable whose name is equal to that
     * of a global variable or to that of a local variable at a lower
     * level might indicate a bug.
     */
    if (((sym=findloc(name))!=NULL && sym->compound!=pc_nestlevel) || findglb(name,sGLOBAL)!=NULL)
      error(219,name);                  /* variable shadows another symbol */
    while (matchtoken('[')){
      ident=iARRAY;
      if (numdim == sDIMEN_MAX) {
        error(53);                      /* exceeding maximum number of dimensions */
        return ident;
      } /* if */
      size=needsub(&idxtag[numdim],&enumroot); /* get size; size==0 for "var[]" */
      #if INT_MAX < LONG_MAX
        if (size > INT_MAX)
          error(105);                   /* overflow, exceeding capacity */
      #endif
      dim[numdim++]=(int)size;
    } /* while */
    if (getstates(name))
      error(88,name);           /* local variables may not have states */
    if (ident==iARRAY || fstatic) {
      if (sc_alignnext) {
        aligndata(sc_dataalign);
        sc_alignnext=FALSE;
      } /* if */
      cur_lit=litidx;           /* save current index in the literal table */
      initials(ident,tag,&size,dim,numdim,enumroot,&explicit_init);
      if (size==0)
        return ident;           /* error message already given */
      if (numdim==1)
        dim[0]=(int)size;
    } /* if */
    /* reserve memory (on the stack) for the variable */
    if (fstatic) {
      /* write zeros for uninitialized fields */
      while (litidx<cur_lit+size)
        litadd(0);
      sym=addvariable(name,(cur_lit+glb_declared)*sizeof(cell),ident,sSTATIC,
                      tag,dim,numdim,idxtag,pc_nestlevel);
    } else {
      declared+=(int)size;      /* variables are put on stack, adjust "declared" */
      sym=addvariable(name,-declared*sizeof(cell),ident,sLOCAL,
                      tag,dim,numdim,idxtag,pc_nestlevel);
      if (ident==iVARIABLE) {
        assert(!staging);
        stgset(TRUE);           /* start stage-buffering */
        assert(stgidx==0);
        staging_start=stgidx;
      } /* if */
      markexpr(sLDECL,name,-declared*sizeof(cell)); /* mark for better optimization */
      modstk(-(int)size*sizeof(cell));
      assert(curfunc!=NULL);
      assert((curfunc->usage & uNATIVE)==0);
      if (curfunc->x.stacksize<declared+1)
        curfunc->x.stacksize=declared+1;  /* +1 for PROC opcode */
    } /* if */
    /* now that we have reserved memory for the variable, we can proceed
     * to initialize it */
    assert(sym!=NULL);          /* we declared it, it must be there */
    if (fconst) {
      symbol *cur=sym;
      do {
        cur->usage|=uCONST;
      } while ((cur=cur->child)!=NULL);
    } /* if */
    if (!fstatic) {             /* static variables already initialized */
      if (ident==iVARIABLE) {
        /* simple variable, also supports initialization */
        int ctag = tag;         /* set to "tag" by default */
        explicit_init=FALSE;
        if (matchtoken('=')) {
          int initexpr_ident;
          sym->usage &= ~uDEFINE;   /* temporarily mark the variable as undefined to prevent
                                     * possible self-assignment through its initialization expression */
          initexpr_ident=doexpr(FALSE,FALSE,FALSE,FALSE,&ctag,NULL,TRUE,&val);
          sym->usage |= uDEFINE;
          explicit_init=TRUE;
          suppress_w240=(initexpr_ident==iCONSTEXPR && val==0);
        } else {
          ldconst(0,sPRI);      /* uninitialized variable, set to zero */
        } /* if */
        /* now try to save the value (still in PRI) in the variable */
        lval.sym=sym;
        lval.ident=iVARIABLE;
        lval.constval=0;
        lval.tag=tag;
        suppress_w240 |= check_userop(NULL,ctag,lval.tag,2,NULL,&ctag);
        store(&lval);
        markexpr(sEXPR,NULL,0); /* full expression ends after the store */
        assert(staging);        /* end staging phase (optimize expression) */
        stgout(staging_start);
        stgset(FALSE);
        check_tagmismatch(tag,ctag,TRUE,-1);
        /* if the variable was not explicitly initialized, reset the
         * "uWRITTEN" flag that store() set */
        if (!explicit_init)
          sym->usage &= ~uWRITTEN;
      } else {
        /* an array */
        assert(cur_lit>=0 && cur_lit<=litidx && litidx<=litmax);
        assert(size>0 && size>=sym->dim.array.length);
        assert(numdim>1 || size==sym->dim.array.length);
        /* final literal values that are zero make no sense to put in the literal
         * pool, because values get zero-initialized anyway; we check for this,
         * because users often explicitly initialize strings to ""
         */
        while (litidx>cur_lit && litq[litidx-1]==0)
          litidx--;
        /* if the array is not completely filled, set all values to zero first */
        if (litidx-cur_lit<size && (ucell)size<CELL_MAX)
          fillarray(sym,size*sizeof(cell),0);
        if (cur_lit<litidx) {
          /* check whether the complete array is set to a single value; if
           * it is, more compact code can be generated */
          cell first=litq[cur_lit];
          int i;
          for (i=cur_lit; i<litidx && litq[i]==first; i++)
            /* nothing */;
          if (i==litidx) {
            /* all values are the same */
            fillarray(sym,(litidx-cur_lit)*sizeof(cell),first);
            litidx=cur_lit;     /* reset literal table */
          } else {
            /* copy the literals to the array */
            ldconst((cur_lit+glb_declared)*sizeof(cell),sPRI);
            copyarray(sym,(litidx-cur_lit)*sizeof(cell));
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    if (explicit_init)
      markinitialized(sym,!suppress_w240);
    if (pc_ovlassignment)
      sym->usage |= uREAD;
    if (matchtoken(t__PRAGMA))
      dopragma();
    pragma_apply(sym);
  } while (matchtoken(',')); /* enddo */   /* more? */
  needtoken(tTERM);    /* if not comma, must be semicolon */
  return ident;
}

/* this function returns the maximum value for a cell in case of an error
 * (invalid dimension).
 */
static cell calc_arraysize(int dim[],int numdim,int cur)
{
  cell subsize;
  ucell newsize;

  /* the return value is in cells, not bytes */
  assert(cur>=0 && cur<=numdim);
  if (cur==numdim)
    return 0;
  subsize=calc_arraysize(dim,numdim,cur+1);
  newsize=dim[cur]+dim[cur]*subsize;
  if (newsize==0)
    return 0;
  if ((ucell)subsize>=CELL_MAX || newsize>=CELL_MAX || newsize<(ucell)subsize
      || newsize*sizeof(cell)>=CELL_MAX)
    return CELL_MAX;
  return newsize;
}

static void adjust_indirectiontables(int dim[],int numdim,int startlit,
                                     constvalue_root *lastdim,int *skipdim)
{
static int base;
  int cur;
  int i,d;
  cell accum;
  cell size;

  assert(startlit==-1 || (startlit>=0 && startlit<=litidx));
  base=startlit;
  size=1;
  for (cur=0; cur<numdim-1; cur++) {
    /* 2 or more dimensions left, fill in an indirection vector */
    if (dim[cur+1]>0) {
      for (i=0; i<size; i++)
        for (d=0; d<dim[cur]; d++)
          litq[base++]=(size*dim[cur]+(dim[cur+1]-1)*(dim[cur]*i+d)) * sizeof(cell);
    } else {
      /* final dimension is variable length */
      constvalue *ld;
      assert(dim[cur+1]==0);
      assert(lastdim!=NULL);
      assert(skipdim!=NULL);
      accum=0;
      for (i=0; i<size; i++) {
        /* skip the final dimension sizes for all earlier major dimensions */
        for (d=0,ld=lastdim->first; d<*skipdim; d++,ld=ld->next) {
          assert(ld!=NULL);
        } /* for */
        for (d=0; d<dim[cur]; d++) {
          assert(ld!=NULL);
          assert(strtol(ld->name,NULL,16)==d);
          litq[base++]=(size*dim[cur]+accum) * sizeof(cell);
          accum+=ld->value-1;
          *skipdim+=1;
          ld=ld->next;
        } /* for */
      } /* for */
    } /* if */
    size*=dim[cur];
  } /* for */
}

/*  initials
 *
 *  Initialize global objects and local arrays.
 *    size==array cells (count), if 0 on input, the routine counts the number of elements
 *    tag==required tagname id (not the returned tag)
 *
 *  Global references: litidx (altered)
 */
static void initials(int ident,int tag,cell *size,int dim[],int numdim,
                     constvalue_root *enumroot,int *explicit_init)
{
  int ctag;
  cell tablesize;
  int curlit=litidx;
  int err=0;
  int i;

  if (explicit_init!=NULL)
    *explicit_init=FALSE;
  if (!matchtoken('=')) {
    assert(ident!=iARRAY || numdim>0);
    if (ident==iARRAY) {
      assert(numdim>0 && numdim<=sDIMEN_MAX);
      for (i=0; i<numdim; i++) {
        if (dim[i]==0) {
          /* declared like "myvar[];" which is senseless (note: this *does* make
           * sense in the case of a iREFARRAY, which is a function parameter)
           */
          error(9); /* array has zero length -> invalid size */
          return;
        } /* if */
      } /* for */
      *size=calc_arraysize(dim,numdim,0);
      if (*size==(cell)CELL_MAX) {
        error(9);       /* array is too big -> invalid size */
        return;
      } /* if */
      /* first reserve space for the indirection vectors of the array, then
       * adjust it to contain the proper values
       * (do not use dumpzero(), as it bypasses the literal queue)
       */
      for (tablesize=calc_arraysize(dim,numdim-1,0); tablesize>0; tablesize--)
        litadd(0);
      if (dim[numdim-1]!=0)     /* error 9 has already been given */
        adjust_indirectiontables(dim,numdim,curlit,NULL,NULL);
    } /* if */
    return;
  } /* if */

  if (explicit_init!=NULL)
    *explicit_init=TRUE;
  if (ident==iVARIABLE) {
    assert(*size==1);
    init(ident,&ctag,NULL);
    check_tagmismatch(tag,ctag,TRUE,-1);
  } else {
    assert(numdim>0);
    if (numdim==1) {
      *size=initvector(ident,tag,dim[0],litidx,FALSE,enumroot,NULL);
    } else {
      int errorfound=FALSE;
      int counteddim[sDIMEN_MAX];
      int idx;
      constvalue_root lastdim = { NULL, NULL};     /* sizes of the final dimension */
      int skipdim=0;

      /* check if size specified for all dimensions */
      for (idx=0; idx<numdim; idx++)
        if (dim[idx]==0)
          break;
      /* already reserve space for the indirection tables (for an array with
       * known dimensions)
       * (do not use dumpzero(), as it bypasses the literal queue)
       */
      if(idx==numdim)
        *size=calc_arraysize(dim,numdim,0);
      else
        *size=0; /* size of one or more dimensions is unknown */
      for (tablesize=calc_arraysize(dim,numdim-1,0); tablesize>0; tablesize--)
        litadd(0);
      /* now initialize the sub-arrays */
      memset(counteddim,0,sizeof counteddim);
      initarray(ident,tag,dim,numdim,0,curlit,counteddim,&lastdim,enumroot,&errorfound);
      /* check the specified array dimensions with the initialler counts */
      for (idx=0; idx<numdim-1; idx++) {
        if (dim[idx]==0) {
          dim[idx]=counteddim[idx];
        } else if (counteddim[idx]<dim[idx]) {
          error(52);            /* array is not fully initialized */
          err++;
        } else if (counteddim[idx]>dim[idx]) {
          error(18);            /* initialization data exceeds declared size */
          err++;
        } /* if */
      } /* for */
      if (numdim>1 && dim[numdim-1]==0) {
        /* also look whether, by any chance, all "counted" final dimensions are
         * the same value; if so, we can store this
         */
        constvalue *ld=lastdim.first;
        int d,match;
        for (d=0; d<dim[numdim-2]; d++) {
          assert(ld!=NULL);
          assert(strtol(ld->name,NULL,16)==d);
          if (d==0)
            match=ld->value;
          else if (match!=ld->value)
            break;
          ld=ld->next;
        } /* for */
        if (d==dim[numdim-2])
          dim[numdim-1]=match;
      } /* if */
      /* after all arrays have been initialized, we know the (major) dimensions
       * of the array and we can properly adjust the indirection vectors
       */
      if (err==0)
        adjust_indirectiontables(dim,numdim,curlit,&lastdim,&skipdim);
      delete_consttable(&lastdim);  /* clear list of minor dimension sizes */
    } /* if */
  } /* if */

  if (*size==0)
    *size=litidx-curlit;        /* number of elements defined */
}

static cell initarray(int ident,int tag,int dim[],int numdim,int cur,
                      int startlit,int counteddim[],constvalue_root *lastdim,
                      constvalue_root *enumroot,int *errorfound)
{
  cell dsize,totalsize;
  int idx,idx_ellips,vidx,do_insert;
  int abortparse;
  int curlit;
  int prev1_idx=-1,prev2_idx=-1;

  assert(cur>=0 && cur<numdim);
  assert(startlit>=0);
  assert(cur+2<=numdim);        /* there must be 2 dimensions or more to do */
  assert(errorfound!=NULL && *errorfound==FALSE);
  totalsize=0;
  needtoken('{');
  for (do_insert=FALSE,idx=0; idx<=cur; idx++) {
    if (dim[idx]==0) {
      do_insert=TRUE;
      break;
    } /* if */
  } /* for */
  for (idx=0,abortparse=FALSE; !abortparse; idx++) {
    /* In case the major dimension is zero, we need to store the offset
     * to the newly detected sub-array into the indirection table; i.e.
     * this table needs to be expanded and updated.
     * In the current design, the indirection vectors for a multi-dimensional
     * array are adjusted after parsing all initiallers. Hence, it is only
     * necessary at this point to reserve space for an extra cell in the
     * indirection vector.
     */
    if (do_insert) {
      litinsert(0,startlit);
    } else if (idx>=dim[cur]) {
      error(18);                /* initialization data exceeds array size */
      break;
    } /* if */
    if (cur+2<numdim) {
      dsize=initarray(ident,tag,dim,numdim,cur+1,startlit,counteddim,
                      lastdim,enumroot,errorfound);
    } else {
      curlit=litidx;
      if (matchtoken(tELLIPS)!=0) {
        /* found an ellipsis; fill up the rest of the array with a series
         * of one-dimensional arrays ("2d ellipsis")
         */
        if (prev1_idx!=-1) {
          for (idx_ellips=1; idx < dim[cur]; idx++, idx_ellips++) {
            for (vidx=0; vidx < dsize; vidx++) {
              if (prev2_idx!=-1)
                litadd(litq[prev1_idx+vidx]+idx_ellips*(litq[prev1_idx+vidx]-litq[prev2_idx+vidx]));
              else
                litadd(litq[prev1_idx+vidx]);
            } /* for */
            append_constval(lastdim,itoh(idx),dsize,0);
          } /* for */
          idx--;
        } else
          error(41);            /* invalid ellipsis, array size unknown */
      } else {
        prev2_idx=prev1_idx;
        prev1_idx=litidx;
        dsize=initvector(ident,tag,dim[cur+1],curlit,TRUE,enumroot,errorfound);
        /* The final dimension may be variable length. We need to save the
         * lengths of the final dimensions in order to set the indirection
         * vectors for the next-to-last dimension.
         */
        append_constval(lastdim,itoh(idx),dsize,0);
      } /* if */
    } /* if */
    totalsize+=dsize;
    if (*errorfound || !matchtoken(','))
      abortparse=TRUE;
  } /* for */
  needtoken('}');
  assert(counteddim!=NULL);
  if (counteddim[cur]>0) {
    if (idx<counteddim[cur])
      error(52);                /* array is not fully initialized */
    else if (idx>counteddim[cur])
      error(18);                /* initialization data exceeds declared size */
  } /* if */
  counteddim[cur]=idx;

  return totalsize+dim[cur];    /* size of sub-arrays + indirection vector */
}

/*  initvector
 *  Initialize a single dimensional array
 */
static cell initvector(int ident,int tag,cell size,int startlit,int fillzero,
                       constvalue_root *enumroot,int *errorfound)
{
  cell prev1=0,prev2=0;
  int ellips=FALSE;
  int rtag,ctag;

  assert(ident==iARRAY || ident==iREFARRAY);
  if (matchtoken('{')) {
    constvalue *enumfield=(enumroot!=NULL) ? enumroot->first : NULL;
    do {
      int fieldlit=litidx;
      int matchbrace,i;
      if (matchtoken('}')) {    /* to allow for trailing ',' after the initialization */
        lexpush();
        break;
      } /* if */
      if ((ellips=matchtoken(tELLIPS))!=0)
        break;
      /* for enumeration fields, allow another level of braces ("{...}") */
      matchbrace=0;             /* preset */
      ellips=0;
      if (enumfield!=NULL)
        matchbrace=matchtoken('{');
      for ( ;; ) {
        prev2=prev1;
        prev1=init(ident,&ctag,errorfound);
        if (!matchbrace)
          break;
        if ((ellips=matchtoken(tELLIPS))!=0)
          break;
        if (!matchtoken(',')) {
          needtoken('}');
          break;
        } /* if */
      } /* for */
      /* if this array is based on an enumeration, fill the "field" up with
       * zeros, and toggle the tag
       */
      if (enumroot!=NULL && enumfield==NULL)
        error(227);             /* more initiallers than enum fields */
      rtag=tag;                 /* preset, may be overridden by enum field tag */
      if (enumfield!=NULL) {
        cell step;
        int cmptag=enumfield->index;
        symbol *symfield=findconst(enumfield->name,&cmptag);
        if (cmptag>1)
          error(91,enumfield->name); /* ambiguous constant, needs tag override */
        assert(symfield!=NULL);
        assert(fieldlit<litidx);
        if (litidx-fieldlit>symfield->dim.array.length)
          error(228);           /* length of initialler exceeds size of the enum field */
        if (ellips) {
          step=prev1-prev2;
        } else {
          step=0;
          prev1=0;
        } /* if */
        for (i=litidx-fieldlit; i<symfield->dim.array.length; i++) {
          prev1+=step;
          litadd(prev1);
        } /* for */
        rtag=symfield->x.tags.index;  /* set the expected tag to the index tag */
        enumfield=enumfield->next;
      } /* if */
      check_tagmismatch(rtag,ctag,TRUE,-1);
    } while (matchtoken(',')); /* do */
    needtoken('}');
  } else {
    init(ident,&ctag,errorfound);
    check_tagmismatch(tag,ctag,TRUE,-1);
  } /* if */
  /* fill up the literal queue with a series */
  if (ellips) {
    cell step=((litidx-startlit)==1) ? (cell)0 : prev1-prev2;
    if (size==0 || (litidx-startlit)==0)
      error(41);                /* invalid ellipsis, array size unknown */
    else if ((litidx-startlit)==(int)size)
      error(18);                /* initialisation data exceeds declared size */
    while ((litidx-startlit)<(int)size) {
      prev1+=step;
      litadd(prev1);
    } /* while */
  } /* if */
  if (fillzero && size>0) {
    while ((litidx-startlit)<(int)size)
      litadd(0);
  } /* if */
  if (size==0) {
    size=litidx-startlit;         /* number of elements defined */
  } else if (litidx-startlit>(int)size) { /* e.g. "myvar[3]={1,2,3,4};" */
    error(18);                  /* initialisation data exceeds declared size */
    litidx=(int)size+startlit;    /* avoid overflow in memory moves */
  } /* if */
  return size;
}

/*  init
 *
 *  Evaluate one initializer.
 */
static cell init(int ident,int *tag,int *errorfound)
{
  cell i = 0;

  if (matchtoken(tSTRING)){
    /* lex() automatically stores strings in the literal table (and
     * increases "litidx")
     */
    if (ident==iVARIABLE) {
      error(6);         /* must be assigned to an array */
      litidx=1;         /* reset literal queue */
    } /* if */
    *tag=0;
  } else if (constexpr(&i,tag,NULL)){
    litadd(i);          /* store expression result in literal table */
  } else {
    if (errorfound!=NULL)
      *errorfound=TRUE;
  } /* if */
  return i;
}

/*  needsub
 *
 *  Get required array size
 */
static cell needsub(int *tag,constvalue_root **enumroot)
{
  cell val;
  symbol *sym;

  assert(tag!=NULL);
  *tag=0;
  if (enumroot!=NULL)
    *enumroot=NULL;         /* preset */
  if (matchtoken(']'))      /* we have already seen "[" */
    return 0;               /* zero size (like "char msg[]") */

  constexpr(&val,tag,&sym); /* get value (must be constant expression) */
  if (val<=0) {
    error(9);               /* negative array size is invalid; assumed zero */
    val=0;
  } /* if */
  needtoken(']');

  if (enumroot!=NULL) {
    /* get the field list for an enumeration */
    assert(*enumroot==NULL);/* should have been preset */
    assert(sym==NULL || sym->ident==iCONSTEXPR);
    if (sym!=NULL && (sym->usage & uENUMROOT)==uENUMROOT) {
      assert(sym->dim.enumlist!=NULL);
      *enumroot=sym->dim.enumlist;
    } /* if */
  } /* if */

  return val;               /* return array size */
}

/*  decl_const  - declare a single constant
 *
 */
static void decl_const(int vclass)
{
  char constname[sNAMEMAX+1];
  cell val;
  char *str;
  int tag,exprtag;
  int symbolline;
  int fstatic;
  symbol *sym;

  fstatic=(vclass==sGLOBAL && matchtoken(tSTATIC));
  insert_docstring_separator();         /* see comment in newfunc() */
  do {
    tag=pc_addtag(NULL);
    if (lex(&val,&str)!=tSYMBOL)        /* read in (new) token */
      error(20,str);                    /* invalid symbol name */
    symbolline=fline;                   /* save line where symbol was found */
    strcpy(constname,str);              /* save symbol name */
    needtoken('=');
    constexpr(&val,&exprtag,NULL);      /* get value */
    /* add_constant() checks for duplicate definitions */
    check_tagmismatch(tag,exprtag,FALSE,symbolline);
    sym=add_constant(constname,val,vclass,tag);
    if (sym!=NULL) {
      if (fstatic)
        sym->fnumber=fcurrent;
      sc_attachdocumentation(sym);/* attach any documentation to the constant */
    } /* if */
  } while (matchtoken(',')); /* enddo */   /* more? */
  needtoken(tTERM);
}

/*  decl_enum   - declare enumerated constants
 *
 */
static void decl_enum(int vclass,int fstatic)
{
  char enumname[sNAMEMAX+1],constname[sNAMEMAX+1];
  cell val,value,size;
  char *str;
  int tag,explicittag;
  int unique;
  int inctok;
  int warn_overflow,warn_noeffect;
  cell increment;
  constvalue_root *enumroot=NULL;
  symbol *enumsym=NULL;
  symbol *noeffect_sym=NULL;
  short filenum;

  filenum=fcurrent;

  /* get an explicit tag, if any (we need to remember whether an explicit
   * tag was passed, even if that explicit tag was "_:", so we cannot call
   * pc_addtag() here
   */
  if (lex(&val,&str)==tLABEL) {
    tag=pc_addtag(str);
    explicittag=TRUE;
  } else {
    lexpush();
    tag=0;
    explicittag=FALSE;
  } /* if */

  /* get optional enum name (also serves as a tag if no explicit tag was set) */
  if (lex(&val,&str)==tSYMBOL) {        /* read in (new) token */
    strcpy(enumname,str);               /* save enum name (last constant) */
    if (!explicittag)
      tag=pc_addtag(enumname);
  } else {
    lexpush();                          /* analyze again */
    enumname[0]='\0';
  } /* if */

  /* get the increment */
  increment=1;
  inctok=taADD;
  if (matchtoken('(')) {
    int tok=lex(&val,&str);
    if (tok==taADD || tok==taMULT || tok==taSHL) {
      inctok=tok;
      constexpr(&increment,NULL,NULL);
      if (tok==taSHL) {
        if (increment<0 || increment>=PAWN_CELL_SIZE)
          error(241);                   /* negative or too big shift count */
        if (increment<0)
          increment=0;
      } /* if */
    } else {
      lexpush();
    } /* if */
    needtoken(')');
  } /* if */

  if (!strempty(enumname)) {
    /* already create the root symbol, so the fields can have it as their "parent" */
    enumsym=add_constant(enumname,0,vclass,tag);
    if (enumsym!=NULL) {
      enumsym->usage |= uENUMROOT;
      unique=0;
      if (fstatic)
        enumsym->fnumber=filenum;
      /* if we redefined a root symbol of another enum, then we need to delete
       * the previous list of enum elements, otherwise it would be leaked */
      if (enumsym->dim.enumlist!=NULL)
        delete_consttable(enumsym->dim.enumlist);
    } /* if */
    /* start a new list for the element names */
    if ((enumroot=(constvalue_root*)malloc(sizeof(constvalue_root)))==NULL)
      error(103);                       /* insufficient memory (fatal error) */
    memset(enumroot,0,sizeof(constvalue_root));
  } /* if */

  needtoken('{');
  /* go through all constants */
  value=0;                              /* default starting value */
  warn_overflow=warn_noeffect=FALSE;
  do {
    int idxtag,fieldtag;
    int symline;
    symbol *sym;
    if (matchtoken('}')) {              /* quick exit if '}' follows ',' */
      lexpush();
      break;
    } /* if */
    symline=fline;
    idxtag=(enumname[0]=='\0') ? tag : pc_addtag(NULL); /* optional explicit item tag */
    if (needtoken(tSYMBOL)) {           /* read in (new) token */
      tokeninfo(&val,&str);             /* get the information */
      strcpy(constname,str);            /* save symbol name */
    } else {
      constname[0]='\0';
    } /* if */
    size=(inctok==taADD) ? increment : 1;/* default increment of 'val' */
    fieldtag=0;                         /* default field tag */
    if (matchtoken('[')) {
      constexpr(&size,&fieldtag,NULL);  /* get size */
      needtoken(']');
    } /* if */
    if (matchtoken('=')) {
      constexpr(&value,NULL,NULL);      /* get value */
      warn_overflow=warn_noeffect=FALSE;
    } else {
      if (warn_overflow) {
        int num=(inctok==taSHL) ? 242   /* shift overflow in enum element declaration */
                                : 246;  /* multiplication overflow in enum element declaration */
        errorset(sSETPOS,symline);
        error(num,constname);
        errorset(sSETPOS,-1);
        /* don't reset "warn_overflow" yet, we'll need to use it later */
      } /* if */
      if (warn_noeffect) {
        const char *name=noeffect_sym->name;
        str=sc_tokens[inctok-tFIRST];
        errorset(sSETPOS,noeffect_sym->lnumber);
        error(245,str,increment,name);  /* enum increment has no effect on zero value */
        errorset(sSETPOS,-1);
        warn_noeffect=FALSE;
      } /* if */
    } /* if */
    /* add_constant() checks whether a variable (global or local) or
     * a constant with the same name already exists
     */
    sym=add_constant(constname,value,vclass,tag);
    if (sym==NULL)
      continue;                         /* error message already given */
    /* modify the symbol only if it's not the current enum root symbol
     * being redefined by the user */
    if (sym!=enumsym) {
      /* clear the "enum root" flag and delete the list of enum elements,
       * in case we redefined a root symbol of another enum */
      if ((sym->usage & uENUMROOT)!=0) {
        sym->usage &= ~uENUMROOT;
        delete_consttable(sym->dim.enumlist);
      } /* if */
      /* set the item tag and the item size, for use in indexing arrays */
      sym->x.tags.index=idxtag;
      sym->x.tags.field=fieldtag;
      sym->dim.array.length=size;
      sym->dim.array.level=0;
      sym->parent=enumsym;
      if (enumsym)
        enumsym->child=sym;

      if (fstatic)
        sym->fnumber=filenum;

      if (enumroot!=NULL && find_constval_byval(enumroot,value)==NULL)
        unique++;

      /* add the constant to a separate list as well */
      if (enumroot!=NULL) {
        sym->usage |= uENUMFIELD;
        append_constval(enumroot,constname,value,tag);
      } /* if */
    } /* if */
    if (inctok!=taADD && value==0 && increment!=0
        && noeffect_sym==NULL && warn_overflow==FALSE) {
      warn_noeffect=TRUE;
      noeffect_sym=sym;
    } /* if */
    warn_overflow=FALSE;
    if (inctok==taADD) {
      value+=size;
    } else if (inctok==taMULT) {
#if PAWN_CELL_SIZE<64
      /* use a bigger type to detect overflow */
      int64_t t=(int64_t)value*(int64_t)size*(int64_t)increment;
      if (t>(int64_t)CELL_MAX || t<(~(int64_t)CELL_MAX))
#else
      /* casting to a bigger type isn't possible as we don't have int128_t,
       * so we'll have to use slower division */
      cell t=size*increment;
      if (value!=0 && (value*t)/value!=t)
#endif
        warn_overflow=TRUE;
      value*=(size*increment);
    } else { // taSHL
      if (increment>0 && increment<PAWN_CELL_SIZE
          && ((ucell)value>=((ucell)1 << (PAWN_CELL_SIZE-increment))))
        warn_overflow=TRUE;
      value*=(size << increment);
    } /* if */
  } while (matchtoken(','));
  needtoken('}');       /* terminates the constant list */
  matchtoken(';');      /* eat an optional ; */

  /* set the enum name to the "next" value (typically the last value plus one) */
  if (enumsym!=NULL) {
    assert((enumsym->usage & uENUMROOT)!=0);
    enumsym->addr=value;
    enumsym->x.tags.unique=unique;
    /* assign the constant list */
    assert(enumroot!=NULL);
    enumsym->dim.enumlist=enumroot;
    sc_attachdocumentation(enumsym);  /* attach any documentation to the enumeration */
  } /* if */
}

static int getstates(const char *funcname)
{
  char fsaname[sNAMEMAX+1],statename[sNAMEMAX+1];
  cell val;
  char *str;
  constvalue *automaton;
  constvalue *state;
  int fsa,islabel;
  int *list;
  int count,listsize,state_id;

  if (!matchtoken('<'))
    return 0;
  if (matchtoken('>'))
    return -1;          /* special construct: all other states (fall-back) */

  count=0;
  listsize=0;
  list=NULL;
  fsa=-1;

  do {
    if (!(islabel=matchtoken(tLABEL)) && !needtoken(tSYMBOL))
      break;
    tokeninfo(&val,&str);
    assert(strlen(str)<arraysize(fsaname));
    strcpy(fsaname,str);  /* assume this is the name of the automaton */
    if (islabel || matchtoken(':')) {
      /* token is an automaton name, add the name and get a new token */
      if (!needtoken(tSYMBOL))
        break;
      tokeninfo(&val,&str);
      assert(strlen(str)<arraysize(statename));
      strcpy(statename,str);
    } else {
      /* the token was the state name (part of an anynymous automaton) */
      assert(strlen(fsaname)<arraysize(statename));
      strcpy(statename,fsaname);
      fsaname[0]='\0';
    } /* if */
    if (fsa<0 || fsaname[0]!='\0') {
      automaton=automaton_add(fsaname);
      assert(automaton!=NULL);
      if (fsa>=0 && automaton->index!=fsa)
        error(83,funcname); /* multiple automatons for a single function/variable */
      fsa=automaton->index;
    } /* if */
    state=state_add(statename,fsa);
    /* add this state to the state combination list (it will be attached to the
     * automaton later) */
    state_buildlist(&list,&listsize,&count,(int)state->value);
  } while (matchtoken(','));
  needtoken('>');

  if (count>0) {
    assert(automaton!=NULL);
    assert(fsa>=0);
    state_id=state_addlist(list,count,fsa);
    assert(state_id>0);
  } else {
    /* error is already given */
    state_id=0;
  } /* if */
  free(list);

  return state_id;
}

static void attachstatelist(symbol *sym, int state_id)
{
  assert(sym!=NULL);

  if (state_id!=0) {
    /* add the state list id */
    constvalue *stateptr;
    if (sym->states==NULL) {
      if ((sym->states=(constvalue_root*)malloc(sizeof(constvalue_root)))==NULL)
        error(103);             /* insufficient memory (fatal error) */
      memset(sym->states,0,sizeof(constvalue_root));
    } /* if */
    /* see whether the id already exists (add new state only if it does not
     * yet exist
     */
    assert(sym->states!=NULL);
    for (stateptr=sym->states->first; stateptr!=NULL && stateptr->index!=state_id; stateptr=stateptr->next)
      /* nothing */;
    assert(state_id<=SHRT_MAX);
    if (stateptr==NULL)
      append_constval(sym->states,"",code_idx,(short)state_id);
    else if (stateptr->value==0)
      stateptr->value=code_idx;
    else
      error(84,sym->name);
    /* also check for another conflicting situation: a fallback function
     * without any states
     */
    if (state_id==-1 && sc_status!=statFIRST) {
      /* in the second round, all states should have been accumulated */
      assert(sym->states!=NULL);
      for (stateptr=sym->states->first; stateptr!=NULL && stateptr->index==-1; stateptr=stateptr->next)
        /* nothing */;
      if (stateptr==NULL)
        error(85,sym->name);      /* no states are defined for this function */
    } /* if */
  } /* if */
}

/*
 *  Finds a function in the global symbol table or creates a new entry.
 *  It does some basic processing and error checking.
 */
SC_FUNC symbol *fetchfunc(char *name,int tag)
{
  symbol *sym;

  if ((sym=findglb(name,sGLOBAL))!=NULL) {/* already in symbol table? */
    if (sym->ident!=iFUNCTN) {
      error(21,name);                     /* yes, but not as a function */
      return NULL;                        /* make sure the old symbol is not damaged */
    } else if ((sym->usage & uNATIVE)!=0) {
      error(21,name);                     /* yes, and it is a native */
    } /* if */
    assert(sym->vclass==sGLOBAL);
    if ((sym->usage & uPROTOTYPED)!=0 && sym->tag!=tag)
      error(25);                          /* mismatch from earlier prototype */
    if ((sym->usage & uDEFINE)==0) {
      /* as long as the function stays undefined, update the address and the tag */
      if (sym->states==NULL)
        sym->addr=code_idx;
      sym->tag=tag;
    } /* if */
  } else {
    /* don't set the "uDEFINE" flag; it may be a prototype */
    sym=addsym(name,code_idx,iFUNCTN,sGLOBAL,tag,0);
    assert(sym!=NULL);          /* fatal error 103 must be given on error */
    /* assume no arguments */
    sym->dim.arglist=(arginfo*)calloc(1,sizeof(arginfo));
    /* set library ID to NULL (only for native functions) */
    sym->x.lib=NULL;
    /* set the required stack size to zero (only for non-native functions) */
    sym->x.stacksize=1;         /* 1 for PROC opcode */
  } /* if */
  pragma_deprecated(sym);

  return sym;
}

/* This routine adds symbolic information for each argument.
 */
static void define_args(void)
{
  symbol *sym;

  /* At this point, no local variables have been declared. All
   * local symbols are function arguments.
   */
  sym=loctab.next;
  while (sym!=NULL) {
    assert(sym->ident!=iLABEL);
    assert(sym->vclass==sLOCAL);
    markexpr(sLDECL,sym->name,sym->addr); /* mark for better optimization */
    sym=sym->next;
  } /* while */
}

static int operatorname(char *name)
{
  int opertok;
  char *str;
  cell val;

  assert(name!=NULL);

  /* check the operator */
  opertok=lex(&val,&str);
  switch (opertok) {
  case '+':
  case '-':
  case '*':
  case '/':
  case '%':
  case '>':
  case '<':
  case '!':
  case '~':
  case '=':
    name[0]=(char)opertok;
    name[1]='\0';
    break;
  case tINC:
    strcpy(name,"++");
    break;
  case tDEC:
    strcpy(name,"--");
    break;
  case tlEQ:
    strcpy(name,"==");
    break;
  case tlNE:
    strcpy(name,"!=");
    break;
  case tlLE:
    strcpy(name,"<=");
    break;
  case tlGE:
    strcpy(name,">=");
    break;
  default:
    name[0]='\0';
    error(7);           /* operator cannot be redefined (or bad operator name) */
    return 0;
  } /* switch */

  return opertok;
}

static int operatoradjust(int opertok,symbol *sym,char *opername,int resulttag)
{
  int tags[2]={0,0};
  int count=0;
  arginfo *arg;
  char tmpname[sNAMEMAX+1];
  symbol *oldsym;

  if (opertok==0)
    return TRUE;

  assert(sym!=NULL && sym->ident==iFUNCTN && sym->dim.arglist!=NULL);
  /* count arguments and save (first two) tags */
  while (arg=&sym->dim.arglist[count], arg->ident!=0) {
    if (count<2) {
      if (arg->numtags>1)
        error(65,count+1);  /* function argument may only have a single tag */
      else if (arg->numtags==1)
        tags[count]=arg->tags[0];
    } /* if */
    if (opertok=='~' && count==0) {
      if (arg->ident!=iREFARRAY)
        error(73,arg->name);/* must be an array argument */
    } else {
      if (arg->ident!=iVARIABLE)
        error(66,arg->name);/* must be non-reference argument */
    } /* if */
    if (arg->hasdefault)
      error(59,arg->name);  /* arguments of an operator may not have a default value */
    count++;
  } /* while */

  /* for '!', '++' and '--', count must be 1
   * for '-', count may be 1 or 2
   * for '=', count must be 1, and the resulttag is also important
   * for all other (binary) operators and the special '~' operator, count must be 2
   */
  switch (opertok) {
  case '!':
  case '=':
  case tINC:
  case tDEC:
    if (count!=1)
      error(62);      /* number or placement of the operands does not fit the operator */
    break;
  case '-':
    if (count!=1 && count!=2)
      error(62);      /* number or placement of the operands does not fit the operator */
    break;
  default:
    if (count!=2)
      error(62);      /* number or placement of the operands does not fit the operator */
  } /* switch */

  if (tags[0]==0 && ((opertok!='=' && tags[1]==0) || (opertok=='=' && resulttag==0)))
    error(64);        /* cannot change predefined operators */

  /* change the operator name */
  assert(!strempty(opername));
  operator_symname(tmpname,opername,tags[0],tags[1],count,resulttag);
  if ((oldsym=findglb(tmpname,sGLOBAL))!=NULL) {
    int i;
    if ((oldsym->usage & uDEFINE)!=0) {
      char errname[2*sNAMEMAX+16];
      funcdisplayname(errname,tmpname);
      error(21,errname);        /* symbol already defined */
    } /* if */
    sym->usage|=oldsym->usage;  /* copy flags from the previous definition */
    for (i=0; i<oldsym->numrefers; i++)
      if (oldsym->refer[i]!=NULL)
        refer_symbol(sym,oldsym->refer[i]);
    delete_symbol(&glbtab,oldsym);
  } /* if */
  rename_symbol(sym,tmpname);

  /* operators should return a value, except the '~' operator */
  if (opertok!='~')
    sym->usage |= uRETVALUE;

  return TRUE;
}

static int check_operatortag(int opertok,int resulttag,char *opername)
{
  assert(opername!=NULL && !strempty(opername));
  switch (opertok) {
  case '!':
  case '<':
  case '>':
  case tlEQ:
  case tlNE:
  case tlLE:
  case tlGE:
    if (resulttag!=BOOLTAG) {
      error(63,opername,"bool:"); /* operator X requires a "bool:" result tag */
      return FALSE;
    } /* if */
    break;
  case '~':
    if (resulttag!=0) {
      error(63,opername,"_:");    /* operator "~" requires a "_:" result tag */
      return FALSE;
    } /* if */
    break;
  } /* switch */
  return TRUE;
}

static char *tag2str(char *dest,int tag)
{
  tag &= TAGMASK;
  assert(tag>=0);
  sprintf(dest,"0%x",tag);
  return isdigit(dest[1]) ? &dest[1] : dest;
}

SC_FUNC char *operator_symname(char *symname,char *opername,int tag1,int tag2,int numtags,int resulttag)
{
  char tagstr1[10], tagstr2[10];
  int opertok;

  assert(numtags>=1 && numtags<=2);
  opertok= (opername[1]=='\0') ? opername[0] : 0;
  if (opertok=='=')
    sprintf(symname,"%s%s%s",tag2str(tagstr1,resulttag),opername,tag2str(tagstr2,tag1));
  else if (numtags==1 || opertok=='~')
    sprintf(symname,"%s%s",opername,tag2str(tagstr1,tag1));
  else
    sprintf(symname,"%s%s%s",tag2str(tagstr1,tag1),opername,tag2str(tagstr2,tag2));
  return symname;
}

static int parse_funcname(char *fname,int *tag1,int *tag2,char *opname)
{
  char *ptr,*name;
  int unary;

  /* tags are only positive, so if the function name starts with a '-',
   * the operator is an unary '-' or '--' operator.
   */
  if (*fname=='-') {
    *tag1=0;
    unary=TRUE;
    ptr=fname;
  } else {
    *tag1=(int)strtol(fname,&ptr,16);
    unary= ptr==fname;  /* unary operator if it doesn't start with a tag name */
  } /* if */
  assert(!unary || *tag1==0);
  assert(*ptr!='\0');
  for (name=opname; !isdigit(*ptr); )
    *name++ = *ptr++;
  *name='\0';
  *tag2=(int)strtol(ptr,NULL,16);
  return unary;
}

static constvalue *find_tag_byval(int tag)
{
  constvalue *tagsym;
  tagsym=find_constval_byval(&tagname_tab,tag & ~PUBLICTAG);
  if (tagsym==NULL)
    tagsym=find_constval_byval(&tagname_tab,tag | PUBLICTAG);
  return tagsym;
}

SC_FUNC void check_index_tagmismatch(char *symname,int expectedtag,int actualtag,int allowcoerce,int errline)
{
  assert(symname!=NULL);
  if (!matchtag(expectedtag,actualtag,allowcoerce)) {
    constvalue *tagsym;
    char expected_tagname[sNAMEMAX+3]="none (\"_\"),",actual_tagname[sNAMEMAX+2]="none (\"_\")"; /* two extra characters for quotes */
    if(expectedtag!=0) {
      tagsym=find_tag_byval(expectedtag);
      sprintf(expected_tagname,"\"%s\",",(tagsym!=NULL) ? tagsym->name : "-unknown-");
    } /* if */
    if(actualtag!=0) {
      tagsym=find_tag_byval(actualtag);
      sprintf(actual_tagname,"\"%s\"",(tagsym!=NULL) ? tagsym->name : "-unknown-");
    } /* if */
    if(errline>0)
      errorset(sSETPOS,errline);
    error(229,symname,expected_tagname,actual_tagname); /* index tag mismatch */
    if(errline>0)
      errorset(sSETPOS,-1);
  } /* if */
}

SC_FUNC void check_tagmismatch(int formaltag,int actualtag,int allowcoerce,int errline)
{
  if (!matchtag(formaltag,actualtag,allowcoerce)) {
    constvalue *tagsym;
    char formal_tagname[sNAMEMAX+3]="none (\"_\"),",actual_tagname[sNAMEMAX+2]="none (\"_\")"; /* two extra characters for quotes */
    if(formaltag!=0) {
      tagsym=find_tag_byval(formaltag);
      sprintf(formal_tagname,"\"%s\",",(tagsym!=NULL) ? tagsym->name : "-unknown-");
    } /* if */
    if(actualtag!=0) {
      tagsym=find_tag_byval(actualtag);
      sprintf(actual_tagname,"\"%s\"",(tagsym!=NULL) ? tagsym->name : "-unknown-");
    } /* if */
    if(errline>0)
      errorset(sSETPOS,errline);
    error(213,"tag",formal_tagname,actual_tagname); /* tag mismatch */
    if(errline>0)
      errorset(sSETPOS,-1);
  } /* if */
}

SC_FUNC void check_tagmismatch_multiple(int formaltags[],int numtags,int actualtag,int errline)
{
  if (!checktag(formaltags, numtags, actualtag)) {
    int i;
    constvalue *tagsym;
    char formal_tagnames[sLINEMAX+1]="",actual_tagname[sNAMEMAX+2]="none (\"_\")";
    int notag_allowed=FALSE,add_comma=FALSE;
    size_t size;
    for (i=0; i<numtags; i++) {
      if(formaltags[i]!=0) {
        if((i+1)==numtags && add_comma==TRUE && notag_allowed==FALSE)
          strlcat(formal_tagnames,", or ",arraysize(formal_tagnames));
        else if(add_comma)
          strlcat(formal_tagnames,", ",arraysize(formal_tagnames));
        add_comma=TRUE;
        tagsym=find_tag_byval(formaltags[i]);
        size=snprintf(formal_tagnames,
                      sizeof(formal_tagnames),
                      "%s\"%s\"",
                      formal_tagnames,
                      (tagsym!=NULL) ? tagsym->name : "-unknown-");
        if(size>=sizeof(formal_tagnames))
          break;
      } else {
        notag_allowed=TRUE;
      } /* if */
    } /* for */
    if(notag_allowed==TRUE) {
      if(add_comma==TRUE)
        strlcat(formal_tagnames,", or ",arraysize(formal_tagnames));
      strlcat(formal_tagnames,"none (\"_\")",arraysize(formal_tagnames));
    } /* if */
    strlcat(formal_tagnames,(numtags==1) ? "," : ";",arraysize(formal_tagnames));
    if(actualtag!=0) {
      tagsym=find_tag_byval(actualtag);
      sprintf(actual_tagname,"\"%s\"",(tagsym!=NULL) ? tagsym->name : "-unknown-");
    } /* if */
    if(errline>0)
      errorset(sSETPOS,errline);
    error(213,(numtags==1) ? "tag" : "tags",formal_tagnames,actual_tagname); /* tag mismatch */
    if(errline>0)
      errorset(sSETPOS,-1);
  } /* if */
}

SC_FUNC char *funcdisplayname(char *dest,char *funcname)
{
  int tags[2];
  char opname[10];
  constvalue *tagsym[2];
  int unary;

  if (isalpha(*funcname) || *funcname=='_' || *funcname==PUBLIC_CHAR || *funcname=='\0') {
    if (dest!=funcname)
      strcpy(dest,funcname);
    return dest;
  } /* if */

  unary=parse_funcname(funcname,&tags[0],&tags[1],opname);
  tagsym[1]=find_tag_byval(tags[1]);
  assert(tagsym[1]!=NULL);
  if (unary) {
    sprintf(dest,"operator%s(%s:)",opname,tagsym[1]->name);
  } else {
    tagsym[0]=find_tag_byval(tags[0]);
    assert(tagsym[0]!=NULL);
    /* special case: the assignment operator has the return value as the 2nd tag */
    if (opname[0]=='=' && opname[1]=='\0')
      sprintf(dest,"%s:operator%s(%s:)",tagsym[0]->name,opname,tagsym[1]->name);
    else
      sprintf(dest,"operator%s(%s:,%s:)",opname,tagsym[0]->name,tagsym[1]->name);
  } /* if */
  return dest;
}

static void check_reparse(symbol *sym)
{
  /* if the function was used before being declared, and it has a tag for the
   * result, add a third pass (as second "skimming" parse) because the function
   * result may have been used with user-defined operators, which have now
   * been incorrectly flagged (as the return tag was unknown at the time of
   * the call)
   */
  if ((sym->usage & (uPROTOTYPED | uREAD))==uREAD && sym->tag!=0) {
    int curstatus=sc_status;
    sc_status=statWRITE;  /* temporarily set status to WRITE, so the warning isn't blocked */
    error(208);
    sc_status=curstatus;
    sc_reparse=TRUE;      /* must add another pass to "initial scan" phase */
  } /* if */
}

static void funcstub(int fnative)
{
  int tok,tag,fpublic;
  char *str;
  cell val,size;
  char symbolname[sNAMEMAX+1];
  int idxtag[sDIMEN_MAX];
  int dim[sDIMEN_MAX];
  int numdim;
  symbol *sym,*sub;
  int opertok;
  unsigned int bck_attributes;
  char *bck_deprecate;  /* in case the user tries to use __pragma("deprecated")
                         * on a function argument */

  opertok=0;
  lastst=0;
  litidx=0;                     /* clear the literal pool */
  assert(loctab.next==NULL);    /* local symbol table should be empty */

  tag=pc_addtag(NULL);			/* get the tag of the return value */
  numdim=0;
  while (matchtoken('[')) {
    /* the function returns an array, get this tag for the index and the array
     * dimensions
     */
    if (numdim == sDIMEN_MAX) {
      error(53);                /* exceeding maximum number of dimensions */
      return;
    } /* if */
    size=needsub(&idxtag[numdim],NULL); /* get size; size==0 for "var[]" */
    if (size==0)
      error(9);                 /* invalid array size */
    #if INT_MAX < LONG_MAX
      if (size > INT_MAX)
        error(105);             /* overflow, exceeding capacity */
    #endif
    dim[numdim++]=(int)size;
  } /* while */

  tok=lex(&val,&str);
  fpublic=(tok==tPUBLIC) || (tok==tSYMBOL && str[0]==PUBLIC_CHAR);
  if (fnative) {
    if (fpublic || tok==tSTOCK || tok==tSTATIC || (tok==tSYMBOL && *str==PUBLIC_CHAR))
      error(42);                /* invalid combination of class specifiers */
  } else {
    if (tok==tPUBLIC || tok==tSTOCK || tok==tSTATIC)
      tok=lex(&val,&str);
  } /* if */

  if (tok==t__PRAGMA) {
    dopragma();
    tok=lex(&val,&str);
  } /* if */

  if (tok==tOPERATOR) {
    if (numdim!=0)
      error(10);                /* invalid function or declaration */
    opertok=operatorname(symbolname);
    if (opertok==0)
      return;                   /* error message already given */
    check_operatortag(opertok,tag,symbolname);
  } else {
    if (tok!=tSYMBOL && freading) {
      error(10);                /* illegal function or declaration */
      return;
    } /* if */
    strcpy(symbolname,str);
  } /* if */
  needtoken('(');               /* only functions may be native/forward */

  sym=fetchfunc(symbolname,tag);/* get a pointer to the function entry */
  if (sym==NULL)
    return;
  if (fnative) {
    sym->usage=(short)(uNATIVE | uRETVALUE | uDEFINE | (sym->usage & uPROTOTYPED));
    sym->x.lib=curlibrary;
  } else if (fpublic && opertok==0) {
    sym->usage|=uPUBLIC;
  } /* if */
  sym->usage|=uFORWARD;
  check_reparse(sym);

  bck_attributes=pc_attributes;
  bck_deprecate=pc_deprecate;
  pc_attributes=0;
  pc_deprecate=NULL;

  declargs(sym,FALSE);
  /* "declargs()" found the ")" */
  sc_attachdocumentation(sym);  /* attach any documentation to the function */
  if (!operatoradjust(opertok,sym,symbolname,tag))
    sym->usage &= ~uDEFINE;
  if (fpublic && opertok!=0) {
    char symname[2*sNAMEMAX+16];  /* allow space for user defined operators */
    funcdisplayname(symname,sym->name);
    error(56,symname);  /* operators cannot be public */
  } /* if */

  if (getstates(symbolname)!=0) {
    if (fnative || opertok!=0)
      error(82);                /* native functions and operators may not have states */
    else
      error(231);               /* ignoring state specifications on forward declarations */
  } /* if */

  /* for a native operator, also need to specify an "exported" function name;
   * for a native function, this is optional
   */
  if (fnative) {
    if ((opertok!=0) ? needtoken('=') : matchtoken('=')) {
      /* allow number or symbol */
      if (matchtoken(tSYMBOL)) {
        tokeninfo(&val,&str);
        insert_alias(sym->name,str);
      } else {
        constexpr(&val,NULL,NULL);
        sym->addr=val;
        /* At the moment, I have assumed that this syntax is only valid if
         * val < 0. To properly mix "normal" native functions and indexed
         * native functions, one should use negative indices anyway.
         * Special code for a negative index in sym->addr exists in SC4.C
         * (ffcall()) and in SC6.C (the loops for counting the number of native
         * variables and for writing them).
         */
      } /* if */
    } /* if */
  } /* if */

  pc_deprecate=bck_deprecate;
  pc_attributes=bck_attributes;
  if (matchtoken(t__PRAGMA))
    dopragma();
  pragma_apply(sym);

  needtoken(tTERM);

  if (numdim>0) {
    if (sym->child==NULL) {
      /* attach the array to the function symbol */
      assert(sym!=NULL);
      assert(curfunc==NULL);
      curfunc=sym;
      sub=addvariable(symbolname,0,iREFARRAY,sGLOBAL,tag,dim,numdim,idxtag,0);
      curfunc=NULL;
      sub->parent=sym;
      if (sym!=NULL)
        sym->child=sub;
    } else {
      /* the array is already created and attached to the function (which means
       * this is not the first compilation pass), but we need to make sure the
       * current dimensions match the dimensions declared at the first pass, as
       * we can't rely on the code being the same on all passes (mainly because
       * of conditional compilation, e.g. '#if defined <function name>') */
      if (numdim!=sym->child->dim.array.level+1) {
        error(25);              /* function heading differs from prototype */
      } else {
        int i=0;
        sub=sym->child;
        do {
          if (dim[i]!=sub->dim.array.length) {
            error(25);          /* function heading differs from prototype */
            break;
          } /* if */
          sub=sub->child;
        } while (++i<numdim);
      } /* if */
    } /* if */
  } /* if */

  litidx=0;                     /* clear the literal pool */
  delete_symbols(&loctab,0,TRUE,TRUE);/* clear local variables queue */
}

/*  newfunc    - begin a function
 *
 *  This routine is called from "parse" and tries to make a function
 *  out of the following text
 *
 *  Global references: funcstatus,lastst,litidx
 *                     rettype  (altered)
 *                     curfunc  (altered)
 *                     declared (altered)
 *                     glb_declared (altered)
 *                     sc_alignnext (altered)
 */
static int newfunc(char *firstname,int firsttag,int fpublic,int fstatic,int stock)
{
  symbol *sym,*lvar,*depend;
  int argcnt,tok,tag,funcline,i;
  int opertok,opererror;
  char symbolname[sNAMEMAX+1];
  char *str;
  cell val,cidx,glbdecl;
  short filenum;
  int state_id;
  unsigned int bck_attributes;
  char *bck_deprecate;  /* in case the user tries to use __pragma("deprecated")
                         * on a function argument */

  assert(litidx==0);    /* literal queue should be empty */
  litidx=0;             /* clear the literal pool (should already be empty) */
  opertok=0;
  lastst=0;             /* no statement yet */
  cidx=0;               /* just to avoid compiler warnings */
  glbdecl=0;
  assert(loctab.next==NULL);    /* local symbol table should be empty */
  filenum=fcurrent;     /* save file number at the start of the declaration */

  if (firstname!=NULL) {
    assert(strlen(firstname)<=sNAMEMAX);
    strcpy(symbolname,firstname);       /* save symbol name */
    tag=firsttag;
  } else {
    tag= (firsttag>=0) ? firsttag : pc_addtag(NULL);
    tok=lex(&val,&str);
    if (tok==tNATIVE || (tok==tPUBLIC && stock))
      error(42);                /* invalid combination of class specifiers */
    if (tok==t__PRAGMA) {
      dopragma();
      tok=lex(&val,&str);
    } /* if */
    if (tok==tOPERATOR) {
      opertok=operatorname(symbolname);
      if (opertok==0)
        return TRUE;            /* error message already given */
      check_operatortag(opertok,tag,symbolname);
    } else {
      if (tok!=tSYMBOL && freading) {
        error(20,str);          /* invalid symbol name */
        return FALSE;
      } /* if */
      assert(strlen(str)<=sNAMEMAX);
      strcpy(symbolname,str);
    } /* if */
  } /* if */
  /* check whether this is a function or a variable declaration */
  if (!matchtoken('('))
    return FALSE;
  /* so it is a function, proceed */
  funcline=fline;               /* save line at which the function is defined */
  if (symbolname[0]==PUBLIC_CHAR) {
    fpublic=TRUE;               /* implicitly public function */
    if (stock)
      error(42);                /* invalid combination of class specifiers */
  } /* if */
  sym=fetchfunc(symbolname,tag);/* get a pointer to the function entry */
  if (sym==NULL || (sym->usage & uNATIVE)!=0)
    return TRUE;                /* it was recognized as a function declaration, but not as a valid one */
  if (fpublic && opertok==0)
    sym->usage|=uPUBLIC;
  if (fstatic)
    sym->fnumber=filenum;
  check_reparse(sym);
  /* we want public functions to be explicitly prototyped, as they are called
   * from the outside
   */
  if (fpublic && (sym->usage & uFORWARD)==0 && opertok==0)
    error(235,symbolname);
  bck_attributes=pc_attributes;
  bck_deprecate=pc_deprecate;
  pc_attributes=0;
  pc_deprecate=NULL;
  /* declare all arguments */
  argcnt=declargs(sym,TRUE);
  opererror=!operatoradjust(opertok,sym,symbolname,tag);
  if (fpublic && opertok!=0) {
    char symname[2*sNAMEMAX+16];  /* allow space for user defined operators */
    funcdisplayname(symname,sym->name);
    error(56,symname);  /* operators cannot be public */
  } /* if */
  if (strcmp(symbolname,uMAINFUNC)==0 || strcmp(symbolname,uENTRYFUNC)==0) {
    if (argcnt>0)
      error(5);         /* "main()" and "entry()" functions may not have any arguments */
    sym->usage|=uREAD;  /* "main()" is the program's entry point: always used */
  } /* if */
  state_id=getstates(symbolname);
  if (state_id>0 && (opertok!=0 || strcmp(symbolname,uMAINFUNC)==0))
    error(82);          /* operators may not have states, main() may neither */
  pc_deprecate=bck_deprecate;
  pc_attributes=bck_attributes;
  if (matchtoken(t__PRAGMA))
    dopragma();
  pragma_apply(sym);
  /* "declargs()" found the ")"; if a ";" appears after this, it was a
   * prototype */
  if (matchtoken(';')) {
    sym->usage|=uFORWARD;
    if (!sc_needsemicolon)
      error(218);       /* old style prototypes used with optional semicolons */
    if (state_id!=0)
      error(231);       /* state specification on forward declaration is ignored */
    delete_symbols(&loctab,0,TRUE,TRUE);  /* prototype is done; forget everything */
    return TRUE;
  } /* if */
  attachstatelist(sym,state_id);
  /* so it is not a prototype, proceed */
  /* if this is a function that is not referred to (this can only be detected
   * in the second stage), shut code generation off */
  if (sc_status==statWRITE && (sym->usage & uREAD)==0 && !fpublic) {
    sc_status=statSKIP;
    cidx=code_idx;
    glbdecl=glb_declared;
  } /* if */
  if ((sym->usage & uDEFINE)!=0 && (sym->states==NULL || state_id==0))
    error(21,sym->name); /* function already defined, either without states or the current definition has no states */
  if ((sym->flags & flagDEPRECATED)!=0 && fpublic) {
    char *ptr= (sym->documentation!=NULL) ? sym->documentation : "";
    error(234,symbolname,ptr);  /* deprecated (definitely a public function) */
  } /* if */
  if (pc_naked) {
    sym->flags|=flagNAKED;
    pc_naked=FALSE;
  } /* if */
  begcseg();
  sym->usage|=uDEFINE;  /* set the definition flag */
  if (stock)
    sym->usage|=uSTOCK;
  if (opertok!=0 && opererror)
    sym->usage &= ~uDEFINE;
  /* if the function has states, dump the label to the start of the function */
  if (state_id!=0) {
    constvalue *ptr=sym->states->first;
    while (ptr!=NULL) {
      assert(sc_status!=statWRITE || !strempty(ptr->name));
      if (ptr->index==state_id) {
        setlabel((int)strtol(ptr->name,NULL,16));
        break;
      } /* if */
      ptr=ptr->next;
    } /* while */
  } /* if */
  startfunc(sym->name,(sym->flags & flagNAKED)==0); /* creates stack frame */
  insert_dbgline(funcline);
  setline(FALSE);
  if (sc_alignnext) {
    alignframe(sc_dataalign);
    sc_alignnext=FALSE;
  } /* if */
  declared=0;           /* number of local cells */
  rettype=(sym->usage & uRETVALUE);      /* set "return type" variable */
  curfunc=sym;
  define_args();        /* add the symbolic info for the function arguments */
  #if !defined SC_LIGHT
    if (matchtoken('{')) {
      lexpush();
    } else {
      /* Insert a separator so that comments following the statement will not
       * be attached to this function; they should be attached to the next
       * function. This is not a problem for functions having a compound block,
       * because the closing brace is an explicit "end token" for the function.
       * With single statement functions, the preprocessor may overread the
       * source code before the parser determines an "end of statement".
       */
      insert_docstring_separator();
    } /* if */
  #endif
  sc_curstates=state_id;/* set state id, for accessing global state variables */
  statement(NULL,FALSE);
  sc_curstates=0;
  if ((rettype & uRETVALUE)!=0)
    sym->usage|=uRETVALUE;
  if (declared!=0 && (curfunc->flags & flagNAKED)==0) {
    /* This happens only in a very special (and useless) case, where a function
     * has only a single statement in its body (no compound block) and that
     * statement declares a new variable
     */
    modstk((int)declared*sizeof(cell)); /* remove all local variables */
    declared=0;
  } /* if */
  if (!isterminal(lastst) && lastst!=tGOTO && (sym->flags & flagNAKED)==0) {
    destructsymbols(&loctab,0);
    ldconst(0,sPRI);
    ffret(strcmp(sym->name,uENTRYFUNC)!=0);
    if ((sym->usage & uRETVALUE)!=0) {
      char symname[2*sNAMEMAX+16];  /* allow space for user defined operators */
      funcdisplayname(symname,sym->name);
      error(209,symname);       /* function should return a value */
    } /* if */
  } /* if */
  endfunc();
  sym->codeaddr=code_idx;
  sc_attachdocumentation(sym);  /* attach collected documentation to the function */
  if (litidx) {                 /* if there are literals defined */
    glb_declared+=litidx;
    begdseg();                  /* flip to DATA segment */
    dumplits();                 /* dump literal strings */
    litidx=0;
  } /* if */
  for (i=0; i<argcnt; i++) {
    if (sym->dim.arglist[i].ident==iREFARRAY
        && (lvar=findloc(sym->dim.arglist[i].name))!=NULL) {
      if ((sym->dim.arglist[i].usage & uWRITTEN)==0) {
        /* check if the argument was written in this definition */
        for (depend=lvar; depend!=NULL; depend=depend->child) {
          if ((depend->usage & uWRITTEN)!=0) {
            sym->dim.arglist[i].usage|=depend->usage & uWRITTEN;
            break;
          } /* if */
        } /* for */
      } /* if */
      /* mark argument as written if it was written in another definition */
      lvar->usage|=sym->dim.arglist[i].usage & uWRITTEN;
    } /* if */
  } /* for */

  testsymbols(&loctab,0,TRUE,TRUE);     /* test for unused arguments and labels */
  delete_symbols(&loctab,0,TRUE,TRUE);  /* clear local variables queue */
  assert(loctab.next==NULL);
  curfunc=NULL;
  if (sc_status==statSKIP) {
    sc_status=statWRITE;
    code_idx=cidx;
    glb_declared=glbdecl;
  } /* if */
  return TRUE;
}

static int argcompare(arginfo *a1,arginfo *a2)
{
  int result,level,i;

  result= strcmp(a1->name,a2->name)==0;     /* name */
  if (result)
    result= a1->ident==a2->ident;           /* type/class */
  if (result)
    result= a1->usage==a2->usage;           /* "const" flag */
  if (result)
    result= a1->numtags==a2->numtags;       /* tags (number and names) */
  for (i=0; result && i<a1->numtags; i++)
    result= a1->tags[i]==a2->tags[i];
  if (result)
    result= a1->numdim==a2->numdim;         /* array dimensions & index tags */
  for (level=0; result && level<a1->numdim; level++)
    result= a1->dim[level]==a2->dim[level];
  for (level=0; result && level<a1->numdim; level++)
    result= a1->idxtag[level]==a2->idxtag[level];
  if (result)
    result= a1->hasdefault==a2->hasdefault; /* availability of default value */
  if (a1->hasdefault) {
    if (a1->ident==iREFARRAY) {
      if (result)
        result= a1->defvalue.array.size==a2->defvalue.array.size;
      if (result)
        result= a1->defvalue.array.arraysize==a2->defvalue.array.arraysize;
      if (result)
        result=(memcmp(a1->defvalue.array.data,a2->defvalue.array.data,a1->defvalue.array.size*sizeof(cell))==0);
    } else {
      if (result) {
        if ((a1->hasdefault & uSIZEOF)!=0 || (a1->hasdefault & uTAGOF)!=0)
          result= a1->hasdefault==a2->hasdefault
                  && strcmp(a1->defvalue.size.symname,a2->defvalue.size.symname)==0
                  && a1->defvalue.size.level==a2->defvalue.size.level;
        else if ((a1->hasdefault & uTAGOF_TAG)!=0)
          a1->defvalue.val=a2->defvalue.val;
        else
          result= a1->defvalue.val==a2->defvalue.val;
      } /* if */
    } /* if */
    if (result)
      result= a1->defvalue_tag==a2->defvalue_tag;
  } /* if */
  return result;
}

/*  declargs()
 *
 *  This routine adds an entry in the local symbol table for each argument
 *  found in the argument list. It returns the number of arguments.
 */
static int declargs(symbol *sym,int chkshadow)
{
  #define MAXTAGS 16
  char *ptr;
  int argcnt,oldargcnt,tok,tags[MAXTAGS],numtags;
  cell val;
  arginfo arg, *arglist;
  char name[sNAMEMAX+1];
  int ident,fpublic,fconst,fpragma;
  int idx;

  /* if the function is already defined earlier, get the number of arguments
   * of the existing definition
   */
  oldargcnt=0;
  if ((sym->usage & uPROTOTYPED)!=0)
    while (sym->dim.arglist[oldargcnt].ident!=0)
      oldargcnt++;
  argcnt=0;                             /* zero arguments up to now */
  ident=iVARIABLE;
  numtags=0;
  fconst=fpragma=FALSE;
  fpublic= (sym->usage & uPUBLIC)!=0;
  /* the '(' parentheses has already been parsed */
  if (!matchtoken(')')){
    do {                                /* there are arguments; process them */
      /* any legal name increases argument count (and stack offset) */
      tok=lex(&val,&ptr);
      switch (tok) {
      case 0:
        /* nothing */
        break;
      case '&':
        if (ident!=iVARIABLE || numtags>0)
          error(1,sc_tokens[tSYMBOL-tFIRST],"&");
        if (fconst)
          error(238, "const reference"); /* meaningless combination of class specifiers */
        ident=iREFERENCE;
        break;
      case tCONST:
        if (ident!=iVARIABLE || numtags>0 || fpragma)
          error(1,sc_tokens[tSYMBOL-tFIRST],sc_tokens[tCONST-tFIRST]);
        fconst=TRUE;
        break;
      case t__PRAGMA:
        if (ident!=iVARIABLE || numtags>0)
          error(1,sc_tokens[tSYMBOL-tFIRST],sc_tokens[t__PRAGMA-tFIRST]);
        dopragma();
        fpragma=TRUE;
        break;
      case tLABEL:
        if (numtags>0)
          error(1,sc_tokens[tSYMBOL-tFIRST],"-tagname-");
        tags[0]=pc_addtag(ptr);
        numtags=1;
        break;
      case '{':
        if (numtags>0)
          error(1,sc_tokens[tSYMBOL-tFIRST],"-tagname-");
        numtags=0;
        while (numtags<MAXTAGS) {
          if (!matchtoken('_') && !needtoken(tSYMBOL))
            break;
          tokeninfo(&val,&ptr);
          tags[numtags++]=pc_addtag(ptr);
          if (matchtoken('}'))
            break;
          needtoken(',');
        } /* while */
        needtoken(':');
        tok=tLABEL;     /* for outer loop: flag that we have seen a tagname */
        break;
      case tSYMBOL:
        if (argcnt>=sMAXARGS)
          error(45);                    /* too many function arguments */
        strcpy(name,ptr);               /* save symbol name */
        if (name[0]==PUBLIC_CHAR)
          error(56,name);               /* function arguments cannot be public */
        if (numtags==0)
          tags[numtags++]=0;            /* default tag */
        /* Stack layout:
         *   base + 0*sizeof(cell)  == previous "base"
         *   base + 1*sizeof(cell)  == function return address
         *   base + 2*sizeof(cell)  == number of arguments
         *   base + 3*sizeof(cell)  == first argument of the function
         * So the offset of each argument is "(argcnt+3) * sizeof(cell)".
         */
        doarg(name,ident,(argcnt+3)*sizeof(cell),tags,numtags,fpublic,fconst,!!(sym->dim.arglist[argcnt].usage & uWRITTEN),chkshadow,&arg);
        if (fpublic && arg.hasdefault)
          error(59,name);       /* arguments of a public function may not have a default value */
        if ((sym->usage & uPROTOTYPED)==0) {
          /* redimension the argument list, add the entry */
          arginfo* new_arglist=(arginfo*)realloc(sym->dim.arglist,(argcnt+2)*sizeof(arginfo));
          if (new_arglist==NULL)
            error(103);                 /* insufficient memory */
          sym->dim.arglist=new_arglist;
          memset(&sym->dim.arglist[argcnt+1],0,sizeof(arginfo));  /* keep the list terminated */
          sym->dim.arglist[argcnt]=arg;
        } else {
          /* check the argument with the earlier definition */
          if (argcnt>oldargcnt || !argcompare(&sym->dim.arglist[argcnt],&arg))
            error(25);          /* function definition does not match prototype */
          /* may need to free default array argument and the tag list */
          if (arg.ident==iREFARRAY && arg.hasdefault)
            free(arg.defvalue.array.data);
          else if (arg.ident==iVARIABLE
                   && ((arg.hasdefault & uSIZEOF)!=0 || (arg.hasdefault & uTAGOF)!=0))
            free(arg.defvalue.size.symname);
          free(arg.tags);
        } /* if */
        argcnt++;
        ident=iVARIABLE;
        numtags=0;
        fconst=fpragma=FALSE;
        break;
      case tELLIPS:
        if (ident!=iVARIABLE || fpragma)
          error(10);                    /* illegal function or declaration */
        if (fconst)
          error(238, "const variable arguments"); /* meaningless combination of class specifiers */
        if (numtags==0)
          tags[numtags++]=0;            /* default tag */
        if ((sym->usage & uPROTOTYPED)==0) {
          /* redimension the argument list, add the entry iVARARGS */
          arginfo* new_arglist=(arginfo*)realloc(sym->dim.arglist,(argcnt+2)*sizeof(arginfo));
          if (new_arglist==NULL)
            error(103);                 /* insufficient memory */
          sym->dim.arglist=new_arglist;
          memset(&sym->dim.arglist[argcnt+1],0,sizeof(arginfo));  /* keep the list terminated */
          sym->dim.arglist[argcnt].ident=iVARARGS;
          sym->dim.arglist[argcnt].hasdefault=FALSE;
          sym->dim.arglist[argcnt].defvalue.val=0;
          sym->dim.arglist[argcnt].defvalue_tag=0;
          sym->dim.arglist[argcnt].numtags=numtags;
          sym->dim.arglist[argcnt].tags=(int*)malloc(numtags*sizeof tags[0]);
          if (sym->dim.arglist[argcnt].tags==NULL)
            error(103);                 /* insufficient memory */
          memcpy(sym->dim.arglist[argcnt].tags,tags,numtags*sizeof tags[0]);
        } else {
          if (argcnt>oldargcnt || sym->dim.arglist[argcnt].ident!=iVARARGS)
            error(25);          /* function definition does not match prototype */
        } /* if */
        argcnt++;
        break;
      default:
        error(10);                      /* illegal function or declaration */
      } /* switch */
    } while (tok=='&' || tok==tLABEL || tok==tCONST || tok==t__PRAGMA
             || (tok!=tELLIPS && matchtoken(','))); /* more? */
    /* if the next token is not ",", it should be ")" */
    needtoken(')');
  } /* if */
  /* resolve any "sizeof" arguments (now that all arguments are known) */
  assert(sym->dim.arglist!=NULL);
  arglist=sym->dim.arglist;
  for (idx=0; idx<argcnt && arglist[idx].ident!=0; idx++) {
    if ((arglist[idx].hasdefault & uSIZEOF)!=0 || (arglist[idx].hasdefault & uTAGOF)!=0) {
      int altidx;
      /* Find the argument with the name mentioned after the "sizeof". Note
       * that we cannot use findloc here because we need the arginfo struct,
       * not the symbol.
       */
      ptr=arglist[idx].defvalue.size.symname;
      assert(ptr!=NULL);
      for (altidx=0; altidx<argcnt && strcmp(ptr,arglist[altidx].name)!=0; altidx++)
        /* nothing */;
      if (altidx>=argcnt) {
        error(17,ptr);                  /* undefined symbol */
      } else {
        assert(arglist[idx].defvalue.size.symname!=NULL);
        /* check the level against the number of dimensions */
        if (arglist[idx].defvalue.size.level>0
            && arglist[idx].defvalue.size.level>=arglist[altidx].numdim)
          error(28,arglist[idx].name);  /* invalid subscript */
        /* check the type of the argument whose size to take; for a iVARIABLE
         * or a iREFERENCE, this is always 1 (so the code is redundant)
         */
        assert(arglist[altidx].ident!=iVARARGS);
        if (arglist[altidx].ident!=iREFARRAY && (arglist[idx].hasdefault & uSIZEOF)!=0) {
          if ((arglist[idx].hasdefault & uTAGOF)!=0) {
            error(81,arglist[idx].name);  /* cannot take "tagof" an indexed array */
          } else {
            assert(arglist[altidx].ident==iVARIABLE || arglist[altidx].ident==iREFERENCE);
            error(223,ptr);             /* redundant sizeof */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
  } /* for */

  sym->usage|=uPROTOTYPED;
  errorset(sRESET,0);           /* reset error flag (clear the "panic mode")*/
  return argcnt;
}

/*  doarg       - declare one argument type
 *
 *  this routine is called from "declargs()" and adds an entry in the local
 *  symbol table for one argument.
 *
 *  "fpublic" indicates whether the function for this argument list is public.
 *  The arguments themselves are never public.
 */
static void doarg(char *name,int ident,int offset,int tags[],int numtags,
                  int fpublic,int fconst,int written,int chkshadow,arginfo *arg)
{
  symbol *argsym;
  constvalue_root *enumroot=NULL;
  cell size;

  strcpy(arg->name,name);
  arg->hasdefault=FALSE;        /* preset (most common case) */
  arg->defvalue.val=0;          /* clear */
  arg->defvalue_tag=0;
  arg->numdim=0;
  if (matchtoken('[')) {
    if (ident==iREFERENCE)
      error(67,name);           /* illegal declaration ("&name[]" is unsupported) */
    do {
      if (arg->numdim == sDIMEN_MAX) {
        error(53);              /* exceeding maximum number of dimensions */
        return;
      } /* if */
      size=needsub(&arg->idxtag[arg->numdim],&enumroot);/* may be zero here, it is a pointer anyway */
      #if INT_MAX < LONG_MAX
        if (size > INT_MAX)
          error(105);           /* overflow, exceeding capacity */
      #endif
      arg->dim[arg->numdim]=(int)size;
      arg->numdim+=1;
    } while (matchtoken('['));
    ident=iREFARRAY;            /* "reference to array" (is a pointer) */
    if (matchtoken('=')) {
      lexpush();                /* initials() needs the "=" token again */
      assert(litidx==0);        /* at the start of a function, this is reset */
      assert(numtags>0);
      initials(ident,tags[0],&size,arg->dim,arg->numdim,enumroot,NULL);
      assert(size>=litidx);
      /* allocate memory to hold the initial values */
      arg->defvalue.array.data=(cell *)malloc(litidx*sizeof(cell));
      if (arg->defvalue.array.data!=NULL) {
        int i;
        memcpy(arg->defvalue.array.data,litq,litidx*sizeof(cell));
        arg->hasdefault=TRUE;   /* argument has default value */
        arg->defvalue.array.size=litidx;
        arg->defvalue.array.addr=-1;
        /* calculate size to reserve on the heap */
        arg->defvalue.array.arraysize=1;
        for (i=0; i<arg->numdim; i++)
          arg->defvalue.array.arraysize*=arg->dim[i];
        if (arg->defvalue.array.arraysize < arg->defvalue.array.size)
          arg->defvalue.array.arraysize = arg->defvalue.array.size;
      } /* if */
      litidx=0;                 /* reset */
    } /* if */
  } else {
    if (matchtoken('=')) {
      unsigned char size_tag_token;
      assert(ident==iVARIABLE || ident==iREFERENCE);
      arg->hasdefault=TRUE;     /* argument has a default value */
      size_tag_token=(unsigned char)(matchtoken(tSIZEOF) ? uSIZEOF : 0);
      if (size_tag_token==0)
        size_tag_token=(unsigned char)(matchtoken(tTAGOF) ? uTAGOF : 0);
      if (size_tag_token!=0) {
        char* symname;
        cell val;
        int parentheses;
        if (ident==iREFERENCE)
          error(66,name);       /* argument may not be a reference */
        parentheses=0;
        while (matchtoken('('))
          parentheses++;
        if (size_tag_token==uTAGOF && matchtoken(tLABEL)) {
          constvalue *tagsym;
          tokeninfo(&val,&symname);
          tagsym=find_constval(&tagname_tab,symname,0);
          arg->defvalue.val=(tagsym!=NULL) ? tagsym->value : (cell)0;
          arg->hasdefault |= uTAGOF_TAG;
        } else if (needtoken(tSYMBOL)) {
          /* save the name of the argument whose size id to take */
          tokeninfo(&val,&symname);
          if ((arg->defvalue.size.symname=duplicatestring(symname)) == NULL)
            error(103);         /* insufficient memory */
          arg->defvalue.size.level=0;
          if (size_tag_token==uSIZEOF) {
            while (matchtoken('[')) {
              arg->defvalue.size.level+=(short)1;
              needtoken(']');
            } /* while */
          } /* if */
          if (ident==iVARIABLE) /* make sure we set this only if not a reference */
            arg->hasdefault |= size_tag_token;  /* uSIZEOF or uTAGOF */
        } else {
          /* ignore the argument, otherwise it would cause more error messages, until it
           * will trigger a fatal error because of too many error messages on one line */
          lexclr(FALSE);
        } /* if */
        while (parentheses--)
          needtoken(')');
      } else {
        constexpr(&arg->defvalue.val,&arg->defvalue_tag,NULL);
        assert(numtags>0);
        check_tagmismatch(tags[0],arg->defvalue_tag,TRUE,-1);
      } /* if */
    } /* if */
  } /* if */
  arg->ident=(char)ident;
  arg->usage=(char)(fconst ? uCONST : 0);
  arg->usage|=(char)(written ? uWRITTEN : 0);
  arg->numtags=numtags;
  arg->tags=(int*)malloc(numtags*sizeof tags[0]);
  if (arg->tags==NULL)
    error(103);                 /* insufficient memory */
  memcpy(arg->tags,tags,numtags*sizeof tags[0]);
  argsym=findloc(name);
  if (argsym!=NULL) {
    error(21,name);             /* symbol already defined */
  } else {
    if (chkshadow && (argsym=findglb(name,sSTATEVAR))!=NULL && argsym->ident!=iFUNCTN)
      error(219,name);          /* variable shadows another symbol */
    /* add details of type and address */
    assert(numtags>0);
    argsym=addvariable(name,offset,ident,sLOCAL,tags[0],
                       arg->dim,arg->numdim,arg->idxtag,0);
    if (fpublic) {
      argsym->usage|=uREAD;     /* arguments of public functions are always "used" */
      if(argsym->ident==iREFARRAY || argsym->ident==iREFERENCE)
        argsym->usage|=uWRITTEN;
    } else if (argsym->ident==iVARIABLE) {
      argsym->usage|=uASSIGNED;
      argsym->assignlevel=1;
    } /* if */

    if (fconst)
      argsym->usage|=uCONST;
  } /* if */
  if (matchtoken(t__PRAGMA))
    dopragma();
  pragma_apply(argsym);
}

static int has_referrers(symbol *entry)
{
  int i;
  for (i=0; i<entry->numrefers; i++)
    if (entry->refer[i]!=NULL)
      return TRUE;
  return ((entry->usage & uGLOBALREF)!=0);
}

#if !defined SC_LIGHT
static int find_xmltag(char *source,char *xmltag,char *xmlparam,char *xmlvalue,
                       char **outer_start,int *outer_length,
                       char **inner_start,int *inner_length)
{
  char *ptr,*inner_end;
  int xmltag_len,xmlparam_len,xmlvalue_len;
  int match;

  assert(source!=NULL);
  assert(xmltag!=NULL);
  assert(outer_start!=NULL);
  assert(outer_length!=NULL);
  assert(inner_start!=NULL);
  assert(inner_length!=NULL);

  /* both NULL or both non-NULL */
  assert(xmlvalue!=NULL && xmlparam!=NULL || xmlvalue==NULL && xmlparam==NULL);

  xmltag_len=strlen(xmltag);
  xmlparam_len= (xmlparam!=NULL) ? strlen(xmlparam) : 0;
  xmlvalue_len= (xmlvalue!=NULL) ? strlen(xmlvalue) : 0;
  ptr=source;
  /* find an opening '<' */
  while ((ptr=strchr(ptr,'<'))!=NULL) {
    *outer_start=ptr;           /* be optimistic... */
    match=FALSE;                /* ...and pessimistic at the same time */
    ptr++;                      /* skip '<' */
    while (*ptr!='\0' && *ptr<=' ')
      ptr++;                    /* skip white space */
    if (strncmp(ptr,xmltag,xmltag_len)==0 && (*(ptr+xmltag_len)<=' ' || *(ptr+xmltag_len)=='>')) {
      /* xml tag found, optionally check the parameter */
      ptr+=xmltag_len;
      while (*ptr!='\0' && *ptr<=' ')
        ptr++;                  /* skip white space */
      if (xmlparam!=NULL) {
        if (strncmp(ptr,xmlparam,xmlparam_len)==0 && (*(ptr+xmlparam_len)<=' ' || *(ptr+xmlparam_len)=='=')) {
          ptr+=xmlparam_len;
          while (*ptr!='\0' && *ptr<=' ')
            ptr++;              /* skip white space */
          if (*ptr=='=') {
            ptr++;              /* skip '=' */
            while (*ptr!='\0' && *ptr<=' ')
              ptr++;            /* skip white space */
            if (*ptr=='"' || *ptr=='\'')
              ptr++;            /* skip " or ' */
            assert(xmlvalue!=NULL);
            if (strncmp(ptr,xmlvalue,xmlvalue_len)==0
                && (*(ptr+xmlvalue_len)<=' '
                    || *(ptr+xmlvalue_len)=='>'
                    || *(ptr+xmlvalue_len)=='"'
                    || *(ptr+xmlvalue_len)=='\''))
              match=TRUE;       /* found it */
          } /* if */
        } /* if */
      } else {
        match=TRUE;             /* don't check the parameter */
      } /* if */
    } /* if */
    if (match) {
      /* now find the end of the opening tag */
      while (*ptr!='\0' && *ptr!='>')
        ptr++;
      if (*ptr=='>')
        ptr++;
      while (*ptr!='\0' && *ptr<=' ')
        ptr++;                  /* skip white space */
      *inner_start=ptr;
      /* find the start of the closing tag (assume no nesting) */
      while ((ptr=strchr(ptr,'<'))!=NULL) {
        inner_end=ptr;
        ptr++;                  /* skip '<' */
        while (*ptr!='\0' && *ptr<=' ')
          ptr++;                /* skip white space */
        if (*ptr=='/') {
          ptr++;                /* skip / */
          while (*ptr!='\0' && *ptr<=' ')
            ptr++;              /* skip white space */
          if (strncmp(ptr,xmltag,xmltag_len)==0 && (*(ptr+xmltag_len)<=' ' || *(ptr+xmltag_len)=='>')) {
            /* find the end of the closing tag */
            while (*ptr!='\0' && *ptr!='>')
              ptr++;
            if (*ptr=='>')
              ptr++;
            /* set the lengths of the inner and outer segment */
            assert(*inner_start!=NULL);
            *inner_length=(int)(inner_end-*inner_start);
            assert(*outer_start!=NULL);
            *outer_length=(int)(ptr-*outer_start);
            break;              /* break out of the loop */
          } /* if */
        } /* if */
      } /* while */
      return TRUE;
    } /* if */
  } /* while */
  return FALSE; /* not found */
}

static char *xmlencode(char *dest,char *source)
{
  char temp[2*sNAMEMAX+20],*ptr;

  /* replace < by &lt; and such; normally, such a symbol occurs at most once in
   * a symbol name (e.g. "operator<")
   */
  ptr=temp;
  while (*source!='\0') {
    switch (*source) {
    case '<':
      strcpy(ptr,"&lt;");
      ptr+=4;
      break;
    case '>':
      strcpy(ptr,"&gt;");
      ptr+=4;
      break;
    case '&':
      strcpy(ptr,"&amp;");
      ptr+=5;
      break;
    default:
      *ptr++=*source;
    } /* switch */
    source++;
  } /* while */
  *ptr='\0';
  strcpy(dest,temp);
  return dest;
}

static void make_report(symbol *root,FILE *log,char *sourcefile)
{
  char symname[_MAX_PATH];
  int i,arg;
  symbol *sym,*ref;
  constvalue *tagsym;
  constvalue_root *enumroot;
  char *ptr;

  /* adapt the installation directory */
  strcpy(symname,sc_rootpath);
  #if DIRSEP_CHAR=='\\'
    while ((ptr=strchr(symname,':'))!=NULL)
      *ptr='|';
    while ((ptr=strchr(symname,DIRSEP_CHAR))!=NULL)
      *ptr='/';
  #endif

  /* the XML header */
  fprintf(log,"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
  fprintf(log,"<?xml-stylesheet href=\"file:///%s/xml/pawndoc.xsl\" type=\"text/xsl\"?>\n",symname);
  fprintf(log,"<doc source=\"%s\">\n",sourcefile);
  ptr=strrchr(sourcefile,DIRSEP_CHAR);
  if (ptr!=NULL)
    ptr++;
  else
    ptr=sourcefile;
  fprintf(log,"\t<assembly>\n\t\t<name>%s</name>\n\t</assembly>\n",ptr);

  /* attach the global documentation, if any */
  if (sc_documentation!=NULL) {
    fprintf(log,"\n\t<!-- general -->\n");
    fprintf(log,"\t<general>\n\t\t");
    fputs(sc_documentation,log);
    fprintf(log,"\n\t</general>\n\n");
  } /* if */

  /* use multiple passes to print constants variables and functions in
   * separate sections
   */
  fprintf(log,"\t<members>\n");

  fprintf(log,"\n\t\t<!-- enumerations -->\n");
  for (sym=root->next; sym!=NULL; sym=sym->next) {
    if (sym->parent!=NULL)
      continue;                 /* hierarchical data type */
    assert(sym->ident==iCONSTEXPR || sym->ident==iVARIABLE
           || sym->ident==iARRAY || sym->ident==iFUNCTN);
    if (sym->ident!=iCONSTEXPR || (sym->usage & uENUMROOT)==0)
      continue;
    if ((sym->usage & uREAD)==0)
      continue;
    fprintf(log,"\t\t<member name=\"T:%s\" value=\"%"PRIdC"\">\n",funcdisplayname(symname,sym->name),sym->addr);
    if (sym->tag!=0) {
      tagsym=find_tag_byval(sym->tag);
      assert(tagsym!=NULL);
      fprintf(log,"\t\t\t<tagname value=\"%s\"/>\n",tagsym->name);
    } /* if */
    /* browse through all fields */
    if ((enumroot=sym->dim.enumlist)!=NULL) {
      constvalue *cur=enumroot->first;  /* skip root */
      while (cur!=NULL) {
        fprintf(log,"\t\t\t<member name=\"C:%s\" value=\"%"PRIdC"\">\n",funcdisplayname(symname,cur->name),cur->value);
        /* find the constant with this name and get the tag */
        ref=findglb(cur->name,sGLOBAL);
        if (ref!=NULL) {
          if (ref->x.tags.index!=0) {
            tagsym=find_tag_byval(ref->x.tags.index);
            assert(tagsym!=NULL);
            fprintf(log,"\t\t\t\t<tagname value=\"%s\"/>\n",tagsym->name);
          } /* if */
          if (ref->dim.array.length!=1)
            fprintf(log,"\t\t\t\t<size value=\"%ld\"/>\n",(long)ref->dim.array.length);
        } /* if */
        fprintf(log,"\t\t\t</member>\n");
        cur=cur->next;
      } /* while */
    } /* if */
    assert(sym->refer!=NULL);
    for (i=0; i<sym->numrefers; i++) {
      if ((ref=sym->refer[i])!=NULL)
        fprintf(log,"\t\t\t<referrer name=\"%s\"/>\n",xmlencode(symname,funcdisplayname(symname,ref->name)));
    } /* for */
    if (sym->documentation!=NULL)
      fprintf(log,"\t\t\t%s\n",sym->documentation);
    fprintf(log,"\t\t</member>\n");
  } /* for */

  fprintf(log,"\n\t\t<!-- constants -->\n");
  for (sym=root->next; sym!=NULL; sym=sym->next) {
    if (sym->parent!=NULL)
      continue;                 /* hierarchical data type */
    assert(sym->ident==iCONSTEXPR || sym->ident==iVARIABLE
           || sym->ident==iARRAY || sym->ident==iFUNCTN);
    if (sym->ident!=iCONSTEXPR)
      continue;
    if ((sym->usage & uREAD)==0 || (sym->usage & (uENUMFIELD | uENUMROOT))!=0)
      continue;
    fprintf(log,"\t\t<member name=\"C:%s\" value=\"%"PRIdC"\">\n",funcdisplayname(symname,sym->name),sym->addr);
    if (sym->tag!=0) {
      tagsym=find_tag_byval(sym->tag);
      assert(tagsym!=NULL);
      fprintf(log,"\t\t\t<tagname value=\"%s\"/>\n",tagsym->name);
    } /* if */
    assert(sym->refer!=NULL);
    for (i=0; i<sym->numrefers; i++) {
      if ((ref=sym->refer[i])!=NULL)
        fprintf(log,"\t\t\t<referrer name=\"%s\"/>\n",xmlencode(symname,funcdisplayname(symname,ref->name)));
    } /* for */
    if (sym->documentation!=NULL)
      fprintf(log,"\t\t\t%s\n",sym->documentation);
    fprintf(log,"\t\t</member>\n");
  } /* for */

  fprintf(log,"\n\t\t<!-- variables -->\n");
  for (sym=root->next; sym!=NULL; sym=sym->next) {
    if (sym->parent!=NULL)
      continue;                 /* hierarchical data type */
    if (sym->ident!=iVARIABLE && sym->ident!=iARRAY)
      continue;
    fprintf(log,"\t\t<member name=\"F:%s\">\n",funcdisplayname(symname,sym->name));
    if (sym->tag!=0) {
      tagsym=find_tag_byval(sym->tag);
      assert(tagsym!=NULL);
      fprintf(log,"\t\t\t<tagname value=\"%s\"/>\n",tagsym->name);
    } /* if */
    assert(sym->refer!=NULL);
    if ((sym->usage & uPUBLIC)!=0)
      fprintf(log,"\t\t\t<attribute name=\"public\"/>\n");
    for (i=0; i<sym->numrefers; i++) {
      if ((ref=sym->refer[i])!=NULL)
        fprintf(log,"\t\t\t<referrer name=\"%s\"/>\n",xmlencode(symname,funcdisplayname(symname,ref->name)));
    } /* for */
    if (sym->documentation!=NULL)
      fprintf(log,"\t\t\t%s\n",sym->documentation);
    fprintf(log,"\t\t</member>\n");
  } /* for */

  fprintf(log,"\n\t\t<!-- functions -->\n");
  for (sym=root->next; sym!=NULL; sym=sym->next) {
    if (sym->parent!=NULL)
      continue;                 /* hierarchical data type */
    if (sym->ident!=iFUNCTN)
      continue;
    if ((sym->usage & (uREAD | uNATIVE))==uNATIVE)
      continue;                 /* unused native function */
    funcdisplayname(symname,sym->name);
    xmlencode(symname,symname);
    fprintf(log,"\t\t<member name=\"M:%s\" syntax=\"%s(",symname,symname);
    /* print only the names of the parameters between the parentheses */
    assert(sym->dim.arglist!=NULL);
    for (arg=0; sym->dim.arglist[arg].ident!=0; arg++) {
      int dim;
      if (arg>0)
        fprintf(log,", ");
      switch (sym->dim.arglist[arg].ident) {
      case iVARIABLE:
        fprintf(log,"%s",sym->dim.arglist[arg].name);
        break;
      case iREFERENCE:
        fprintf(log,"&amp;%s",sym->dim.arglist[arg].name);
        break;
      case iREFARRAY:
        fprintf(log,"%s",sym->dim.arglist[arg].name);
        for (dim=0; dim<sym->dim.arglist[arg].numdim;dim++)
          fprintf(log,"[]");
        break;
      case iVARARGS:
        fprintf(log,"...");
        break;
      } /* switch */
    } /* for */
    /* ??? should also print an "array return" size */
    fprintf(log,")\">\n");
    if (sym->tag!=0) {
      tagsym=find_tag_byval(sym->tag);
      assert(tagsym!=NULL);
      fprintf(log,"\t\t\t<tagname value=\"%s\"/>\n",tagsym->name);
    } /* if */
    /* check whether this function is called from the outside */
    if ((sym->usage & uNATIVE)!=0)
      fprintf(log,"\t\t\t<attribute name=\"native\"/>\n");
    if ((sym->usage & uPUBLIC)!=0)
      fprintf(log,"\t\t\t<attribute name=\"public\"/>\n");
    if (strcmp(sym->name,uMAINFUNC)==0 || strcmp(sym->name,uENTRYFUNC)==0)
      fprintf(log,"\t\t\t<attribute name=\"entry\"/>\n");
    if ((sym->usage & uNATIVE)==0)
      fprintf(log,"\t\t\t<stacksize value=\"%ld\"/>\n",(long)sym->x.stacksize);
    if (sym->states!=NULL) {
      constvalue *stlist=sym->states->first;
      assert(stlist!=NULL);     /* there should be at least one state item */
      while (stlist!=NULL && stlist->index==-1)
        stlist=stlist->next;
      assert(stlist!=NULL);     /* state id should be found */
      i=state_getfsa(stlist->index);
      assert(i>=0);             /* automaton 0 exists */
      stlist=automaton_findid(i);
      assert(stlist!=NULL);     /* automaton should be found */
      fprintf(log,"\t\t\t<automaton name=\"%s\"/>\n", !strempty(stlist->name) ? stlist->name : "(anonymous)");
      //??? dump state decision table
    } /* if */
    assert(sym->refer!=NULL);
    for (i=0; i<sym->numrefers; i++)
      if ((ref=sym->refer[i])!=NULL)
        fprintf(log,"\t\t\t<referrer name=\"%s\"/>\n",xmlencode(symname,funcdisplayname(symname,ref->name)));
    /* print all symbols that are required for this function to compile */
    for (ref=root->next; ref!=NULL; ref=ref->next) {
      if (ref==sym)
        continue;
      for (i=0; i<ref->numrefers; i++)
        if (ref->refer[i]==sym)
          fprintf(log,"\t\t\t<dependency name=\"%s\"/>\n",xmlencode(symname,funcdisplayname(symname,ref->name)));
    } /* for */
    /* print parameter list, with tag & const information, plus descriptions */
    assert(sym->dim.arglist!=NULL);
    for (arg=0; sym->dim.arglist[arg].ident!=0; arg++) {
      int dim,paraminfo;
      char *outer_start,*inner_start;
      int outer_length,inner_length;
      if (sym->dim.arglist[arg].ident==iVARARGS)
        fprintf(log,"\t\t\t<param name=\"...\">\n");
      else
        fprintf(log,"\t\t\t<param name=\"%s\">\n",sym->dim.arglist[arg].name);
      /* print the tag name(s) for each parameter */
      assert(sym->dim.arglist[arg].numtags>0);
      assert(sym->dim.arglist[arg].tags!=NULL);
      paraminfo=(sym->dim.arglist[arg].numtags>1 || sym->dim.arglist[arg].tags[0]!=0)
                || sym->dim.arglist[arg].ident==iREFERENCE
                || sym->dim.arglist[arg].ident==iREFARRAY;
      if (paraminfo)
        fprintf(log,"\t\t\t\t<paraminfo>");
      if (sym->dim.arglist[arg].numtags>1 || sym->dim.arglist[arg].tags[0]!=0) {
        assert(paraminfo);
        if (sym->dim.arglist[arg].numtags>1)
          fprintf(log," {");
        for (i=0; i<sym->dim.arglist[arg].numtags; i++) {
          if (i>0)
            fprintf(log,",");
          tagsym=find_tag_byval(sym->dim.arglist[arg].tags[i]);
          assert(tagsym!=NULL);
          fprintf(log,"%s",tagsym->name);
        } /* for */
        if (sym->dim.arglist[arg].numtags>1)
          fprintf(log,"}");
      } /* if */
      switch (sym->dim.arglist[arg].ident) {
      case iREFERENCE:
        fprintf(log," &amp;");
        break;
      case iREFARRAY:
        fprintf(log," ");
        for (dim=0; dim<sym->dim.arglist[arg].numdim; dim++) {
          if (sym->dim.arglist[arg].dim[dim]==0) {
            fprintf(log,"[]");
          } else {
            //??? find index tag
            fprintf(log,"[%d]",sym->dim.arglist[arg].dim[dim]);
          } /* if */
        } /* for */
        break;
      } /* switch */
      if (paraminfo)
        fprintf(log," </paraminfo>\n");
      /* print the user description of the parameter (parse through
       * sym->documentation)
       */
      if (sym->documentation!=NULL
          && find_xmltag(sym->documentation, "param", "name", sym->dim.arglist[arg].name,
                         &outer_start, &outer_length, &inner_start, &inner_length))
      {
        char *tail;
        fprintf(log,"\t\t\t\t%.*s\n",inner_length,inner_start);
        /* delete from documentation string */
        tail=outer_start+outer_length;
        memmove(outer_start,tail,strlen(tail)+1);
      } /* if */
      fprintf(log,"\t\t\t</param>\n");
    } /* for */
    if (sym->documentation!=NULL)
      fprintf(log,"\t\t\t%s\n",sym->documentation);
    fprintf(log,"\t\t</member>\n");
  } /* for */

  fprintf(log,"\n\t</members>\n");
  fprintf(log,"</doc>\n");
}
#endif

/* Every symbol has a referrer list, that contains the functions that use
 * the symbol. Now, if function "apple" is accessed by functions "banana" and
 * "citron", but neither function "banana" nor "citron" are used by anyone
 * else, then, by inference, function "apple" is not used either.
 */
static void reduce_referrers(symbol *root)
{
  int i,restart;
  symbol *sym,*ref;

  do {
    restart=0;
    for (sym=root->next; sym!=NULL; sym=sym->next) {
      if (sym->parent!=NULL)
        continue;                 /* hierarchical data type */
      if (sym->ident==iFUNCTN
          && (sym->usage & uNATIVE)==0
          && (sym->usage & uPUBLIC)==0 && strcmp(sym->name,uMAINFUNC)!=0 && strcmp(sym->name,uENTRYFUNC)!=0
          && !has_referrers(sym))
      {
        sym->usage&=~(uREAD | uWRITTEN);  /* erase usage bits if there is no referrer */
        /* find all symbols that are referred by this symbol */
        for (ref=root->next; ref!=NULL; ref=ref->next) {
          if (ref->parent!=NULL)
            continue;             /* hierarchical data type */
          assert(ref->refer!=NULL);
          for (i=0; i<ref->numrefers && ref->refer[i]!=sym; i++)
            /* nothing */;
          if (i<ref->numrefers) {
            assert(ref->refer[i]==sym);
            ref->refer[i]=NULL;
            restart++;
          } /* if */
        } /* for */
      } else if ((sym->ident==iVARIABLE || sym->ident==iARRAY)
                 && (sym->usage & uPUBLIC)==0
                 && sym->parent==NULL
                 && !has_referrers(sym))
      {
        sym->usage&=~(uREAD | uWRITTEN);  /* erase usage bits if there is no referrer */
      } /* if */
    } /* for */
    /* after removing a symbol, check whether more can be removed */
  } while (restart>0);
}

#if !defined SC_LIGHT
static long max_stacksize_recurse(symbol **sourcesym,symbol *sym,symbol **rsourcesym,long basesize,int *pubfuncparams,int *recursion)
{
  long size,maxsize;
  int i,stkpos;

  assert(sourcesym!=NULL);
  assert(sym!=NULL);
  assert(sym->ident==iFUNCTN);
  assert((sym->usage & uNATIVE)==0);
  assert(recursion!=NULL);

  maxsize=sym->x.stacksize;
  for (i=0; i<sym->numrefers; i++) {
    if (sym->refer[i]!=NULL) {
      assert(sym->refer[i]->ident==iFUNCTN);
      assert((sym->refer[i]->usage & uNATIVE)==0); /* a native function cannot refer to a user-function */
      *(rsourcesym)=sym;
      *(rsourcesym+1)=NULL;
      for (stkpos=0; sourcesym[stkpos]!=NULL; stkpos++) {
        if (sym->refer[i]==sourcesym[stkpos]) {   /* recursion detection */
          *recursion=TRUE;
          goto break_recursion;         /* recursion was detected, quit loop */
        } /* if */
      } /* for */
      /* add this symbol to the stack */
      sourcesym[stkpos]=sym;
      sourcesym[stkpos+1]=NULL;
      /* check size of callee */
      size=max_stacksize_recurse(sourcesym,sym->refer[i],rsourcesym+1,sym->x.stacksize,pubfuncparams,recursion);
      if (maxsize<size)
        maxsize=size;
      /* remove this symbol from the stack */
      sourcesym[stkpos]=NULL;
    } /* if */
  } /* for */
  break_recursion:

  if ((sym->usage & uPUBLIC)!=0) {
    /* Find out how many parameters a public function has, then see if this
     * is bigger than some maximum
     */
    arginfo *arg=sym->dim.arglist;
    int count=0;
    assert(arg!=0);
    while (arg->ident!=0) {
      count++;
      arg++;
    } /* while */
    assert(pubfuncparams!=0);
    if (count>*pubfuncparams)
      *pubfuncparams=count;
  } /* if */

  return maxsize+basesize;
}

static long max_stacksize(symbol *root,int *recursion)
{
  /* Loop over all non-native functions. For each function, loop
   * over all of its referrers, accumulating the stack requirements.
   * Detect (indirect) recursion with a "mark-and-sweep" algorithm.
   * I (mis-)use the "compound" field of the symbol structure for
   * the marker, as this field is unused for functions.
   *
   * Note that the stack is shared with the heap. A host application
   * may "eat" cells from the heap as well, through amx_Allot(). The
   * stack requirements are thus only an estimate.
   */
  long size,maxsize;
  int maxparams,numfunctions;
  symbol *sym;
  symbol **symstack,**rsymstack;

  assert(root!=NULL);
  assert(recursion!=NULL);
  /* count number of functions (for allocating the stack for recursion detection) */
  numfunctions=0;
  for (sym=root->next; sym!=NULL; sym=sym->next) {
    if (sym->ident==iFUNCTN) {
      assert(sym->compound==0);
      if ((sym->usage & uNATIVE)==0)
        numfunctions++;
    } /* if */
  } /* for */
  /* allocate function symbol stack */
  symstack=(symbol **)malloc((numfunctions+1)*sizeof(symbol*));
  rsymstack=(symbol **)malloc((numfunctions+1)*sizeof(symbol*));
  if (symstack==NULL || rsymstack==NULL)
    error(103);         /* insufficient memory (fatal error) */
  memset(symstack,0,(numfunctions+1)*sizeof(symbol*));
  memset(rsymstack,0,(numfunctions+1)*sizeof(symbol*));

  maxsize=0;
  maxparams=0;
  *recursion=FALSE;     /* assume no recursion */
  for (sym=root->next; sym!=NULL; sym=sym->next) {
    int recursion_detected;
    /* drop out if this is not a user-implemented function */
    if (sym->ident!=iFUNCTN || (sym->usage & uNATIVE)!=0)
      continue;
    /* accumulate stack size for this symbol */
    symstack[0]=sym;
    assert(symstack[1]==NULL);
    recursion_detected=FALSE;
    size=max_stacksize_recurse(symstack,sym,rsymstack,0L,&maxparams,&recursion_detected);
    if (recursion_detected && pc_recursion) {
      if (rsymstack[1]==NULL) {
        pc_printf("recursion detected: function %s directly calls itself\n", sym->name);
      } else {
        int i;
        pc_printf("recursion detected: function %s indirectly calls itself:\n", sym->name);
        pc_printf("%s ", sym->name);
        for (i=1; rsymstack[i]!=NULL; i++) {
          pc_printf("<- %s ", rsymstack[i]->name);
        }
        pc_printf("<- %s\n", sym->name);
      }
      *recursion=recursion_detected;
    }
    assert(size>=0);
    if (maxsize<size)
      maxsize=size;
  } /* for */

  free((void*)symstack);
  free((void*)rsymstack);
  maxsize++;                  /* +1 because a zero cell is always pushed on top
                               * of the stack to catch stack overwrites */
  return maxsize+(maxparams+1);/* +1 because # of parameters is always pushed on entry */
}
#endif

/*  testsymbols - test for unused local or global variables
 *
 *  "Public" functions are excluded from the check, since these
 *  may be exported to other object modules.
 *  Labels are excluded from the check if the argument 'testlabs'
 *  is 0. Thus, labels are not tested until the end of the function.
 *  Constants may also be excluded (convenient for global constants).
 *
 *  When the nesting level drops below "level", the check stops.
 *
 *  The function returns whether there is an "entry" point for the file.
 *  This flag will only be 1 when browsing the global symbol table.
 */
static int testsymbols(symbol *root,int level,int testlabs,int testconst)
{
  char symname[2*sNAMEMAX+16];
  int entry=FALSE;

  symbol *sym=root->next;
  while (sym!=NULL && sym->compound>=level) {
    switch (sym->ident) {
    case iLABEL:
      if (testlabs) {
        if ((sym->usage & uDEFINE)==0) {
          error_suggest(19,sym->name,NULL,estSYMBOL,esfLABEL);  /* not a label: ... */
        } else if ((sym->usage & uREAD)==0) {
          errorset(sSETPOS,sym->lnumber);
          error(203,sym->name);     /* symbol isn't used: ... */
          errorset(sSETPOS,-1);
        } /* if */
      } /* if */
      break;
    case iFUNCTN:
      if ((sym->usage & (uDEFINE | uREAD | uNATIVE | uSTOCK | uPUBLIC))==uDEFINE) {
        funcdisplayname(symname,sym->name);
        if (!strempty(symname))
          error(203,symname);       /* symbol isn't used ... (and not public/native/stock) */
      } /* if */
      if (((sym->usage & uPUBLIC)!=0 || strcmp(sym->name,uMAINFUNC)==0)
          && (sym->usage & uDEFINE)!=0)
        entry=TRUE;                 /* there is an entry point */
      /* also mark the function to the debug information */
      if (((sym->usage & uREAD)!=0 || (sym->usage & uPUBLIC)!=0) && (sym->usage & uNATIVE)==0)
        insert_dbgsymbol(sym);
      break;
    case iCONSTEXPR:
      if (testconst && (sym->usage & uREAD)==0) {
        errorset(sSETPOS,sym->lnumber);
        error(203,sym->name);       /* symbol isn't used: ... */
        errorset(sSETPOS,-1);
      } /* if */
      break;
    default:
      /* a variable */
      if (sym->parent!=NULL)
        break;                      /* hierarchical data type */
      if ((sym->usage & (uWRITTEN | uREAD | uSTOCK | uPUBLIC))==0) {
        errorset(sSETPOS,sym->lnumber);
        error(203,sym->name,sym->lnumber);  /* symbol isn't used (and not stock) */
        errorset(sSETPOS,-1);
      } else if ((sym->usage & (uREAD | uSTOCK | uPUBLIC))==0 && sym->ident!=iREFERENCE) {
        errorset(sSETPOS,sym->lnumber);
        error(204,sym->name);       /* value assigned to symbol is never used */
        errorset(sSETPOS,-1);
      } else if ((sym->usage & (uWRITTEN | uPUBLIC | uCONST))==0 && sym->ident==iREFARRAY) {
        int warn = TRUE;
        symbol *depend;
        for (depend=sym->child; depend!=NULL; depend=depend->child) {
          if ((depend->usage & (uWRITTEN | uPUBLIC | uCONST))!=0) {
            warn=FALSE;
            break;
          } /* if */
        } /* for */
        if (warn) {
          errorset(sSETPOS, sym->lnumber);
          error(214, sym->name);       /* make array argument "const" */
          errorset(sSETPOS, -1);
        } /* if */
      } /* if */
      /* also mark the variable (local or global) to the debug information */
      if ((sym->usage & (uWRITTEN | uREAD))!=0 && (sym->usage & uNATIVE)==0)
        insert_dbgsymbol(sym);
    } /* switch */
    sym=sym->next;
  } /* while */

  return entry;
}

static void scanloopvariables(symstate **loopvars,int dowhile)
{
  symbol *start,*sym;
  int num;

  /* error messages are only printed on the "writing" pass,
   * so if we are not writing yet, then we have a quick exit */
  if (sc_status!=statWRITE)
    return;

  /* if there's no enclosing loop (only one active loop entry, which is the
   * current loop), and the current loop is not 'do-while', then we don't need
   * to memoize usage flags for local variables, so we have an early exit */
  if (wqptr-wqSIZE==wq && !dowhile)
    return;

  /* skip labels */
  start=&loctab;
  while ((start=start->next)!=NULL && start->ident==iLABEL)
    /* nothing */;
  /* if there are no other local symbols, we have an early exit */
  if (start==NULL)
    return;

  /* count the number of local symbols */
  for (num=0,sym=start; sym!=NULL; num++,sym=sym->next)
    /* nothing */;

  assert(*loopvars==NULL);
  assert(num!=0);
  *loopvars=(symstate *)calloc((size_t)num,sizeof(symstate));
  if (*loopvars==NULL)
    error(103); /* insufficient memory */

  for (num=0,sym=start; sym!=NULL; num++,sym=sym->next) {
    /* If the variable already has the uLOOPVAR flag set (from being used
     * in an enclosing loop), we have to set the uNOLOOPVAR to exclude it
     * from checks in the current loop, ... */
    if ((sym->ident==iVARIABLE || sym->ident==iREFERENCE)
        && (dowhile || (sym->usage & uLOOPVAR)!=0)) {
      /* ... but it might be already set from an enclosing loop as well, so we
       * have to temporarily store it in "loopvars[num]" first. Also, if this is
       * a 'do-while' loop, we need to memoize and unset the 'uWRITTEN' flag, so
       * later when analyzing the loop condition (which comes after the loop
       * body) we'll be able to determine if the variable was modified inside
       * the loop body by checking if the 'uWRITTEN' flag is set. */
      (*loopvars)[num].usage |= (sym->usage & (uNOLOOPVAR | uWRITTEN));
      sym->usage &= ~uWRITTEN;
      if (wqptr-wqSIZE!=wq)
        sym->usage |= uNOLOOPVAR;
    } /* if */
  } /* if */
}

static void testloopvariables(symstate *loopvars,int dowhile,int line)
{
  symbol *start,*sym;
  int num,warnnum=0;

  /* the error messages are only printed on the "writing" pass,
   * so if we are not writing yet, then we have a quick exit */
  if (sc_status!=statWRITE)
    return;

  /* skip labels */
  start=&loctab;
  while ((start=start->next)!=NULL && start->ident==iLABEL)
    /* nothing */;

  /* decrement pc_numloopvars by 1 for each variable that wasn't modified
   * inside the loop body; if pc_numloopvars gets zeroed after this, it would
   * mean none of the variables used inside the loop condition were modified */
  if (pc_numloopvars!=0) {
    warnnum=(pc_numloopvars==1) ? 250 : 251;
    for (sym=start; sym!=NULL; sym=sym->next)
      if ((sym->ident==iVARIABLE || sym->ident==iREFERENCE)
          && (sym->usage & (uLOOPVAR | uNOLOOPVAR))==uLOOPVAR
          && (!dowhile || (sym->usage & uWRITTEN)==0))
        pc_numloopvars--;
    if (pc_numloopvars==0 && warnnum==251) {
      errorset(sSETPOS,line);
      error(251); /* none of the variables used in loop condition are modified in loop body */
      errorset(sSETPOS,-1);
    } /* if */
  } /* if */

  for (num=0,sym=start; sym!=NULL; num++,sym=sym->next) {
    if (sym->ident==iVARIABLE || sym->ident==iREFERENCE) {
      if ((sym->usage & (uLOOPVAR | uNOLOOPVAR))==uLOOPVAR) {
        sym->usage &= ~uLOOPVAR;
        /* warn only if none of the variables used inside the loop condition
         * were modified inside the loop body */
        if (pc_numloopvars==0 && warnnum==250) {
          errorset(sSETPOS,line);
          error(250,sym->name); /* variable used in loop condition not modified in loop body */
          errorset(sSETPOS,-1);
        } /* if */
      } /* if */
      sym->usage &= ~uNOLOOPVAR;
      if (loopvars!=NULL)
        sym->usage |= loopvars[num].usage;
    } /* if */
  } /* for */
  free(loopvars);
}

static cell calc_array_datasize(symbol *sym, cell *offset)
{
  cell length;

  assert(sym!=NULL);
  assert(sym->ident==iARRAY || sym->ident==iREFARRAY);
  length=sym->dim.array.length;
  if (sym->dim.array.level > 0) {
    cell sublength=calc_array_datasize(sym->child,offset);
    if (offset!=NULL)
      *offset=length*(*offset+sizeof(cell));
    if (sublength>0)
      length*=sublength;
    else
      length=0;
  } else {
    if (offset!=NULL)
      *offset=0;
  } /* if */
  return length;
}

static void destructsymbols(symbol *root,int level)
{
  cell offset=0;
  int savepri=FALSE;
  symbol *sym=root->next;
  while (sym!=NULL && sym->compound>=level) {
    if ((sym->ident==iVARIABLE || sym->ident==iARRAY) && !(sym->vclass==sSTATIC && sym->fnumber==-1) && !(sym->usage&uNODESTRUCT)) {
      char symbolname[16];
      symbol *opsym;
      cell elements;
      /* check that the '~' operator is defined for this tag */
      operator_symname(symbolname,"~",sym->tag,0,1,0);
      if ((opsym=findglb(symbolname,sGLOBAL))!=NULL) {
        if ((opsym->usage & uMISSING)!=0 || (opsym->usage & uPROTOTYPED)==0) {
          char symname[2*sNAMEMAX+16];  /* allow space for user defined operators */
          char *ptr= (sym->documentation!=NULL) ? sym->documentation : "";
          funcdisplayname(symname,opsym->name);
          if ((opsym->usage & uMISSING)!=0)
            error(4,symname,ptr);       /* function not defined */
          if ((opsym->usage & uPROTOTYPED)==0)
            error(71,symname);          /* operator must be declared before use */
        } /* if */
        /* save PRI, in case of a return statement */
        if (!savepri) {
          pushreg(sPRI);        /* right-hand operand is in PRI */
          savepri=TRUE;
        } /* if */
        /* if the variable is an array, get the number of elements */
        if (sym->ident==iARRAY) {
          /* according to the PAWN Implementer Guide, the destructor
           * should be triggered for the data of the array only; hence
           * if the array is a part of a larger array, it must be ignored
           * as it's parent would(or has already) trigger(ed) the destructor
           */
          if (sym->parent!=NULL) {
            sym=sym->next;
            continue;
          } /* if */
          elements=calc_array_datasize(sym,&offset);
          /* "elements" can be zero when the variable is declared like
           *    new mytag: myvar[2][] = { {1, 2}, {3, 4} }
           * one should declare all dimensions!
           */
          if (elements==0)
            error(46,sym->name);        /* array size is unknown */
        } else {
          elements=1;
          offset=0;
        } /* if */
        pushval(elements);
        /* call the '~' operator */
        address(sym,sPRI);
        addconst(offset);       /* add offset to array data to the address */
        pushreg(sPRI);
        pushval(2*sizeof(cell));/* 2 parameters */
        assert(opsym->ident==iFUNCTN);
        ffcall(opsym,NULL,1);
        if (sc_status!=statSKIP)
          markusage(opsym,uREAD);   /* do not mark as "used" when this call itself is skipped */
        if ((opsym->usage & uNATIVE)!=0 && opsym->x.lib!=NULL)
          opsym->x.lib->value += 1; /* increment "usage count" of the library */
      } /* if */
    } else if (level==0 && sym->ident==iREFERENCE) {
      sym->usage &= ~uASSIGNED;
    } /* if */
    sym=sym->next;
  } /* while */
  /* restore PRI, if it was saved */
  if (savepri)
    popreg(sPRI);
}

static constvalue *insert_constval(constvalue *prev,constvalue *next,
                                   const char *name,cell val,int index)
{
  constvalue *cur;

  if ((cur=(constvalue*)malloc(sizeof(constvalue)))==NULL)
    error(103);       /* insufficient memory (fatal error) */
  memset(cur,0,sizeof(constvalue));
  if (name!=NULL) {
    assert(strlen(name)<sNAMEMAX+1);
    strcpy(cur->name,name);
  } /* if */
  cur->value=val;
  cur->index=index;
  cur->next=next;
  if (prev!=NULL)
    prev->next=cur;
  return cur;
}

SC_FUNC constvalue *append_constval(constvalue_root *table,const char *name,
                                    cell val,int index)
{
  constvalue *newvalue;

  if (table->last!=NULL) {
    newvalue=insert_constval(table->last,NULL,name,val,index);
  } else {
    newvalue=insert_constval(NULL,NULL,name,val,index);
    table->first=newvalue;
  } /* if */
  table->last=newvalue;
  return newvalue;
}

SC_FUNC constvalue *find_constval(constvalue_root *table,char *name,int index)
{
  constvalue *ptr = table->first;

  while (ptr!=NULL) {
    if (strcmp(name,ptr->name)==0 && ptr->index==index)
      return ptr;
    ptr=ptr->next;
  } /* while */
  return NULL;
}

static constvalue *find_constval_byval(constvalue_root *table,cell val)
{
  constvalue *ptr = table->first;

  while (ptr!=NULL) {
    if (ptr->value==val)
      return ptr;
    ptr=ptr->next;
  } /* while */
  return NULL;
}

#if 0   /* never used */
static int delete_constval(constvalue_root *table,char *name)
{
  constvalue *prev=NULL;
  constvalue *cur=table->first;

  while (cur!=NULL) {
    if (strcmp(name,cur->name)==0) {
      if (prev!=NULL)
        prev->next=cur->next;
      else
        table->first=cur->next;
      if (table->last==cur)
        table->last=prev;
      free(cur);
      return TRUE;
    } /* if */
    prev=cur;
    cur=cur->next;
  } /* while */
  return FALSE;
}
#endif

SC_FUNC void delete_consttable(constvalue_root *table)
{
  constvalue *cur=table->first, *next;

  while (cur!=NULL) {
    next=cur->next;
    free(cur);
    cur=next;
  } /* while */
  memset(table,0,sizeof(constvalue_root));
}

/*  add_constant
 *
 *  Adds a symbol to the symbol table. Returns NULL on failure.
 */
SC_FUNC symbol *add_constant(char *name,cell val,int vclass,int tag)
{
  symbol *sym;

  /* Test whether a global or local symbol with the same name exists. Since
   * constants are stored in the symbols table, this also finds previously
   * defined constants. */
  sym=findglb(name,sSTATEVAR);
  if (sym==NULL)
    sym=findloc(name);
  if (sym!=NULL) {
    int redef=FALSE;
    if (sym->ident!=iCONSTEXPR)
      redef=TRUE;               /* redefinition of a function/variable to a constant is not allowed */
    if ((sym->usage & uENUMFIELD)!=0) {
      /* enum field, special case if it has a different tag and the new symbol is also an enum field */
      constvalue *tagid;
      symbol *tagsym;
      if (sym->tag==tag)
        redef=TRUE;             /* enumeration field is redefined (same tag) */
      tagid=find_tag_byval(tag);
      if (tagid==NULL) {
        redef=TRUE;             /* new constant does not have a tag */
      } else {
        tagsym=findconst(tagid->name,NULL);
        if (tagsym==NULL || (tagsym->usage & uENUMROOT)==0)
          redef=TRUE;           /* new constant is not an enumeration field */
      } /* if */
      /* in this particular case (enumeration field that is part of a different
       * enum, and non-conflicting with plain constants) we want to be able to
       * redefine it
       */
      if (!redef)
        goto redef_enumfield;
    } else if (sym->tag!=tag) {
      redef=TRUE;               /* redefinition of a constant (non-enum) to a different tag is not allowed */
    } /* if */
    if (redef) {
      error(21,name);           /* symbol already defined */
      return NULL;
    } else if (sym->addr!=val || (sym->usage & uENUMROOT)!=0) {
      error(201,name);          /* redefinition of constant (different value) */
      sym->addr=val;            /* set new value */
    } /* if */
    /* silently ignore redefinitions of constants with the same value & tag */
    return sym;
  } /* if */

  /* constant doesn't exist yet (or is allowed to be redefined) */
redef_enumfield:
  sym=addsym(name,val,iCONSTEXPR,vclass,tag,uDEFINE);
  assert(sym!=NULL);            /* fatal error 103 must be given on error */
  if (vclass==sLOCAL)
    sym->compound=pc_nestlevel;
  return sym;
}

/* add_builtin_constant
 *
 * Adds a predefined constant to the symbol table.
 */
SC_FUNC symbol *add_builtin_constant(char *name,cell val,int vclass,int tag)
{
  symbol *sym;

  sym=add_constant(name,val,vclass,tag);
  sym->flags|=flagPREDEF;
  return sym;
}

/*  add_builtin_string_constant
 *
 *  Adds a predefined string constant to the symbol table.
 */
SC_FUNC symbol *add_builtin_string_constant(char *name,const char *val,
                                            int vclass)
{
  symbol *sym;

  /* Test whether a global or local symbol with the same name exists. Since
   * constants are stored in the symbols table, this also finds previously
   * defined constants. */
  sym=findglb(name,sSTATEVAR);
  if (sym==NULL)
    sym=findloc(name);
  if (sym!=NULL) {
    if (sym->ident!=iARRAY) {
      error(21,name);           /* symbol already defined */
      return NULL;
    } /* if */
  } else {
    sym=addsym(name,0,iARRAY,vclass,0,uDEFINE | uSTOCK);
  } /* if */
  sym->addr=(litidx+glb_declared)*sizeof(cell);
  /* Store this constant only if it's used somewhere. This can be detected
   * in the second stage. */
  if (sc_status==statIDLE
      || (sc_status==statWRITE && (sym->usage & uREAD)!=0)) {
    assert(litidx==0);
    begdseg();
    while (*val!='\0')
      litadd(*val++);
    litadd(0);
    glb_declared+=litidx;
    dumplits();
    litidx=0;
  }
  sym->usage|=uDEFINE;
  sym->flags|=flagPREDEF;
  return sym;
}

/*  statement           - The Statement Parser
 *
 *  This routine is called whenever the parser needs to know what statement
 *  it encounters (i.e. whenever program syntax requires a statement).
 */
static void statement(int *lastindent,int allow_decl)
{
  int tok,save;
  cell val;
  char *st;

  if (!freading) {
    error(36);                  /* empty statement */
    return;
  } /* if */
  errorset(sRESET,0);

  tok=lex(&val,&st);
  if ((emit_flags & efBLOCK)!=0) {
    emit_parse_line();
    if (matchtoken('}'))
      emit_flags &= ~efBLOCK;
    return;
  } /* if */
  if (tok!='{') {
    insert_dbgline(fline);
    setline(TRUE);
  } /* if */
  /* lex() has set stmtindent */
  if (lastindent!=NULL && tok!=tLABEL) {
    if (*lastindent>=0 && *lastindent!=stmtindent && !indent_nowarn && sc_tabsize>0)
      error(217);               /* loose indentation */
    *lastindent=stmtindent;
    indent_nowarn=FALSE;        /* if warning was blocked, re-enable it */
  } /* if */
  switch (tok) {
  case 0:
    /* nothing */
    break;
  case tSTOCK:
    error(10);                  /* invalid function or declaration */
    /* fallthrough */
  case tNEW:
    if (allow_decl) {
      declloc(FALSE);
      lastst=tNEW;
    } else {
      error(3);                 /* declaration only valid in a block */
    } /* if */
    break;
  case tSTATIC:
    if (matchtoken(tENUM))
      decl_enum(sLOCAL,FALSE);
    else if (allow_decl) {
      declloc(TRUE);
      lastst=tNEW;
    } else {
      error(3);                 /* declaration only valid in a block */
    } /* if */
    break;
  case '{':
  case tBEGIN:
    save=fline;
    if (!matchtoken('}'))       /* {} is the empty statement */
      compound(save==fline,tok);
    /* lastst (for "last statement") does not change */
    break;
  case ';':
    error(36);                  /* empty statement */
    break;
  case tIF:
    lastst=doif();
    break;
  case tWHILE:
    lastst=dowhile();
    break;
  case tDO:
    lastst=dodo();
    break;
  case tFOR:
    lastst=dofor();
    break;
  case tSWITCH:
    lastst=doswitch();
    break;
  case tCASE:
  case tDEFAULT:
    docase(tok==tDEFAULT);
    break;
  case tGOTO:
    lastst=dogoto();
    break;
  case tLABEL:
    dolabel();
    lastst=tLABEL;
    break;
  case tRETURN:
    doreturn();
    lastst=tRETURN;
    break;
  case tBREAK:
    dobreak();
    lastst=tBREAK;
    break;
  case tCONTINUE:
    docont();
    lastst=tCONTINUE;
    break;
  case tEXIT:
    doexit();
    lastst=tEXIT;
    break;
  case tASSERT:
    doassert();
    lastst=tASSERT;
    break;
  case tSLEEP:
    dosleep();
    lastst=tSLEEP;
    break;
  case tSTATE:
    dostate();
    lastst=tSTATE;
    break;
  case tCONST:
    decl_const(sLOCAL);
    break;
  case tENUM:
    matchtoken(tSTATIC);
    decl_enum(sLOCAL,FALSE);
    break;
  case t__PRAGMA:
    dopragma();
    needtoken(tTERM);
    pragma_apply(curfunc);
    break;
  case t__EMIT: {
    const unsigned char *bck_lptr=lptr-strlen(sc_tokens[tok-tFIRST]);
    if (matchtoken('{')) {
      emit_flags |= efBLOCK;
      lastst=t__EMIT;
      break;
    } /* if */
    lptr=bck_lptr;
    lexclr(FALSE);
    tok=lex(&val,&st);
  } /* case */
  /* fallthrough */
  default:          /* non-empty expression */
    sc_allowproccall=optproccall;
    if (!allow_decl)
      pc_nestlevel++;
    lexpush();      /* analyze token later */
    doexpr(TRUE,TRUE,TRUE,TRUE,NULL,NULL,FALSE,NULL);
    needtoken(tTERM);
    lastst=tEXPR;
    if (!allow_decl)
      pc_nestlevel--;
    sc_allowproccall=FALSE;
  } /* switch */
}

static void compound(int stmt_sameline,int starttok)
{
  int indent=-1;
  cell save_decl=declared;
  int count_stmt=0;
  int block_start=fline;  /* save line where the compound block started */
  int endtok;

  /* if there is more text on this line, we should adjust the statement indent */
  if (stmt_sameline) {
    int i;
    const unsigned char *p=lptr;
    /* go back to the opening brace */
    while (*p!=starttok) {
      assert(p>pline);
      p--;
    } /* while */
    assert(*p==starttok);  /* it should be found */
    /* go forward, skipping white-space */
    p++;
    while (*p<=' ' && *p!='\0')
      p++;
    assert(*p!='\0'); /* a token should be found */
    stmtindent=0;
    for (i=0; i<(int)(p-pline); i++)
      if (pline[i]=='\t' && sc_tabsize>0)
        stmtindent += (int)(sc_tabsize - (stmtindent+sc_tabsize) % sc_tabsize);
      else
        stmtindent++;
  } /* if */

  endtok=(starttok=='{') ? '}' : tEND;
  pc_nestlevel+=1;              /* increase compound statement level */
  while (matchtoken(endtok)==0){/* repeat until compound statement is closed */
    if (!freading){
      error(30,block_start);    /* compound block not closed at end of file */
      break;
    } else {
      if (count_stmt>0 && isterminal(lastst)) {
        if (matchtoken(tLABEL)) {
          cell val;
          char *name;
          symbol *sym;
          tokeninfo(&val,&name);
          lexpush();            /* push the token so it can be analyzed later */
          sym=findloc(name);
          /* before issuing a warning, check if the label was previously used (via 'goto') */
          if (sym!=NULL && sym->ident==iLABEL && (sym->usage & uREAD)==0)
            error(225);         /* unreachable code */
        } else if (lastst==tTERMSWITCH && matchtoken(tRETURN)) {
          lexpush();            /* push the token so it can be analyzed later */
        } else {
          error(225);           /* unreachable code */
        } /* if */
      } /* if */
      statement(&indent,TRUE);  /* do a statement */
      count_stmt++;
    } /* if */
  } /* while */
  if (lastst!=tRETURN)
    if (pc_nestlevel >= 1 || (curfunc->flags & flagNAKED)==0)
      destructsymbols(&loctab,pc_nestlevel);
  if (!isterminal(lastst) && (pc_nestlevel>=1 || (curfunc->flags & flagNAKED)==0))
      modstk((int)(declared-save_decl)*sizeof(cell)); /* delete local variable space */
  testsymbols(&loctab,pc_nestlevel,FALSE,TRUE);     /* look for unused block locals */
  declared=save_decl;
  delete_symbols(&loctab,pc_nestlevel,FALSE,TRUE);  /* erase local symbols, but
                                                     * retain block local labels
                                                     * (within the function) */
  pc_nestlevel-=1;              /* decrease compound statement level */
}

/*  doexpr
 *
 *  Global references: stgidx   (referred to only)
 */
static int doexpr(int comma,int chkeffect,int allowarray,int mark_endexpr,
                  int *tag,symbol **symptr,int chkfuncresult,cell *val)
{
  int index,ident;
  int localstaging=FALSE;

  if (!staging) {
    stgset(TRUE);               /* start stage-buffering */
    localstaging=TRUE;
    assert(stgidx==0);
  } /* if */
  index=stgidx;
  errorset(sEXPRMARK,0);
  do {
    /* on second round through, mark the end of the previous expression */
    if (index!=stgidx) {
      markexpr(sEXPR,NULL,0);
      /* also, if this is not the first expression and we are inside a "return"
       * statement, we need to manually free the heap space allocated for the
       * array returned by the function called in the previous expression */
      if (pc_retexpr) {
        modheap(pc_retheap);
        pc_retheap=0;
      } /* if */
    } /* if */
    pc_sideeffect=FALSE;
    pc_ovlassignment=FALSE;
    ident=expression(val,tag,symptr,chkfuncresult);
    if (!allowarray && (ident==iARRAY || ident==iREFARRAY))
      error(33,"-unknown-");    /* array must be indexed */
    if (chkeffect && !pc_sideeffect)
      error(215);               /* expression has no effect */
    sc_allowproccall=FALSE;     /* cannot use "procedure call" syntax anymore */
  } while (comma && matchtoken(',')); /* more? */
  if (mark_endexpr)
    markexpr(sEXPR,NULL,0);     /* optionally, mark the end of the expression */
  errorset(sEXPRRELEASE,0);
  if (localstaging) {
    stgout(index);
    stgset(FALSE);              /* stop staging */
  } /* if */
  return ident;
}

/*  constexpr
 */
SC_FUNC int constexpr(cell *val,int *tag,symbol **symptr)
{
  int ident,index;
  cell cidx;

  stgset(TRUE);         /* start stage-buffering */
  stgget(&index,&cidx); /* mark position in code generator */
  errorset(sEXPRMARK,0);
  ident=expression(val,tag,symptr,FALSE);
  stgdel(index,cidx);   /* scratch generated code */
  stgset(FALSE);        /* stop stage-buffering */
  if (ident!=iCONSTEXPR) {
    error(8);           /* must be constant expression */
    if (val!=NULL)
      *val=0;
    if (tag!=NULL)
      *tag=0;
    if (symptr!=NULL)
      *symptr=NULL;
  } /* if */
  errorset(sEXPRRELEASE,0);
  return (ident==iCONSTEXPR);
}

/*  test
 *
 *  In the case a "simple assignment" operator ("=") is used within a test,
 *  the warning "possibly unintended assignment" is displayed. This routine
 *  sets the global variable "sc_intest" to true, it is restored upon termination.
 *  In the case the assignment was intended, use parentheses around the
 *  expression to avoid the warning; primary() sets "sc_intest" to 0.
 *
 *  Global references: sc_intest (altered, but restored upon termination)
 */
static int test(int label,int parens,int invert)
{
  int index,tok;
  cell cidx;
  int ident,tag;
  int endtok;
  cell constval;
  symbol *sym;
  int localstaging=FALSE;

  if (!staging) {
    stgset(TRUE);               /* start staging */
    localstaging=TRUE;
    #if !defined NDEBUG
      stgget(&index,&cidx);     /* should start at zero if started locally */
      assert(index==0);
    #endif
  } /* if */

  PUSHSTK_I(sc_intest);
  sc_intest=TRUE;
  endtok=0;
  if (parens!=TEST_PLAIN) {
    if (matchtoken('('))
      endtok=')';
    else if (parens==TEST_THEN)
      endtok=tTHEN;
    else if (parens==TEST_DO)
      endtok=tDO;
  } /* if */
  do {
    stgget(&index,&cidx);       /* mark position (of last expression) in
                                 * code generator */
    pc_sideeffect=FALSE;
    ident=expression(&constval,&tag,&sym,TRUE);
    tok=matchtoken(',');
    if (tok) {
      if (!pc_sideeffect)
        error(248);
      markexpr(sEXPR,NULL,0);
    } /* if */
  } while (tok); /* do */
  if (endtok!=0)
    needtoken(endtok);
  if (ident==iARRAY || ident==iREFARRAY) {
    char *ptr=(sym!=NULL) ? sym->name : "-unknown-";
    error(33,ptr);              /* array must be indexed */
  } /* if */
  if (ident==iCONSTEXPR) {      /* constant expression */
    int testtype=0;
    sc_intest=(short)POPSTK_I();/* restore stack */
    stgdel(index,cidx);
    if (constval) {             /* code always executed */
      error(206);               /* redundant test: always non-zero */
      testtype=tENDLESS;
    } else {
      error(205);               /* redundant code: never executed */
      jumplabel(label);
    } /* if */
    if (localstaging) {
      stgout(0);                /* write "jumplabel" code */
      stgset(FALSE);            /* stop staging */
    } /* if */
    return testtype;
  } /* if */
  if (tag!=0 && tag!=BOOLTAG)
    if (check_userop(lneg,tag,0,1,NULL,&tag))
      invert= !invert;          /* user-defined ! operator inverted result */
  if (invert)
    jmp_ne0(label);             /* jump to label if true (different from 0) */
  else
    jmp_eq0(label);             /* jump to label if false (equal to 0) */
  markexpr(sEXPR,NULL,0);       /* end expression (give optimizer a chance) */
  sc_intest=(short)POPSTK_I();  /* double typecast to avoid warning with Microsoft C */
  if (localstaging) {
    stgout(0);                  /* output queue from the very beginning (see
                                 * assert() when localstaging is set to TRUE) */
    stgset(FALSE);              /* stop staging */
  } /* if */
  return 0;
}

static int doif(void)
{
  int flab1,flab2;
  int ifindent;
  int lastst_true;
  int returnst=tIF;
  symstate *assignments=NULL;

  lastst=0;                     /* reset the last statement */
  ifindent=stmtindent;          /* save the indent of the "if" instruction */
  flab1=getlabel();             /* get label number for false branch */
  test(flab1,TEST_THEN,FALSE);  /* get expression, branch to flab1 if false */
  statement(NULL,FALSE);        /* if true, do a statement */
  if (!matchtoken(tELSE)) {     /* if...else ? */
    setlabel(flab1);            /* no, simple if..., print false label */
  } else {
    lastst_true=lastst;         /* save last statement of the "true" branch */
    lastst=0;                   /* reset the last statement */
    /* to avoid the "dangling else" error, we want a warning if the "else"
     * has a lower indent than the matching "if" */
    if (stmtindent<ifindent && sc_tabsize>0)
      error(217);               /* loose indentation */
    memoizeassignments(pc_nestlevel+1,&assignments);
    flab2=getlabel();
    if (!isterminal(lastst))
      jumplabel(flab2);         /* "true" branch jumps around "else" clause, unless the "true" branch statement already jumped */
    setlabel(flab1);            /* print false label */
    statement(NULL,FALSE);      /* do "else" clause */
    setlabel(flab2);            /* print true label */
    /* if both the "true" branch and the "false" branch ended with the same
     * kind of statement, set the last statement id to that kind, rather than
     * to the generic tIF; this allows for better "unreachable code" checking
     */
    if (lastst==lastst_true && lastst!=0)
      returnst=lastst;
    /* otherwise, if both branches end with terminal statements (not necessary
     * of the same kind), set the last statement ID to tTERMINAL */
    else if (isterminal(lastst_true) && isterminal(lastst))
      returnst=tTERMINAL;
  } /* if */
  restoreassignments(pc_nestlevel+1,assignments);
  return returnst;
}

static int dowhile(void)
{
  int wq[wqSIZE];               /* allocate local queue */
  int save_endlessloop,save_numloopvars,retcode;
  int loopline=fline;
  symstate *loopvars=NULL;

  save_endlessloop=endlessloop;
  save_numloopvars=pc_numloopvars;
  pc_numloopvars=0;
  addwhile(wq);                 /* add entry to queue for "break" */
  setlabel(wq[wqLOOP]);         /* loop label */
  /* The debugger uses the "break" opcode to be able to "break" out of
   * a loop. To make sure that each loop has a break opcode, even for the
   * tiniest loop, set it below the top of the loop
   */
  setline(TRUE);
  scanloopvariables(&loopvars,FALSE);
  pc_nestlevel++; /* temporarily increase the "compound statement" nesting level,
                   * so any assignments made inside the loop control expression
                   * could be cleaned up later */
  pc_loopcond=tWHILE;
  endlessloop=test(wq[wqEXIT],TEST_DO,FALSE);/* branch to wq[wqEXIT] if false */
  pc_loopcond=0;
  pc_nestlevel--;
  statement(NULL,FALSE);        /* if so, do a statement */
  clearassignments(pc_nestlevel+1);
  testloopvariables(loopvars,FALSE,loopline);
  jumplabel(wq[wqLOOP]);        /* and loop to "while" start */
  setlabel(wq[wqEXIT]);         /* exit label */
  delwhile();                   /* delete queue entry */

  retcode=endlessloop ? tENDLESS : tWHILE;
  pc_numloopvars=save_numloopvars;
  endlessloop=save_endlessloop;
  return retcode;
}

/*
 *  Note that "continue" will in this case not jump to the top of the loop, but
 *  to the end: just before the TRUE-or-FALSE testing code.
 */
static int dodo(void)
{
  int wq[wqSIZE],top;
  int save_endlessloop,save_numloopvars,retcode;
  int loopline=fline;
  symstate *loopvars=NULL;

  save_endlessloop=endlessloop;
  save_numloopvars=pc_numloopvars;
  pc_numloopvars=0;
  addwhile(wq);           /* see "dowhile" for more info */
  top=getlabel();         /* make a label first */
  setlabel(top);          /* loop label */
  scanloopvariables(&loopvars,TRUE);
  statement(NULL,FALSE);
  needtoken(tWHILE);
  setlabel(wq[wqLOOP]);   /* "continue" always jumps to WQLOOP. */
  setline(TRUE);
  pc_nestlevel++; /* temporarily increase the "compound statement" nesting level,
                   * so any assignments made inside the loop control expression
                   * could be cleaned up later */
  pc_loopcond=tDO;
  endlessloop=test(wq[wqEXIT],TEST_OPT,FALSE);
  pc_loopcond=0;
  pc_nestlevel--;
  clearassignments(pc_nestlevel+1);
  testloopvariables(loopvars,TRUE,loopline);
  jumplabel(top);
  setlabel(wq[wqEXIT]);
  delwhile();
  needtoken(tTERM);

  retcode=endlessloop ? tENDLESS : tDO;
  pc_numloopvars=save_numloopvars;
  endlessloop=save_endlessloop;
  return retcode;
}

static int dofor(void)
{
  int wq[wqSIZE],skiplab;
  cell save_decl;
  int save_nestlevel,save_endlessloop,save_numloopvars;
  int index,endtok;
  int *ptr;
  int loopline=fline;
  symstate *loopvars=NULL;

  save_decl=declared;
  save_nestlevel=pc_nestlevel;
  save_endlessloop=endlessloop;
  save_numloopvars=pc_numloopvars;
  pc_numloopvars=0;

  addwhile(wq);
  skiplab=getlabel();
  endtok= matchtoken('(') ? ')' : tDO;
  pc_nestlevel++; /* temporarily increase the "compound statement" nesting level,
                   * so any assignments made inside the loop initialization, control
                   * expression and increment blocks could be cleaned up later */
  if (matchtoken(';')==0) {
    /* new variable declarations are allowed here */
    if (matchtoken(tNEW)) {
      /* The variable in expr1 of the for loop is at a
       * 'compound statement' level of it own.
       */
      declloc(FALSE); /* declare local variable */
    } else {
      doexpr(TRUE,TRUE,TRUE,TRUE,NULL,NULL,FALSE,NULL); /* expression 1 */
      needtoken(';');
    } /* if */
  } /* if */
  /* Adjust the "declared" field in the "while queue", in case that
   * local variables were declared in the first expression of the
   * "for" loop. These are deleted in separately, so a "break" or a "continue"
   * must ignore these fields.
   */
  ptr=readwhile();
  assert(ptr!=NULL);
  ptr[wqBRK]=(int)declared;
  ptr[wqCONT]=(int)declared;
  ptr[wqLVL]=pc_nestlevel+1;
  jumplabel(skiplab);               /* skip expression 3 1st time */
  setlabel(wq[wqLOOP]);             /* "continue" goes to this label: expr3 */
  setline(TRUE);
  scanloopvariables(&loopvars,FALSE);
  /* Expressions 2 and 3 are reversed in the generated code: expression 3
   * precedes expression 2. When parsing, the code is buffered and marks for
   * the start of each expression are inserted in the buffer.
   */
  assert(!staging);
  stgset(TRUE);                     /* start staging */
  assert(stgidx==0);
  index=stgidx;
  stgmark(sSTARTREORDER);
  stgmark((char)(sEXPRSTART+0));    /* mark start of 2nd expression in stage */
  setlabel(skiplab);                /* jump to this point after 1st expression */
  if (matchtoken(';')) {
    endlessloop=1;
  } else {
    pc_loopcond=tFOR;
    endlessloop=test(wq[wqEXIT],TEST_PLAIN,FALSE);/* expression 2 (jump to wq[wqEXIT] if false) */
    pc_loopcond=0;
    needtoken(';');
  } /* if */
  stgmark((char)(sEXPRSTART+1));    /* mark start of 3th expression in stage */
  if (!matchtoken(endtok)) {
    doexpr(TRUE,TRUE,TRUE,TRUE,NULL,NULL,FALSE,NULL);   /* expression 3 */
    needtoken(endtok);
  } /* if */
  stgmark(sENDREORDER);             /* mark end of reversed evaluation */
  stgout(index);
  stgset(FALSE);                    /* stop staging */
  statement(NULL,FALSE);
  clearassignments(save_nestlevel+1);
  testloopvariables(loopvars,FALSE,loopline);
  jumplabel(wq[wqLOOP]);
  setlabel(wq[wqEXIT]);
  delwhile();

  assert(pc_nestlevel>save_nestlevel);
  if (declared>save_decl) {
    /* Clean up the space and the symbol table for the local
     * variable in "expr1".
     */
    destructsymbols(&loctab,pc_nestlevel);
    modstk((int)(declared-save_decl)*sizeof(cell));
    testsymbols(&loctab,pc_nestlevel,FALSE,TRUE);   /* look for unused block locals */
    declared=save_decl;
    delete_symbols(&loctab,pc_nestlevel,FALSE,TRUE);
  } /* if */
  pc_nestlevel=save_nestlevel;    /* reset 'compound statement' nesting level */

  index=endlessloop ? tENDLESS : tFOR;
  pc_numloopvars=save_numloopvars;
  endlessloop=save_endlessloop;
  return index;
}

/* The switch statement is incompatible with its C sibling:
 * 1. the cases are not drop through
 * 2. only one instruction may appear below each case, use a compound
 *    instruction to execute multiple instructions
 * 3. the "case" keyword accepts a comma separated list of values to
 *    match, it also accepts a range using the syntax "1 .. 4"
 *
 * SWITCH param
 *   PRI = expression result
 *   param = table offset (code segment)
 *
 */
static int doswitch(void)
{
  int lbl_table,lbl_exit,lbl_case;
  int swdefault,casecount;
  int tok,endtok;
  int swtag,csetag;
  int allterminal;
  int enumsymcount;
  int save_fline;
  symbol *enumsym,*csesym;
  int ident;
  cell val;
  char *str;
  constvalue_root caselist = { NULL, NULL};   /* case list starts empty */
  constvalue *cse,*csp,*newval;
  char labelname[sNAMEMAX+1];
  symstate *assignments=NULL;

  endtok= matchtoken('(') ? ')' : tDO;
  ident=doexpr(TRUE,FALSE,FALSE,FALSE,&swtag,NULL,TRUE,NULL);   /* evaluate switch expression */
  if (ident==iCONSTEXPR)
    error(243);                 /* redundant code: switch control expression is constant */
  needtoken(endtok);
  /* generate the code for the switch statement, the label is the address
   * of the case table (to be generated later).
   */
  lbl_table=getlabel();
  lbl_case=0;                   /* just to avoid a compiler warning */
  ffswitch(lbl_table);

  save_fline=fline;
  enumsym=NULL;
  if (swtag!=0) {
    constvalue *tagsym=find_tag_byval(swtag);
    assert(tagsym->name!=NULL);
    enumsymcount=0;
    enumsym=findconst(tagsym->name,NULL);
    if (enumsym!=NULL && (enumsym->tag!=swtag || enumsym->dim.enumlist==NULL))
      enumsym=NULL;
  } /* if */

  if (matchtoken(tBEGIN)) {
    endtok=tEND;
  } else {
    endtok='}';
    needtoken('{');
  } /* if */
  lbl_exit=getlabel();          /* get label number for jumping out of switch */
  swdefault=FALSE;
  allterminal=TRUE;             /* assume that all cases end with terminal statements */
  casecount=0;
  do {
    tok=lex(&val,&str);         /* read in (new) token */
    switch (tok) {
    case tCASE:
      lastst=0;
      if (casecount!=0)
        memoizeassignments(pc_nestlevel+1,&assignments);
      if (swdefault!=FALSE)
        error(15);        /* "default" case must be last in switch statement */
      lbl_case=getlabel();
      PUSHSTK_I(sc_allowtags);
      sc_allowtags=FALSE; /* do not allow tagnames here */
      do {
        casecount++;

        /* ??? enforce/document that, in a switch, a statement cannot start
         *     with a label. Then, you can search for:
         *     * the first semicolon (marks the end of a statement)
         *     * an opening brace (marks the start of a compound statement)
         *     and search for the right-most colon before that statement
         *     Now, by replacing the ':' by a special COLON token, you can
         *     parse all expressions until that special token.
         */

        constexpr(&val,&csetag,&csesym);
        check_tagmismatch(swtag,csetag,TRUE,-1);
        if (enumsym!=NULL) {
          if (csesym!=NULL && csesym->parent==enumsym)
            enumsymcount++;
          else
            enumsym=NULL;
        } /* if */
        /* Search the insertion point (the table is kept in sorted order, so
         * that advanced abstract machines can sift the case table with a
         * binary search). Check for duplicate case values at the same time.
         */
        for (csp=NULL, cse=caselist.first;
             cse!=NULL && cse->value<val;
             csp=cse, cse=cse->next)
          /* nothing */;
        if (cse!=NULL && cse->value==val)
          error(40,val);                /* duplicate "case" label */
        /* Since the label is stored as a string in the "constvalue", the
         * size of an identifier must be at least 8, as there are 8
         * hexadecimal digits in a 32-bit number.
         */
        #if sNAMEMAX < 8
          #error Length of identifier (sNAMEMAX) too small.
        #endif
        assert(csp==NULL || csp->next==cse);
        newval=insert_constval(csp,cse,itoh(lbl_case),val,0);
        if (csp==NULL)
          caselist.first=newval;
        if (matchtoken(tDBLDOT)) {
          cell end;
          constexpr(&end,&csetag,NULL);
          if (end<=val)
            error(50);                  /* invalid range */
          check_tagmismatch(swtag,csetag,TRUE,-1);
          enumsym=NULL; /* stop counting the number of covered enum elements */
          while (++val<=end) {
            casecount++;
            /* find the new insertion point */
            for (csp=NULL, cse=caselist.first;
                 cse!=NULL && cse->value<val;
                 csp=cse, cse=cse->next)
              /* nothing */;
            if (cse!=NULL && cse->value==val)
              error(40,val);            /* duplicate "case" label */
            assert(csp==NULL || csp->next==cse);
            insert_constval(csp,cse,itoh(lbl_case),val,0);
          } /* while */
        } /* if */
      } while (matchtoken(','));
      needtoken(':');                   /* ':' ends the case */
      sc_allowtags=(short)POPSTK_I();   /* reset */
      setlabel(lbl_case);
      statement(NULL,FALSE);
      jumplabel(lbl_exit);
      allterminal &= isterminal(lastst);
      break;
    case tDEFAULT:
      lastst=0;
      if (casecount!=0)
        memoizeassignments(pc_nestlevel+1,&assignments);
      if (swdefault!=FALSE)
        error(16);         /* multiple defaults in switch */
      lbl_case=getlabel();
      setlabel(lbl_case);
      needtoken(':');
      swdefault=TRUE;
      statement(NULL,FALSE);
      /* Jump to lbl_exit, even thouh this is the last clause in the
       * switch, because the jump table is generated between the last
       * clause of the switch and the exit label.
       */
      jumplabel(lbl_exit);
      allterminal &= isterminal(lastst);
      break;
    default:
      if (tok!=endtok) {
        error(2);
        indent_nowarn=TRUE; /* disable this check */
        tok=endtok;     /* break out of the loop after an error */
      } /* if */
    } /* switch */
  } while (tok!=endtok);
  restoreassignments(pc_nestlevel+1,assignments);

  if (enumsym!=NULL && swdefault==FALSE && enumsym->x.tags.unique-enumsymcount<=2) {
    constvalue_root *enumlist=enumsym->dim.enumlist;
    constvalue *cur,*found,*prev=NULL,*save_next=NULL;
    for (cur=enumlist->first; cur!=NULL; prev=cur,cur=cur->next) {
      /* if multiple enum elements share the same value, we only want to count the first one */
      if (prev!=NULL) {
        /* see if there's another constvalue before the current one that has the same value */
        save_next=prev->next;
        prev->next=NULL;
        found=find_constval_byval(enumlist,cur->value);
        prev->next=save_next;
        if (found!=NULL)
          continue;
      } /* if */
      /* check if the value of this constant is handled in switch, if so - continue */
      if (find_constval_byval(&caselist,cur->value)!=NULL)
        continue;
      errorset(sSETPOS,save_fline);
      error(244,cur->name); /* enum element not handled in switch */
      errorset(sSETPOS,-1);
    } /* for */
  } /* if */

  #if !defined NDEBUG
    /* verify that the case table is sorted (unfortunately, duplicates can
     * occur; there really shouldn't be duplicate cases, but the compiler
     * may not crash or drop into an assertion for a user error). */
    for (cse=caselist.first; cse!=NULL && cse->next!=NULL; cse=cse->next)
      assert(cse->value <= cse->next->value);
  #endif
  /* generate the table here, before lbl_exit (general jump target) */
  setlabel(lbl_table);
  assert(swdefault==FALSE || swdefault==TRUE);
  if (swdefault==FALSE) {
    /* store lbl_exit as the "none-matched" label in the switch table */
    strcpy(labelname,itoh(lbl_exit));
  } else {
    /* lbl_case holds the label of the "default" clause */
    strcpy(labelname,itoh(lbl_case));
  } /* if */
  ffcase(casecount,labelname,TRUE);
  /* generate the rest of the table */
  for (cse=caselist.first; cse!=NULL; cse=cse->next)
    ffcase(cse->value,cse->name,FALSE);

  setlabel(lbl_exit);
  delete_consttable(&caselist); /* clear list of case labels */

  return (swdefault && allterminal) ? tTERMSWITCH : tSWITCH;
}

/* docase() is only called in erroneous situations when there's a case
 * outside of switch.
 */
static void docase(int isdefault)
{
  error(14);                        /* invalid statement; not in switch */
  if (!isdefault) {
    /* try to skim through the case values, so they won't be
     * misinterpreted as a separate statement later */
    PUSHSTK_I(sc_allowtags);
    sc_allowtags=FALSE;             /* do not allow tagnames here */
    do {
      /* no need to verify the values, as the error output is blocked
       * for the rest of the statement anyway (by "error(14)" above);
       * simply eat the values by calling constexpr() */
      constexpr(NULL,NULL,NULL);
      if (matchtoken(tDBLDOT))
        constexpr(NULL,NULL,NULL);
    } while (matchtoken(','));
    sc_allowtags=(short)POPSTK_I(); /* reset */
  } /* if */
  needtoken(':');                   /* ':' ends the case */
}

static void doassert(void)
{
  int flab1,index;
  cell cidx;

  if ((sc_debug & sCHKBOUNDS)!=0) {
    flab1=getlabel();           /* get label number for "OK" branch */
    test(flab1,TEST_PLAIN,TRUE);/* get expression and branch to flab1 if true */
    insert_dbgline(fline);      /* make sure we can find the correct line number */
    ffabort(xASSERTION);
    setlabel(flab1);
  } else {
    stgset(TRUE);               /* start staging */
    stgget(&index,&cidx);       /* mark position in code generator */
    do {
      expression(NULL,NULL,NULL,FALSE);
      stgdel(index,cidx);       /* just scrap the code */
    } while (matchtoken(','));
    stgset(FALSE);              /* stop staging */
  } /* if */
  needtoken(tTERM);
}

static int dogoto(void)
{
  char *st;
  cell val;
  symbol *sym;
  int returnst=tGOTO;

  /* if we were inside an endless loop, assume that we jump out of it */
  endlessloop=0;

  if (lex(&val,&st)==tSYMBOL) {
    sym=fetchlab(st);
    if ((sym->usage & uDEFINE)!=0)
      clearassignments(1);
    jumplabel((int)sym->addr);
    sym->usage|=uREAD;  /* set "uREAD" bit */
    if ((sym->usage & uDEFINE)!=0) {
      /* if there are no unimplemented labels, then the subsequent code is unreachable */
      symbol *cur;
      for (cur=&loctab; (cur=cur->next)!=NULL; )
        if (cur->ident==iLABEL && (cur->usage & uDEFINE)==0)
          break;
      if (cur==NULL)
        returnst=tTERMINAL;
    } /* if */
    // ??? if the label is defined (check sym->usage & uDEFINE), check
    //     sym->compound (nesting level of the label) against pc_nestlevel;
    //     if sym->compound < pc_nestlevel, call the destructor operator
  } else {
    error_suggest(20,st,NULL,estSYMBOL,esfLABEL);   /* illegal symbol name */
  } /* if */
  needtoken(tTERM);
  return returnst;
}

static void dolabel(void)
{
  char *st;
  cell val;
  symbol *sym;

  tokeninfo(&val,&st);  /* retrieve label name again */
  if (find_constval(&tagname_tab,st,0)!=NULL)
    error(221,st);      /* label name shadows tagname */
  sym=fetchlab(st);
  if ((sym->usage & uDEFINE)!=0)
    error(21,st);       /* symbol already defined */
  setlabel((int)sym->addr);
  /* since one can jump around variable declarations or out of compound
   * blocks, the stack must be manually adjusted
   */
  setstk(-declared*sizeof(cell));
  sym->usage|=uDEFINE;  /* label is now defined */
}

/*  fetchlab
 *
 *  Finds a label from the (local) symbol table or adds one to it.
 *  Labels are local in scope.
 *
 *  Note: The "_usage" bit is set to zero. The routines that call "fetchlab()"
 *        must set this bit accordingly.
 */
static symbol *fetchlab(char *name)
{
  symbol *sym;

  sym=findloc(name);            /* labels are local in scope */
  if (sym) {
    if (sym->ident!=iLABEL)
      error_suggest(19,sym->name,NULL,estSYMBOL,esfLABEL);  /* not a label: ... */
  } else {
    sym=addsym(name,getlabel(),iLABEL,sLOCAL,0,0);
    assert(sym!=NULL);          /* fatal error 103 must be given on error */
    sym->x.declared=(int)declared;
    sym->compound=pc_nestlevel;
  } /* if */
  return sym;
}

static void emit_invalid_token(int expected_token,int found_token)
{
  char s[2];

  assert(expected_token>=tFIRST);
  if (found_token<tFIRST) {
    sprintf(s,"%c",(char)found_token);
    error(1,sc_tokens[expected_token-tFIRST],s);
  } else {
    error(1,sc_tokens[expected_token-tFIRST],sc_tokens[found_token-tFIRST]);
  } /* if */
}

static regid emit_findreg(char *opname)
{
  const char *regname=strrchr(opname,'.');
  assert(regname!=NULL);
  regname+=1;
  assert(strcmp(regname,"pri")==0 || strcmp(regname,"alt")==0);
  return (strcmp(regname,"pri")==0) ? sPRI : sALT;
}

/* emit_getlval
 *
 * Looks for an lvalue and generates code to get cell address in PRI
 * if the lvalue is an array element (iARRAYCELL or iARRAYCHAR).
 */
static int emit_getlval(int *identptr,emit_outval *p,int *islocal,
                        regid reg,int allow_char,int allow_const,
                        int store_pri,int store_alt,int *ispushed)
{
  int tok,index,ident,close;
  cell cidx,val,length;
  char *str;
  symbol *sym;

  assert(identptr!=NULL);
  assert(p!=NULL);
  assert((!store_pri && !store_alt) || ((store_pri ^ store_alt) && (reg==sALT && ispushed!=NULL)));

  if (staging) {
    assert((emit_flags & efEXPR)!=0);
    stgget(&index,&cidx);
  } /* if */

  tok=lex(&val,&str);
  if (tok!=tSYMBOL) {
invalid_lvalue:
    error(22);          /* must be lvalue */
    return FALSE;
  } /* if */

  sym=findloc(str);
  if (sym==NULL)
    sym=findglb(str,sSTATEVAR);
  if (sym==NULL || (sym->ident!=iFUNCTN && sym->ident!=iREFFUNC && (sym->usage & uDEFINE)==0)) {
    error(17,str);      /* undefined symbol */
    return FALSE;
  } /* if */
  markusage(sym,uREAD | uWRITTEN);
  if (!allow_const && (sym->usage & uCONST)!=0)
    goto invalid_lvalue;

  p->type=eotNUMBER;
  switch (sym->ident)
  {
  case iVARIABLE:
  case iREFERENCE:
    *identptr=sym->ident;
    *islocal=((sym->vclass & sLOCAL)!=0);
    p->value.ucell=*(ucell *)&sym->addr;
    break;
  case iARRAY:
  case iREFARRAY:
    /* get the index */
    if (matchtoken('[')) {
      *identptr=iARRAYCELL;
      close=']';
    } else if (matchtoken('{')) {
      *identptr=iARRAYCHAR;
      close='}';
    } else {
      error(33,sym->name);      /* array must be indexed */
      return FALSE;
    } /* if */
    if (store_alt || store_pri) {
      pushreg(store_pri ? sPRI : sALT);
      *ispushed=TRUE;
    } /* if */
    ident=expression(&val,NULL,NULL,TRUE);
    needtoken(close);

    /* check if the index isn't out of bounds */
    length=sym->dim.array.length;
    if (close=='}')
      length *= (8*sizeof(cell))/sCHARBITS;
    if (ident==iCONSTEXPR) {    /* if the index is a constant value, check it at compile time */
      if (val<0 || (length!=0 && val>=length)) {
        error(32,sym->name);    /* array index out of bounds */
        return FALSE;
      } /* if */
    } else if (length!=0) {     /* otherwise generate code for a run-time boundary check */
      ffbounds(length-1);
    } /* if */

    /* calculate cell address */
    if (ident==iCONSTEXPR) {
      if (staging)
        stgdel(index,cidx);     /* erase generated code */
      if (store_alt || store_pri)
        *ispushed=FALSE;
      p->value.ucell= *(ucell *)&sym->addr;
      if (close==']')
        val *= (cell)sizeof(cell);
      else
        val *= (cell)(sCHARBITS/8);
      if (sym->ident==iARRAY) {
        p->value.ucell += (ucell)val;
        if (close==']') {
          /* If we are accessing an array cell and its address is known at
           * compile time, we can return it as 'iVARIABLE',
           * so the calling function could generate more optimal code.
           */
          *islocal=((sym->vclass & sLOCAL)!=0);
          *identptr=iVARIABLE;
          break;
        } /* if */
        if (reg==sPRI) {
          outinstr(((sym->vclass & sLOCAL)!=0) ? "addr.pri" : "const.pri",p,1);
        } else {
          if (store_alt)
            moveto1();
          outinstr(((sym->vclass & sLOCAL)!=0) ? "addr.alt" : "const.alt",p,1);
        } /* if */
      } else {  /* sym->ident==iREFARRAY */
        if (close==']' && val==0) {
          *identptr=iREFERENCE;
          break;
        } /* if */
        if (reg==sPRI) {
          outinstr("load.s.pri",p,1);
          if (val==1)
            outinstr("inc.pri",NULL,0);
          else
            addconst(val);
        } else {
          if (val==0 || val==1) {
            if (store_alt)
              outinstr("move.pri",NULL,0);
            outinstr("load.s.alt",p,1);
            if (val==1)
              outinstr("inc.alt",NULL,0);
          } else {
            if (store_pri)
              outinstr("move.alt",NULL,0);
            outinstr("load.s.pri",p,1);
            addconst(val);
            if (store_pri || store_alt)
              outinstr("xchg",NULL,0);
            else
              outinstr("move.alt",NULL,0);
          } /* if */
        } /* if */
      } /* if */
      if (close=='}') {
        p->value.ucell=(ucell)(sCHARBITS/8);
        outinstr((reg==sPRI) ? "align.pri" : "align.alt",p,1);
      } /* if */
    } else {    /* ident!=iCONSTEXPR */
      if (close=='}' && sym->ident==iARRAY && (sym->vclass & sLOCAL)==0) {
        char2addr();
        addconst(sym->addr);
        charalign();
        if (reg==sALT)
          outinstr("move.alt",NULL,0);
        break;
      } /* if */
      p->value.ucell= *(ucell *)&sym->addr;
      if (sym->ident==iARRAY)
        outinstr(((sym->vclass & sLOCAL)!=0) ? "addr.alt" : "const.alt",p,1);
      else      /* sym->ident==iREFARRAY */
        outinstr("load.s.alt",p,1);
      if (close==']') {
        outinstr("idxaddr",NULL,0);
      } else {
        char2addr();
        ob_add();
        charalign();
      } /* if */
      if (reg==sALT)
        outinstr("move.alt",NULL,0);
    } /* if */
    break;
  default:
    goto invalid_lvalue;
  } /* switch */

  if (!staging) {   /* issue an error if a pseudo-opcode is used outside of function body */
    error(10);      /* invalid function or declaration */
    return FALSE;
  } /* if */
  if ((sym->ident==iARRAY || sym->ident==iREFARRAY) && close=='}' && !allow_char) {
    /* issue an error if array character access isn't allowed
     * (currently it's only in 'push.u.adr')
     */
    error(35,1);    /* argument type mismatch (argument 1) */
    return FALSE;
  } /* if */
  return TRUE;
}

/* emit_getrval
 *
 * Looks for an rvalue and generates code to handle expressions.
 */
static int emit_getrval(int *identptr,cell *val)
{
  int index,result=TRUE;
  cell cidx;
  symbol *sym;

  assert(identptr!=NULL);
  assert(val!=NULL);

  if (staging) {
    assert((emit_flags & efEXPR)!=0);
    stgget(&index,&cidx);
  } else {
    error(10);          /* invalid function or declaration */
    result=FALSE;
  } /* if */

  *identptr=expression(val,NULL,&sym,TRUE);
  switch (*identptr) {
  case iVARIABLE:
  case iREFERENCE:
    *val=sym->addr;
    break;
  case iCONSTEXPR:
    /* If the expression result is a constant value or a variable - erase the code
     * for this expression so the caller would be able to generate more optimal
     * code for it, without unnecessary register clobbering.
     */
    if (staging)
      stgdel(index,cidx);
    break;
  case iARRAY:
  case iREFARRAY:
    error(33,(sym!=NULL) ? sym->name : "-unknown-");    /* array must be indexed */
    result=FALSE;
    break;
  } /* switch */

  return result;
}

static int emit_param_any_internal(emit_outval *p,int expected_tok,
                                   int allow_nonint,int allow_expr)
{
  char *str;
  cell val,cidx;
  symbol *sym;
  int tok,negate,ident,index;

  negate=FALSE;
  p->type=eotNUMBER;
fetchtok:
  tok=lex(&val,&str);
  switch (tok) {
  case tNUMBER:
    p->value.ucell=(ucell)(negate ? -val : val);
    break;
  case tRATIONAL:
    if (!allow_nonint)
      goto invalid_token;
    p->value.ucell=(negate ? ((ucell)val|((ucell)1 << (PAWN_CELL_SIZE-1))) : (ucell)val);
    break;
  case tSYMBOL:
    sym=findloc(str);
    if (sym==NULL)
      sym=findglb(str,sSTATEVAR);
    if (sym==NULL || (sym->ident!=iFUNCTN && sym->ident!=iREFFUNC && (sym->usage & uDEFINE)==0)) {
      error(17,str);    /* undefined symbol */
      return FALSE;
    } /* if */
    if (sym->ident==iLABEL) {
      sym->usage|=uREAD;
      if (negate) {
        tok=tLABEL;
        goto invalid_token_neg;
      } /* if */
      if (!allow_nonint) {
        tok=tLABEL;
        goto invalid_token;
      } /* if */
      p->type=eotLABEL;
      p->value.ucell=(ucell)sym->addr;
    } else if (sym->ident==iFUNCTN || sym->ident==iREFFUNC) {
      const int ntvref=sym->usage & uREAD;
      markusage(sym,uREAD);
      if (negate) {
        tok=teFUNCTN;
        goto invalid_token_neg;
      } /* if */
      if (!allow_nonint) {
        tok=(sym->usage & uNATIVE) ? teNATIVE : teFUNCTN;
        goto invalid_token;
      } /* if */
      if ((sym->usage & uNATIVE)!=0 && ntvref==0 && sym->addr>=0)
        sym->addr=ntv_funcid++;
      p->type=eotFUNCTION;
      p->value.string=str;
    } else {
      markusage(sym,uREAD | uWRITTEN);
      if (!allow_nonint && sym->ident!=iCONSTEXPR) {
        if (sym->vclass==sLOCAL)
          tok=(sym->ident==iREFERENCE || sym->ident==iREFARRAY) ? teREFERENCE : teLOCAL;
        else
          tok=teDATA;
        goto invalid_token;
      } /* if */
      p->value.ucell=(ucell)(negate ? -sym->addr : sym->addr);
    } /* if */
    break;
  case '(':
    if (!allow_expr)
      goto invalid_token;
    if ((emit_flags & efEXPR)==0)
      stgset(TRUE);
    stgget(&index,&cidx);
    ident=expression(&val,NULL,NULL,FALSE);
    stgdel(index,cidx);
    if ((emit_flags & efEXPR)==0)
      stgset(FALSE);
    needtoken(')');
    p->value.ucell=(ucell)(negate ? -val : val);
    if (ident!=iCONSTEXPR) {
      error(8);         /* must be constant expression */
      return FALSE;
    } /* if */
    break;
  case ':':
    if (negate)
      goto invalid_token_neg;
    tok=lex(&val,&str);
    if (tok!=tSYMBOL) {
      emit_invalid_token(tSYMBOL,tok);
      return FALSE;
    } /* if */
    sym=fetchlab(str);
    sym->usage|=uREAD;
    p->type=eotLABEL;
    p->value.ucell=(ucell)sym->addr;
    break;
  case '-':
    if (!negate) {
      negate=TRUE;
      goto fetchtok;
    } else {
      char ival[sNAMEMAX+2];
    invalid_token_neg:
      if (tok<tFIRST)
        sprintf(ival,"-%c",tok);
      else
        sprintf(ival,"-(%s)",sc_tokens[tok-tFIRST]);
      error(1,sc_tokens[expected_tok-tFIRST],ival);
      return FALSE;
    } /* if */
  default:
  invalid_token:
    emit_invalid_token(expected_tok,tok);
    return FALSE;
  } /* switch */
  return TRUE;
}

static void emit_param_any(emit_outval *p)
{
  emit_param_any_internal(p,teANY,TRUE,TRUE);
}

static void emit_param_integer(emit_outval *p)
{
  emit_param_any_internal(p,tNUMBER,FALSE,TRUE);
}

static void emit_param_index(emit_outval *p,int isrange,
                             const cell *valid_values,int numvalues)
{
  int i;
  cell val;

  assert(isrange ? (numvalues==2) : (numvalues>0));
  if (!emit_param_any_internal(p,tNUMBER,FALSE,FALSE))
    return;
  val=(cell)p->value.ucell;
  if (isrange) {
    if (valid_values[0]<=val && val<=valid_values[1])
      return;
  } else {
    for (i=0; i<numvalues; i++)
      if (val==valid_values[i])
        return;
  } /* if */
  error(241);    /* negative or too big shift count */
}

static void emit_param_nonneg(emit_outval *p)
{
  if (!emit_param_any_internal(p,teNONNEG,FALSE,TRUE))
    return;
  if ((cell)p->value.ucell<(cell)0) {
#if PAWN_CELL_SIZE==16
    char ival[7];
#elif PAWN_CELL_SIZE==32
    char ival[12];
#elif PAWN_CELL_SIZE==64
    char ival[21];
#else
  #error Unsupported cell size
#endif
    sprintf(ival,"%"PRIdC,(cell)p->value.ucell);
    error(1,sc_tokens[teNONNEG-tFIRST],ival);
  } /* if */
}

static void emit_param_shift(emit_outval *p)
{
  if (emit_param_any_internal(p,tNUMBER,FALSE,TRUE))
    if (p->value.ucell>=(sizeof(cell)*8))
      error(50);    /* invalid range */
}

static void emit_param_data(emit_outval *p)
{
  cell val;
  char *str;
  symbol *sym;
  int tok;

  p->type=eotNUMBER;
  tok=lex(&val,&str);
  switch (tok) {
  case tSYMBOL:
    sym=findloc(str);
    if (sym!=NULL) {
      markusage(sym,uREAD | uWRITTEN);
      if (sym->ident==iLABEL) {
        tok=tLABEL;
        goto invalid_token;
      } /* if */
      if (sym->vclass!=sSTATIC) {
        if (sym->ident==iCONSTEXPR)
          tok=teNUMERIC;
        else
          tok=(sym->ident==iREFERENCE || sym->ident==iREFARRAY) ? teREFERENCE : teLOCAL;
        goto invalid_token;
      } /* if */
    } else {
      sym=findglb(str,sSTATEVAR);
      if (sym==NULL) {
        error(17,str);  /* undefined symbol */
        return;
      } /* if */
      markusage(sym,(sym->ident==iFUNCTN || sym->ident==iREFFUNC) ? uREAD : (uREAD | uWRITTEN));
      if (sym->ident==iFUNCTN || sym->ident==iREFFUNC) {
        tok=((sym->usage & uNATIVE)!=0) ? teNATIVE : teFUNCTN;
        goto invalid_token;
      } /* if */
      if (sym->ident==iCONSTEXPR) {
        tok=teNUMERIC;
        goto invalid_token;
      } /* if */
    } /* if */
    val=sym->addr;
    break;
  default:
  invalid_token:
    emit_invalid_token(teDATA,tok);
    return;
  } /* switch */
  if ((val % sizeof(cell))==0)
    p->value.ucell=(ucell)val;
  else
    error(11);  /* must be a multiple of cell size */
}

static void emit_param_local(emit_outval *p,int allow_ref)
{
  cell val;
  char *str;
  symbol *sym;
  int tok,negate;

  negate=FALSE;
  p->type=eotNUMBER;
fetchtok:
  tok=lex(&val,&str);
  switch (tok) {
  case tNUMBER:
    break;
  case tSYMBOL:
    sym=findloc(str);
    if (sym!=NULL) {
      markusage(sym,uREAD | uWRITTEN);
      if (sym->ident==iLABEL) {
        tok=tLABEL;
        goto invalid_token;
      } /* if */
      if (sym->vclass==sSTATIC) {
        tok=teDATA;
        goto invalid_token;
      } /* if */
      if (allow_ref==FALSE && (sym->ident==iREFERENCE || sym->ident==iREFARRAY)) {
        tok=teREFERENCE;
        goto invalid_token;
      } /* if */
      if (negate && sym->ident!=iCONSTEXPR) {
        tok=(sym->ident==iREFERENCE || sym->ident==iREFARRAY) ? teREFERENCE : teLOCAL;
        goto invalid_token_neg;
      } /* if */
    } else {
      sym=findglb(str,sSTATEVAR);
      if (sym==NULL) {
        error(17,str);  /* undefined symbol */
        return;
      } /* if */
      markusage(sym,(sym->ident==iFUNCTN || sym->ident==iREFFUNC) ? uREAD : (uREAD | uWRITTEN));
      if (sym->ident!=iCONSTEXPR) {
        if (sym->ident==iFUNCTN || sym->ident==iREFFUNC)
          tok=((sym->usage & uNATIVE)!=0) ? teNATIVE : teFUNCTN;
        else
          tok=teDATA;
        goto invalid_token;
      } /* if */
    } /* if */
    val=sym->addr;
    break;
  case '-':
    if (!negate) {
      negate=TRUE;
      goto fetchtok;
    } else {
      char ival[sNAMEMAX+2];
    invalid_token_neg:
      if (tok<tFIRST)
        sprintf(ival,"-%c",tok);
      else
        sprintf(ival,"-(%s)",sc_tokens[tok-tFIRST]);
      error(1,sc_tokens[teLOCAL-tFIRST],ival);
      return;
    }
  default:
  invalid_token:
    if (negate)
      goto invalid_token_neg;
    emit_invalid_token(teLOCAL,tok);
    return;
  } /* switch */
  if ((val % sizeof(cell))!=0) {
    error(11);  /* must be a multiple of cell size */
    return;
  }
  p->value.ucell=(ucell)(negate ? -val : val);
}

static void emit_param_label(emit_outval *p)
{
  cell val;
  char *str;
  symbol *sym;
  int tok;

  p->type=eotNUMBER;
  tok=lex(&val,&str);
  switch (tok)
  {
  case ':':
    tok=lex(&val,&str);
    if (tok!=tSYMBOL)
      goto invalid_token;
    /* fallthrough */
  case tSYMBOL:
    sym=findloc(str);
    if (sym==NULL)
      sym=findglb(str,sSTATEVAR);
    if (sym!=NULL) {
      markusage(sym,(sym->ident==iFUNCTN || sym->ident==iREFFUNC) ? uREAD : (uREAD | uWRITTEN));
      if (sym->ident!=iLABEL) {
        if (sym->ident==iFUNCTN || sym->ident==iREFFUNC) {
          tok=((sym->usage & uNATIVE)!=0) ? teNATIVE : teFUNCTN;
        } else if (sym->ident==iCONSTEXPR) {
          tok=teNUMERIC;
        } else {
          if (sym->vclass==sLOCAL)
            tok=(sym->ident==iREFERENCE || sym->ident==iREFARRAY) ? teREFERENCE : teLOCAL;
          else
            tok=teDATA;
        } /* if */
        goto invalid_token;
      } /* if */
    } else {
      sym=fetchlab(str);
    } /* if */
    sym->usage|=uREAD;
    p->value.ucell=(ucell)sym->addr;
    break;
  default:
  invalid_token:
    emit_invalid_token(tLABEL,tok);
  }
}

static void emit_param_function(emit_outval *p,int isnative)
{
  cell val;
  char *str;
  symbol *sym;
  int tok,ntvref;

  p->type=eotNUMBER;
  tok=lex(&val,&str);
  switch (tok)
  {
  case tSYMBOL:
    sym=findloc(str);
    if (sym==NULL)
      sym=findglb(str,sSTATEVAR);
    if (sym==NULL) {
      error(17,str);    /* undefined symbol */
      return;
    } /* if */
    if (sym->ident==iFUNCTN || sym->ident==iREFFUNC) {
      ntvref=sym->usage & uREAD;
      markusage(sym,uREAD);
      if (!!(sym->usage & uNATIVE)==isnative)
        break;
      tok=(isnative!=FALSE) ? teFUNCTN : teNATIVE;
    } else {
      markusage(sym,uREAD | uWRITTEN);
      if (sym->ident==iLABEL) {
        tok=tLABEL;
      } else if (sym->ident==iCONSTEXPR) {
        tok=teNUMERIC;
      } else {
        if (sym->vclass==sLOCAL)
          tok=(sym->ident==iREFERENCE || sym->ident==iREFARRAY) ? teREFERENCE : teLOCAL;
        else
          tok=teDATA;
      } /* if */
    } /* if */
    /* fallthrough */
  default:
    emit_invalid_token((isnative!=FALSE) ? teNATIVE : teFUNCTN,tok);
    return;
  } /* switch */

  if (isnative!=FALSE) {
    if (ntvref==0 && sym->addr>=0)
      sym->addr=ntv_funcid++;
    p->value.ucell=(ucell)sym->addr;
  } else {
    p->type=eotFUNCTION;
    p->value.string=str;
  } /* if */
}

static void emit_noop(char *name)
{
  (void)name;
}

static void emit_parm0(char *name)
{
  outinstr(name,NULL,0);
}

static void emit_parm1_any(char *name)
{
  emit_outval p[1];

  emit_param_any(&p[0]);
  outinstr(name,p,arraysize(p));
}

static void emit_parm1_integer(char *name)
{
  emit_outval p[1];

  emit_param_integer(&p[0]);
  outinstr(name,p,arraysize(p));
}

static void emit_parm1_nonneg(char *name)
{
  emit_outval p[1];

  emit_param_nonneg(&p[0]);
  outinstr(name,p,arraysize(p));
}

static void emit_parm1_shift(char *name)
{
  emit_outval p[1];

  emit_param_shift(&p[0]);
  outinstr(name,p,arraysize(p));
}

static void emit_parm1_data(char *name)
{
  emit_outval p[1];

  emit_param_data(&p[0]);
  outinstr(name,p,arraysize(p));
}

static void emit_parm1_local(char *name)
{
  emit_outval p[1];

  emit_param_local(&p[0],TRUE);
  outinstr(name,p,arraysize(p));
}

static void emit_parm1_local_noref(char *name)
{
  emit_outval p[1];

  emit_param_local(&p[0],FALSE);
  outinstr(name,p,arraysize(p));
}

static void emit_parm1_label(char *name)
{
  emit_outval p[1];

  emit_param_label(&p[0]);
  outinstr(name,p,arraysize(p));
}

static void emit_do_casetbl(char *name)
{
  emit_outval p[2];

  (void)name;
  emit_param_nonneg(&p[0]);
  emit_param_label(&p[1]);
  stgwrite("\tcasetbl\n");
  outinstr("case",p,arraysize(p));
}

static void emit_do_case(char *name)
{
  emit_outval p[2];

  emit_param_any(&p[0]);
  emit_param_label(&p[1]);
  outinstr("case",p,arraysize(p));
  code_idx-=opcodes(1);
}

static void emit_do_lodb_strb(char *name)
{
  static const cell valid_values[] = { 1,2,4 };
  emit_outval p[1];

  emit_param_index(&p[0],FALSE,valid_values,arraysize(valid_values));
  outinstr(name,p,arraysize(p));
}

static void emit_do_align(char *name)
{
  static const cell valid_values[] = { 0,sizeof(cell)-1 };
  emit_outval p[1];

  emit_param_index(&p[0],TRUE,valid_values,arraysize(valid_values));
  outinstr(name,p,arraysize(p));
}

static void emit_do_call(char *name)
{
  emit_outval p[1];

  emit_param_function(&p[0],FALSE);
  outinstr(name,p,arraysize(p));
}

static void emit_do_sysreq_c(char *name)
{
  emit_outval p[1];

  emit_param_function(&p[0],TRUE);

  /* if macro optimisations aren't enabled, output a 'sysreq.c' instruction,
   * otherwise generate the following sequence:
   *   const.pri <funcid>
   *   sysreq.pri
   */
  if (pc_optimize<=sOPTIMIZE_NOMACRO) {
    outinstr(name,p,1);
  } else {
    outinstr("const.pri",&p[0],1);
    outinstr("sysreq.pri",NULL,0);
  } /* if */
}

static void emit_do_sysreq_n(char *name)
{
  emit_outval p[2];

  emit_param_function(&p[0],TRUE);
  emit_param_any(&p[1]);

  /* if macro optimisations are enabled, output a 'sysreq.n' instruction,
   * otherwise generate the following sequence:
   *   push <argsize>
   *   sysreq.c <funcid>
   *   stack <argsize> + <cellsize>
   */
  if (pc_optimize>sOPTIMIZE_NOMACRO) {
    outinstr(name,p,2);
  } else {
    outinstr("push.c",&p[1],1);
    outinstr("sysreq.c",&p[0],1);
    p[1].value.ucell+=sizeof(cell);
    outinstr("stack",&p[1],1);
  } /* if */
}

static void emit_do_const(char *name)
{
  emit_outval p[2];

  emit_param_data(&p[0]);
  emit_param_any(&p[1]);

  /* if macro optimisations are enabled, output a 'const' instruction,
   * otherwise generate the following sequence:
   *   push.pri
   *   const.pri <val>
   *   stor.pri <addr>
   *   pop.pri
   */
  if (pc_optimize>sOPTIMIZE_NOMACRO) {
    outinstr(name,p,2);
  } else {
    outinstr("push.pri",NULL,0);
    outinstr("const.pri",&p[1],1);
    outinstr("stor.pri",&p[0],1);
    outinstr("pop.pri",NULL,0);
  } /* if */
}

static void emit_do_const_s(char *name)
{
  emit_outval p[2];

  emit_param_local(&p[0],FALSE);
  emit_param_any(&p[1]);

  /* if macro optimisations are enabled, output a 'const.s' instruction,
   * otherwise generate the following sequence:
   *   push.pri
   *   const.pri <val>
   *   stor.s.pri <addr>
   *   pop.pri
   */
  if (pc_optimize>sOPTIMIZE_NOMACRO) {
    outinstr(name,p,2);
  } else {
    outinstr("push.pri",NULL,0);
    outinstr("const.pri",&p[1],1);
    outinstr("stor.s.pri",&p[0],1);
    outinstr("pop.pri",NULL,0);
  } /* if */
}

static void emit_do_load_both(char *name)
{
  emit_outval p[2];

  emit_param_data(&p[0]);
  emit_param_data(&p[1]);

  /* if macro optimisations are enabled, output a 'load.both' instruction,
   * otherwise generate the following sequence:
   *   load.pri <val1>
   *   load.alt <val2>
   */
  if (pc_optimize>sOPTIMIZE_NOMACRO) {
    outinstr(name,p,2);
  } else {
    outinstr("load.pri",&p[0],1);
    outinstr("load.alt",&p[1],1);
  } /* if */
}

static void emit_do_load_s_both(char *name)
{
  emit_outval p[2];

  emit_param_local(&p[0],TRUE);
  emit_param_local(&p[1],TRUE);

  /* if macro optimisations are enabled, output a 'load.s.both' instruction,
   * otherwise generate the following sequence:
   *   load.s.pri <val1>
   *   load.s.alt <val2>
   */
  if (pc_optimize>sOPTIMIZE_NOMACRO) {
    outinstr(name,p,2);
  } else {
    outinstr("load.s.pri",&p[0],1);
    outinstr("load.s.alt",&p[1],1);
  } /* if */
}

static void emit_do_pushn_c(char *name)
{
  emit_outval p[5];
  int i,numargs;

  assert(name[0]=='p' && name[1]=='u' && name[2]=='s'
         && name[3]=='h' && '2'<=name[4] && name[4]<='5');
  numargs=name[4]-'0';
  for (i=0; i<numargs; i++)
    emit_param_any(&p[i]);

  /* if macro optimisations are enabled, output a 'push<N>.c' instruction,
   * otherwise generate a sequence of <N> 'push.c' instructions
   */
  if (pc_optimize>sOPTIMIZE_NOMACRO) {
    outinstr(name,p,numargs);
  } else {
    for (i=0; i<numargs; i++)
      outinstr("push.c",&p[i],1);
  } /* if */
}

static void emit_do_pushn(char *name)
{
  emit_outval p[5];
  int i,numargs;

  assert(name[0]=='p' && name[1]=='u' && name[2]=='s'
         && name[3]=='h' && '2'<=name[4] && name[4]<='5');
  numargs=name[4]-'0';
  for (i=0; i<numargs; i++)
    emit_param_data(&p[i]);

  /* if macro optimisations are enabled, output a 'push<N>' instruction,
   * otherwise generate a sequence of <N> 'push' instructions
   */
  if (pc_optimize>sOPTIMIZE_NOMACRO) {
    outinstr(name,p,numargs);
  } else {
    for (i=0; i<numargs; i++)
      outinstr("push",&p[i],1);
  } /* if */
}

static void emit_do_pushn_s_adr(char *name)
{
  emit_outval p[5];
  int i,numargs;

  assert(name[0]=='p' && name[1]=='u' && name[2]=='s'
         && name[3]=='h' && '2'<=name[4] && name[4]<='5' && name[5]=='.');
  numargs=name[4]-'0';
  for (i=0; i<numargs; i++)
    emit_param_local(&p[i],TRUE);

  /* if macro optimisations are enabled, output a 'push<N>.s/.adr' instruction,
   * otherwise generate a sequence of <N> 'push.s/.adr' instructions
   */
  if (pc_optimize>sOPTIMIZE_NOMACRO) {
    outinstr(name,p,numargs);
  } else {
    name=(name[6]=='s') ? "push.s" : "push.adr";
    for (i=0; i<numargs; i++)
      outinstr(name,&p[i],1);
  } /* if */
}

static void emit_do_load_u_pri_alt(char *name)
{
  cell val;
  regid reg;
  int ident;

  if (!emit_getrval(&ident,&val))
    return;
  reg=emit_findreg(name);
  if (ident==iCONSTEXPR)
    ldconst(val,reg);
  else if (reg==sALT)
    outinstr("move.alt",NULL,0);
}

static void emit_do_stor_u_pri_alt(char *name)
{
  emit_outval p[1];
  regid reg;
  int ident,islocal,ispushed;

  reg=emit_findreg(name);
  if (!emit_getlval(&ident,&p[0],&islocal,sALT,TRUE,FALSE,(reg==sPRI),(reg==sALT),&ispushed))
    return;
  switch (ident) {
  case iVARIABLE:
    if (islocal)
      outinstr((reg==sPRI) ? "stor.s.pri" : "stor.s.alt",p,1);
    else
      outinstr((reg==sPRI) ? "stor.pri" : "stor.alt",p,1);
    break;
  case iREFERENCE:
    outinstr((reg==sPRI) ? "sref.s.pri" : "sref.s.alt",p,1);
    break;
  case iARRAYCELL:
  case iARRAYCHAR:
    if (ispushed)
      popreg(sPRI);
    if (ident==iARRAYCELL) {
      outinstr("stor.i",NULL,0);
    } else {
      p[0].value.ucell=sCHARBITS/8;
      outinstr("strb.i",p,1);
    } /* if */
    break;
  default:
    assert(0);
    break;
  } /* switch */
}

static void emit_do_addr_u_pri_alt(char *name)
{
  emit_outval p[1];
  regid reg;
  int ident,islocal;

  reg=emit_findreg(name);
  if (!emit_getlval(&ident,&p[0],&islocal,reg,TRUE,TRUE,FALSE,FALSE,NULL))
    return;
  switch (ident) {
  case iVARIABLE:
    if (islocal)
      outinstr((reg==sPRI) ? "addr.pri" : "addr.alt",p,1);
    else if (p[0].value.ucell==(ucell)0)
      outinstr((reg==sPRI) ? "zero.pri" : "zero.alt",NULL,0);
    else
      outinstr((reg==sPRI) ? "const.pri" : "const.alt",p,1);
    break;
  case iREFERENCE:
    outinstr((reg==sPRI) ? "load.s.pri" : "load.s.alt",p,1);
    break;
  case iARRAYCELL:
  case iARRAYCHAR:
    break;
  default:
    assert(0);
    break;
  } /* switch */
}

static void emit_do_push_u(char *name)
{
  cell val;
  int ident;

  if (!emit_getrval(&ident,&val))
    return;
  if (ident==iCONSTEXPR)
    pushval(val);
  else
    outinstr("push.pri",NULL,0);
}

static void emit_do_push_u_adr(char *name)
{
  emit_outval p[1];
  int ident,islocal;

  if (!emit_getlval(&ident,&p[0],&islocal,sPRI,FALSE,TRUE,FALSE,FALSE,NULL))
    return;
  switch (ident) {
  case iVARIABLE:
    outinstr(islocal ? "push.adr" : "push.c",p,1);
    break;
  case iREFERENCE:
    outinstr("push.s",p,1);
    break;
  case iARRAYCELL:
  case iARRAYCHAR:
    pushreg(sPRI);
    break;
  default:
    assert(0);
    break;
  } /* switch */
}

static void emit_do_zero_u(char *name)
{
  emit_outval p[1];
  int ident,islocal;

  if (!emit_getlval(&ident,&p[0],&islocal,sALT,TRUE,FALSE,FALSE,FALSE,NULL))
    return;
  switch (ident) {
  case iVARIABLE:
    outinstr(islocal ? "zero.s" : "zero",p,1);
    break;
  case iREFERENCE:
    outinstr("zero.pri",NULL,0);
    outinstr("sref.s.pri",&p[0],1);
    break;
  case iARRAYCELL:
    outinstr("zero.pri",NULL,0);
    outinstr("stor.i",NULL,0);
    break;
  case iARRAYCHAR:
    outinstr("zero.pri",NULL,0);
    p[0].value.ucell=(ucell)(sCHARBITS/8);
    outinstr("strb.i",p,1);
    break;
  default:
    assert(0);
    break;
  } /* switch */
}

static void emit_do_inc_dec_u(char *name)
{
  emit_outval p[1];
  int ident,islocal;

  assert(strcmp(name,"inc.u")==0 || strcmp(name,"dec.u")==0);

  if (!emit_getlval(&ident,&p[0],&islocal,sPRI,TRUE,FALSE,FALSE,FALSE,NULL))
    return;
  switch (ident) {
  case iVARIABLE:
    if (islocal)
      outinstr((name[0]=='i') ? "inc.s" : "dec.s",p,1);
    else
      outinstr((name[0]=='i') ? "inc" : "dec",p,1);
    break;
  case iREFERENCE:
    outinstr("load.s.pri",&p[0],1);
    /* fallthrough */
  case iARRAYCELL:
    outinstr((name[0]=='i') ? "inc.i" : "dec.i",NULL,0);
    break;
  case iARRAYCHAR:
    p[0].value.ucell=(ucell)(sCHARBITS/8);
    outinstr("move.alt",NULL,0);
    outinstr("lodb.i",p,1);
    outinstr((name[0]=='i') ? "inc.pri" : "dec.pri",NULL,0);
    outinstr("strb.i",p,1);
    break;
  default:
    assert(0);
    break;
  } /* switch */
}

static EMIT_OPCODE emit_opcodelist[] = {
  { NULL,         emit_noop },
  { "add",        emit_parm0 },
  { "add.c",      emit_parm1_any },
  { "addr.alt",   emit_parm1_local },
  { "addr.pri",   emit_parm1_local },
  { "addr.u.alt", emit_do_addr_u_pri_alt },
  { "addr.u.pri", emit_do_addr_u_pri_alt },
  { "align.alt",  emit_do_align },
  { "align.pri",  emit_do_align },
  { "and",        emit_parm0 },
  { "bounds",     emit_parm1_integer },
  { "break",      emit_parm0 },
  { "call",       emit_do_call },
  { "call.pri",   emit_parm0 },
  { "case",       emit_do_case },
  { "casetbl",    emit_do_casetbl },
  { "cmps",       emit_parm1_nonneg },
  { "const",      emit_do_const },
  { "const.alt",  emit_parm1_any },
  { "const.pri",  emit_parm1_any },
  { "const.s",    emit_do_const_s },
  { "dec",        emit_parm1_data },
  { "dec.alt",    emit_parm0 },
  { "dec.i",      emit_parm0 },
  { "dec.pri",    emit_parm0 },
  { "dec.s",      emit_parm1_local_noref },
  { "dec.u",      emit_do_inc_dec_u },
  { "eq",         emit_parm0 },
  { "eq.c.alt",   emit_parm1_any },
  { "eq.c.pri",   emit_parm1_any },
  { "fill",       emit_parm1_nonneg },
  { "geq",        emit_parm0 },
  { "grtr",       emit_parm0 },
  { "halt",       emit_parm1_nonneg },
  { "heap",       emit_parm1_integer },
  { "idxaddr",    emit_parm0 },
  { "idxaddr.b",  emit_parm1_shift },
  { "inc",        emit_parm1_data },
  { "inc.alt",    emit_parm0 },
  { "inc.i",      emit_parm0 },
  { "inc.pri",    emit_parm0 },
  { "inc.s",      emit_parm1_local_noref },
  { "inc.u",      emit_do_inc_dec_u },
  { "invert",     emit_parm0 },
  { "jeq",        emit_parm1_label },
  { "jgeq",       emit_parm1_label },
  { "jgrtr",      emit_parm1_label },
  { "jleq",       emit_parm1_label },
  { "jless",      emit_parm1_label },
  { "jneq",       emit_parm1_label },
  { "jnz",        emit_parm1_label },
  { "jrel",       emit_parm1_integer },
  { "jsgeq",      emit_parm1_label },
  { "jsgrtr",     emit_parm1_label },
  { "jsleq",      emit_parm1_label },
  { "jsless",     emit_parm1_label },
  { "jump",       emit_parm1_label },
  { "jump.pri",   emit_parm0 },
  { "jzer",       emit_parm1_label },
  { "lctrl",      emit_parm1_integer },
  { "leq",        emit_parm0 },
  { "less",       emit_parm0 },
  { "lidx",       emit_parm0 },
  { "lidx.b",     emit_parm1_shift },
  { "load.alt",   emit_parm1_data },
  { "load.both",  emit_do_load_both },
  { "load.i",     emit_parm0 },
  { "load.pri",   emit_parm1_data },
  { "load.s.alt", emit_parm1_local },
  { "load.s.both",emit_do_load_s_both },
  { "load.s.pri", emit_parm1_local },
  { "load.u.alt", emit_do_load_u_pri_alt },
  { "load.u.pri", emit_do_load_u_pri_alt },
  { "lodb.i",     emit_do_lodb_strb },
  { "lref.alt",   emit_parm1_data },
  { "lref.pri",   emit_parm1_data },
  { "lref.s.alt", emit_parm1_local },
  { "lref.s.pri", emit_parm1_local },
  { "move.alt",   emit_parm0 },
  { "move.pri",   emit_parm0 },
  { "movs",       emit_parm1_nonneg },
  { "neg",        emit_parm0 },
  { "neq",        emit_parm0 },
  { "nop",        emit_parm0 },
  { "not",        emit_parm0 },
  { "or",         emit_parm0 },
  { "pop.alt",    emit_parm0 },
  { "pop.pri",    emit_parm0 },
  { "proc",       emit_parm0 },
  { "push",       emit_parm1_data },
  { "push.adr",   emit_parm1_local },
  { "push.alt",   emit_parm0 },
  { "push.c",     emit_parm1_any },
  { "push.pri",   emit_parm0 },
  { "push.r",     emit_parm1_integer },
  { "push.s",     emit_parm1_local },
  { "push.u",     emit_do_push_u },
  { "push.u.adr", emit_do_push_u_adr },
  { "push2",      emit_do_pushn },
  { "push2.adr",  emit_do_pushn_s_adr },
  { "push2.c",    emit_do_pushn_c },
  { "push2.s",    emit_do_pushn_s_adr },
  { "push3",      emit_do_pushn },
  { "push3.adr",  emit_do_pushn_s_adr },
  { "push3.c",    emit_do_pushn_c },
  { "push3.s",    emit_do_pushn_s_adr },
  { "push4",      emit_do_pushn },
  { "push4.adr",  emit_do_pushn_s_adr },
  { "push4.c",    emit_do_pushn_c },
  { "push4.s",    emit_do_pushn_s_adr },
  { "push5",      emit_do_pushn },
  { "push5.adr",  emit_do_pushn_s_adr },
  { "push5.c",    emit_do_pushn_c },
  { "push5.s",    emit_do_pushn_s_adr },
  { "ret",        emit_parm0 },
  { "retn",       emit_parm0 },
  { "sctrl",      emit_parm1_integer },
  { "sdiv",       emit_parm0 },
  { "sdiv.alt",   emit_parm0 },
  { "sgeq",       emit_parm0 },
  { "sgrtr",      emit_parm0 },
  { "shl",        emit_parm0 },
  { "shl.c.alt",  emit_parm1_shift },
  { "shl.c.pri",  emit_parm1_shift },
  { "shr",        emit_parm0 },
  { "shr.c.alt",  emit_parm1_shift },
  { "shr.c.pri",  emit_parm1_shift },
  { "sign.alt",   emit_parm0 },
  { "sign.pri",   emit_parm0 },
  { "sleq",       emit_parm0 },
  { "sless",      emit_parm0 },
  { "smul",       emit_parm0 },
  { "smul.c",     emit_parm1_integer },
  { "sref.alt",   emit_parm1_data },
  { "sref.pri",   emit_parm1_data },
  { "sref.s.alt", emit_parm1_local },
  { "sref.s.pri", emit_parm1_local },
  { "sshr",       emit_parm0 },
  { "stack",      emit_parm1_integer },
  { "stor.alt",   emit_parm1_data },
  { "stor.i",     emit_parm0 },
  { "stor.pri",   emit_parm1_data },
  { "stor.s.alt", emit_parm1_local_noref },
  { "stor.s.pri", emit_parm1_local_noref },
  { "stor.u.alt", emit_do_stor_u_pri_alt },
  { "stor.u.pri", emit_do_stor_u_pri_alt },
  { "strb.i",     emit_do_lodb_strb },
  { "sub",        emit_parm0 },
  { "sub.alt",    emit_parm0 },
  { "swap.alt",   emit_parm0 },
  { "swap.pri",   emit_parm0 },
  { "switch",     emit_parm1_label },
  { "sysreq.c",   emit_do_sysreq_c },
  { "sysreq.n",   emit_do_sysreq_n },
  { "sysreq.pri", emit_parm0 },
  { "udiv",       emit_parm0 },
  { "udiv.alt",   emit_parm0 },
  { "umul",       emit_parm0 },
  { "xchg",       emit_parm0 },
  { "xor",        emit_parm0 },
  { "zero",       emit_parm1_data },
  { "zero.alt",   emit_parm0 },
  { "zero.pri",   emit_parm0 },
  { "zero.s",     emit_parm1_local_noref },
  { "zero.u",     emit_do_zero_u },
};

static int emit_findopcode(const char *instr)
{
  int low,high,mid,cmp;

  /* look up the instruction with a binary search */
  low=1;                /* entry 0 is reserved (for "not found") */
  high=arraysize(emit_opcodelist)-1;
  while (low<high) {
    mid=(low+high)/2;
    cmp=strcmp(instr,emit_opcodelist[mid].name);
    if (cmp>0)
      low=mid+1;
    else
      high=mid;
  } /* while */

  assert(low==high);
  if (strcmp(instr,emit_opcodelist[low].name)==0)
    return low;         /* found */
  return 0;             /* not found, return special index */
}

SC_FUNC void emit_parse_line(void)
{
  cell val;
  char* st;
  int tok,len,i;
  symbol *sym;
  char name[MAX_INSTR_LEN];

  #if !defined NDEBUG
    /* verify that the opcode list is sorted (skip entry 1; it is reserved
     * for a non-existent opcode)
     */
    { /* local */
      static int sorted=FALSE;
      if (!sorted) {
        assert(emit_opcodelist[1].name!=NULL);
        for (i=2; i<arraysize(emit_opcodelist); i++) {
          assert(emit_opcodelist[i].name!=NULL);
          assert(stricmp(emit_opcodelist[i].name,emit_opcodelist[i-1].name)>0);
        } /* for */
        sorted=TRUE;
      } /* if */
    } /* local */
  #endif

  tok=tokeninfo(&val,&st);
  if (tok==tSYMBOL || (tok>tMIDDLE && tok<=tLAST)) {
    /* get the token length */
    if (tok>tMIDDLE && tok<=tLAST)
      len=strlen(sc_tokens[tok-tFIRST]);
    else
      len=strlen(st);

    /* move back to the start of the last fetched token
     * and copy the instruction name
     */
    lptr-=len;
    for (i=0; i<arraysize(name)-1 && (isalnum(*lptr) || *lptr=='.'); ++i,++lptr)
      name[i]=(char)tolower(*lptr);
    name[i]='\0';

    /* find the corresponding argument handler and call it */
    i=emit_findopcode(name);
    if (emit_opcodelist[i].name==NULL && name[0]!='\0')
      error(104,name); /* invalid assembler instruction */
    emit_opcodelist[i].func(name);
  } else if (tok==tLABEL) {
    if ((emit_flags & (efEXPR | efGLOBAL))!=0) {
      error(29);        /* invalid expression, assumed zero */
    } else if (find_constval(&tagname_tab,st,0)!=NULL) {
      error(221,st);    /* label name shadows tagname */
    } else {
      sym=fetchlab(st);
      if ((sym->usage & uDEFINE)!=0)
        error(21,st);   /* symbol already defined */
      setlabel((int)sym->addr);
      sym->usage|=uDEFINE;
    } /* if */
  } /* if */

  if ((emit_flags & (efEXPR | efGLOBAL))==0) {
    assert((emit_flags & efBLOCK)!=0);
    /* make sure the string only contains whitespaces
     * and an optional trailing '}'
     */
    while (*lptr<=' ' && *lptr!='\0')
      lptr++;
    if (*lptr!='\0' && *lptr!='}')
      error(38);  /* extra characters on line */
  } /* if */
}

/* isvariadic
 *
 * Checks if the function is variadic.
 */
static int isvariadic(symbol *sym)
{
  int i;
  for (i=0; curfunc->dim.arglist[i].ident!=0; i++) {
    /* check whether this is a variadic function */
    if (curfunc->dim.arglist[i].ident==iVARARGS) {
      return TRUE;
    } /* if */
  } /* for */
  return FALSE;
}

/* isterminal
 *
 * Checks if the token represents one of the terminal kinds of statements.
 */
static int isterminal(int tok)
{
  return (tok==tRETURN || tok==tBREAK || tok==tCONTINUE || tok==tENDLESS
          || tok==tEXIT || tok==tTERMINAL || tok==tTERMSWITCH);
}

/*  doreturn
 *
 *  Global references: rettype  (altered)
 */
static void doreturn(void)
{
  int tag,ident;
  int level;
  symbol *sym,*sub;

  if (!matchtoken(tTERM)) {
    /* "return <value>" */
    if ((rettype & uRETNONE)!=0)
      error(78);                        /* mix "return;" and "return value;" */
    assert(pc_retexpr==FALSE);
    pc_retexpr=TRUE;
    pc_retheap=0;
    ident=doexpr(TRUE,FALSE,TRUE,FALSE,&tag,&sym,TRUE,NULL);
    pc_retexpr=FALSE;
    needtoken(tTERM);
    /* only warn about unreachable code if the return value is not constant */
    if (ident!=iCONSTEXPR && lastst==tTERMSWITCH)
      error(225); /* unreachable code */
    /* see if this function already has a sub type (an array attached) */
    assert(curfunc!=NULL);
    sub=curfunc->child;
    assert(sub==NULL || sub->ident==iREFARRAY);
    if ((rettype & uRETVALUE)!=0) {
      int retarray=(ident==iARRAY || ident==iREFARRAY);
      /* there was an earlier "return" statement in this function */
      if ((sub==NULL && retarray && sym!=NULL) || (sub!=NULL && !retarray))
        error(79);                      /* mixing "return array;" and "return value;" */
      if (retarray && (curfunc->usage & uPUBLIC)!=0)
        error(90,curfunc->name);        /* public function may not return array */
    } /* if */
    rettype|=uRETVALUE;                 /* function returns a value */
    if (ident==iARRAY || ident==iREFARRAY) {
      int dim[sDIMEN_MAX],numdim=0;
      cell arraysize;
      if (sym==NULL) {
        /* array literals cannot be returned directly */
        error(29); /* invalid expression, assumed zero */
      } else {
        if (sub!=NULL) {
          assert(sub->ident==iREFARRAY);
          /* this function has an array attached already; check that the current
           * "return" statement returns exactly the same array
           */
          level=sym->dim.array.level;
          if (sub->dim.array.level!=level) {
            error(48);                    /* array dimensions must match */
          } else {
            for (numdim=0; numdim<=level; numdim++) {
              dim[numdim]=(int)sub->dim.array.length;
              if (sym->dim.array.length!=dim[numdim])
                error(47);    /* array sizes must match */
              if (numdim<level) {
                sym=sym->child;
                sub=sub->child;
                assert(sym!=NULL && sub!=NULL);
                /* ^^^ both arrays have the same dimensions (this was checked
                 *     earlier) so the dependent should always be found
                 */
              } /* if */
            } /* for */
          } /* if */
        } else {
          int idxtag[sDIMEN_MAX];
          int argcount;
          /* this function does not yet have an array attached; clone the
           * returned symbol beneath the current function
           */
          sub=sym;
          assert(sub!=NULL);
          level=sub->dim.array.level;
          for (numdim=0; numdim<=level; numdim++) {
            dim[numdim]=(int)sub->dim.array.length;
            idxtag[numdim]=sub->x.tags.index;
            if (numdim<level) {
              sub=sub->child;
              assert(sub!=NULL);
            } /* if */
            /* check that all dimensions are known */
            if (dim[numdim]<=0)
              error(46,sym->name);
          } /* for */
          /* the address of the array is stored in a hidden parameter; the address
           * of this parameter is 1 + the number of parameters (times the size of
           * a cell) + the size of the stack frame and the return address
           *   base + 0*sizeof(cell)         == previous "base"
           *   base + 1*sizeof(cell)         == function return address
           *   base + 2*sizeof(cell)         == number of arguments
           *   base + 3*sizeof(cell)         == first argument of the function
           *   ...
           *   base + ((n-1)+3)*sizeof(cell) == last argument of the function
           *   base + (n+3)*sizeof(cell)     == hidden parameter with array address
           */
          assert(curfunc!=NULL);
          assert(curfunc->dim.arglist!=NULL);
          for (argcount=0; curfunc->dim.arglist[argcount].ident!=0; argcount++)
            /* nothing */;
          sub=addvariable(curfunc->name,(argcount+3)*sizeof(cell),iREFARRAY,sGLOBAL,
                          curfunc->tag,dim,numdim,idxtag,0);
          sub->parent=curfunc;
          curfunc->child=sub;
        } /* if */
        /* get the hidden parameter, copy the array (the array is on the heap;
         * it stays on the heap for the moment, and it is removed -usually- at
         * the end of the expression/statement, see expression() in SC3.C)
         */
        if (isvariadic(sub)) {
          pushreg(sPRI);                  /* save source address stored in PRI */
          sub->addr=2*sizeof(cell);
          address(sub,sALT);              /* get the number of arguments */
          getfrm();
          addconst(3*sizeof(cell));
          ob_add();
          dereference();
          swap1();
          popreg(sALT);                   /* ALT = destination */
        } else {
          address(sub,sALT);              /* ALT = destination */
        } /* if */
        arraysize=calc_arraysize(dim,numdim,0);
        memcopy(arraysize*sizeof(cell));  /* source already in PRI */
        /* moveto1(); is not necessary, callfunction() does a popreg() */
      } /* if */
    } /* if */
    modheap(pc_retheap);
    /* try to use "operator=" if tags don't match */
    if (!matchtag(curfunc->tag,tag,TRUE))
      check_userop(NULL,tag,curfunc->tag,2,NULL,&tag);
    /* check tagname with function tagname */
    check_tagmismatch(curfunc->tag,tag,TRUE,-1);
  } else {
    /* this return statement contains no expression */
    ldconst(0,sPRI);
    if ((rettype & uRETVALUE)!=0 && (curfunc->flags & flagNAKED)==0) {
      char symname[2*sNAMEMAX+16];      /* allow space for user defined operators */
      assert(curfunc!=NULL);
      funcdisplayname(symname,curfunc->name);
      error(209,symname);               /* function should return a value */
    } /* if */
    rettype|=uRETNONE;                  /* function does not return anything */
  } /* if */
  destructsymbols(&loctab,0);           /* call destructor for *all* locals */
  modstk((int)declared*sizeof(cell));   /* end of function, remove *all*
                                         * local variables */
  ffret(strcmp(curfunc->name,uENTRYFUNC)!=0);
}

static void dobreak(void)
{
  int *ptr;

  endlessloop=0;      /* if we were inside an endless loop, we just jumped out */
  ptr=readwhile();      /* readwhile() gives an error if not in loop */
  needtoken(tTERM);
  if (ptr==NULL)
    return;
  destructsymbols(&loctab,ptr[wqLVL]);
  clearassignments(1);
  modstk(((int)declared-ptr[wqBRK])*sizeof(cell));
  jumplabel(ptr[wqEXIT]);
}

static void docont(void)
{
  int *ptr;

  ptr=readwhile();      /* readwhile() gives an error if not in loop */
  needtoken(tTERM);
  if (ptr==NULL)
    return;
  destructsymbols(&loctab,ptr[wqLVL]);
  clearassignments(1);
  modstk(((int)declared-ptr[wqCONT])*sizeof(cell));
  jumplabel(ptr[wqLOOP]);
}

SC_FUNC void exporttag(int tag)
{
  /* find the tag by value in the table, then set the top bit to mark it
   * "public"
   */
  if (tag!=0 && (tag & PUBLICTAG)==0) {
    constvalue *ptr;
    for (ptr=tagname_tab.first; ptr!=NULL && tag!=(int)(ptr->value & TAGMASK); ptr=ptr->next)
      /* nothing */;
    if (ptr!=NULL)
      ptr->value |= PUBLICTAG;
  } /* if */
}

static void doexit(void)
{
  int tag=0;

  if (matchtoken(tTERM)==0){
    doexpr(TRUE,FALSE,FALSE,FALSE,&tag,NULL,TRUE,NULL);
    needtoken(tTERM);
  } else {
    ldconst(0,sPRI);
  } /* if */
  ldconst(tag,sALT);
  exporttag(tag);
  destructsymbols(&loctab,0);           /* call destructor for *all* locals */
  ffabort(xEXIT);
}

static void dosleep(void)
{
  int tag=0;

  if (matchtoken(tTERM)==0){
    doexpr(TRUE,FALSE,FALSE,FALSE,&tag,NULL,TRUE,NULL);
    needtoken(tTERM);
  } else {
    ldconst(0,sPRI);
  } /* if */
  ldconst(tag,sALT);
  exporttag(tag);
  ffabort(xSLEEP);

  /* for stack usage checking, mark the use of the sleep instruction */
  pc_memflags |= suSLEEP_INSTR;
}

static void dostate(void)
{
  constvalue *automaton;
  constvalue *state;
  constvalue *stlist;
  int flabel;
  symbol *sym;
  #if !defined SC_LIGHT
    int length,index,listid,listindex,stateindex;
    char *doc;
  #endif

  /* check for an optional condition */
  if (matchtoken('(')) {
    flabel=getlabel();          /* get label number for "false" branch */
    pc_docexpr=TRUE;            /* attach expression as a documentation string */
    test(flabel,TEST_PLAIN,FALSE);/* get expression, branch to flabel if false */
    pc_docexpr=FALSE;
    needtoken(')');
  } else {
    flabel=-1;
  } /* if */

  if (!sc_getstateid(&automaton,&state)) {
    delete_autolisttable();
    return;
  } /* if */
  needtoken(tTERM);

  /* store the new state id */
  assert(state!=NULL);
  ldconst(state->value,sPRI);
  assert(automaton!=NULL);
  assert(automaton->index==0 && automaton->name[0]=='\0' || automaton->index>0);
  storereg(automaton->value,sPRI);

  /* find the optional entry() function for the state */
  sym=findglb(uENTRYFUNC,sGLOBAL);
  if (sc_status==statWRITE && sym!=NULL && sym->ident==iFUNCTN && sym->states!=NULL) {
    for (stlist=sym->states->first; stlist!=NULL; stlist=stlist->next) {
      assert(!strempty(stlist->name));
      if (state_getfsa(stlist->index)==automaton->index && state_inlist(stlist->index,(int)state->value))
        break;      /* found! */
    } /* for */
    assert(stlist==NULL || state_inlist(stlist->index,state->value));
    if (stlist!=NULL) {
      /* the label to jump to is in stlist->name */
      ffcall(sym,stlist->name,0);
    } /* if */
  } /* if */

  if (flabel>=0)
    setlabel(flabel);           /* condition was false, jump around the state switch */

  #if !defined SC_LIGHT
    /* mark for documentation */
    if (sc_status==statFIRST) {
      char *str;
      /* get the last list id attached to the function, this contains the source states */
      assert(curfunc!=NULL);
      if (curfunc->states!=NULL) {
        stlist=curfunc->states->first;
        assert(stlist!=NULL);
        while (stlist->next!=NULL)
          stlist=stlist->next;
        listid=stlist->index;
      } else {
        listid=-1;
      } /* if */
      listindex=0;
      length=strlen(state->name)+70; /* +70 for the fixed part "<transition ... />\n" */
      /* see if there are any condition strings to attach */
      for (index=0; (str=get_autolist(index))!=NULL; index++)
        length+=strlen(str);
      if ((doc=(char*)malloc(length*sizeof(char)))!=NULL) {
        do {
          sprintf(doc,"<transition target=\"%s\"",state->name);
          if (listid>=0) {
            /* get the source state */
            stateindex=state_listitem(listid,listindex);
            state=state_findid(stateindex);
            assert(state!=NULL);
            sprintf(doc+strlen(doc)," source=\"%s\"",state->name);
          } /* if */
          if (get_autolist(0)!=NULL) {
            /* add the condition */
            strcat(doc," condition=\"");
            for (index=0; (str=get_autolist(index))!=NULL; index++) {
              /* remove the ')' token that may be appended before detecting that the expression has ended */
              if (*str!=')' || *(str+1)!='\0' || get_autolist(index+1)!=NULL)
                strcat(doc,str);
            } /* for */
            strcat(doc,"\"");
          } /* if */
          strcat(doc,"/>\n");
          insert_docstring(doc);
        } while (listid>=0 && ++listindex<state_count(listid));
        free(doc);
      } /* if */
    } /* if */
  #endif
  delete_autolisttable();
}


static void addwhile(int *ptr)
{
  int k;

  ptr[wqBRK]=(int)declared;     /* stack pointer (for "break") */
  ptr[wqCONT]=(int)declared;    /* for "continue", possibly adjusted later */
  ptr[wqLOOP]=getlabel();
  ptr[wqEXIT]=getlabel();
  ptr[wqLVL]=pc_nestlevel+1;
  if (wqptr>=(wq+wqTABSZ-wqSIZE))
    error(102,"loop table");    /* loop table overflow (too many active loops)*/
  k=0;
  while (k<wqSIZE){     /* copy "ptr" to while queue table */
    *wqptr=*ptr;
    wqptr+=1;
    ptr+=1;
    k+=1;
  } /* while */
}

static void delwhile(void)
{
  if (wqptr>wq)
    wqptr-=wqSIZE;
}

static int *readwhile(void)
{
  if (wqptr<=wq){
    error(24);          /* out of context */
    return NULL;
  } else {
    return (wqptr-wqSIZE);
  } /* if */
}

/* parsestringparam()
 *
 * Uses the standard string parsing mechanism to parse string parameters
 * for operator '__pragma'.
 */
static char *parsestringparam(int onlycheck,int *bck_litidx)
{
  int tok;
  int bck_packstr;
  cell val;
  char *str;

  assert(bck_litidx!=NULL);

  /* back up 'litidx', so we can remove the string from the literal queue later */
  *bck_litidx=litidx;
  /* force the string to be packed by default, so it would be easier to process it */
  bck_packstr=sc_packstr;
  sc_packstr=TRUE;

  /* read the string parameter */
  tok=lex(&val,&str);
  sc_packstr=bck_packstr;
  if (tok!=tSTRING || !pc_ispackedstr) {
    /* either not a string, or the user prepended "!" to the option string */
    char tokstr[2];
    if (tok==tSTRING) {
      tok='!';
      litidx=*bck_litidx;       /* remove the string from the literal queue */
    } /* if */
    if (tok<tFIRST) {
      sprintf(tokstr,"%c",tok);
      str=tokstr;
    } else {
      str=sc_tokens[tok-tFIRST];
    } /* if */
    error(1,sc_tokens[tSTRING-tFIRST],str);
    return NULL;
  } /* if */
  assert(litidx>*bck_litidx);

  if (onlycheck) {
    /* skip the byte swapping and remove the string from the literal queue,
     * as the caller only needed to check if the string was valid */
    litidx=*bck_litidx;
    return NULL;
  } /* if */

  /* swap the cell bytes if we're on a Little Endian platform */
#if BYTE_ORDER==LITTLE_ENDIAN
  { /* local */
    char *bytes;
    cell i=val;
    do {
      char t;
      bytes=(char *)&litq[i++];
      t=bytes[0], bytes[0]=bytes[sizeof(cell)-1], bytes[sizeof(cell)-1]=t;
#if PAWN_CELL_SIZE>=32
        t=bytes[1], bytes[1]=bytes[sizeof(cell)-2], bytes[sizeof(cell)-2]=t;
#if PAWN_CELL_SIZE==64
        t=bytes[2], bytes[2]=bytes[sizeof(cell)-3], bytes[sizeof(cell)-3]=t;
        t=bytes[3], bytes[3]=bytes[sizeof(cell)-4], bytes[sizeof(cell)-4]=t;
#endif // PAWN_CELL_SIZE==64
#endif // PAWN_CELL_SIZE>=32
    } while (bytes[0]!='\0' && bytes[1]!='\0'
#if PAWN_CELL_SIZE>=32
             && bytes[2]!='\0' && bytes[3]!='\0'
#if PAWN_CELL_SIZE==64
             && bytes[4]!='\0' && bytes[5]!='\0' && bytes[6]!='\0' && bytes[7]!='\0'
#endif // PAWN_CELL_SIZE==64
#endif // PAWN_CELL_SIZE>=32
    ); /* do */
  } /* local */
#endif
  return (char*)&litq[val];
}

static void dopragma(void)
{
  int bck_litidx;
  int i;
  cell val;
  char *str;

  needtoken('(');

  /* The options are specified as strings, e.g.
   *   native Func() __pragma("naked", "deprecated - use OtherFunc() instead");
   * In order to process the options, we can reuse the standard string parsing
   * mechanism. This way, as a bonus, we'll also be able to use multi-line
   * strings and the stringization operator.
   */
  do {
    /* read the option string */
    str=parsestringparam(FALSE,&bck_litidx);
    if (str==NULL)
      continue;

    /* split the option name from parameters */
    for (i=0; str[i]!='\0' && str[i]!=' '; i++)
      /* nothing */;
    if (str[i]!='\0') {
      str[i]='\0';
      while (str[++i]==' ')
        /* nothing */;
    } /* if */

    /* check the option name, set the corresponding attribute flag
     * and parse the argument(s), if needed */
    if (!strcmp(str,"deprecated")) {
      free(pc_deprecate);
      pc_deprecate=duplicatestring(&str[i]);
      if (pc_deprecate==NULL)
        error(103);     /* insufficient memory */
      pc_attributes |= (1U << attrDEPRECATED);
    } else if (!strcmp(str,"unused")) {
      pc_attributes |= (1U << attrUNUSED);
      if (str[i]!='\0') goto unknown_pragma;
    } else if (!strcmp(str,"unread")) {
      pc_attributes |= (1U << attrUNREAD);
      if (str[i]!='\0') goto unknown_pragma;
    } else if (!strcmp(str,"unwritten")) {
      pc_attributes |= (1U << attrUNWRITTEN);
      if (str[i]!='\0') goto unknown_pragma;
    } else if (!strcmp(str,"nodestruct")) {
      pc_attributes |= (1U << attrNODESTRUCT);
      if (str[i]!='\0') goto unknown_pragma;
    } else if (!strcmp(str,"naked")) {
      pc_attributes |= (1U << attrNAKED);
      if (str[i]!='\0') goto unknown_pragma;
    } else if (!strcmp(str,"warning")) {
      str += i;
      while (*str==' ') str++;
      for (i=0; str[i]!='\0' && str[i]!=' '; i++)
        /* nothing */;
      if (str[i]!='\0') {
        str[i]='\0';
        while (str[++i]==' ')
          /* nothing */;
      } /* if */
      if (strcmp(str,"enable")==0 || strcmp(str,"disable")==0) {
        int len=number(&val,(unsigned char *)&str[i]);
        if (len==0)
          goto unknown_pragma;
        pc_enablewarning((int)val,(str[0]=='e') ? warnENABLE : warnDISABLE);
        /* warn if there are extra characters after the warning number */
        for (i += len; str[i]==' '; i++)
          /* nothing */;
        if (str[i]!='\0')
          goto unknown_pragma;
      } else if (strcmp(str,"push")==0 && str[i]=='\0') {
        pc_pushwarnings();
      } else if (strcmp(str,"pop")==0 && str[i]=='\0') {
        pc_popwarnings();
      } else {
        goto unknown_pragma;
      } /* if */
    } else {
unknown_pragma:
      error(207);       /* unknown #pragma */
    } /* if */

    /* remove the string from the literal queue */
    litidx=bck_litidx;
  } while (matchtoken(','));

  needtoken(')');
}

static void pragma_apply(symbol *sym)
{
  int attr;

  /* make sure we have enough space for all attribute flags */
  assert_static((int)NUM_ATTRS<=sizeof(pc_attributes)*8);

  /* if no attributes are set, then we have a quick exit */
  if (pc_attributes==0)
    return;

  assert(sym!=NULL);

  for (attr=0; attr<NUM_ATTRS; attr++) {
    if ((pc_attributes & (1U << attr))==0)
      continue;
    switch (attr) {
    case attrDEPRECATED:
      pragma_deprecated(sym);
      break;
    case attrUNREAD:
    case attrUNWRITTEN:
    case attrUNUSED:
      pragma_unused(sym,(attr==attrUNREAD),(attr==attrUNWRITTEN));
      break;
    case attrNODESTRUCT:
      pragma_nodestruct(sym);
      break;
    case attrNAKED:
      if (sym->ident==iFUNCTN)
        sym->flags |= flagNAKED;
      break;
    default:
      assert(0);
    } /* switch */
  } /* for */

  pc_attributes=0;
}

SC_FUNC void pragma_deprecated(symbol *sym)
{
  if (pc_deprecate!=NULL) {
    if (sym->ident==iFUNCTN) {
      sym->flags |= flagDEPRECATED;
      if (sc_status==statWRITE) {
        if (sym->documentation!=NULL)
          free(sym->documentation);
        sym->documentation=pc_deprecate;
        pc_deprecate=NULL;
      } /* if */
    } /* if */
    free(pc_deprecate);
    pc_deprecate=NULL;
  } /* if */
}

SC_FUNC void pragma_unused(symbol *sym, int unread, int unwritten)
{
  assert(!unread || !unwritten);
  /* mark as read if the pragma wasn't "unwritten" */
  if (!unwritten) {
    sym->usage |= uREAD;
    sym->usage &= ~uASSIGNED;
  } /* if */
  /* mark as written if the pragma wasn't "unread" */
  if (sym->ident == iVARIABLE || sym->ident == iREFERENCE
      || sym->ident == iARRAY || sym->ident == iREFARRAY)
    sym->usage |= unread ? 0 : uWRITTEN;
}

SC_FUNC void pragma_nodestruct(symbol *sym)
{
  if (sym->ident==iVARIABLE || sym->ident==iARRAY)
    sym->usage |= uNODESTRUCT;
}

/* do_static_check()
 * Checks compile-time assertions and triggers an error/warning.
 *
 * The 'use_warning' parameter is set to TRUE if warnings are to be
 * used instead of errors to notify assertion failures.
 */
SC_FUNC cell do_static_check(int use_warning)
{
  int already_staging,already_recording,optmsg;
  int ident,index;
  int bck_litidx,recstartpos;
  cell cidx,val;
  char *str;

  optmsg=FALSE;
  index=0;
  cidx=0;

  needtoken('(');
  already_staging=stgget(&index,&cidx);
  if (!already_staging) {
    stgset(TRUE);       /* start stage-buffering */
    errorset(sEXPRMARK,0);
  } /* if */
  already_recording=pc_isrecording;
  if (!already_recording) {
    recstart();
    recstartpos=0;
  } else {
    recstop();  /* trim out the part of the current line that hasn't been read by lex() yet */
    recstartpos=strlen(pc_recstr);
    recstart(); /* restart recording */
  } /* if */
  ident=expression(&val,NULL,NULL,FALSE);
  if (!already_recording || val==0)
    recstop();
  str=&pc_recstr[recstartpos];
  if (recstartpos!=0 && pc_recstr[recstartpos]==' ')
    str++;      /* skip leading whitespace */
  if (ident!=iCONSTEXPR)
    error(8);           /* must be constant expression */
  stgdel(index,cidx);   /* scratch generated code */
  if (!already_staging) {
    errorset(sEXPRRELEASE,0);
    stgset(FALSE);      /* stop stage-buffering */
  } /* if */

  /* read the optional message */
  if (matchtoken(',')) {
    if (!already_recording) {
      free(pc_recstr);
      pc_recstr=NULL;
    } /* if */
    optmsg=TRUE;
    str=parsestringparam(val!=0,&bck_litidx);
  } /* if */

  if (val==0) {
    int errnum=use_warning ? 249    /* check failed */
                           : 110;   /* assertion failed */
    error(errnum,(str!=NULL) ? str : "");
    if (optmsg)
      litidx=bck_litidx;        /* remove the string from the literal queue */
    if (already_recording)
      recstart();               /* restart recording */
  } /* if */
  if (!optmsg && !already_recording) {
    free(pc_recstr);
    pc_recstr=NULL;
  } /* if */
  needtoken(')');
  return !!val;
}
