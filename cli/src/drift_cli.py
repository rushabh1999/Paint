#!/usr/bin/env python3
"""
Configuration Drift Detector CLI
Hash-based integrity verification for monitored files
"""

import os
import sys
import hashlib
import shutil
import argparse
from pathlib import Path

CONFIG_DIR = Path.home() / ".config" / "drift_detector"
HASH_ALGORITHM = "sha256"


def init_config_dir():
    """Initialize the configuration directory"""
    CONFIG_DIR.mkdir(parents=True, exist_ok=True)


def get_baseline_path(filepath):
    """Get the baseline file path for a given file"""
    filename = Path(filepath).name
    return CONFIG_DIR / f"{filename}.baseline"


def get_hash_path(filepath):
    """Get the hash file path for a given file"""
    filename = Path(filepath).name
    return CONFIG_DIR / f"{filename}.hash"


def calculate_file_hash(filepath, algorithm="sha256"):
    """Calculate hash of a file using specified algorithm"""
    hash_obj = hashlib.new(algorithm)
    
    try:
        with open(filepath, "rb") as f:
            while chunk := f.read(8192):
                hash_obj.update(chunk)
        return hash_obj.hexdigest()
    except Exception as e:
        print(f"ERROR: Failed to calculate hash for {filepath}: {e}", file=sys.stderr)
        return None


def store_baseline(filepath):
    """Store baseline copy of a file and its hash"""
    init_config_dir()
    
    filepath = Path(filepath)
    if not filepath.exists():
        print(f"ERROR: File {filepath} does not exist", file=sys.stderr)
        return False
    
    # Copy file to baseline
    baseline_path = get_baseline_path(filepath)
    try:
        shutil.copy2(filepath, baseline_path)
    except Exception as e:
        print(f"ERROR: Failed to create baseline for {filepath}: {e}", file=sys.stderr)
        return False
    
    # Calculate and store hash
    file_hash = calculate_file_hash(filepath, HASH_ALGORITHM)
    if file_hash is None:
        return False
    
    hash_path = get_hash_path(filepath)
    try:
        with open(hash_path, "w") as f:
            f.write(f"{file_hash}\n")
    except Exception as e:
        print(f"ERROR: Failed to store hash for {filepath}: {e}", file=sys.stderr)
        return False
    
    print(f"SUCCESS: Baseline and hash stored for {filepath}")
    print(f"  Baseline: {baseline_path}")
    print(f"  Hash ({HASH_ALGORITHM.upper()}): {file_hash}")
    return True


def verify_hash(filepath):
    """Verify file hash against stored hash"""
    filepath = Path(filepath)
    if not filepath.exists():
        print(f"ERROR: File {filepath} does not exist", file=sys.stderr)
        return False
    
    # Calculate current hash
    current_hash = calculate_file_hash(filepath, HASH_ALGORITHM)
    if current_hash is None:
        return False
    
    # Read stored hash
    hash_path = get_hash_path(filepath)
    if not hash_path.exists():
        print(f"ERROR: No stored hash found for {filepath}. Run baseline command first.", file=sys.stderr)
        return False
    
    try:
        with open(hash_path, "r") as f:
            stored_hash = f.read().strip()
    except Exception as e:
        print(f"ERROR: Failed to read stored hash for {filepath}: {e}", file=sys.stderr)
        return False
    
    # Compare hashes
    if current_hash == stored_hash:
        print(f"OK: Hash verification passed for {filepath}")
        print(f"  Hash ({HASH_ALGORITHM.upper()}): {current_hash}")
        return True
    else:
        print(f"ALERT: Hash mismatch detected for {filepath} - Potential integrity breach!")
        print(f"  Stored Hash:  {stored_hash}")
        print(f"  Current Hash: {current_hash}")
        return False


def check_drift(filepath):
    """Check for drift by comparing file with baseline and verifying hash"""
    filepath = Path(filepath)
    if not filepath.exists():
        print(f"ERROR: File {filepath} does not exist", file=sys.stderr)
        return False
    
    baseline_path = get_baseline_path(filepath)
    if not baseline_path.exists():
        print(f"ERROR: No baseline found for {filepath}. Run baseline command first.", file=sys.stderr)
        return False
    
    # First verify hash
    print("Verifying hash integrity...")
    hash_valid = verify_hash(filepath)
    
    if not hash_valid:
        return False
    
    # Compare with baseline
    print("\nChecking for drift from baseline...")
    try:
        with open(filepath, "rb") as current_file, open(baseline_path, "rb") as baseline_file:
            current_content = current_file.read()
            baseline_content = baseline_file.read()
            
            if current_content == baseline_content:
                print(f"OK: No drift detected for {filepath} (hash verified)")
                return True
            else:
                print(f"DRIFT: File {filepath} has drifted from baseline")
                return False
    except Exception as e:
        print(f"ERROR: Failed to compare files: {e}", file=sys.stderr)
        return False


def list_monitored_files():
    """List all monitored files (files with baselines)"""
    if not CONFIG_DIR.exists():
        print("No monitored files found.")
        return
    
    baseline_files = list(CONFIG_DIR.glob("*.baseline"))
    if not baseline_files:
        print("No monitored files found.")
        return
    
    print(f"Monitored files ({len(baseline_files)}):")
    for baseline in baseline_files:
        filename = baseline.stem.replace(".baseline", "")
        hash_path = CONFIG_DIR / f"{filename}.hash"
        has_hash = "✓" if hash_path.exists() else "✗"
        print(f"  {has_hash} {filename}")


def main():
    """Main CLI entry point"""
    parser = argparse.ArgumentParser(
        description="Configuration Drift Detector - Hash-based integrity verification",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s baseline /etc/config.conf    # Store baseline and hash
  %(prog)s verify /etc/config.conf      # Verify hash integrity
  %(prog)s check /etc/config.conf       # Check for drift
  %(prog)s list                         # List monitored files
        """
    )
    
    parser.add_argument(
        "command",
        choices=["baseline", "verify", "check", "list"],
        help="Command to execute"
    )
    
    parser.add_argument(
        "file",
        nargs="?",
        help="File to monitor (not required for 'list' command)"
    )
    
    args = parser.parse_args()
    
    if args.command in ["baseline", "verify", "check"] and not args.file:
        parser.error(f"the following arguments are required for '{args.command}': file")
    
    if args.command == "baseline":
        success = store_baseline(args.file)
        sys.exit(0 if success else 1)
    elif args.command == "verify":
        success = verify_hash(args.file)
        sys.exit(0 if success else 1)
    elif args.command == "check":
        success = check_drift(args.file)
        sys.exit(0 if success else 1)
    elif args.command == "list":
        list_monitored_files()
        sys.exit(0)


if __name__ == "__main__":
    main()
