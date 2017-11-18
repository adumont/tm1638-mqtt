all:
	gcc -std=c99 -o tm1638mqtt -fPIC -Os -Wall -lpaho-mqtt3c tm1638mqtt.c -lbcm2835 -ltm1638 -pthread 
