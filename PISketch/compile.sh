#!/bin/bash
make clean
make
./demo.out 1000 8 40 5 1 100000
./demo.out 2000 8 40 5 1 100000
./demo.out 4000 8 40 5 1 100000
./demo.out 8000 8 40 5 1 100000


