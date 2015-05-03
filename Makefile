# Elisei Alexandru 316CC
# Structuri de Date
# Tema 2

CC = "gcc"
override CFLAGS += "-Wall"
PROG = "huffman"

HEADERS = common.h

SOURCES = main.c			\
	  $(HEADERS:%.h=%.c)

OBJS = $(SOURCES:%.c=%.o)

.PHONY: build
build: $(PROG)

$(PROG): $(OBJS) $(HEADERS)
	$(CC) $(OBJS) -o $(PROG) $(CFLAGS)
	
%.o: %.c
	$(CC) -c $^ -o $@ $(CFLAGS)

.PHONY: clean
clean:
	rm -rf a.out $(PROG) $(OBJS)
