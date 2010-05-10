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

static void forth_test_string(const char *string)
{
    printf("Executing the Forth string: %s\n", string);

    fenv_t *f = fenv_new();
    fcode_init(f);
    f->input_str = fstr_new(f, string);
    fobj_t *token;
    do {
        int r = fparse_token(f, &token);
        if (!r) break;
        fparse_do_token(f, token);
    } while (1);

    fenv_free(f);

    printf("\n");
}

int main(int argc, char *argv[])
{
    forth_test_string("1.25 2 1.5 + + .");
    forth_test_string("{} dup 5 swap 1 ] ! 2 ] @ .");
    forth_test_string("{} const foo");
    return 0;
}
