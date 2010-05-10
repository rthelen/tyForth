/*
 * This file is part of arm-sim: http://madscientistroom.org/arm-sim
 *
 * Copyright (c) 2010 Randy Thelen. All rights reserved, and all wrongs
 * reversed. (See the file COPYRIGHT for details.)
 */

#include "forth.h"
#include "fobj.h"
#include "fwords.h"

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

fheader_t *dictionary_ptrs[] = {
/*
 * Bring in a list of the FWORDs defined in fprimitives.c and possibly
 * elsewhere.
 */

#include "fwords.c"

    NULL
};

void add_code_dict(fenv_t *f, fheader_t *h)
{
    fcode_new_primitive(f, fstr_new(f, h->name), h->code);

}

static void forth_test_string(const char *string)
{
    printf("Executing the Forth string: %s\n", string);

    fenv_t *f = fenv_new();

    /*
     * Add the words to the dictionary
     */

    for (int i = 0; dictionary_ptrs[i]; i++) {
        add_code_dict(f, dictionary_ptrs[i]);
    }

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
