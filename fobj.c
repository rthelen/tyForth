/*
 * This file is part of arm-sim: http://madscientistroom.org/arm-sim
 *
 * Copyright (c) 2010 Randy Thelen. All rights reserved, and all wrongs
 * reversed. (See the file COPYRIGHT for details.)
 */

#include "forth.h"
#include "fobj.h"


const foptable_t op_table[FOBJ_NUM_TYPES] = {
    { 0 }, // The zeroth entry is INVALID
    { "number", NULL, NULL, fnum_print, NULL, NULL, fnum_add, fnum_sub },
    { "string", NULL, fstr_free, fstr_print, NULL, fstr_fetch, fstr_add, fstr_sub },
    { "table",  ftable_visit, NULL, ftable_print, ftable_store, ftable_fetch },
    { "array",  farray_visit, farray_free, farray_print, farray_store, farray_fetch },
    { "hash",   fhash_visit,  fhash_free, fhash_print,  fhash_store, fhash_fetch },
    { "stack",  fstack_visit, fstack_free, fstack_print, fstack_store, fstack_fetch },
    { "index",  findex_visit, NULL, NULL, NULL, NULL },
};

#define NUM_OBJ_MEM		1024

struct fobj_mem_s {
    uint32_t	inuse_bitmap[NUM_OBJ_MEM / 32];
    fobj_t		objs[NUM_OBJ_MEM];
};

static int	fobj_obj_mem_index(fenv_t *f, fobj_t *p)
{
    if (p >= &f->obj_memory->objs[0] &&
        p <  &f->obj_memory->objs[NUM_OBJ_MEM]) {
        return p - &f->obj_memory->objs[0];
    }
    fassert(f, 0, 1, "Only 1024 objects currently supported");
    return -1;
}

static int fobj_obj_index_used(fenv_t *f, int idx)
{
    int bit = 1 << (idx & 31);
    int bitmap_i = idx >> 5;

    if (f->obj_memory->inuse_bitmap[bitmap_i] & bit) {
        return 1;
    } else {
        f->obj_memory->inuse_bitmap[bitmap_i] |= bit;
        return 0;
    }
}

static int fobj_obj_mem_used(fenv_t *f, fobj_t *p)
{
    int i = fobj_obj_mem_index(f, p);
    return fobj_obj_index_used(f, i);
}

fenv_t *fenv_new(void)
{
    fenv_t *f = calloc(1, sizeof(*f));

    f->obj_memory = calloc(1, sizeof(*f->obj_memory));
    f->dstack = fstack_new(f);
    f->rstack = fstack_new(f);
    f->words  = ftable_new(f);

    return f;
}

void fenv_free(fenv_t *f)
{
    f->dstack = NULL;
    f->rstack = NULL;
    fobj_garbage_collection(f);
#ifdef DEBUG
    for (int i = 0; i < NUM_OBJ_MEM/32; i++) {
        ASSERT(f->obj_memory->inuse_bitmap[i] == 0);
    }
#endif
}

fobj_t *fobj_new(fenv_t *f, int type)
{
    for (int i = 0; i < NUM_OBJ_MEM; i++) {
        if (!fobj_obj_index_used(f, i)) {
            fobj_t *p = &f->obj_memory->objs[i];
            p->type = type;
            return p;
        }
    }

    fassert(f, 0, 1, "out of memory allocating a new fobj");
    return NULL;
}

void fobj_visit(fenv_t *f, fobj_t *p)
{
    if (!p) return;

    if (fobj_obj_mem_used(f, p)) return;

    if (op_table[p->type].visit) {
        op_table[p->type].visit(f, p);
    }
}

void fobj_garbage_collection(fenv_t *f)
{
    // Copy the in-use bitmap
    // visit f->dstack and f->rstack
    // Determine which objects are no longer used and call their free routine
    uint32_t *copy_inuse_bitmap = calloc(1, NUM_OBJ_MEM / 32 * sizeof(uint32_t));
    for (int i = 0; i < NUM_OBJ_MEM / 32; i++) {
        copy_inuse_bitmap[i] = f->obj_memory->inuse_bitmap[i];
        f->obj_memory->inuse_bitmap[i] = 0;
    }

    fobj_visit(f, f->dstack);
    fobj_visit(f, f->rstack);

    fobj_t *p = &f->obj_memory->objs[0];
    for (int i = 0; i < NUM_OBJ_MEM / 32; i++) {
        for (uint32_t bit = 1; bit; bit <<= 1, p++) {
            if (f->obj_memory->inuse_bitmap[i] & bit) {
                ASSERT(copy_inuse_bitmap[i] & bit);
            }
            if (f->obj_memory->inuse_bitmap[i] & bit) {
                continue;
            }
            if (!(copy_inuse_bitmap[i] & bit)) {
                continue;
            }
            if (op_table[p->type].free) {
                op_table[p->type].free(f, p);
            }
            bzero(p, sizeof(*p));
        }
    }
    ASSERT(p == &f->obj_memory->objs[NUM_OBJ_MEM]);
    free(copy_inuse_bitmap);
}

void fobj_print(fenv_t *f, fobj_t *p)
{
    ASSERT(p);
    ASSERT(p->type > 0);
    ASSERT(op_table[p->type].print);

#ifdef DEBUG
    printf("Object %p: type = %d\n", p, p->type);
#endif
    op_table[p->type].print(f, p);
}

fobj_t *fobj_add(fenv_t *f, fobj_t *op1, fobj_t *op2)
{
    fassert(f, !!op_table[op1->type].add, 1, "%s <> + not supported",
            op_table[op1->type].type_name);

    return op_table[op1->type].add(f, op1, op2);
}    

fobj_t *fobj_sub(fenv_t *f, fobj_t *op1, fobj_t *op2)
{
    fassert(f, !!op_table[op1->type].sub, 1, "%s <> - not supported",
            op_table[op1->type].type_name);

    return op_table[op1->type].sub(f, op1, op2);
}    

fobj_t *fobj_fetch(fenv_t *f, fobj_t *addr, fobj_t *index)
{
    fassert(f, !!op_table[addr->type].fetch, 1, "%s @ not supported",
            op_table[addr->type].type_name);

    return op_table[addr->type].fetch(f, addr, index);
}    

void    fobj_store(fenv_t *f, fobj_t *addr, fobj_t *index, fobj_t *data)
{
    fassert(f, !!op_table[addr->type].store, 1, "%s ! not supported",
            op_table[addr->type].type_name);

    op_table[addr->type].store(f, addr, index, data);
}

fobj_t *findex_new(fenv_t *f, fobj_t *addr, fobj_t *index)
{
    if (!index) return addr;

    fobj_t *p = fobj_new(f, FOBJ_INDEX);
    findex_t *i = &p->u.index;
    i->addr = addr;
    i->index = index;
    return p;
}

void findex_visit(fenv_t *f, fobj_t *p)
{
    findex_t *i = &p->u.index;
    fobj_visit(f, i->addr);
    fobj_visit(f, i->index);
}

int fobj_is_index(fenv_t *f, fobj_t *obj)
{
    if (obj && obj->type == FOBJ_INDEX) {
        return 1;
    } else {
        return 0;
    }
}

fobj_t *fcode_new(fenv_t *f, fcode_t c)
{
    fobj_t *p = fobj_new(f, FOBJ_CODE);
    p->u.code = c;
    return p;
}
