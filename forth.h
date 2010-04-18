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
#define FOBJ_FUNC		3
#define FOBJ_TABLE		4
#define FOBJ_WORD		5
#define FOBJ_NUM_TYPES	6

typedef struct fobj_s fobj_t;
typedef struct felem_s felem_t;
typedef struct fstack_s fstack_t;

typedef struct fenv_s {
    int x;
} fenv_t;

void fassert(fenv_t *f, int condition, int error, const char *fmt, ...);

void felem_init(fenv_t *f, felem_t *p, fobj_t *obj);
void felem_free(fenv_t *f, felem_t *p);

fstack_t *fstack_new(fenv_t *f, const char *name);
void      fstack_free(fenv_t *f, fstack_t *s);
felem_t  *fstack_pop(fenv_t *f, fstack_t *s, felem_t *p);
void      fstack_push(fenv_t *f, fstack_t *s, felem_t *p);
void      fstack_push_obj(fenv_t *f, fstack_t *s, fobj_t *p);

fobj_t *fobj_new(fenv_t *f, int type);
void    fobj_retain(fenv_t *f, fobj_t *p);
void    fobj_release(fenv_t *f, fobj_t *p);
void    fobj_print(fenv_t *f, fobj_t *p);
void    fobj_add(fenv_t *f, felem_t *dest, felem_t *op1, felem_t *op2);
void    fobj_sub(fenv_t *f, felem_t *dest, felem_t *op1, felem_t *op2);

fobj_t *fnum_new(fenv_t *f, double n);
void    fnum_print(fenv_t *f, fobj_t *p);
void    fnum_free(fenv_t *f, fobj_t *p);
void    fnum_add(fenv_t *f, felem_t *dest, felem_t *op1, felem_t *op2);
void    fnum_sub(fenv_t *f, felem_t *dest, felem_t *op1, felem_t *op2);

fobj_t *fstr_new(fenv_t *f, const char *str);
void    fstr_print(fenv_t *f, fobj_t *p);
void    fstr_free(fenv_t *f, fobj_t *p);
void    fstr_add(fenv_t *f, felem_t *dest, felem_t *op1, felem_t *op2);
void    fstr_sub(fenv_t *f, felem_t *dest, felem_t *op1, felem_t *op2);


#endif /* __FORTH_H__ */
