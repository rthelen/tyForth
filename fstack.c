/*
 * This file is part of arm-sim: http://madscientistroom.org/arm-sim
 *
 * Copyright (c) 2010 Randy Thelen. All rights reserved, and all wrongs
 * reversed. (See the file COPYRIGHT for details.)
 */

#include "forth.h"
#include "fobj.h"

fstack_t *fstack_new(fenv_t *f, const char *name)
{
    fstack_t *s = malloc(sizeof(fstack_t));
    fassert(f, s != NULL, 1, "out of memory allocating a new fstack");

    s->name = name;
    s->num_elems = 0;
    s->max_elems = 0;
    s->elems = NULL;

    return s;
}

void felem_free(fenv_t *f, felem_t *p)
{
    fobj_release(f, p->obj);
    if (p->index.type == FINDEX_STR) {
        fobj_release(f, p->index.i.str);
    }
}

/*
 * felem_init()
 *
 * Is this routine useful or just a nuisance?
 */

void felem_init(fenv_t *f, felem_t *p, fobj_t *obj)
{
    if (obj) {
        fobj_retain(f, obj);
    }

    p->obj = obj;
    p->index.type = FINDEX_NONE;

}

void fstack_free(fenv_t *f, fstack_t *s)
{
    while (s->num_elems > 0) {
        felem_t elem;
        fstack_pop(f, s, &elem);
        felem_free(f, &elem);
    }

    free(s->elems);
    free(s);
}

felem_t *fstack_pop(fenv_t *f, fstack_t *s, felem_t *p)
{
    fassert(f, s->num_elems > 0, 1, "stack %s has underflowed", s->name);
    if (!p) {
        p = malloc(sizeof(felem_t *));
        fassert(f, p != NULL, 1, "out of memory allocating a new stack elem");
    }

    s->num_elems -= 1;
    *p = s->elems[s->num_elems];

    return p;
}

void fstack_push(fenv_t *f, fstack_t *s, felem_t *p)
{
    ASSERT(s->num_elems <= s->max_elems);

    if (s->num_elems == s->max_elems) {
        s->max_elems += 20;
        s->elems = realloc(s->elems, s->max_elems * sizeof(felem_t));
        fassert(f, s->elems != NULL, 1, "out of memory growing stack %s", s->name);        
    }

    s->elems[s->num_elems] = *p;
    s->num_elems += 1;
}

void fstack_push_obj(fenv_t *f, fstack_t *s, fobj_t *p)
{
    felem_t elem = { p, { FINDEX_NONE } };
    fstack_push(f, s, &elem);
}
