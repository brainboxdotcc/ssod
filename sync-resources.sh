#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

rsync -avz --progress --delete resource/ ssod@beholder:/var/www/vhosts/images.ssod.org/resource/

