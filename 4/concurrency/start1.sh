#!/bin/bash
rm -f ./out >&1
g++ -std=c++11 -lpthread problem1.cpp -o out >&2
./out

