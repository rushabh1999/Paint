# Build and Test Guide

This document provides detailed information about building, testing, and developing the Internet Uptime Tracker.

## Build Requirements

### Required Tools

1. **GCC Compiler (MinGW-w64)**
   - Download from: https://www.mingw-w64.org/
   - Or use MSYS2: https://www.msys2.org/
   - Version: GCC 8.0 or later recommended

2. **Make Utility**
   - Included with MinGW-w64
   - Or install separately from GnuWin32

3. **Windows SDK** (optional, usually included with MinGW)
   - For Windows API headers
   - IPHLPAPI.lib and WS2_32.lib

### Optional Tools

- **Python 3.6+** for utility scripts
- **Git** for version control
- **Visual Studio Code** or other IDE for development

## Building from Source

### Method 1: Using Make (Recommended)

```cmd
cd C:\path\to\Paint
make
```

This will:
1. Create the `obj` directory
2. Compile all `.c` files to `.o` object files
3. Link the object files into `uptime_tracker.exe`

### Method 2: Using MinGW Directly

```cmd
gcc -Wall -Wextra -O2 -Iinclude -c src/main.c -o obj/main.o
gcc -Wall -Wextra -O2 -Iinclude -c src/service.c -o obj/service.o
gcc -Wall -Wextra -O2 -Iinclude -c src/ping_checker.c -o obj/ping_checker.o
gcc -Wall -Wextra -O2 -Iinclude -c src/state_machine.c -o obj/state_machine.o
gcc -Wall -Wextra -O2 -Iinclude -c src/logger.c -o obj/logger.o
gcc -Wall -Wextra -O2 -Iinclude -c src/statistics.c -o obj/statistics.o
gcc obj/*.o -o uptime_tracker.exe -liphlpapi -lws2_32
```

### Method 3: Using MSVC (Visual Studio)

Create a new Console Application project and:
1. Add all `.c` files from `src/` directory
2. Add `include/` directory to include paths
3. Link against `iphlpapi.lib` and `ws2_32.lib`
4. Build the project

## Build Options

### Debug Build

```cmd
gcc -Wall -Wextra -g -O0 -Iinclude -DDEBUG ...
```

### Release Build (Optimized)

```cmd
gcc -Wall -Wextra -O2 -Iinclude ...
```

### Static Build (Standalone)

```cmd
gcc -Wall -Wextra -O2 -Iinclude ... -static
```

## Testing

### Unit Testing Strategy

The project uses Python scripts for testing due to the Windows service nature of the application.

### Test 1: Console Mode Test

```cmd
uptime_tracker.exe console
```

Expected behavior:
- Program starts without errors
- Prints "Starting Internet Uptime Tracker in console mode..."
- Creates `logs` directory
- Creates daily log file
- Writes entries every 60 seconds
- Responds to Ctrl+C

### Test 2: Log File Validation

```cmd
python utils/test_service.py --test format
```

Verifies:
- Log file exists
- CSV header is correct
- Entries have 4 columns
- Timestamps are valid

### Test 3: Service Installation Test

**Run as Administrator:**

```cmd
uptime_tracker.exe install
sc query InternetUptimeTracker
```

Expected output:
- "Service installed successfully"
- Service appears in Windows Services

### Test 4: Service Functionality Test

**Run as Administrator:**

```cmd
sc start InternetUptimeTracker
python utils/test_service.py --monitor 120
sc stop InternetUptimeTracker
```

Verifies:
- Service starts successfully
- Logs are created
- Entries are written every 60 seconds
- Service can be stopped cleanly

### Test 5: Python Utilities Test

```cmd
python utils/query_logs.py --log-file examples/sample_log.csv
```

Expected:
- Parses CSV correctly
- Calculates statistics accurately
- Displays formatted output

### Test 6: Outage Detection Test

1. Start the tracker in console mode
2. Disconnect your network cable or disable WiFi
3. Wait 2-3 minutes
4. Reconnect
5. Check the logs

Expected log entries:
```csv
2024-01-19 10:00:00,UP,25,0
2024-01-19 10:01:00,DOWN,-1,0
2024-01-19 10:02:00,DOWN,-1,0
2024-01-19 10:03:00,UP,30,120
```

