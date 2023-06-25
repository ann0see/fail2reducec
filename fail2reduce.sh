#!/bin/bash
set -eu
# fail2reducec outputs monitors_file if it was the first invocation meaning it also does the monitoring and reducing
MONITORS_FILE=$(./fail2reducec "${1}" "/tmp/fail2reducebuffer.tmp" "/var/run/lock/fail2reducec.lock")
if [ "${MONITORS_FILE}" == "monitors_file" ]; then
  flock -x /tmp/fail2reducebuffer.tmp -c "cat /tmp/fail2reducebuffer.tmp | ${2}; rm /tmp/fail2reducebuffer.tmp"
fi
