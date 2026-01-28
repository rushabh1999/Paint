# Installation Guide

## Prerequisites

Before installing the Configuration Drift Detector, ensure you have:

- Linux operating system (kernel 2.6.13 or later for inotify)
- GCC compiler (version 4.8 or later)
- Python 3.6 or later
- Make build tool
- Internet access (for downloading dependencies)

## Step-by-Step Installation

### 1. Clone the Repository

```bash
git clone https://github.com/rushabh1999/Paint.git
cd Paint
```

### 2. Build the Backend Service

```bash
cd backend
make
```

This will:
- Compile all C source files
- Link the drift_detector executable
- The binary will be created at `backend/drift_detector`

If you encounter build errors, ensure you have GCC installed:
```bash
gcc --version
```

### 3. Optional: System-wide Installation

To install the backend service system-wide:

```bash
cd backend
sudo make install
```

This will copy the `drift_detector` binary to `/usr/local/bin/`.

To uninstall:
```bash
sudo make uninstall
```

### 4. Make CLI Executable

The CLI tool is already executable, but verify:

```bash
chmod +x cli/drift_cli.py
```

### 5. Verify Installation

Test the backend builds correctly:
```bash
./backend/drift_detector --help
```

Test the CLI works:
```bash
./cli/drift_cli.py --help
```

## Post-Installation Setup

### 1. Create Configuration Directory

The system will automatically create the configuration directory on first use, but you can create it manually:

```bash
mkdir -p ~/.config/drift_detector/baselines
```

### 2. Start the Backend Service

For testing (foreground mode):
```bash
./backend/drift_detector
```

For production (daemon mode):
```bash
./backend/drift_detector --daemon
```

### 3. Configure Email Alerts (Optional)

Set up SMTP settings for email notifications:

```bash
./cli/drift_cli.py configure
```

Follow the prompts to enter:
- SMTP server address
- SMTP port
- Username
- Password
- Alert email address

## Systemd Integration (Optional)

To run the drift detector as a systemd service:

### 1. Create a Service File

Create `/etc/systemd/system/drift-detector.service`:

```ini
[Unit]
Description=Configuration Drift Detector Service
After=network.target

[Service]
Type=forking
ExecStart=/usr/local/bin/drift_detector --daemon
Restart=on-failure
User=root
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
```

### 2. Enable and Start the Service

```bash
sudo systemctl daemon-reload
sudo systemctl enable drift-detector
sudo systemctl start drift-detector
```

### 3. Check Service Status

```bash
sudo systemctl status drift-detector
```

## Verification

Test the complete workflow:

```bash
# Create a test file
echo "test=value" > /tmp/test.conf

# Add to monitoring
./cli/drift_cli.py add /tmp/test.conf

# Create baseline
./cli/drift_cli.py baseline /tmp/test.conf

# Check status
./cli/drift_cli.py status

# Modify the file
echo "test=newvalue" >> /tmp/test.conf

# Wait a moment for detection (check backend logs)
sleep 2

# Cleanup
./cli/drift_cli.py remove /tmp/test.conf
rm /tmp/test.conf
```

## Troubleshooting

### Build Fails

**Error: gcc: command not found**
```bash
# Ubuntu/Debian
sudo apt-get install build-essential

# RHEL/CentOS
sudo yum groupinstall "Development Tools"
```

**Error: make: command not found**
```bash
# Ubuntu/Debian
sudo apt-get install make

# RHEL/CentOS
sudo yum install make
```

### Backend Won't Start

**Error: Cannot bind socket**

The socket file may already exist:
```bash
rm /tmp/drift_detector.sock
```

**Error: inotify_init failed**

Check inotify limits:
```bash
cat /proc/sys/fs/inotify/max_user_watches
```

Increase if needed:
```bash
sudo sysctl fs.inotify.max_user_watches=524288
```

To make permanent, add to `/etc/sysctl.conf`:
```
fs.inotify.max_user_watches=524288
```

### Python CLI Issues

**Error: python3: command not found**

Install Python 3:
```bash
# Ubuntu/Debian
sudo apt-get install python3

# RHEL/CentOS
sudo yum install python3
```

**Error: Permission denied**

Make the CLI executable:
```bash
chmod +x cli/drift_cli.py
```

## Next Steps

After installation:

1. Review the [README.md](README.md) for usage examples
2. Check the [Configuration Guide](docs/CONFIGURATION.md) for advanced settings
3. Run the demo script: `./demo.sh`
4. Start monitoring your critical configuration files!

## Uninstallation

To completely remove the drift detector:

```bash
# Stop the backend
pkill drift_detector

# Remove system-wide installation (if installed)
sudo make uninstall

# Remove configuration
rm -rf ~/.config/drift_detector

# Remove socket file
rm -f /tmp/drift_detector.sock

# Remove systemd service (if configured)
sudo systemctl stop drift-detector
sudo systemctl disable drift-detector
sudo rm /etc/systemd/system/drift-detector.service
sudo systemctl daemon-reload
```
