# Quick Start Guide - Internet Uptime Tracker

This guide will help you get the Internet Uptime Tracker up and running quickly.

## Prerequisites

- Windows 7 or later
- MinGW-w64 (for compilation)
- Python 3.6+ (for utility scripts)
- Administrator privileges (for service installation)

## Step 1: Build the Service

Open a Command Prompt and navigate to the project directory:

```cmd
cd C:\path\to\Paint
make
```

This creates `uptime_tracker.exe` in the current directory.

## Step 2: Test in Console Mode

Before installing as a service, test the tracker in console mode:

```cmd
uptime_tracker.exe console
```

You should see output like:
```
Starting Internet Uptime Tracker in console mode...
Press Ctrl+C to stop
```

Let it run for a minute or two, then press Ctrl+C to stop.

## Step 3: Check the Logs

Look in the `logs` directory that was created:

```cmd
dir logs
```

You should see a file like `uptime_log_2024-01-19.csv`.

View the log:
```cmd
type logs\uptime_log_2024-01-19.csv
```

## Step 4: Analyze the Logs

Use the Python utility to get statistics:

```cmd
python utils\query_logs.py
```

## Step 5: Install as Service (Optional)

To run the tracker continuously in the background:

**Important: Open Command Prompt as Administrator**

```cmd
uptime_tracker.exe install
sc start InternetUptimeTracker
```

Verify it's running:
```cmd
sc query InternetUptimeTracker
```

## Step 6: Monitor the Service

You can monitor the logs in real-time:

```cmd
python utils\test_service.py --monitor 60
```

Or check statistics periodically:

```cmd
python utils\query_logs.py
```

## Troubleshooting

### Build Errors

If `make` fails, ensure MinGW-w64 is installed and in your PATH:
```cmd
gcc --version
```

### Service Won't Start

1. Make sure you ran `install` as Administrator
2. Check Event Viewer for errors
3. Try console mode first to diagnose issues

### No Logs Created

1. Check that the service is running
2. Verify write permissions for the logs directory
3. Look for errors in console mode

## Next Steps

- Read the full [README.md](../README.md) for detailed documentation
- Customize check interval and ping target in `src/service.c`
- Set up automatic log analysis with scheduled tasks
- Review troubleshooting section for common issues

## Example Output

After running for a day, you should see statistics like:

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
```

## Uninstallation

To remove the service:

```cmd
sc stop InternetUptimeTracker
uptime_tracker.exe uninstall
```

Then delete the program files and logs as desired.
