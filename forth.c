/*
 * This file is part of arm-sim: http://madscientistroom.org/arm-sim
 *
 * Copyright (c) 2010 Randy Thelen. All rights reserved, and all wrongs
 * reversed. (See the file COPYRIGHT for details.)
 */

#include "forth.h"
#include "fobj.h"

void FASSERT(int x, const char *err, const char *file, int line)
{
    if (!(x)) {
        fprintf(stderr, "INTERNAL ERROR: %s in file %s on line %d\n", err, file, line);
        exit(-1);
    }
}

void fassert(fenv_t *f, int condition, int err, const char *fmt, ...)
{
    va_list ap;

    if (condition) return;

    va_start(ap, fmt);
    fprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);

    exit(-1);
}

void fadd(fenv_t *f, fobj_t *stack)
{
    fobj_t *op2 = ftable_pop(f, stack);
    fobj_t *op1 = ftable_pop(f, stack);

    ftable_push(f, stack, fobj_add(f, op1, op2));

    fobj_release(f, op1);
    fobj_release(f, op2);
}

void fsub(fenv_t *f, fobj_t *stack)
{
    fobj_t *op2 = ftable_pop(f, stack);
    fobj_t *op1 = ftable_pop(f, stack);

    ftable_push(f, stack, fobj_sub(f, op1, op2));

    fobj_release(f, op1);
    fobj_release(f, op2);
}

int main(int argc, char *argv[])
{
    fenv_t *f = NULL;
    fobj_t *r;
    fobj_t *dstack = ftable_new(f, NULL);

    ftable_push(f, dstack, fnum_new(f, 2));
    ftable_push(f, dstack, fnum_new(f, 5));
    ftable_push(f, dstack, fnum_new(f, 8));

    fadd(f, dstack);
    fsub(f, dstack);

    printf("Result = ");
    r = ftable_pop(f, dstack);
    fobj_print(f, r);
    fobj_release(f, r);

    ftable_push(f, dstack, fstr_new(f, "Hello"));
    ftable_push(f, dstack, fstr_new(f, " world"));
    fadd(f, dstack);

    printf("Result = ");
    r = ftable_pop(f, dstack);
    fobj_print(f, r);
    fobj_release(f, r);

    fobj_release(f, dstack);

    return 0;
}
