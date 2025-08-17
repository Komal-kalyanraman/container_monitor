# Podman Multi-Container Resource Monitor

This Python script continuously monitors live CPU, memory, and PID usage for **all running Podman containers** on your system. It dynamically adds new containers to monitoring and removes them when stopped or deleted. Output is similar to `podman stats`, but uses direct cgroup kernel data for accuracy.

## Features

- Tracks all running containers (no need to specify names/IDs).
- Prints live stats every second: container ID, name, CPU %, memory usage/limit, memory %, PIDs, average CPU %.
- Dynamically adds/removes containers as they start/stop.
- Uses cgroup v1 resource files (default for most Podman setups).
- No dependencies except Python 3.

## Usage

1. **Run the script as root (or with sudo):**
   ```bash
   sudo python3 cgroups_resource_reader.py
   ```

2. **Output example:**
   ```
   ID           NAME           CPU %     MEM USAGE / LIMIT   MEM %    PIDS   AVG CPU %
   04cf567d5479 nginx-limited   2.50%      7.24MB / 256.0MB   2.83%      9      2.50%
   ...
   ```

3. **Stop monitoring:**
   Press `Ctrl+C` to exit.

## Notes

- The script uses `sudo podman` commands and reads cgroup files, so root privileges are required.
- For cgroup v2 support, extend the script with logic for unified cgroup v2 resource files.
- Works with Podman containers; can be adapted for Docker with minor changes.

## Requirements

- Python 3.x
- Podman installed and running containers
- Root privileges (or configure passwordless sudo for podman and file reads)

## Extending

- Add cgroup v2 support for newer systems.
- Add CSV or log file output for historical tracking.
- Integrate with a real-time dashboard or UI.

---

**This script is ideal for development, validation, and lightweight monitoring in automotive and embedded