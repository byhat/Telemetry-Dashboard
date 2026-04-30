#!/usr/bin/env python3
"""
Device Simulator for IoT Backend Testing

This script simulates multiple IoT devices sending telemetry data to a backend server
via HTTP POST (REST API). It's designed for load testing and development purposes.

Features:
- Simulate 1-100+ simultaneous devices
- Send telemetry data (temperature, power, voltage) via HTTP POST
- Periodically send error messages
- Configurable update intervals
- Graceful error handling and logging

Usage Examples:
    # Simulate 5 devices with default settings
    python device_simulator.py --devices 5

    # Simulate 20 devices with custom URL
    python device_simulator.py --devices 20 --url http://localhost:18080/api/telemetry

    # Simulate 10 devices with 2-second update interval
    python device_simulator.py --devices 10 --interval 2

    # Simulate 50 devices connecting to a remote server
    python device_simulator.py --devices 50 --url http://example.com/api/telemetry

Requirements:
    - requests (for HTTP POST)
    - threading (built-in)
"""

import argparse
import json
import logging
import random
import sys
import threading
import time
from typing import Dict, Any
from datetime import datetime, timezone

try:
    import requests
    REQUESTS_AVAILABLE = True
except ImportError:
    REQUESTS_AVAILABLE = False


# Configure logging - global logger without device_id
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    datefmt='%Y-%m-%d %H:%M:%S'
)
logger = logging.getLogger(__name__)


class DeviceSimulator:
    """
    Simulates a single IoT device sending telemetry data via HTTP POST.
    
    Each device has a unique ID and sends periodic telemetry data including
    temperature, power consumption, and voltage readings.
    """
    
    # Error messages that can be randomly generated
    ERROR_MESSAGES = [
        "Power fluctuation outside normal range",
        "Communication timeout",
        "Temperature sensor reading unstable",
        "Connection unstable, retrying",
        "Memory buffer near capacity",
        "Overheating warning"
    ]
    
    def __init__(
        self,
        device_id: str,
        url: str,
        interval: float = 1.0,
        error_interval: float = 10.0
    ):
        """
        Initialize a device simulator.
        
        Args:
            device_id: Unique identifier for this device
            url: Backend URL (HTTP endpoint for telemetry)
            interval: Time in seconds between telemetry updates
            error_interval: Time in seconds between error messages
        """
        self.device_id = device_id
        self.url = url
        self.interval = interval
        self.error_interval = error_interval
        self.running = False
        self.last_error_time = 0
        
        # Device-specific characteristics for more realistic simulation
        self.base_temperature = random.uniform(30, 60)  # Base temperature in °C
        self.base_power = random.uniform(100, 800)      # Base power in W
        self.base_voltage = random.uniform(110, 240)    # Base voltage in V
        
        # Create a device-specific logger with device_id in format
        device_logger = logging.getLogger(f'{__name__}.{device_id}')
        device_logger.setLevel(logger.level)
        device_logger.propagate = False  # Prevent duplicate logging to root logger
        handler = logging.StreamHandler()
        handler.setFormatter(logging.Formatter(
            '%(asctime)s - %(levelname)s - [%(device_id)s] %(message)s',
            datefmt='%Y-%m-%d %H:%M:%S'
        ))
        device_logger.addHandler(handler)
        self.logger = logging.LoggerAdapter(device_logger, {'device_id': device_id})
    
    def generate_telemetry(self) -> Dict[str, Any]:
        """
        Generate realistic telemetry data.
        
        Returns:
            Dictionary containing device_id, temperature, power, and voltage
        """
        # Add some realistic variation to the base values
        temperature = self.base_temperature + random.uniform(-5, 10)
        power = max(0, self.base_power + random.uniform(-50, 100))
        voltage = max(0, self.base_voltage + random.uniform(-10, 10))
        
        # Clamp values to reasonable ranges
        temperature = max(0, min(100, temperature))
        power = max(0, min(1000, power))
        voltage = max(0, min(250, voltage))
        
        return {
            "type": "telemetry",
            "device_id": self.device_id,
            "temperature": round(temperature, 2),
            "power": round(power, 2),
            "voltage": round(voltage, 2),
            "timestamp": int(datetime.now(timezone.utc).timestamp() * 1000)
        }
    
    def generate_error(self) -> Dict[str, Any]:
        """
        Generate a random error message.
        
        Returns:
            Dictionary containing device_id and error message
        """
        error_message = random.choice(self.ERROR_MESSAGES)
        return {
            "type": "error",
            "device_id": self.device_id,
            "error": error_message,
            "timestamp": int(datetime.now(timezone.utc).timestamp() * 1000)
        }
    
    def send_http(self, data: Dict[str, Any]) -> bool:
        """
        Send data via HTTP POST.
        
        Args:
            data: Dictionary to send as JSON
            
        Returns:
            True if successful, False otherwise
        """
        try:
            # Determine the correct endpoint based on data type
            if data.get("type") == "error":
                endpoint = self.url.replace("/telemetry", "/error")
            else:
                endpoint = self.url
            
            response = requests.post(
                endpoint,
                json=data,
                headers={'Content-Type': 'application/json'},
                timeout=5
            )
            response.raise_for_status()
            return True
        except requests.exceptions.RequestException as e:
            self.logger.error(f"HTTP error: {e}")
            return False
    
    def run(self):
        """
        Main device simulation loop.
        
        Continuously sends telemetry data and periodic error messages
        until stopped.
        """
        self.running = True
        self.logger.info(f"Device simulator started")
        
        while self.running:
            try:
                # Send telemetry data
                telemetry = self.generate_telemetry()
                self.logger.debug(f"Sending telemetry: {telemetry}")
                
                success = self.send_http(telemetry)
                
                if success:
                    self.logger.debug(f"Telemetry sent: temp={telemetry['temperature']}°C, "
                                    f"power={telemetry['power']}W, voltage={telemetry['voltage']}V")
                
                # Check if it's time to send an error
                current_time = time.time()
                if current_time - self.last_error_time >= self.error_interval:
                    # 30% chance to send an error at each interval
                    if random.random() < 0.3:
                        error_data = self.generate_error()
                        self.logger.info(f"Sending error: {error_data['error']}")
                        self.send_http(error_data)
                        self.last_error_time = current_time
                
                # Wait for the next interval
                time.sleep(self.interval)
                
            except KeyboardInterrupt:
                break
            except Exception as e:
                self.logger.error(f"Unexpected error in device loop: {e}")
                time.sleep(self.interval)
        
        self.stop()
    
    def stop(self):
        """Stop the device simulator and cleanup resources."""
        self.running = False
        self.logger.info("Device simulator stopped")


