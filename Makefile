CFLAGS = -O2

VPATH = src
OBJ = main.o options.o core.o binding.o

all: ckains

ckains: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

clean:
	rm -f *.o && rm -f ckains
