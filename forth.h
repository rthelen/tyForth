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
#define FOBJ_FUNC		7
#define FOBJ_WORD		8
#define FOBJ_INDEX		9
#define FOBJ_NUM_TYPES	10

typedef struct fobj_s fobj_t;
typedef struct ftable_s ftable_t;

typedef struct fenv_s {
    int x;
} fenv_t;

void fassert(fenv_t *f, int condition, int error, const char *fmt, ...);

fobj_t *fobj_new(fenv_t *f, int type);
fobj_t *fobj_retain(fenv_t *f, fobj_t *p);
void    fobj_release(fenv_t *f, fobj_t *p);
void    fobj_print(fenv_t *f, fobj_t *p);
fobj_t *fobj_add(fenv_t *f, fobj_t *op1, fobj_t *op2);
fobj_t *fobj_sub(fenv_t *f, fobj_t *op1, fobj_t *op2);
fobj_t *fobj_fetch(fenv_t *f, fobj_t *addr, fobj_t *index);
void    fobj_store(fenv_t *f, fobj_t *addr, fobj_t *index, fobj_t *data);
int     fobj_is_index(fenv_t *f, fobj_t *obj);

fobj_t *fnum_new(fenv_t *f, double n);
void    fnum_print(fenv_t *f, fobj_t *p);
void    fnum_free(fenv_t *f, fobj_t *p);
fobj_t *fnum_add(fenv_t *f, fobj_t *op1, fobj_t *op2);
fobj_t *fnum_sub(fenv_t *f, fobj_t *op1, fobj_t *op2);

fobj_t *fstr_new(fenv_t *f, const char *str);
void    fstr_print(fenv_t *f, fobj_t *p);
void    fstr_free(fenv_t *f, fobj_t *p);
fobj_t *fstr_add(fenv_t *f, fobj_t *op1, fobj_t *op2);
fobj_t *fstr_sub(fenv_t *f, fobj_t *op1, fobj_t *op2);
fobj_t *fstr_fetch(fenv_t *f, fobj_t *addr, fobj_t *index);

fobj_t *ftable_new(fenv_t *f);
void    ftable_print(fenv_t *f, fobj_t *p);
void    ftable_free(fenv_t *f, fobj_t *p);
fobj_t *ftable_add(fenv_t *f, fobj_t *op1, fobj_t *op2);
fobj_t *ftable_sub(fenv_t *f, fobj_t *op1, fobj_t *op2);
fobj_t *ftable_fetch(fenv_t *f, fobj_t *addr, fobj_t *index);
void    ftable_store(fenv_t *f, fobj_t *addr, fobj_t *index, fobj_t *data);
void    ftable_push(fenv_t *f, fobj_t *stack, fobj_t *data);
fobj_t *ftable_pop(fenv_t *f, fobj_t *stack);

fobj_t *farray_new(fenv_t *f);
void    farray_print(fenv_t *f, fobj_t *a);
void    farray_free(fenv_t *f, fobj_t *a);
void    farray_store(fenv_t *f, fobj_t *addr, fobj_t *index, fobj_t *data);
fobj_t *farray_fetch(fenv_t *f, fobj_t *addr, fobj_t *index);

fobj_t *fstack_new(fenv_t *f);
void    fstack_print(fenv_t *f, fobj_t *a);
void    fstack_free(fenv_t *f, fobj_t *a);
void    fstack_store(fenv_t *f, fobj_t *addr, fobj_t *index, fobj_t *data);
fobj_t *fstack_fetch(fenv_t *f, fobj_t *addr, fobj_t *index);

fobj_t *fhash_new(fenv_t *f);
void    fhash_print(fenv_t *f, fobj_t *a);
void    fhash_free(fenv_t *f, fobj_t *a);
void    fhash_store(fenv_t *f, fobj_t *addr, fobj_t *index, fobj_t *data);
fobj_t *fhash_fetch(fenv_t *f, fobj_t *addr, fobj_t *index);

#endif /* __FORTH_H__ */
