/*  Pawn compiler
 *
 *  Global (cross-module) variables.
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
 *  Version: $Id: scvars.c 3655 2006-10-23 20:17:52Z thiadmer $
 */

#include <stdio.h>
#include <stdlib.h>     /* for _MAX_PATH */
#include "sc.h"

/*  global variables
 *
 *  All global variables that are shared amongst the compiler files are
 *  declared here.
 */
SC_VDEFINE symbol loctab;                   /* local symbol table */
SC_VDEFINE symbol glbtab;                   /* global symbol table */
SC_VDEFINE struct hashtable_t symbol_cache_ht;
SC_VDEFINE cell *litq;                      /* the literal queue */
SC_VDEFINE unsigned char pline[sLINEMAX+1]; /* the line read from the input file */
SC_VDEFINE const unsigned char *lptr;       /* points to the current position in "pline" */
SC_VDEFINE constvalue_root tagname_tab={ NULL, NULL};  /* tagname table */
SC_VDEFINE constvalue_root libname_tab={ NULL, NULL};  /* library table (#pragma library "..." syntax) */
SC_VDEFINE constvalue *curlibrary=NULL;     /* current library */
SC_VDEFINE int pc_addlibtable=TRUE;         /* is the library table added to the AMX file? */
SC_VDEFINE symbol *curfunc;                 /* pointer to current function */
SC_VDEFINE char *inpfname;                  /* pointer to name of the file currently read from */
SC_VDEFINE char outfname[_MAX_PATH];        /* intermediate (assembler) file name */
SC_VDEFINE char binfname[_MAX_PATH];        /* binary file name */
SC_VDEFINE char errfname[_MAX_PATH];        /* error file name */
SC_VDEFINE char sc_ctrlchar=CTRL_CHAR;      /* the control character (or escape character)*/
SC_VDEFINE char sc_ctrlchar_org=CTRL_CHAR;  /* the default control character */
SC_VDEFINE int litidx=0;                    /* index to literal table */
SC_VDEFINE int litmax=sDEF_LITMAX;          /* current size of the literal table */
SC_VDEFINE int litgrow=sDEF_LITMAX;         /* amount to increase the literal table by */
SC_VDEFINE int stgidx=0;                    /* index to the staging buffer */
SC_VDEFINE int sc_labnum=0;                 /* number of (internal) labels */
SC_VDEFINE int staging=FALSE;               /* true if staging output */
SC_VDEFINE cell declared=0;                 /* number of local cells declared */
SC_VDEFINE cell glb_declared=0;             /* number of global cells declared */
SC_VDEFINE cell code_idx=0;                 /* number of bytes with generated code */
SC_VDEFINE int ntv_funcid= 0;               /* incremental number of native function */
SC_VDEFINE int errnum=0;                    /* number of errors */
SC_VDEFINE int warnnum=0;                   /* number of warnings */
SC_VDEFINE int sc_debug=sCHKBOUNDS;         /* by default: bounds checking+assertions */
SC_VDEFINE int sc_packstr= FALSE;           /* strings are packed by default? */
SC_VDEFINE int sc_asmfile= FALSE;           /* create .ASM file? */
SC_VDEFINE int sc_listing= FALSE;           /* create .LST file? */
SC_VDEFINE int sc_compress=TRUE;            /* compress bytecode? */
SC_VDEFINE int sc_needsemicolon=TRUE;       /* semicolon required to terminate expressions? */
SC_VDEFINE int sc_dataalign=sizeof(cell);   /* data alignment value */
SC_VDEFINE int sc_alignnext=FALSE;          /* must frame of the next function be aligned? */
SC_VDEFINE int pc_docexpr=FALSE;            /* must expression be attached to documentation comment? */
SC_VDEFINE int curseg=0;                    /* 1 if currently parsing CODE, 2 if parsing DATA */
SC_VDEFINE cell pc_stksize=sDEF_AMXSTACK;   /* default stack size */
SC_VDEFINE cell pc_amxlimit=0;              /* default abstract machine size limit = none */
SC_VDEFINE cell pc_amxram=0;                /* default abstract machine data size limit = none */
SC_VDEFINE int freading=FALSE;              /* Is there an input file ready for reading? */
SC_VDEFINE int fline=0;                     /* the line number in the current file */
SC_VDEFINE symbol *line_sym=NULL;           /* the line number constant (__line) */
SC_VDEFINE short fnumber=0;                 /* the file number in the file table (debugging) */
SC_VDEFINE short fcurrent=0;                /* current file being processed (debugging) */
SC_VDEFINE short sc_intest=FALSE;           /* true if inside a test */
SC_VDEFINE int pc_sideeffect=FALSE;         /* true if an expression causes a side-effect */
SC_VDEFINE int pc_ovlassignment=FALSE;      /* true if an expression contains an overloaded assignment */
SC_VDEFINE int stmtindent=0;                /* current indent of the statement */
SC_VDEFINE int indent_nowarn=FALSE;         /* skip warning "217 loose indentation" */
SC_VDEFINE int sc_tabsize=8;                /* number of spaces that a TAB represents */
SC_VDEFINE short sc_allowtags=TRUE;         /* allow/detect tagnames in lex() */
SC_VDEFINE int sc_status;                   /* read/write status */
SC_VDEFINE int sc_rationaltag=0;            /* tag for rational numbers */
SC_VDEFINE int rational_digits=0;           /* number of fractional digits */
SC_VDEFINE int sc_allowproccall=FALSE;      /* allow/detect tagnames in lex() */
SC_VDEFINE short sc_is_utf8=FALSE;          /* is this source file in UTF-8 encoding */
SC_VDEFINE char *pc_deprecate=NULL;         /* if non-null, mark next declaration as deprecated */
SC_VDEFINE int sc_curstates=0;              /* ID of the current state list */
SC_VDEFINE int pc_optimize=sOPTIMIZE_NOMACRO; /* (peephole) optimization level */
SC_VDEFINE int pc_memflags=0;               /* special flags for the stack/heap usage */
SC_VDEFINE int pc_naked=FALSE;              /* if true mark following function as naked */
SC_VDEFINE int pc_compat=FALSE;             /* running in compatibility mode? */
SC_VDEFINE int pc_recursion=FALSE;          /* enable detailed recursion report? */
SC_VDEFINE int pc_retexpr=FALSE;            /* true if the current expression is a part of a "return" statement */
SC_VDEFINE int pc_retheap=0;                /* heap space (in bytes) to be manually freed when returning an array returned by another function */
SC_VDEFINE int pc_nestlevel=0;              /* number of active (open) compound statements */
SC_VDEFINE unsigned int pc_attributes=0;    /* currently set attribute flags (for the "__pragma" operator) */
SC_VDEFINE int pc_ispackedstr=FALSE;        /* true if the last tokenized string is packed */
SC_VDEFINE int pc_isrecording=FALSE;        /* true if recording input */
SC_VDEFINE char *pc_recstr=NULL;            /* recorded input */
SC_VDEFINE int pc_loopcond=FALSE;           /* true if the current expression is a loop condition */
SC_VDEFINE int pc_numloopvars=0;            /* number of variables used inside a loop condition */

