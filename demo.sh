#!/bin/bash
# Demo script for Configuration Drift Detector

echo "=== Configuration Drift Detector Demo ==="
echo ""

# Create a test configuration file
TEST_FILE="/tmp/test_config.conf"
echo "Creating test configuration file: $TEST_FILE"
echo "# Test Configuration" > $TEST_FILE
echo "setting1=value1" >> $TEST_FILE
echo "setting2=value2" >> $TEST_FILE
echo ""

# Start the backend in the background
echo "Starting drift detector backend..."
cd /home/runner/work/Paint/Paint
./backend/drift_detector &
BACKEND_PID=$!
echo "Backend started with PID: $BACKEND_PID"
sleep 2
echo ""

# Check status
echo "Checking status (should be empty):"
./cli/drift_cli.py status
echo ""

# Add the test file
echo "Adding test file to monitoring:"
./cli/drift_cli.py add $TEST_FILE
sleep 1
echo ""

# Check status again
echo "Checking status (should show the file):"
./cli/drift_cli.py status
echo ""

# Create baseline
echo "Creating baseline for test file:"
./cli/drift_cli.py baseline $TEST_FILE
sleep 1
echo ""

# Modify the file to trigger drift detection
echo "Modifying test file (adding a line)..."
sleep 2
echo "setting3=value3" >> $TEST_FILE
echo "File modified!"
sleep 3
echo ""

# Show status again
echo "Final status:"
./cli/drift_cli.py status
echo ""

# Remove the file from monitoring
echo "Removing test file from monitoring:"
./cli/drift_cli.py remove $TEST_FILE
echo ""

# Final status
echo "Final status (should be empty):"
./cli/drift_cli.py status
echo ""

# Cleanup
echo "Stopping backend..."
kill $BACKEND_PID
sleep 1
rm -f $TEST_FILE
rm -f /tmp/drift_detector.sock

echo ""
echo "=== Demo Complete ==="
