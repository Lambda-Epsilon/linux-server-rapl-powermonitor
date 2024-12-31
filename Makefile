# Makefile

CC = gcc
CFLAGS = -O2 -Wall

all: rapl_monitor

rapl_monitor: rapl_monitor.c
	$(CC) $(CFLAGS) -o rapl_monitor rapl_monitor.c

clean:
	rm -f rapl_monitor
