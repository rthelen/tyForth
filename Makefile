#
# This file is part of arm-sim: http://madscientistroom.org/arm-sim
#
# Copyright (c) 2010 Randy Thelen. All rights reserved, and all wrongs
# reversed. (See the file COPYRIGHT for details.)
#

SRC  = forth.c fobj.c fnum.c fstr.c ftable.c farray.c fhash.c fstack.c fprimitives.c
OBJS = $(patsubst %.c, objects/%.o, ${SRC})
INCL = forth.h fobj.h

CFLAGS = -Wall -Werror -std=c99

ifneq ($(DEBUG),)
	CFLAGS += -ggdb -DDEBUG
else
	CFLAGS += -O2
endif

forth: clean ${OBJS} ${INCL}
	cc ${OBJS} -o $@

objects/forth.o: fwords.inc

fwords.inc: fprimitives.c forth.h gen_fword_inc.pl
	./gen_fword_inc.pl < $< > $@

.PHONY: objects
objects:
	@mkdir -p objects

objects/%.o: %.c ${INCL} objects
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@

clean:
	rm -f *~
	rm -rf objects
	rm -f forth fwords.inc
