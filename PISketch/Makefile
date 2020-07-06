GCC = g++
CFLAGS = -O2 -w -std=c++14 
SSEFLAGS =   -march=native
FILES = demo.out 

all: $(FILES) 

demo.out: demo.cpp
	$(GCC) $(CFLAGS) $(SSEFLAGS) -o demo.out demo.cpp

.PHONY: clean
clean:
	-rm -f *.out $(objs)
