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
    { "number", NULL, NULL, NULL, fnum_print, fnum_cmp, NULL, NULL, fnum_add, fnum_sub },
    { "string", NULL, NULL, fstr_free, fstr_print, fstr_cmp, NULL, fstr_fetch, fstr_add, fstr_sub },
    { "table",  NULL, ftable_visit, NULL, ftable_print, NULL, ftable_store, ftable_fetch },
    { "array",  NULL, farray_visit, farray_free, farray_print, NULL, farray_store, farray_fetch },
    { "hash",   NULL, fhash_visit,  fhash_free, fhash_print, NULL, fhash_store, fhash_fetch },
    { "stack",  NULL, fstack_visit, fstack_free, fstack_print, NULL, fstack_store, fstack_fetch },
    { "index",  NULL, findex_visit, NULL, NULL, NULL, NULL, NULL },
    { "word",   NULL, fword_visit, fword_free, fword_print, NULL, NULL, NULL, NULL, NULL },
};

#define NUM_OBJ_MEM		1024

struct fobj_mem_s {
    uint32_t	inuse_bitmap[NUM_OBJ_MEM / 32];
    int			num_free_objs;
    uint16_t	next_free[NUM_OBJ_MEM];
    fobj_t		objs[NUM_OBJ_MEM];
};

static int	fobj_obj_mem_index(fenv_t *f, fobj_t *p)
{
    fobj_mem_t *m = f->obj_memory;
    if (p >= &m->objs[0] &&
        p <  &m->objs[NUM_OBJ_MEM]) {
        return p - &m->objs[0];
    }
    fassert(f, 0, 1, "Only 1024 objects currently supported");
    return -1;
}

static int fobj_obj_index_used(fenv_t *f, int idx)
{
    uint32_t bit = 1 << (idx & 31);
    int bitmap_i = idx >> 5;
    fobj_mem_t *m = f->obj_memory;

    if (m->inuse_bitmap[bitmap_i] & bit) {
        return 1;
    } else {
        m->inuse_bitmap[bitmap_i] |= bit;
        return 0;
    }
}

static int fobj_obj_mem_used(fenv_t *f, fobj_t *p)
{
    int i = fobj_obj_mem_index(f, p);
    return fobj_obj_index_used(f, i);
}

static void fobj_obj_mem_init(fenv_t *f)
{
    f->obj_memory = calloc(1, sizeof(*f->obj_memory));
    fobj_mem_t *m = f->obj_memory;
    m->num_free_objs = NUM_OBJ_MEM;
    for (int i = 0; i < NUM_OBJ_MEM; i++) {
        m->next_free[i] = NUM_OBJ_MEM - 1 - i;
    }
}

fenv_t *fenv_new(void)
{
    fenv_t *f = calloc(1, sizeof(*f));

    fobj_obj_mem_init(f);

    f->hold_stack = fstack_new(f);
    f->dstack = fstack_new(f);
    f->rstack = fstack_new(f);
    f->words  = ftable_new(f);

    fobj_hold_clear(f);

    return f;
}

void fenv_free(fenv_t *f)
{
    f->dstack = NULL;
    f->rstack = NULL;
    f->hold_stack = NULL;
    f->running = NULL;
    f->words = NULL;
    f->new_words = NULL;
    f->imm_words = NULL;
    f->input_str = NULL;
    f->current_compiling = NULL;

    fobj_garbage_collection(f);
#ifdef DEBUG
    for (int i = 0; i < NUM_OBJ_MEM/32; i++) {
        ASSERT(f->obj_memory->inuse_bitmap[i] == 0);
    }
#endif
}

#if DEBUG_MISSING_OBJECTS
fobj_t *fobj_findp = NULL;
fobj_t *fobj_foundp = NULL;

void fobj_find(fenv_t *f, fobj_t *p)
{
    fobj_findp = p;
    fobj_foundp = NULL;
    fobj_mem_t *m = f->obj_memory;

    bzero(m->inuse_bitmap, NUM_OBJ_MEM/8);

    fobj_visit(f, f->dstack);
    fobj_visit(f, f->rstack);
    fobj_visit(f, f->current_compiling); // during colon definitions
    fobj_visit(f, f->new_words);
    fobj_visit(f, f->words);
    fobj_visit(f, f->input_str);
    fobj_visit(f, f->running);
    fobj_visit(f, f->hold_stack);

    ASSERT(!!fobj_foundp);
}
#endif /* DEBUG_MISSING_OBJECTS */

