
VPATH = src
OBJ = main.o options.o core.o binding.o

ckains: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)