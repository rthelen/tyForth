/*
 * This file is part of arm-sim: http://madscientistroom.org/arm-sim
 *
 * Copyright (c) 2010 Randy Thelen. All rights reserved, and all wrongs
 * reversed. (See the file COPYRIGHT for details.)
 */

#include "forth.h"
#include "fobj.h"

void fstack_print(fenv_t *f, fobj_t *p)
{
    ASSERT(p->type == FOBJ_STACK);
    fstack_t *s = &p->u.stack;

    printf("Depth = %d\n", s->sp);
}

fobj_t *fstack_new(fenv_t *f)
{
    fobj_t *p = fobj_new(f, FOBJ_STACK);
    fstack_t *s = &p->u.stack;

    s->sp = 0;
    s->max_sp = 0;
    s->elems = NULL;

    return p;
}

void fstack_visit(fenv_t *f, fobj_t *p)
{
    fstack_t *s = &p->u.stack;
    for (int i = 0; i < s->sp; i++) {
        fobj_visit(f, s->elems[i]);
    }
}

void fstack_free(fenv_t *f, fobj_t *p)
{
    fstack_t *s = &p->u.stack;

    if (s->elems) {
        free(s->elems);
    }
}

static void fstack_grow(fenv_t *f, fstack_t *s)
{
    s->max_sp += 32;
    s->elems = realloc(s->elems, s->max_sp * sizeof(fobj_t **));
    for (int i = s->sp; i < s->max_sp; i++) {
        s->elems[i] = NULL;
    }
}

fobj_t *fstack_fetch(fenv_t *f, fobj_t *addr, fobj_t *index)
{
    fstack_t *s = &addr->u.stack;
    FASSERT(!index, "indexed fetch of stack not supported");  // Yet.
    FASSERT(s->sp > 0, "stack underflow error");
    return HOLD(s->elems[--s->sp]);
}

void fstack_store(fenv_t *f, fobj_t *addr, fobj_t *index, fobj_t *data)
{
    FASSERT(!index, "indexed store of stack not supported");  // Yet.
    fstack_t *s = &addr->u.stack;
    ASSERT(s->sp <= s->max_sp);
    ASSERT(s->sp >= 0);
    if (s->sp == s->max_sp) {
        fstack_grow(f, s);
    }
    s->elems[s->sp++] = data;
}
