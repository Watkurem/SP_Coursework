SHELL=/bin/zsh
CC=gcc
CFLAGS=-g -c -fdiagnostics-color=always -std=gnu11 -Wall -Wpedantic -Wextra -MMD -MP
ELF = xas

.PHONY: clean all rebuild nice

all : $(ELF)

nice : $(ELF)
	rm -f **/*.o(N) **/*.d(N)

$(ELF) : $(patsubst %.c, %.o, $(shell print **/*.c))
	$(CC) $^ -o $@

%.o : %.cpp
	$(CC) $(CFLAGS) $< -o $@

clean: clean_specific
	rm -f **/*.o(N) **/*.d(N) $(ELF)

clean_specific:


rebuild: clean all

-include $(patsubst %.c, %.d, $(shell print **/*.c))
