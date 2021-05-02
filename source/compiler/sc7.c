/*  Pawn compiler - Staging buffer and optimizer
 *
 *  The staging buffer
 *  ------------------
 *  The staging buffer allows buffered output of generated code, deletion
 *  of redundant code, optimization by a tinkering process and reversing
 *  the output of evaluated expressions (which is used for the reversed
 *  evaluation of arguments in functions).
 *  Initially, stgwrite() writes to the file directly, but after a call to
 *  stgset(TRUE), output is redirected to the buffer. After a call to
 *  stgset(FALSE), stgwrite()'s output is directed to the file again. Thus
 *  only one routine is used for writing to the output, which can be
 *  buffered output or direct output.
 *
 *  staging buffer variables:   stgbuf  - the buffer
 *                              stgidx  - current index in the staging buffer
 *                              staging - if true, write to the staging buffer;
 *                                        if false, write to file directly.
 *
 * The peephole optimizer uses a dual "pipeline". The staging buffer (described
 * above) gets optimized for each expression or sub-expression in a function
 * call. The peephole optimizer is recursive, but it does not span multiple
 * sub-expressions. However, the data gets written to a second buffer that
 * behaves much like the staging buffer. This second buffer gathers all
 * optimized strings from the staging buffer for a complete expression. The
 * peephole optmizer then runs over this second buffer to find optimzations
 * across function parameter boundaries.
 *
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
 *  Version: $Id: sc7.c 3579 2006-06-06 13:35:29Z thiadmer $
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>     /* for atoi() */
#include <string.h>
#include <ctype.h>
#if defined FORTIFY
  #include <alloc/fortify.h>
#endif
#include "sc.h"

#define seqsize(o,p)    (opcodes(o)+opargs(p))
typedef struct {
  char *find;
  char *replace;
  int savesize;         /* number of bytes saved (in bytecode) */
} SEQUENCE;

