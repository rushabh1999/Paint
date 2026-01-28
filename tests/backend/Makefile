CC = gcc
CFLAGS = -Wall -Wextra -O2 -g -I../../backend
BACKEND_DIR = ../../backend
BACKEND_OBJS = $(BACKEND_DIR)/config.o $(BACKEND_DIR)/drift.o $(BACKEND_DIR)/cJSON.o

all: test_backend

test_backend: test_backend.c $(BACKEND_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BACKEND_DIR)/%.o: $(BACKEND_DIR)/%.c
	cd $(BACKEND_DIR) && $(MAKE) $*.o

clean:
	rm -f test_backend

run: test_backend
	./test_backend

.PHONY: all clean run
