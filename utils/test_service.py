#!/usr/bin/env python3
"""
Test script for the Internet Uptime Tracker service.

This script can be used to test the service by simulating network
conditions and verifying that the service handles them correctly.
"""

import sys
import time
import argparse
from pathlib import Path
import subprocess
import os


def test_log_creation(log_dir):
    """Test that log files are being created."""
    log_path = Path(log_dir)
    if not log_path.exists():
        print(f"❌ Log directory does not exist: {log_dir}")
        return False
    
    log_files = list(log_path.glob('uptime_log_*.csv'))
    if not log_files:
        print(f"❌ No log files found in {log_dir}")
        return False
    
    print(f"✓ Found {len(log_files)} log file(s) in {log_dir}")
    for log_file in log_files:
        print(f"  - {log_file.name}")
    
    return True


def test_log_format(log_dir):
    """Test that log files have the correct format."""
    log_path = Path(log_dir)
    log_files = list(log_path.glob('uptime_log_*.csv'))
    
    if not log_files:
        print("❌ No log files to test")
        return False
    
    # Check the most recent log file
    log_files.sort(key=lambda x: x.stat().st_mtime, reverse=True)
    log_file = log_files[0]
    
    try:
        with open(log_file, 'r') as f:
            header = f.readline().strip()
            expected_header = "Timestamp,Status,Latency_ms,Outage_Duration_sec"
            
            if header != expected_header:
                print(f"❌ Invalid log header: {header}")
                print(f"   Expected: {expected_header}")
                return False
            
            print(f"✓ Log file has correct header format")
            
            # Read a few entries
            line_count = 0
            for line in f:
                line_count += 1
                if line_count > 5:
                    break
                parts = line.strip().split(',')
                if len(parts) != 4:
                    print(f"❌ Invalid log entry: {line.strip()}")
                    return False
            
            if line_count > 0:
                print(f"✓ Log entries have correct format ({line_count} entries checked)")
            else:
                print("⚠ No log entries found yet (service may have just started)")
            
            return True
    
    except Exception as e:
        print(f"❌ Error reading log file: {e}")
        return False


def test_service_running():
    """Test if the service is running (Windows only)."""
    try:
        result = subprocess.run(
            ['sc', 'query', 'InternetUptimeTracker'],
            capture_output=True,
            text=True
        )
        
        if result.returncode == 0 and 'RUNNING' in result.stdout:
            print("✓ Service is running")
            return True
        elif result.returncode == 0:
            print("⚠ Service exists but is not running")
            print("  Use 'sc start InternetUptimeTracker' to start it")
            return False
        else:
            print("⚠ Service is not installed")
            print("  Use 'uptime_tracker.exe install' to install it")
            return False
    
    except FileNotFoundError:
        print("⚠ Cannot check service status (sc command not found)")
        print("  This is expected on non-Windows systems")
        return False
    except Exception as e:
        print(f"❌ Error checking service status: {e}")
        return False


def monitor_logs(log_dir, duration=60):
    """Monitor log files for a specified duration."""
    log_path = Path(log_dir)
    print(f"\nMonitoring logs in {log_dir} for {duration} seconds...")
    print("Press Ctrl+C to stop early\n")
    
    # Find the current log file
    log_files = list(log_path.glob('uptime_log_*.csv'))
    if not log_files:
        print("❌ No log files found")
        return False
    
    log_files.sort(key=lambda x: x.stat().st_mtime, reverse=True)
    log_file = log_files[0]
    
    print(f"Monitoring: {log_file.name}\n")
    
    try:
        # Get current file size
        last_size = log_file.stat().st_size
        start_time = time.time()
        check_count = 0
        
        while time.time() - start_time < duration:
            time.sleep(5)
            
            current_size = log_file.stat().st_size
            if current_size > last_size:
                # New data written
                with open(log_file, 'r') as f:
                    f.seek(last_size)
                    new_lines = f.read()
                    
                print(f"[{time.strftime('%H:%M:%S')}] New entries:")
                for line in new_lines.strip().split('\n'):
                    if line:
                        print(f"  {line}")
                
                last_size = current_size
                check_count += 1
        
        elapsed = time.time() - start_time
        print(f"\nMonitoring complete. {check_count} updates detected in {elapsed:.1f} seconds")
        
        if check_count == 0:
            print("⚠ No new log entries detected. Service may not be running.")
            return False
        
        return True
    
    except KeyboardInterrupt:
        print("\n\nMonitoring stopped by user")
        return True
    except Exception as e:
        print(f"❌ Error monitoring logs: {e}")
        return False


def run_all_tests(log_dir):
    """Run all tests."""
    print("\n" + "="*60)
    print("Internet Uptime Tracker - Test Suite")
    print("="*60 + "\n")
    
    tests = [
        ("Service Status", lambda: test_service_running()),
        ("Log File Creation", lambda: test_log_creation(log_dir)),
        ("Log File Format", lambda: test_log_format(log_dir)),
    ]
    
    results = []
    for test_name, test_func in tests:
        print(f"\nTest: {test_name}")
        print("-" * 40)
        result = test_func()
        results.append((test_name, result))
        print()
    
    print("\n" + "="*60)
    print("Test Summary")
    print("="*60)
    
    passed = sum(1 for _, result in results if result)
    total = len(results)
    
    for test_name, result in results:
        status = "✓ PASS" if result else "✗ FAIL"
        print(f"{status}: {test_name}")
    
    print(f"\nTotal: {passed}/{total} tests passed")
    print("="*60 + "\n")
    
    return passed == total


def main():
    parser = argparse.ArgumentParser(
        description='Test the Internet Uptime Tracker service'
    )
    parser.add_argument(
        '--log-dir',
        default='logs',
        help='Directory containing log files (default: logs)'
    )
    parser.add_argument(
        '--monitor',
        type=int,
        metavar='SECONDS',
        help='Monitor logs for specified number of seconds'
    )
    parser.add_argument(
        '--test',
        choices=['service', 'logs', 'format', 'all'],
        default='all',
        help='Which test to run (default: all)'
    )
    
    args = parser.parse_args()
    
    if args.monitor:
        return 0 if monitor_logs(args.log_dir, args.monitor) else 1
    
    if args.test == 'all':
        return 0 if run_all_tests(args.log_dir) else 1
    elif args.test == 'service':
        return 0 if test_service_running() else 1
    elif args.test == 'logs':
        return 0 if test_log_creation(args.log_dir) else 1
    elif args.test == 'format':
        return 0 if test_log_format(args.log_dir) else 1


if __name__ == '__main__':
    sys.exit(main())
