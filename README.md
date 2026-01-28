# Configuration Drift Detector

A robust configuration drift detection system with hash-based integrity verification. This tool monitors configuration files and detects unauthorized changes using SHA-256 cryptographic hashing.

## Features

- **Hash-based Integrity Verification**: Uses SHA-256 to detect file modifications
- **Baseline Management**: Store and compare against known-good file versions
- **Dual Implementation**: C backend for performance, Python CLI for ease of use
- **Automatic Alert Generation**: Clear alerts on hash mismatches indicating potential breaches
- **Simple CLI**: Easy-to-use command-line interface for all operations

## Architecture

The system consists of two components:

1. **C Backend** (`backend/`): High-performance drift detection using OpenSSL for cryptographic operations
2. **Python CLI** (`cli/`): User-friendly command-line interface using Python's hashlib

Both implementations store data in `~/.config/drift_detector/`:
- `.baseline` files: Baseline copies of monitored files
- `.hash` files: SHA-256 hashes of baseline files

## Installation

### Prerequisites

**For C Backend:**
- GCC compiler
- OpenSSL development libraries (`libssl-dev` on Debian/Ubuntu)

**For Python CLI:**
- Python 3.7 or higher

### Building the C Backend

```bash
cd backend
make
```

Optional: Install system-wide
```bash
sudo make install
```

### Installing the Python CLI

```bash
cd cli
pip install -e .
```

Or use directly without installation:
```bash
python3 cli/src/drift_cli.py <command> [file]
```

## Usage

### Python CLI

The Python CLI provides the following commands:

#### 1. Store Baseline

Create a baseline snapshot of a file and calculate its hash:

```bash
drift-cli baseline /etc/nginx/nginx.conf
```

Output:
```
SUCCESS: Baseline and hash stored for /etc/nginx/nginx.conf
  Baseline: /home/user/.config/drift_detector/nginx.conf.baseline
  Hash (SHA256): e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
```

#### 2. Verify Hash

Manually verify a file's hash against the stored hash:

```bash
drift-cli verify /etc/nginx/nginx.conf
```

Output (on success):
```
OK: Hash verification passed for /etc/nginx/nginx.conf
  Hash (SHA256): e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
```

Output (on hash mismatch):
```
ALERT: Hash mismatch detected for /etc/nginx/nginx.conf - Potential integrity breach!
  Stored Hash:  e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
  Current Hash: 5feceb66ffc86f38d952786c6d696c79c2dbc239dd4e91b46729d73a27fb57e9
```

#### 3. Check for Drift

Check if a file has drifted from its baseline (includes hash verification):

```bash
drift-cli check /etc/nginx/nginx.conf
```

Output (on no drift):
```
Verifying hash integrity...
OK: Hash verification passed for /etc/nginx/nginx.conf
  Hash (SHA256): e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855

Checking for drift from baseline...
OK: No drift detected for /etc/nginx/nginx.conf (hash verified)
```

Output (on drift detected):
```
Verifying hash integrity...
ALERT: Hash mismatch detected for /etc/nginx/nginx.conf - Potential integrity breach!
  Stored Hash:  e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
  Current Hash: 5feceb66ffc86f38d952786c6d696c79c2dbc239dd4e91b46729d73a27fb57e9
```

#### 4. List Monitored Files

List all files currently being monitored:

```bash
drift-cli list
```

Output:
```
Monitored files (3):
  ✓ nginx.conf
  ✓ sshd_config
  ✓ app.conf
```

### C Backend

The C backend provides similar functionality:

```bash
# Store baseline
./backend/bin/drift_detector baseline /etc/nginx/nginx.conf

# Verify hash
./backend/bin/drift_detector verify /etc/nginx/nginx.conf

# Check for drift
./backend/bin/drift_detector check /etc/nginx/nginx.conf
```

## Testing

### C Backend Tests

```bash
cd backend
make test
```

### Python CLI Tests

Create test files and verify functionality:

```bash
# Create a test file
echo "Test configuration" > /tmp/test.conf

# Store baseline
drift-cli baseline /tmp/test.conf

# Verify (should pass)
drift-cli verify /tmp/test.conf

# Modify file
echo "Modified configuration" > /tmp/test.conf

# Verify (should fail with hash mismatch alert)
drift-cli verify /tmp/test.conf

# Check for drift (should detect drift)
drift-cli check /tmp/test.conf
```

## Security Considerations

1. **Hash Algorithm**: Uses SHA-256, a cryptographically secure hash function
2. **File Permissions**: Configuration directory uses mode 0700 (owner-only access)
3. **Storage Location**: Files stored in user's home directory (`~/.config/drift_detector/`)
4. **Integrity Alerts**: Clear messaging when hash mismatches are detected

## Use Cases

1. **Configuration Monitoring**: Detect unauthorized changes to critical config files
2. **Compliance Auditing**: Verify system configurations haven't drifted from approved baselines
3. **Security Incident Response**: Quickly identify compromised configuration files
4. **Change Management**: Track and validate configuration changes

## Workflow Example

```bash
# Initial setup - establish baselines for important files
drift-cli baseline /etc/ssh/sshd_config
drift-cli baseline /etc/nginx/nginx.conf
drift-cli baseline /etc/mysql/my.cnf

# Regular monitoring (run via cron or systemd timer)
drift-cli check /etc/ssh/sshd_config
drift-cli check /etc/nginx/nginx.conf
drift-cli check /etc/mysql/my.cnf

# After authorized changes
drift-cli baseline /etc/nginx/nginx.conf  # Update baseline

# Manual verification
drift-cli verify /etc/ssh/sshd_config
```

## File Storage Structure

```
~/.config/drift_detector/
├── nginx.conf.baseline      # Baseline copy of nginx.conf
├── nginx.conf.hash          # SHA-256 hash of nginx.conf
├── sshd_config.baseline     # Baseline copy of sshd_config
├── sshd_config.hash         # SHA-256 hash of sshd_config
└── ...
```

## Troubleshooting

### "No stored hash found" Error

This means no baseline has been established for the file. Run:
```bash
drift-cli baseline <filepath>
```

### "Failed to calculate hash" Error

Check:
- File exists and is readable
- You have permission to read the file
- File is not corrupted

### Hash Mismatch Alerts

A hash mismatch indicates:
- The file has been modified since baseline was created
- Potential unauthorized access or corruption
- Time to investigate changes or update baseline if changes were authorized

## Contributing

Contributions are welcome! Please ensure:
- C code compiles without warnings
- Python code follows PEP 8 style guidelines
- All tests pass before submitting

## License

This project is provided as-is for educational and operational purposes.