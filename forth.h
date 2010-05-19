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
#define FOBJ_CALL		9
#define FOBJ_STATE		10
#define FOBJ_LOOP		11
#define FOBJ_NUM_TYPES	12

typedef long double fnumber_t;
typedef int32_t fint_t;
typedef uint32_t fuint_t;

typedef struct fobj_s fobj_t;
typedef struct ftable_s ftable_t;
typedef struct fobj_mem_s fobj_mem_t;
typedef struct fenv_s fenv_t;
typedef struct fword_s fword_t;
typedef struct floop_s floop_t;

typedef void (*fcode_t)(fenv_t *f, fobj_t *w);
typedef struct fbody_s fbody_t;

struct fenv_s {
    fobj_mem_t		*obj_memory;
    fobj_t			*dstack;
    fobj_t			*rstack;
    fobj_t			*words;
    fobj_t			*imm_words;

    fobj_t			*new_words;

    fobj_t			*input_str;
    int				 input_offset;

    fobj_t			*hold_stack;

    fobj_t			*running;
    fbody_t			*ip;

    /*
     * Compiling code.
     */
    int				 in_colon;
    fobj_t			*current_compiling;
};

void fassert(fenv_t *f, int condition, int error, const char *fmt, ...);

void fobj_garbage_collection(fenv_t *f);

#define HOLD(p)				fobj_hold(f, p)
#define HOLD1(p)			fobj_hold(f, p)
#define HOLD2(a,b)			fobj_hold_n(f, 2, a, b)
#define HOLD3(a,b,c)		fobj_hold_n(f, 3, a, b, c)
#define HOLD4(a,b,c,d)		fobj_hold_n(f, 4, a, b, c, d)


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

void    fobj_hold(fenv_t *f, fobj_t *p);
void    fobj_hold_n(fenv_t *f, int n, ...);
void    fobj_hold_clear(fenv_t *f);

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

void    fcode_init(fenv_t *f);
void    fcode_new_word(fenv_t *f, fobj_t *name, fbody_t *body);
void fcode_handle_token(fenv_t *f, fobj_t *token);
void fcode_compile_string(fenv_t *f, const char *string);

void fword_visit(fenv_t *f, fobj_t *w);
void fword_free(fenv_t *f, fobj_t *w);
void fword_print(fenv_t *f, fobj_t *w);

fobj_t *fstate_new(fenv_t *f, int state, int offset);

/**********************************************************
 *
 * Input Processing Routines
 *
 **********************************************************/

int fparse_token_to_number(fenv_t *f, fobj_t *token, fnumber_t *n);
int  fparse_token(fenv_t *f, fobj_t **token_str);
void fparse_do_token(fenv_t *f, fobj_t *token);


#endif /* __FORTH_H__ */
