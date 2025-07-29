 #!/bin/bash
set -e

# Générer des stats système
DATE=$(date)
UPTIME=$(uptime)
MEM=$(free -m | awk '/Mem:/ {print $3"/"$2" MB"}')
DISK=$(df -h / | awk 'NR==2 {print $3"/"$2}')

# Créer le JSON
JSON="{\n  \"success\": true,\n  \"date\": \"$DATE\",\n  \"uptime\": \"$UPTIME\",\n  \"memory\": \"$MEM\",\n  \"disk\": \"$DISK\"\n}"

# En-tête HTTP
echo "Content-Type: application/json"
echo

# Afficher le JSON
echo -e "$JSON"