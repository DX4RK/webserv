#!/usr/bin/env python3
import os, sys, json

print("Content-Type: application/json;charset=utf-8\r\n")

try:
    content_length = int(os.environ.get('CONTENT_LENGTH', '0'))
    data = sys.stdin.buffer.read(content_length)
    data_str = data.decode('latin-1', errors='ignore')
    
    start = data_str.find('filename="') + 10
    end = data_str.find('"', start)
    filename = data_str[start:end]
    
    if not filename:
        print(json.dumps({"status": "error", "message": "No filename provided"}))
    else:
        file_start = data.find(b'\r\n\r\n') + 4
        file_end = data.rfind(b'\r\n-')
        file_data = data[file_start:file_end]
        
        storage_dir = os.path.join(os.path.dirname(__file__), 'files_uploaded_by_users')
        if not os.path.exists(storage_dir):
            os.mkdir(storage_dir)
        
        file_path = os.path.join(storage_dir, filename)
        with open(file_path, 'wb') as f:
            f.write(file_data)
        
        print(json.dumps({"status": "success", "filename": filename}))

except Exception as e:
    print(json.dumps({"status": "error", "message": "Error uploading file"}))
