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

void fadd(fenv_t *f, fstack_t *stack)
{
    felem_t dest, op1, op2;

    fstack_pop(f, stack, &op2);
    fstack_pop(f, stack, &op1);

    fobj_add(f, &dest, &op1, &op2);

    felem_free(f, &op1);
    felem_free(f, &op2);

    fstack_push(f, stack, &dest);
}

void fsub(fenv_t *f, fstack_t *stack)
{
    felem_t dest, op1, op2;

    fstack_pop(f, stack, &op2);
    fstack_pop(f, stack, &op1);

    fobj_sub(f, &dest, &op1, &op2);

    felem_free(f, &op1);
    felem_free(f, &op2);

    fstack_push(f, stack, &dest);
}

int main(int argc, char *argv[])
{
    fenv_t *f = NULL;

    fstack_t *dstack = fstack_new(f, "data stack");

    fstack_push_obj(f, dstack, fnum_new(f, 2));
    fstack_push_obj(f, dstack, fnum_new(f, 5));
    fstack_push_obj(f, dstack, fnum_new(f, 8));

    fadd(f, dstack);
    fsub(f, dstack);

    felem_t n;

    fstack_pop(f, dstack, &n);
    printf("Result = ");
    fobj_print(f, n.obj);

    fstack_push_obj(f, dstack, fstr_new(f, "Hello"));
    fstack_push_obj(f, dstack, fstr_new(f, " world"));
    fadd(f, dstack);

    fstack_pop(f, dstack, &n);
    printf("Result = ");
    fobj_print(f, n.obj);

    fstack_free(f, dstack);

    return 0;
}
