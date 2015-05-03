# Elisei Alexandru 316CC
# Structuri de Date
# Tema 2

# Number of cores to be used by make; number of physical cores + 1
CC = "gcc'
MAKEOPTS = "-j9"
override CFLAGS += "-Wall"
PROG = "huffman"

# HEADERS = 

SOURCES = main.c
OBJS = $(SOURCES:%.c=%.o)

.PHONY: build
#build: $(PROG)
