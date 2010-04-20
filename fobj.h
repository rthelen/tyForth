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
typedef struct ffunc_s ffunc_t;
//typedef struct ftable_s ftable_t;
typedef struct fword_s fword_t;
typedef struct fcell_s fcell_t;
typedef struct fbounds_s fbounds_t;
typedef struct findex_s findex_t;
typedef struct foptable_s foptable_t;
//typedef struct felem_s felem_t;
//typedef struct fstack_s fstack_t;

typedef void (*fcode_t)(fword_t *w);

typedef void (*fop_t)(fenv_t *f, fobj_t *d_op, fobj_t *l_op, fobj_t *r_op);

struct fnum_s {
    double		n;
};

struct fstr_s {
    int			len;
    char		*buf;
};

struct ffunc_s {
    fcode_t		 code;
    int			 num_cells;
    fcell_t		*cells;
};

struct ftable_s {
    int			 i_start, i_limit;
    fobj_t		**array;

    int			 num_kv_pairs;
    fobj_t		**keys;
    fobj_t		**vals;
};

struct fword_s {
    fobj_t		*name; // Must be String
    fobj_t		*value;  // Can be anything
    fobj_t		*next; // Must be a Word
};

#define FCELL_FUNC		1
#define FCELL_NUM		2
#define FCELL_OBJ		3

struct fcell_s {
    int			 type;
    union {
        fobj_t	*func;  // points to a word, but we execute it's value, which must be a func.  The Word pointer is used so we can know what the word name is (!).
        fnum_t	 n;
        fobj_t	*obj;	// points to an arbitrary object for processing by the previous func.
    } cell;
};

struct fbounds_s {
    double		 index, limit;
};

struct fobj_s {
    int				 type;
    int				 refcount;
    union {
        fnum_t		 num;
        fstr_t		 str;
        ffunc_t		 func;
        ftable_t	 table;
        fword_t		 word;
    } u;
};

#define FINDEX_NONE		0
#define FINDEX_NUM		1
#define FINDEX_STR		2

struct findex_s {
    int				 	 type;
    union {
        fnum_t			 num;
        fobj_t			*str;  // Must be a String obj
    } i;
};

struct felem_s {
    fobj_t			*obj;
    findex_t		 index;
};

struct fstack_s {
    const char		*name;
    int				 num_elems;
    int				 max_elems;
    felem_t			*elems;
};

struct foptable_s {
    const char *type_name;
    void (*free)(fenv_t *f, fobj_t *p);
    void (*print)(fenv_t *f, fobj_t *p);

    int  (*add)(fenv_t *f, felem_t *dest, felem_t *op1, felem_t *op2);
    int  (*sub)(fenv_t *f, felem_t *dest, felem_t *op1, felem_t *op2);
};

extern const foptable_t op_table[];


#endif /* __FOBJ_H__ */
