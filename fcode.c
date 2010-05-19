/*
 * This file is part of arm-sim: http://madscientistroom.org/arm-sim
 *
 * Copyright (c) 2010 Randy Thelen. All rights reserved, and all wrongs
 * reversed. (See the file COPYRIGHT for details.)
 */

#include "forth.h"
#include "fobj.h"

typedef struct fheader_s fheader_t;

#define MKFNAME(x)			fcode_ ## x
#define MKHDRNAME(_name)	_name ## _header

#define FWORD_HEADER(_name, _str, _imm)              \
    void _name(fenv_t *f, fobj_t *w);           \
    fheader_t MKHDRNAME(_name) = { _str, _name, _imm };  \
    void _name(fenv_t *f, fobj_t *w)

#define FWORD2(_name, _str)                    \
    FWORD_HEADER(MKFNAME(_name), _str, 0)

#define FWORD(_name)                           \
    FWORD2(_name, # _name)

#define FWORD_IMM2(_name, _str)                \
    FWORD_HEADER(MKFNAME(_name), _str, 1)

#define FWORD_IMM(_name)                       \
    FWORD_IMM2(_name, # _name)

#define FWORD_DO2(_name, _str)                 \
    FWORD_HEADER(MKFNAME(do_ ## _name), _str, 0)

#define FWORD_DO(_name)                        \
    FWORD_DO2(_name, "(" # _name ")")

#define FCODE(x)			void MKFNAME(x)(fenv_t *f, fobj_t *w)

#define DEPTH				(f->dstack->u.stack.sp)
#define RDEPTH				(f->rstack->u.stack.sp)

#define PUSH(x)				MKFNAME(push)(f, x)
#define PUSHN(n)			MKFNAME(push)(f, fnum_new(f, n))
#define PUSHS(s)			MKFNAME(push)(f, fstr_new(f, s))
#define POP					MKFNAME(pop)(f)
#define POPN				MKFNAME(pop_num)(f)
#define POPI				MKFNAME(pop_int)(f)

#define RPOP				MKFNAME(rpop)(f)
#define RPUSH(x)			MKFNAME(rpush)(f, x)

#define OVER				MKFNAME(over)(f, NULL)
#define SWAP				MKFNAME(swap)(f, NULL)
#define DUP					MKFNAME(dup)(f, NULL)
#define DROP				MKFNAME(drop)(f, NULL)
#define ADD					MKFNAME(add)(f, NULL)
#define SUB					MKFNAME(sub)(f, NULL)

#define PRINT				MKFNAME(print)(f, w)
#define STORE				MKFNAME(store)(f, w)
#define FETCH				MKFNAME(fetch)(f, w)
#define INDEX				MKFNAME(index)(f, w)

void    MKFNAME(push)(fenv_t *f, fobj_t *p);
fobj_t *MKFNAME(pop)(fenv_t *f);
fnumber_t MKFNAME(pop_num)(fenv_t *f);
fint_t  MKFNAME(pop_int)(fenv_t *f);
struct fbody_s {
    fobj_t			*word;
    fnumber_t		 n;
};

struct fheader_s {
    char				*name;
    fcode_t				 code;
    int					 immediate;
};

/*
 * Include an auto-generated file with the prototypes for all of the
 * functions in this file.
 */

#include "fwords.h"


/*
 * This file contains all the primitives of the system.
 */

fheader_t *fcode_primitives_ptrs[] = {
	/*
     * Bring in an auto-generated list of the FWORDs defined in
     * fprimitives.c and possibly elsewhere.
     */

	#include "fwords.c"

    NULL
};

/*
 * Standard fobj routines for class "word"
 */

void fword_visit(fenv_t *f, fobj_t *p)
{
    fword_t *w = &p->u.word;
    fobj_visit(f, w->name);
    if (w->body_offset > 0) {
        for (int i = 0; i < w->body_offset; i++) {
            fobj_visit(f, w->u.body[i].word);
        }
    } else if (w->u.value) {
        fobj_visit(f, w->u.value);
    }
}

void fword_free(fenv_t *f, fobj_t *p)
{
    fword_t *w = &p->u.word;
    if (w->body_offset > 0) {
        free(w->u.body);
    }
}

void fword_print(fenv_t *f, fobj_t *p)
{
    // Another time.
}

static fobj_t *fcode_new(fenv_t *f,
                         fobj_t*name,
                         fcode_t code,
                         int immediate,
                         fbody_t *body,
                         fobj_t *value)
{
    fobj_t *word = fobj_new(f, FOBJ_WORD);
    fword_t *w = &word->u.word;
    w->name = name;
    w->code = code;
    w->immediate = immediate;
    w->body_len = 0;
    w->body_allocated = 0;
    w->body_offset = 0;

    if (body) {
        w->u.body = body;
    } else if (value) {
        w->u.value = value;
    } else {
        w->u.body = NULL;  // Default
    }

    return word;
}

static void fcode_install(fenv_t *f, fobj_t *word)
{
    ftable_store(f, f->words, word->u.word.name, word);
}
    

fobj_t *do_zbranch;
fobj_t *do_branch;
fobj_t *do_do;
fobj_t *do_loop;
fobj_t *do_exit;

static fobj_t *fcode_lookup_word(fenv_t *f, char *name)
{
    return ftable_fetch(f, f->words, fstr_new(f, name));
}

void fcode_init(fenv_t *f)
{
    /*
     * Add the words to the dictionary
     */
    fheader_t *p;

    for (int i = 0; (p = fcode_primitives_ptrs[i]); i++) {
        fobj_t *name = fstr_new(f, p->name);
        fcode_install(f, fcode_new(f, name, p->code, p->immediate, NULL, NULL));
    }

    f->current_compiling = NULL;

    do_zbranch = fcode_lookup_word(f, "(zbranch)");
    do_branch  = fcode_lookup_word(f, "(branch)");
    do_do  = fcode_lookup_word(f, "(do)");
    do_loop  = fcode_lookup_word(f, "(loop)");
    do_exit = fcode_lookup_word(f, "(exit)");
}

void fcode_new_var(fenv_t *f, fobj_t *name, fobj_t *value)
{
    fcode_install(f, fcode_new(f, name, fcode_do_var_header.code, 0, NULL, value));
}

void fcode_new_constant(fenv_t *f, fobj_t *name, fobj_t *value)
{
    fcode_install(f, fcode_new(f, name, fcode_do_constant_header.code, 0, NULL, value));
}

void fcode_new_word(fenv_t *f, fobj_t *name, fbody_t *body)
{
    fcode_install(f, fcode_new(f, name, fcode_do_colon_header.code, 0, body, NULL));
}

fobj_t *MKFNAME(pop)(fenv_t *f)
{
    return fstack_fetch(f, f->dstack, NULL);
}

void MKFNAME(push)(fenv_t *f, fobj_t *p)
{
    fstack_store(f, f->dstack, NULL, p);
}

fnumber_t MKFNAME(pop_num)(fenv_t *f)
{
    fobj_t *num_obj = POP;
    fassert(f, num_obj->type == FOBJ_NUM, 1, "A number was expected here");
    return num_obj->u.num.n;
}

fint_t MKFNAME(pop_int)(fenv_t *f)
{
    return (fint_t) MKFNAME(pop_num)(f);
}

fobj_t *MKFNAME(rpop)(fenv_t *f)
{
    return fstack_fetch(f, f->rstack, NULL);
}

void MKFNAME(rpush)(fenv_t *f, fobj_t *p)
{
    fstack_store(f, f->rstack, NULL, p);
}

FCODE(print)
{
    fobj_print(f, POP);
}

/**********************************************************
 *
 * The Most Rudimentary Stack Operators
 *
 **********************************************************/
#define A		fobj_t *a
#define B		fobj_t *b
#define C		fobj_t *c

FWORD2(dot, ".") { PRINT; }
FWORD(dup)      { A = POP; PUSH(a); PUSH(a); }
FWORD(drop)     { POP; }
FWORD(swap)     { A = POP; B = POP; PUSH(a); PUSH(b); }
FWORD(nip)      { SWAP; DROP; }
FWORD(2drop)    { POP; POP; }

            /* over:  a b -> a b a */
FWORD(over)         { B = POP; A = POP; PUSH(a); PUSH(b); PUSH(a); }

/**********************************************************
 *
 * Arithmetic operators
 *
 * Note that plus and minus are overloaded operators and that's why
 * fobj_{add,sub}() is called instead of just allowing the C compiler
 * to generate add/sub mnemonics.
 *
 **********************************************************/

FWORD2(plus, "+")
{
    B = POP;
    A = POP;

    PUSH(fobj_add(f, a, b));
}

FWORD2(minus, "-")
{
    B = POP;
    A = POP;

    PUSH(fobj_sub(f, a, b));
}

FWORD2(star, "*")    { PUSHN(POPN * POPN); }
FWORD2(slash, "/")   { fnumber_t b = POPN; fnumber_t a = POPN; PUSHN(a / b); }
FWORD(and)           { PUSHN(POPI & POPI); }
FWORD(or)            { PUSHN(POPI | POPI); }
FWORD(xor)           { PUSHN(POPI ^ POPI); }

FWORD(negate)        { PUSHN(-POPN); }
FWORD(invert)        { PUSHN(~POPI); }

FWORD2(2star, "2*")        { PUSHN(POPI * 2); }
FWORD2(2slash, "2/")       { PUSHN(POPI / 2); }
FWORD2(u2slash, "u2/")     { PUSHN(POPI >> 1); }

FWORD2(shift_left, "<<")
{ fint_t cnt = POPI, n =  POPI; PUSHN(n << cnt); }

FWORD2(shift_right, ">>")
{ fint_t cnt = POPI; fint_t n = POPI; PUSHN(n >> cnt); }

FWORD2(ushift_right, "u>>")
{ fint_t cnt = POPI; fuint_t n = POPI; PUSHN(n >> cnt); }

FWORD2(uless, "u<")
{  fuint_t b =  POPI;  fuint_t a = POPI; PUSHN(a < b ? -1 : 0); }

FWORD2(fetch, "@")
{
    A = POP;
    B = NULL;
    if (fobj_is_index(f, a)) {
        b = a->u.index.index;
        a =  a->u.index.addr;
    }

    PUSH(fobj_fetch(f, a, b));
}

FWORD2(store, "!")
{
    A = POP;  // Object to store into
    B = NULL;     // Index
    if (fobj_is_index(f, a)) {
        b = a->u.index.index;
        a =  a->u.index.addr;
    }
    C = POP;

    fobj_store(f, a, b, c);
}

FWORD2(mktable, "{}")
{
    PUSH(ftable_new(f));
}

FWORD2(index, "]")
{
    B = POP;
    A = POP;

    PUSH(findex_new(f, a, b));
}

FWORD2(one, "1")
{
    PUSHN(1);
}

/*
 **********************************************************
 *
 * I/O Words
 *
 **********************************************************
 **/

FWORD(emit)
{
    char c = POPN;
    printf("%c", c);
}

/*
 **********************************************************
 *
 * Compiling Words
 *
 **********************************************************
 **/

#define CURRENT		(&f->current_compiling->u.word)

static void forth_compile_alloc(fenv_t *f)
{
    fword_t *w = CURRENT;

    if (w->body_offset < w->body_allocated) {
        return;
    }

    w->body_allocated += 64;  // Arbitrary
    w->u.body = realloc(w->u.body, sizeof (*w->u.body) * w->body_allocated);
}

static void forth_compile_word(fenv_t *f, fobj_t *c, int offset)
{
    fword_t *w = CURRENT;

    forth_compile_alloc(f);
    w->u.body[w->body_offset].word = c;
    w->u.body[w->body_offset].n = offset;
    w->body_offset ++;
}

static void forth_compile_cons(fenv_t *f, fnumber_t n)
{
    fobj_t *cons = fnum_new(f, n);
    PUSH(cons);
    fobj_t *name = fstr_new(f, "constant");
    PUSH(name);
    fobj_t *t = fcode_new(f, name, fcode_do_constant_header.code, 0, NULL, cons);
    PUSH(t);
    forth_compile_word(f, t, 0);
    DROP; DROP; DROP;
}

FWORD_DO(var)
{
    PUSH(w);  // Use @ and ! to read or modify w's u.value field.
}

#define RP		(f->rstack->u.stack.sp)
#define IP		(f->ip)
#define CALL(w)	  ((w)->u.word.code(f, w))
#define NEST      RPUSH(IP)
#define UNNEST    (IP = RPOP)

FWORD_DO(exit)
{
    fobj_t *wp = RPOP;
    fassert(f, wp->type == FOBJ_CALL, 1,
            "the return stack does not contain a return address for an exit");

    f->running = wp->u.call.w;
    f->ip = wp->u.call.ip;
}

FWORD_DO(colon)
{
    int depth_saved = RDEPTH;
    fobj_t *wp = fobj_new(f, FOBJ_CALL);

    wp->u.call.w = f->running;
    wp->u.call.ip = IP;
    RPUSH(wp);

    IP = w->u.word.u.body;
    f->running = w;

    do {
        fobj_t *nw = IP++ -> word;
        CALL(nw);
    } while (RDEPTH > depth_saved);
}

FWORD_DO(constant)
{
    PUSH(w->u.word.u.value);
}

FWORD2(mkvar, "var")
{
    fobj_t *var_name;

    (void) fparse_token(f, &var_name);
    fassert(f, !!var_name, 1, "var must be followed by a variable name");

    ftable_store(f, f->new_words, var_name, NULL);
}

/*
 * DO(const)
 *
 * This is the second half of IMM(const).  In the immediate code, the name
 * was read and an entry was created with the code field pointing here.
 * Then, when the code ran for the first time, this code will POP a value off
 * the stack and store that value into this word's value field.  And, it will
 * change the code field to point to DO(constant) where -that- code will
 * -push- the value field on the stack whenever the word is used.  Whew!
 */
FWORD_DO(const)
{
    fobj_t *cons_value = POP;
    fassert(f, !!cons_value, 1, "const must be preceded by a non-null value");

    w->u.word.u.value = cons_value;
    w->u.word.code = fcode_do_constant_header.code;
}

/*
 * IMM(const)
 *
 * Immediately run and parse the following token.  However, POP doesn't
 * work here because the code is running in the compiler.  So, we compile a
 * code word to update a constant at run time.  Huh?  What constant?  The
 * problem is that any code that follows here wouldn't find the newly
 * minted constant.  So, instead we add a word to the dictionary.
 */

FWORD_IMM(const)
{
    fobj_t *cons_name;
    (void) fparse_token(f, &cons_name);
    fassert(f, !!cons_name, 1, "const must be followed by a name");
    fobj_t *cons = fcode_new(f, cons_name, fcode_do_const_header.code, 0, NULL, NULL);

    ftable_store(f, f->new_words, cons_name, cons);
    forth_compile_word(f, cons, 0);
}

/**********************************************************
 *
 * Branch Words
 *
 **********************************************************/

FWORD_DO(branch)
{
    IP += (int) ((IP -1) ->n);
}

FWORD_DO(zbranch)
{
    if (POP == 0) {
        fcode_do_branch(f, w);
    } else {
        // Branch offset a part of the previous instruction;
        // Skipped.
    }
}

/**********************************************************
 *
 * Control Words
 *
 **********************************************************/

#define FSTATE_COLON       1
#define FSTATE_DO          2
#define FSTATE_IF          3

static int forth_state(fenv_t *f)
{
    if (DEPTH == 0) {
        return 0;
    }

    fobj_t *p = POP;
    PUSH(p);
    if (p->type != FOBJ_STATE) {
        return 0;
    }

    return p->u.state.state;
}

static void forth_state_push(fenv_t *f, int state, int offset)
{
    PUSH(fstate_new(f, state, CURRENT->body_offset));
}

static fobj_t *forth_state_pop(fenv_t *f)
{
    fobj_t *p = POP;

    fassert(f, p->type == FOBJ_STATE, 1, "compiler state not top of the stack");

    return p;
}

static void forth_mark(fenv_t *f, fobj_t *branch_word, int state_type)
{
    if (branch_word) {
        forth_compile_word(f, branch_word, 0); // Unknown offset at this time
    }

    forth_state_push(f, state_type, CURRENT->body_offset);
}

static int forth_resolve(fenv_t *f, int state_type)
{
    fassert(f, forth_state(f) == state_type,
                 1,
                 "Control words must be matched properly: if [else] then, do loop, : ; etc.");
    fobj_t *mark = POP;
    assert(mark->type == FOBJ_STATE);  // Should be an fassert()
    assert(mark->u.state.state == state_type);  // Logic error in this code if not
    int offset = mark->u.state.offset;
    CURRENT->u.body[offset -1].n = CURRENT->body_offset - offset;
    return offset;
}

static void forth_back_branch(fenv_t *f, fobj_t *branch_word, int target_offset)
{
    forth_compile_word(f, branch_word, - (CURRENT->body_offset - target_offset));
}

FWORD_IMM(if)
{
    forth_mark(f, do_zbranch, FSTATE_IF);
}

FWORD_IMM(else)
{
    fassert(f, forth_state(f) == FSTATE_IF,
                 1,
                 "else must follow an if");
    fobj_t *if_mark = POP;
    forth_mark(f, do_branch, FSTATE_IF);
    PUSH(if_mark);
    forth_resolve(f, FSTATE_IF);
}

FWORD_IMM(then)
{
    forth_resolve(f, FSTATE_IF);
}

FWORD_DO(do)
{
    int start = POPN;
    int limit = POPN;
    fobj_t *loop = fobj_new(f, FOBJ_LOOP);
    loop->u.loop.limit = limit;
    loop->u.loop.index = start;
    RPUSH(loop);
}

FWORD_DO(loop)
{
    // Pop the loop info off the return stack and process it
    fobj_t *p = RPOP;
    fassert(f, p->type == FOBJ_LOOP, 1, "attempting a looping word without a loop condition on the stack.");
    floop_t *do_loop = &p->u.loop;

    do_loop->index ++;
    if (do_loop->index >= do_loop->limit) {
        // Do nothing
    } else {
        RPUSH(p);
        IP += (int) ((IP -1) ->n);
    }
}

FWORD(i)
{
    fobj_t *p = RPOP;
    RPUSH(p);

    fassert(f, p->type == FOBJ_LOOP, 1, "attempting a looping word without a loop condition on the stack.");
    floop_t *do_loop = &p->u.loop;

    PUSHN(do_loop->index);
}    


FWORD_IMM(do)
{
    forth_mark(f, NULL, FSTATE_DO);
    forth_compile_word(f, do_do, 0);
}

FWORD_IMM(loop)
{
    fassert(f, forth_state(f) == FSTATE_DO,
                 1,
                 "loop must follow a do");

    fobj_t *do_mark = forth_state_pop(f);

    forth_back_branch(f, do_loop, do_mark->u.state.offset);
}
    

/**********************************************************
 *
 * Defining Words (e.g., : ;)
 *
 **********************************************************/

/*
 * colon()
 *
 * Continue executing words until an exit pops an IP and the RP
 * finally returns to its value on entry.
 */

FWORD_IMM2(colon, ":")
{
    // Verfiy we're not currently compiling or otherwise encumbered
    fassert(f, !forth_state(f),
                 1,
                 "Cannot use the colon word inside a do, if, etc.");
    fassert(f, f->in_colon == 0, 1, "Can't compile inside a colon definition");

    PUSH(f->current_compiling);
    f->in_colon = 1;

    // Allocate memory for the header
    f->current_compiling = fobj_new(f, FOBJ_WORD);

    // Fetch the next token, i.e., the name of the new word
    fobj_t *name_token;
    int r = fparse_token(f, &name_token);
    fassert(f, r, 1, "Need more input");

    CURRENT->name = name_token;
    CURRENT->code = fcode_do_colon_header.code;
    CURRENT->immediate = 0;
}

FWORD_IMM2(semicolon, ";")
{
    fassert(f, forth_state(f) == 0,
            1,
            "Semicolon (;) can only be used used to terminate a colon definition");
    fassert(f, f->in_colon == 1,
            1,
            "Semicolon (;) can only be used used to terminate a colon definition");

    // Compile an exit word
    forth_compile_word(f, do_exit, 0);

    ftable_store(f, f->new_words, CURRENT->name, f->current_compiling);
    f->current_compiling = POP;
    f->in_colon = 0;
}


void fcode_handle_token(fenv_t *f, fobj_t *token)
{
    ASSERT(token->type == FOBJ_STR);
    fobj_t *w = ftable_fetch(f, f->words, token);
    if (!w) w = ftable_fetch(f, f->new_words, token);
    if (w) {
        ASSERT(w->type == FOBJ_WORD);
        if (w->u.word.immediate) {
            w->u.word.code(f, w);
        } else {
            forth_compile_word(f, w, 0);
        }
    } else {
        fnumber_t n = 0;
        int r = fparse_token_to_number(f, token, &n);
        fassert(f, r, 1,
                "Input token not found in dictionary and isn't a number: <%s>",
                token->u.str.buf);
        forth_compile_cons(f, n);
    }
}

void fcode_compile_string(fenv_t *f, const char *string)
{
    fobj_t *token;

    f->input_str = fstr_new(f, string);
    f->new_words = ftable_new(f);

    f->current_compiling = fcode_new(f, fstr_new(f, "input string"),
                                     fcode_do_colon_header.code,
                                     /* immediate */ 0,
                                     NULL, NULL);

    do {
        int r = fparse_token(f, &token);
        if (!r) break;
        fcode_handle_token(f, token);
    } while (1);
    forth_compile_word(f, do_exit, 0);

    /*
     * Merge f->new_words into f->words.  This algorithm should probably be
     * in fhash.c.
     */

    fhash_t *new_words = &f->new_words->u.table.hash->u.hash;
    for (int i = 0; i < new_words->num_kv; i++) {
        ftable_store(f, f->words, 
                     new_words->keys_values[i * 2],
                     new_words->keys_values[i * 2 + 1]);
    }

    CURRENT->code(f, f->current_compiling);
    f->current_compiling = NULL;
}
