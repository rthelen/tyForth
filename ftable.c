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
    fobj_t **array = t->array;
    for (int i = t->i_start; i < t->i_limit; i++) {
        printf("array[%d] = ", i);
        fobj_print(f, *array++);
    }

    fobj_t **keys, **vals;
    keys = t->keys;
    vals = t->vals;
    for (int i = 0; i < t->num_kv_pairs; i++) {
        fobj_print(f, *keys++);
        printf(" = ");
        fobj_print(f, *vals++);
    }
}


void ftable_set_i_val(fenv_t *f, ftable_t *t, int i, fobj_t *val)
{
    fobj_retain(f, val);

    if (i < t->i_start) {
        int n = t->i_limit - i;
        t->array = realloc(t->array, n * sizeof(fobj_t **));
        n = t->i_start - i;
        memmove(&t->array[n], &t->array[0], n * sizeof(fobj_t **));
        bzero(&t->array[0], n * sizeof(fobj_t **));
        t->i_start = i;
        t->array[0] = val;
    } else if (i >= t->i_limit) {
        int n = t->i_limit + 1 - i;
        t->array = realloc(t->array, n * sizeof(fobj_t **));
        bzero(&t->array[t->i_limit], n * sizeof(fobj_t **));
        t->i_limit = i + 1;
        t->array[i] = val;
    } else {
        ASSERT(i >= t->i_start && i < t->i_limit);
        if (t->array[i]) {
            fobj_release(f, t->array[i]);
        }
        t->array[i] = val;
    }
}

static int ftable_key_lookup(fenv_t *f, ftable_t *t, fstr_t *s, fobj_t ***key, fobj_t ***val)
{
    for (int i = 0; i < t->num_kv_pairs; i++) {
        ASSERT(t->keys[i]->type == FOBJ_STR);
        if (strcmp(s->buf, t->keys[i]->u.str.buf) == 0) {
            *key = &t->keys[i];
            *val = &t->vals[i];
            return 1;
        }
    }

    return 0;
}

void ftable_set_key_val(fenv_t *f, ftable_t *t, fobj_t *key, fobj_t *val)
{
    /* Remember: key's must be strings */
    ASSERT(key->type == FOBJ_STR);
    fobj_t **keyp, **valp;
    if (ftable_key_lookup(f, t, &key->u.str, &keyp, &valp)) {
        fobj_release(f, *valp);
        fobj_retain(f, val);
        *valp = val;
    } else {
        int i = t->num_kv_pairs;
        t->num_kv_pairs += 1;
        t->keys = realloc(t->keys, sizeof(fobj_t *) * t->num_kv_pairs);
        t->vals = realloc(t->vals, sizeof(fobj_t *) * t->num_kv_pairs);
        fobj_retain(f, key);
        fobj_retain(f, val);
        t->keys[i] = key;
        t->vals[i] = val;
    }
}

fobj_t *ftable_new(fenv_t *f, const ftable_t *t)
{
    fobj_t *p = fobj_new(f, FOBJ_TABLE);
    ftable_t *t2 = &p->u.table;

    if (t) {
        /*
         * Copy table to the newly allocated table
         */
        t2->i_start = t->i_start;
        t2->i_limit = t->i_limit;

        int n = t2->i_limit - t2->i_start;
        t2->array = malloc(sizeof(fobj_t **) * n);

        for (int i = 0; i < t2->i_limit - t2->i_start; i++) {
            t2->array[i] = t->array[i];
            fobj_retain(f, t2->array[i]);
        }

        t2->num_kv_pairs = t->num_kv_pairs;
        t2->keys = malloc(sizeof(fobj_t **) * t2->num_kv_pairs);
        t2->vals = malloc(sizeof(fobj_t **) * t2->num_kv_pairs);
        for (int i = 0; i < t2->num_kv_pairs; i++) {
            t2->keys[i] = t->keys[i];
            t2->vals[i] = t->keys[i];
            fobj_retain(f, t2->keys[i]);
            fobj_retain(f, t2->vals[i]);
        }
    } else {
        t2->i_start = 0;
        t2->i_limit = 0;
        t2->array = NULL;

        t2->num_kv_pairs = 0;
        t2->keys = NULL;
        t2->vals = NULL;
    }

    return p;
}

void ftable_free(fenv_t *f, fobj_t *p)
{
    ftable_t *t = &p->u.table;
    fobj_t **array = t->array;
    for (int i = t->i_start; i < t->i_limit; i++) {
        fobj_release(f, *array++);
    }

    fobj_t **keys, **vals;
    keys = t->keys;
    vals = t->vals;
    for (int i = 0; i < t->num_kv_pairs; i++) {
        fobj_release(f, *keys++);
        fobj_release(f, *vals++);
    }
}

/*
 * Currently the keys are in as-added order.  That means on average lookups
 * consume time propotional to the number of keys present.  :-(
 */

/*
 * ftable_add()
 *
 * Supported operand types (for op2):
 *    - String: concatenate the two strings
 *    - Number: index into string
 */

/*
 * ftable_concatenate()
 *
 * Add all the array elements of op2 to the end of the array elements of
 * op1.  Then, copy all of the keys and values from op2 to op1.  But, use
 * the ftable_set_key_val() routine so that any duplicate entries are
 * properly replaced.  Thus, to avoid leaks.
 */
void ftable_concatenate(fenv_t *f, felem_t *dest, felem_t *op1, felem_t *op2)
{
    int ftable_implemented = 0;
    ASSERT(ftable_implemented);
}

int ftable_add(fenv_t *f, felem_t *dest, felem_t *op1, felem_t *op2)
{
    ASSERT(op1->obj->type == FOBJ_TABLE);

    switch(op2->obj->type) {
    case FOBJ_STR:
        felem_init(f, dest, op1);
        felem_clear_index(f, dest, FINDEX_STR);
        fobj_retain(f, op2->obj);
        dest->index.i.str = op2->obj;
        return 1;

    case FOBJ_NUM:
        felem_init(f, dest, op1);
        felem_clear_index(f, dest, FINDEX_NUM);
        dest->index.i.num.n += op2->obj->u.num.n;
        return 1;

    default:
        return 0;
    }
}

/*
 * ftable_sub()
 *
 * Supported operand types (for op2):
 *    - String: compare the two strings (leave a number)
 *    - Number: index into string
 */

int ftable_sub(fenv_t *f, felem_t *dest, felem_t *op1, felem_t *op2)
{
    ASSERT(op1->obj->type == FOBJ_TABLE);

    printf("String Sub\n");

    switch(op2->obj->type) {
    default:
        return 0;
    }
}
