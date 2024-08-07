#!/bin/sh
objcopy --only-keep-debug --compress-debug-sections=zlib ./ssod ./ssod.debug
sentry-cli debug-files upload -p ssod-bot --log-level=info ./*.debug

