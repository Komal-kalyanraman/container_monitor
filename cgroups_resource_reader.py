import subprocess
import os
import time
import json
import argparse
import shutil
import sys

SUPPORTED_RUNTIMES = ("docker", "podman")

def which_or_exit(name):
    path = shutil.which(name)
    if not path:
        print(f"Runtime binary '{name}' not found in PATH.")
        sys.exit(1)

def build_cmd(runtime, sudo, args):
    if sudo:
        return ["sudo", runtime] + args
    return [runtime] + args

def list_containers(runtime, sudo):
    # Common Go template works for both docker & podman
    cmd = build_cmd(runtime, sudo, ["ps", "--format", "{{.ID}} {{.Names}}"])
    try:
        output = subprocess.check_output(cmd).decode().strip()
    except subprocess.CalledProcessError:
        return []
    containers = []
    for line in output.splitlines():
        parts = line.split()
        if len(parts) >= 2:
            containers.append({"Id": parts[0], "Name": parts[1]})
    return containers

def get_pid(runtime, sudo, name):
    cmd = build_cmd(runtime, sudo, ["inspect", "-f", "{{.State.Pid}}", name])
    try:
        pid = subprocess.check_output(cmd).decode().strip()
        return pid if pid.isdigit() else None
    except Exception:
        return None

def inspect_container(runtime, sudo, name):
    cmd = build_cmd(runtime, sudo, ["inspect", name])
    try:
        data = json.loads(subprocess.check_output(cmd).decode())[0]
        return data
    except Exception:
        return {}

def detect_cgroup_version():
    return "v2" if os.path.exists("/sys/fs/cgroup/cgroup.controllers") else "v1"

def get_cgroup_paths(pid, version):
    paths = {}
    try:
        with open(f"/proc/{pid}/cgroup") as f:
            for line in f:
                parts = line.strip().split(":")
                if version == "v2":
                    # unified line looks like: 0::/user.slice/...
                    if parts[0] == "0":
                        paths["base"] = parts[2]
                else:  # v1
                    controllers = parts[1].split(",")
                    path = parts[2]
                    if "memory" in controllers:
                        paths["memory"] = path
                    if "cpu" in controllers or "cpuacct" in controllers:
                        # unify under cpu key
                        paths["cpu"] = path
                    if "pids" in controllers:
                        paths["pids"] = path
    except Exception:
        pass
    return paths

def safe_read(path):
    try:
        with open(path) as f:
            return f.read().strip()
    except Exception:
        return None

def parse_int(value):
    if value is None:
        return None
    try:
        return int(value)
    except Exception:
        return None

def get_stats_v1(paths, hinted_mem_limit):
    # memory
    mem_path = f"/sys/fs/cgroup/memory{paths.get('memory','')}"
    mem_usage = parse_int(safe_read(f"{mem_path}/memory.usage_in_bytes")) or 0
    mem_limit = parse_int(safe_read(f"{mem_path}/memory.limit_in_bytes"))
    if not mem_limit or mem_limit <= 0 or mem_limit > (1 << 60):
        mem_limit = hinted_mem_limit if hinted_mem_limit else 0
    mem_percent = (mem_usage / mem_limit * 100) if mem_limit else 0.0

    # cpu
    cpu_path = f"/sys/fs/cgroup/cpu{paths.get('cpu','')}"
    cpu_usage = parse_int(safe_read(f"{cpu_path}/cpuacct.usage")) or 0

    # pids
    pids_path = f"/sys/fs/cgroup/pids{paths.get('pids','')}"
    pids_current = parse_int(safe_read(f"{pids_path}/pids.current")) or 0
    raw_max = safe_read(f"{pids_path}/pids.max")
    if raw_max in (None, "max"):
        pids_max = None
    else:
        pids_max = parse_int(raw_max)
    return {
        "mem_usage": mem_usage,
        "mem_limit": mem_limit,
        "mem_percent": mem_percent,
        "cpu_usage_ns": cpu_usage,
        "pids_current": pids_current,
        "pids_limit": pids_max
    }

