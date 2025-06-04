#!/usr/bin/env python3
import cgi, os, json, datetime

# Récupération des données du formulaire
client_data = cgi.FieldStorage()
action_by_user = client_data.getvalue("type")  # "forum" ou "get_posts"
storage_posts = os.path.join(os.path.dirname(__file__), 'forum_posts.json')

print("Content-Type: application/json\r\n")

if action_by_user == "forum":
    # Poster un nouveau message
    user_logged = client_data.getvalue("user")
    message_by_user = client_data.getvalue("message")

    if not user_logged or not message_by_user:
        print(json.dumps({"success": False, "error": "Missing data ༼つಠ益ಠ༽つ"}))
    else:
        # Charger posts existants ou créer vide sur le fichier forum_post.json
        posts_data = json.load(open(storage_posts)) if os.path.exists(storage_posts) else {"posts": []}
        
        # Créer un nouveau post
        new_post = {"id": len(posts_data["posts"]) + 1, "user": user_logged.rstrip('\x00\r\n'), "message": message_by_user.rstrip('\x00'), "timestamp": datetime.datetime.now().isoformat(), "date": datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")}
        
        # Ajouter et sauvegarder
        posts_data["posts"].append(new_post)
        json.dump(posts_data, open(storage_posts, 'w'), indent=2)
        print(json.dumps({"success": True, "message": "Post created 🚬🗿", "post_id": new_post["id"]}))

elif action_by_user == "get_posts":
    # Récupérer tous les posts (ternaire : si fichier existe ? charger : vide)
    print(json.dumps(json.load(open(storage_posts)) if os.path.exists(storage_posts) else {"posts": []}))

else:
    print(json.dumps({"success": False, "error": "Invalid action ༼つಠ益ಠ༽つ"}))