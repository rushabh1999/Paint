# Development Guide

## Project Architecture

### Component Overview

```
Configuration Drift Detector
├── Backend Service (C)
│   ├── File Monitoring (inotify)
│   ├── Drift Detection (diff)
│   ├── Email Alerting (SMTP)
│   ├── Socket Server (Unix sockets)
│   └── Configuration Management (JSON)
└── CLI Tool (Python)
    ├── Command Interface
    └── Socket Client
```

### Backend Components

**drift_detector.h**
- Main header file with all function declarations and data structures

**main.c**
- Entry point
- Main event loop with select() for multiplexing inotify and socket events
- Signal handlers for graceful shutdown

**config.c**
- JSON configuration file parsing and saving using cJSON library
- Manages ~/.config/drift_detector/config.json

**monitor.c**
- inotify initialization and watch management
- File change event processing
- Triggers drift detection on file changes

**drift.c**
- Baseline creation and management
- Drift detection using diff command
- Baseline storage in ~/.config/drift_detector/baselines/

**alert.c**
- Email alert generation
- SMTP communication via Python subprocess

**socket_server.c**
- Unix socket server for IPC
- Command parsing and handling (ADD, REMOVE, BASELINE, STATUS, CONFIGURE)

**daemon.c**
- Daemonization functionality
- Double-fork pattern for proper daemon creation

### CLI Tool Components

**drift_cli.py**
- Command-line interface implementation
- Unix socket client for backend communication
- Argument parsing with argparse
- Interactive configuration prompts

## Building from Source

### Backend

```bash
cd backend
make clean
make
```

Build options:
- Debug build: `make CFLAGS="-Wall -Wextra -g"`
- Release build: `make CFLAGS="-Wall -Wextra -O2"`

### Running Tests

**Backend Tests:**
```bash
cd tests/backend
make run
```

**Python Tests:**
```bash
python3 -m unittest discover -s tests/cli -p "test_*.py" -v
```

**Full Test Suite:**
```bash
# Run all tests
./run_tests.sh
```

## Code Style

### C Code Style

- Use 4 spaces for indentation
- Opening braces on same line for functions and control structures
- Descriptive variable names
- Comments for complex logic
- Check return values
- Avoid memory leaks

Example:
```c
int add_watch(int fd, const char *path) {
    int wd = inotify_add_watch(fd, path, IN_MODIFY | IN_CLOSE_WRITE);
    if (wd < 0) {
        fprintf(stderr, "Error adding watch for %s: %s\n", path, strerror(errno));
        return -1;
    }
    printf("Added watch for %s (wd=%d)\n", path, wd);
    return wd;
}
```

### Python Code Style

- Follow PEP 8
- Use 4 spaces for indentation
- Descriptive function and variable names
- Docstrings for functions and classes
- Type hints where appropriate

Example:
```python
def send_command(self, command: str) -> str:
    """Send command to backend service via Unix socket"""
    try:
        sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        sock.connect(self.socket_path)
        sock.sendall(command.encode())
        response = sock.recv(4096).decode()
        sock.close()
        return response
    except Exception as e:
        return f"ERROR: {str(e)}"
```

## Adding New Features

### Adding a New CLI Command

1. **Update drift_cli.py:**

```python
# Add new subparser
new_parser = subparsers.add_parser('newcommand', help='Description')
new_parser.add_argument('arg', help='Argument description')

# Add handler in main()
elif args.command == 'newcommand':
    return cli.new_command(args.arg)

# Implement method in DriftDetectorCLI class
def new_command(self, arg):
    """Implementation"""
    response = self.send_command(f"NEWCOMMAND {arg}")
    print(response.strip())
    return 0 if response.startswith("OK") else 1
```

2. **Update socket_server.c:**

```c
else if (strcmp(cmd, "NEWCOMMAND") == 0) {
    char *arg = strtok(NULL, "\n");
    if (!arg) {
        snprintf(response, sizeof(response), "ERROR: No argument specified\n");
    } else {
        // Implementation
        snprintf(response, sizeof(response), "OK: Command executed\n");
    }
}
```

3. **Add tests**

4. **Update documentation**

### Adding New Configuration Options

1. **Update drift_detector.h:**

```c
typedef struct {
    // ... existing fields ...
    char new_option[256];
} Config;
```

2. **Update config.c load_config():**

```c
item = cJSON_GetObjectItem(json, "new_option");
if (item && cJSON_IsString(item)) {
    strncpy(config->new_option, item->valuestring, 255);
}
```

3. **Update config.c save_config():**

```c
cJSON_AddStringToObject(json, "new_option", config->new_option);
```

## Debugging

### Backend Debugging

Run in foreground mode with verbose output:
```bash
./backend/drift_detector
```