static SEQUENCE sequences[] = {
  /* A very common sequence in four varieties
   *    load.s.pri n1           load.s.pri n2
   *    push.pri                load.s.alt n1
   *    load.s.pri n2           -
   *    pop.alt                 -
   *    --------------------------------------
   *    load.pri n1             load.s.pri n2
   *    push.pri                load.alt n1
   *    load.s.pri n2           -
   *    pop.alt                 -
   *    --------------------------------------
   *    load.s.pri n1           load.pri n2
   *    push.pri                load.s.alt n1
   *    load.pri n2             -
   *    pop.alt                 -
   *    --------------------------------------
   *    load.pri n1             load.pri n2
   *    push.pri                load.alt n1
   *    load.pri n2             -
   *    pop.alt                 -
   */
  {
    "load.s.pri %1!push.pri!load.s.pri %2!pop.alt!",
    "load.s.pri %2!load.s.alt %1!",
    seqsize(4,2) - seqsize(2,2)
  },
  {
    "load.pri %1!push.pri!load.s.pri %2!pop.alt!",
    "load.s.pri %2!load.alt %1!",
    seqsize(4,2) - seqsize(2,2)
  },
  {
    "load.s.pri %1!push.pri!load.pri %2!pop.alt!",
    "load.pri %2!load.s.alt %1!",
    seqsize(4,2) - seqsize(2,2)
  },
  {
    "load.pri %1!push.pri!load.pri %2!pop.alt!",
    "load.pri %2!load.alt %1!",
    seqsize(4,2) - seqsize(2,2)
  },
  /* (#1#) The above also occurs with "addr.pri" (array
   * indexing) as the first line; so that adds 2 cases.
   */
  {
    "addr.pri %1!push.pri!load.s.pri %2!pop.alt!",
    "addr.alt %1!load.s.pri %2!",
    seqsize(4,2) - seqsize(2,2)
  },
  {
    "addr.pri %1!push.pri!load.pri %2!pop.alt!",
    "addr.alt %1!load.pri %2!",
    seqsize(4,2) - seqsize(2,2)
  },
  /* And the same sequence with const.pri as either the first
   * or the second load instruction: four more cases.
   */
  {
    "const.pri %1!push.pri!load.s.pri %2!pop.alt!",
    "load.s.pri %2!const.alt %1!",
    seqsize(4,2) - seqsize(2,2)
  },
  {
    "const.pri %1!push.pri!load.pri %2!pop.alt!",
    "load.pri %2!const.alt %1!",
    seqsize(4,2) - seqsize(2,2)
  },
  {
    "load.s.pri %1!push.pri!const.pri %2!pop.alt!",
    "const.pri %2!load.s.alt %1!",
    seqsize(4,2) - seqsize(2,2)
  },
  {
    "load.pri %1!push.pri!const.pri %2!pop.alt!",
    "const.pri %2!load.alt %1!",
    seqsize(4,2) - seqsize(2,2)
  },
  /* The same as above, but now with "addr.pri" (array
   * indexing) on the first line and const.pri on
   * the second.
   */
  {
    "addr.pri %1!push.pri!const.pri %2!pop.alt!",
    "addr.alt %1!const.pri %2!",
    seqsize(4,2) - seqsize(2,2)
  },
  {
    "addr.pri %1!push.pri!zero.pri!pop.alt!",
    "addr.alt %1!zero.pri!",
    seqsize(4,1) - seqsize(2,1)
  },
  /* ??? add references */
  /* Chained relational operators can contain sequences like:
   *    move.pri                load.s.pri n1
   *    push.pri                -
   *    load.s.pri n1           -
   *    pop.alt                 -
   * The above also occurs for "load.pri" and for "const.pri",
   * so add another two cases.
   */
  {
    "move.pri!push.pri!load.s.pri %1!pop.alt!",
    "load.s.pri %1!",
    seqsize(4,1) - seqsize(1,1)
  },
  {
    "move.pri!push.pri!load.pri %1!pop.alt!",
    "load.pri %1!",
    seqsize(4,1) - seqsize(1,1)
  },
  {
    "move.pri!push.pri!const.pri %1!pop.alt!",
    "const.pri %1!",
    seqsize(4,1) - seqsize(1,1)
  },
  /* More optimizations for chained relational operators; the
   * continuation sequences can be simplified if they turn out
   * to be termination sequences:
   *    xchg                    sless       also for sless, sgeq and sleq
   *    sgrtr                   pop.alt
   *    swap.alt                and
   *    and                     ;$exp
   *    pop.alt                 -
   *    ;$exp                   -
   *    --------------------------------------
   *    xchg                    sless       also for sless, sgeq and sleq
   *    sgrtr                   pop.alt
   *    swap.alt                and
   *    and                     jzer n1
   *    pop.alt                 -
   *    jzer n1                 -
   *    --------------------------------------
   *    xchg                    jsgeq  n1   also for sless, sgeq and sleq
   *    sgrtr                   ;$exp       (occurs for non-chained comparisons)
   *    jzer n1                 -
   *    ;$exp                   -
   *    --------------------------------------
   *    xchg                    sless       also for sless, sgeq and sleq
   *    sgrtr                   ;$exp       (occurs for non-chained comparisons)
   *    ;$exp                   -
   */
  {
    "xchg!sgrtr!swap.alt!and!pop.alt!;$exp!",
    "sless!pop.alt!and!;$exp!",
    seqsize(5,0) - seqsize(3,0)
  },
  {
    "xchg!sless!swap.alt!and!pop.alt!;$exp!",
    "sgrtr!pop.alt!and!;$exp!",
    seqsize(5,0) - seqsize(3,0)
  },
  {
    "xchg!sgeq!swap.alt!and!pop.alt!;$exp!",
    "sleq!pop.alt!and!;$exp!",
    seqsize(5,0) - seqsize(3,0)
  },
  {
    "xchg!sleq!swap.alt!and!pop.alt!;$exp!",
    "sgeq!pop.alt!and!;$exp!",
    seqsize(5,0) - seqsize(3,0)
  },
  {
    "xchg!sgrtr!swap.alt!and!pop.alt!jzer %1!",
    "sless!pop.alt!and!jzer %1!",
    seqsize(5,0) - seqsize(3,0)
  },
  {
    "xchg!sless!swap.alt!and!pop.alt!jzer %1!",
    "sgrtr!pop.alt!and!jzer %1!",
    seqsize(5,0) - seqsize(3,0)
  },
  {
    "xchg!sgeq!swap.alt!and!pop.alt!jzer %1!",
    "sleq!pop.alt!and!jzer %1!",
    seqsize(5,0) - seqsize(3,0)
  },
  {
    "xchg!sleq!swap.alt!and!pop.alt!jzer %1!",
    "sgeq!pop.alt!and!jzer %1!",
    seqsize(5,0) - seqsize(3,0)
  },
  {
    "xchg!sgrtr!jzer %1!;$exp!",
    "jsgeq %1!;$exp!",
    seqsize(3,1) - seqsize(1,1)
  },
  {
    "xchg!sless!jzer %1!;$exp!",
    "jsleq %1!;$exp!",
    seqsize(3,1) - seqsize(1,1)
  },
  {
    "xchg!sgeq!jzer %1!;$exp!",
    "jsgrtr %1!;$exp!",
    seqsize(3,1) - seqsize(1,1)
  },
  {
    "xchg!sleq!jzer %1!;$exp!",
    "jsless %1!;$exp!",
    seqsize(3,1) - seqsize(1,1)
  },
  {
    "xchg!sgrtr!;$exp!",
    "sless!;$exp!",
    seqsize(2,0) - seqsize(1,0)
  },
  {
    "xchg!sless!;$exp!",
    "sgrtr!;$exp!",
    seqsize(2,0) - seqsize(1,0)
  },
  {
    "xchg!sgeq!;$exp!",
    "sleq!;$exp!",
    seqsize(2,0) - seqsize(1,0)
  },
  {
    "xchg!sleq!;$exp!",
    "sgeq!;$exp!",
    seqsize(2,0) - seqsize(1,0)
  },
  /* The entry to chained operators is also opt to optimization
   *    load.s.pri n1           load.s.pri n2
   *    load.s.alt n2           load.s.alt n1
   *    xchg                    -
   *    --------------------------------------
   *    load.s.pri n1           load.pri n2
   *    load.alt n2             load.s.alt n1
   *    xchg                    -
   *    --------------------------------------
   *    load.s.pri n1           const.pri n2
   *    const.alt n2            load.s.alt n1
   *    xchg                    -
   *    --------------------------------------
   * and all permutations...
   */
  {
    "load.s.pri %1!load.s.alt %2!xchg!",
    "load.s.pri %2!load.s.alt %1!",
    seqsize(3,2) - seqsize(2,2)
  },
  {
    "load.s.pri %1!load.alt %2!xchg!",
    "load.pri %2!load.s.alt %1!",
    seqsize(3,2) - seqsize(2,2)
  },
  {
    "load.s.pri %1!const.alt %2!xchg!",
    "const.pri %2!load.s.alt %1!",
    seqsize(3,2) - seqsize(2,2)
  },
  {
    "load.pri %1!load.s.alt %2!xchg!",
    "load.s.pri %2!load.alt %1!",
    seqsize(3,2) - seqsize(2,2)
  },
  {
    "load.pri %1!load.alt %2!xchg!",
    "load.pri %2!load.alt %1!",
    seqsize(3,2) - seqsize(2,2)
  },
  {
    "load.pri %1!const.alt %2!xchg!",
    "const.pri %2!load.alt %1!",
    seqsize(3,2) - seqsize(2,2)
  },
  {
    "const.pri %1!load.s.alt %2!xchg!",
    "load.s.pri %2!const.alt %1!",
    seqsize(3,2) - seqsize(2,2)
  },
  {
    "const.pri %1!load.alt %2!xchg!",
    "load.pri %2!const.alt %1!",
    seqsize(3,2) - seqsize(2,2)
  },
  /* some sequences where PRI is moved to ALT can be optimized
   * further when considering what follows
   *    move.alt                const.alt n1
   *    const.pri %1            -
   *    xchg                    -
   * (also for load.s.pri and load.pri)
   *    --------------------------------------
   *    lref.pri %1             lref.alt %1
   *    move.alt                [load.pri %2]
   *    [load.pri %2]           -
   * (where [load.pri %2] may also be another operatrion loading PRI)
   */
  {
    "move.alt!const.pri %1!xchg!",
    "const.alt %1!",
    seqsize(3,1) - seqsize(1,1)
  },
  {
    "move.alt!load.pri %1!xchg!",
    "load.alt %1!",
    seqsize(3,1) - seqsize(1,1)
  },
  {
    "move.alt!load.s.pri %1!xchg!",
    "load.s.alt %1!",
    seqsize(3,1) - seqsize(1,1)
  },
  /* ----- */
  {
    "lref.pri %1!move.alt!load.pri %2!",
    "lref.alt %1!load.pri %2!",
    seqsize(3,2) - seqsize(2,2)
  },
  {
    "lref.pri %1!move.alt!load.s.pri %2!",
    "lref.alt %1!load.s.pri %2!",
    seqsize(3,2) - seqsize(2,2)
  },
  {
    "lref.pri %1!move.alt!const.pri %2!",
    "lref.alt %1!const.pri %2!",
    seqsize(3,2) - seqsize(2,2)
  },
  {
    "lref.s.pri %1!move.alt!load.pri %2!",
    "lref.s.alt %1!load.pri %2!",
    seqsize(3,2) - seqsize(2,2)
  },
  {
    "lref.s.pri %1!move.alt!load.s.pri %2!",
    "lref.s.alt %1!load.s.pri %2!",
    seqsize(3,2) - seqsize(2,2)
  },
  {
    "lref.s.pri %1!move.alt!const.pri %2!",
    "lref.s.alt %1!const.pri %2!",
    seqsize(3,2) - seqsize(2,2)
  },
  /* Array indexing can merit from special instructions.
   * Simple indexed array lookup can be optimized quite
   * a bit.
   *    addr.pri n1             addr.alt n1
   *    push.pri                load.s.pri n2
   *    load.s.pri n2           bounds n3
   *    bounds n3               lidx.b n4
   *    shl.c.pri n4            -
   *    pop.alt                 -
   *    add                     -
   *    load.i                  -
   *
   * And to prepare for storing a value in an array
   *    addr.pri n1             addr.alt n1
   *    push.pri                load.s.pri n2
   *    load.s.pri n2           bounds n3
   *    bounds n3               idxaddr.b n4
   *    shl.c.pri n4            -
   *    pop.alt                 -
   *    add                     -
   *
   * Notes (additional cases):
   * 1. instruction addr.pri can also be const.pri (for
   *    global arrays)
   * 2. the bounds instruction can be absent
   * 3. when "n4" (the shift value) is the 2 (with 32-bit cells), use the
   *    even more optimal instructions LIDX and IDDXADDR
   *
   * If the array index is more complex, one can only optimize
   * the last four instructions:
   *    shl.c.pri n1            pop.alt
   *    pop.alt                 lidx.b n1
   *    add                     -
   *    loadi                   -
   *    --------------------------------------
   *    shl.c.pri n1            pop.alt
   *    pop.alt                 idxaddr.b n1
   *    add                     -
   */
#if !defined BIT16
  /* loading from array, "cell" shifted */
  {
    "addr.pri %1!push.pri!load.s.pri %2!bounds %3!shl.c.pri 2!pop.alt!add!load.i!",
    "addr.alt %1!load.s.pri %2!bounds %3!lidx!",
    seqsize(8,4) - seqsize(4,3)
  },
  {
    "const.pri %1!push.pri!load.s.pri %2!bounds %3!shl.c.pri 2!pop.alt!add!load.i!",
    "const.alt %1!load.s.pri %2!bounds %3!lidx!",
    seqsize(8,4) - seqsize(4,3)
  },
  {
    "addr.pri %1!push.pri!load.s.pri %2!shl.c.pri 2!pop.alt!add!load.i!",
    "addr.alt %1!load.s.pri %2!lidx!",
    seqsize(7,3) - seqsize(3,2)
  },
  {
    "const.pri %1!push.pri!load.s.pri %2!shl.c.pri 2!pop.alt!add!load.i!",
    "const.alt %1!load.s.pri %2!lidx!",
    seqsize(7,3) - seqsize(3,2)
  },
#endif
  /* loading from array, not "cell" shifted */
  {
    "addr.pri %1!push.pri!load.s.pri %2!bounds %3!shl.c.pri %4!pop.alt!add!load.i!",
    "addr.alt %1!load.s.pri %2!bounds %3!lidx.b %4!",
    seqsize(8,4) - seqsize(4,4)
  },
  {
    "const.pri %1!push.pri!load.s.pri %2!bounds %3!shl.c.pri %4!pop.alt!add!load.i!",
    "const.alt %1!load.s.pri %2!bounds %3!lidx.b %4!",
    seqsize(8,4) - seqsize(4,4)
  },
  {
    "addr.pri %1!push.pri!load.s.pri %2!shl.c.pri %3!pop.alt!add!load.i!",
    "addr.alt %1!load.s.pri %2!lidx.b %3!",
    seqsize(7,3) - seqsize(3,3)
  },
  {
    "const.pri %1!push.pri!load.s.pri %2!shl.c.pri %3!pop.alt!add!load.i!",
    "const.alt %1!load.s.pri %2!lidx.b %3!",
    seqsize(7,3) - seqsize(3,3)
  },
#if !defined BIT16
  /* array index calculation for storing a value, "cell" aligned */
  {
    "addr.pri %1!push.pri!load.s.pri %2!bounds %3!shl.c.pri 2!pop.alt!add!",
    "addr.alt %1!load.s.pri %2!bounds %3!idxaddr!",
    seqsize(7,4) - seqsize(4,3)
  },
  {
    "const.pri %1!push.pri!load.s.pri %2!bounds %3!shl.c.pri 2!pop.alt!add!",
    "const.alt %1!load.s.pri %2!bounds %3!idxaddr!",
    seqsize(7,4) - seqsize(4,3)
  },
  {
    "addr.pri %1!push.pri!load.s.pri %2!shl.c.pri 2!pop.alt!add!",
    "addr.alt %1!load.s.pri %2!idxaddr!",
    seqsize(6,3) - seqsize(3,2)
  },
  {
    "const.pri %1!push.pri!load.s.pri %2!shl.c.pri 2!pop.alt!add!",
    "const.alt %1!load.s.pri %2!idxaddr!",
    seqsize(6,3) - seqsize(3,2)
  },
#endif
  /* array index calculation for storing a value, not "cell" packed */
  {
    "addr.pri %1!push.pri!load.s.pri %2!bounds %3!shl.c.pri %4!pop.alt!add!",
    "addr.alt %1!load.s.pri %2!bounds %3!idxaddr.b %4!",
    seqsize(7,4) - seqsize(4,4)
  },
  {
    "const.pri %1!push.pri!load.s.pri %2!bounds %3!shl.c.pri %4!pop.alt!add!",
    "const.alt %1!load.s.pri %2!bounds %3!idxaddr.b %4!",
    seqsize(7,4) - seqsize(4,4)
  },
  {
    "addr.pri %1!push.pri!load.s.pri %2!shl.c.pri %3!pop.alt!add!",
    "addr.alt %1!load.s.pri %2!idxaddr.b %3!",
    seqsize(6,3) - seqsize(3,3)
  },
  {
    "const.pri %1!push.pri!load.s.pri %2!shl.c.pri %3!pop.alt!add!",
    "const.alt %1!load.s.pri %2!idxaddr.b %3!",
    seqsize(6,3) - seqsize(3,3)
  },
#if !defined BIT16
  /* the shorter array indexing sequences, see above for comments */
  {
    "shl.c.pri 2!pop.alt!add!loadi!",
    "pop.alt!lidx!",
    seqsize(4,1) - seqsize(2,0)
  },
  {
    "shl.c.pri 2!pop.alt!add!",
    "pop.alt!idxaddr!",
    seqsize(3,1) - seqsize(2,0)
  },
#endif
  {
    "shl.c.pri %1!pop.alt!add!loadi!",
    "pop.alt!lidx.b %1!",
    seqsize(4,1) - seqsize(2,1)
  },
  {
    "shl.c.pri %1!pop.alt!add!",
    "pop.alt!idxaddr.b %1!",
    seqsize(3,1) - seqsize(2,1)
  },
  /* For packed arrays, there is another case (packed arrays
   * do not take advantage of the LIDX or IDXADDR instructions).
   *    addr.pri n1             addr.alt n1
   *    push.pri                load.s.pri n2
   *    load.s.pri n2           bounds n3
   *    bounds n3               -
   *    pop.alt                 -
   *
   * Notes (additional cases):
   * 1. instruction addr.pri can also be const.pri (for
   *    global arrays)
   * 2. the bounds instruction can be absent, but that
   *    case is already handled (see #1#)
   */
  {
    "addr.pri %1!push.pri!load.s.pri %2!bounds %3!pop.alt!",
    "addr.alt %1!load.s.pri %2!bounds %3!",
    seqsize(5,3) - seqsize(3,3)
  },
  {
    "const.pri %1!push.pri!load.s.pri %2!bounds %3!pop.alt!",
    "const.alt %1!load.s.pri %2!bounds %3!",
    seqsize(5,3) - seqsize(3,3)
  },
  /* Declaration of simple variables often follows the sequence:
   *    ;$lcl <name> <stk>      ;$lcl <name> <stk>
   *    stack -4                push.c <constval>
   *    const.pri <constval>    ;$exp
   *    stor.s.pri <stk>        -
   *    ;$exp                   -
   */
  {
    ";$lcl %1 %2!stack -4!const.pri %3!stor.s.pri %2!;$exp!",
    ";$lcl %1 %2!push.c %3!;$exp!",
    seqsize(3,3) - seqsize(1,1)
  },
  {
    ";$lcl %1 %2!stack -4!zero.pri!stor.s.pri %2!;$exp!",
    ";$lcl %1 %2!push.c 0!;$exp!",
    seqsize(3,2) - seqsize(1,1)
  },
  /* During a calculation, the intermediate result must sometimes
   * be moved from PRI to ALT, like in:
   *    push.pri                move.alt
   *    load.s.pri n1           load.s.pri n1
   *    pop.alt                 -
   *
   * The above also occurs for "load.pri" and for "const.pri",
   * so add another two cases.
   */
  {
    "push.pri!load.s.pri %1!pop.alt!",
    "move.alt!load.s.pri %1!",
    seqsize(3,1) - seqsize(2,1)
  },
  {
    "push.pri!load.pri %1!pop.alt!",
    "move.alt!load.pri %1!",
    seqsize(3,1) - seqsize(2,1)
  },
  {
    "push.pri!const.pri %1!pop.alt!",
    "move.alt!const.pri %1!",
    seqsize(3,1) - seqsize(2,1)
  },
  {
    "push.pri!zero.pri!pop.alt!",
    "move.alt!zero.pri!",
    seqsize(3,0) - seqsize(2,0)
  },
  /* saving PRI and then loading from its address
   * occurs when indexing a multi-dimensional array
   */
  {
    "push.pri!load.i!pop.alt!",
    "move.alt!load.i!",
    seqsize(3,0) - seqsize(2,0)
  },
  /* An even simpler PUSH/POP optimization (occurs in
   * switch statements):
   *    push.pri                move.alt
   *    pop.alt                 -
   */
  {
    "push.pri!pop.alt!",
    "move.alt!",
    seqsize(2,0) - seqsize(1,0)
  },
  /* Some simple arithmetic sequences
   */
  {
    "move.alt!load.s.pri %1!add!",
    "load.s.alt %1!add!",
    seqsize(3,1) - seqsize(2,1)
  },
  {
    "move.alt!load.pri %1!add!",
    "load.alt %1!add!",
    seqsize(3,1) - seqsize(2,1)
  },
  {
    "move.alt!const.pri %1!add!",
    "const.alt %1!add!",
    seqsize(3,1) - seqsize(2,1)
  },
  {
    "move.alt!load.s.pri %1!sub.alt!",
    "load.s.alt %1!sub!",
    seqsize(3,1) - seqsize(2,1)
  },
  {
    "move.alt!load.pri %1!sub.alt!",
    "load.alt %1!sub!",
    seqsize(3,1) - seqsize(2,1)
  },
  {
    "move.alt!const.pri %1!sub.alt!",
    "const.alt %1!sub!",
    seqsize(3,1) - seqsize(2,1)
  },
  /* User-defined operators first load the operands into registers and
   * then have them pushed onto the stack. This can give rise to sequences
   * like:
   *    const.pri n1            push.c n1
   *    const.alt n2            push.c n2
   *    push.pri                -
   *    push.alt                -
   * A similar sequence occurs with the two PUSH.pri/alt instructions inverted.
   * The first, second, or both CONST.pri/alt instructions can also be
   * LOAD.pri/alt.
   * This gives 2 x 4 cases.
   */
  {
    "const.pri %1!const.alt %2!push.pri!push.alt!",
    "push.c %1!push.c %2!",
    seqsize(4,2) - seqsize(2,2)
  },
  {
    "const.pri %1!const.alt %2!push.alt!push.pri!",
    "push.c %2!push.c %1!",
    seqsize(4,2) - seqsize(2,2)
  },
  {
    "const.pri %1!load.alt %2!push.pri!push.alt!",
    "push.c %1!push %2!",
    seqsize(4,2) - seqsize(2,2)
  },
  {
    "const.pri %1!load.alt %2!push.alt!push.pri!",
    "push %2!push.c %1!",
    seqsize(4,2) - seqsize(2,2)
  },
  {
    "load.pri %1!const.alt %2!push.pri!push.alt!",
    "push %1!push.c %2!",
    seqsize(4,2) - seqsize(2,2)
  },
  {
    "load.pri %1!const.alt %2!push.alt!push.pri!",
    "push.c %2!push %1!",
    seqsize(4,2) - seqsize(2,2)
  },
  {
    "load.pri %1!load.alt %2!push.pri!push.alt!",
    "push %1!push %2!",
    seqsize(4,2) - seqsize(2,2)
  },
  {
    "load.pri %1!load.alt %2!push.alt!push.pri!",
    "push %2!push %1!",
    seqsize(4,2) - seqsize(2,2)
  },
  /* Function calls (parameters are passed on the stack)
   *    load.s.pri n1           push.s n1
   *    push.pri                -
   *    --------------------------------------
   *    load.pri n1             push n1
   *    push.pri                -
   *    --------------------------------------
   *    const.pri n1            push.c n1
   *    push.pri                -
   *    --------------------------------------
   *    zero.pri                push.c 0
   *    push.pri                -
   *    --------------------------------------
   *    addr.pri n1             push.adr n1
   *    push.pri                -
   *
   * However, PRI must not be needed after this instruction
   * if this shortcut is used. Check for the ;$par comment.
   */
  {
    "load.s.pri %1!push.pri!;$par!",
    "push.s %1!;$par!",
    seqsize(2,1) - seqsize(1,1)
  },
  {
    "load.pri %1!push.pri!;$par!",
    "push %1!;$par!",
    seqsize(2,1) - seqsize(1,1)
  },
  {
    "const.pri %1!push.pri!;$par!",
    "push.c %1!;$par!",
    seqsize(2,1) - seqsize(1,1)
  },
  {
    "zero.pri!push.pri!;$par!",
    "push.c 0!;$par!",
    seqsize(2,0) - seqsize(1,1)
  },
  {
    "addr.pri %1!push.pri!;$par!",
    "push.adr %1!;$par!",
    seqsize(2,1) - seqsize(1,1)
  },
  /* References with a default value generate new cells on the heap
   * dynamically. That code often ends with:
   *    move.pri                push.alt
   *    push.pri                -
   */
  {
    "move.pri!push.pri!",
    "push.alt!",
    seqsize(2,0) - seqsize(1,0)
  },
  /* Simple arithmetic operations on constants. Noteworthy is the
   * subtraction of a constant, since it is converted to the addition
   * of the inverse value.
   *    const.alt n1            add.c n1
   *    add                     -
   *    --------------------------------------
   *    const.alt n1            add.c -n1
   *    sub                     -
   *    --------------------------------------
   *    const.alt n1            smul.c n1
   *    smul                    -
   *    --------------------------------------
   *    const.alt n1            eq.c.pri n1
   *    eq                      -
   */
  {
    "const.alt %1!add!",
    "add.c %1!",
    seqsize(2,1) - seqsize(1,1)
  },
  {
    "const.alt %1!smul!",
    "smul.c %1!",
    seqsize(2,1) - seqsize(1,1)
  },
  {
    "const.alt %1!eq!",
    "eq.c.pri %1!",
    seqsize(2,1) - seqsize(1,1)
  },
  /* Subtraction of a constant. Note that the subtraction is converted to
   * the addition of the inverse value.
   *    const.pri n1            load.s.pri n2
   *    load.s.alt n2           add.c -n1
   *    sub                     -
   *    --------------------------------------
   *    const.pri n1            load.pri n2
   *    load.alt n2             add.c -n1
   *    sub                     -
   */
  {
    "const.pri %1!load.s.alt %2!sub!",
    "load.s.pri %2!add.c -%1!",
    seqsize(3,2) - seqsize(2,2)
  },
  {
    "const.pri %1!load.alt %2!sub.alt!",
    "load.pri %2!add.c -%1!",
    seqsize(3,2) - seqsize(2,2)
  },
  /* With arrays indexed with constants that come from enumerations, it happens
   * multiple add.c opcodes follow in sequence.
   *    add.c n1                add.c n1+n2
   *    add.c n2                -
   */
  {
    "add.c %1!add.c %2!",
    "add.c %1+%2!",
    seqsize(2,2) - seqsize(1,1)
  },
  /* Compare and jump
   *    eq                      jneq n1
   *    jzer n1                 -
   *    --------------------------------------
   *    eq                      jeq n1
   *    jnz n1                  -
   *    --------------------------------------
   *    neq                     jeq n1
   *    jzer n1                 -
   *    --------------------------------------
   *    neq                     jneq n1
   *    jnz n1                  -
   * An similarly for other relations
   *    sless                   jsgeq n1
   *    jzer n1                 -
   *    --------------------------------------
   *    sless                   jsless n1
   *    jnz n1                  -
   *    --------------------------------------
   *    sleq                    jsgrtr n1
   *    jzer n1                 -
   *    --------------------------------------
   *    sleq                    jsleq n1
   *    jnz n1                  -
   *    --------------------------------------
   *    sgrtr                   jsleq n1
   *    jzer n1                 -
   *    --------------------------------------
   *    sgrtr                   jsgrtr n1
   *    jnz n1                  -
   *    --------------------------------------
   *    sgeq                    jsless n1
   *    jzer n1                 -
   *    --------------------------------------
   *    sgeq                    jsgeq n1
   *    jnz n1                  -
   * We can relax the optimizations for the unsigned comparisons,
   * because the Pawn compiler currently only generates signed
   * comparisons.
   */
  {
    "eq!jzer %1!",
    "jneq %1!",
    seqsize(2,1) - seqsize(1,1)
  },
  {
    "eq!jnz %1!",
    "jeq %1!",
    seqsize(2,1) - seqsize(1,1)
  },
  {
    "neq!jzer %1!",
    "jeq %1!",
    seqsize(2,1) - seqsize(1,1)
  },
  {
    "neq!jnz %1!",
    "jneq %1!",
    seqsize(2,1) - seqsize(1,1)
  },
  {
    "sless!jzer %1!",
    "jsgeq %1!",
    seqsize(2,1) - seqsize(1,1)
  },
  {
    "sless!jnz %1!",
    "jsless %1!",
    seqsize(2,1) - seqsize(1,1)
  },
  {
    "sleq!jzer %1!",
    "jsgrtr %1!",
    seqsize(2,1) - seqsize(1,1)
  },
  {
    "sleq!jnz %1!",
    "jsleq %1!",
    seqsize(2,1) - seqsize(1,1)
  },
  {
    "sgrtr!jzer %1!",
    "jsleq %1!",
    seqsize(2,1) - seqsize(1,1)
  },
  {
    "sgrtr!jnz %1!",
    "jsgrtr %1!",
    seqsize(2,1) - seqsize(1,1)
  },
  {
    "sgeq!jzer %1!",
    "jsless %1!",
    seqsize(2,1) - seqsize(1,1)
  },
  {
    "sgeq!jnz %1!",
    "jsgeq %1!",
    seqsize(2,1) - seqsize(1,1)
  },
  /* Test for zero (common case, especially for strings)
   * E.g. the test expression of: "for (i=0; str{i}!=0; ++i)"
   *
   *    zero.alt                jzer n1
   *    jeq n1                  -
   *    --------------------------------------
   *    zero.alt                jnz n1
   *    jneq n1                 -
   */
  {
    "zero.alt!jeq %1!",
    "jzer %1!",
    seqsize(2,1) - seqsize(1,1)
  },
  {
    "zero.alt!jneq %1!",
    "jnz %1!",
    seqsize(2,1) - seqsize(1,1)
  },
  /* Incrementing and decrementing leaves a value in
   * in PRI which may not be used (for example, as the
   * third expression in a "for" loop).
   *    inc n1                  inc n1  ; ++n
   *    load.pri n1             ;$exp
   *    ;$exp                   -
   *    --------------------------------------
   *    load.pri n1             inc n1  ; n++, e.g. "for (n=0; n<10; n++)"
   *    inc n1                  ;$exp
   *    ;$exp                   -
   * Plus the varieties for stack relative increments
   * and decrements.
   */
  {
    "inc %1!load.pri %1!;$exp!",
    "inc %1!;$exp!",
    seqsize(2,2) - seqsize(1,1)
  },
  {
    "load.pri %1!inc %1!;$exp!",
    "inc %1!;$exp!",
    seqsize(2,2) - seqsize(1,1)
  },
  {
    "inc.s %1!load.s.pri %1!;$exp!",
    "inc.s %1!;$exp!",
    seqsize(2,2) - seqsize(1,1)
  },
  {
    "load.s.pri %1!inc.s %1!;$exp!",
    "inc.s %1!;$exp!",
    seqsize(2,2) - seqsize(1,1)
  },
  {
    "dec %1!load.pri %1!;$exp!",
    "dec %1!;$exp!",
    seqsize(2,2) - seqsize(1,1)
  },
  {
    "load.pri %1!dec %1!;$exp!",
    "dec %1!;$exp!",
    seqsize(2,2) - seqsize(1,1)
  },
  {
    "dec.s %1!load.s.pri %1!;$exp!",
    "dec.s %1!;$exp!",
    seqsize(2,2) - seqsize(1,1)
  },
  {
    "load.s.pri %1!dec.s %1!;$exp!",
    "dec.s %1!;$exp!",
    seqsize(2,2) - seqsize(1,1)
  },
  /* ??? the same (increments and decrements) for references */
  /* Loading the constant zero has a special opcode.
   * When storing zero in memory, the value of PRI must not be later on.
   *    const.pri 0             zero n1
   *    stor.pri n1             ;$exp
   *    ;$exp                   -
   *    --------------------------------------
   *    const.pri 0             zero.s n1
   *    stor.s.pri n1           ;$exp
   *    ;$exp                   -
   *    --------------------------------------
   *    zero.pri                zero n1
   *    stor.pri n1             ;$exp
   *    ;$exp                   -
   *    --------------------------------------
   *    zero.pri                zero.s n1
   *    stor.s.pri n1           ;$exp
   *    ;$exp                   -
   *    --------------------------------------
   *    const.pri 0             zero.pri
   *    --------------------------------------
   *    const.alt 0             zero.alt
   * The last two alternatives save more memory than they save
   * time, but anyway...
   */
  {
    "const.pri 0!stor.pri %1!;$exp!",
    "zero %1!;$exp!",
    seqsize(2,2) - seqsize(1,1)
  },
  {
    "const.pri 0!stor.s.pri %1!;$exp!",
    "zero.s %1!;$exp!",
    seqsize(2,2) - seqsize(1,1)
  },
  {
    "zero.pri!stor.pri %1!;$exp!",
    "zero %1!;$exp!",
    seqsize(2,1) - seqsize(1,1)
  },
  {
    "zero.pri!stor.s.pri %1!;$exp!",
    "zero.s %1!;$exp!",
    seqsize(2,1) - seqsize(1,1)
  },
  {
    "const.pri 0!",
    "zero.pri!",
    seqsize(1,1) - seqsize(1,0)
  },
  {
    "const.alt 0!",
    "zero.alt!",
    seqsize(1,1) - seqsize(1,0)
  },

  /* ------------------ */
  /* Macro instructions */
  /* ------------------ */

  { "", "", 0 },    /* separator, so optimizer can stop before generating macro opcodes */

  /* optimizing the calling of native functions (which always have a parameter
   * count pushed before, and the stack pointer restored afterwards
   */
  {
    "push.c %1!sysreq.c %2!stack %3!",        /* note: %3 == %1 + 4 */
    "sysreq.n %2 %1!",
    seqsize(3,3) - seqsize(1,2)
  },
  /* ----- */
  /* Functions with many parameters with the same "type" have sequences like:
   *    push.c n1               push3.c n1 n2 n3
   *    ;$par                   ;$par
   *    push.c n2               -
   *    ;$par                   -
   *    push.c n3               -
   *    ;$par                   -
   *    etc.                    etc.
   *
   * Similar sequences occur with PUSH, PUSH.s and PUSHADDR
   */
  {
    "push.c %1!;$par!push.c %2!;$par!push.c %3!;$par!push.c %4!;$par!push.c %5!;$par!",
    "push5.c %1 %2 %3 %4 %5!",
    seqsize(5,5) - seqsize(1,5)
  },
  {
    "push.c %1!;$par!push.c %2!;$par!push.c %3!;$par!push.c %4!;$par!",
    "push4.c %1 %2 %3 %4!",
    seqsize(4,4) - seqsize(1,4)
  },
  {
    "push.c %1!;$par!push.c %2!;$par!push.c %3!;$par!",
    "push3.c %1 %2 %3!",
    seqsize(3,3) - seqsize(1,3)
  },
  {
    "push.c %1!;$par!push.c %2!;$par!",
    "push2.c %1 %2!",
    seqsize(2,2) - seqsize(1,2)
  },
  /* ----- */
  {
    "push %1!;$par!push %2!;$par!push %3!;$par!push %4!;$par!push %5!;$par!",
    "push5 %1 %2 %3 %4 %5!",
    seqsize(5,5) - seqsize(1,5)
  },
  {
    "push %1!;$par!push %2!;$par!push %3!;$par!push %4!;$par!",
    "push4 %1 %2 %3 %4!",
    seqsize(4,4) - seqsize(1,4)
  },
  {
    "push %1!;$par!push %2!;$par!push %3!;$par!",
    "push3 %1 %2 %3!",
    seqsize(3,3) - seqsize(1,3)
  },
  {
    "push %1!;$par!push %2!;$par!",
    "push2 %1 %2!",
    seqsize(2,2) - seqsize(1,2)
  },
  /* ----- */
  {
    "push.s %1!;$par!push.s %2!;$par!push.s %3!;$par!push.s %4!;$par!push.s %5!;$par!",
    "push5.s %1 %2 %3 %4 %5!",
    seqsize(5,5) - seqsize(1,5)
  },
  {
    "push.s %1!;$par!push.s %2!;$par!push.s %3!;$par!push.s %4!;$par!",
    "push4.s %1 %2 %3 %4!",
    seqsize(4,4) - seqsize(1,4)
  },
  {
    "push.s %1!;$par!push.s %2!;$par!push.s %3!;$par!",
    "push3.s %1 %2 %3!",
    seqsize(3,3) - seqsize(1,3)
  },
  {
    "push.s %1!;$par!push.s %2!;$par!",
    "push2.s %1 %2!",
    seqsize(2,2) - seqsize(1,2)
  },
  /* ----- */
  {
    "push.adr %1!;$par!push.adr %2!;$par!push.adr %3!;$par!push.adr %4!;$par!push.adr %5!;$par!",
    "push5.adr %1 %2 %3 %4 %5!",
    seqsize(5,5) - seqsize(1,5)
  },
  {
    "push.adr %1!;$par!push.adr %2!;$par!push.adr %3!;$par!push.adr %4!;$par!",
    "push4.adr %1 %2 %3 %4!",
    seqsize(4,4) - seqsize(1,4)
  },
  {
    "push.adr %1!;$par!push.adr %2!;$par!push.adr %3!;$par!",
    "push3.adr %1 %2 %3!",
    seqsize(3,3) - seqsize(1,3)
  },
  {
    "push.adr %1!;$par!push.adr %2!;$par!",
    "push2.adr %1 %2!",
    seqsize(2,2) - seqsize(1,2)
  },
  /* Loading two registers at a time
   *    load.pri n1             load.both n1 n2
   *    load.alt n2             -
   *    --------------------------------------
   *    load.alt n2             load.both n1 n2
   *    load.pri n1             -
   *    --------------------------------------
   *    load.s.pri n1           load.s.both n1 n2
   *    load.s.alt n2           -
   *    --------------------------------------
   *    load.s.alt n2           load.s.both n1 n2
   *    load.s.pri n1           -
   */
  {
    "load.pri %1!load.alt %2!",
    "load.both %1 %2!",
    seqsize(2,2) - seqsize(1,2)
  },
  {
    "load.alt %2!load.pri %1!",
    "load.both %1 %2!",
    seqsize(2,2) - seqsize(1,2)
  },
  {
    "load.s.pri %1!load.s.alt %2!",
    "load.s.both %1 %2!",
    seqsize(2,2) - seqsize(1,2)
  },
  {
    "load.s.alt %2!load.s.pri %1!",
    "load.s.both %1 %2!",
    seqsize(2,2) - seqsize(1,2)
  },
  /* Loading two registers and then pushing them occurs with user operators
   *    load.both n1 n2         push2 n1 n2
   *    push.pri                -
   *    push.alt                -
   *    --------------------------------------
   *    load.s.both n1 n2       push2.s n1 n2
   *    push.pri                -
   *    push.alt                -
   */
  {
    "load.both %1 %2!push.pri!push.alt!",
    "push2 %1 %2!",
    seqsize(3,2) - seqsize(1,2)
  },
  {
    "load.s.both %1 %2!push.pri!push.alt!",
    "push2.s %1 %2!",
    seqsize(3,2) - seqsize(1,2)
  },
  /* Load a constant in a variable
   *    const.pri n1            const n2 n1
   *    stor.pri n2             -
   *    --------------------------------------
   *    const.pri n1            const.s n2 n1
   *    stor.s.pri n2           -
   */
  {
    "const.pri %1!stor.pri %2!",
    "const %2 %1!",
    seqsize(2,2) - seqsize(1,2)
  },
  {
    "const.pri %1!stor.s.pri %2!",
    "const.s %2 %1!",
    seqsize(2,2) - seqsize(1,2)
  },
  /* ----- */
  { NULL, NULL, 0 }
};

