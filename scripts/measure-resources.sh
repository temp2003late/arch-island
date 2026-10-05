#!/bin/bash
# Measures only this application's CPU ticks and RSS; never stops another process.
set -euo pipefail
profile_binary=$(realpath "${1:-./build/arch-island}")
profile_mode=${2:-animated}
if [[ "$profile_mode" != animated && "$profile_mode" != reduced ]]; then
    echo 'Usage: measure-resources.sh [binary] [animated|reduced]' >&2
    exit 2
fi
profile_config=$(mktemp -d /tmp/arch-island-profile.XXXXXX)
trap 'rm -rf "$profile_config"' EXIT
mkdir -p "$profile_config/ArchIsland"
if [[ "$profile_mode" == reduced ]]; then
    printf '[appearance]\nreducedMotion=true\nmode=night\n' > "$profile_config/ArchIsland/arch-island.conf"
else
    printf '[appearance]\nreducedMotion=false\nmode=night\n' > "$profile_config/ArchIsland/arch-island.conf"
fi
XDG_CONFIG_HOME="$profile_config" "$profile_binary" --smoke-test 30 > "$profile_config/run.log" 2>&1 &
profile_pid=$!
sleep 5
profile_cpu_start=$(awk '{print $14+$15}' "/proc/$profile_pid/stat")
profile_time_start=$(awk '{print $1}' /proc/uptime)
sleep 20
profile_cpu_end=$(awk '{print $14+$15}' "/proc/$profile_pid/stat")
profile_time_end=$(awk '{print $1}' /proc/uptime)
profile_rss=$(awk '/^VmRSS:/ {print $2}' "/proc/$profile_pid/status")
awk -v mode="$profile_mode" -v a="$profile_cpu_start" -v b="$profile_cpu_end" -v t0="$profile_time_start" -v t1="$profile_time_end" -v hz="$(getconf CLK_TCK)" -v rss="$profile_rss" 'BEGIN {printf "%s: elapsed=%.2fs CPU=%.2f%% of one core, RSS=%.2f MiB\n",mode,t1-t0,100*(b-a)/hz/(t1-t0),rss/1024}'
wait "$profile_pid"
cat "$profile_config/run.log"
