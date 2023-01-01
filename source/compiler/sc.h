/*  Pawn compiler
 *
 *  Drafted after the Small-C compiler Version 2.01, originally created
 *  by Ron Cain, july 1980, and enhanced by James E. Hendrix.
 *
 *  This version comes close to a complete rewrite.
 *
 *  Copyright R. Cain, 1980
 *  Copyright J.E. Hendrix, 1982, 1983
 *  Copyright ITB CompuPhase, 1997-2006
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
 *  Version: $Id: sc.h 3660 2006-11-05 13:05:09Z thiadmer $
 */

#ifndef SC_H_INCLUDED
#define SC_H_INCLUDED
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#if defined __BORLANDC__ && defined _Windows && !(defined __32BIT__ || defined __WIN32__)
  /* setjmp() and longjmp() not well supported in 16-bit windows */
  #include <windows.h>
  typedef int jmp_buf[9];
  #define setjmp(b)     Catch(b)
  #define longjmp(b,e)  Throw(b,e)
#else
  #include <setjmp.h>
#endif
#include "hashtable/wrap_hashtable.h"
#include "../amx/osdefs.h"
#include "../amx/amx.h"

/* Note: the "cell" and "ucell" types are defined in AMX.H */

#define PUBLIC_CHAR '@'     /* character that defines a function "public" */
#define CTRL_CHAR   '\\'    /* default control character */
#define sCHARBITS   8       /* size of a packed character */

#define sDIMEN_MAX       4    /* maximum number of array dimensions */
#define sLINEMAX      4095    /* input line length (in characters) */
#define sCOMP_STACK     32    /* maximum nesting of #if .. #endif sections */
#define sDEF_LITMAX  10000    /* initial size of the literal pool, in "cells" */
#define sDEF_AMXSTACK 4096    /* default stack size for AMX files */
#define PREPROC_TERM  '\x7f'  /* termination character for preprocessor expressions (the "DEL" code) */
#define sDEF_PREFIX   "default.inc" /* default prefix filename */

typedef union {
  void *pv;                 /* e.g. a name */
  int i;
} stkitem;                  /* type of items stored on the compiler stack */

typedef struct s_arginfo {  /* function argument info */
  char name[sNAMEMAX+1];
  char ident;           /* iVARIABLE, iREFERENCE, iREFARRAY or iVARARGS */
  char usage;           /* uCONST */
  int *tags;            /* argument tag id. list */
  int numtags;          /* number of tags in the tag list */
  int dim[sDIMEN_MAX];
  int idxtag[sDIMEN_MAX];
  int numdim;           /* number of dimensions */
  unsigned char hasdefault; /* bit0: is there a default value? bit6: "tagof"; bit7: "sizeof" */
  union {
    cell val;           /* default value */
    struct {
      char *symname;    /* name of another symbol */
      short level;      /* indirection level for that symbol */
    } size;             /* used for "sizeof" default value */
    struct {
      cell *data;       /* values of default array */
      int size;         /* complete length of default array */
      int arraysize;    /* size to reserve on the heap */
      cell addr;        /* address of the default array in the data segment */
    } array;
  } defvalue;           /* default value, or pointer to default array */
  int defvalue_tag;     /* tag of the default value */
} arginfo;

/*  Equate table, tagname table, library table */
typedef struct s_constvalue {
  struct s_constvalue *next;
  char name[sNAMEMAX+1];
  cell value;
  int index;            /* index level, for constants referring to array sizes/tags
                         * automaton id, for states and automatons
                         * tag for enumeration lists */
} constvalue;

typedef struct s_constvalue_root {
  constvalue *first,*last;
} constvalue_root;

/*  Symbol table format
 *
 *  The symbol name read from the input file is stored in "name", the
 *  value of "addr" is written to the output file. The address in "addr"
 *  depends on the class of the symbol:
 *      global          offset into the data segment
 *      local           offset relative to the stack frame
 *      label           generated hexadecimal number
 *      function        offset into code segment
 */
typedef struct s_symbol {
  struct s_symbol *next;
  struct s_symbol *parent;  /* hierarchical types (multi-dimensional arrays) */
  struct s_symbol *child;
  struct s_symbol *htnext;
  char name[sNAMEMAX+1];
  cell addr;            /* address or offset (or value for constant, index for native function) */
  cell codeaddr;        /* address (in the code segment) where the symbol declaration starts */
  char vclass;          /* sLOCAL if "addr" refers to a local symbol */
  char ident;           /* see below for possible values */
  short usage;          /* see below for possible values */
  char flags;           /* see below for possible values */
  int compound;         /* compound level (braces nesting level) */
  int tag;              /* tagname id */
  union {
    int declared;       /* label: how many local variables are declared */
    struct {
      int index;        /* array & enum: tag of array indices or the enum item */
      int field;        /* enumeration fields, where a size is attached to the field */
      int unique;       /* number of enumeration elements with unique value */
    } tags;             /* extra tags */
    constvalue *lib;    /* native function: library it is part of */
    long stacksize;     /* normal/public function: stack requirements */
  } x;                  /* 'x' for 'extra' */
  union {
    arginfo *arglist;   /* types of all parameters for functions */
    constvalue_root *enumlist;/* list of names for the "root" of an enumeration */
    struct {
      cell length;      /* arrays: length (size) */
      short level;      /* number of dimensions below this level */
    } array;
  } dim;                /* for 'dimension', both functions and arrays */
  int assignlevel;      /* 'compound statement' level at which the variable was assigned a value */
  constvalue_root *states;/* list of state function/state variable ids + addresses */
  int fnumber;          /* static global variables: file number in which the declaration is visible */
  int lnumber;          /* line number (in the current source file) for the declaration */
  struct s_symbol **refer;  /* referrer list, functions that "use" this symbol */
  int numrefers;        /* number of entries in the referrer list */
  char *documentation;  /* optional documentation string */
} symbol;


