/*
 * This file is part of arm-sim: http://madscientistroom.org/arm-sim
 *
 * Copyright (c) 2010 Randy Thelen. All rights reserved, and all wrongs
 * reversed. (See the file COPYRIGHT for details.)
 */

#include "forth.h"
#include "fobj.h"

void fstr_print(fenv_t *f, fobj_t *p)
{
    if (p->u.str.buf) {
        printf("    String = \"%s\"\n", p->u.str.buf);
    } else {
        printf("    String = (null)\n");
    }
}

fobj_t *fstr_new(fenv_t *f, const char *str)
{
    fobj_t *p = fobj_new(f, FOBJ_STR);
    p->u.str.len = strlen(str);
    p->u.str.buf = malloc(p->u.str.len + 1);
    p->u.str.buf[0] = 0;
    strcat(p->u.str.buf, str);
    return p;
}

void fstr_free(fenv_t *f, fobj_t *p)
{
    if (p->u.str.buf) {
        free(p->u.str.buf);
        p->u.str.buf = NULL;
    }
    p->u.str.len = 0;
}

/*
 * fstr_add()
 *
 * Supported operand types (for op2):
 *    - String: concatenate the two strings
 *    - Number: index into string
 */

static void fstr_concatenate(fenv_t *f, felem_t *dest, felem_t *op1, felem_t *op2)
{
    dest->obj = fstr_new(f, op1->obj->u.str.buf);
    dest->obj->u.str.len += op2->obj->u.str.len;
    dest->obj->u.str.buf = realloc(dest->obj->u.str.buf, dest->obj->u.str.len + 1);
    strcat(dest->obj->u.str.buf, op2->obj->u.str.buf);
}

static void fstr_index(fenv_t *f, felem_t *dest, felem_t *op1, felem_t *op2)
{
    felem_init(f, dest, op1);

    ASSERT(dest->index.type == FINDEX_NONE ||
           dest->index.type == FINDEX_NUM);

    if (dest->index.type == FINDEX_NONE) {
        dest->index.type = FINDEX_NUM;
        dest->index.i.num.n = 0;
    }
    dest->index.i.num.n += op2->obj->u.num.n;
}

static void fstr_compare(fenv_t *f, felem_t *dest, felem_t *op1, felem_t *op2)
{
    int r = strcmp(op1->obj->u.str.buf, op2->obj->u.str.buf);
    fobj_t *obj = fnum_new(f, r);

    felem_init(f, dest, NULL);
    dest->obj = obj;
}

int fstr_add(fenv_t *f, felem_t *dest, felem_t *op1, felem_t *op2)
{
    ASSERT(op1->obj->type == FOBJ_STR);

    switch(op2->obj->type) {
    case FOBJ_STR:
        fstr_concatenate(f, dest, op1, op2);
        return 1;

    case FOBJ_NUM:
        fstr_index(f, dest, op1, op2);
        return 1;

    default:
        return 0;
    }
}

/*
 * fstr_sub()
 *
 * Supported operand types (for op2):
 *    - String: compare the two strings (leave a number)
 *    - Number: index into string
 */

int fstr_sub(fenv_t *f, felem_t *dest, felem_t *op1, felem_t *op2)
{
    ASSERT(op1->obj->type == FOBJ_STR);

    printf("String Sub\n");

    switch(op2->obj->type) {
    case FOBJ_STR:
        fstr_compare(f, dest, op1, op2);
        return 1;

    case FOBJ_NUM:
        fstr_index(f, dest, op1, op2);
        return 1;

    default:
        return 0;
    }
}
