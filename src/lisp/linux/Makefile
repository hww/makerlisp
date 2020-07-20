# Make MakerLisp

# Tools and rules
OPTIMIZE=	-O3
CFLAGS=		-Wall -I$(PLATFORM)

.SUFFIXES:
.SUFFIXES: .c .h

# Components
LISP=		lisp
SRC=		..
LISPSRC=	$(SRC)/lisp.c

PLATFORM=	.
OSSRC=		$(PLATFORM)/platform.c
OSHDR=		$(PLATFORM)/platform.h

HEADERS=	$(OSHDR)
SOURCES=	$(LISPSRC) $(OSSRC)
MATHLIB=	-lm

# Targets
all:		$(LISP)

$(LISP):	$(SOURCES) $(HEADERS)
		$(CC) -o $@ $(CFLAGS) $(OPTIMIZE) $(SOURCES) $(MATHLIB)

reference:	$(LISP)
		@./runtests.sh reference

test:		$(LISP)
		@./runtests.sh test

clean:
		rm -f $(LISP)

clobber:	clean
		nclb clean --force > /dev/null

# Dependencies