/*  Possible entries for "ident". These are used in the "symbol", "value"
 *  and arginfo structures. Not every constant is valid for every use.
 *  In an argument list, the list is terminated with a "zero" ident; labels
 *  cannot be passed as function arguments, so the value 0 is overloaded.
 */
#define iLABEL      0
#define iVARIABLE   1   /* cell that has an address and that can be fetched directly (lvalue) */
#define iREFERENCE  2   /* iVARIABLE, but must be dereferenced */
#define iARRAY      3
#define iREFARRAY   4   /* an array passed by reference (i.e. a pointer) */
#define iARRAYCELL  5   /* array element, cell that must be fetched indirectly */
#define iARRAYCHAR  6   /* array element, character from cell from array */
#define iEXPRESSION 7   /* expression result, has no address (rvalue) */
#define iCONSTEXPR  8   /* constant expression (or constant symbol) */
#define iFUNCTN     9
#define iREFFUNC    10
#define iVARARGS    11  /* function specified ... as argument(s) */

/*  Possible entries for "usage"
 *
 *  This byte is used as a series of bits, the syntax is different for
 *  functions and other symbols:
 *
 *  VARIABLE
 *  bits: 0     (uDEFINE) the variable is defined in the source file
 *        1     (uREAD) the variable is "read" (accessed) in the source file
 *        2     (uWRITTEN) the variable is altered (assigned a value)
 *        3     (uCONST) the variable is constant (may not be assigned to)
 *        4     (uPUBLIC) the variable is public
 *        6     (uSTOCK) the variable is discardable (without warning)
 *
 *  FUNCTION
 *  bits: 0     (uDEFINE) the function is defined ("implemented") in the source file
 *        1     (uREAD) the function is invoked in the source file
 *        2     (uRETVALUE) the function returns a value (or should return a value)
 *        3     (uPROTOTYPED) the function was prototyped (implicitly via a definition or explicitly)
 *        4     (uPUBLIC) the function is public
 *        5     (uNATIVE) the function is native
 *        6     (uSTOCK) the function is discardable (without warning)
 *        7     (uMISSING) the function is not implemented in this source file
 *        8     (uFORWARD) the function is explicitly forwardly declared
 *
 *  CONSTANT
 *  bits: 0     (uDEFINE) the symbol is defined in the source file
 *        1     (uREAD) the constant is "read" (accessed) in the source file
 *        2     (uWRITTEN) redundant, but may be set for constants passed by reference
 *        5     (uENUMROOT) the constant is the "root" of an enumeration
 *        6     (uENUMFIELD) the constant is a field in a named enumeration
 */
#define uDEFINE     0x001
#define uREAD       0x002
#define uWRITTEN    0x004
#define uRETVALUE   0x004 /* function returns (or should return) a value */
#define uCONST      0x008
#define uPROTOTYPED 0x008
#define uPUBLIC     0x010
#define uNATIVE     0x020
#define uENUMROOT   0x020
#define uSTOCK      0x040
#define uENUMFIELD  0x040
#define uMISSING    0x080
#define uFORWARD    0x100
#define uNODESTRUCT 0x200 /* "no destruct(or)", not "node struct" */
/* symbol is referenced "globally", e.g. via "__emit" or "#emit" used outside functions */
#define uGLOBALREF  0x400
/* uRETNONE is not stored in the "usage" field of a symbol. It is
 * used during parsing a function, to detect a mix of "return;" and
 * "return value;" in a few special cases.
 */
#define uRETNONE    0x010
/* uASSIGNED indicates that a value assigned to the variable is not used yet */
#define uASSIGNED   0x080
/* uLOOPVAR is set when a variable is read inside of a loop condition. This is
 * used to detect situations when a variable is used in a loop condition, but
 * not modified inside of a loop body. */
#define uLOOPVAR    0x1000
/* uNOLOOPVAR is set when a variable is
 *   * modified inside of a loop condition before being read, or
 *   * used in an enclosing loop and should be excluded from checks in an inner loop,
 * so the compiler would know it shouldn't set the uLOOPVAR flag when the variable
 * is read inside a loop condition */
#define uNOLOOPVAR  0x2000

#define flagDEPRECATED 0x01  /* symbol is deprecated (avoid use) */
#define flagNAKED     0x10  /* function is naked */
#define flagPREDEF    0x20  /* symbol is pre-defined; successor of uPREDEF */

#define uTAGOF_TAG 0x20  /* set in the "hasdefault" field of the arginfo struct */
#define uTAGOF    0x40  /* set in the "hasdefault" field of the arginfo struct */
#define uSIZEOF   0x80  /* set in the "hasdefault" field of the arginfo struct */

#define uMAINFUNC "main"
#define uENTRYFUNC "entry"

#define sGLOBAL   0     /* global variable/constant class (no states) */
#define sLOCAL    1     /* local variable/constant */
#define sSTATIC   2     /* global life, local scope */

#define sSTATEVAR  3    /* criterion to find variables (sSTATEVAR implies a global variable) */

typedef struct s_value {
  symbol *sym;          /* symbol in symbol table, NULL for (constant) expression */
  cell constval;        /* value of the constant expression (if ident==iCONSTEXPR)
                         * also used for the size of a literal array */
  int tag;              /* tag (of the expression) */
  int cmptag;           /* for searching symbols: choose the one with the matching tag */
  char ident;           /* iCONSTEXPR, iVARIABLE, iARRAY, iARRAYCELL,
                         * iEXPRESSION or iREFERENCE */
  char boolresult;      /* boolean result for relational operators */
  cell *arrayidx;       /* last used array indices, for checking self assignment */
} value;

/*  "while" statement queue (also used for "for" and "do - while" loops) */
enum {
  wqBRK,        /* used to restore stack for "break" */
  wqCONT,       /* used to restore stack for "continue" */
  wqLOOP,       /* loop start label number */
  wqEXIT,       /* loop exit label number (jump if false) */
  wqLVL,        /* "compound statement" nesting level for the loop body
                 * (used to call destructors for "break" and "continue") */
  /* --- */
  wqSIZE        /* "while queue" size */
};
#define wqTABSZ (24*wqSIZE)    /* 24 nested loop statements */

