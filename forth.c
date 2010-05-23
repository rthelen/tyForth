/*
 * This file is part of arm-sim: http://madscientistroom.org/arm-sim
 *
 * Copyright (c) 2010 Randy Thelen. All rights reserved, and all wrongs
 * reversed. (See the file COPYRIGHT for details.)
 */

#include "forth.h"

void zzz(void)
{
    fflush(stdout);
}

void forth_internal_assert(const char *err, const char *file, int line)
{
    zzz();
    fprintf(stderr, "INTERNAL ERROR: %s in file %s on line %d\n", err, file, line);
    exit(-1);
}

void forth_runtime_assert(fenv_t *f, const char *fmt, ...)
{
    va_list ap;

    zzz();
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);

    exit(-1);
}

static void forth_test_string(const char *string)
{
    printf("Executing the Forth string: %s\n", string);

    fenv_t *f = fenv_new();
    fcode_init(f);
    fcode_compile_string(f, string);
    fenv_free(f);

    printf("\n");
}

int main(int argc, char *argv[])
{
    forth_test_string("5 begin 1 - dup while dup . repeat 13 emit");
    forth_test_string("1.25 2 1.5 + + .");
    forth_test_string("{} constant arr   10 arr 1 ] !   20 arr 2 ] !  arr 1 ] @ .  arr 2 ] @ .  arr 3 ] @ .");
    forth_test_string("5 3 + . : eight 5 3 + ; eight eight * .");
    forth_test_string("10 1 do i . loop");
    forth_test_string(": bl 32 emit ; "
                      ": cr 13 emit 10 emit ; "
                      ": star 42 emit ; "
                      ": stars 0 do star loop ; "
                      ": spaces 0 do bl loop ; "
                      ": tree-top dup 0 do dup 1 + i - spaces i 2* 1 + stars cr loop drop ; "
                      ": trunk 5 0 do dup 1 + spaces star cr loop drop ; "
                      ": tree dup tree-top trunk ; "
                      "5 tree 20 tree");
    forth_test_string(": hello 72 emit 101 emit 108 emit 108 emit 111 emit ; "
                      "1 if hello then");
    forth_test_string("7919 2/ constant maxp "
                      "{} constant sieve_map "
                      ": primes "
                      "   maxp 0 do 1 sieve_map i ] ! loop "
                      "   1 "
                      "   maxp 0 do "
                      "      sieve_map i ] @ if "
                      "         i 2* 3 + dup . dup i + "
                      "         begin dup maxp < "
                      "         while 0 over sieve_map swap ] ! "
                      "             over + "
                      "         repeat "
                      "         2drop 1+ "
                      "      then "
                      "   loop ; "
                      " primes .");
    return 0;
}
