/*
 * This file is part of arm-sim: http://madscientistroom.org/arm-sim
 *
 * Copyright (c) 2010 Randy Thelen. All rights reserved, and all wrongs
 * reversed. (See the file COPYRIGHT for details.)
 */

#include "forth.h"
#include "fobj.h"

fobj_t *fpop(fenv_t *f)
{
    return fstack_fetch(f, f->dstack, NULL);
}

void fpush(fenv_t *f, fobj_t *p)
{
    fstack_store(f, f->dstack, NULL, p);
}

fobj_t *frpop(fenv_t *f)
{
    return fstack_fetch(f, f->rstack, NULL);
}

void frpush(fenv_t *f, fobj_t *p)
{
    fstack_store(f, f->rstack, NULL, p);
}

void fover(fenv_t *f)
{
    fobj_t *a, *b;
    a = fpop(f);
    b = fpop(f);
    fpush(f, b);
    fpush(f, a);
    fpush(f, b);
}

void fdup(fenv_t *f)
{
    fobj_t *a;
    a = fpop(f);
    fpush(f, a);
    fpush(f, a);
}

void fadd(fenv_t *f)
{
    fobj_t *op2 = fpop(f);
    fobj_t *op1 = fpop(f);

    fpush(f, fobj_add(f, op1, op2));
}

void fsub(fenv_t *f)
{
    fobj_t *op2 = fpop(f);
    fobj_t *op1 = fpop(f);

    fpush(f, fobj_sub(f, op1, op2));
}

void ffetch(fenv_t *f)
{
    fobj_t *addr = fpop(f);
    fobj_t *index = NULL;
    if (fobj_is_index(f, addr)) {
        index = addr->u.index.index;
        addr =  addr->u.index.addr;
    }

    fpush(f, fobj_fetch(f, addr, index));
}

void fstore(fenv_t *f)
{
    fobj_t *addr = fpop(f);
    fobj_t *index = NULL;
    if (fobj_is_index(f, addr)) {
        index = addr->u.index.index;
        addr =  addr->u.index.addr;
    }
    fobj_t *data = fpop(f);

    fobj_store(f, addr, index, data);
}

void findex(fenv_t *f)
{
    fobj_t *i = fpop(f);
    fobj_t *a = fpop(f);

    fpush(f, findex_new(f, a, i));
}
