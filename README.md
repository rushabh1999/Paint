# Internet Uptime Tracker

A lightweight Windows service that monitors internet connectivity, tracks outages, and provides uptime/downtime statistics.

## Overview

The Internet Uptime Tracker is a C-based Windows service designed to:
- Periodically check internet connectivity (default: every 60 seconds)
- Record connection status, timestamps, and ping latency
- Detect and track outage events with start/end times
- Generate daily statistics including uptime percentage, downtime duration, and outage frequency
- Store logs in CSV format for easy analysis

## Features

- **Periodic Connectivity Checks**: Pings Google's DNS (8.8.8.8) every 60 seconds
- **Outage Detection**: Automatically detects when connection goes down and comes back up
- **Detailed Logging**: Records all checks with timestamps, status, latency, and outage durations
- **Daily Statistics**: 
  - Total uptime/downtime percentages
  - Number of outages
  - Average, longest, and shortest outage durations
- **Windows Service**: Runs reliably in the background
- **Lightweight**: Minimal CPU usage (<5% during idle)
- **Self-Contained**: No external dependencies beyond system libraries
- **Python Utilities**: Query and analyze logs easily

## Requirements

### For Building
- Windows OS (7 or later)
- GCC compiler (MinGW-w64 recommended)
- Make utility

### For Running
- Windows OS (7 or later)
- Administrator privileges (for service installation)

### For Python Utilities
- Python 3.6 or later

## Installation

### 1. Build the Service

Open a command prompt and navigate to the project directory:

```cmd
cd /path/to/Paint
make
```

This will compile the service and create `uptime_tracker.exe`.

### 2. Install as Windows Service

**Important**: You must run this command as Administrator.

```cmd
uptime_tracker.exe install
```

Or use the Makefile:

```cmd
make install
```

### 3. Start the Service

```cmd
sc start InternetUptimeTracker
```

The service will now run in the background and begin monitoring your internet connection.

## Usage

### Running the Service

The service runs automatically after installation. You can control it using Windows service commands:

```cmd
# Start the service
sc start InternetUptimeTracker

# Stop the service
sc stop InternetUptimeTracker

# Check service status
sc query InternetUptimeTracker
```

### Running in Console Mode (Testing)

For testing purposes, you can run the tracker in console mode without installing it as a service:

```cmd
uptime_tracker.exe console
```

Or:

```cmd
make console
```

Press Ctrl+C to stop.

### Viewing Logs

Logs are stored in the `logs` directory (created in the same directory as the executable):

```
logs/
├── uptime_log_2024-01-19.csv    # Daily log file
├── uptime_log_2024-01-20.csv
└── summary_2024-01-19.txt       # Daily summary (generated at midnight)
```

### Using Python Utilities

#### Query and Analyze Logs

```bash
# Analyze the most recent log file
python utils/query_logs.py

# Analyze a specific date
python utils/query_logs.py --date 2024-01-19

# Analyze a specific log file
python utils/query_logs.py --log-file logs/uptime_log_2024-01-19.csv

# List all available log files
python utils/query_logs.py --list

# Specify custom log directory
python utils/query_logs.py --log-dir C:\custom\path\logs
```

#### Test the Service

```bash
# Run all tests
python utils/test_service.py

# Run specific tests
python utils/test_service.py --test service   # Check if service is running
python utils/test_service.py --test logs      # Check if logs are being created
python utils/test_service.py --test format    # Verify log format

# Monitor logs in real-time for 60 seconds
python utils/test_service.py --monitor 60
```

## Log Format

Logs are stored in CSV format with the following columns:

```csv
Timestamp,Status,Latency_ms,Outage_Duration_sec
2024-01-19 10:30:00,UP,25,0
2024-01-19 10:31:00,UP,28,0
2024-01-19 10:32:00,DOWN,-1,0
2024-01-19 10:33:00,DOWN,-1,0
2024-01-19 10:34:00,UP,30,120
```

- **Timestamp**: Date and time of the check
- **Status**: Connection status (UP or DOWN)
- **Latency_ms**: Ping latency in milliseconds (-1 if connection is down)
- **Outage_Duration_sec**: Duration of the outage in seconds (recorded when connection comes back up)

