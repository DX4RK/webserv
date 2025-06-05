#!/usr/bin/env python3
import os, sys

print("Content-Type: text/html;charset=utf-8\r\n")

try:
    content_length = int(os.environ.get('CONTENT_LENGTH', '0'))
    data = sys.stdin.buffer.read(content_length)
    data_str = data.decode('latin-1', errors='ignore')
    
    start = data_str.find('filename="') + 10
    end = data_str.find('"', start)
    filename = data_str[start:end]
    
    if not filename:
        print("<h1>Uploading Failed ༼つಠ益ಠ༽つ </h1>")
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
        
        print(f"<h1>File \"{filename}\" Uploading Successful 🚬🗿 </h1>")

except:
    print("<h1>Uploading Failed ༼つಠ益ಠ༽つ </h1>")