static int stgstring(char *start,char *end);
static void stgopt(char *start,char *end,int (*outputfunc)(char *str));


#define sSTG_GROW   512
#define sSTG_MAX    20480

static char *stgbuf=NULL;
static int stgmax=0;           /* current size of the staging buffer */
static int stglen=0;           /* current length of the staging buffer */
static int stggrow=sSTG_GROW;  /* amount to increase the staging buffer by */

static char *stgpipe=NULL;
static int pipemax=0;          /* current size of the stage pipe, a second staging buffer */
static int pipeidx=0;
static int pipegrow=sSTG_GROW; /* amount to increase the staging pipe by */

#define CHECK_STGBUFFER(index) if ((int)(index)>=stgmax)  grow_stgbuffer(&stgbuf, &stgmax, &stggrow, (index)+1)
#define CHECK_STGPIPE(index)   if ((int)(index)>=pipemax) grow_stgbuffer(&stgpipe, &pipemax, &pipegrow, (index)+1)

static void grow_stgbuffer(char **buffer, int *curmax, int *growth, int requiredsize)
{
  char *p;
  int clear= (*buffer==NULL); /* if previously none, empty buffer explicitly */

  assert(*curmax<requiredsize);
  /* if the staging buffer (holding intermediate code for one line) grows
   * over a few kBytes, there is probably a run-away expression
   */
  if (requiredsize>sSTG_MAX)
    error(102,"staging buffer");    /* staging buffer overflow (fatal error) */
  *curmax=requiredsize+*growth;
  *growth*=2; /* not as easy to grow this one by fibonacci, just double it */
  p=(char *)realloc(*buffer,*curmax*sizeof(char));
  if (p==NULL)
    error(102,"staging buffer");    /* staging buffer overflow (fatal error) */
  *buffer=p;
  if (clear)
    **buffer='\0';
}

