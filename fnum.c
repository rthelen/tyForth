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
#ifdef DEBUG
    printf("    Value = %f\n", p->u.num.n);
#else
    printf(" %f\n", p->u.num.n);
#endif
}

fobj_t *fnum_new(fenv_t *f, double n)
{
    fobj_t *p = fobj_new(f, FOBJ_NUM);
    p->u.num.n = n;
    return p;
}

fobj_t *fnum_add(fenv_t *f, fobj_t *op1, fobj_t *op2)
{
    ASSERT(op1->type == FOBJ_NUM);
    fassert(f, op2->type == FOBJ_NUM, 1, "Wrong type");
    return fnum_new(f, op1->u.num.n + op2->u.num.n);
}

fobj_t *fnum_sub(fenv_t *f, fobj_t *op1, fobj_t *op2)
{
    ASSERT(op1->type == FOBJ_NUM);
    fassert(f, op2->type == FOBJ_NUM, 1, "Wrong type");
    return fnum_new(f, op1->u.num.n - op2->u.num.n);
}
