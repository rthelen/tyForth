/*
 * This file is part of arm-sim: http://madscientistroom.org/arm-sim
 *
 * Copyright (c) 2010 Randy Thelen. All rights reserved, and all wrongs
 * reversed. (See the file COPYRIGHT for details.)
 */

#ifndef __FORTH_H__
#define __FORTH_H__

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <assert.h>
#include <stdarg.h>
#include <ctype.h>

#ifndef FALSE
#define FALSE		0
#define TRUE		(!FALSE)
#endif

void FASSERT(int x, const char *err, const char *file, int line);
#define ASSERT(x)	FASSERT(!!(x), #x, __FILE__, __LINE__)

#define FOBJ_NUM		1
#define FOBJ_STR		2
#define FOBJ_TABLE		3
#define FOBJ_ARRAY		4
#define FOBJ_HASH		5
#define FOBJ_STACK		6
#define FOBJ_INDEX		7
#define FOBJ_WORD		8
#define FOBJ_NUM_TYPES	13

typedef long double fnumber_t;
typedef int32_t fint_t;
typedef uint32_t fuint_t;

typedef struct fobj_s fobj_t;
typedef struct ftable_s ftable_t;
typedef struct fobj_mem_s fobj_mem_t;
typedef struct fheader_s fheader_t;
typedef struct fenv_s fenv_t;
typedef struct fword_s fword_t;
typedef struct fdocolon_s fdocolon_t;

typedef void (*fcode_t)(fenv_t *f, fobj_t *w);
typedef union fbody_u fbody_t;

struct fenv_s {
    fobj_mem_t		*obj_memory;
    fobj_t			*dstack;
    fobj_t			*rstack;
    fobj_t			*words;
    fobj_t			*imm_words;

    fobj_t			*input_str;
    int				 input_offset;

    fbody_t			*ip;
};

void fassert(fenv_t *f, int condition, int error, const char *fmt, ...);

void fobj_garbage_collection(fenv_t *f);

fenv_t *fenv_new(void);
void    fenv_free(fenv_t *f);

fobj_t *findex_new(fenv_t *f, fobj_t *addr, fobj_t *index);
void    findex_visit(fenv_t *f, fobj_t *p);

fobj_t *fobj_new(fenv_t *f, int type);
void    fobj_visit(fenv_t *f, fobj_t *p);
void    fobj_print(fenv_t *f, fobj_t *p);
fobj_t *fobj_add(fenv_t *f, fobj_t *op1, fobj_t *op2);
fobj_t *fobj_sub(fenv_t *f, fobj_t *op1, fobj_t *op2);
fobj_t *fobj_fetch(fenv_t *f, fobj_t *addr, fobj_t *index);
void    fobj_store(fenv_t *f, fobj_t *addr, fobj_t *index, fobj_t *data);
int     fobj_cmp(fenv_t *f, fobj_t *a, fobj_t *b);
int     fobj_is_index(fenv_t *f, fobj_t *obj);

fobj_t *fnum_new(fenv_t *f, fnumber_t n);
void    fnum_print(fenv_t *f, fobj_t *p);
int     fnum_cmp(fenv_t *f, fobj_t *a, fobj_t *b);
fobj_t *fnum_add(fenv_t *f, fobj_t *op1, fobj_t *op2);
fobj_t *fnum_sub(fenv_t *f, fobj_t *op1, fobj_t *op2);

fobj_t *fstr_new(fenv_t *f, const char *str);
fobj_t *fstr_new_buf(fenv_t *f, const char *buf, int len);
void    fstr_free(fenv_t *f, fobj_t *p);
void    fstr_print(fenv_t *f, fobj_t *p);
int     fstr_cmp(fenv_t *f, fobj_t *a, fobj_t *b);
fobj_t *fstr_add(fenv_t *f, fobj_t *op1, fobj_t *op2);
fobj_t *fstr_sub(fenv_t *f, fobj_t *op1, fobj_t *op2);
fobj_t *fstr_fetch(fenv_t *f, fobj_t *addr, fobj_t *index);
int     fstr_len(fenv_t *f, fobj_t *str);
int     fstr_getchar(fenv_t *f, fobj_t *str, int offset);
fnumber_t fstr_to_number(fenv_t *f, fobj_t *str);

fobj_t *ftable_new(fenv_t *f);
void    ftable_visit(fenv_t *f, fobj_t *p);
void    ftable_free(fenv_t *f, fobj_t *p);
void    ftable_print(fenv_t *f, fobj_t *p);
fobj_t *ftable_add(fenv_t *f, fobj_t *op1, fobj_t *op2);
fobj_t *ftable_sub(fenv_t *f, fobj_t *op1, fobj_t *op2);
fobj_t *ftable_fetch(fenv_t *f, fobj_t *addr, fobj_t *index);
void    ftable_store(fenv_t *f, fobj_t *addr, fobj_t *index, fobj_t *data);
void    ftable_push(fenv_t *f, fobj_t *stack, fobj_t *data);
fobj_t *ftable_pop(fenv_t *f, fobj_t *stack);

