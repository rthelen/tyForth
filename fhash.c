/*
 * This file is part of arm-sim: http://madscientistroom.org/arm-sim
 *
 * Copyright (c) 2010 Randy Thelen. All rights reserved, and all wrongs
 * reversed. (See the file COPYRIGHT for details.)
 */

#include "forth.h"
#include "fobj.h"

void fhash_print(fenv_t *f, fobj_t *p)
{
    ASSERT(p->type == FOBJ_HASH);
    fhash_t *h = &p->u.hash;

    for (int i = 0; i < h->num_kv; i++) {
        fobj_print(f, h->keys[i]);
        printf(" = ");
        fobj_print(f, h->vals[i]);
    }
}

fobj_t *fhash_new(fenv_t *f)
{
    fobj_t *p = fobj_new(f, FOBJ_HASH);
    fhash_t *h = &p->u.hash;

    h->num_kv = 0;
    h->keys = NULL;
    h->vals = NULL;

    return p;
}

void fhash_visit(fenv_t *f, fobj_t *p)
{
    fhash_t *h = &p->u.hash;
    for (int i = 0; i < h->num_kv; i++) {
        fobj_visit(f, h->keys[i]);
        fobj_visit(f, h->vals[i]);
    }
}

void fhash_free(fenv_t *f, fobj_t *p)
{
    fhash_t *h = &p->u.hash;

    if (h->keys) {
        free(h->keys);
        free(h->vals);
    }
}

static void fhash_add_key_val(fenv_t *f, fhash_t *h, fobj_t *key, fobj_t *val)
{
    int n = h->num_kv + 1;
    h->keys = realloc(h->keys, n * sizeof(fobj_t **));
    h->vals = realloc(h->vals, n * sizeof(fobj_t **));
    h->keys[h->num_kv] = key;
    h->vals[h->num_kv] = val;
    h->num_kv ++;
}

static fobj_t **fhash_key_index(fenv_t *f, fhash_t *h, fobj_t *key)
{
    for (int i = 0; i < h->num_kv; i++) {
        ASSERT(h->keys[i]->type == FOBJ_STR);
        if (strcmp(key->u.str.buf, h->keys[i]->u.str.buf) == 0) {
            return &h->vals[i];
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