def run_multiple_devices(
    num_devices: int,
    url: str,
    interval: float = 1.0,
    error_interval: float = 10.0
):
    """
    Run multiple device simulators concurrently.
    
    Args:
        num_devices: Number of devices to simulate
        url: Backend URL
        interval: Time in seconds between telemetry updates
        error_interval: Time in seconds between error messages
    """
    logger.info(f"Starting {num_devices} device simulators...")
    logger.info(f"Backend URL: {url}")
    logger.info(f"Protocol: HTTP POST")
    logger.info(f"Update interval: {interval}s")
    logger.info(f"Error interval: ~{error_interval}s")
    
    devices = []
    threads = []
    
    # Create device simulators
    for i in range(1, num_devices + 1):
        device_id = f"device_{i}"
        device = DeviceSimulator(
            device_id=device_id,
            url=url,
            interval=interval,
            error_interval=error_interval
        )
        devices.append(device)
        
        # Create and start thread for each device
        thread = threading.Thread(target=device.run, daemon=True)
        thread.start()
        threads.append(thread)
    
    try:
        # Wait for all threads to complete (they run indefinitely)
        for thread in threads:
            thread.join()
    except KeyboardInterrupt:
        logger.info("Received interrupt signal, stopping all devices...")
    finally:
        # Stop all devices
        for device in devices:
            device.stop()
        
        logger.info("All device simulators stopped")


def parse_arguments():
    """
    Parse command-line arguments.
    
    Returns:
        Parsed arguments namespace
    """
    parser = argparse.ArgumentParser(
        description='IoT Device Simulator - Simulate multiple devices sending telemetry data via HTTP POST',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Simulate 5 devices with default settings
  python device_simulator.py --devices 5

  # Simulate 20 devices with custom URL
  python device_simulator.py --devices 20 --url http://localhost:18080/api/telemetry

  # Simulate 10 devices with 2-second update interval
  python device_simulator.py --devices 10 --interval 2

  # Simulate 50 devices connecting to a remote server
  python device_simulator.py --devices 50 --url http://example.com/api/telemetry
        """
    )
    
    parser.add_argument(
        '-d', '--devices',
        type=int,
        default=1,
        help='Number of devices to simulate (default: 1)'
    )
    
    parser.add_argument(
        '-i', '--interval',
        type=float,
        default=1.0,
        help='Update interval in seconds for telemetry data (default: 1.0)'
    )
    
    parser.add_argument(
        '-e', '--error-interval',
        type=float,
        default=10.0,
        help='Approximate interval in seconds between error messages (default: 10.0)'
    )
    
    parser.add_argument(
        '-u', '--url',
        type=str,
        default='http://localhost:18080/api/telemetry',
        help='Backend URL for telemetry (default: http://localhost:18080/api/telemetry)'
    )
    
    parser.add_argument(
        '-v', '--verbose',
        action='store_true',
        help='Enable verbose (debug) logging'
    )
    
    return parser.parse_args()


def main():
    """Main entry point for the device simulator."""
    args = parse_arguments()
    
    # Set logging level
    if args.verbose:
        logging.getLogger().setLevel(logging.DEBUG)
    
    # Check dependencies
    if not REQUESTS_AVAILABLE:
        logger.error("HTTP mode requires 'requests' package. "
                    "Install it with: pip install requests")
        sys.exit(1)
    
    # Validate arguments
    if args.devices < 1:
        logger.error("Number of devices must be at least 1")
        sys.exit(1)
    
    if args.interval <= 0:
        logger.error("Interval must be greater than 0")
        sys.exit(1)
    
    # Run the device simulators
    try:
        run_multiple_devices(
            num_devices=args.devices,
            url=args.url,
            interval=args.interval,
            error_interval=args.error_interval
        )
    except KeyboardInterrupt:
        logger.info("Simulation stopped by user")
        sys.exit(0)


if __name__ == '__main__':
    main()
