#!/bin/sh
bin/$1 test/$3.c -o test/$3.o
bin/$2 test/$3.o
