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

void fhash_free(fenv_t *f, fobj_t *p)
{
    fhash_t *h = &p->u.hash;

    for (int i = 0; i < h->num_kv; i++) {
        fobj_release(f, h->keys[i]);
        fobj_release(f, h->vals[i]);
    }

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
    h->keys[h->num_kv] = fobj_retain(f, key);
    h->vals[h->num_kv] = fobj_retain(f, val);
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

static fobj_t *fhash_obj_fetch(fenv_t *f, fobj_t **p)
{
    if (!p) {
        return NULL;
    }

    return fobj_retain(f, *p);
}

static fobj_t *fhash_key_fetch(fenv_t *f, fhash_t *h, fobj_t *key)
{
    return fhash_obj_fetch(f, fhash_key_index(f, h, key));
}

static void fhash_obj_store(fenv_t *f, fobj_t **valp, fobj_t *data)
{
    ASSERT(valp);

    if (*valp) {
        fobj_release(f, *valp);
    }

    *valp = fobj_retain(f, data);
}

static void fhash_key_store(fenv_t *f, fhash_t *h, fobj_t *key, fobj_t *data)
{
    fobj_t **valp = fhash_key_index(f, h, key);

    if (valp) {
        fhash_obj_store(f, valp, data);
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

