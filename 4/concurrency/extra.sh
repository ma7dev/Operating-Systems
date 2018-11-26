#!/bin/bash
rm -f ./out >&1
g++ -std=c++11 -lpthread extra.cpp -o out >&2
./out

