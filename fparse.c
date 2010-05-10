/*
 * This file is part of arm-sim: http://madscientistroom.org/arm-sim
 *
 * Copyright (c) 2010 Randy Thelen. All rights reserved, and all wrongs
 * reversed. (See the file COPYRIGHT for details.)
 */

#include "forth.h"
#include "fobj.h"

/**********************************************************
 *
 * Input Processing Routines
 *
 **********************************************************/

static int fparse_int(fenv_t *f, char delim, fobj_t **token) // delim --
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

int fparse_token(fenv_t *f, fobj_t **token_str)
{
    do {
        int c = fstr_getchar(f, f->input_str, f->input_offset);
        if (isspace(c)) f->input_offset ++;
        else break;
    } while (1);

    /*
     * Parse for the next white space delimited token
     */

    (void) fparse_int(f, ' ', token_str);

    if (*token_str == NULL) {
        return 0;
    } else {
        return 1;
    }
}

static int fparse_token_to_number(fenv_t *f, fobj_t *token, fnumber_t *n)
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

    /*
     * See if the number is a float
     */

    *n = strtold(buf, &p);
    if (*p == '\0') return 1;

    /*
     * See if the number is a hex or octal number
     */

    *n = strtoll(buf, &p, 0);
    if (*p == '\0') return 1;
    else            return 0;
}


/*
 * This function is a hack.  The whole of this functionality will move to
 * fcode.c as a function like fcode_compile_string() or something. :-)
 *
 * Until then there's a hack extern to allow the compiler to generate more
 * or less proper code.
 */

void fparse_do_token(fenv_t *f, fobj_t *token)
{
    fobj_t *val  = ftable_fetch(f, f->words, token);
    if (val) {
        fcode_t code = val->u.word.code;
        code(f, NULL);
    } else {
        fnumber_t n = 0;
        int r = fparse_token_to_number(f, token, &n);
        fassert(f, r, 1, "Word %s not found in the dictionary", token->u.str.buf);
        fobj_t *num = fnum_new(f, n);
        extern void fcode_push(void *, void *, void *);
        fcode_push(f, NULL, num);
    }
}

