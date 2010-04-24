/*
 * This file is part of arm-sim: http://madscientistroom.org/arm-sim
 *
 * Copyright (c) 2010 Randy Thelen. All rights reserved, and all wrongs
 * reversed. (See the file COPYRIGHT for details.)
 */

#ifndef __FOBJ_H__
#define __FOBJ_H__

typedef struct fnum_s fnum_t;
typedef struct fstr_s fstr_t;
typedef struct findex_s findex_t;
typedef struct farray_s farray_t;
typedef struct fhash_s fhash_t;
typedef struct fstack_s fstack_t;

typedef double fnumber_t;

struct fnum_s {
    fnumber_t		n;
};

struct fstr_s {
    int			len;
    char		*buf;
};

struct farray_s {
    int			 num;
    fobj_t		**elems;
};

struct fstack_s {
    int			 sp;
    int			 max_sp;
    fobj_t		**elems;
};

struct fhash_s {
    int			 num_kv;
    fobj_t		**keys;
    fobj_t		**vals;
};

struct ftable_s {
    fobj_t		*array;
    fobj_t		*hash;
};

struct findex_s {
    fobj_t		*source;
    fobj_t		*index;
};

struct fobj_s {
    int				 type;
    int				 refcount;
    union {
        fnum_t		 num;
        fstr_t		 str;
        ftable_t	 table;
        findex_t	 index;
        farray_t	 array;
        fhash_t		 hash;
        fstack_t	 stack;
    } u;
};

typedef struct foptable_s {
    const char *type_name;
    void (*free)(fenv_t *f, fobj_t *p);
    void (*print)(fenv_t *f, fobj_t *p);

    void    (*store)(fenv_t *f, fobj_t *addr, fobj_t *index, fobj_t *data);
    fobj_t *(*fetch)(fenv_t *f, fobj_t *addr, fobj_t *index);

    fobj_t *(*add)(fenv_t *f, fobj_t *op1, fobj_t *op2);
    fobj_t *(*sub)(fenv_t *f, fobj_t *op1, fobj_t *op2);
} foptable_t;

extern const foptable_t op_table[];


#endif /* __FOBJ_H__ */
