#!/bin/bash

echo "Content-Type: application/json"
echo ""

UPLOAD_DIR="./www/cgi-bin/files_uploaded_by_users"
FORUM_POSTS="./www/cgi-bin/forum_posts.json"

upload_count=0
post_count=0
total_users=0

if [ -d "$UPLOAD_DIR" ]; then
    upload_count=$(find "$UPLOAD_DIR" -type f | wc -l)
fi

if [ -f "$FORUM_POSTS" ]; then
    post_count=$(grep -o '"id"' "$FORUM_POSTS" 2>/dev/null | wc -l)
fi

current_date=$(date "+%Y-%m-%d %H:%M:%S")

uptime_info=$(uptime | awk '{print $3, $4}' | sed 's/,//')

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