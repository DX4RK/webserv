import cgi, os

client_data = cgi.FieldStorage()
file_uploaded_by_user = client_data["uploadedFile"]
storage_files_uploaded = os.path.join(os.path.dirname(__file__), 'files_uploaded_by_users')

print("Content-Type: text/html;charset=utf-8\r\n")

if not file_uploaded_by_user.filename:
    print("<h1>Uploading Failed ༼つಠ益ಠ༽つ </h1>")
else:
    if not os.path.exists(storage_files_uploaded):
        os.mkdir(storage_files_uploaded)
    filename_uploaded_by_user = os.path.basename(file_uploaded_by_user.filename)
    store_file_path = os.path.join(storage_files_uploaded, filename_uploaded_by_user)
    with open(store_file_path, 'wb') as f:
        f.write(file_uploaded_by_user.file.read())
        print(f"<h1>File \"{filename_uploaded_by_user}\" Uploading Successful 🚬🗿 </h1>")
