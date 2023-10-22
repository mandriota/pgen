CC ?=
CFLAGS := -Wall -Wextra -Werror

run: build
	./pgen $(a)

build: main.c
	$(CC) $(CFLAGS) -o pgen main.c