fobj_t *fobj_new(fenv_t *f, int type)
{
    if (f->obj_memory->num_free_objs == 0) {
        fobj_garbage_collection(f);
    }

#ifdef DEBUG
    /*
     * DEBUG: Always garbage collect!
     */

    if (f->hold_stack) {
        fobj_garbage_collection(f);
    }
#endif /* DEBUG */

    fobj_mem_t *m = f->obj_memory;

#if DEBUG_MISSING_OBJECTS
    if (m->num_free_objs == 0) {
        for (int idx = 0; idx < NUM_OBJ_MEM; idx ++) {
            fobj_find(f, &m->objs[idx]);
        }
    }
#endif /* DEBUG_MISSING_OBJECTS */

    fassert(f, m->num_free_objs > 0, 1, "out of memory allocating a new fobj");

    int i = --(m->num_free_objs);
    int pi = m->next_free[i];
    m->next_free[i] = 0;
    fobj_t *p = &m->objs[pi];
    
    p->type = type;

    int r = fobj_obj_mem_used(f, p);
    FASSERT(!r, "Just allocated an already in use block", __FILE__, __LINE__);

    if (f->hold_stack) {
        HOLD(p);  // Hold newly allocated object
    }

    return p;
}

void fobj_visit(fenv_t *f, fobj_t *p)
{
    if (!p) return;

#if DEBUG_MISSING_OBJECTS
    if (fobj_findp == p) fobj_foundp = p;
    if (fobj_foundp)     return;
#endif /* DEBUG_MISSING_OBJECTS */

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
    uint32_t copy_inuse_bitmap[NUM_OBJ_MEM / 32];
    int n = NUM_OBJ_MEM / 32;
    fobj_mem_t *m = f->obj_memory;
    int nbytes = sizeof(m->inuse_bitmap);

    bcopy(m->inuse_bitmap, copy_inuse_bitmap, nbytes);
    bzero(m->inuse_bitmap, nbytes);

    fobj_visit(f, f->dstack);
    fobj_visit(f, f->rstack);
    fobj_visit(f, f->current_compiling); // during colon definitions
    fobj_visit(f, f->new_words);
    fobj_visit(f, f->words);
    fobj_visit(f, f->input_str);
    fobj_visit(f, f->running);
    fobj_visit(f, f->hold_stack);

    for (int i = 0; i < n; i++) {
        /*
         * free will have bits set for items which the allocator had
         * -thought- were in use (i.e., they had been allocated at some
         * point in the past) but are no longer in use (i.e., they're inuse
         * bit in m->inuse_bitmap[] is now zero).
         *
         * The algorithm works by noting that if a bit had been set
         * previously (i.e., a bit is set in copy_inuse_bitmap[]), but is
         * now clear (i.e., the bit is clear in m->inuse_bitmap[]), then we
         * need to add that object to the free list.
         */
        uint32_t free = copy_inuse_bitmap[i] & ~m->inuse_bitmap[i];
        if (!free) {
            continue;
        }

        fobj_t *p = &m->objs[i * 32];
        for (uint32_t bit = 1; free && bit; bit <<= 1, p++) {
            if (!(free & bit)) {
                continue;
            }

            free ^= bit;
            if (op_table[p->type].free) {
                op_table[p->type].free(f, p);
            }
            bzero(p, sizeof(*p));
            
            int j = m->num_free_objs++;
            int pj = p - &m->objs[0];
            m->next_free[j] = pj;
        }
    }
}

fobj_t *fobj_hold(fenv_t *f, fobj_t *p)
{
    if (!p) return NULL;
    fstack_store(f, f->hold_stack, NULL, p);
    return p;
}

void fobj_hold_n(fenv_t *f, int n, ...)
{
    va_list ap;

    va_start(ap, n);
    for (int i = 0; i < n; i++) {
        fobj_hold(f, va_arg(ap, fobj_t *));
    }
    va_end(ap);
}

void fobj_hold_clear(fenv_t *f)
{
    f->hold_stack->u.stack.sp = 0;
}

void fobj_print(fenv_t *f, fobj_t *p)
{
    if (!p) {
        printf("(null)");
    } else {
        ASSERT(p->type > 0);
        ASSERT(op_table[p->type].print);

#ifdef DEBUG
        printf("Object %p: type = %d\n", p, p->type);
#endif
        op_table[p->type].print(f, p);
    }
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

int fobj_cmp(fenv_t *f, fobj_t *a, fobj_t *b)
{
    if (a->type == b->type && op_table[a->type].cmp) {
        return op_table[a->type].cmp(f, a, b);
    } else {
        if (a < b)  return -1;
        if (a == b) return  0;
        else        return  1;
    }
}

int fobj_hash(fenv_t *f, fobj_t *a)
{
    int hash = 0;
    int entropy = a->type | a->type << 4;
    unsigned char *p = (unsigned char *) a;

    for (int i = 0; i < sizeof(fobj_t *); i++) {
        hash += (hash << 3) + (p[i] ^ entropy);
    }

    return hash;
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

fobj_t *fstate_new(fenv_t *f, int state, int offset)
{
    fobj_t *p = fobj_new(f, FOBJ_STATE);
    fstate_t *s = &p->u.state;
    s->state = state;
    s->offset = offset;
    return p;
}
