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
#define PUSHN(n)			fpush(f, fnum_new(f, n))
#define PUSHS(s)			fpush(f, fstr_new(f, s))
#define POP()				fpop(f)
#define OVER()				fover(f)
#define DUP()				fdup(f)
#define ADD()				fadd(f)
#define SUB()				fsub(f)
#define PRINT()				fobj_print(f, fpop(f))

#define STORE()				fstore(f)
#define FETCH()				ffetch(f)
#define INDEX()				findex(f)

static void test_strings(fenv_t *f)
{
    PUSHS("Hello");
    GC;
    PUSHS(" world\n");
    GC;
    ADD();
    GC;

    printf("Result = ");
    PRINT();
    GC;

    char line[256];
    int i;
    PUSH(ftable_new(f));


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
        STORE();
        GC;
    }
    for (i = 0; i < 25; i++) {
        DUP();
        GC;
        PUSHN(i);
        GC;
        INDEX();
        GC;
        FETCH();
        GC;
        PRINT();
        GC;
    }
}

int main(int argc, char *argv[])
{
    fenv_t *f = fenv_new();

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

    ADD();
    GC;
    SUB();
    GC;

    printf("Result = ");
    PRINT();
    GC;

    test_strings(f);

    fenv_free(f);

    return 0;
}
