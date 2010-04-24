/*
 * This file is part of arm-sim: http://madscientistroom.org/arm-sim
 *
 * Copyright (c) 2010 Randy Thelen. All rights reserved, and all wrongs
 * reversed. (See the file COPYRIGHT for details.)
 */

#include "forth.h"
#include "fobj.h"

void ftable_print(fenv_t *f, fobj_t *p)
{
    ASSERT(p->type == FOBJ_TABLE);
    ftable_t *t = &p->u.table;

    farray_print(f, t->array);
    fhash_print(f, t->hash);
}

fobj_t *ftable_new(fenv_t *f)
{
    fobj_t *p = fobj_new(f, FOBJ_TABLE);
    ftable_t *t = &p->u.table;

    t->array = farray_new(f);
    t->hash = fhash_new(f);

    return p;
}

void ftable_free(fenv_t *f, fobj_t *p)
{
    ftable_t *t = &p->u.table;

    farray_free(f, t->array);
    fhash_free(f, t->hash);
}

/***********************************
 *
 * ftable_fetch()
 *
 ***********************************/

fobj_t *ftable_fetch(fenv_t *f, fobj_t *addr, fobj_t *index)
{
    ftable_t *t = &addr->u.table;

    if (index) {
        switch(index->type) {
        case FOBJ_NUM:
            fassert(f, t->array != NULL, 1, "Can't index an empty array");
            return farray_fetch(f, t->array, index);

        case FOBJ_STR:
            fassert(f, t->hash != NULL, 1, "Can't index an empty hash");
            return fhash_fetch(f, t->hash, index);

        default:
            fassert(f, 0, 1, "An indexed table store must be indexed by either a NUM or STRING");
            return NULL;
        }
    } else {
        /*
         * Return the number of integer elements in the table.  I.e., it's
         * the array size if the table is used as an array.
         */

        if (!t->array) {
            return fnum_new(f, 0);
        } else {
            return fnum_new(f, farray_fetch(f, t->hash, NULL)->u.num.n);
        }
    }
}

/***********************************
 *
 * ftable_store()
 *
 ***********************************/

void ftable_store(fenv_t *f, fobj_t *addr, fobj_t *index, fobj_t *data)
{
    fassert(f, !!index, 1, "table store must be indexed");
    ftable_t *t = &addr->u.table;

    switch(index->type) {
    case FOBJ_NUM:
        fassert(f, t->array != NULL, 1, "Can't index an empty hash");
        farray_store(f, t->array, index, data);
        break;

    case FOBJ_STR:
        fassert(f, t->hash != NULL, 1, "Can't index an empty hash");
        fhash_store(f, t->hash, index, data);
        break;

    default:
        fassert(f, 0, 1, "table store must be indexed by NUM or STRING");
    }
}
