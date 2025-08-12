#!/bin/bash
set -e

DATE=$(date)

JSON="{\n  \"success\": true,\n  \"date\": \"$DATE\",\n}"

echo "Content-Type: application/json"
echo

echo -e "$JSON"