SC_FUNC void stgbuffer_cleanup(void)
{
  if (stgbuf!=NULL) {
    free(stgbuf);
    stgbuf=NULL;
    stglen=0;
    stgmax=0;
  } /* if */
  if (stgpipe!=NULL) {
    free(stgpipe);
    stgpipe=NULL;
    pipemax=0;
    pipeidx=0;
  } /* if */
}

/* the variables "stgidx" and "staging" are declared in "scvars.c" */

/*  stgmark
 *
 *  Copies a mark into the staging buffer. At this moment there are three
 *  possible marks:
 *     sSTARTREORDER    identifies the beginning of a series of expression
 *                      strings that must be written to the output file in
 *                      reordered order
 *    sENDREORDER       identifies the end of 'reverse evaluation'
 *    sEXPRSTART + idx  only valid within a block that is evaluated in
 *                      reordered order, it identifies the start of an
 *                      expression; the "idx" value is the argument position
 *
 *  Global references: stgidx  (altered)
 *                     stgbuf  (altered)
 *                     staging (referred to only)
 */
SC_FUNC void stgmark(char mark)
{
  if (staging) {
    CHECK_STGBUFFER(stgidx);
    stgbuf[stgidx++]=mark;
    stglen++;
  } /* if */
}

static int rebuffer(char *str)
{
  if (sc_status==statWRITE) {
    int st_len=strlen(str);
    if (pipeidx>=2 && stgpipe[pipeidx-1]=='\0' && stgpipe[pipeidx-2]!='\n')
      pipeidx-=1;                           /* overwrite last '\0' */
    CHECK_STGPIPE(pipeidx+st_len+1);
    memcpy(stgpipe+pipeidx,str,st_len+1);   /* copy to staging buffer */
    pipeidx+=st_len+1;
  } /* if */
  return TRUE;
}

