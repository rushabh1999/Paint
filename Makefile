# Makefile for Internet Uptime Tracker (Windows)
# Requires MinGW or similar GCC toolchain for Windows

CC = gcc
CFLAGS = -Wall -Wextra -O2 -I./include
LDFLAGS = -liphlpapi -lws2_32
TARGET = uptime_tracker.exe
SRCDIR = src
OBJDIR = obj
INCDIR = include

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Default target
all: $(TARGET)

# Create object directory if it doesn't exist
$(OBJDIR):
	mkdir $(OBJDIR)

# Link the executable
$(TARGET): $(OBJDIR) $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo Build complete: $(TARGET)

# Compile source files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	if exist $(OBJDIR) rmdir /s /q $(OBJDIR)
	if exist $(TARGET) del $(TARGET)
	@echo Cleaned build artifacts

# Install the service (requires Administrator privileges)
install: $(TARGET)
	@echo Installing service...
	$(TARGET) install
	@echo Service installed. Use 'sc start InternetUptimeTracker' to start it.

# Uninstall the service (requires Administrator privileges)
uninstall:
	@echo Uninstalling service...
	$(TARGET) uninstall
	@echo Service uninstalled.

# Run in console mode for testing
console: $(TARGET)
	$(TARGET) console

# Help target
help:
	@echo Internet Uptime Tracker - Build Instructions
	@echo.
	@echo Available targets:
	@echo   all        - Build the executable (default)
	@echo   clean      - Remove build artifacts
	@echo   install    - Build and install as Windows service (requires Admin)
	@echo   uninstall  - Uninstall the Windows service (requires Admin)
	@echo   console    - Build and run in console mode for testing
	@echo   help       - Show this help message
	@echo.
	@echo Requirements:
	@echo   - GCC compiler (MinGW or similar)
	@echo   - Windows OS
	@echo.

.PHONY: all clean install uninstall console help
