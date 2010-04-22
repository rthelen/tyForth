/*
 * This file is part of arm-sim: http://madscientistroom.org/arm-sim
 *
 * Copyright (c) 2010 Randy Thelen. All rights reserved, and all wrongs
 * reversed. (See the file COPYRIGHT for details.)
 */

#include "forth.h"
#include "fobj.h"


const foptable_t op_table[FOBJ_NUM_TYPES] = {
    { 0 }, // The zeroth entry is INVALID
    { "number", fnum_free, fnum_print, fnum_add, fnum_sub },
    { "string", fstr_free, fstr_print, fstr_add, fstr_sub, NULL, fstr_fetch },
    { "table",  ftable_free, ftable_print, NULL, NULL, ftable_store, ftable_fetch },
};


fobj_t *fobj_new(fenv_t *f, int type)
{
    fobj_t *n = malloc(sizeof(fobj_t));
    fassert(f, n != NULL, 1, "out of memory allocating a new fobj");

    n->type = type;
    n->refcount = 1;

    return n;
}

void fobj_release(fenv_t *f, fobj_t *p)
{
    ASSERT(p);
    ASSERT(p->type);
    ASSERT(p->refcount > 0);

    p->refcount -= 1;

    if (p->refcount == 0) {
        op_table[p->type].free(f, p);
        free(p);
    }
}

fobj_t *fobj_retain(fenv_t *f, fobj_t *p)
{
    if (!p) return p;

    ASSERT(p->type);
    ASSERT(p->refcount > 0);

    p->refcount += 1;

    return p;
}

void fobj_print(fenv_t *f, fobj_t *p)
{
    ASSERT(p);
    ASSERT(p->type > 0);
    ASSERT(p->refcount > 0);
    ASSERT(op_table[p->type].print);

    printf("Object %p: type = %d, refcount = %d\n", p, p->type, p->refcount);

    op_table[p->type].print(f, p);
}

fobj_t *fobj_add(fenv_t *f, fobj_t *op1, fobj_t *op2)
{
    fassert(f, !!op_table[op1->type].add, 1, "%s <> + not supported",
            op_table[op1->type].type_name);

    return op_table[op1->type].add(f, op1, op2);
}    

fobj_t *fobj_sub(fenv_t *f, fobj_t *op1, fobj_t *op2)
{
    fassert(f, !!op_table[op1->type].sub, 1, "%s <> - not supported",
            op_table[op1->type].type_name);

    return op_table[op1->type].sub(f, op1, op2);
}    

fobj_t *fobj_fetch(fenv_t *f, fobj_t *addr, fobj_t *index)
{
    fassert(f, !!op_table[addr->type].fetch, 1, "%s @ not supported",
            op_table[addr->type].type_name);

    return op_table[addr->type].fetch(f, addr, index);
}    

