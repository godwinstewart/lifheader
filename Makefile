.PHONY: clean install

SRC = lifheader.c liffiletype.c
OBJ = $(SRC:.c=.o)
HDR = $(SRC:.c=.h)

lifheader: $(OBJ)
	gcc -o lifheader -Wall $(OBJ)

$(OBJ): %.o: %.c $(HDR)
	gcc -Wall -Wextra -pedantic -c $< -o $@

clean:
	rm -fv *.o lifheader

install: lifheader
	cp -v lifheader /usr/bin
