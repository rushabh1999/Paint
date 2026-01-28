# Usage Examples

## Common Use Cases

### Monitoring System Configuration Files

Monitor critical system configuration files for unauthorized changes:

```bash
# SSH configuration
./cli/drift_cli.py add /etc/ssh/sshd_config
./cli/drift_cli.py baseline /etc/ssh/sshd_config

# Nginx configuration
./cli/drift_cli.py add /etc/nginx/nginx.conf
./cli/drift_cli.py baseline /etc/nginx/nginx.conf

# Apache configuration
./cli/drift_cli.py add /etc/apache2/apache2.conf
./cli/drift_cli.py baseline /etc/apache2/apache2.conf

# MySQL configuration
./cli/drift_cli.py add /etc/mysql/my.cnf
./cli/drift_cli.py baseline /etc/mysql/my.cnf
```

### Application Configuration Monitoring

Monitor application-specific configuration files:

```bash
# Application config
./cli/drift_cli.py add /opt/myapp/config.json
./cli/drift_cli.py baseline /opt/myapp/config.json

# Environment file
./cli/drift_cli.py add /opt/myapp/.env
./cli/drift_cli.py baseline /opt/myapp/.env
```

### Container Configuration

Monitor Docker or Kubernetes configurations:

```bash
# Docker daemon
./cli/drift_cli.py add /etc/docker/daemon.json
./cli/drift_cli.py baseline /etc/docker/daemon.json

# Kubernetes config
./cli/drift_cli.py add /etc/kubernetes/admin.conf
./cli/drift_cli.py baseline /etc/kubernetes/admin.conf
```

## Complete Workflow Example

### Step 1: Start the Backend

```bash
# Start in daemon mode for production
./backend/drift_detector --daemon

# OR start in foreground for debugging
./backend/drift_detector
```

### Step 2: Configure Email Alerts

```bash
./cli/drift_cli.py configure
```

Example input:
```
SMTP Server (e.g., smtp.gmail.com): smtp.gmail.com
SMTP Port (default 587): 587
SMTP Username/Email: myemail@gmail.com
SMTP Password: [your app password]
Alert Email Address: alerts@example.com
```

### Step 3: Add Files to Monitor

```bash
# Add multiple configuration files
./cli/drift_cli.py add /etc/nginx/nginx.conf
./cli/drift_cli.py add /etc/ssh/sshd_config
./cli/drift_cli.py add /etc/mysql/my.cnf
```

### Step 4: Create Baselines

```bash
# Create baseline for each file
./cli/drift_cli.py baseline /etc/nginx/nginx.conf
./cli/drift_cli.py baseline /etc/ssh/sshd_config
./cli/drift_cli.py baseline /etc/mysql/my.cnf
```

### Step 5: Check Status

```bash
./cli/drift_cli.py status
```

Output:
```
Monitored files:
  [1] /etc/nginx/nginx.conf (wd=1)
  [2] /etc/ssh/sshd_config (wd=2)
  [3] /etc/mysql/my.cnf (wd=3)
```

### Step 6: Simulate a Change

```bash
# Modify a configuration file
sudo nano /etc/nginx/nginx.conf
# Make some changes and save
```

The backend will:
1. Detect the change via inotify
2. Compare with the baseline
3. Generate a diff report
4. Send an email alert (if configured)

### Step 7: Update Baseline After Authorized Change

If the change was authorized:

```bash
./cli/drift_cli.py baseline /etc/nginx/nginx.conf
```

### Step 8: Remove File from Monitoring

When you no longer need to monitor a file:

```bash
./cli/drift_cli.py remove /etc/nginx/nginx.conf
```

## Advanced Usage

### Running Demo

Test the complete system with the included demo:

```bash
./demo.sh
```

This will:
- Create a test configuration file
- Start the backend
- Add the file to monitoring
- Create a baseline
- Modify the file (trigger drift)
- Show drift detection in action
- Clean up

### Monitoring Multiple Files in Batch

Create a script to add multiple files:

```bash
#!/bin/bash
FILES=(
    "/etc/nginx/nginx.conf"
    "/etc/ssh/sshd_config"
    "/etc/mysql/my.cnf"
    "/etc/apache2/apache2.conf"
)

for file in "${FILES[@]}"; do
    if [ -f "$file" ]; then
        ./cli/drift_cli.py add "$file"
        ./cli/drift_cli.py baseline "$file"
        echo "Added and baselined: $file"
    fi
done
```

