# shiftcipher - Shift cipher encrypter and decrypter

include config.mk

SRC = shiftcipher.c
OBJ = ${SRC:.c=.o}

all: options shiftcipher

options:
	@echo shiftcipher build options:
	@echo "CC       = ${CC}"
	@echo "CFLAGS   = ${CFLAGS}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

shiftcipher: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f shiftcipher ${OBJ}

.PHONY: all options clean