## Statistics

The service generates daily summaries automatically at midnight. You can also query statistics at any time using the Python utility:

```
Internet Uptime Statistics
============================================================

Period: 2024-01-19 00:00:00 to 2024-01-19 23:59:00
Total Duration: 24.00 hours

--- Connection Status ---
Total Checks: 1440
Successful Checks: 1425
Failed Checks: 15
Uptime Percentage: 98.96%
Downtime Percentage: 1.04%

--- Outage Statistics ---
Number of Outages: 3
Total Downtime: 15.00 minutes
Average Outage Duration: 5.00 minutes
Longest Outage: 10.00 minutes
Shortest Outage: 2.00 minutes

--- Latency Statistics ---
Average Latency: 27.50 ms
Minimum Latency: 15 ms
Maximum Latency: 85 ms
```

## Configuration

The check interval and ping target are defined in `src/service.c`:

```c
#define CHECK_INTERVAL_MS 60000  // 60 seconds
#define PING_HOST "8.8.8.8"      // Google DNS
```

To change these values, modify the source code and rebuild:

```cmd
make clean
make
```

Then reinstall the service:

```cmd
uptime_tracker.exe uninstall
uptime_tracker.exe install
sc start InternetUptimeTracker
```

## Uninstallation

1. Stop the service:
   ```cmd
   sc stop InternetUptimeTracker
   ```

2. Uninstall the service:
   ```cmd
   uptime_tracker.exe uninstall
   ```
   
   Or:
   ```cmd
   make uninstall
   ```

3. Delete the program files and logs as needed.

## Troubleshooting

### Service won't install
- Make sure you're running the command prompt as Administrator
- Check that no other service with the same name exists
- Verify that the executable path is correct

### Service won't start
- Check Windows Event Viewer for error messages
- Ensure the logs directory can be created/written to
- Try running in console mode first to diagnose issues: `uptime_tracker.exe console`

### No logs are being created
- Verify the service is running: `sc query InternetUptimeTracker`
- Check that the service has permission to write to the logs directory
- Run in console mode to see error messages

### High CPU usage
- The service should use <1% CPU during normal operation
- If CPU usage is high, check for network issues causing timeouts
- Verify the check interval is appropriate (default 60 seconds)

### False outage detections
- Check if your network has intermittent issues
- Consider increasing the timeout in `src/ping_checker.c` (TIMEOUT_MS)
- Verify that 8.8.8.8 is not blocked by your firewall

## Architecture

The tracker consists of several modular components:

- **ping_checker.c**: Uses Windows ICMP API to ping remote hosts
- **state_machine.c**: Tracks connection state transitions and detects outages
- **logger.c**: Writes entries to CSV files with daily rotation
- **statistics.c**: Calculates uptime/downtime statistics from logs
- **service.c**: Windows service infrastructure and main monitoring loop
- **main.c**: Entry point with install/uninstall/console options

## Development

### Building from Source

```cmd
git clone https://github.com/rushabh1999/Paint.git
cd Paint
make
```

### Project Structure

```
Paint/
├── src/                    # Source files
│   ├── main.c
│   ├── service.c
│   ├── ping_checker.c
│   ├── state_machine.c
│   ├── logger.c
│   └── statistics.c
├── include/                # Header files
│   ├── service.h
│   ├── ping_checker.h
│   ├── state_machine.h
│   ├── logger.h
│   └── statistics.h
├── utils/                  # Python utilities
│   ├── query_logs.py
│   └── test_service.py
├── logs/                   # Log files (created at runtime)
├── Makefile
└── README.md
```

### Testing

Run the service in console mode for interactive testing:

```cmd
make console
```

Use the Python test utility:

```bash
python utils/test_service.py --monitor 60
```

## Performance

- CPU Usage: <1% during normal operation, <5% during checks
- Memory: ~2-3 MB
- Disk Space: ~1 KB per day of logs (at 60-second intervals)
- Network: Minimal (32-byte ICMP packets every 60 seconds)

## License

This project is open source and available for use and modification.

## Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.

## Author

Built for tracking internet uptime on Windows systems.