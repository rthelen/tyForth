/*
 * This file is part of arm-sim: http://madscientistroom.org/arm-sim
 *
 * Copyright (c) 2010 Randy Thelen. All rights reserved, and all wrongs
 * reversed. (See the file COPYRIGHT for details.)
 */

#include "forth.h"
#include "fobj.h"

void fnum_print(fenv_t *f, fobj_t *p)
{
    printf("    Value = %f\n", p->u.num.n);
}

fobj_t *fnum_new(fenv_t *f, double n)
{
    fobj_t *p = fobj_new(f, FOBJ_NUM);
    p->u.num.n = n;
    return p;
}

void fnum_free(fenv_t *f, fobj_t *p)
{
    /* Nothing for a number */
}

void fnum_add(fenv_t *f, felem_t *dest, felem_t *op1, felem_t *op2)
{
    ASSERT(op1->obj->type == FOBJ_NUM);
    ASSERT(op2->obj->type > 0);
    fassert(f, op2->obj->type == FOBJ_NUM, 1, "%s %s + not supported", 
            op_table[op1->obj->type].type_name,
            op_table[op2->obj->type].type_name);

    printf("Number Add\n");

    dest->obj = fnum_new(f, op1->obj->u.num.n + op2->obj->u.num.n);
}

void fnum_sub(fenv_t *f, felem_t *dest, felem_t *op1, felem_t *op2)
{
    ASSERT(op1->obj->type == FOBJ_NUM);
    ASSERT(op2->obj->type > 0);
    fassert(f, op2->obj->type == FOBJ_NUM, 1, "%s %s - not supported", 
            op_table[op1->obj->type].type_name,
            op_table[op2->obj->type].type_name);

    printf("Number Sub\n");

    dest->obj = fnum_new(f, op1->obj->u.num.n - op2->obj->u.num.n);
}