fobj_t *farray_new(fenv_t *f);
void    farray_visit(fenv_t *f, fobj_t *a);
void    farray_free(fenv_t *f, fobj_t *a);
void    farray_print(fenv_t *f, fobj_t *a);
void    farray_store(fenv_t *f, fobj_t *addr, fobj_t *index, fobj_t *data);
fobj_t *farray_fetch(fenv_t *f, fobj_t *addr, fobj_t *index);

fobj_t *fstack_new(fenv_t *f);
void    fstack_visit(fenv_t *f, fobj_t *a);
void    fstack_free(fenv_t *f, fobj_t *a);
void    fstack_print(fenv_t *f, fobj_t *a);
void    fstack_store(fenv_t *f, fobj_t *addr, fobj_t *index, fobj_t *data);
fobj_t *fstack_fetch(fenv_t *f, fobj_t *addr, fobj_t *index);

fobj_t *fhash_new(fenv_t *f);
void    fhash_visit(fenv_t *f, fobj_t *a);
void    fhash_free(fenv_t *f, fobj_t *a);
void    fhash_print(fenv_t *f, fobj_t *a);
void    fhash_visit(fenv_t *f, fobj_t *a);
void    fhash_store(fenv_t *f, fobj_t *addr, fobj_t *index, fobj_t *data);
fobj_t *fhash_fetch(fenv_t *f, fobj_t *addr, fobj_t *index);

fobj_t *fvar_new(fenv_t *f, fobj_t *name, fobj_t *value);
fobj_t *fcons_new(fenv_t *f, fobj_t *name, fobj_t *value);

void    fcode_new_primitive(fenv_t *f, fobj_t *name, fcode_t code);

#define MKFNAME(x)			fcode_ ## x
#define FCODE(x)			void MKFNAME(x)(fenv_t *f, fobj_t *w)

#define PUSH(x)				MKFNAME(push)(f, w, x)
#define PUSHN(n)			MKFNAME(push)(f, w, fnum_new(f, n))
#define PUSHS(s)			MKFNAME(push)(f, w, fstr_new(f, s))
#define POP					MKFNAME(pop)(f, w)
#define POPN				MKFNAME(pop_num)(f, w)
#define POPI				MKFNAME(pop_int)(f, w)

#define RPOP				MKFNAME(rpop)(f, w)
#define RPUSH(x)			MKFNAME(rpush)(f, w, x)

#define OVER				MKFNAME(over)(f, w)
#define SWAP				MKFNAME(swap)(f, w)
#define DUP					MKFNAME(dup)(f, w)
#define DROP				MKFNAME(drop)(f, w)
#define ADD					MKFNAME(add)(f, w)
#define SUB					MKFNAME(sub)(f, w)
#define PRINT				MKFNAME(print)(f, w)

#define STORE				MKFNAME(store)(f, w)
#define FETCH				MKFNAME(fetch)(f, w)
#define INDEX				MKFNAME(index)(f, w)

void    MKFNAME(push)(fenv_t *f, fobj_t *w, fobj_t *p);
fobj_t *MKFNAME(pop)(fenv_t *f, fobj_t *w);
fnumber_t MKFNAME(pop_num)(fenv_t *f, fobj_t *w);
fint_t  MKFNAME(pop_int)(fenv_t *f, fobj_t *w);
union fbody_u {
    fobj_t			*word;
    fnumber_t		 n;
};

struct fheader_s {
    char				*name;
    fcode_t				 code;
};

#define MKHDRNAME(_name)	_name ## _header
#define FWORD_HEADER(_name, _str)                       \
    void _name(fenv_t *f, fobj_t *w);           \
    fheader_t MKHDRNAME(_name) = { _str, _name };   \
    void _name(fenv_t *f, fobj_t *w)

#define FWORD2(_name, _str)                    \
    FWORD_HEADER(MKFNAME(_name), _str)

#define FWORD(_name)                           \
    FWORD2(_name, # _name)

#define FWORD_IMM2(_name, _str)                \
    FWORD_HEADER(MKFNAME(_name), _str)

#define FWORD_IMM(_name)                       \
    FWORD_IMM2(_name, # _name)

#define FWORD_DO2(_name, _str)                 \
    FWORD_HEADER(fcode_do_ ## _name, _str)

#define FWORD_DO(_name)                        \
    FWORD_DO2(_name, "(" # _name ")")

/**********************************************************
 *
 * Input Processing Routines
 *
 **********************************************************/

int  fparse_token(fenv_t *f, fobj_t **token_str);
void fparse_do_token(fenv_t *f, fobj_t *token);


#endif /* __FORTH_H__ */
