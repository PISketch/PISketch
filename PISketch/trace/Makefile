GCC = g++
CFLAGS = -std=c++14
SSEFLAGS =   -march=native
FILES = demo.out

all: $(FILES)

demo.out: main.cpp
	$(GCC) $(CFLAGS)  -o demo.out main.cpp
clean:
	rm $(all) -f *~ *.o *.out