enum {
  statIDLE,     /* not compiling yet */
  statFIRST,    /* first pass */
  statSECOND,   /* second pass */
  statWRITE,    /* writing output */
  statSKIP,     /* skipping output */
};

typedef struct s_stringlist {
  char **data;
  char **strings;
  int length;
  int size;
} stringlist;

typedef struct s_stringpair {
  struct s_stringpair *next;
  char *first;
  char *second;
  int matchlength;
} stringpair;

typedef struct s_valuepair {
  struct s_valuepair *next;
  long first;
  long second;
} valuepair;

/* struct "symstate" is used to:
 * * synchronize the status of assignments between all "if" branches or "switch"
 *   cases, so the compiler could detect unused assignments in all of those
 *   branches/cases, not only in the last one;
 * * back up the "uNOLOOPVAR" flag when scanning for variables that were used
 *   in a loop exit condition, but weren't modified inside the loop body */
typedef struct s_assigninfo {
  int lnumber;      /* line number of the first unused assignment made in one of
                     * the branches (used for error messages) */
  short usage;      /* usage flags to memoize (currently only uASSIGNED) */
} symstate;

/* macros for code generation */
#define opcodes(n)      ((n)*sizeof(cell))      /* opcode size */
#define opargs(n)       ((n)*sizeof(cell))      /* size of typical argument */

/* general purpose macros */
#if defined _MSC_VER
  #define SC_FASTCALL __fastcall
#elif defined __GNUC__ && (defined __i386__ || defined __x86_64__ || defined __amd64__)
  #if !defined __x86_64__ && !defined __amd64__ && (__GNUC__>=4 || __GNUC__==3 && __GNUC_MINOR__>=4)
    #define SC_FASTCALL __attribute__((fastcall))
  #else
    #define SC_FASTCALL __attribute__((regparm(3)))
  #endif
#endif
#if !defined SC_FASTCALL
  #define SC_FASTCALL
#endif
#if !defined strempty
  #define strempty(str) ((str)[0]=='\0')
#endif
#if !defined arraysize
  #if defined __clang__
    #if !__is_identifier(__builtin_types_compatible_p)
      #define USE_GCC_ARRAYSIZE
    #endif
  #elif !defined __clang__ && defined __GNUC__
    #if (__GNUC__==3 && __GNUC_MINOR__>=1) || __GNUC__>=4
      #define USE_GCC_ARRAYSIZE
    #endif
  #endif
  #if defined USE_GCC_ARRAYSIZE
    #undef USE_GCC_ARRAYSIZE
    #define arraysize(array) \
      (sizeof(struct{int x[-__builtin_types_compatible_p(typeof(array),typeof(&(array)[0]))];}) | \
      sizeof(array) / sizeof(array[0]))
  #else
    #define arraysize(array) (sizeof(array) / sizeof(array[0]))
  #endif
#endif
#if !defined makelong
  #define makelong(low,high) ((long)(low) | ((long)(high) << (sizeof(long)*4)))
#endif

/*  Tokens recognized by lex()
 *  Some of these constants are assigned as well to the variable "lastst" (see SC1.C)
 */
enum {
  /* multi-character operators */
  tFIRST  = 256,  /* value of first multi-character operator */

  taMULT  =  tFIRST,  /* *= */
  taDIV,  /* /= */
  taMOD,  /* %= */
  taADD,  /* += */
  taSUB,  /* -= */
  taSHL,  /* <<= */
  taSHRU, /* >>>= */
  taSHR,  /* >>= */
  taAND,  /* &= */
  taXOR,  /* ^= */
  taOR,   /* |= */
  tlOR,   /* || */
  tlAND,  /* && */
  tlEQ,   /* == */
  tlNE,   /* != */
  tlLE,   /* <= */
  tlGE,   /* >= */
  tSHL,   /* << */
  tSHRU,  /* >>> */
  tSHR,   /* >> */
  tINC,   /* ++ */
  tDEC,   /* -- */
  tELLIPS,  /* ... */
  tDBLDOT,  /* .. */

  tMIDDLE = tDBLDOT, /* value of last multi-character operator */

  /* reserved words (statements) */
  t__ADDRESSOF,
  tASSERT,
  tBEGIN,
  tBREAK,
  tCASE,
  tCHAR,
  tCONST,
  tCONTINUE,
  tDEFAULT,
  tDEFINED,
  tDO,
  tELSE,
  t__EMIT,
  tEND,
  tENUM,
  tEXIT,
  tFOR,
  tFORWARD,
  tGOTO,
  tIF,
  t__NAMEOF,
  tNATIVE,
  tNEW,
  tOPERATOR,
  t__PRAGMA,
  tPUBLIC,
  tRETURN,
  tSIZEOF,
  tSLEEP,
  tSTATE,
  tSTATIC,
  t__STATIC_ASSERT,
  t__STATIC_CHECK,
  tSTOCK,
  tSWITCH,
  tTAGOF,
  tTHEN,
  tWHILE,

  /* compiler directives */
  tpASSERT, /* #assert */
  tpDEFINE,
  tpELSE, /* #else */
  tpELSEIF, /* #elseif */
  tpEMIT,
  tpENDIF,
  tpENDINPUT,
  tpENDSCRPT,
  tpERROR,
  tpFILE,
  tpIF,/* #if */
  tpINCLUDE,
  tpLINE,
  tpPRAGMA,
  tpTRYINCLUDE,
  tpUNDEF,
  tpWARNING,

  tLAST = tpWARNING, /* value of last multi-character match-able token */

