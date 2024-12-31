# Makefile

CC      = gcc
CFLAGS  = -O2 -Wall
TARGET  = rapl_monitor
SOURCE  = rapl-power.c

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET)
