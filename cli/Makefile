# = PROGRAM CONFIG =
VERSION = 0.2

# = INPUT AND OUTPUT FILES =

## Output directory
OUT_DIR = bin
## Output executable
BIN     = spnotes-cli

## Source File(s)
SRCS    = ${BIN}.c
DEPS    =

# = COMPILER OPTIONS =

CFLAGS  = -std=c99 -pedantic -Wall -Wextra -Wno-deprecated-declarations -DVERSION=\"${VERSION}\"
DFLAGS ?= -g
LIBS    =

# = TARGETS =

all: ${OUT_DIR}/${BIN}

${OUT_DIR}/${BIN}: ${SRCS} ${OUT_DIR}
	${CC} ${CFLAGS} ${DFLAGS} ${SRCS} ${DEPS} -o $@ ${LIBS}

release:
	DFLAGS=-DRELEASE make

${OUT_DIR}:
	mkdir $@

clean:
	rm -rf ${OUT_DIR}

install: ${OUT_DIR}/${BIN}
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f ${OUT_DIR}/${BIN} ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/${BIN}
	cp -f wrapper-scripts/* ${DESTDIR}${PREFIX}/bin

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/${BIN}

.PHONY: all debug clean install uninstall
