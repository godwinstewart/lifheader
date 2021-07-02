.PHONY: clean install

SRC = lifheader.c liffiletype.c
OBJ = $(SRC:.c=.o)
HDR = $(SRC:.c=.h)


ifeq ($(OS),Windows_NT)
lifheader.exe: $(OBJ)
	gcc -o lifheader -Wall $(OBJ) -lws2_32
else
lifheader: $(OBJ)
	gcc -o lifheader -Wall $(OBJ)
endif

$(OBJ): %.o: %.c $(HDR)
	gcc -Wall -Wextra -pedantic -c $< -o $@

ifeq ($(OS),Windows_NT)
clean:
	rm -fv *.o lifheader.exe
else
clean:
	rm -fv *.o lifheader
endif

ifneq ($(OS),Windows_NT)
install: lifheader
	cp -v lifheader /usr/bin
endif