SC_VDEFINE char *sc_tokens[] = {
  "*=", "/=", "%=", "+=", "-=", "<<=", ">>>=", ">>=", "&=", "^=", "|=",
  "||", "&&", "==", "!=", "<=", ">=", "<<", ">>>", ">>", "++", "--",
  "...", "..",
  "__addressof", "assert", "break", "case", "char", "const", "continue",
  "default", "defined", "do", "else", "__emit", "enum", "exit", "for",
  "forward", "goto", "if", "__nameof", "native", "new", "operator", "__pragma",
  "public", "return", "sizeof", "sleep", "state", "static", "__static_assert",
  "__static_check", "stock", "switch", "tagof", "while",
  "#assert", "#define", "#else", "#elseif", "#emit", "#endif", "#endinput",
  "#endscript", "#error", "#file", "#if", "#include", "#line", "#pragma",
  "#tryinclude", "#undef", "#warning",
  ";", ";", "-integer value-", "-rational value-", "-identifier-",
  "-label-", "-string-",
  "-any value-", "-numeric value-", "-data offset-", "-local variable-",
  "-reference-", "-function-", "-native function-", "-nonnegative integer-"
};

SC_VDEFINE constvalue_root sc_automaton_tab = { NULL, NULL}; /* automaton table */
SC_VDEFINE constvalue_root sc_state_tab = { NULL, NULL};   /* state table */

SC_VDEFINE FILE *inpf    = NULL;   /* file read from (source or include) */
SC_VDEFINE FILE *inpf_org= NULL;   /* main source file */
SC_VDEFINE FILE *outf    = NULL;   /* (intermediate) text file written to */

SC_VDEFINE jmp_buf errbuf;

SC_VDEFINE int emit_flags;
SC_VDEFINE int emit_stgbuf_idx;

#if !defined SC_LIGHT
  SC_VDEFINE int sc_makereport=FALSE; /* generate a cross-reference report */
#endif

#if defined __WATCOMC__ && !defined NDEBUG
  /* Watcom's CVPACK dislikes .OBJ files without functions */
  static int dummyfunc(void)
  {
    return 0;
  }
#endif
