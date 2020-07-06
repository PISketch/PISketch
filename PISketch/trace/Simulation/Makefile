GCC = g++
CFLAGS = -O2 -w -std=c++14 
SSEFLAGS =   -march=native
FILES = sim.out

all: $(FILES)

sim.out: GenSimData.cpp
	$(GCC) $(CFLAGS)  -o sim.out GenSimData.cpp
.PHONY: clean
clean:
	-rm -f *.out $(objs)
