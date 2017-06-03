all:
	gcc -std=c99 -o main -fPIC -Os -Wall -lpaho-mqtt3c main.c -lbcm2835 -ltm1638
