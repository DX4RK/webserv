#!/usr/bin/env python3
import os
import sys
import json

print("Content-Type: application/json\r\n")

try:
    content_length = int(os.environ.get('CONTENT_LENGTH', '0'))
    data = sys.stdin.buffer.read(content_length)
    
    data_str = data.decode('latin-1', errors='ignore')
    
    filename_start = data_str.find('filename="') + 10
    filename_end = data_str.find('"', filename_start)
    filename = data_str[filename_start:filename_end]
    
    if not filename:
        raise Exception("No filename found")
    
    binary_start = data.find(b'\r\n\r\n')
    if binary_start == -1:
        binary_start = data.find(b'\n\n')
        binary_start += 2
    else:
        binary_start += 4
    
    binary_end = data.rfind(b'\n--')
    if binary_end == -1:
        binary_end = data.rfind(b'\r\n--')
    if binary_end == -1:
        binary_end = len(data) - 100
    
    file_data = data[binary_start:binary_end]
    
    upload_dir = os.environ.get('UPLOAD_DIR')
    os.makedirs(upload_dir, exist_ok=True)
    
    file_path = os.path.join(upload_dir, filename)
    with open(file_path, 'wb') as f:
        f.write(file_data)
    
    response = {
        "success": True,
        "message": f"File '{filename}' uploaded successfully",
        "filename": filename,
        "file_size": len(file_data)
    }
    
    print(json.dumps(response))

except Exception as e:
    response = {"success": False, "error": str(e)}
    print(json.dumps(response))
