/*
 * This file is part of arm-sim: http://madscientistroom.org/arm-sim
 *
 * Copyright (c) 2010 Randy Thelen. All rights reserved, and all wrongs
 * reversed. (See the file COPYRIGHT for details.)
 */

#include "forth.h"
#include "fobj.h"

#define KEY(_h, _i)		(_h)->keys_values[2 * (_i) + 0]
#define VAL(_h, _i)		(_h)->keys_values[2 * (_i) + 1]

void fhash_print(fenv_t *f, fobj_t *p)
{
    ASSERT(p->type == FOBJ_HASH);
    fhash_t *h = &p->u.hash;

    for (int i = 0; i < h->num_kv; i++) {
        fobj_print(f, KEY(h, i));
        printf(" = ");
        fobj_print(f, VAL(h, i));
    }
}

fobj_t *fhash_new(fenv_t *f)
{
    fobj_t *p = fobj_new(f, FOBJ_HASH);
    fhash_t *h = &p->u.hash;

    h->num_kv = 0;
    h->keys_values = NULL;

    return p;
}

void fhash_visit(fenv_t *f, fobj_t *p)
{
    fhash_t *h = &p->u.hash;
    for (int i = 0; i < h->num_kv; i++) {
        fobj_visit(f, KEY(h, i));
        fobj_visit(f, VAL(h, i));
    }
}

void fhash_free(fenv_t *f, fobj_t *p)
{
    fhash_t *h = &p->u.hash;

    if (h->keys_values) {
        free(h->keys_values);
    }
}

static void fhash_add_key_val(fenv_t *f, fhash_t *h, fobj_t *key, fobj_t *val)
{
    int n = 2 * h->num_kv + 2;
    h->keys_values = realloc(h->keys_values, n * sizeof(fobj_t **));
    KEY(h, h->num_kv) = key;
    VAL(h, h->num_kv) = val;
    h->num_kv ++;
}

static fobj_t **fhash_key_index(fenv_t *f, fhash_t *h, fobj_t *key)
{
    for (int i = 0; i < h->num_kv; i++) {
        ASSERT(KEY(h, i)->type == FOBJ_STR);
        if (strcmp(key->u.str.buf, KEY(h, i)->u.str.buf) == 0) {
            return &VAL(h, i);
        }
    }

    return NULL;
}

static fobj_t *fhash_key_fetch(fenv_t *f, fhash_t *h, fobj_t *key)
{
    return *fhash_key_index(f, h, key);
}

static void fhash_key_store(fenv_t *f, fhash_t *h, fobj_t *key, fobj_t *data)
{
    fobj_t **valp = fhash_key_index(f, h, key);

    if (valp) {
        *valp = data;
    } else {
        fhash_add_key_val(f, h, key, data);
    }
}

fobj_t *fhash_fetch(fenv_t *f, fobj_t *addr, fobj_t *index)
{
    fhash_t *h = &addr->u.hash;

    if (!index) {
        return fnum_new(f, h->num_kv);
    }

    return fhash_key_fetch(f, h, index);
}

void fhash_store(fenv_t *f, fobj_t *addr, fobj_t *index, fobj_t *data)
{
    fassert(f, !!index, 1, "hash store must be indexed");
    fassert(f, index->type == FOBJ_STR, 1, "hash store must be indexed by STRING");

    fhash_t *h = &addr->u.hash;

    fhash_key_store(f, h, index, data);
}

