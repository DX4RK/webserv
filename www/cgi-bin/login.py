from flask import Flask, request, redirect, session, jsonify
import requests
import os
import json

app = Flask(__name__)
app.secret_key = os.urandom(24)

# ---------- CONFIG GITHUB ----------
GITHUB_CLIENT_ID = "TON_CLIENT_ID"
GITHUB_CLIENT_SECRET = "TON_CLIENT_SECRET"
GITHUB_REDIRECT_URI = "http://localhost:5000/github/callback"

# ---------- LOGIN CLASSIQUE ----------
def verify_user(username, password):
    try:
        with open("users.json", "r") as f:
            users = json.load(f)
        return users.get(username) == password
    except Exception as e:
        print("Erreur de lecture users.json:", e)
        return False

@app.route("/login/classic", methods=["POST"])
def login_classic():
    data = request.json
    username = data.get("username")
    password = data.get("password")

    if verify_user(username, password):
        session["user"] = username
        return jsonify({"success": True, "method": "classic", "user": username})
    else:
        return jsonify({"success": False, "message": "Identifiants invalides"}), 401

# ---------- LOGIN GITHUB ----------
@app.route("/login/github")
def login_github():
    return redirect(
        f"https://github.com/login/oauth/authorize?client_id={GITHUB_CLIENT_ID}&redirect_uri={GITHUB_REDIRECT_URI}"
    )

@app.route("/github/callback")
def github_callback():
    code = request.args.get("code")
    if not code:
        return "Code manquant", 400

    token_url = "https://github.com/login/oauth/access_token"
    headers = {"Accept": "application/json"}
    data = {
        "client_id": GITHUB_CLIENT_ID,
        "client_secret": GITHUB_CLIENT_SECRET,
        "code": code,
        "redirect_uri": GITHUB_REDIRECT_URI,
    }
    token_response = requests.post(token_url, headers=headers, data=data)
    access_token = token_response.json().get("access_token")

    user_info = requests.get(
        "https://api.github.com/user",
        headers={"Authorization": f"token {access_token}"}
    ).json()

    session["user"] = user_info.get("login")
    return jsonify({"success": True, "method": "github", "github_user": user_info.get("login")})

# ---------- LOGOUT ----------
@app.route("/logout")
def logout():
    session.clear()
    return jsonify({"success": True, "message": "Déconnecté"})

if __name__ == "__main__":
    app.run(debug=True)