static int filewrite(char *str)
{
  if (sc_status==statWRITE)
    return pc_writeasm(outf,str);
  return TRUE;
}

/*  stgwrite
 *
 *  Writes the string "st" to the staging buffer or to the output file. In the
 *  case of writing to the staging buffer, the terminating byte of zero is
 *  copied too, but... the optimizer can only work on complete lines (not on
 *  fractions of it. Therefore if the string is staged, if the last character
 *  written to the buffer is a '\0' and the previous-to-last is not a '\n',
 *  the string is concatenated to the last string in the buffer (the '\0' is
 *  overwritten). This also means an '\n' used in the middle of a string isn't
 *  recognized and could give wrong results with the optimizer.
 *  Even when writing to the output file directly, all strings are buffered
 *  until a whole line is complete.
 *
 *  Global references: stgidx  (altered)
 *                     stgbuf  (altered)
 *                     staging (referred to only)
 *                     stglen  (altered)
 */
SC_FUNC void stgwrite(const char *str)
{
  int len;
  int st_len=strlen(str);

  if (staging) {
    assert(stgidx==0 || stgbuf!=NULL);  /* staging buffer must be valid if there is (apparently) something in it */
    if (stgidx>=2 && stgbuf[stgidx-1]=='\0' && stgbuf[stgidx-2]!='\n')
      stgidx-=1;                        /* overwrite last '\0' */
    CHECK_STGBUFFER(stgidx+st_len+1);
    memcpy(stgbuf+stgidx,str,st_len+1); /* copy to staging buffer */
    stgidx+=st_len+1;
    stglen+=st_len;
  } else {
    len=(stgbuf!=NULL) ? stglen : 0;
    CHECK_STGBUFFER(len+st_len+1);
    memcpy(stgbuf+len,str,st_len+1);
    len=len+st_len;
    stglen=len;
    if (len>0 && stgbuf[len-1]=='\n') {
      filewrite(stgbuf);
      stgbuf[0]='\0';
      stglen=0;
    } /* if */
  } /* if */
}