def format_line(cid, name, cpu_pct, s, avg_cpu):
    mem_usage_mb = s["mem_usage"] / 1024 / 1024
    mem_limit_mb = (s["mem_limit"] / 1024 / 1024) if s["mem_limit"] else 0
    pids_limit_str = str(s["pids_limit"]) if s["pids_limit"] else "-"
    return (f"{cid[:12]:<12} {name:<14} {cpu_pct:>7.2f}%   "
            f"{mem_usage_mb:>7.2f}MB / {mem_limit_mb:.1f}MB   "
            f"{s['mem_percent']:>6.2f}%   {s['pids_current']:>5}/{pids_limit_str:<5} {avg_cpu:>7.2f}%")

class ContainerMonitor:
    def __init__(self, runtime, sudo):
        self.runtime = runtime
        self.sudo = sudo
        self.version = detect_cgroup_version()
        self.containers = {}  # id -> info

    def update(self):
        running = list_containers(self.runtime, self.sudo)
        running_ids = {c["Id"] for c in running}
        # prune
        for cid in list(self.containers.keys()):
            if cid not in running_ids:
                del self.containers[cid]
        # add
        for c in running:
            if c["Id"] not in self.containers:
                pid = get_pid(self.runtime, self.sudo, c["Name"])
                if not pid:
                    continue
                paths = get_cgroup_paths(pid, self.version)
                info = inspect_container(self.runtime, self.sudo, c["Name"])
                mem_limit = info.get("HostConfig", {}).get("Memory", 0) or 0
                self.containers[c["Id"]] = {
                    "Name": c["Name"],
                    "PID": pid,
                    "Paths": paths,
                    "MemLimit": mem_limit,
                    "PrevCPU": None,
                    "PrevTime": None,
                    "CPUSum": 0.0,
                    "Count": 0
                }

    def print_stats(self):
        if self.version == "v2":
            print("(Note) v2 detected: this script currently uses v1 metric logic (extend soon).")
        print("ID           NAME           CPU %     MEM USAGE / LIMIT   MEM %    PIDS        AVG CPU %")
        now = time.time()
        for cid, meta in self.containers.items():
            stats = get_stats_v1(meta["Paths"], meta["MemLimit"]) if self.version == "v1" else {"mem_usage":0,"mem_limit":0,"mem_percent":0,"cpu_usage_ns":0,"pids_current":0,"pids_limit":0}
            prev_cpu = meta["PrevCPU"]
            prev_time = meta["PrevTime"]
            cpu_pct = 0.0
            avg_cpu = 0.0
            if prev_cpu is not None and prev_time is not None and stats["cpu_usage_ns"]:
                delta_cpu = stats["cpu_usage_ns"] - prev_cpu
                delta_t = now - prev_time
                if delta_t > 0:
                    cpu_pct = (delta_cpu / 1e9) / delta_t * 100
                meta["CPUSum"] += cpu_pct
                meta["Count"] += 1
                avg_cpu = meta["CPUSum"] / meta["Count"]
            print(format_line(cid, meta["Name"], cpu_pct, stats, avg_cpu))
            meta["PrevCPU"] = stats["cpu_usage_ns"]
            meta["PrevTime"] = now

def parse_args():
    ap = argparse.ArgumentParser(description="Simple container cgroup monitor (Docker/Podman).")
    ap.add_argument("--runtime", choices=SUPPORTED_RUNTIMES, default="docker", help="Container runtime to use.")
    ap.add_argument("--sudo", action="store_true", help="Prefix runtime commands with sudo.")
    ap.add_argument("--interval", type=float, default=1.0, help="Sampling interval seconds.")
    return ap.parse_args()

def main():
    args = parse_args()
    which_or_exit(args.runtime)
    mon = ContainerMonitor(args.runtime, args.sudo)
    try:
        while True:
            mon.update()
            mon.print_stats()
            time.sleep(args.interval)
    except KeyboardInterrupt:
        pass

if __name__ == "__main__":
    main()