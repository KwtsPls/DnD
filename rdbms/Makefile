# compile with gcc
CC=gcc

# Compiler options:
#
# -pg is used for profiling (gprof)
CFLAGS = -g -Wall -Werror $(shell pkg-config --cflags glib-2.0)

# Linker options:
#
# -lm: link with the math library
LDFLAGS += -lm
LDLIBS = $(shell pkg-config --libs glib-2.0)

# set the name of the executable file to compile here
PROGRAM = main
OBJS = main.o ./compiler/tokenizer.o ./compiler/parser.o


$(PROGRAM): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $(PROGRAM) $(LDLIBS)

clean:
	rm $(PROGRAM) $(OBJS)