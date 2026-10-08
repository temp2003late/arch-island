#!/bin/bash
# Bounded two-minute visible night run; samples only the process started here.
set -euo pipefail
stability_binary=$(realpath "${1:-./build/arch-island}")
stability_config=$(mktemp -d /tmp/arch-island-stability.XXXXXX)
trap 'rm -rf "$stability_config"' EXIT
mkdir -p "$stability_config/ArchIsland"
printf '[appearance]\nreducedMotion=false\nmode=night\n' > "$stability_config/ArchIsland/arch-island.conf"
XDG_CONFIG_HOME="$stability_config" "$stability_binary" --smoke-test 120 > "$stability_config/run.log" 2>&1 &
stability_pid=$!
sleep 15
for stability_sample in 15 45 75 105; do
    awk -v t="$stability_sample" '/^VmRSS:/ {printf "time=%ds RSS=%.2f MiB\n",t,$2/1024}' "/proc/$stability_pid/status"
    awk -v t="$stability_sample" '{printf "time=%ds CPU_ticks=%d\n",t,$14+$15}' "/proc/$stability_pid/stat"
    if [[ "$stability_sample" != 105 ]]; then sleep 30; fi
done
wait "$stability_pid"
cat "$stability_config/run.log"
