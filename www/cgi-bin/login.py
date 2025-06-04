#!/usr/bin/env python3
import cgi
import json
import urllib.request
import urllib.parse
import os

GITHUB_CLIENT_ID = "Ov23liqR1ibSAhoNpfGM"
GITHUB_CLIENT_SECRET = "b2273d5fcbdecd7e843efecbb4efac99afa8a509"

print("Content-Type: application/json\r\n")

form = cgi.FieldStorage()
login_type = form.getvalue("type")

try:
    if login_type == "github":
        code = form.getvalue("code")
        
        data = urllib.parse.urlencode({
            "client_id": GITHUB_CLIENT_ID,
            "client_secret": GITHUB_CLIENT_SECRET,
            "code": code
        }).encode()
        
        req = urllib.request.Request("https://github.com/login/oauth/access_token", data=data)
        req.add_header('Accept', 'application/json')
        
        with urllib.request.urlopen(req) as response:
            token_data = json.loads(response.read())
        
        access_token = token_data.get('access_token')
        
        user_req = urllib.request.Request("https://api.github.com/user")
        user_req.add_header('Authorization', f'token {access_token}')
        
        with urllib.request.urlopen(user_req) as user_response:
            user_data = json.loads(user_response.read())
        
        print(json.dumps({"success": True, "login": user_data.get("login")}))
        
    elif login_type == "standard":
        email = form.getvalue("email")
        password = form.getvalue("password")
        
        # Fix caractères null
        if email:
            email = email.rstrip('\x00')
        if password:
            password = password.rstrip('\x00')
        
        script_dir = os.path.dirname(os.path.abspath(__file__))
        users_file = os.path.join(script_dir, "users.json")
        with open(users_file, "r") as f:
            users = json.load(f)
        
        username = email.split("@")[0] if "@" in email else email
        
        if users.get(username) == password:
            print(json.dumps({"success": True, "login": username}))
        else:
            print(json.dumps({"success": False, "error": "Invalid credentials"}))
    
    else:
        print(json.dumps({"success": False, "error": "Invalid type"}))

except Exception as e:
    print(json.dumps({"success": False, "error": str(e)}))