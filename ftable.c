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

    for (int i = 0; i < t->num; i++) {
        printf("array[%d] = ", i);
        fobj_print(f, t->array[i]);
    }

    for (int i = 0; i < t->num_kv; i++) {
        fobj_print(f, t->keys[i]);
        printf(" = ");
        fobj_print(f, t->vals[i]);
    }
}

static void ftable_grow_array(fenv_t *f, ftable_t *t, int n)
{
    ASSERT(n > t->num);
    t->array = realloc(t->array, n * sizeof(fobj_t **));
    bzero(&t->array[t->num], (n - t->num) * sizeof(fobj_t **));
    t->num = n;
}

static void ftable_add_key_val(fenv_t *f, ftable_t *t, fobj_t *key, fobj_t *val)
{
    int n = t->num_kv + 1;
    t->keys = realloc(t->keys, n * sizeof(fobj_t **));
    t->vals = realloc(t->vals, n * sizeof(fobj_t **));
    t->keys[t->num_kv] = fobj_retain(f, key);
    t->vals[t->num_kv] = fobj_retain(f, val);
    t->num_kv ++;
}

static fobj_t **ftable_copy_array(fenv_t *f, int n, fobj_t **src_array)
{
    if (!n) {
        return NULL;
    }

    fobj_t **dest_array = malloc(sizeof(fobj_t **) * n);
    fassert(f, !!dest_array, 1, "allocating a new table array");

    for (int i = 0; i < n; i++) {
        dest_array[i] = fobj_retain(f, src_array[i]);
    }

    return dest_array;
}

fobj_t *ftable_new(fenv_t *f, const ftable_t *t)
{
    fobj_t *p = fobj_new(f, FOBJ_TABLE);
    ftable_t *t2 = &p->u.table;

    if (t) {
        /*
         * Copy table to the newly allocated table
         */
        t2->num = t->num;
        t2->array = ftable_copy_array(f, t->num, t->array);

        t2->num_kv = t->num_kv;
        t2->keys = ftable_copy_array(f, t->num_kv, t->keys);
        t2->vals = ftable_copy_array(f, t->num_kv, t->vals);
    } else {
        t2->num = 0;
        t2->array = NULL;

        t2->num_kv = 0;
        t2->keys = NULL;
        t2->vals = NULL;
    }

    return p;
}

void ftable_free(fenv_t *f, fobj_t *p)
{
    ftable_t *t = &p->u.table;

    for (int i = 0; i < t->num; i++) {
        fobj_release(f, t->array[i]);
    }

    if (t->array) {
        free(t->array);
    }

    for (int i = 0; i < t->num_kv; i++) {
        fobj_release(f, t->keys[i]);
        fobj_release(f, t->vals[i]);
    }

    if (t->keys) {
        free(t->keys);
        free(t->vals);
    }
}

static fobj_t **ftable_num_index(fenv_t *f, ftable_t *t, fnumber_t n)
{
    if (n < 0) return NULL;
    if (n >= t->num) return NULL;

    return t->array + (int) n;
}

static fobj_t **ftable_key_index(fenv_t *f, ftable_t *t, fobj_t *key)
{
    for (int i = 0; i < t->num_kv; i++) {
        ASSERT(t->keys[i]->type == FOBJ_STR);
        if (strcmp(key->u.str.buf, t->keys[i]->u.str.buf) == 0) {
            return &t->vals[i];
        }
    }

    return NULL;
}

/***********************************
 *
 * ftable_fetch()
 *
 ***********************************/

static fobj_t *ftable_obj_fetch(fenv_t *f, fobj_t **p)
{
    if (!p) {
        return NULL;
    }

    return fobj_retain(f, *p);
}

static fobj_t *ftable_num_fetch(fenv_t *f, ftable_t *t, fnumber_t n)
{
    return ftable_obj_fetch(f, ftable_num_index(f, t, n));
}

static fobj_t *ftable_key_fetch(fenv_t *f, ftable_t *t, fobj_t *key)
{
    return ftable_obj_fetch(f, ftable_key_index(f, t, key));
}

