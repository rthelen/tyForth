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

#define PUSH(x)				fstack_store(f, stack, NULL, x)
#define PUSHN(n)			PUSH(fnum_new(f, n))
#define PUSHS(s)			PUSH(fstr_new(f, s))
#define POP					fstack_fetch(f, stack, NULL)
#define RETAIN(x)			fobj_retain(f, x)
#define RELEASE(x)			fobj_release(f, x)
#define ADD(a, b)			fobj_add(f, a, b)
#define SUB(a, b)			fobj_sub(f, a, b)
#define STORE(a,i,d)		fobj_store(f, a, i, d)
#define FETCH(a,i)			fobj_fetch(f, a, i)
#define PRINT(x)			fobj_print(f, x)

#define FADD				fadd(f, stack)
#define FSUB				fsub(f, stack)
#define FSTORE				fstore(f, stack)
#define FFETCH				ffetch(f, stack)

void fadd(fenv_t *f, fobj_t *stack)
{
    fobj_t *op2 = POP;
    fobj_t *op1 = POP;

    PUSH(ADD(op1, op2));

    RELEASE(op1);
    RELEASE(op2);
}

void fsub(fenv_t *f, fobj_t *stack)
{
    fobj_t *op2 = POP;
    fobj_t *op1 = POP;

    PUSH(SUB(op1, op2));

    RELEASE(op1);
    RELEASE(op2);
}

void ffetch(fenv_t *f, fobj_t *stack)
{
    fobj_t *addr = POP;
    fobj_t *index = NULL;
    if (fobj_is_index(f, addr)) {
        index = addr;
        addr = POP;
    }

    PUSH(fobj_fetch(f, addr, index));

    RELEASE(addr);
    RELEASE(index);
}

void fstore(fenv_t *f, fobj_t *stack)
{
    fobj_t *addr = POP;
    fobj_t *index = NULL;
    if (fobj_is_index(f, addr)) {
        index = addr;
        addr = POP;
    }
    fobj_t *data = POP;

    fobj_store(f, addr, index, data);

    RELEASE(addr);
    RELEASE(index);
    RELEASE(data);
}

int main(int argc, char *argv[])
{
    fenv_t *f = NULL;
    fobj_t *r;
    fobj_t *stack = fstack_new(f);

    /*
     * Test arithmetic
     */

    PUSHN(2);
    PUSHN(5);
    PUSHN(8);

    FADD;
    FSUB;

    printf("Result = ");
    r = POP;
    PRINT(r);
    RELEASE(r);

    /*
     * Test strings
     */

    PUSHS("Hello");
    PUSHS(" world");
    FADD;

    printf("Result = ");
    r = POP;
    PRINT(r);
    RELEASE(r);

    /*
     * Test tables
     */

    RELEASE(stack);

    return 0;
}
