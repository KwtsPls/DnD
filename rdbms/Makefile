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
OBJS = main.o ./compiler/tokenizer.o ./compiler/parser.o ./compiler/semantic.o ./disk/block.o ./memory/buffer_manager.o ./memory/priority_queue.o ./table/databox.o ./table/record.o ./table/table.o ./table/result_set.o ./db_files/heapfile.o ./db_files/result_file.o ./db_files/db_file.o ./joins/bnl.o

PROGRAM_CREATE = create_files
OBJS_CREATE = create_files.o ./compiler/tokenizer.o ./compiler/parser.o ./disk/block.o ./memory/buffer_manager.o ./memory/priority_queue.o ./table/databox.o ./table/record.o ./table/table.o ./table/result_set.o ./db_files/heapfile.o ./db_files/result_file.o


$(PROGRAM): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $(PROGRAM) $(LDLIBS)

$(PROGRAM_CREATE): $(OBJS_CREATE)
	$(CC) $(CFLAGS) $(OBJS_CREATE) $(LDFLAGS) -o $(PROGRAM_CREATE) $(LDLIBS)

clean:
	rm $(PROGRAM) $(OBJS) $(PROGRAM_CREATE) $(OBJS_CREATE)