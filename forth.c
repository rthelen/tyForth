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

/**********************************************************
 *
 * Input Processing Routines
 *
 **********************************************************/

static int forth_parse(fenv_t *f, char delim, fobj_t **token) // delim --
{
    int start = f->input_offset;
    char *buf = f->input_str->u.str.buf + start;
    
    do {
        int len = f->input_offset - start;
        int c = fstr_getchar(f, f->input_str, f->input_offset++);

        if ((c == EOF) || (delim == ' ' && isspace(c)) || (delim == c)) {

            if (len == 0) {
                *token = NULL;
                return 0;
            }

            *token = fstr_new_buf(f, buf, len);
            if (c == EOF) {
                return 0; // Didn't find the delimiter
            } else {
                return 1; // Found the delimiter
            }
        }
    } while (1);
}

static int forth_token(fenv_t *f, fobj_t **token_str)
{
    do {
        int c = fstr_getchar(f, f->input_str, f->input_offset);
        if (isspace(c)) f->input_offset ++;
        else break;
    } while (1);

    /*
     * Parse for the next white space delimited token
     */

    (void) forth_parse(f, ' ', token_str);

    if (*token_str == NULL) {
        return 0;
    } else {
        return 1;
    }
}

static int forth_number_token(fenv_t *f, fobj_t *token, fnumber_t *n)
{
    int i, len;
    char *p, *buf;

    if (token->type != FOBJ_STR) {
        return 0;
    }

    buf = token->u.str.buf;
    len = token->u.str.len;

    if (len == 8) {
        for (i = 0; i < 8; i++) {
            if (!isxdigit(buf[i]))
                break;
        }

        if (i == 8) {
            // Assume it's a hex humber
            *n = strtoll(buf, NULL, 16);
            return 1;
        }
    }

    // This doesn't properly handle floating point numbers.  :-(
    *n = strtoll(buf, &p, 0);
    if (*p == '\0') return 1;
    else            return 0;
}


static void forth_do_token(fenv_t *f, fobj_t *token)
{
    fobj_t *val  = ftable_fetch(f, f->words, token);
    if (val) {
        fcode_t code = val->u.code;
        code(f, NULL);
    } else {
        void *w = NULL;  // for PUSHN()
        fnumber_t n = 0;
        int r = forth_number_token(f, token, &n);
        fassert(f, r, 1, "Word %s not found in the dictionary", token->u.str.buf);
        PUSHN(n);
    }
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

    f->input_str = fstr_new(f, "1 2 + .");
    fobj_t *token;
    do {
        int r = forth_token(f, &token);
        if (!r) break;
        forth_do_token(f, token);
    } while (1);

    fenv_free(f);

    return 0;
}
