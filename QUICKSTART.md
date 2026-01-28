# Configuration Drift Detector - Quick Reference

## Installation

### C Backend
```bash
cd backend
make
# Optional: sudo make install
```

### Python CLI
```bash
cd cli
pip install -e .
# Or use directly: python3 cli/src/drift_cli.py
```

## Commands

### Python CLI

**Create baseline:**
```bash
drift-cli baseline /path/to/file
# or: python3 cli/src/drift_cli.py baseline /path/to/file
```

**Verify hash:**
```bash
drift-cli verify /path/to/file
```

**Check for drift:**
```bash
drift-cli check /path/to/file
```

**List monitored files:**
```bash
drift-cli list
```

### C Backend

**Create baseline:**
```bash
./backend/bin/drift_detector baseline /path/to/file
```

**Verify hash:**
```bash
./backend/bin/drift_detector verify /path/to/file
```

**Check for drift:**
```bash
./backend/bin/drift_detector check /path/to/file
```

## Example Workflow

```bash
# 1. Store baseline for critical config files
drift-cli baseline /etc/ssh/sshd_config
drift-cli baseline /etc/nginx/nginx.conf

# 2. Verify integrity (returns 0 on success, 1 on mismatch)
drift-cli verify /etc/ssh/sshd_config

# 3. Check for drift (includes hash verification)
drift-cli check /etc/nginx/nginx.conf

# 4. After authorized changes, update baseline
drift-cli baseline /etc/nginx/nginx.conf

# 5. List all monitored files
drift-cli list
```

## Exit Codes

- **0**: Success / No drift / Hash match
- **1**: Hash mismatch / Drift detected
- **Non-zero**: Error

## Storage Location

Files are stored in `~/.config/drift_detector/`:
- `{filename}.{path_hash}.baseline` - Baseline file copy
- `{filename}.{path_hash}.hash` - SHA-256 hash

The `{path_hash}` ensures files with same name but different paths don't collide.

## Security Features

- **SHA-256 hashing** - Cryptographically secure
- **Path-based uniqueness** - Prevents file collisions
- **Clear alerts** - "ALERT: Potential integrity breach!"
- **Secure storage** - 0700 permissions on config directory

## Common Use Cases

1. **Monitor critical system configs**
2. **Detect unauthorized changes**
3. **Compliance auditing**
4. **Security incident response**
5. **Configuration change management**

## Testing

Run the demo:
```bash
./demo.sh
```

Run C backend tests:
```bash
cd backend && make test
```
