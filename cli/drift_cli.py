#!/usr/bin/env python3
"""
Configuration Drift Detector CLI Tool

This tool manages the drift detector backend service.
"""

import argparse
import socket
import sys
import os
import getpass

SOCKET_PATH = "/tmp/drift_detector.sock"

class DriftDetectorCLI:
    """CLI client for drift detector service"""
    
    def __init__(self):
        self.socket_path = SOCKET_PATH
    
    def send_command(self, command):
        """Send command to backend service via Unix socket"""
        try:
            sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            sock.connect(self.socket_path)
            sock.sendall(command.encode())
            response = sock.recv(4096).decode()
            sock.close()
            return response
        except FileNotFoundError:
            return "ERROR: Backend service not running (socket not found)"
        except ConnectionRefusedError:
            return "ERROR: Backend service not running (connection refused)"
        except Exception as e:
            return f"ERROR: {str(e)}"
    
    def add_file(self, filepath):
        """Add a file to the monitoring list"""
        # Expand path to absolute
        filepath = os.path.abspath(os.path.expanduser(filepath))
        
        # Check if file exists
        if not os.path.exists(filepath):
            print(f"Error: File '{filepath}' does not exist")
            return 1
        
        response = self.send_command(f"ADD {filepath}")
        print(response.strip())
        
        if response.startswith("OK"):
            return 0
        return 1
    
    def remove_file(self, filepath):
        """Remove a file from the monitoring list"""
        filepath = os.path.abspath(os.path.expanduser(filepath))
        response = self.send_command(f"REMOVE {filepath}")
        print(response.strip())
        
        if response.startswith("OK"):
            return 0
        return 1
    
    def create_baseline(self, filepath):
        """Create or reset baseline for a file"""
        filepath = os.path.abspath(os.path.expanduser(filepath))
        
        # Check if file exists
        if not os.path.exists(filepath):
            print(f"Error: File '{filepath}' does not exist")
            return 1
        
        response = self.send_command(f"BASELINE {filepath}")
        print(response.strip())
        
        if response.startswith("OK"):
            return 0
        return 1
    
    def get_status(self):
        """Get status of monitored files"""
        response = self.send_command("STATUS")
        print(response.strip())
        return 0
    
    def configure_email(self):
        """Configure SMTP email settings"""
        print("Configure Email Alert Settings")
        print("=" * 40)
        
        server = input("SMTP Server (e.g., smtp.gmail.com): ").strip()
        port = input("SMTP Port (default 587): ").strip() or "587"
        user = input("SMTP Username/Email: ").strip()
        password = getpass.getpass("SMTP Password: ").strip()
        alert_email = input("Alert Email Address: ").strip()
        
        if not all([server, port, user, password, alert_email]):
            print("Error: All fields are required")
            return 1
        
        # Send configuration to backend
        config_str = f"CONFIGURE {server}|{port}|{user}|{password}|{alert_email}"
        response = self.send_command(config_str)
        print(response.strip())
        
        if response.startswith("OK"):
            return 0
        return 1

def main():
    """Main CLI entry point"""
    parser = argparse.ArgumentParser(
        description="Configuration Drift Detector CLI",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s add /etc/nginx/nginx.conf    Add file to monitoring
  %(prog)s remove /etc/nginx/nginx.conf Remove file from monitoring
  %(prog)s baseline /etc/nginx/nginx.conf  Create/reset baseline
  %(prog)s status                       View monitored files
  %(prog)s configure                    Configure email alerts
        """
    )
    
    subparsers = parser.add_subparsers(dest='command', help='Available commands')
    
    # Add command
    add_parser = subparsers.add_parser('add', help='Add file to monitoring')
    add_parser.add_argument('file', help='Path to file to monitor')
    
    # Remove command
    remove_parser = subparsers.add_parser('remove', help='Remove file from monitoring')
    remove_parser.add_argument('file', help='Path to file to remove')
    
    # Baseline command
    baseline_parser = subparsers.add_parser('baseline', help='Create/reset baseline for file')
    baseline_parser.add_argument('file', help='Path to file')
    
    # Status command
    subparsers.add_parser('status', help='View monitored files status')
    
    # Configure command
    subparsers.add_parser('configure', help='Configure email alert settings')
    
    args = parser.parse_args()
    
    if not args.command:
        parser.print_help()
        return 1
    
    cli = DriftDetectorCLI()
    
    if args.command == 'add':
        return cli.add_file(args.file)
    elif args.command == 'remove':
        return cli.remove_file(args.file)
    elif args.command == 'baseline':
        return cli.create_baseline(args.file)
    elif args.command == 'status':
        return cli.get_status()
    elif args.command == 'configure':
        return cli.configure_email()
    
    return 1

if __name__ == '__main__':
    sys.exit(main())
