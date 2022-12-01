#!make -f
CC=gcc
CXX=g++
FLAGS=-Werror

all: sniffer myping

sniffer: sniffer.c
	$(CC) $(FLAGS) sniffer.c -o sniffer -lpcap

myping: myping.cpp
	$(CXX) $(FLAGS) myping.cpp -o myping

clean:
	rm -f myping sniffer