The last entry should show the outage duration (120 seconds).

## Continuous Testing

### Automated Test Script

Run all tests at once:

```cmd
python utils/test_service.py --test all
```

### Integration Testing

1. Install the service
2. Let it run for 24 hours
3. Manually disconnect/reconnect a few times
4. Verify daily summary is generated
5. Check statistics with Python utility

## Performance Testing

### CPU Usage Test

```cmd
# Start service
sc start InternetUptimeTracker

# Monitor CPU usage
tasklist /FI "IMAGENAME eq uptime_tracker.exe"

# Expected: <1% CPU during normal operation
```

### Memory Test

```cmd
# Check memory usage
tasklist /FI "IMAGENAME eq uptime_tracker.exe" /FO TABLE /NH

# Expected: 2-3 MB
```

### Log Size Test

After 24 hours of operation:
- Log file size should be ~50-100 KB
- 1440 entries (one per minute)
- Each entry ~60-70 bytes

## Development Workflow

### Making Changes

1. Edit source files in `src/` or `include/`
2. Clean previous build:
   ```cmd
   make clean
   ```
3. Rebuild:
   ```cmd
   make
   ```
4. Test in console mode:
   ```cmd
   make console
   ```
5. If successful, reinstall service:
   ```cmd
   sc stop InternetUptimeTracker
   uptime_tracker.exe uninstall
   uptime_tracker.exe install
   sc start InternetUptimeTracker
   ```

### Code Style

- Follow C99 standard
- Use 4-space indentation
- Include error checking for all system calls
- Comment complex logic
- Keep functions focused and modular

### Adding Features

Example: Adding email notifications

1. Create `notification.c` and `notification.h`
2. Add to Makefile sources
3. Implement email sending logic
4. Call from `service.c` when outage detected
5. Rebuild and test

## Common Build Issues

### Issue: "Cannot find iphlpapi.h"

Solution: Install Windows SDK or ensure MinGW includes Windows headers

### Issue: "Undefined reference to IcmpCreateFile"

Solution: Add `-liphlpapi` to linker flags

### Issue: "Permission denied" when installing

Solution: Run Command Prompt as Administrator

### Issue: Make fails with "command not found"

Solution: Add MinGW\bin to PATH environment variable

## Debugging

### Using GDB

```cmd
# Build with debug symbols
gcc -g -O0 -Iinclude src/*.c -o uptime_tracker.exe -liphlpapi -lws2_32

# Run in debugger
gdb uptime_tracker.exe

# GDB commands
(gdb) run console
(gdb) break service.c:100
(gdb) continue
(gdb) print variable_name
```

### Logging Debug Information

Add debug prints in critical sections:

```c
#ifdef DEBUG
fprintf(stderr, "Debug: Connection check returned %d\n", result.is_connected);
#endif
```

Build with `-DDEBUG` flag to enable.

## Contributing

When contributing code:

1. Follow existing code style
2. Add comments for new functions
3. Test thoroughly in console mode
4. Verify service installation works
5. Update documentation as needed
6. Submit pull request with clear description

## Verification Checklist

Before considering the build complete:

- [ ] Code compiles without warnings
- [ ] Program runs in console mode
- [ ] Logs are created correctly
- [ ] Service installs successfully
- [ ] Service starts and stops cleanly
- [ ] Outage detection works
- [ ] Python utilities work with logs
- [ ] CPU usage is acceptable (<5%)
- [ ] Memory usage is acceptable (<5 MB)
- [ ] Documentation is updated

## Release Process

1. Clean build:
   ```cmd
   make clean
   make
   ```

2. Test all functionality

3. Create release package:
   ```
   Paint-v1.0/
   ├── uptime_tracker.exe
   ├── README.md
   ├── QUICKSTART.md
   ├── utils/
   │   ├── query_logs.py
   │   └── test_service.py
   └── examples/
       ├── sample_log.csv
       └── sample_summary.txt
   ```

4. Tag release in Git:
   ```cmd
   git tag -a v1.0 -m "Release version 1.0"
   git push origin v1.0
   ```

5. Create release notes documenting:
   - New features
   - Bug fixes
   - Known issues
   - Installation instructions
