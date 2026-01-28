#!/bin/bash
# Demo script for Configuration Drift Detector

echo "======================================================================"
echo "Configuration Drift Detector - Demo"
echo "======================================================================"
echo ""

# Create test configuration files
echo "Creating test configuration files..."
mkdir -p /tmp/demo_configs
echo "server {
    listen 80;
    server_name example.com;
}" > /tmp/demo_configs/nginx.conf

echo "Port 22
PermitRootLogin no
PasswordAuthentication yes" > /tmp/demo_configs/sshd_config

echo "Done!"
echo ""

# Python CLI Demo
echo "======================================================================"
echo "PYTHON CLI DEMO"
echo "======================================================================"
echo ""

echo "1. Storing baselines for test files..."
python3 cli/src/drift_cli.py baseline /tmp/demo_configs/nginx.conf
echo ""
python3 cli/src/drift_cli.py baseline /tmp/demo_configs/sshd_config
echo ""

echo "2. Listing monitored files..."
python3 cli/src/drift_cli.py list
echo ""

echo "3. Verifying hashes (should all pass)..."
python3 cli/src/drift_cli.py verify /tmp/demo_configs/nginx.conf
echo ""
python3 cli/src/drift_cli.py verify /tmp/demo_configs/sshd_config
echo ""

echo "4. Checking for drift (should find none)..."
python3 cli/src/drift_cli.py check /tmp/demo_configs/nginx.conf
echo ""

echo "5. Modifying a file to simulate unauthorized change..."
echo "server {
    listen 80;
    server_name hacked.com;
}" > /tmp/demo_configs/nginx.conf
echo "Done!"
echo ""

echo "6. Verifying hash after modification (should detect breach)..."
python3 cli/src/drift_cli.py verify /tmp/demo_configs/nginx.conf
echo ""

echo "7. Checking for drift (should detect drift)..."
python3 cli/src/drift_cli.py check /tmp/demo_configs/nginx.conf
echo ""

# C Backend Demo
echo "======================================================================"
echo "C BACKEND DEMO"
echo "======================================================================"
echo ""

echo "1. Creating new test file for C backend..."
echo "database.host=localhost
database.port=3306
database.name=myapp" > /tmp/demo_configs/db.conf
echo ""

echo "2. Storing baseline using C backend..."
backend/bin/drift_detector baseline /tmp/demo_configs/db.conf
echo ""

echo "3. Verifying hash using C backend..."
backend/bin/drift_detector verify /tmp/demo_configs/db.conf
echo ""

echo "4. Checking for drift using C backend..."
backend/bin/drift_detector check /tmp/demo_configs/db.conf
echo ""

echo "5. Modifying file..."
echo "database.host=attacker.com
database.port=3306
database.name=myapp" > /tmp/demo_configs/db.conf
echo ""

echo "6. Detecting hash mismatch with C backend..."
backend/bin/drift_detector verify /tmp/demo_configs/db.conf
echo ""

echo "======================================================================"
echo "Demo completed!"
echo "======================================================================"
echo ""
echo "Check ~/.config/drift_detector/ to see stored baselines and hashes"
ls -la ~/.config/drift_detector/