### Viewing Backend Logs

When running in foreground mode, logs are displayed directly.

When running as a daemon, you can check system logs:

```bash
# If using systemd
sudo journalctl -u drift-detector -f

# Or check system messages
tail -f /var/log/syslog | grep drift_detector
```

### Manual Baseline Management

Baselines are stored in `~/.config/drift_detector/baselines/`.

View a baseline:
```bash
cat ~/.config/drift_detector/baselines/nginx.conf
```

Manually backup baselines:
```bash
tar czf baselines-backup.tar.gz ~/.config/drift_detector/baselines/
```

Restore baselines:
```bash
tar xzf baselines-backup.tar.gz -C ~/
```

### Scripting with the CLI

The CLI returns exit codes:
- 0: Success
- 1: Error

Use in scripts:

```bash
#!/bin/bash

if ./cli/drift_cli.py add /etc/nginx/nginx.conf; then
    echo "Successfully added file"
    ./cli/drift_cli.py baseline /etc/nginx/nginx.conf
else
    echo "Failed to add file"
    exit 1
fi
```

## Email Alert Examples

### Gmail Configuration

For Gmail with 2FA:

1. Enable 2-Factor Authentication
2. Generate App Password: https://myaccount.google.com/apppasswords
3. Use the app password in configuration

```
SMTP Server: smtp.gmail.com
Port: 587
Username: your-email@gmail.com
Password: [16-character app password]
Alert Email: alerts@yourdomain.com
```

### Outlook/Office 365

```
SMTP Server: smtp.office365.com
Port: 587
Username: your-email@outlook.com
Password: [your password]
Alert Email: alerts@yourdomain.com
```

### Custom SMTP Server

```
SMTP Server: mail.yourcompany.com
Port: 587
Username: monitoring@yourcompany.com
Password: [your password]
Alert Email: security@yourcompany.com
```

## Integration Examples

### Cron Job for Automatic Baseline Updates

Update baselines weekly (if changes are expected):

```bash
# Add to crontab: crontab -e
0 2 * * 0 /path/to/cli/drift_cli.py baseline /etc/nginx/nginx.conf
```

### Integration with Ansible

Use in Ansible playbooks:

```yaml
- name: Add file to drift monitoring
  command: /path/to/cli/drift_cli.py add {{ config_file }}

- name: Create baseline
  command: /path/to/cli/drift_cli.py baseline {{ config_file }}
```

### Integration with CI/CD

In your deployment pipeline:

```bash
# After deploying configuration changes
./cli/drift_cli.py baseline /etc/myapp/config.json
```

## Troubleshooting Common Scenarios

### Alerts Not Received

1. Check SMTP configuration:
   ```bash
   cat ~/.config/drift_detector/config.json
   ```

2. Test SMTP manually:
   ```bash
   python3 -c "import smtplib; s=smtplib.SMTP('smtp.gmail.com', 587); s.starttls(); s.login('user', 'pass'); print('OK')"
   ```

3. Check backend logs for errors

### File Changes Not Detected

1. Verify file is being monitored:
   ```bash
   ./cli/drift_cli.py status
   ```

2. Check if backend is running:
   ```bash
   ps aux | grep drift_detector
   ```

3. Ensure baseline exists:
   ```bash
   ls ~/.config/drift_detector/baselines/
   ```

### Too Many Alerts

If a file changes frequently:

1. Consider if it should be monitored
2. Remove from monitoring:
   ```bash
   ./cli/drift_cli.py remove /path/to/file
   ```

## Best Practices

1. **Create baselines immediately after configuration**: Don't wait - baseline right after adding a file
2. **Document authorized changes**: Keep a log of why baselines were updated
3. **Monitor critical files only**: Don't monitor files that change frequently
4. **Test email alerts**: Verify email delivery before relying on it
5. **Backup baselines**: Keep backups of your baseline directory
6. **Use version control**: Consider keeping configuration files in git
7. **Regular reviews**: Periodically review monitored files list
8. **Secure credentials**: Protect the configuration file with proper permissions

## Quick Reference

```bash
# Start backend
./backend/drift_detector --daemon

# Add file
./cli/drift_cli.py add /path/to/file

# Create baseline
./cli/drift_cli.py baseline /path/to/file

# View status
./cli/drift_cli.py status

# Configure email
./cli/drift_cli.py configure

# Remove file
./cli/drift_cli.py remove /path/to/file

# Stop backend
pkill drift_detector
```
