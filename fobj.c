/*
 * This file is part of arm-sim: http://madscientistroom.org/arm-sim
 *
 * Copyright (c) 2010 Randy Thelen. All rights reserved, and all wrongs
 * reversed. (See the file COPYRIGHT for details.)
 */

#include "forth.h"
#include "fobj.h"


const foptable_t op_table[FOBJ_NUM_TYPES] = {
    { 0, 0, 0 }, // The zeroth entry is INVALID
    { "number", fnum_free, fnum_print, fnum_add, fnum_sub },
    { "string", fstr_free, fstr_print, fstr_add, fstr_sub },
    { "table",  ftable_free, ftable_print, ftable_add, ftable_sub },
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

void fobj_retain(fenv_t *f, fobj_t *p)
{
    ASSERT(p);
    ASSERT(p->type);
    ASSERT(p->refcount > 0);

    p->refcount += 1;
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

void fobj_add(fenv_t *f, felem_t *dest, felem_t *op1, felem_t *op2)
{
    felem_init(f, dest, NULL);
    op_table[op1->obj->type].add(f, dest, op1, op2);
}    

void fobj_sub(fenv_t *f, felem_t *dest, felem_t *op1, felem_t *op2)
{
    felem_init(f, dest, NULL);
    if (op_table[op1->obj->type].sub(f, dest, op1, op2) == 0) {
        fassert(f, FALSE, 1, "%s %s + not supported",
                op_table[op1->obj->type].type_name,
                op_table[op2->obj->type].type_name);
    }
}    