  /* semicolon is a special case, because it can be optional */
  tTERM,/* semicolon or newline */
  tENDEXPR, /* forced end of expression */
  /* other recognized tokens */
  tNUMBER,/* integer number */
  tRATIONAL, /* rational number */
  tSYMBOL,
  tLABEL,
  tSTRING,
  /* argument types for emit/__emit */
  teANY, /* any value */
  teNUMERIC, /* integer/rational number */
  teDATA, /* data (variable name or address) */
  teLOCAL, /* local variable (name or offset) */
  teREFERENCE, /* function argument passed by reference */
  teFUNCTN, /* Pawn function */
  teNATIVE, /* native function */
  teNONNEG, /* nonnegative integer */
  /* for assignment to "lastst" only (see SC1.C) */
  tEXPR,
  tENDLESS, /* endless loop */
  tTERMINAL, /* signalizes that the code after this statement is unreachable,
              * which can happen when:
              *  * both 'if' branches end with different kinds of "terminal"
              *    statements, such as 'return', 'break', 'continue' or endless
              *    loop;
              *  * a 'goto' is used on an already implemented label (which has
              *    the 'uDEFINE' flag set) and there are no undefined labels
              *    ("declared" through 'goto', but not implemented yet)
              */
  tTERMSWITCH, /* signalizes that all 'switch' cases (including 'default') end
                * with terminal statements; */
};

/* (reversed) evaluation of staging buffer */
#define sSTARTREORDER 0x01
#define sENDREORDER   0x02
#define sEXPRSTART    0x80      /* top bit set, rest is free */
#define sMAXARGS      127       /* relates to the bit pattern of sEXPRSTART */

#define sDOCSEP       0x01      /* to separate documentation comments between functions */

/* codes for ffabort() */
#define xEXIT           1       /* exit code in PRI */
#define xASSERTION      2       /* abort caused by failing assertion */
#define xSTACKERROR     3       /* stack/heap overflow */
#define xBOUNDSERROR    4       /* array index out of bounds */
#define xMEMACCESS      5       /* data access error */
#define xINVINSTR       6       /* invalid instruction */
#define xSTACKUNDERFLOW 7       /* stack underflow */
#define xHEAPUNDERFLOW  8       /* heap underflow */
#define xCALLBACKERR    9       /* no, or invalid, callback */
#define xSLEEP         12       /* sleep, exit code in PRI, tag in ALT */

/* Miscellaneous  */
#if !defined TRUE
  #define FALSE         0
  #define TRUE          1
#endif
#define sIN_CSEG        1       /* if parsing CODE */
#define sIN_DSEG        2       /* if parsing DATA */
#define sCHKBOUNDS      1       /* bit position in "debug" variable: check bounds */
#define sSYMBOLIC       2       /* bit position in "debug" variable: symbolic info */
#define sRESET          0       /* reset error flag */
#define sFORCESET       1       /* force error flag on */
#define sEXPRMARK       2       /* mark start of expression */
#define sEXPRRELEASE    3       /* mark end of expression */
#define sSETPOS         4       /* set line number for the error */

enum {
  sOPTIMIZE_NONE,               /* no optimization */
  sOPTIMIZE_NOMACRO,            /* no macro instructions */
  sOPTIMIZE_FULL,               /* full optimization */
  /* ----- */
  sOPTIMIZE_NUMBER
};

typedef enum s_regid {
  sPRI,                         /* indicates the primary register */
  sALT,                         /* indicates the secundary register */
} regid;

typedef enum s_optmark {
  sEXPR,                        /* end of expression (for expressions that form a statement) */
  sPARM,                        /* end of parameter (in a function parameter list) */
  sLDECL,                       /* start of local declaration (variable) */
} optmark;

#define suSLEEP_INSTR 0x01      /* the "sleep" instruction was used */

#if INT_MAX<0x8000u
  #define PUBLICTAG   0x8000u
  #define FIXEDTAG    0x4000u
#else
  #define PUBLICTAG   0x80000000Lu
  #define FIXEDTAG    0x40000000Lu
#endif
#define TAGMASK       (~PUBLICTAG)
#define BOOLTAG       1
#define CELL_MAX      (((ucell)1 << (sizeof(cell)*8-1)) - 1)

#define MAX_INSTR_LEN   30

typedef enum s_warnmode {
  warnDISABLE,
  warnENABLE,
  warnTOGGLE
} warnmode;

#define eotNUMBER       0
#define eotFUNCTION     1
#define eotLABEL        2
typedef struct s_emit_outval {
  int type;
  union {
    ucell ucell;
    const char *string;
  } value;
} emit_outval;

/* constants for error_suggest() */
#define MAX_EDIT_DIST 2 /* allow two mis-typed characters; when there are more,
                         * the names are too different, and no match is returned */
enum {  /* identifier types */
  estSYMBOL = 0,
  estNONSYMBOL,
  estAUTOMATON,
  estSTATE
};
enum {  /* search types for error_suggest() when the identifier type is "estSYMBOL" */
/* symbol type flags */
  esfLABEL      = 1 << 0, /* label */
  esfCONST      = 1 << 1, /* named constant */
  esfVARIABLE   = 1 << 2, /* single variable */
  esfARRAY      = 1 << 3, /* array */
  esfPAWNFUNC   = 1 << 4, /* Pawn function */
  esfNATIVE     = 1 << 5, /* native function */

/* composite search types */
  /* find symbols of any type (used only to define other search types) */
  esfANY        = esfLABEL | esfCONST | esfVARIABLE | esfARRAY | esfPAWNFUNC | esfNATIVE,

  /* any function */
  esfFUNCTION   = esfPAWNFUNC | esfNATIVE,

  /* find symbols of any type but labels */
  esfNONLABEL   = esfANY & ~esfLABEL,

  /* find symbols of any type except constants and native functions
   * (for the "__addressof" operator) */
  esfADDRESSOF  = esfANY & ~(esfCONST | esfNATIVE),

  /* find an array, a single variable, or a named constant */
  esfVARCONST   = esfCONST | esfVARIABLE | esfARRAY
};

enum { /* attribute flags for "__pragma" */
  attrDEPRECATED,
  attrUNUSED,
  attrUNREAD,
  attrUNWRITTEN,
  attrNODESTRUCT,
  attrNAKED,
  NUM_ATTRS
};