/*  stgout
 *
 *  Writes the staging buffer to the output file via stgstring() (for
 *  reversing expressions in the buffer) and stgopt() (for optimizing). It
 *  resets "stgidx".
 *
 *  Global references: stgidx  (altered)
 *                     stgbuf  (referred to only)
 *                     staging (referred to only)
 */
SC_FUNC void stgout(int index)
{
  int reordered=0;
  int idx;

  if (!staging)
    return;
  assert(pipeidx==0);

  /* first pass: sub-expressions */
  if (sc_status==statWRITE)
    reordered=stgstring(&stgbuf[index],&stgbuf[stgidx]);
  stglen=stgidx-index;
  stgidx=index;

  /* second pass: optimize the buffer created in the first pass */
  if (sc_status==statWRITE) {
    if (reordered) {
      stgopt(stgpipe,stgpipe+pipeidx,filewrite);
    } else {
      /* there is no sense in re-optimizing if the order of the sub-expressions
       * did not change; so output directly
       */
      for (idx=0; idx<pipeidx; idx+=strlen(stgpipe+idx)+1)
        filewrite(stgpipe+idx);
    } /* if */
  } /* if */
  if (stgidx<=emit_stgbuf_idx)
    emit_stgbuf_idx=-1;
  pipeidx=0;  /* reset second pipe */
}

