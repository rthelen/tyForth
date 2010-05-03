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
    printf("    Value = %Lf\n", p->u.num.n);
#else
    printf(" %Lg\n", p->u.num.n);
#endif
}

fobj_t *fnum_new(fenv_t *f, fnumber_t n)
{
    fobj_t *p = fobj_new(f, FOBJ_NUM);
    p->u.num.n = n;
    return p;
}

int fnum_cmp(fenv_t *f, fobj_t *a, fobj_t *b)
{
    if (a->u.num.n  < b->u.num.n) return -1;
    if (a->u.num.n == b->u.num.n) return  0;
    else                          return  1;
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