/* interface functions */
#if defined __cplusplus
  extern "C" {
#endif

/*
 * Functions you call from the "driver" program
 */
int pc_compile(int argc, char **argv);
int pc_addconstant(char *name,cell value,int tag);
int pc_addtag(char *name);
int pc_enablewarning(int number,warnmode enable);
void pc_pushwarnings(void);
void pc_popwarnings(void);
void pc_seterrorwarnings(int enable);
int pc_geterrorwarnings(void);

/*
 * Functions called from the compiler (to be implemented by you)
 */

/* general console output */
int pc_printf(const char *message,...);

/* error report function */
int pc_error(int number,char *message,char *filename,int firstline,int lastline,va_list argptr);

/* input from source file */
void *pc_opensrc(char *filename); /* reading only */
void *pc_createsrc(char *filename);
void *pc_createtmpsrc(char **filename);
void pc_closesrc(void *handle);   /* never delete */
void pc_resetsrc(void *handle,void *position);  /* reset to a position marked earlier */
char *pc_readsrc(void *handle,unsigned char *target,int maxchars);
int pc_writesrc(void *handle,unsigned char *source);
void *pc_getpossrc(void *handle); /* mark the current position */
int  pc_eofsrc(void *handle);

/* output to intermediate (.ASM) file */
void *pc_openasm(char *filename); /* read/write */
void pc_closeasm(void *handle,int deletefile);
void pc_resetasm(void *handle);
int  pc_writeasm(void *handle,char *str);
char *pc_readasm(void *handle,char *string,int maxchars);

/* output to binary (.AMX) file */
void *pc_openbin(char *filename);
void pc_closebin(void *handle,int deletefile);
void pc_resetbin(void *handle,long offset);
int  pc_writebin(void *handle,void *buffer,int size);
long pc_lengthbin(void *handle); /* return the length of the file */

#if defined __cplusplus
  }
#endif


/* by default, functions and variables used in throughout the compiler
 * files are "external"
 */
#if !defined SC_FUNC
  #define SC_FUNC
#endif
#if !defined SC_VDECL
  #define SC_VDECL  extern
#endif
#if !defined SC_VDEFINE
  #define SC_VDEFINE
#endif

/* function prototypes in SC1.C */
SC_FUNC void set_extension(char *filename,char *extension,int force);
SC_FUNC symbol *fetchfunc(char *name,int tag);
SC_FUNC char *operator_symname(char *symname,char *opername,int tag1,int tag2,int numtags,int resulttag);
SC_FUNC void check_index_tagmismatch(char *symname,int expectedtag,int actualtag,int allowcoerce,int errline);
SC_FUNC void check_tagmismatch(int formaltag,int actualtag,int allowcoerce,int errline);
SC_FUNC void check_tagmismatch_multiple(int formaltags[],int numtags,int actualtag,int errline);
SC_FUNC char *funcdisplayname(char *dest,char *funcname);
SC_FUNC int constexpr(cell *val,int *tag,symbol **symptr);
SC_FUNC constvalue *insert_constval(constvalue *prev,constvalue *next,const char *name,cell val,int index);
SC_FUNC constvalue *append_constval(constvalue_root *table,const char *name,cell val,int index);
SC_FUNC constvalue *find_constval(constvalue_root *table,char *name,int index);
SC_FUNC void delete_consttable(constvalue_root *table);
SC_FUNC symbol *add_constant(char *name,cell val,int vclass,int tag);
SC_FUNC symbol *add_builtin_constant(char *name,cell val,int vclass,int tag);
SC_FUNC symbol *add_builtin_string_constant(char *name,const char *val,int vclass);
SC_FUNC void exporttag(int tag);
SC_FUNC void sc_attachdocumentation(symbol *sym);
SC_FUNC void emit_parse_line(void);
SC_FUNC void pragma_deprecated(symbol *sym);
SC_FUNC void pragma_unused(symbol *sym,int unread,int unwritten);
SC_FUNC void pragma_nodestruct(symbol *sym);
SC_FUNC cell do_static_check(int use_warning);

/* function prototypes in SC2.C */
#define PUSHSTK_P(v)  { stkitem s_; s_.pv=(v); pushstk(s_); }
#define PUSHSTK_I(v)  { stkitem s_; s_.i=(v); pushstk(s_); }
#define POPSTK_P()    (popstk().pv)
#define POPSTK_I()    (popstk().i)
SC_FUNC void pushstk(stkitem val);
SC_FUNC stkitem popstk(void);
SC_FUNC void clearstk(void);
SC_FUNC int plungequalifiedfile(char *name);  /* explicit path included */
SC_FUNC int plungefile(char *name,int try_currentpath,int try_includepaths);   /* search through "include" paths */
SC_FUNC int number(cell *val,const unsigned char *curptr);
SC_FUNC void preprocess(void);
SC_FUNC void lexinit(void);
SC_FUNC int lex(cell *lexvalue,char **lexsym);
SC_FUNC void lexpush(void);
SC_FUNC void lexclr(int clreol);
SC_FUNC void recstart(void);
SC_FUNC void recstop(void);
SC_FUNC int matchtoken(int token);
SC_FUNC int tokeninfo(cell *val,char **str);
SC_FUNC int needtoken(int token);
SC_FUNC void litadd(cell value);
SC_FUNC void litinsert(cell value,int pos);
SC_FUNC int alphanum(char c);
SC_FUNC int ishex(char c);
SC_FUNC void delete_symbol(symbol *root,symbol *sym);
SC_FUNC void delete_symbols(symbol *root,int level,int delete_labels,int delete_functions);
SC_FUNC int refer_symbol(symbol *entry,symbol *bywhom);
SC_FUNC void markusage(symbol *sym,int usage);
SC_FUNC void markinitialized(symbol *sym,int assignment);
SC_FUNC void clearassignments(int fromlevel);
SC_FUNC void memoizeassignments(int fromlevel,symstate **assignments);
SC_FUNC void restoreassignments(int fromlevel,symstate *assignments);
SC_FUNC void rename_symbol(symbol *sym,const char *newname);
SC_FUNC symbol *findglb(const char *name,int filter);
SC_FUNC symbol *findloc(const char *name);
SC_FUNC symbol *findconst(const char *name,int *cmptag);
SC_FUNC symbol *addsym(const char *name,cell addr,int ident,int vclass,int tag,
                       int usage);
