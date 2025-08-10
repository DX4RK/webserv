#!/bin/bash
set -e

# Générer des stats système
DATE=$(date)
UPTIME=$(uptime)

# Créer le JSON
JSON="{\n  \"success\": true,\n  \"date\": \"$DATE\",\n  \"uptime\": \"$UPTIME\"\n}"

echo "Content-Type: application/json"
echo

echo -e "$JSON"