#!/bin/bash


for depth in 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30
do 
	echo Now running quicksort with depth of $depth...
    echo -e "  1 Thread..."
	echo -e "\nDepth: $depth   Threads: 1\n"    >> out.results52
	./quicksort -q -s 42 -d $depth -n 1  300000000 >> out.results52
	echo -e "  4 Thread..."
	echo -e "\nDepth: $depth   Threads: 4\n" >> out.results52
    ./quicksort -q -s 42 -d $depth -n 4  300000000 >> out.results52
	echo -e "  6 Thread..."
	echo -e "\nDepth: $depth   Threads: 6\n" >> out.results52
    ./quicksort -q -s 42 -d $depth -n 6  300000000 >> out.results52
	echo -e "  8 Thread..."
	echo -e "\nDepth: $depth   Threads: 8\n" >> out.results52
    ./quicksort -q -s 42 -d $depth -n 8  300000000 >> out.results52
	echo -e "  12 Thread..."
	echo -e "\nDepth: $depth   Threads: 12\n" >> out.results52
    ./quicksort -q -s 42 -d $depth -n 12 300000000 >> out.results52
	echo -e "  16 Thread..."
	echo -e "\nDepth: $depth   Threads: 16\n" >> out.results52
    ./quicksort -q -s 42 -d $depth -n 16 300000000 >> out.results52
done