SC_FUNC symbol *addvariable(const char *name,cell addr,int ident,int vclass,int tag,
                            int dim[],int numdim,int idxtag[],int compound);
SC_FUNC int getlabel(void);
SC_FUNC char *itoh(ucell val);

/* function prototypes in SC3.C */
SC_FUNC int check_userop(void (*oper)(void),int tag1,int tag2,int numparam,
                         value *lval,int *resulttag);
SC_FUNC int matchtag(int formaltag,int actualtag,int allowcoerce);
SC_FUNC int checktag(int tags[],int numtags,int exprtag);
SC_FUNC int expression(cell *val,int *tag,symbol **symptr,int chkfuncresult);
SC_FUNC int sc_getstateid(constvalue **automaton,constvalue **state);
SC_FUNC cell array_totalsize(symbol *sym);

/* function prototypes in SC4.C */
SC_FUNC void writeleader(symbol *root);
SC_FUNC void writetrailer(void);
SC_FUNC void begcseg(void);
SC_FUNC void begdseg(void);
SC_FUNC void setline(int chkbounds);
SC_FUNC void setfiledirect(char *name);
SC_FUNC void setfileconst(char *name);
SC_FUNC void setlinedirect(int line);
SC_FUNC void setlineconst(int line);
SC_FUNC void setlabel(int number);
SC_FUNC void markexpr(optmark type,const char *name,cell offset);
SC_FUNC void startfunc(char *fname,int generateproc);
SC_FUNC void endfunc(void);
SC_FUNC void alignframe(int numbytes);
SC_FUNC void rvalue(value *lval);
SC_FUNC void dereference(void);
SC_FUNC void address(symbol *sym,regid reg);
SC_FUNC void store(value *lval);
SC_FUNC void loadreg(cell address,regid reg);
SC_FUNC void storereg(cell address,regid reg);
SC_FUNC void memcopy(cell size);
SC_FUNC void copyarray(symbol *sym,cell size);
SC_FUNC void fillarray(symbol *sym,cell size,cell value);
SC_FUNC void ldconst(cell val,regid reg);
SC_FUNC void moveto1(void);
SC_FUNC void pushreg(regid reg);
SC_FUNC void pushval(cell val);
SC_FUNC void popreg(regid reg);
SC_FUNC void swap1(void);
SC_FUNC void ffswitch(int label);
SC_FUNC void ffcase(cell value,char *labelname,int newtable);
SC_FUNC void ffcall(symbol *sym,const char *label,int numargs);
SC_FUNC void ffret(int remparams);
SC_FUNC void ffabort(int reason);
SC_FUNC void ffbounds(cell size);
SC_FUNC void jumplabel(int number);
SC_FUNC void defstorage(void);
SC_FUNC void defcompactstorage(void);
SC_FUNC void getfrm(void);
SC_FUNC void modstk(int delta);
SC_FUNC void setstk(cell value);
SC_FUNC void modheap(int delta);
SC_FUNC void setheap_pri(void);
SC_FUNC void setheap(cell value);
SC_FUNC void cell2addr(void);
SC_FUNC void cell2addr_alt(void);
SC_FUNC void addr2cell(void);
SC_FUNC void char2addr(void);
SC_FUNC void charalign(void);
SC_FUNC void addconst(cell value);

/*  Code generation functions for arithmetic operators.
 *
 *  Syntax: o[u|s|b]_name
 *          |   |   | +--- name of operator
 *          |   |   +----- underscore
 *          |   +--------- "u"nsigned operator, "s"igned operator or "b"oth
 *          +------------- "o"perator
 */
SC_FUNC void os_mult(void); /* multiplication (signed) */
SC_FUNC void os_div(void);  /* division (signed) */
SC_FUNC void os_mod(void);  /* modulus (signed) */
SC_FUNC void ob_add(void);  /* addition */
SC_FUNC void ob_sub(void);  /* subtraction */
SC_FUNC void ob_sal(void);  /* shift left (arithmetic) */
SC_FUNC void os_sar(void);  /* shift right (arithmetic, signed) */
SC_FUNC void ou_sar(void);  /* shift right (logical, unsigned) */
SC_FUNC void ob_or(void);   /* bitwise or */
SC_FUNC void ob_xor(void);  /* bitwise xor */
SC_FUNC void ob_and(void);  /* bitwise and */
SC_FUNC void ob_eq(void);   /* equality */
SC_FUNC void ob_ne(void);   /* inequality */
SC_FUNC void relop_prefix(void);
SC_FUNC void relop_suffix(void);
SC_FUNC void os_le(void);   /* less or equal (signed) */
SC_FUNC void os_ge(void);   /* greater or equal (signed) */
SC_FUNC void os_lt(void);   /* less (signed) */
SC_FUNC void os_gt(void);   /* greater (signed) */

SC_FUNC void lneg(void);
SC_FUNC void neg(void);
SC_FUNC void invert(void);
SC_FUNC void nooperation(void);
SC_FUNC void inc(value *lval);
SC_FUNC void dec(value *lval);
SC_FUNC void jmp_ne0(int number);
SC_FUNC void jmp_eq0(int number);
SC_FUNC void outval(cell val,int newline);
SC_FUNC void outinstr(const char *name,emit_outval params[],int numparams);

/* function prototypes in SC5.C */
SC_FUNC int error(long number,...);
SC_FUNC void errorset(int code,int line);
SC_FUNC void warnstack_init(void);
SC_FUNC void warnstack_cleanup(void);
SC_FUNC int error_suggest(int number,const char *name,const char *name2,int type,int subtype);

