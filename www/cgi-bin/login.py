#!/usr/bin/env python3
import cgi, os, json, urllib.request, urllib.parse

# Configuration GitHub OAuth
github_client_id = "Ov23liqR1ibSAhoNpfGM"
github_client_secret = "b2273d5fcbdecd7e843efecbb4efac99afa8a509"

# Récupération des données du formulaire
client_data = cgi.FieldStorage()
login_type_by_user = client_data.getvalue("type")

print("Content-Type: application/json\r\n")

if not login_type_by_user:
    print(json.dumps({"success": False, "error": "Login Failed ༼つಠ益ಠ༽つ No type provided"}))
elif login_type_by_user == "github":
    # Gestion login GitHub
    code_from_github = client_data.getvalue("code")
    
    if not code_from_github:
        print(json.dumps({"success": False, "error": "GitHub Login Failed ༼つಠ益ಠ༽つ No code"}))
    else:
        try:
            # Échange du code contre un token
            github_token_data = urllib.parse.urlencode({
                "client_id": github_client_id,
                "client_secret": github_client_secret,
                "code": code_from_github
            }).encode()
            
            token_request = urllib.request.Request("https://github.com/login/oauth/access_token", data=github_token_data)
            token_request.add_header('Accept', 'application/json')
            
            with urllib.request.urlopen(token_request) as token_response:
                token_data_received = json.loads(token_response.read())
            
            access_token_from_github = token_data_received.get('access_token')
            
            if not access_token_from_github:
                print(json.dumps({"success": False, "error": "GitHub Token Failed ༼つಠ益ಠ༽つ"}))
            else:
                # Récupération des infos utilisateur
                user_info_request = urllib.request.Request("https://api.github.com/user")
                user_info_request.add_header('Authorization', f'token {access_token_from_github}')
                
                with urllib.request.urlopen(user_info_request) as user_response:
                    user_data_received = json.loads(user_response.read())
                
                username_from_github = user_data_received.get("login")
                print(json.dumps({"success": True, "login": username_from_github}))
                
        except Exception as github_error:
            print(json.dumps({"success": False, "error": f"GitHub Error: {str(github_error)}"}))

elif login_type_by_user == "standard":
    # Gestion login standard
    email_by_user = client_data.getvalue("email")
    password_by_user = client_data.getvalue("password")
    
    if not email_by_user or not password_by_user:
        print(json.dumps({"success": False, "error": "Standard Login Failed ༼つಠ益ಠ༽つ Missing data"}))
    else:
        # Nettoyage des caractères null
        if email_by_user:
            email_by_user = email_by_user.rstrip('\x00')
        if password_by_user:
            password_by_user = password_by_user.rstrip('\x00')
        
        # Stockage des utilisateurs
        storage_users_data = os.path.join(os.path.dirname(__file__), 'users.json')
        
        if not os.path.exists(storage_users_data):
            print(json.dumps({"success": False, "error": "Users File Not Found ༼つಠ益ಠ༽つ"}))
        else:
            with open(storage_users_data, 'r') as users_file:
                users_data_loaded = json.load(users_file)
            
            # Extraction du username
            username_extracted = email_by_user.split("@")[0] if "@" in email_by_user else email_by_user
            
            # Vérification des credentials
            if users_data_loaded.get(username_extracted) == password_by_user:
                print(json.dumps({"success": True, "login": username_extracted}))
            else:
                print(json.dumps({"success": False, "error": "Invalid Credentials ༼つಠ益ಠ༽つ"}))

else:
    print(json.dumps({"success": False, "error": "Invalid Login Type ༼つಠ益ಠ༽つ"}))