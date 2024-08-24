#!/bin/sh
mkdir -p debug
sentry-cli debug-files upload -p ssod-bot --log-level=info ./ssod

