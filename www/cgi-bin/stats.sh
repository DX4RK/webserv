#!/bin/bash

# Script de statistiques pour webserv
# Compte les uploads et posts du forum

# Headers HTTP pour la réponse
echo "Content-Type: application/json"
echo ""

# Chemins vers les fichiers de données
UPLOAD_DIR="./www/cgi-bin/files_uploaded_by_users"
FORUM_POSTS="./www/cgi-bin/forum_posts.json"

# Initialiser les compteurs
upload_count=0
post_count=0
total_users=0

# Compter les fichiers uploadés
if [ -d "$UPLOAD_DIR" ]; then
    upload_count=$(find "$UPLOAD_DIR" -type f | wc -l)
fi

# Compter les posts du forum
if [ -f "$FORUM_POSTS" ]; then
    # Utiliser grep pour compter les posts (chaque post a un "id")
    post_count=$(grep -o '"id"' "$FORUM_POSTS" 2>/dev/null | wc -l)
fi

# Obtenir la date actuelle
current_date=$(date "+%Y-%m-%d %H:%M:%S")

# Calculer l'uptime du serveur (approximatif)
uptime_info=$(uptime | awk '{print $3, $4}' | sed 's/,//')

# Générer la réponse JSON
cat << EOF
{
  "success": true,
  "stats": {
    "uploads": $upload_count,
    "forum_posts": $post_count,
    "last_updated": "$current_date",
    "server_uptime": "$uptime_info"
  },
  "message": "Statistics retrieved successfully 📊🗿"
}
EOF