typedef struct {
  char *start,*end;
} argstack;

/*  stgstring
 *
 *  Analyses whether code strings should be output to the file as they appear
 *  in the staging buffer or whether portions of it should be re-ordered.
 *  Re-ordering takes place in function argument lists; Pawn passes arguments
 *  to functions from right to left. When arguments are "named" rather than
 *  positional, the order in the source stream is indeterminate.
 *  This function calls itself recursively in case it needs to re-order code
 *  strings, and it uses a private stack (or list) to mark the start and the
 *  end of expressions in their correct (reversed) order.
 *  In any case, stgstring() sends a block as large as possible to the
 *  optimizer stgopt().
 *
 *  In "reorder" mode, each set of code strings must start with the token
 *  sEXPRSTART, even the first. If the token sSTARTREORDER is represented
 *  by '[', sENDREORDER by ']' and sEXPRSTART by '|' the following applies:
 *     '[]...'     valid, but useless; no output
 *     '[|...]     valid, but useless; only one string
 *     '[|...|...] valid and useful
 *     '[...|...]  invalid, first string doesn't start with '|'
 *     '[|...|]    invalid
 */
static int stgstring(char *start,char *end)
{
  char *ptr;
  int nest,argc,arg;
  argstack *stack;
  int reordered=FALSE;

  while (start<end) {
    if (*start==sSTARTREORDER) {
      start+=1;         /* skip token */
      /* allocate a argstack with sMAXARGS items */
      stack=(argstack *)malloc(sMAXARGS*sizeof(argstack));
      if (stack==NULL)
        error(103);     /* insufficient memory */
      reordered=TRUE;   /* mark that the expression is reordered */
      nest=1;           /* nesting counter */
      argc=0;           /* argument counter */
      arg=-1;           /* argument index; no valid argument yet */
      do {
        switch (*start) {
        case sSTARTREORDER:
          nest++;
          start++;
          break;
        case sENDREORDER:
          nest--;
          start++;
          break;
        default:
          if ((*start & sEXPRSTART)==sEXPRSTART) {
            if (nest==1) {
              if (arg>=0)
                stack[arg].end=start-1; /* finish previous argument */
              arg=(unsigned char)*start - sEXPRSTART;
              stack[arg].start=start+1;
              if (arg>=argc)
                argc=arg+1;
            } /* if */
            start++;
          } else {
            start+=strlen(start)+1;
          } /* if */
        } /* switch */
      } while (nest); /* enddo */
      if (arg>=0)
        stack[arg].end=start-1;   /* finish previous argument */
      while (argc>0) {
        argc--;
        stgstring(stack[argc].start,stack[argc].end);
      } /* while */
      free(stack);
    } else {
      ptr=start;
      while (ptr<end && *ptr!=sSTARTREORDER)
        ptr+=strlen(ptr)+1;
      stgopt(start,ptr,rebuffer);
      start=ptr;
    } /* if */
  } /* while */
  return reordered;
}

/*  stgdel
 *
 *  Scraps code from the staging buffer by resetting "stgidx" to "index".
 *
 *  Global references: stgidx (altered)
 *                     staging (referred to only)
 */
SC_FUNC void stgdel(int index,cell code_index)
{
  if (staging) {
    stgidx=index;
    code_idx=code_index;
  } /* if */
}

SC_FUNC int stgget(int *index,cell *code_index)
{
  if (staging) {
    *index=stgidx;
    *code_index=code_idx;
  } /* if */
  return staging;
}

/*  stgset
 *
 *  Sets staging on or off. If it's turned off, the staging buffer must be
 *  initialized to an empty string. If it's turned on, the routine makes sure
 *  the index ("stgidx") is set to 0 (it should already be 0).
 *
 *  Global references: staging  (altered)
 *                     stgidx   (altered)
 *                     stgbuf   (contents altered)
 */
SC_FUNC void stgset(int onoff)
{
  staging=onoff;
  if (staging){
    assert(stgidx==0);
    stgidx=0;
    CHECK_STGBUFFER(stgidx);
    /* write any contents that may be put in the buffer by stgwrite()
     * when "staging" was 0
     */
    if (stglen>0)
      filewrite(stgbuf);
  } /* if */
  stgbuf[0]='\0';
  stglen=0;
}

#define MAX_OPT_VARS    5
#define MAX_OPT_CAT     5       /* max. values that are concatenated */
#if sNAMEMAX > (PAWN_CELL_SIZE/4) * MAX_OPT_CAT
  #define MAX_ALIAS       sNAMEMAX
