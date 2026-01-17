import sqlite3
import requests
import time
import os

DB_NAME = "scan_results_test.db"
UUID = "550e8400-e29b-41d4-a716-446655440000"

def create_db():
    if os.path.exists(DB_NAME):
        os.remove(DB_NAME)
    
    conn = sqlite3.connect(DB_NAME)
    cursor = conn.cursor()
    
    # Create system_info table as per heartbeat.cpp
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS system_info (
            uuid TEXT PRIMARY KEY, 
            internal_ip TEXT, 
            mac_address TEXT, 
            hostname TEXT, 
            os TEXT, 
            online_status TEXT, 
            last_active INTEGER, 
            external_ip TEXT, 
            data_status TEXT, 
            auto_screenshot INTEGER, 
            heartbeat_interval INTEGER
        );
    """)
    
    # Insert test data with auto_screenshot=1
    cursor.execute("""
        INSERT INTO system_info (
            uuid, internal_ip, mac_address, hostname, os, 
            online_status, last_active, external_ip, data_status, 
            auto_screenshot, heartbeat_interval
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    """, (UUID, "192.168.1.100", "00:11:22:33:44:55", "TestHost", "TestOS", 
          "Online", 1234567890, "8.8.8.8", "Testing", 1, 60))
    
    conn.commit()
    conn.close()
    print(f"Created {DB_NAME}")

def upload_db():
    url = "http://localhost:8101/api/c2/upload"
    files = {'file': (DB_NAME, open(DB_NAME, 'rb'), 'application/octet-stream')}
    data = {'uuid': UUID, 'taskId': 'test-task-1'}
    
    try:
        response = requests.post(url, files=files, data=data)
        print(f"Upload Status: {response.status_code}")
        print(f"Upload Response: {response.text}")
    except Exception as e:
        print(f"Upload Failed: {e}")

def check_device():
    url = f"http://localhost:8101/api/debug/device?uuid={UUID}"
    try:
        for i in range(10):
            try:
                response = requests.get(url)
                if response.status_code == 200 and response.text:
                    print(f"Device Info: {response.text}")
                    if '"isMonitorOn":1' in response.text or '"isMonitorOn": 1' in response.text:
                        print("SUCCESS: isMonitorOn is 1")
                        return
            except:
                pass
            print("Waiting for processing...")
            time.sleep(1)
        print("FAILED: Device info not updated or isMonitorOn mismatch")
    except Exception as e:
        print(f"Check Failed: {e}")

if __name__ == "__main__":
    create_db()
    upload_db()
    time.sleep(2) # Wait for async processing
    check_device()
