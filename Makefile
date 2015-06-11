
VPATH = src
OBJ = main.o options.o core.o

ckains: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)