# Configuration Drift Detector

A robust configuration file monitoring system that detects and alerts on unauthorized changes to critical configuration files.

## Overview

The Configuration Drift Detector consists of two main components:

1. **Backend Service (C)**: A lightweight, efficient daemon that monitors configuration files using Linux's inotify system
2. **CLI Tool (Python)**: A user-friendly command-line interface for managing the monitoring service

## Features

- **Real-time Monitoring**: Uses inotify for efficient file change detection
- **Drift Detection**: Compares current files against stored baselines to detect changes
- **Email Alerts**: Sends detailed diff reports via SMTP when drift is detected
- **Unix Socket Communication**: Efficient IPC between CLI and backend
- **Daemonization**: Runs as a background service for continuous monitoring
- **JSON Configuration**: Easy-to-manage configuration file format

## Installation

### Prerequisites

- Linux operating system (for inotify support)
- GCC compiler
- Python 3.6 or later
- Make

### Building the Backend

```bash
cd backend
make
```

### Optional: System-wide Installation

```bash
cd backend
sudo make install
```

This installs the `drift_detector` binary to `/usr/local/bin/`.

## Usage

### Starting the Backend Service

Start in foreground mode (for testing):
```bash
./backend/drift_detector
```

Start as a daemon:
```bash
./backend/drift_detector --daemon
```

Or if installed system-wide:
```bash
drift_detector --daemon
```

### Using the CLI Tool

The CLI tool provides several commands to manage the drift detector:

#### Add a File to Monitoring

```bash
./cli/drift_cli.py add /path/to/config/file
```

Example:
```bash
./cli/drift_cli.py add /etc/nginx/nginx.conf
```

#### Remove a File from Monitoring

```bash
./cli/drift_cli.py remove /path/to/config/file
```

#### Create or Reset Baseline

Before drift detection works, you need to create a baseline:

```bash
./cli/drift_cli.py baseline /path/to/config/file
```

This captures the current state of the file as the "known good" configuration.

#### View Status

See all monitored files and their watch descriptors:

```bash
./cli/drift_cli.py status
```

#### Configure Email Alerts

Set up SMTP settings for email alerts:

```bash
./cli/drift_cli.py configure
```

You'll be prompted for:
- SMTP Server (e.g., smtp.gmail.com)
- SMTP Port (default: 587)
- SMTP Username/Email
- SMTP Password
- Alert Email Address

## Configuration

### Configuration File Location

The configuration is stored in JSON format at:
```
~/.config/drift_detector/config.json
```

### Baseline Storage

File baselines are stored in:
```
~/.config/drift_detector/baselines/
```

### Example Configuration File

```json
{
  "files": [
    "/etc/nginx/nginx.conf",
    "/etc/ssh/sshd_config"
  ],
  "smtp": {
    "server": "smtp.gmail.com",
    "port": 587,
    "user": "your-email@gmail.com",
    "password": "your-app-password",
    "alert_email": "alerts@example.com"
  }
}
```

## How It Works

1. **File Monitoring**: The backend uses inotify to watch for `IN_MODIFY` and `IN_CLOSE_WRITE` events on configured files
2. **Drift Detection**: When a file changes, the backend compares it against the stored baseline using `diff`
3. **Alerting**: If differences are detected, a diff report is generated and sent via email
4. **Management**: The CLI communicates with the backend via Unix sockets to add/remove files and configure settings

## Architecture

```
┌─────────────────┐         Unix Socket         ┌──────────────────┐
│   CLI Tool      │◄──────────────────────────►│  Backend Service │
│   (Python)      │                             │      (C)         │
└─────────────────┘                             └──────────────────┘
                                                         │
                                                         │ inotify
                                                         ▼
                                                 ┌──────────────┐
                                                 │ Config Files │
                                                 └──────────────┘
```

## Testing

### Running Python Tests

```bash
cd tests/cli
python3 test_drift_cli.py
```

Or using Python's unittest discovery:
```bash
python3 -m unittest discover -s tests/cli -p "test_*.py"
```

## Email Configuration Tips

### Gmail

If using Gmail, you need to:
1. Enable 2-factor authentication
2. Create an app-specific password
3. Use the app password in the configuration

### Other Providers

Most SMTP servers work with TLS on port 587. Check your email provider's documentation for specific settings.

## Security Considerations

- The configuration file contains sensitive information (SMTP password)
- Ensure `~/.config/drift_detector/config.json` has restricted permissions (600)
- Consider using environment variables for sensitive data
- Run the backend service with appropriate user privileges

## Troubleshooting

### Backend won't start

- Check if port/socket is already in use
- Verify inotify limits: `cat /proc/sys/fs/inotify/max_user_watches`
- Check logs for error messages

### Email alerts not working

- Verify SMTP settings are correct
- Check firewall rules for outbound SMTP traffic
- Test SMTP credentials separately
- Review backend logs for email sending errors

### CLI can't connect to backend

- Ensure backend service is running
- Check socket file exists: `/tmp/drift_detector.sock`
- Verify Unix socket permissions

## Limitations

- Maximum 100 files can be monitored (configurable in `drift_detector.h`)
- Email alerts require internet connectivity and valid SMTP credentials
- Only works on Linux systems (inotify dependency)

## Development

### Project Structure

```
.
├── backend/              # C backend service
│   ├── drift_detector.h  # Header file
│   ├── main.c           # Main entry point
│   ├── config.c         # Configuration management
│   ├── monitor.c        # File monitoring (inotify)
│   ├── drift.c          # Drift detection logic
│   ├── alert.c          # Email alerting
│   ├── socket_server.c  # Unix socket server
│   ├── daemon.c         # Daemonization
│   ├── cJSON.c/h        # JSON parser
│   └── Makefile         # Build configuration
├── cli/                 # Python CLI tool
│   └── drift_cli.py     # CLI implementation
├── tests/               # Test suites
│   ├── backend/         # C tests
│   └── cli/             # Python tests
└── README.md            # This file
```

### Building for Development

```bash
cd backend
make clean
make
```

### Adding New Features

1. Update header file (`drift_detector.h`) with new function declarations
2. Implement functionality in appropriate C file
3. Update CLI if needed
4. Add tests
5. Update documentation

## License

See repository license file.

## Contributing

Contributions are welcome! Please ensure:
- Code follows existing style
- Tests are included for new features
- Documentation is updated