/* function prototypes in SC6.C */
SC_FUNC int assemble(FILE *fout,FILE *fin);

/* function prototypes in SC7.C */
SC_FUNC void stgbuffer_cleanup(void);
SC_FUNC void stgmark(char mark);
SC_FUNC void stgwrite(const char *str);
SC_FUNC void stgout(int index);
SC_FUNC void stgdel(int index,cell code_index);
SC_FUNC int stgget(int *index,cell *code_index);
SC_FUNC void stgset(int onoff);

/* function prototypes in SCLIST.C */
SC_FUNC char* duplicatestring(const char* sourcestring);
SC_FUNC stringpair *insert_alias(char *name,char *alias);
SC_FUNC int lookup_alias(char *target,char *name);
SC_FUNC void delete_aliastable(void);
SC_FUNC stringlist *insert_path(char *path);
SC_FUNC char *get_path(int index);
SC_FUNC void delete_pathtable(void);
SC_FUNC stringpair *insert_subst(char *pattern,char *substitution,int prefixlen);
SC_FUNC stringpair *find_subst(char *name,int length);
SC_FUNC int delete_subst(char *name,int length);
SC_FUNC void delete_substtable(void);
SC_FUNC stringlist *insert_sourcefile(char *string);
SC_FUNC char *get_sourcefile(int index);
SC_FUNC void delete_sourcefiletable(void);
SC_FUNC stringlist *insert_docstring(char *string);
SC_FUNC char *get_docstring(int index);
SC_FUNC void delete_docstring(int index);
SC_FUNC void delete_docstringtable(void);
SC_FUNC stringlist *insert_autolist(char *string);
SC_FUNC char *get_autolist(int index);
SC_FUNC void delete_autolisttable(void);
SC_FUNC valuepair *push_heaplist(long first, long second);
SC_FUNC int popfront_heaplist(long *first, long *second);
SC_FUNC void delete_heaplisttable(void);
SC_FUNC stringlist *insert_dbgfile(const char *filename);
SC_FUNC stringlist *insert_dbgline(int linenr);
SC_FUNC stringlist *insert_dbgsymbol(symbol *sym);
SC_FUNC char *get_dbgstring(int index);
SC_FUNC void delete_dbgstringtable(void);

/* function prototypes in SCMEMFILE.C */
#if !defined tMEMFILE
  typedef unsigned char MEMFILE;
  #define tMEMFILE  1
#endif
MEMFILE *mfcreate(const char *filename);
void mfclose(MEMFILE *mf);
int mfdump(MEMFILE *mf);
long mflength(const MEMFILE *mf);
long mfseek(MEMFILE *mf,long offset,int whence);
unsigned int mfwrite(MEMFILE *mf,const unsigned char *buffer,unsigned int size);
unsigned int mfread(MEMFILE *mf,unsigned char *buffer,unsigned int size);
char *mfgets(MEMFILE *mf,char *string,unsigned int size);
int mfputs(MEMFILE *mf,const char *string);

/* function prototypes in SCI18N.C */
#define MAXCODEPAGE 12
SC_FUNC int cp_path(const char *root,const char *directory);
SC_FUNC int cp_set(const char *name);
SC_FUNC cell cp_translate(const unsigned char *string,const unsigned char **endptr);
SC_FUNC cell get_utf8_char(const unsigned char *string,const unsigned char **endptr);
SC_FUNC int scan_utf8(FILE *fp,const char *filename);

/* function prototypes in SCSTATE.C */
SC_FUNC constvalue *automaton_add(const char *name);
SC_FUNC constvalue *automaton_find(const char *name);
SC_FUNC constvalue *automaton_findid(int id);
SC_FUNC constvalue *state_add(const char *name,int fsa_id);
SC_FUNC constvalue *state_find(const char *name,int fsa_id);
SC_FUNC constvalue *state_findid(int id);
SC_FUNC void state_buildlist(int **list,int *listsize,int *count,int stateid);
SC_FUNC int state_addlist(int *list,int count,int fsa_id);
SC_FUNC void state_deletetable(void);
SC_FUNC int state_getfsa(int listid);
SC_FUNC int state_count(int listid);
SC_FUNC int state_inlist(int listid,int state);
SC_FUNC int state_listitem(int listid,int index);
SC_FUNC void state_conflict(symbol *root);
SC_FUNC int state_conflict_id(int listid1,int listid2);

