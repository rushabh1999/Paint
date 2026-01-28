CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude
LDFLAGS = -lssl -lcrypto

SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin

SOURCES = $(SRC_DIR)/drift_detector.c $(SRC_DIR)/main.c
OBJECTS = $(BUILD_DIR)/drift_detector.o $(BUILD_DIR)/main.o
TARGET = $(BIN_DIR)/drift_detector

.PHONY: all clean install

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $(TARGET)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/

.PHONY: test
test: $(TARGET)
	@echo "Running basic tests..."
	@echo "Test file content" > /tmp/test_drift.txt
	@$(TARGET) baseline /tmp/test_drift.txt
	@$(TARGET) verify /tmp/test_drift.txt
	@$(TARGET) check /tmp/test_drift.txt
	@echo "Modifying test file..."
	@echo "Modified content" > /tmp/test_drift.txt
	@$(TARGET) verify /tmp/test_drift.txt || echo "Expected: Hash mismatch detected"
	@$(TARGET) check /tmp/test_drift.txt || echo "Expected: Drift detected"
	@rm /tmp/test_drift.txt
	@echo "Tests completed"
