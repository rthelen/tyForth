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

forth_header_t *dictionary_ptrs[] = {
/*
 * Bring in a list of the FWORDs defined in fprimitives.c and possibly
 * elsewhere.
 */

#include "fwords.c"

    NULL
};

void add_code_dict(fenv_t *f, forth_header_t *h)
{
    fobj_t *name = fstr_new(f, h->name);
    fobj_t *code = fcode_new(f, h->code);

    ftable_store(f, f->words, name, code);
}

int main(int argc, char *argv[])
{
    fenv_t *f = fenv_new();

    /*
     * Add the words to the dictionary
     */

    for (int i = 0; dictionary_ptrs[i]; i++) {
        add_code_dict(f, dictionary_ptrs[i]);
    }

    fenv_free(f);

    return 0;
}
