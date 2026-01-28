CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
LDFLAGS = 
TARGET = drift_detector
OBJS = main.o config.o monitor.o drift.o alert.o socket_server.o daemon.o cJSON.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c drift_detector.h
	$(CC) $(CFLAGS) -c $<

cJSON.o: cJSON.c cJSON.h
	$(CC) $(CFLAGS) -c cJSON.c

clean:
	rm -f $(OBJS) $(TARGET)

install:
	install -m 755 $(TARGET) /usr/local/bin/

uninstall:
	rm -f /usr/local/bin/$(TARGET)

.PHONY: all clean install uninstall
