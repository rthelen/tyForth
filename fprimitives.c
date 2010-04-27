/*
 * This file is part of arm-sim: http://madscientistroom.org/arm-sim
 *
 * Copyright (c) 2010 Randy Thelen. All rights reserved, and all wrongs
 * reversed. (See the file COPYRIGHT for details.)
 */

#include "forth.h"
#include "fobj.h"

fobj_t *MKFNAME(pop)(fenv_t *f, forth_header_t *w)
{
    return fstack_fetch(f, f->dstack, NULL);
}

void MKFNAME(push)(fenv_t *f, forth_header_t *w, fobj_t *p)
{
    fstack_store(f, f->dstack, NULL, p);
}

fnumber_t MKFNAME(pop_num)(fenv_t *f, forth_header_t *w)
{
    fobj_t *num_obj = POP;
    fassert(f, num_obj->type == FOBJ_NUM, 1, "A number was expected here");
    return num_obj->u.num.n;
}

fint_t MKFNAME(pop_int)(fenv_t *f, forth_header_t *w)
{
    return (fint_t) MKFNAME(pop_num)(f, w);
}

fobj_t *MKFNAME(rpop)(fenv_t *f, forth_header_t *w)
{
    return fstack_fetch(f, f->rstack, NULL);
}

void MKFNAME(rpush)(fenv_t *f, forth_header_t *w, fobj_t *p)
{
    fstack_store(f, f->rstack, NULL, p);
}

FCODE(print)
{
    fobj_print(f, POP);
}

/**********************************************************
 *
 * The Most Rudimentary Stack Operators
 *
 **********************************************************/
#define A		fobj_t *a
#define B		fobj_t *b
#define C		fobj_t *c

FWORD(dup)      { A = POP; PUSH(a); PUSH(a); }
FWORD(drop)     { POP; }
FWORD(swap)     { A = POP; B = POP; PUSH(a); PUSH(b); }
FWORD(nip)      { SWAP; DROP; }
FWORD(2drop)    { POP; POP; }

            /* over:  a b -> a b a */
FWORD(over)         { B = POP; A = POP; PUSH(a); PUSH(b); PUSH(a); }

/**********************************************************
 *
 * Arithmetic operators
 *
 * Note that plus and minus are overloaded operators and that's why
 * fobj_{add,sub}() is called instead of just allowing the C compiler
 * to generate add/sub mnemonics.
 *
 **********************************************************/

FWORD2(plus, "+")
{
    B = POP;
    A = POP;

    PUSH(fobj_add(f, a, b));
}

FWORD2(minus, "-")
{
    B = POP;
    A = POP;

    PUSH(fobj_sub(f, a, b));
}

FWORD2(star, "*")    { PUSHN(POPN * POPN); }
FWORD2(slash, "/")   { fnumber_t b = POPN; fnumber_t a = POPN; PUSHN(a / b); }
FWORD(and)           { PUSHN(POPI & POPI); }
FWORD(or)            { PUSHN(POPI | POPI); }
FWORD(xor)           { PUSHN(POPI ^ POPI); }

FWORD(negate)        { PUSHN(-POPN); }
FWORD(invert)        { PUSHN(~POPI); }

FWORD2(2star, "2*")        { PUSHN(POPI * 2); }
FWORD2(2slash, "2/")       { PUSHN(POPI / 2); }
FWORD2(u2slash, "u2/")     { PUSHN(POPI >> 1); }

FWORD2(shift_left, "<<")
{ fint_t cnt = POPI, n =  POPI; PUSHN(n << cnt); }

FWORD2(shift_right, ">>")
{ fint_t cnt = POPI; fint_t n = POPI; PUSHN(n >> cnt); }

FWORD2(ushift_right, "u>>")
{ fint_t cnt = POPI; fuint_t n = POPI; PUSHN(n >> cnt); }

FWORD2(uless, "u<")
{  fuint_t b =  POPI;  fuint_t a = POPI; PUSHN(a < b ? -1 : 0); }

FWORD2(fetch, "@")
{
    A = POP;
    B = NULL;
    if (fobj_is_index(f, a)) {
        b = a->u.index.index;
        a =  a->u.index.addr;
    }

    PUSH(fobj_fetch(f, a, b));
}

FWORD2(store, "!")
{
    A = POP;  // Object to store into
    B = NULL;     // Index
    if (fobj_is_index(f, a)) {
        b = a->u.index.index;
        a =  a->u.index.addr;
    }
    C = POP;

    fobj_store(f, a, b, c);
}

FWORD2(index, "]")
{
    B = POP;
    A = POP;

    PUSH(findex_new(f, a, b));
}
