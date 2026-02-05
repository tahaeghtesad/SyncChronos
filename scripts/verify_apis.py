import re
import urllib.request
import urllib.error
import json
import socket
import struct
import time
import os

def parse_config():
    config = {}
    config_path = os.path.join(os.path.dirname(__file__), '..', 'src', 'config.h')
    
    try:
        with open(config_path, 'r') as f:
            content = f.read()
            
            # Extract WEATHER_API_KEY
            m = re.search(r'#define WEATHER_API_KEY "(.*)"', content)
            if m: config['weather_key'] = m.group(1)
            
            # Extract NTP_SERVER
            m = re.search(r'#define NTP_SERVER "(.*)"', content)
            if m: config['ntp_server'] = m.group(1)
            
    except FileNotFoundError:
        print(f"❌ Config file not found at {config_path}")
        
    return config

def check_weather(api_key):
    print(f"Checking OpenWeatherMap API Key: {api_key[:5]}...")
    url = f"https://api.openweathermap.org/data/2.5/weather?q=London&appid={api_key}"
    try:
        with urllib.request.urlopen(url) as response:
            if response.status == 200:
                print("✅ Weather API: Success")
                return True
    except urllib.error.HTTPError as e:
        print(f"❌ Weather API: Failed ({e.code}) - {e.reason}")
    except Exception as e:
        print(f"❌ Weather API: Error - {e}")
    return False

def check_timezone():
    print("Checking Open-Meteo Timezone API...")
    # Test with London coordinates
    url = "https://api.open-meteo.com/v1/forecast?latitude=51.5074&longitude=-0.1278&current=weather_code&timezone=auto"
    try:
        with urllib.request.urlopen(url) as response:
            if response.status == 200:
                data = json.loads(response.read().decode())
                if 'utc_offset_seconds' in data:
                    print(f"✅ Timezone API: Success (Offset: {data['utc_offset_seconds']}s)")
                    return True
                else:
                    print("❌ Timezone API: Response format unexpected")
    except urllib.error.HTTPError as e:
        print(f"❌ Timezone API: Failed ({e.code}) - {e.reason}")
    except Exception as e:
        print(f"❌ Timezone API: Error - {e}")
    return False

def check_ntp(server):
    print(f"Checking NTP Server: {server}")
    client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    client.settimeout(3)
    msg = b'\x1b' + 47 * b'\0'
    try:
        client.sendto(msg, (server, 123))
        client.recvfrom(1024)
        print("✅ NTP Server: Success (Response received)")
        return True
    except Exception as e:
        print(f"❌ NTP Server: Error - {e}")
        return False
    finally:
        client.close()

if __name__ == "__main__":
    print("--- VFD Clock API Verification ---")
    cfg = parse_config()
    
    if 'weather_key' in cfg:
        check_weather(cfg['weather_key'])
    else:
        print("⚠️ Weather API Key not found in config.h")
        
    check_timezone()
    
    if 'ntp_server' in cfg:
        check_ntp(cfg['ntp_server'])
    else:
        print("⚠️ NTP Server not found in config.h")
