# PISketch

## Introduction
Finding persistent and infrequent items is a critical issue in databases. It is helpful to find persistent and inactive activities, especially intrusion activities, which are challenging to detect. The majority of works focus on finding persistent items or frequent items, and none of the existing works can find persistent and infrequent items. In this paper, we first propose a sketch named PI sketch for finding persistent and infrequent items. The PI sketch’s key idea is to estimate the probability that an item is persistent and infrequent according to its duration and frequency and keep persistent and infrequent items as many as possible. We compare the performance of the PI Sketch with a straightforward method in terms of finding persistent and infrequent items. Experimental results show that the PI sketch achieves 30 ∼ 1000 times higher accuracy of measuring
item weights than the method.

## About this repo
- `data`: generate synthetic datasets for tests, each item in a dataset is 16 bytes: 8 bytes timestamp (meaningless, to conform the CAIDA format), 8 bytes ID.
  - `compile.sh`: 5 arguments after `./sim.out`: number of PI flows, number of all flows, window size, numerator of alpha, denominator of alpha.
- `src`: contains codes of data structures and parameters.
- `trace`: another set of codes of generating synthetic datasets (not used in the paper).
- `compile.sh`: 6 arguments after `./demo.out`: number of buckets, number of cells per bucket, persistency threshold, numerator of infrequency threshold, denominator of infrequency threshold, window size.
- `demo.cpp`: main codes of marking the PI flows, taking the tests and calculating the metircs.
- `Detector.h`: main codes of data structures and operations of PI sketch.

## Requirements
- g++

## How to make
- `cd data; ./compile.sh; cd ..; ./compile.sh` to take the tests. 
- You may need to change some file paths in `./data/GenSimData.cpp` and `./demo.cpp`.
- You may need to change some parameters in `./data/compile.sh` and `./compile.sh`.