fobj_t *ftable_fetch(fenv_t *f, fobj_t *addr, fobj_t *index)
{
    if (index) {
        switch(index->type) {
        case FOBJ_NUM:
            return ftable_num_fetch(f, &addr->u.table, index->u.num.n);

        case FOBJ_STR:
            return ftable_key_fetch(f, &addr->u.table, index);

        default:
            fassert(f, 0, 1, "An indexed table store must be indexed by either a NUM or STRING");
            return NULL;
        }
    } else {
        /*
         * Return the number of integer elements in the table.  I.e., it's
         * the array size if the table is used as an array.
         */
        return fnum_new(f, addr->u.table.num);
    }
}

/***********************************
 *
 * ftable_store()
 *
 ***********************************/

static void ftable_obj_store(fenv_t *f, fobj_t **valp, fobj_t *data)
{
    ASSERT(valp);

    if (*valp) {
        fobj_release(f, *valp);
    }

    *valp = fobj_retain(f, data);
}

static void ftable_num_store(fenv_t *f, ftable_t *t, fnumber_t n, fobj_t *data)
{
    fobj_t **valp = ftable_num_index(f, t, n);

    if (!valp) {
        ftable_grow_array(f, t, n+1);
        valp = ftable_num_index(f, t, n);
        ASSERT(valp);
    }

    ftable_obj_store(f, valp, data);
}

static void ftable_key_store(fenv_t *f, ftable_t *t, fobj_t *key, fobj_t *data)
{
    fobj_t **valp = ftable_key_index(f, t, key);

    if (valp) {
        ftable_obj_store(f, valp, data);
    } else {
        ftable_add_key_val(f, t, key, data);
    }
}

void ftable_store(fenv_t *f, fobj_t *addr, fobj_t *index, fobj_t *data)
{
    fassert(f, !!index, 1, "table store must be indexed");

    switch(index->type) {
    case FOBJ_NUM:
        ftable_num_store(f, &addr->u.table, index->u.num.n, data);

    case FOBJ_STR:
        ftable_key_store(f, &addr->u.table, index, data);

    default:
        fassert(f, 0, 1, "table store must be indexed by NUM or STRING");
    }
}

/********************************************
 *
 * push and pop operations on a table
 *
 */

static fobj_t *ftable_get_sp(fenv_t *f, fobj_t *stack)
{
    ftable_t *t = &stack->u.table;
    fobj_t *sp_str = fstr_new(f, "sp");
    fobj_t *sp = ftable_key_fetch(f, t, sp_str);
    if (!sp) {
        sp = fnum_new(f, 0);
        ftable_key_store(f, t, sp_str, sp);
    }
    fobj_release(f, sp_str);
    return sp;
}

static void ftable_set_sp(fenv_t *f, fobj_t *stack, fobj_t *sp)
{
    ftable_t *t = &stack->u.table;
    fobj_t *sp_str = fstr_new(f, "sp");
    ftable_key_store(f, t, sp_str, sp);
    fobj_release(f, sp_str);
}


void ftable_push(fenv_t *f, fobj_t *stack, fobj_t *data)
{
    fassert(f, stack->type == FOBJ_TABLE, 1, "Wrong type");
    ftable_t *t = &stack->u.table;
    fobj_t *sp = ftable_get_sp(f, stack);
    ftable_num_store(f, t, sp->u.num.n++, data);
    ftable_set_sp(f, stack, sp);
}

fobj_t *ftable_pop(fenv_t *f, fobj_t *stack)
{
    fassert(f, stack->type == FOBJ_TABLE, 1, "Wrong type");
    ftable_t *t = &stack->u.table;
    fassert(f, t->num > 0, 1, "stack underflow");

    fobj_t *sp = ftable_get_sp(f, stack);
    fobj_t *result = ftable_num_fetch(f, t, --sp->u.num.n);
    ftable_set_sp(f, stack, sp);

    return result;
}
