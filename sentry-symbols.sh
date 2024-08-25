#!/bin/bash
if [ "$HOSTNAME" = beholder ]; then
  sentry-cli debug-files upload -p ssod-bot --log-level=info ./ssod
else
  echo "Not uploading symbols on server $HOSTNAME"
fi
