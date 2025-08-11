#!/usr/bin/env python3
import os
import cgi
import datetime
import html

print("Content-Type: text/html\n")

target_path = os.environ.get('PATH_INFO')

if not target_path:
	print("Content-Type: text/plain\n")
	print("Error: PATH environment variable not set")
	exit()

current_dir = os.path.normpath(target_path)

#current_dir = os.getcwd()
entries = os.listdir(current_dir)

dirs = []
files = []

for entry in entries:
    full_path = os.path.join(current_dir, entry)
    if os.path.isdir(full_path):
        dirs.append(entry)
    else:
        files.append(entry)

dir_count = len(dirs)
file_count = len(files)

print(f"""<!DOCTYPE html>
<html lang="en-US">
<head>
    <meta charset="UTF-8">
    <title>Index Of:</title>
    <style>
        body {{
            display: block;
            color: white;
            font-family: 'Courier New', Courier, monospace;
            justify-content: center;
            background-color: rgb(24, 24, 24);
        }}
        h1, p {{
            margin: 0;
        }}
        .header {{
            width: 98%;
            padding: 1%;
            display: grid;
            height: fit-content;
        }}
        .header h1 {{
            margin-bottom: 0.25%;
            font-size: 150%;
            opacity: 0.85;
        }}
        .header p {{
            font-size: 85%;
            opacity: 0.65;
        }}
        #line {{
            width: 100%;
            height: 1px;
            display: flex;
            justify-content: center;
        }}
        .line {{
            height: 100%;
            width: 98%;
            opacity: 0.2;
            background-color: white;
        }}
        main {{
            padding-top: 0.5%;
            display: flex;
            justify-content: center;
        }}
        table {{
            width: 98%;
            border-collapse: collapse;
        }}
        th, td {{
            text-align: left;
            padding: 10px 0;
        }}
        tr {{
            padding: 5%;
            opacity: 0.85;
            border-bottom: 1px dashed rgba(255, 255, 255, 0.25);
        }}
		a:-webkit-any-link {{
			color: #00b6ff;
		}}
    </style>
</head>
<body>
    <div class="header">
        <h1>Index of {html.escape(current_dir)}</h1>
        <p>{dir_count} directorie(s), {file_count} file(s)</p>
    </div>
    <div id="line"><div class="line"></div></div>
    <main>
        <table>
            <thead>
                <tr>
                    <th></th>
                    <th>Name</th>
                    <th>Size</th>
                    <th>Modified</th>
                </tr>
            </thead>
            <tbody>
                <tr>
                    <td></td>
                    <td><a href="../">..</a></td>
                    <td data-order="1">—</td>
                    <td><time>—</time></td>
                </tr>
""")

for d in sorted(dirs):
    path = os.path.join(current_dir, d)
    mod_time = datetime.datetime.fromtimestamp(os.path.getmtime(path))
    print(f"""
                <tr>
                    <td></td>
                    <td><a href="{html.escape(d)}/">{html.escape(d)}</a></td>
                    <td data-order="1">0 bytes</td>
                    <td><time>{mod_time}</time></td>
                </tr>
    """)

for f in sorted(files):
    path = os.path.join(current_dir, f)
    size = os.path.getsize(path)
    mod_time = datetime.datetime.fromtimestamp(os.path.getmtime(path))
    print(f"""
                <tr>
                    <td></td>
                    <td><a href="{html.escape(f)}">{html.escape(f)}</a></td>
                    <td data-order="1">{size} bytes</td>
                    <td><time>{mod_time}</time></td>
                </tr>
    """)

print("""
            </tbody>
        </table>
    </main>
</body>
</html>
""")
