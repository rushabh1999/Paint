#!/usr/bin/env python3
"""
Unit tests for drift_cli.py
"""

import unittest
import os
import sys
import tempfile
from unittest.mock import patch, MagicMock

# Add parent directory to path
cli_path = os.path.join(os.path.dirname(__file__), '..', '..', 'cli')
sys.path.insert(0, cli_path)

import drift_cli
from drift_cli import DriftDetectorCLI

class TestDriftDetectorCLI(unittest.TestCase):
    """Test cases for DriftDetectorCLI"""
    
    def setUp(self):
        """Set up test fixtures"""
        self.cli = DriftDetectorCLI()
        self.temp_file = tempfile.NamedTemporaryFile(delete=False)
        self.temp_file.write(b"test content")
        self.temp_file.close()
    
    def tearDown(self):
        """Clean up test fixtures"""
        if os.path.exists(self.temp_file.name):
            os.unlink(self.temp_file.name)
    
    @patch('socket.socket')
    def test_send_command_success(self, mock_socket):
        """Test successful command sending"""
        mock_sock = MagicMock()
        mock_sock.recv.return_value = b"OK: Success"
        mock_socket.return_value = mock_sock
        
        response = self.cli.send_command("TEST")
        
        self.assertEqual(response, "OK: Success")
        mock_sock.connect.assert_called_once()
        mock_sock.sendall.assert_called_once_with(b"TEST")
    
    @patch('socket.socket')
    def test_send_command_connection_error(self, mock_socket):
        """Test connection error handling"""
        mock_sock = MagicMock()
        mock_sock.connect.side_effect = ConnectionRefusedError()
        mock_socket.return_value = mock_sock
        
        response = self.cli.send_command("TEST")
        
        self.assertTrue(response.startswith("ERROR"))
    
    @patch.object(DriftDetectorCLI, 'send_command')
    def test_add_file_success(self, mock_send):
        """Test adding a file successfully"""
        mock_send.return_value = "OK: File added to monitoring"
        
        result = self.cli.add_file(self.temp_file.name)
        
        self.assertEqual(result, 0)
        mock_send.assert_called_once()
        self.assertTrue(mock_send.call_args[0][0].startswith("ADD"))
    
    def test_add_file_not_exists(self):
        """Test adding a non-existent file"""
        result = self.cli.add_file("/nonexistent/file")
        self.assertEqual(result, 1)
    
    @patch.object(DriftDetectorCLI, 'send_command')
    def test_remove_file_success(self, mock_send):
        """Test removing a file successfully"""
        mock_send.return_value = "OK: File removed from monitoring"
        
        result = self.cli.remove_file(self.temp_file.name)
        
        self.assertEqual(result, 0)
        mock_send.assert_called_once()
        self.assertTrue(mock_send.call_args[0][0].startswith("REMOVE"))
    
    @patch.object(DriftDetectorCLI, 'send_command')
    def test_create_baseline_success(self, mock_send):
        """Test creating baseline successfully"""
        mock_send.return_value = "OK: Baseline created"
        
        result = self.cli.create_baseline(self.temp_file.name)
        
        self.assertEqual(result, 0)
        mock_send.assert_called_once()
        self.assertTrue(mock_send.call_args[0][0].startswith("BASELINE"))
    
    def test_create_baseline_not_exists(self):
        """Test creating baseline for non-existent file"""
        result = self.cli.create_baseline("/nonexistent/file")
        self.assertEqual(result, 1)
    
    @patch.object(DriftDetectorCLI, 'send_command')
    def test_get_status(self, mock_send):
        """Test getting status"""
        mock_send.return_value = "Monitored files:\n  [1] /etc/test.conf"
        
        result = self.cli.get_status()
        
        self.assertEqual(result, 0)
        mock_send.assert_called_once_with("STATUS")

if __name__ == '__main__':
    unittest.main()
