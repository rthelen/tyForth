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

#define GC					fobj_garbage_collection(f)

#define PUSH(x)				fpush(f, x)
#define PUSHN(n)			PUSH(fnum_new(f, n))
#define PUSHS(s)			PUSH(fstr_new(f, s))
#define POP					fpop(f)
#define OVER()				fover(f)
#define DUP()				fdup(f)
#define ADD(a, b)			fobj_add(f, a, b)
#define SUB(a, b)			fobj_sub(f, a, b)
#define STORE(a,i,d)		fobj_store(f, a, i, d)
#define FETCH(a,i)			fobj_fetch(f, a, i)
#define PRINT(x)			fobj_print(f, x)

#define FADD				fadd(f)
#define FSUB				fsub(f)
#define FSTORE()			fstore(f)
#define FFETCH()			ffetch(f)

void fadd(fenv_t *f)
{
    fobj_t *op2 = POP;
    fobj_t *op1 = POP;

    PUSH(ADD(op1, op2));
}

void fsub(fenv_t *f)
{
    fobj_t *op2 = POP;
    fobj_t *op1 = POP;

    PUSH(SUB(op1, op2));
}

fobj_t *fpop(fenv_t *f)
{
    return fstack_fetch(f, f->dstack, NULL);
}

void fpush(fenv_t *f, fobj_t *p)
{
    fstack_store(f, f->dstack, NULL, x);
}

void fover(fenv_t *f)
{
    fobj_t *a, *b;
    a = POP;
    b = POP;
    PUSH(b);
    PUSH(a);
    PUSH(b);
}

void fdup(fenv_t *f)
{
    fobj_t *a;
    a = POP;
    PUSH(a);
    PUSH(a);
}

void ffetch(fenv_t *f)
{
    fobj_t *addr = POP;
    fobj_t *index = NULL;
    if (fobj_is_index(f, addr)) {
        index = addr->u.index.index;
        addr =  addr->u.index.addr;
    }

    PUSH(fobj_fetch(f, addr, index));
}

void fstore(fenv_t *f)
{
    fobj_t *addr = POP;
    fobj_t *index = NULL;
    if (fobj_is_index(f, addr)) {
        index = addr->u.index.index;
        addr =  addr->u.index.addr;
    }
    fobj_t *data = POP;

    fobj_store(f, addr, index, data);
}

static void test_strings(fenv_t *f)
{
    PUSHS("Hello");
    GC;
    PUSHS(" world\n");
    GC;
    FADD;
    GC;

    printf("Result = ");
    PRINT(POP);
    GC;

    char line[256];
    int i;
    PUSH(ftable_new(f));

#define INDEX()	do { fobj_t *i = POP; fobj_t *a = POP; PUSH(findex_new(f, a, i)); } while (0)

    for (i = 0; i < 25; i++) {
        snprintf(line, sizeof(line), "Test %d\n", i);
        
        PUSHS(line);
        GC;
        OVER();
        GC;
        PUSHN(i);
        GC;
        INDEX();
        GC;
        FSTORE();
        GC;
    }
    for (i = 0; i < 25; i++) {
        DUP();
        GC;
        PUSHN(i);
        GC;
        INDEX();
        GC;
        FFETCH();
        GC;
        PRINT(POP);
        GC;
    }
}

int main(int argc, char *argv[])
{
    fenv_t myEnv;
    fenv_t *f = fenv_init(&myEnv);

    /*
     * Test arithmetic
     */
    GC;
    PUSHN(2);
    GC;
    PUSHN(5);
    GC;
    PUSHN(8);
    GC;

    FADD;
    GC;
    FSUB;
    GC;

    printf("Result = ");
    PRINT(POP);
    GC;

    test_strings(f);

    return 0;
}