#else
  #define MAX_ALIAS       (PAWN_CELL_SIZE/4) * MAX_OPT_CAT
#endif

static int matchsequence(char *start,char *end,char *pattern,
                         char symbols[MAX_OPT_VARS][MAX_ALIAS+1],
                         int *match_length)
{
  int var,i;
  char str[MAX_ALIAS+1];
  char *start_org=start;
  cell value;
  char *ptr;

  *match_length=0;
  for (var=0; var<MAX_OPT_VARS; var++)
    symbols[var][0]='\0';

  while (*start=='\t' || *start==' ')
    start++;
  while (*pattern) {
    if (start>=end)
      return FALSE;
    switch (*pattern) {
    case '%':   /* new "symbol" */
      pattern++;
      assert(isdigit(*pattern));
      var=atoi(pattern) - 1;
      assert(var>=0 && var<MAX_OPT_VARS);
      assert(*start=='-' || alphanum(*start));
      for (i=0; start<end && (*start=='-' || *start=='+' || alphanum(*start)); i++,start++) {
        assert(i<=MAX_ALIAS);
        str[i]=*start;
      } /* for */
      str[i]='\0';
      if (symbols[var][0]!='\0') {
        if (strcmp(symbols[var],str)!=0)
          return FALSE; /* symbols should be identical */
      } else {
        strcpy(symbols[var],str);
      } /* if */
      break;
    case '-':
      value=-strtol(pattern+1,&pattern,16);
      ptr=itoh((ucell)value);
      while (*ptr!='\0') {
        if (tolower(*start) != tolower(*ptr))
          return FALSE;
        start++;
        ptr++;
      } /* while */
      pattern--;  /* there is an increment following at the end of the loop */
      break;
    case ' ':
      if (*start!='\t' && *start!=' ')
        return FALSE;
      while (start<end && (*start=='\t' || *start==' '))
        start++;
      break;
    case '!':
      while (start<end && (*start=='\t' || *start==' '))
        start++;                /* skip trailing white space */
      if (*start==';')
        while (start<end && *start!='\n')
          start++;              /* skip trailing comment */
      if (*start!='\n')
        return FALSE;
      assert(*(start+1)=='\0');
      start+=2;                 /* skip '\n' and '\0' */
      if (*(pattern+1)!='\0')
        while ((start<end && *start=='\t') || *start==' ')
          start++;              /* skip leading white space of next instruction */
      break;
    default:
      if (tolower(*start) != tolower(*pattern))
        return FALSE;
      start++;
    } /* switch */
    pattern++;
  } /* while */

  *match_length=(int)(start-start_org);
  return TRUE;
}

static char *replacesequence(char *pattern,char symbols[MAX_OPT_VARS][MAX_ALIAS+1],int *repl_length)
{
  char *lptr;
  int var;
  char *buffer;

  /* calculate the length of the new buffer
   * this is the length of the pattern plus the length of all symbols (note
   * that the same symbol may occur multiple times in the pattern) plus
   * line endings and startings ('\t' to start a line and '\n\0' to end one)
   */
  assert(repl_length!=NULL);
  *repl_length=0;
  lptr=pattern;
  while (*lptr) {
    switch (*lptr) {
    case '%':
      lptr++;           /* skip '%' */
      assert(isdigit(*lptr));
      var=atoi(lptr) - 1;
      assert(var>=0 && var<MAX_OPT_VARS);
      assert(symbols[var][0]!='\0');    /* variable should be defined */
      *repl_length+=strlen(symbols[var]);
      break;
    case '!':
      *repl_length+=3;  /* '\t', '\n' & '\0' */
      break;
    default:
      *repl_length+=1;
    } /* switch */
    lptr++;
  } /* while */

  /* allocate a buffer to replace the sequence in */
  if ((buffer=(char*)malloc(*repl_length))==NULL)
    error(103);

  /* replace the pattern into this temporary buffer */
  lptr=buffer;
  *lptr++='\t';         /* the "replace" patterns do not have tabs */
  while (*pattern) {
    assert((int)(lptr-buffer)<*repl_length);
    switch (*pattern) {
    case '%':
      /* write out the symbol */
      pattern++;
      assert(isdigit(*pattern));
      var=atoi(pattern) - 1;
      assert(var>=0 && var<MAX_OPT_VARS);
      assert(symbols[var][0]!='\0');    /* variable should be defined */
      strcpy(lptr,symbols[var]);
      lptr+=strlen(symbols[var]);
      break;
    case '!':
      /* finish the line, optionally start the next line with an indent */
      *lptr++='\n';
      *lptr++='\0';
      if (*(pattern+1)!='\0')
        *lptr++='\t';
      break;
    default:
      *lptr++=*pattern;
    } /* switch */
    pattern++;
  } /* while */

  assert((int)(lptr-buffer)==*repl_length);
  return buffer;
}

static void strreplace(char *dest,char *replace,int sub_length,int repl_length,int dest_length)
{
  int offset=sub_length-repl_length;
  if (offset>0) {               /* delete a section */
    memmove(dest,dest+offset,dest_length-offset);
    memset(dest+dest_length-offset,0xcc,offset); /* not needed, but for cleanlyness */
  } else if (offset<0) {        /* insert a section */
    memmove(dest-offset, dest, dest_length);
  } /* if */
  memcpy(dest, replace, repl_length);
}

/*  stgopt
 *
 *  Optimizes the staging buffer by checking for series of instructions that
 *  can be coded more compact. The routine expects the lines in the staging
 *  buffer to be separated with '\n' and '\0' characters.
 *
 *  The longest sequences should probably be checked first.
 */

static void stgopt(char *start,char *end,int (*outputfunc)(char *str))
{
  char symbols[MAX_OPT_VARS][MAX_ALIAS+1];
  int seq,match_length,repl_length;
  int matches;
  char *debut=start;  /* save original start of the buffer */

  /* do not match anything if debug-level is maximum */
  if (pc_optimize>sOPTIMIZE_NONE && sc_status==statWRITE && emit_stgbuf_idx==-1) {
    do {
      matches=0;
      start=debut;
      while (start<end) {
        seq=0;
        while (sequences[seq].find!=NULL) {
          assert(seq>=0);
          if (*sequences[seq].find=='\0') {
            if (pc_optimize==sOPTIMIZE_NOMACRO) {
              break;    /* don't look further */
            } else {
              seq++;    /* continue with next string */
              continue;
            } /* if */
          } /* if */
          if (matchsequence(start,end,sequences[seq].find,symbols,&match_length)) {
            char *replace=replacesequence(sequences[seq].replace,symbols,&repl_length);
            /* If the replacement is bigger than the original section, we may need
             * to "grow" the staging buffer. This is quite complex, due to the
             * re-ordering of expressions that can also happen in the staging
             * buffer. In addition, it should not happen: the peephole optimizer
             * must replace sequences with *shorter* sequences, not longer ones.
             * So, I simply forbid sequences that are longer than the ones they
             * are meant to replace.
             */
            assert(match_length>=repl_length);
            if (match_length>=repl_length) {
              strreplace(start,replace,match_length,repl_length,(int)(end-start));
              end-=match_length-repl_length;
              free(replace);
              code_idx-=sequences[seq].savesize;
              seq=0;                      /* restart search for matches */
              matches++;
            } else {
              /* actually, we should never get here (match_length<repl_length) */
              assert(0);
              seq++;
            } /* if */
          } else {
            seq++;
          } /* if */
        } /* while */
        assert(sequences[seq].find==NULL || (*sequences[seq].find=='\0' && pc_optimize==sOPTIMIZE_NOMACRO));
        start += strlen(start) + 1;       /* to next string */
      } /* while (start<end) */
    } while (matches>0);
  } /* if (pc_optimize>sOPTIMIZE_NONE && sc_status==statWRITE) */

  for (start=debut; start<end; start+=strlen(start)+1)
    outputfunc(start);
}
