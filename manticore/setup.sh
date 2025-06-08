#!/bin/bash
docker compose up -d
echo "CREATE TABLE lore (id BIGINT, content TEXT, source TEXT)" | mysql -h 127.0.0.1 -P 9306
php bulk_lore_loader.php
