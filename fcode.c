/*
 * This file is part of arm-sim: http://madscientistroom.org/arm-sim
 *
 * Copyright (c) 2010 Randy Thelen. All rights reserved, and all wrongs
 * reversed. (See the file COPYRIGHT for details.)
 */

#include "forth.h"
#include "fobj.h"

#define MKFNAME(x)			fcode_ ## x
#define MKHDRNAME(_name)	_name ## _header

#define FWORD_HEADER(_name, _str)                       \
    void _name(fenv_t *f, fobj_t *w);           \
    fheader_t MKHDRNAME(_name) = { _str, _name };   \
    void _name(fenv_t *f, fobj_t *w)

#define FWORD2(_name, _str)                    \
    FWORD_HEADER(MKFNAME(_name), _str)

#define FWORD(_name)                           \
    FWORD2(_name, # _name)

#define FWORD_IMM2(_name, _str)                \
    FWORD_HEADER(MKFNAME(_name), _str)

#define FWORD_IMM(_name)                       \
    FWORD_IMM2(_name, # _name)

#define FWORD_DO2(_name, _str)                 \
    FWORD_HEADER(MKFNAME(do_ ## _name), _str)

#define FWORD_DO(_name)                        \
    FWORD_DO2(_name, "(" # _name ")")

#define FCODE(x)			void MKFNAME(x)(fenv_t *f, fobj_t *w)

#define PUSH(x)				MKFNAME(push)(f, w, x)
#define PUSHN(n)			MKFNAME(push)(f, w, fnum_new(f, n))
#define PUSHS(s)			MKFNAME(push)(f, w, fstr_new(f, s))
#define POP					MKFNAME(pop)(f, w)
#define POPN				MKFNAME(pop_num)(f, w)
#define POPI				MKFNAME(pop_int)(f, w)

#define RPOP				MKFNAME(rpop)(f, w)
#define RPUSH(x)			MKFNAME(rpush)(f, w, x)

#define OVER				MKFNAME(over)(f, w)
#define SWAP				MKFNAME(swap)(f, w)
#define DUP					MKFNAME(dup)(f, w)
#define DROP				MKFNAME(drop)(f, w)
#define ADD					MKFNAME(add)(f, w)
#define SUB					MKFNAME(sub)(f, w)
#define PRINT				MKFNAME(print)(f, w)

#define STORE				MKFNAME(store)(f, w)
#define FETCH				MKFNAME(fetch)(f, w)
#define INDEX				MKFNAME(index)(f, w)

void    MKFNAME(push)(fenv_t *f, fobj_t *w, fobj_t *p);
fobj_t *MKFNAME(pop)(fenv_t *f, fobj_t *w);
fnumber_t MKFNAME(pop_num)(fenv_t *f, fobj_t *w);
fint_t  MKFNAME(pop_int)(fenv_t *f, fobj_t *w);
union fbody_u {
    fobj_t			*word;
    fnumber_t		 n;
};

struct fheader_s {
    char				*name;
    fcode_t				 code;
};


/*
 * Include an auto-generated file with the prototypes for all of the
 * functions in this file.
 */

#include "fwords.h"


/*
 * This file contains all the primitives of the system.
 */

fheader_t *fcode_primitives_ptrs[] = {
	/*
     * Bring in an auto-generated list of the FWORDs defined in
     * fprimitives.c and possibly elsewhere.
     */

	#include "fwords.c"

    NULL
};

void fcode_init(fenv_t *f)
{
    /*
     * Add the words to the dictionary
     */
    fheader_t *p;

    for (int i = 0; (p = fcode_primitives_ptrs[i]); i++) {
        fcode_new_primitive(f, fstr_new(f, p->name), p->code);
    }
}

void fcode_new_primitive(fenv_t *f, fobj_t*name, fcode_t code)
{
    fobj_t *word = fobj_new(f, FOBJ_WORD);
    fword_t *w = &word->u.word;
    w->name = name;
    w->code = code;
    w->u.body = NULL;
    ftable_store(f, f->words, name, word);
}

void fcode_new_var(fenv_t *f, fobj_t *name, fobj_t *value)
{
    fobj_t *word = fobj_new(f, FOBJ_WORD);
    fword_t *w = &word->u.word;
    w->name = name;
    w->code = fcode_do_constant_header.code;
    w->u.value = value;
    ftable_store(f, f->words, name, word);
}

fobj_t *MKFNAME(pop)(fenv_t *f, fobj_t *w)
{
    return fstack_fetch(f, f->dstack, NULL);
}

void MKFNAME(push)(fenv_t *f, fobj_t *w, fobj_t *p)
{
    fstack_store(f, f->dstack, NULL, p);
}

fnumber_t MKFNAME(pop_num)(fenv_t *f, fobj_t *w)
{
    fobj_t *num_obj = POP;
    fassert(f, num_obj->type == FOBJ_NUM, 1, "A number was expected here");
    return num_obj->u.num.n;
}

fint_t MKFNAME(pop_int)(fenv_t *f, fobj_t *w)
{
    return (fint_t) MKFNAME(pop_num)(f, w);
}

fobj_t *MKFNAME(rpop)(fenv_t *f, fobj_t *w)
{
    return fstack_fetch(f, f->rstack, NULL);
}

void MKFNAME(rpush)(fenv_t *f, fobj_t *w, fobj_t *p)
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

FWORD2(dot, ".") { PRINT; }
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

FWORD2(mktable, "{}")
{
    PUSH(ftable_new(f));
}

FWORD2(index, "]")
{
    B = POP;
    A = POP;

    PUSH(findex_new(f, a, b));
}

FWORD2(one, "1")
{
    PUSHN(1);
}

FWORD_DO(var)
{
    PUSH(w);  // Use @ and ! to read or modify w's u.value field.
}

FWORD_DO(constant)
{
    PUSH(w->u.word.u.value);
}

FWORD2(mkvar, "var")
{
    fobj_t *var_name;

    (void) fparse_token(f, &var_name);
    fassert(f, !!var_name, 1, "var must be followed by a variable name");

    ftable_store(f, f->words, var_name, NULL);
}

FWORD2(mkconst, "const")
{
    fobj_t *cons_value = POP;
    fassert(f, !!cons_value, 1, "const must be preceded by a non-null value");

    fobj_t *cons_name;
    (void) fparse_token(f, &cons_name);
    fassert(f, !!cons_name, 1, "const must be followed by a name");

    ftable_store(f, f->words, cons_name, cons_value);
}