/* external variables (defined in scvars.c) */
#if !defined SC_SKIP_VDECL
SC_VDECL symbol loctab;       /* local symbol table */
SC_VDECL symbol glbtab;       /* global symbol table */
SC_VDECL struct hashtable_t symbol_cache_ht;
SC_VDECL symbol *line_sym;
SC_VDECL cell *litq;          /* the literal queue */
SC_VDECL unsigned char pline[]; /* the line read from the input file */
SC_VDECL const unsigned char *lptr;/* points to the current position in "pline" */
SC_VDECL constvalue_root tagname_tab;/* tagname table */
SC_VDECL constvalue_root libname_tab;/* library table (#pragma library "..." syntax) */
SC_VDECL constvalue *curlibrary;/* current library */
SC_VDECL int pc_addlibtable;  /* is the library table added to the AMX file? */
SC_VDECL symbol *curfunc;     /* pointer to current function */
SC_VDECL char *inpfname;      /* name of the file currently read from */
SC_VDECL char outfname[];     /* intermediate (assembler) file name */
SC_VDECL char binfname[];     /* binary file name */
SC_VDECL char errfname[];     /* error file name */
SC_VDECL char sc_ctrlchar;    /* the control character (or escape character) */
SC_VDECL char sc_ctrlchar_org;/* the default control character */
SC_VDECL int litidx;          /* index to literal table */
SC_VDECL int litmax;          /* current size of the literal table */
SC_VDECL int litgrow;         /* amount to increase the literal table by */
SC_VDECL int stgidx;          /* index to the staging buffer */
SC_VDECL int sc_labnum;       /* number of (internal) labels */
SC_VDECL int staging;         /* true if staging output */
SC_VDECL cell declared;       /* number of local cells declared */
SC_VDECL cell glb_declared;   /* number of global cells declared */
SC_VDECL cell code_idx;       /* number of bytes with generated code */
SC_VDECL int ntv_funcid;      /* incremental number of native function */
SC_VDECL int errnum;          /* number of errors */
SC_VDECL int warnnum;         /* number of warnings */
SC_VDECL int sc_debug;        /* debug/optimization options (bit field) */
SC_VDECL int sc_packstr;      /* strings are packed by default? */
SC_VDECL int sc_asmfile;      /* create .ASM file? */
SC_VDECL int sc_listing;      /* create .LST file? */
SC_VDECL int sc_compress;     /* compress bytecode? */
SC_VDECL int sc_needsemicolon;/* semicolon required to terminate expressions? */
SC_VDECL int sc_dataalign;    /* data alignment value */
SC_VDECL int sc_alignnext;    /* must frame of the next function be aligned? */
SC_VDECL int pc_docexpr;      /* must expression be attached to documentation comment? */
SC_VDECL int curseg;          /* 1 if currently parsing CODE, 2 if parsing DATA */
SC_VDECL cell pc_stksize;     /* stack size */
SC_VDECL cell pc_amxlimit;    /* abstract machine size limit (code + data, or only code) */
SC_VDECL cell pc_amxram;      /* abstract machine data size limit */
SC_VDECL int freading;        /* is there an input file ready for reading? */
SC_VDECL int fline;           /* the line number in the current file */
SC_VDECL short fnumber;       /* number of files in the file table (debugging) */
SC_VDECL short fcurrent;      /* current file being processed (debugging) */
SC_VDECL short sc_intest;     /* true if inside a test */
SC_VDECL int pc_sideeffect;   /* true if an expression causes a side-effect */
SC_VDECL int pc_ovlassignment;/* true if an expression contains an overloaded assignment */
SC_VDECL int stmtindent;      /* current indent of the statement */
SC_VDECL int indent_nowarn;   /* skip warning "217 loose indentation" */
SC_VDECL int sc_tabsize;      /* number of spaces that a TAB represents */
SC_VDECL short sc_allowtags;  /* allow/detect tagnames in lex() */
SC_VDECL int sc_status;       /* read/write status */
SC_VDECL int sc_rationaltag;  /* tag for rational numbers */
SC_VDECL int rational_digits; /* number of fractional digits */
SC_VDECL int sc_allowproccall;/* allow/detect tagnames in lex() */
SC_VDECL short sc_is_utf8;    /* is this source file in UTF-8 encoding */
SC_VDECL char *pc_deprecate;  /* if non-NULL, mark next declaration as deprecated */
SC_VDECL int sc_curstates;    /* ID of the current state list */
SC_VDECL int pc_optimize;     /* (peephole) optimization level */
SC_VDECL int pc_memflags;     /* special flags for the stack/heap usage */
SC_VDECL int pc_naked;        /* if true mark following function as naked */
SC_VDECL int pc_compat;       /* running in compatibility mode? */
SC_VDECL int pc_recursion;    /* enable detailed recursion report? */
SC_VDECL int pc_retexpr;      /* true if the current expression is a part of a "return" statement */
SC_VDECL int pc_retheap;      /* heap space (in bytes) to be manually freed when returning an array returned by another function */
SC_VDECL int pc_nestlevel;    /* number of active (open) compound statements */
SC_VDECL unsigned int pc_attributes;/* currently set attribute flags (for the "__pragma" operator) */
SC_VDECL int pc_ispackedstr;  /* true if the last tokenized string is packed */
SC_VDECL int pc_isrecording;  /* true if recording input */
SC_VDECL char *pc_recstr;     /* recorded input */
SC_VDECL int pc_loopcond;     /* equals to 'tFOR', 'tWHILE' or 'tDO' if the current expression is a loop condition, zero otherwise */
SC_VDECL int pc_numloopvars;  /* number of variables used inside a loop condition */

SC_VDECL char *sc_tokens[];

SC_VDECL constvalue_root sc_automaton_tab; /* automaton table */
SC_VDECL constvalue_root sc_state_tab;     /* state table */

SC_VDECL FILE *inpf;          /* file read from (source or include) */
SC_VDECL FILE *inpf_org;      /* main source file */
SC_VDECL FILE *outf;          /* file written to */

SC_VDECL jmp_buf errbuf;      /* target of longjmp() on a fatal error */

/*  Possible entries for "emit_flags"
 *  Bits: 0     (efBLOCK) multiline ('()' block) syntax
 *        1     (efEXPR) used within an expression
 *        2     (efGLOBAL) used outside of a function
 */
#define efBLOCK         1
#define efEXPR          2
#define efGLOBAL        4
SC_VDECL int emit_flags;
SC_VDECL int emit_stgbuf_idx;

#if !defined SC_LIGHT
  SC_VDECL int sc_makereport; /* generate a cross-reference report */
#endif

#endif /* SC_SKIP_VDECL */

/* These macros are adapted from LibDGG libdgg-int64.h, see
 * http://www.dennougedougakkai-ndd.org/pub/libdgg/
 */
#if defined(__STDC_VERSION__) && __STDC_VERSION__>=199901L
  #define __STDC_FORMAT_MACROS
  #define __STDC_CONSTANT_MACROS
  #include <inttypes.h>         /* automatically includes stdint.h */
#elif (defined _MSC_VER || defined __BORLANDC__) && (defined _I64_MAX || defined HAVE_I64)
  #define PRId64 "I64d"
  #define PRIx64 "I64x"
#else
  #define PRId64 "lld"
  #define PRIx64 "llx"
#endif
#if PAWN_CELL_SIZE==64
  #define PRIdC  PRId64
  #define PRIxC  PRIx64
#else
  #define PRIdC  "d"
  #define PRIxC  "x"
#endif

#endif /* SC_H_INCLUDED */
