# = INPUT AND OUTPUT FILES =

## Output
OUT_DIR = bin
BIN     = spnotes-gui

## Source File(s)
SRC = ${BIN}.c
DEP =

## Object File(s)
OBJ_DIR = obj
OBJ     = $(addprefix ${OBJ_DIR}/,$(patsubst %.c,%.o,${SRC}) $(patsubst %.c,%.o,${DEP}))

# = COMPILER OPTIONS =

PKGS =

# Initialize CFLAGS, DFLAGS and LIBS value
CFLAGS  = -std=c99 -pedantic -Wall -Wextra -Wno-deprecated-declarations
DFLAGS ?= -ggdb
INCS    = -I/usr/include/iup
LIBS    = -liup

# Add options to CFLAGS and LIBS if required
ifneq (${PKGS},)
CFLAGS += `pkg-config --cflags ${PKGS}`
LIBS   += `pkg-config --libs ${PKGS}`
endif

# = TARGETS =

all: ${OBJ_DIR} ${OUT_DIR} ${OUT_DIR}/${BIN}

${OUT_DIR}/${BIN}: ${OBJ}
	${CC} ${OBJ} -o $@ ${LIBS}

${OBJ_DIR}/%.o: %.c
	${CC} ${CFLAGS} ${DFLAGS} ${INCS} -c -o $@ $<

${OBJ_DIR}:
	mkdir -p $(dir ${OBJ})

${OUT_DIR}:
	mkdir $@

release:
	DFLAGS= make

clean:
	rm -rf ${OUT_DIR} ${OBJ_DIR}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f ${OUT_DIR}/${BIN} ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/${BIN}

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/${BIN}

.PHONY: all release clean install uninstall
