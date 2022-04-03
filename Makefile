# ---------------------------------------------------------------------

L = NO
D = NO

SRCS = ${shell ls *.c}
OBJS = ${SRCS:.c=.o}
TARGET = esmd

# ---------------------------------------------------------------------

ifeq (${L},NO)
CROSS_COMPILE = sh4-linux-
endif 
CC = ${CROSS_COMPILE}gcc
LD = ${CROSS_COMPILE}gcc
AS = ${CROSS_COMPILE}as
OD = ${CROSS_COMPILE}objdump
OC = ${CROSS_COMPILE}objcopy
ST = ${CROSS_COMPILE}strip

RM=rm -rf
ECHO=echo
CTAGS=ctags

ifeq (${L},NO)
CFLAGS  =  -D__USE_GNU -D_GNU_SOURCE -I. -O0 -Wall -gdwarf-2 -m4-nofpu -ml -pthread
AFLAGS  = 
LDFLAGS = -pthread
else
CFLAGS  = -DIN_LOCAL -D__USE_GNU -D_GNU_SOURCE -I. -O0 -Wall -g -pthread
AFLAGS  = 
LDFLAGS = -pthread
endif

ifeq (${D},NO)
CFLAGS+= -DNDEBUG
endif

# ---------------------------------------------------------------------
.SUFFIXES:
.SUFFIXES: .o .s .c .out .mot

.s.o:
	${AS}    ${AFLAGS} $< -o $@

.c.o:
	${CC} -c ${CFLAGS} $< -o $@

.out.mot:
	${OC} -R .bss -R .bss0 -R .comment -O srec -S $< $@

# ---------------------------------------------------------------------
.PHONY: all clean tags pretest posttest

all: tags depend ${TARGET}

${TARGET}: ${OBJS}
	${LD} -o $@ ${OBJS} ${LDFLAGS}

clean:
	-${RM} *.o
	-${RM} ${TARGET}
	-${RM} *~ core tags
	-${ECHO} > Makefile.deps

tags:
	-ctags -R *.c *.h

depend:
	$(CC) $(CFLAGS) -M $(SRCS) $(TSRCS) >> Makefile.deps

Makefile.deps: depend

include Makefile.deps

#  End of Makefile