Use GDB:
```bash
gdb ./backend/drift_detector
(gdb) run
(gdb) break monitor_files
(gdb) continue
```

Check inotify events manually:
```bash
inotifywait -m /path/to/file
```

### CLI Debugging

Add print statements:
```python
print(f"DEBUG: Sending command: {command}")
```

Use Python debugger:
```python
import pdb; pdb.set_trace()
```

### Socket Communication Debugging

Monitor Unix socket:
```bash
# In one terminal
./backend/drift_detector

# In another terminal
socat - UNIX-CONNECT:/tmp/drift_detector.sock
STATUS
```

## Performance Considerations

### Backend Optimization

1. **inotify Limits:**
   - Default: 8192 watches per user
   - Increase: `sudo sysctl fs.inotify.max_user_watches=524288`

2. **Memory Usage:**
   - Each MonitoredFile: ~4KB
   - Max 100 files: ~400KB
   - cJSON parsing: depends on config size

3. **CPU Usage:**
   - Idle: minimal (select() blocking)
   - Event processing: diff command overhead
   - Email sending: Python subprocess

### Optimization Tips

- Use non-blocking I/O for sockets
- Batch multiple file changes
- Limit diff output size
- Consider rate limiting for alerts

## Security Considerations

### Configuration File Security

The config file contains sensitive SMTP credentials:

```bash
chmod 600 ~/.config/drift_detector/config.json
```

### Running as Non-Root

If monitoring system files, you may need root access. Consider:

1. Run backend as root
2. Restrict CLI access to authorized users
3. Use sudo for sensitive operations

### Socket Security

The Unix socket at `/tmp/drift_detector.sock` should have restricted permissions:

```c
// In socket_server.c after bind()
chmod(SOCKET_PATH, 0600);
```

## Testing Strategy

### Unit Tests

- Test individual functions in isolation
- Mock external dependencies
- Cover edge cases

### Integration Tests

- Test complete workflows
- Use demo.sh as integration test
- Test error conditions

### Manual Testing Checklist

- [ ] Start backend successfully
- [ ] Add file to monitoring
- [ ] Create baseline
- [ ] Detect drift on file change
- [ ] Receive email alert
- [ ] Remove file from monitoring
- [ ] Configure SMTP settings
- [ ] View status
- [ ] Daemon mode works
- [ ] Graceful shutdown

## Contributing

### Workflow

1. Fork the repository
2. Create a feature branch
3. Make changes
4. Run tests
5. Update documentation
6. Submit pull request

### Pull Request Checklist

- [ ] Code follows style guidelines
- [ ] Tests added/updated
- [ ] Tests pass
- [ ] Documentation updated
- [ ] No compiler warnings
- [ ] No memory leaks (valgrind)

### Code Review Focus

- Correctness
- Security
- Performance
- Maintainability
- Documentation

## Common Development Tasks

### Adding a New File Type to Monitor

No special handling needed - any file can be monitored.

### Changing Alert Format

Edit `alert.c` in the `send_email_alert()` function.

### Adding Alternative Diff Tools

Edit `drift.c` to use different diff commands or libraries.

### Supporting Other Notification Channels

Add new functions in a new file (e.g., `slack_alert.c`, `webhook_alert.c`).

## Troubleshooting Development Issues

### Build Errors

**cJSON not found:**
```bash
cd backend
curl -O https://raw.githubusercontent.com/DaveGamble/cJSON/master/cJSON.c
curl -O https://raw.githubusercontent.com/DaveGamble/cJSON/master/cJSON.h
```

**Missing headers:**
Install development packages:
```bash
sudo apt-get install build-essential
```

### Runtime Issues

**Socket already in use:**
```bash
rm /tmp/drift_detector.sock
```

**Permission denied:**
```bash
sudo ./backend/drift_detector
```

**inotify limit reached:**
```bash
sudo sysctl -w fs.inotify.max_user_watches=524288
```

## Resources

- [inotify man page](https://man7.org/linux/man-pages/man7/inotify.7.html)
- [cJSON library](https://github.com/DaveGamble/cJSON)
- [Unix domain sockets](https://man7.org/linux/man-pages/man7/unix.7.html)
- [Python smtplib](https://docs.python.org/3/library/smtplib.html)

## Future Enhancements

Potential features to add:

- [ ] Web dashboard for monitoring
- [ ] Database storage for historical diffs
- [ ] Multiple notification channels (Slack, webhooks)
- [ ] Scheduled baseline updates
- [ ] File integrity checking (checksums)
- [ ] Support for directory monitoring
- [ ] Configuration templates
- [ ] Remote management via REST API
- [ ] Plugin system for custom detectors
- [ ] Windows support (using file system watchers)
